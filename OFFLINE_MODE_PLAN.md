# Offline Elsword: in-process server emulation for ProjectX2

## Context

`X2.exe` (Elsword 2014, `US_SERVICE`) is an online client. It cannot reach a
character-select screen without a live ChannelServer and GameServer, and the
real backend is five executables plus four SQL Server databases.

The goal is a **single-executable offline game**: the user launches one exe and
plays — login, character list (create / soft-delete / restore), tutorial
dungeon, village, main quest chain, dungeons, inventory, skills, cash shop —
with all state persisted in a local SQLite file. No server process, no network.

The approach is **in-process server emulation**: a `CX2OfflineServer` compiled
into the client that intercepts outbound packets at the socket boundary,
answers them from SQLite and the client's own already-loaded game data, and
injects replies back into the normal receive path. Everything above the socket
layer — every state machine, every `Handler_EGS_*`, every UI — runs unmodified.

**Decisions already made** (do not relitigate):

| Decision | Choice |
|---|---|
| Topology | In-process, compiled into the exe. No sockets, no second process. |
| Authority | **Trust the client fully.** It already computes combat, drops and EXP locally (in-match play was always P2P and client-trusted). The offline server records what it is told. |
| Game data source | **Reuse the client's loaded managers** (`CX2ItemManager`, `CX2QuestManager`, …). No duplicate table loading, no `.kom` extraction. |
| Build mode | `SERV_IRUHADEV_OFFLINE` in `KTDXLIB/Always.h`, shipped as `X2_offline.exe`. |
| Data + logs | Game dir root: `els_db.sql`, `offline_server.log`, `offline_packets.log`. |
| Scope | Core solo loop + social systems stubbed + cash shop working. PvP-vs-bots is out of scope. |

Game directory: `F:\elsword stuff\elsword_2014\els_2014\237311\22191271\data`
Source trunk: `f:\elsword stuff\...\EU_CN_US\Trunk` (all paths below are relative to it)

---

## The architecture, in one picture

The client's networking runtime lives in the **`X2ServerProtocol`** project,
which has its **own private copy** of the socket layer (`X2ServerProtocol/Socket/`)
— separate from the server's copy in `KncWX2Server/Common/Socket/`. Editing it
touches client code only.

```
CX2StateServerSelect / CX2State / CX2*Manager      (X2Lib — UNCHANGED)
        |  SendPacket(id, struct)                        ^  Handler_EGS_*_ACK
        v                                                |  XGM_RECEIVE_FROM_SERVER
CX2ServerProtocol  m_pkUserProxy / m_pkChUserProxy   (X2ServerProtocol — UNCHANGED)
        |                                                ^
        v                                                |
KSession::SendPacket(KEvent&)  <== HOOK HERE      KPerformer::QueueingEvent(KEvent)  <== INJECT HERE
        |                                                ^
        +------------> CX2OfflineServer (X2Lib) ---------+
                              |
                         els_db.sql (SQLite)
```

### The three seams — all in `X2ServerProtocol/Socket/Session.cpp`

| Seam | Location | Offline behavior |
|---|---|---|
| Connect | [Session.cpp:415](X2ServerProtocol/Socket/Session.cpp#L415) `KSession::Connect` | Skip `m_spSockObj->Connect`. Still call `Begin()` (starts the worker thread). Set `m_bAuthKeyRecved = true`. Return `true`. |
| Outbound | [Session.cpp:317](X2ServerProtocol/Socket/Session.cpp#L317) `KSession::SendPacket(const KEvent&)` | Hand the `KEvent` to the offline server, log it, return `true`. Never serialize, encrypt, or touch a socket. |
| Inbound | [Session.cpp:523](X2ServerProtocol/Socket/Session.cpp#L523) `QueueingEvent(spEvent)` | The offline server calls `QueueingEvent()` on the same session with a `KEvent` it built. |

**Why this works with zero threading changes:** the client creates its proxies
with `spProxy->Init(false)` — IOCP off ([X2ServerProtocol.cpp:82](X2ServerProtocol/X2ServerProtocol.cpp#L82)) — so
`Connect()` spawns a worker thread running `KSession::Run()`
([Session.cpp:531](X2ServerProtocol/Socket/Session.cpp#L531)). That loop waits on
`WaitForMultipleObjects(..., 100)` and calls **`Tick()` unconditionally every
iteration, timeout included**. So `Tick()` fires 10×/sec even with no socket at
all, drains `m_queEvent` → `ProcessEvent` → `KActorProxy::PostEvent`
([ActorProxy.cpp:24](X2ServerProtocol/ActorProxy.cpp#L24)) → `m_pSendGameMessage(XGM_RECEIVE_FROM_SERVER, …)`
→ `CX2State::MsgProc` ([X2Lib/X2State.cpp:1110](X2Lib/X2State.cpp#L1110)).

**What we get for free by hooking this high:** no DES-CBC/HMAC-MD5 crypto, no
2-byte length framing, no `E_ACCEPT_CONNECTION_NOT` SPI key exchange, no zlib,
no `KEvent` envelope serialization. Only the *inner* payload struct is
serialized, using the exact same `KSerializer` the real server uses.

### Reading a request / writing a reply

```cpp
// Request: deserialize kEvent.m_kbuff into the packet struct
KEGS_CREATE_UNIT_REQ kReq;
KSerializer ks;
ks.BeginReading( const_cast<KSerBuffer*>( &kEvent.m_kbuff ) );
bool bOK = ks.Get( kReq );
ks.EndReading();

// Reply: build a KEvent and queue it on the same session
KEGS_CREATE_UNIT_ACK kAck;
kAck.m_iOK = XEM_OK;
KEventPtr spEvent( new KEvent );
UidType anTrace[2] = { -1, -1 };
spEvent->SetData( PI_GS_PROXY, anTrace, EGS_CREATE_UNIT_ACK, kAck );
pSession->QueueingEvent( spEvent );
```

`m_kbuff.UnCompress()` may be needed on the request — the real receive path does
it at [Session.cpp:492](X2ServerProtocol/Socket/Session.cpp#L492). The client sends with
`bCompress = false` by default ([X2ServerProtocol.h:206](X2ServerProtocol/X2ServerProtocol.h#L206)), so
this is usually a no-op; call it anyway for safety.

---

## Critical facts to keep in mind

1. **`X2StateBeginning.cpp` is dead code.** It is wrapped in
   `#ifdef ELSWORD_NEW_BEGINNING`, defined nowhere (only commented out at
   `KTDXLIB/InHouse3.h:275`, and `InHouse*.h` isn't even included under
   `_SERVICE_`). **`X2Lib/X2StateServerSelect.cpp` is the live character-select
   and login-orchestration state.** Do not waste time in `X2StateBeginning.cpp`.

2. **The exe requires an argv token.** [X2/X2.cpp:806-818](X2/X2.cpp#L806): under
   `SERV_CHANNELING_AERIA` (defined for US) it checks the **last** argv against
   `PATCHER_RUN_ONLY` = `pxk19slammsu286nfha02kpqnf729ck`
   (`KTDXLIB/OnlyGlobal/Always_US.h:11`). Double-clicking the exe returns 0
   silently. Always launch via a `.bat` (see `start.bat` in the game dir).

3. **A missing `.kom` is a deliberate null-deref crash.** [X2Lib/X2Main.cpp:1330-1350](X2Lib/X2Main.cpp#L1330)
   does `int *x = NULL; *x = 1;` under `_SERVICE_` if any of `data001.kom` …
   `data145.kom` fails to load. `KOM_FILE_COUNT = 145` is hardcoded at
   [X2Main.cpp:768](X2Lib/X2Main.cpp#L768). Also, the path prefix is `./` — the client
   **must** run with its working directory set to the game `data\` folder.

4. **No `.kom` repacking is needed.** We intercept `Connect()`, so the server
   address in `Config_US_Service.lua` (currently `174.35.125.2:9400`) is never
   used. `AddChannelServerIP_LUA` runs `gethostbyname()` on it, but a dotted
   quad resolves instantly with no network I/O. Leave the archives alone.

5. **The UDP port check is a separate path.** `KXPT_PORT_CHECK_REQ` is a raw UDP
   struct handled by `Socket/LBSUdpEcho.cpp`, **not** a `KEvent` — it will not
   appear at the `SendPacket` hook. `CX2StateServerSelect` retries it 10 times
   (`m_PortCheckRetryTime < 10`, around [X2StateServerSelect.cpp:826-850](X2Lib/X2StateServerSelect.cpp#L826))
   before giving up. Short-circuit it in Phase 1 or login stalls for ~30s.

6. **Anti-cheat is already off for `US_SERVICE`.** `NO_GAMEGUARD` is defined
   (`Always_US.h:17-19`), `CLIENT_USE_XTRAP` is commented out (`Always_US.h:4-7`),
   HackShield is dead in `KTDX.h:112-121`. The `XTrap\`, `xigncode\`,
   `gameguard.des` files in the game dir are inert leftovers. Nothing to disable.

7. **Existing assets to reuse, not rebuild:**
   - `X2Lib/X2PacketLog.{h,cpp}` (1334 lines) — the studio's own per-packet
     pretty-printer, already covers `KEGS_CONNECT_REQ`, `KEGS_VERIFY_ACCOUNT_*`,
     `KEGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK`, `KEGS_CREATE_UNIT_*`,
     `KEGS_DELETE_UNIT_*`, `KEGS_SELECT_UNIT_*`, room packets, and more.
     Wire it into the offline log rather than writing new formatters.
   - `Libs/ExternalLib/sqlite3/` — SQLite **3.53.4** amalgamation, already on
     disk (untracked; the `/*` rule in `.gitignore` hides it).
   - `DataBase/Database2/` — the **real T-SQL schema**, 68 tables in
     `Database2/Schema Objects/Tables/` plus stored procedures. Use
     `dbo.GUnit.table.sql`, `dbo.GItemInventorySize.table.sql`,
     `dbo.GInventoryInfo.table.sql` as the reference for the SQLite schema.
     Note `GUnit.Deleted` is a computed column:
     `case RegDate when DelDate then 0 else 1 end` — **that is the soft-delete
     mechanism**; restoring a character means setting `DelDate = RegDate`.
   - `KncWX2Server/GameClient/KncRobot.cpp` — the studio's own reference client;
     `:299-440` and `:1094-1180` walk the exact login sequence.
   - `KncWX2Server/GameServer/GSUserSession.cpp` + `GSUserGameCommon.cpp` — the
     real handlers. When unsure what a reply should contain, read these.

8. **Flag visibility trap.** `X2ServerProtocol/StdAfx.h` does **not** include
   `KTDX.h`, so it never sees `KTDXLIB/Always.h`. The flag must be set in *two*
   places (see Phase 0, step 3). This is exactly the "flag set on one project
   and not another silently does nothing" failure mode `CLAUDE.md` warns about.

---

## Code layout to create

```
X2ServerProtocol/
  OfflineHook.h                 NEW  - abstract hook interface + global pointer
  Socket/Session.cpp            EDIT - 3 hook points (Connect, SendPacket, and a wake helper)

X2Lib/Offline/
  X2OfflineServer.h/.cpp        NEW  - singleton, hook impl, dispatch, session registry
  X2OfflineLog.h/.cpp           NEW  - two log files, ring-free, flush-per-line
  X2OfflineDB.h/.cpp            NEW  - sqlite3 wrapper + schema + migrations
  X2OfflineSession.h            NEW  - per-connection state (account, FSM state, selected unit)
  Handlers_Login.cpp            NEW  - Phase 1
  Handlers_Unit.cpp             NEW  - Phase 2
  Handlers_Field.cpp            NEW  - Phase 3
  Handlers_Room.cpp             NEW  - Phase 4
  Handlers_Inventory.cpp        NEW  - Phase 5
  Handlers_Quest.cpp            NEW  - Phase 6
  Handlers_Shop.cpp             NEW  - Phase 7
  Handlers_Stub.cpp             NEW  - catch-all, grows every phase
```

`X2ServerProtocol` cannot include `X2Lib` headers (that would be a circular
project dependency). The hook is therefore an abstract interface declared in
`X2ServerProtocol`, implemented in `X2Lib`, and registered at startup:

```cpp
// X2ServerProtocol/OfflineHook.h
#pragma once
class KEvent;
class KSession;

class IX2OfflineHook
{
public:
    virtual ~IX2OfflineHook() {}
    // return true if the offline server consumed the packet (do not send it)
    virtual bool OnClientSend( KSession* pSession, const KEvent& kEvent ) = 0;
    virtual void OnSessionConnect( KSession* pSession, const char* szIP, unsigned short usPort ) = 0;
    virtual void OnSessionClose( KSession* pSession ) = 0;
};

extern IX2OfflineHook* g_pX2OfflineHook;   // NULL in a normal build
```

Registered from `CX2Data::ResetServerProtocol` ([X2Lib/X2Data.cpp:2122](X2Lib/X2Data.cpp#L2122)),
next to where `CX2ServerProtocol` is constructed.

---

# Phase 0 — Baseline, toolchain, and the flag

**Goal:** prove you can build and launch the stock client, get SQLite compiling
under VS2010, and land the feature flag. No behavior change yet.

### 0.1 Build and launch the stock client

```sh
msbuild X2Project_2010.sln /p:Configuration=US_SERVICE /p:Platform=Win32
```

- Artifact is `X2/US_SERVICE/x2.exe` (**lowercase** — `TargetName` is `x2`).
- The post-build step copies to `E:\Elsword_InHouse\Data\…` and **will fail**.
  Judge success by whether `X2/US_SERVICE/x2.exe` exists, not by exit code.
- Copy it to the game dir as `X2_offline.exe`.
- Create `start_offline.bat` in the game dir:
  ```
  start X2_offline.exe pxk19slammsu286nfha02kpqnf729ck
  ```
- Run it. It must reach the login screen and then fail to connect. **That
  failure is the Phase 0 baseline** — it proves the client boots, mounts all
  145 `.kom` archives, and reaches `CX2StateLogin`.

### 0.2 Add SQLite to the `X2Lib` project

- Add `Libs/ExternalLib/sqlite3/sqlite3.c` to `X2Lib/X2Lib_2010.vcxproj`.
- Set on **that file only**: `CompileAs = CompileAsC`, and disable the project
  PCH (`PrecompiledHeader = NotUsing`) — `sqlite3.c` will not compile through
  `StdAfx.h`.
- Preprocessor definitions for that file: `SQLITE_THREADSAFE=1`,
  `SQLITE_OMIT_LOAD_EXTENSION`, `SQLITE_DEFAULT_FOREIGN_KEYS=1`,
  `_CRT_SECURE_NO_WARNINGS`.
- Add `Libs/ExternalLib/sqlite3` to `AdditionalIncludeDirectories` for `X2Lib`.
- **Risk:** SQLite 3.53.4 is far newer than VS2010. It still carries explicit
  `_MSC_VER<1800` compatibility branches (e.g. `sqlite3.c:132`), so it is
  expected to build. If it does not, drop back to the 3.39.x amalgamation
  rather than fighting it.
- `.gitignore`: `Libs/` is excluded by the root `/*` rule. Either leave sqlite3
  untracked (simplest, it is vendored source) or add an explicit
  `!/Libs/` + re-ignore ladder mirroring the existing `KNCSDK` block at
  `.gitignore:23-40`.

### 0.3 Define the flag — in TWO places

Append to the end of `KTDXLIB/Always.h`, using the house block, **in ASCII**
(the file is CP949; do not re-encode it — see the Edit-tool warning in
`CLAUDE.md`):

```c
//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: <today>
// Description: Offline mode - emulate the game/channel servers in-process,
//              persist all player state to a local SQLite file (els_db.sql).
//              NOTE: X2ServerProtocol does NOT include KTDX.h, so this flag
//              must ALSO be set in X2ServerProtocol_2010.vcxproj's
//              US_SERVICE PreprocessorDefinitions. Toggle both together.
#define SERV_IRUHADEV_OFFLINE
//////////////////////////////////////////////////////////////////////////
```

Then add `SERV_IRUHADEV_OFFLINE` to the `US_SERVICE|Win32` (and `US_TEST|Win32`,
for symmetry) `PreprocessorDefinitions` in `X2ServerProtocol/X2ServerProtocol_2010.vcxproj`.

Do **not** put this flag in `KncWX2Server/Common/ServerDefine.h` — that file
compiles into the servers too.

### 0.4 Verify the encoding rule

After any edit to a file containing non-ASCII bytes (`Always.h`,
`X2StateServerSelect.cpp`, `Session.cpp`, most of `X2Lib`):

```sh
file <path>                  # must NOT become "UTF-8 text"
git diff --stat <path>       # a 5-line edit must not show 500 changed lines
```

If the encoding flipped, `git checkout -- <path>` (confirm with the user first)
and redo the edit with a byte-level Python script in `"rb"`/`"wb"` mode.

### Exit test
`X2_offline.exe` builds with the flag defined, launches via `start_offline.bat`,
reaches the login screen, and fails to connect. `sqlite3.c` compiles clean.

---

# Phase 1 — The loopback seam and character select

**Goal (this is the milestone that matters):** launch `X2_offline.exe` with no
server anywhere and reach a working character-select screen showing one
hardcoded dummy character, with `offline_packets.log` listing every request the
client made and every reply given — and flagging anything unhandled.

**No SQLite yet.** Everything is hardcoded. This phase proves the seam.

### 1.1 The logging facility — build this first

`X2Lib/Offline/X2OfflineLog.{h,cpp}`. Two files, both written to the process
working directory (which is the game `data\` dir):

- **`offline_server.log`** — lifecycle and errors: startup, DB open, session
  connect/close, handler exceptions, warnings.
- **`offline_packets.log`** — one line per packet in each direction.

Format — keep it greppable:

```
[HH:MM:SS.mmm] C->S  GS   EGS_CREATE_UNIT_REQ            (id=37, 24 bytes)  HANDLED
[HH:MM:SS.mmm] S->C  GS   EGS_CREATE_UNIT_ACK            (id=38, 812 bytes)
[HH:MM:SS.mmm] C->S  GS   EGS_SOME_THING_REQ             (id=904, 8 bytes)  *** UNHANDLED ***
[HH:MM:SS.mmm] C->S  CH   ECH_VERIFY_ACCOUNT_REQ         (id=1153, 64 bytes) HANDLED
```

Requirements:
- Name the event via `KEvent::GetIDStr()` — it is already wired to the
  `ms_szEventID[]` table built from the same X-macro headers.
- Tag the proxy as `GS` or `CH` so the two sockets are distinguishable.
- **Flush after every line.** A crash must not lose the last packet — that line
  is usually the cause.
- `*** UNHANDLED ***` must be trivially greppable. This is the core development
  loop for every later phase: play until something breaks, grep for `UNHANDLED`,
  implement those handlers, repeat.
- Optionally call into `CX2PacketLog::PrintLog(...)` for packets it already
  knows how to pretty-print.

### 1.2 The hook

**`X2ServerProtocol/OfflineHook.h`** — as sketched above. Define
`IX2OfflineHook* g_pX2OfflineHook = NULL;` in `Session.cpp` (or a new
`OfflineHook.cpp`).

**`X2ServerProtocol/Socket/Session.cpp`**, three edits, all wrapped:

```cpp
// In KSession::Connect (line ~415), immediately after PROXY_ONLY:
#ifdef SERV_IRUHADEV_OFFLINE
    if( g_pX2OfflineHook != NULL )
    {
        g_pX2OfflineHook->OnSessionConnect( this, szIP_, usPort_ );
        Begin();                  // start the worker thread; Run() Ticks every 100ms
        m_bAuthKeyRecved = true;  // skip the SPI key-exchange wait
        return true;
    }
#endif SERV_IRUHADEV_OFFLINE

// At the very top of KSession::SendPacket( const KEvent& ) (line 317),
// BEFORE the m_spSockObj null check:
#ifdef SERV_IRUHADEV_OFFLINE
    if( g_pX2OfflineHook != NULL )
        return g_pX2OfflineHook->OnClientSend( this, kEvent );
#endif SERV_IRUHADEV_OFFLINE
```

Also add a public `void WakeForOffline()` on `KSession` that does
`SetEvent( m_hEvents[EVENT_RECV_COMPLETED] )`, so a queued reply is delivered
immediately instead of waiting up to 100 ms. A full login is ~15 chained
round-trips; without this it costs ~1.5 s, which is tolerable but sloppy.

Keep the house `#endif SERV_IRUHADEV_OFFLINE` trailing-token style.

### 1.3 The offline server skeleton

`X2Lib/Offline/X2OfflineServer.{h,cpp}`:

- Singleton `CX2OfflineServer`, implements `IX2OfflineHook`.
- Registered in `CX2Data::ResetServerProtocol` ([X2Lib/X2Data.cpp:2122](X2Lib/X2Data.cpp#L2122)),
  under `#ifdef SERV_IRUHADEV_OFFLINE`.
- Maintains `std::map< KSession*, KOfflineSession >` — which proxy is the
  channel server vs. the game server, plus per-session FSM state mirroring
  `KncWX2Server/Common/GSFSMState_def.h`
  (`S_INIT → S_UID_UPDATED → S_LOGINED → S_SERVER_SELECT → S_FIELD_MAP / S_ROOM`).
- `OnClientSend` → log → look up `kEvent.m_usEventID` in a `switch` → call the
  handler → default case logs `*** UNHANDLED ***` and returns `true`
  (**consume it** — never let an unhandled packet reach the dead socket path).
- Helpers all handlers use:
  ```cpp
  template< typename T > bool ReadReq( const KEvent& kEvent, T& out );
  template< typename T > void Reply( KSession* pSession, unsigned short usEventID, T& data );
  void ReplyID( KSession* pSession, unsigned short usEventID );   // no payload
  ```

**Threading note:** `OnClientSend` is called on whichever thread called
`SendPacket` (usually the render thread); `Tick()` drains on the session's
worker thread. `QueueingEvent` is already lock-protected
([Performer.cpp:67](X2ServerProtocol/Performer.cpp#L67)). Guard the offline server's own
state with a critical section. Keep handlers synchronous and short.

### 1.4 Short-circuit the UDP port check

`CX2StateServerSelect` sends `KXPT_PORT_CHECK_REQ` over **raw UDP**, not through
the hook. Under `SERV_IRUHADEV_OFFLINE`, edit `X2Lib/X2StateServerSelect.cpp`
around the `m_bPortCheckReq` / `Handler_KXPT_PORT_CHECK_REQ()` block
([:770-850](X2Lib/X2StateServerSelect.cpp#L770)) to mark the check as immediately
succeeded and skip the send. Otherwise login stalls ~30 s on 10 retries.

### 1.5 Packets to implement in Phase 1

Read the client's `Handler_*` for each to see exactly which fields it inspects;
read `GSUserSession.cpp` / `GSUserGameCommon.cpp` for what the real server put
there. Struct definitions are in `KncWX2Server/Common/{Common,Client}Packet.h`,
serializers in the matching `.cpp`.

**Channel proxy (`PI_CHANNEL_USER`):**

| Request | Reply | Notes |
|---|---|---|
| `ECH_VERIFY_ACCOUNT_REQ` | `ECH_VERIFY_ACCOUNT_ACK` | `KECH_VERIFY_ACCOUNT_REQ` at `ClientPacket.h:5488`. Accept any ID/password. Return `m_iOK = XEM_OK` and a fabricated passport string; the client uses the passport instead of the password from here on. Client handler: [X2StateLogin.cpp:1187](X2Lib/X2StateLogin.cpp#L1187). |
| `ECH_GET_SERVERGROUP_LIST_REQ` | `ECH_GET_SERVERGROUP_LIST_NOT` **then** `_ACK` | IDs in `Common/OnlyGlobal/EventID_Client_Global.h:117-119`. The `_NOT` carries the data. One group. Client: [X2StateServerSelect.cpp:7937](X2Lib/X2StateServerSelect.cpp#L7937). |
| `ECH_GET_CHANNEL_LIST_REQ` | `ECH_GET_CHANNEL_LIST_NOT` **then** `_ACK` | One `KChannelInfo` (`CommonPacket.h:6102`). `m_wstrIP`/`m_usMasterPort` are ignored offline but must be non-empty — the client feeds them straight into `Handler_EGS_CONNECT_REQ`. Set `m_iCurrentUser = 1`, `m_iMaxUser = 100`. Client: [:7125](X2Lib/X2StateServerSelect.cpp#L7125). |
| `ECH_DISCONNECT_REQ` | `ECH_DISCONNECT_ACK` | Client drops the channel socket after connecting to the game server ([:7192](X2Lib/X2StateServerSelect.cpp#L7192)). |

**Game proxy (`PI_GS_USER`):**

| Request | Reply | Notes |
|---|---|---|
| `EGS_CONNECT_REQ` | `EGS_CONNECT_ACK` | `CommonPacket.h:6296-6314`. Request is just `m_wstrVersion`. Reply: `m_iOK`, `m_UDPPort`, `m_iChannelID`. Ignore the version. |
| `EGS_VERIFY_ACCOUNT_REQ` | `EGS_VERIFY_ACCOUNT_ACK`, then push `ENX_USER_LOGIN_NOT` | Req is `KUserAuthenticateReq` (`CommonPacket.h:265`, aliased at `:6318`). Ack is `CommonPacket.h:6320` and carries a `KAccountInfo` (15 fields, `CommonPacket.h:362`). `ENX_USER_LOGIN_NOT` is what moves the client to `S_LOGINED` — see `KncRobot.cpp:350-357`. |
| `EGS_CURRENT_TIME_REQ` | `EGS_CURRENT_TIME_ACK` | Local system time. |
| `EGS_GET_SERVER_SET_DATA_REQ` | `EGS_GET_SERVER_SET_DATA_ACK` | No request body (`SendID`). Real impl: `GSUserSession.cpp:2071-2147`. One server set. |
| `EGS_STATE_CHANGE_SERVER_SELECT_REQ` | `EGS_STATE_CHANGE_SERVER_SELECT_ACK` | No body. Real impl: `GSUserSession.cpp:1987-2063`. Advance session FSM to `S_SERVER_SELECT`. |
| `EGS_SELECT_SERVER_SET_REQ` | `EGS_SELECT_SERVER_SET_ACK` | `ClientPacket.h:223-235`. Returns IP+port of the "target" game server and a `KAccountBlockInfo` — set the block info to "not blocked". |
| `EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ` | `EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK` | **The character list.** `ClientPacket.h:36-74`. Wrapper is small: `m_iOK`, `m_nUnitSlot`, `vector<KUnitInfo>`, plus a few flags. Phase 1: return **one** hardcoded `KUnitInfo`. |
| `EGS_KEYBOARD_MAPPING_INFO_NOT`, `EGS_CHAT_OPTION_INFO_NOT` | (server-push) | Push defaults on entry; client reads them at [:8155](X2Lib/X2StateServerSelect.cpp#L8155) and [:8227](X2Lib/X2StateServerSelect.cpp#L8227). |

### 1.6 The dummy `KUnitInfo`

`KUnitInfo` (`CommonPacket.h:1335-1473`) has **~70 fields**, serialized at
`CommonPacket.cpp:943`. Most can be zero. Write one helper —
`MakeDefaultUnitInfo( UidType nUnitUID, char cUnitClass, const std::wstring& wstrName, int iLevel )` —
that zero-initializes the struct and then sets only:

`m_iOwnerUserUID`, `m_nUnitUID`, `m_cUnitClass`, `m_wstrNickName`, `m_ucLevel`,
`m_iEXP`, `m_iED`, `m_kStat`, `m_kGameStat`, `m_bDeleted = false`,
`m_wstrLastDate`, `m_iSpirit` / `m_iSpiritMax`.

This helper is reused by every later phase — build it carefully once.

### Exit test

1. `start_offline.bat` with **no server process running anywhere**.
2. Login screen accepts any credentials (or auto-logs in via the existing
   `STATIC_AUTO_LOGIN`).
3. Character-select screen appears showing the one dummy character.
4. `offline_packets.log` contains the full ordered request/reply chain.
5. Any packet not yet handled appears as `*** UNHANDLED ***` and the client does
   **not** hang or crash on it.

---

# Phase 2 — SQLite persistence and character CRUD

**Goal:** the character list is real. Create, soft-delete, restore, and select
characters; state survives a restart.

### 2.1 The DB layer

`X2Lib/Offline/X2OfflineDB.{h,cpp}` — a thin `sqlite3*` wrapper. Do **not**
port `KODBC` or any of the 785 stored procedures; the offline server talks to
SQLite directly.

- Open `els_db.sql` in the working directory (the game `data\` dir). Create it
  with the full schema if absent.
- `PRAGMA journal_mode = WAL; PRAGMA foreign_keys = ON; PRAGMA synchronous = NORMAL;`
- One `schema_version` table; a simple integer migration ladder so later phases
  can add tables without wiping saves.
- Prepared statements, `sqlite3_bind_*`. Everything the client sends is
  `std::wstring` (UTF-16LE); store as UTF-8 via `sqlite3_bind_text16` or convert
  with the existing `KncUtil::toNarrowString`. **Pick one convention and hold
  it** — mixed encodings here will show up much later as garbled nicknames.

### 2.2 Schema

Model it on the real T-SQL in `DataBase/Database2/Database2/Schema Objects/Tables/`,
but only the columns the offline server actually needs. Start with:

```sql
CREATE TABLE account (
  user_uid      INTEGER PRIMARY KEY,
  login_id      TEXT NOT NULL UNIQUE,
  unit_slots    INTEGER NOT NULL DEFAULT 3,
  cash_balance  INTEGER NOT NULL DEFAULT 0,
  created_at    TEXT NOT NULL
);

-- mirrors dbo.GUnit; soft delete follows the original's computed-column trick:
--   deleted  <=>  del_date != reg_date
CREATE TABLE unit (
  unit_uid    INTEGER PRIMARY KEY AUTOINCREMENT,
  user_uid    INTEGER NOT NULL REFERENCES account(user_uid),
  unit_class  INTEGER NOT NULL,
  nickname    TEXT NOT NULL,
  level       INTEGER NOT NULL DEFAULT 1,
  exp         INTEGER NOT NULL DEFAULT 0,
  ed          INTEGER NOT NULL DEFAULT 0,
  sp          INTEGER NOT NULL DEFAULT 0,
  spirit      INTEGER NOT NULL DEFAULT 0,
  last_pos    INTEGER NOT NULL DEFAULT 0,
  reg_date    TEXT NOT NULL,
  del_date    TEXT NOT NULL,          -- == reg_date means "not deleted"
  last_date   TEXT NOT NULL
);

CREATE TABLE inventory_size ( unit_uid INTEGER, category INTEGER, size INTEGER,
                              PRIMARY KEY( unit_uid, category ) );
CREATE TABLE item          ( item_uid INTEGER PRIMARY KEY AUTOINCREMENT,
                             unit_uid INTEGER, category INTEGER, slot INTEGER,
                             item_id INTEGER, count INTEGER, equipped INTEGER,
                             blob_data BLOB );   -- serialized KInventoryItemInfo tail
CREATE TABLE unit_skill    ( unit_uid INTEGER, skill_id INTEGER, level INTEGER, slot INTEGER );
CREATE TABLE unit_quest    ( unit_uid INTEGER, quest_id INTEGER, state INTEGER, progress BLOB );
CREATE TABLE unit_dungeon  ( unit_uid INTEGER, dungeon_id INTEGER, clear_count INTEGER, best_rank INTEGER );
```

`blob_data` / `progress` as serialized `KSerBuffer`s is a deliberate escape
hatch: several client structs (`KInventoryItemInfo`, quest progress) have deep
nested state that is not worth normalizing when nothing else queries it. Use
the same `KSerializer` the packets use. Normalize only what you need to filter
or sort on.

### 2.3 Handlers

`X2Lib/Offline/Handlers_Unit.cpp`:

| Packet | Behavior |
|---|---|
| `EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ/ACK` | Replace the Phase 1 hardcoded list with a real query. Include soft-deleted units — `KUnitInfo` has `m_bDeleted`, `m_trDelAbleDate`, `m_trRestoreAbleDate`, and the client renders the restore UI from them. |
| `EGS_CREATE_UNIT_REQ/ACK` | `ClientPacket.h:77-104`. Validate the nickname is unique among non-deleted units; reject with the right `XEM_ERROR*` code from `X2Lib/X2Define.h` otherwise. Insert with `del_date = reg_date`. Seed starting gear, skills and inventory sizes (see 2.4). Reply with the full new `KUnitInfo`. |
| `EGS_DELETE_UNIT_REQ/ACK` | `ClientPacket.h:107-152`. **Soft delete:** set `del_date = now`. Set `m_trRestoreAbleDate` on the returned info so the client offers restore. Never `DELETE FROM`. |
| restore | Find the packet the client sends from the restore button in `X2StateServerSelect.cpp` (grep `Restore` near the unit-slot UI) and log it as `UNHANDLED` first to learn its ID. Handler sets `del_date = reg_date`. |
| `EGS_SELECT_UNIT_REQ` | `CommonPacket.h:7799`. Push `EGS_SELECT_UNIT_1_NOT` … `_5_NOT`, **then** `EGS_SELECT_UNIT_ACK` (`{ int m_iOK; }`, `CommonPacket.h:8112`). Order matters. |

### 2.4 The five `SELECT_UNIT` notifications

`SERV_SELECT_UNIT_PACKET_DIVISION` is defined (`ServerDefine.h:3452`), so the
character payload is split across five packets. Serializers:

| Packet | Put at | Fields | Carries |
|---|---|---|---|
| `_1_NOT` | `CommonPacket.cpp:7692` | 19 | `KUnitInfo`, inventory slot sizes, `map<UidType,KInventoryItemInfo>`, resurrection stones, acquired/unsealed skills, skill notes |
| `_2_NOT` | `CommonPacket.cpp:7791` | 5 | quests, completed quests |
| `_3_NOT` | `CommonPacket.cpp:7817` | 9 | missions, titles |
| `_4_NOT` | `CommonPacket.cpp:7859` | 14 | pets, messenger, blacklist, deny options |
| `_5_NOT` | `CommonPacket.cpp:7915` | 19 | henir ranking, connect-time events, shop agency, events |

Phase 2 populates `_1_NOT` and `_2_NOT` from SQLite; send `_3/_4/_5_NOT` with
empty collections. They are filled in Phases 5-7.

**Starting-gear seeding:** get the item IDs from the client's own
`CX2ItemManager` / the class templates rather than hardcoding a list — that is
the "reuse the client's loaded managers" decision. Inventory category 11
(`ST_E_QUICK_SLOT`) must be seeded at **6** slots to stay consistent with the
existing `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` mod (see `MODS.md`).

### Exit test
Create a character → it appears in the list → restart the exe → it is still
there. Delete it → shows as deleted with a restore timer → restore it → back to
normal. Select it → the client proceeds past character select without error.

---

# Phase 3 — Village and field entry

**Goal:** a selected character loads into the village map and can walk around.

- `EGS_STATE_CHANGE_FIELD_REQ/ACK` (`EventID_Client.h:977-978`). The client's
  `Handler_EGS_STATE_CHANGE_VILLAGE_MAP_REQ` ([X2StateServerSelect.cpp:4247](X2Lib/X2StateServerSelect.cpp#L4247))
  delegates to it at [:5928](X2Lib/X2StateServerSelect.cpp#L5928); the ACK handler is at
  [:6016](X2Lib/X2StateServerSelect.cpp#L6016). There is no separate
  `EGS_STATE_CHANGE_VILLAGE_MAP_*` event ID — do not go looking for one.
- Field enter/leave, field user list (return just the local player), NPC list,
  village channel/instance info.
- Advance the session FSM to `S_FIELD_MAP`.
- Persist `unit.last_pos` so the character returns to where they logged out.
- Reference: `KncWX2Server/GameServer/` field handlers.

**Discovery loop:** run, walk into the village, grep `offline_packets.log` for
`UNHANDLED`, implement, repeat. Expect ~20-40 new packets in this phase, most of
them trivially "ACK with OK".

### Exit test
Character select → loading → village map renders, the character is controllable,
NPCs are present, no error popups, no infinite loading.

---

# Phase 4 — Rooms, the tutorial dungeon, and dungeon results

**Goal:** the first-run flow completes — new character → tutorial stage →
village → main progression quest triggered.

### 4.1 Room emulation

Dungeons run inside a "room" even solo. The offline server must fabricate a
single-occupant room:

- `EGS_ROOM_LIST_REQ/ACK`, `EGS_CREATE_ROOM_REQ/ACK`, `EGS_JOIN_ROOM_REQ/ACK`,
  `EGS_JOIN_ROOM_NOT`, `EGS_LEAVE_ROOM_NOT`, `EGS_CHANGE_TEAM_REQ/ACK`,
  and the game-start packet. Structs: `KRoomInfo`, `KRoomUserInfo`,
  `KRoomSlotInfo` in `CommonPacket.h` (all three already have
  `CX2PacketLog::PrintLog` overloads — use them).
- The local player is always host. `CX2Game::IsHost()` must return true so the
  client drives the simulation; **in-match traffic is P2P UDP and never reaches
  the offline server at all**, which is why "trust the client" costs nothing here.
- Advance the session FSM to `S_ROOM`.

### 4.2 The tutorial

`CX2StateServerSelect` already has the tutorial UI wired
(`m_pDLGTutorialMsgBox`, `m_pDLGGoToFirstDungeon` — see the constructor around
[X2StateServerSelect.cpp:45-47](X2Lib/X2StateServerSelect.cpp#L45)). Find what
gates it (likely a flag in the unit data or `TutorSystem.lua` in the game dir,
currently empty) and make a freshly-created character trigger it. Get the
tutorial dungeon's ID from the client's own dungeon manager rather than
hardcoding it.

### 4.3 Results

- Dungeon clear/fail result packets: accept the client's reported EXP, ED,
  drops and clear rank; write them to `unit`, `item`, `unit_dungeon`.
- Level-up: recompute from the client's EXP table via its manager; do not
  duplicate the table.
- Return to village afterwards.

### Exit test
Fresh character → tutorial dungeon runs and completes → EXP/ED/items awarded and
persisted → returns to village → main progression quest is offered.

---

# Phase 5 — Inventory, equipment, and skills

**Goal:** everything the player owns and has learned round-trips through SQLite.

- Inventory move/swap/split/stack/sell/destroy; `EGS_GET_MY_INVENTORY_REQ/ACK`
  (`ClientPacket.h:157-169`), `EGS_SELECT_UNIT_INVENTORY_INFO_NOT` (`:171-175`).
- Equip/unequip, `m_mapEquippedItem` in `KUnitInfo`.
- Skill tree: learn, reset, slot assignment. Keep consistent with the existing
  `SERV_IRUHADEV_SKILLTREE_NO_LOCK` and `SERV_IRUHADEV_SKILL_SLOT_B_FREE` mods —
  the offline server must apply the same relaxations the real
  `GameServer/UserSkillTree.cpp` does under those flags (`MODS.md` names the
  exact sites).
- Quick slots: category 11 pinned to 6 slots, matching
  `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` (`GameServer/Inventory.cpp:500-534`).
- Now fill `EGS_SELECT_UNIT_1_NOT` completely.
- Item enhancement / socketing if the UI reaches for it.

### Exit test
Equip items, learn skills, rearrange the bag, restart — every change persisted
exactly.

---

# Phase 6 — Quests and progression

**Goal:** the main story chain is playable start to finish.

- Quest accept / progress / complete / reward; `EGS_SELECT_UNIT_2_NOT` fully
  populated from `unit_quest`.
- Quest state comes from the client's `CX2QuestManager` definitions; the offline
  server only records "which quest, what state, what progress counters".
- Titles and missions → `EGS_SELECT_UNIT_3_NOT`.
- Class change / job advancement (`XS_CLASS_CHANGE`, `X2StateClassChange.h`) —
  persist `unit_class`.

### Exit test
Play from level 1 through several story quests and at least one class change,
with progression intact across restarts.

---

# Phase 7 — Cash shop and social stubs

### 7.1 Cash shop (working)

- Catalog from the client's own cash-shop script data.
- `account.cash_balance` as the currency; make the starting balance
  configurable (a row in a small `settings` table is enough).
- Purchase → deduct → grant item → persist. Handlers in
  `X2Lib/X2CashShop.cpp` show which packets are involved
  (dispatcher at [X2CashShop.cpp:1937](X2Lib/X2CashShop.cpp#L1937)).
- Keep hiding the quick-slot expansion ticket per
  `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`.
- `EGS_SELECT_UNIT_5_NOT` shop-agency fields.

### 7.2 Social stubs (present but empty)

Every one of these must return a well-formed empty response — never nothing,
which produces infinite loading spinners:

| System | Dispatcher | Stub behavior |
|---|---|---|
| Guild | [X2GuildManager.cpp:753](X2Lib/X2GuildManager.cpp#L753) | no guild, empty list |
| Party | [X2PartyManager.cpp:29](X2Lib/X2PartyManager.cpp#L29) | solo party of one |
| Friends / messenger | [X2Community.cpp:2045](X2Lib/X2Community.cpp#L2045) | empty; `_4_NOT` empty |
| Mail | [X2PostBox.cpp:1155](X2Lib/X2PostBox.cpp#L1155) | empty inbox |
| Chat | [X2ChatBox.cpp:3196](X2Lib/X2ChatBox.cpp#L3196) | echo locally, no broadcast |
| Ranking / profile | [X2ProfileManager.cpp:185](X2Lib/X2ProfileManager.cpp#L185) | empty boards |
| Pets / riding pets | [X2PetManager.cpp:401](X2Lib/X2PetManager.cpp#L401), [X2RidingPetManager.cpp:253](X2Lib/X2RidingPetManager.cpp#L253) | owned pets from SQLite; no server features |

Where a UI entry point is meaningless offline (auto-party matching, PvP
matchmaking, guild creation), prefer disabling the button in the client over
returning a confusing error — but only where that is a small, contained edit.

### Exit test
Open every menu in the game. Nothing hangs, nothing shows a network error, cash
shop purchases work and persist.

---

# Phase 8 — Hardening and packaging

- **Unhandled-packet sweep.** Play everything, collect all remaining
  `*** UNHANDLED ***` IDs, classify each as "needs an implementation" or
  "safe to ignore" and add explicit ignore entries so the log stays clean and
  signal-bearing.
- **Save integrity.** Wrap multi-statement operations in transactions. Write a
  `.bak` copy of `els_db.sql` on clean shutdown. Confirm WAL recovery after a
  kill -9 leaves a consistent DB.
- **Crash resilience.** Every handler wrapped so a bad packet logs and returns
  instead of taking down the process.
- **Log rotation.** Cap `offline_packets.log`; it grows fast during play.
- **Packaging.** `X2_offline.exe` + `start_offline.bat` in the game dir. Confirm
  a fresh copy of the game dir (no `els_db.sql`) bootstraps a new DB and reaches
  character creation.
- **Documentation.** Add `SERV_IRUHADEV_OFFLINE` to `MODS.md` following the
  existing table format, including the two-place-definition warning. Update
  `CLAUDE.md` with a short "Offline mode" section pointing at
  `X2Lib/Offline/` and the three seams.

---

## Verification — how to test at any point

```sh
# build
msbuild X2Project_2010.sln /p:Configuration=US_SERVICE /p:Platform=Win32
# artifact (lowercase); ignore the post-build copy failure
ls X2/US_SERVICE/x2.exe

# deploy
cp X2/US_SERVICE/x2.exe "F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data/X2_offline.exe"

# run — MUST be from the data dir, MUST have the argv token
cd "F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data" && ./start_offline.bat
```

```sh
# the core development loop
grep "UNHANDLED" offline_packets.log | sort | uniq -c | sort -rn
tail -50 offline_server.log
sqlite3 els_db.sql "select unit_uid, nickname, level, del_date = reg_date as alive from unit;"
```

Encoding check after touching any file with non-ASCII bytes:

```sh
file <path>                 # must not have become UTF-8
git diff --stat <path>      # small edit => small diff
```

There is no runnable test suite in this repo (the one CppUnit fixture is gated
behind `CPPUNIT_BY_TOOL_TEAM`, commented out). Verification is by running the
game and reading the two log files.

## Rollback

Comment out `#define SERV_IRUHADEV_OFFLINE` in `KTDXLIB/Always.h` **and** remove
it from `X2ServerProtocol_2010.vcxproj`'s preprocessor definitions, then
rebuild. `g_pX2OfflineHook` stays `NULL`, every hook compiles out, and the
client is byte-for-byte the stock online build.
