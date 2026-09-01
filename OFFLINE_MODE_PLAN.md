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
msbuild X2/X2_2010.vcxproj /p:Configuration=US_SERVICE /p:Platform=Win32 \
  "/p:SolutionDir=<absolute path to Trunk>\"
```

- Artifact is `X2/US_SERVICE/x2.exe` (**lowercase** — `TargetName` is `x2`).
- **`/p:SolutionDir` is mandatory**, with the trailing backslash. Every
  `IncludePath` entry for Boost, DXSDK, KNCSDK and freetype is written as
  `$(SolutionDir)Libs\…`, so building the project standalone without it
  dies on `fatal error C1083: Cannot open include file: 'boost/shared_ptr.hpp'`.
- **`msbuild X2Project_2010.sln` never produces the exe** — the original
  instruction here was wrong. The solution builds the three client libs
  (KTDXLIB, X2Lib, X2ServerProtocol) correctly, but `X2_2010.vcxproj` itself
  never runs: the `US_SERVICE` sweep drags in ~15 dead tool/server projects
  (NexonModule alone throws ~2400 errors, the five servers throw `MSB8013`
  "doesn't contain the Configuration"), msbuild returns 1, and the exe is
  silently never linked. Build the libs from the sln if you like, but link the
  exe with the direct invocation above.
- **The post-build step does not run and does not fail** — contradicting both
  this plan's first draft and `CLAUDE.md`'s *"Post-build event will fail here"*
  section. [X2/X2_2010.vcxproj:699](X2/X2_2010.vcxproj#L699) sets
  `PostBuildEventUseInBuild` to `false` for `US_SERVICE|Win32`, so the
  `E:\Elsword_InHouse\…` copy is disabled in this configuration and a clean
  client build exits **0**. Judging by the artifact is still good practice,
  just not for the stated reason.
- Copy it to the game dir as `X2_offline.exe`.
- Create `start_offline.bat` in the game dir:
  ```
  start X2_offline.exe pxk19slammsu286nfha02kpqnf729ck
  ```
- Run it with the working directory set to the game `data\` folder. It must
  reach the login screen and then fail to connect. **That failure is the
  Phase 0 baseline** — it proves the client boots, mounts all
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
- **The VS2010 risk was real, and the fix is one define — do not downgrade
  SQLite.** 3.53.4 fails to compile with two instances of
  `error C2065: 'INFINITY' : undeclared identifier` (`sqlite3.c:37174` and
  `:37204`); C99 `INFINITY` only reached MSVC's `math.h` in VS2013. Add
  `INFINITY=HUGE_VAL` to the front of that file's `PreprocessorDefinitions`:
  `HUGE_VAL` is VS2010's double +infinity, and `<math.h>` is already included
  at `sqlite3.c:36409`, well above both uses. The third occurrence,
  `sqlite3.c:15697`, sits inside a dead `#ifdef SQLITE_OMIT_FLOATING_POINT`
  block, so the define produces no redefinition warning. With that, `sqlite3.c`
  compiles clean and the vendored amalgamation stays byte-identical — nothing
  under `Libs/ExternalLib/sqlite3/` was edited.
- The `AdditionalIncludeDirectories` entry matters only for *our* later
  `#include "sqlite3.h"` from `X2Lib` code; `sqlite3.c`'s own quoted include
  resolves next to the source file regardless.
- `sqlite3.c` compiles in **all** ~50 configurations, not just `US_SERVICE`.
  Until a later phase references it, it is unreferenced dead code that the
  linker drops, so `x2.exe` does not change size.
- `.gitignore`: `Libs/` is excluded by the root `/*` rule. Either leave sqlite3
  untracked (simplest, it is vendored source) or add an explicit
  `!/Libs/` + re-ignore ladder mirroring the existing `KNCSDK` block at
  `.gitignore:23-40`.

### 0.3 Define the flag — in TWO places

Append to the end of `KTDXLIB/Always.h`, using the house block, **in ASCII**.

Note on this specific file: `Always.h` is **already UTF-8**, and its Korean
comments were already destroyed (every multi-byte sequence is now U+FFFD)
*before* the git import — `git show <first-commit>:KTDXLIB/Always.h | file -`
confirms it, so this is neither recoverable nor something a later edit caused.
Appending ASCII to it is therefore safe and does not change its encoding class.
The CP949 caution in `CLAUDE.md` still applies in full to every *other* file
later phases will touch (`X2StateServerSelect.cpp`, `Session.cpp`, most of
`X2Lib`):

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

In practice all Phase 0 edits were made with such a script from the start,
matching exact byte anchors and asserting each anchor was unique. That is the
recommended default for this tree rather than a recovery step — it also
catches a stale assumption (a missing anchor aborts the run) instead of
silently editing the wrong place. The `.vcxproj` files are UTF-8 **with BOM**
and CRLF; preserve both.

### Exit test — PASSED (2026-08-31)

`X2_offline.exe` builds with the flag defined, launches via `start_offline.bat`,
reaches the login screen, and fails to connect. `sqlite3.c` compiles clean.

Observed: the client boots fully and stalls on the **"connecting to server"**
dialog, which is the intended baseline. The client runs exclusive-fullscreen
and cannot be screenshotted (see below), so these are the signals that stood in
for a visual check:

- process alive well past the `.kom` mount — a missing archive is an instant
  null-deref (*Critical facts* #3) — ~412 MB working set, window title
  `Elsword`;
- `log.htm` in the game dir rewritten at launch;
- exactly one bound UDP socket (`0.0.0.0:8493`) and **zero** TCP connections,
  i.e. the `KXPT_PORT_CHECK_REQ` path from *Critical facts* #5.

Footprint: 21 inserted lines across `KTDXLIB/Always.h`,
`X2Lib/X2Lib_2010.vcxproj` and `X2ServerProtocol/X2ServerProtocol_2010.vcxproj`;
no `.cpp`/`.h` logic touched. `sqlite3.obj` (2.8 MB) verified present inside
`X2/X2Lib.lib`.

### Operational notes that apply to every later phase

Phases 1+ mean restarting `X2_offline.exe` constantly. Two things make that
slower than expected:

- **The client resists termination.** `taskkill /F /T /PID <pid>` returns
  `ERROR: … could not be terminated. Access is denied.`, and both
  `Process.CloseMainWindow()` and a posted `WM_CLOSE` are ignored. Killing it
  needs Task Manager or an elevated shell. Budget for this in any
  build-run-inspect loop; do not assume a script can reclaim the process.
- **Exclusive fullscreen holds the display.** `GameOptions.lua` has
  `FULLSCREEN = TRUE`, so a hung client leaves the desktop showing a flat
  colour, and a GDI screen capture returns a flat surface rather than the game.
  `ShowWindow(hwnd, SW_MINIMIZE)` releases the display mode immediately without
  killing the process — that is the fast recovery. For genuine visual
  confirmation of a UI state, set `FULLSCREEN = FALSE` in `GameOptions.lua`
  first, and back the file up: the client rewrites it on a graceful exit.

---

# Phase 1 — The loopback seam and character select

**Goal (this is the milestone that matters):** launch `X2_offline.exe` with no
server anywhere and reach a working character-select screen showing one
hardcoded dummy character, with `offline_packets.log` listing every request the
client made and every reply given — and flagging anything unhandled.

**No SQLite yet.** Everything is hardcoded. This phase proves the seam.

### 1.1 The logging facility — build this first

> **Superseded in part.** The advice below to name events with
> `KEvent::GetIDStr()` and to reuse `CX2PacketLog::PrintLog` does not work in a
> `US_SERVICE` build — see *Corrections to this plan*, item 1, after the exit
> test.

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

> **Superseded in part.** The `WakeForOffline()` /
> `SetEvent( m_hEvents[EVENT_RECV_COMPLETED] )` suggestion at the end of this
> section is unsafe — see *Corrections to this plan*, item 3. A fourth hook
> point (`~KSession`) is also required; see *Decisions made while implementing
> phase 1*.

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

> **Incomplete as written.** This list is missing three requests the client
> actually sends (two of them mandatory) and includes one,
> `EGS_GET_SERVER_SET_DATA_REQ`, that is never sent at all. See *Packets the plan
> did not predict* and *Corrections to this plan*, item 4, after the exit test.

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

### Exit test — PASSED (2026-08-31)

All five criteria met. `X2_offline.exe` reaches a working character-select screen
with **no server process anywhere and zero TCP connections**, showing the one
hardcoded character (`OfflineTest`, Elsword/Swordman, Lv1) in a 3-slot list next
to a live "Create a new character" slot, with the channel name `Offline-1` in the
header. No error popups, no infinite loading. Verified visually (windowed mode)
and against `offline_packets.log`.

The full chain, 17 request/reply pairs, ~1.1s wall clock from the first channel
packet to the character list:

```
CH  ECH_VERIFY_ACCOUNT_REQ/ACK
CH  ECH_GET_SERVERGROUP_LIST_REQ -> _NOT then _ACK
CH  ECH_GET_CHANNEL_LIST_REQ     -> _NOT then _ACK
GS  EGS_CONNECT_REQ/ACK
CH  ECH_DISCONNECT_REQ/ACK
GS  EGS_VERIFY_ACCOUNT_REQ/ACK   + push ENX_USER_LOGIN_NOT
GS  EGS_CHECK_MACHINE_ID_REQ/ACK
GS  EGS_STATE_CHANGE_SERVER_SELECT_REQ/ACK
GS  EGS_CURRENT_TIME_REQ/ACK
GS  EGS_SELECT_SERVER_SET_REQ/ACK
GS  EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ/ACK   <- the character list
GS  push EGS_KEYBOARD_MAPPING_INFO_NOT, EGS_CHAT_OPTION_INFO_NOT
GS  EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ/ACK
GS  EGS_DISCONNECT_FOR_SERVER_SELECT_REQ/ACK      <- "Go to Channel Selection"
```

Criterion 5 was demonstrated for real rather than argued: before it was
implemented, `EGS_DISCONNECT_FOR_SERVER_SELECT_REQ` logged as
`*** UNHANDLED ***` and the client neither hung nor crashed — it walked back to
the channel list and carried on (see *Packets the plan did not predict* below for
why it still had to be implemented).

Footprint: 7 new files (~700 lines), 105 inserted lines across 5 existing files,
no line deleted anywhere. Encoding of every CP949 file touched verified unchanged
with `file` + `git diff --stat` after each edit.

### Corrections to this plan, found by doing it

Four things in the sections above are wrong for a `US_SERVICE` build. They cost
real time; later phases should not re-learn them.

1. **`KEvent::GetIDStr()` cannot name events, and `X2PacketLog` is dead code.**
   §1.1 says to use `GetIDStr()` because it "is already wired to the
   `ms_szEventID[]` table", and *Critical facts* #7 recommends reusing
   `CX2PacketLog::PrintLog`. Neither works under `_SERVICE_`:
   - `CX2ServerEvent::SERVER_EVENT_ID_STR` collapses to `{ L"" }`
     (`X2ServerEvent.cpp:19-23`), and `CX2ServerProtocol`'s ctor only passes it
     to `KEvent::SetEventID()` when `bIsSERVICE == false`
     (`X2ServerProtocol/Event.h:58`), which is exactly backwards for us.
   - `KEvent::ms_szEventIDList` therefore stays at the 24-entry
     `EventID_System.h` list with `m_EventIDEnd == E_SYSTEM_EVENT_ID_END`, so
     `GetIDStr()` reads **one past the end of that array** for every `EGS_*` /
     `ECH_*` id (`X2ServerProtocol/Event.cpp:115-122`).
   - every `CX2PacketLog::PrintLog` body is wrapped in `#ifdef _IN_HOUSE_`, and
     writes to `dbg::clog`, not to a file. All 60-odd overloads are no-ops here.

   Fix: `X2Lib/Offline/X2OfflineEventName.cpp` rebuilds the full table from
   `EventID_System.h` + `EventID_Client.h` with the same X-macro, under the flag.

2. **`KSession` has a different layout in X2Lib than in X2ServerProtocol — do
   not read any `KActorProxy` / `KUserProxy` member from X2Lib.**
   `X2ServerProtocol/StdAfx.h:32` defines `ADD_COLLECT_CLIENT_INFO_PROTOCOL`
   *before* including `X2ServerProtocolLib.h`; X2Lib reaches the same header via
   `X2Main.h`, where `Session.h` is included **before** `X2ServerProtocol.h:3`
   defines that macro. The macro adds `m_pSADatabase` to `KSession`, so
   `sizeof(KSession)` differs by 4 bytes between the two projects and every
   member of the derived proxy classes reads 4 bytes low from X2Lib.

   Observed concretely: `KActorProxy::GetClassID()` called from X2Lib returned
   `-1`, which is the value of `m_nAckOK` — the `int` declared immediately
   before `m_iClassID`. This is pre-existing, not caused by offline mode
   (`CX2ServerProtocol::UserProxy_SetIntendedDisconnect()` is an inline that
   writes `KUserProxy::m_bIntendedDisconnect` from X2Lib at the wrong offset).
   Base-class members are safe: `KPerformer::QueueingEvent` and everything in
   `KSession` itself sit before the divergence, which is why the seam works.

   Fix: `CX2OfflineServer::KindFromEventID()` classifies a session from its first
   packet's ID namespace (`ECH_` / `ETR_` / `ECS_` / else game) instead.

3. **Do not wake the session with `SetEvent( m_hEvents[EVENT_RECV_COMPLETED] )`**
   as §1.2 suggests. That event's arm in `KSession::Run()` calls
   `m_spSockObj->OnIOCompleted( KOVERLAPPED::IO_RECV )`, and on a socket that was
   never connected `WSAGetOverlappedResult` fails, leaving `dwTransfered == 0` —
   which `KSkSession::OnRecvCompleted` treats as "closed by remote machine" and
   turns into `OnSocketError()`. Shorten the poll instead: `Run()`'s
   `WaitForMultipleObjects` timeout drops from 100ms to 5ms while the hook is
   registered. A full login then costs ~80ms of polling instead of ~1.5s.

4. **`EGS_GET_SERVER_SET_DATA_REQ` is never sent during login.** §1.5 lists it
   as a phase-1 packet; the client only ever names it in the enum. It was not
   implemented and the chain completes without it.

### Packets the plan did not predict

Three requests appear in the login chain that §1.5 does not list. Two of them
are not optional.

| Packet | Why it matters |
|---|---|
| `EGS_CHECK_MACHINE_ID_REQ` | Sent from `Handler_EGS_VERIFY_ACCOUNT_ACK` (`X2State.cpp:10619`). The ACK **must echo `m_strMachineID` back unchanged** — `CX2State::Handler_EGS_CHECK_MACHINE_ID_ACK` compares it to `g_pMain->GetMachineId()` and on a mismatch reports the player as a hacker via `EGS_REPORT_HACK_USER_NOT`. |
| `EGS_KEYBOARD_MAPPING_INFO_WRITE_REQ` | A consequence of pushing an empty `EGS_KEYBOARD_MAPPING_INFO_NOT`: the client falls back to `SetDefaultMap()` and writes those 40 bindings straight back, exactly as it would against a real server on a new account. It waits on the ACK (`X2KeyPad.cpp:720`), so it must be answered. Phase 1 acknowledges and discards; persisting is phase 5. |
| `EGS_DISCONNECT_FOR_SERVER_SELECT_REQ` | "Go to Channel Selection" on the character-select screen (`UnitSelectExit()`). The ACK is the **only** thing that clears `m_bWaiting_EGS_DISCONNECT_FOR_SERVER_SELECT_ACK`, and while that flag is set the `SSSUCM_SERVER_CONNECT0` case returns early — so leaving it unhandled strands the player on the channel list with every channel button inert. |

### What was actually built, against *Code layout to create*

```
X2ServerProtocol/
  OfflineHook.h                 NEW   38 lines   as planned
  Socket/Session.cpp            EDIT  +56        4 hook points, not 3 (see below)

X2Lib/Offline/
  X2OfflineServer.h/.cpp        NEW   179/289    singleton, hook impl, dispatch, session registry
  X2OfflineLog.h/.cpp           NEW    72/176    two log files, flush per line
  X2OfflineEventName.cpp        NEW    78        NOT IN THE PLAN - see correction 1
  Handlers_Login.cpp            NEW   386        11 handlers
```

Three deviations from the planned layout:

- **`X2OfflineEventName.cpp` is new**, and is not optional — without it the
  packet log cannot name anything (correction 1).
- **`X2OfflineSession.h` was not created.** Per-connection state is a nested
  `CX2OfflineServer::KOfflineSession` struct instead; it is 7 fields and only the
  offline server touches it, so a separate header bought nothing. Promote it if
  phase 3+ makes it grow.
- **`Handlers_Stub.cpp` was not needed yet.** The catch-all lives in
  `OnClientSend`'s "consume and log `*** UNHANDLED ***`" path, which is enough
  while the handled set is small. Create the file when there are real per-packet
  stubs to write (phase 7's social systems).

`X2OfflineDB.{h,cpp}` is untouched, as specified — no SQLite in phase 1.

### Decisions made while implementing phase 1

Design calls that are not in the plan and that later phases inherit. All of it
sits behind the single `SERV_IRUHADEV_OFFLINE` flag, per `CLAUDE.md`.

**`IsConnected()` was deliberately left returning `false`.**
The tempting move is to fake it — the proxies are "connected" in every sense the
game cares about. It was rejected because the only way to do it is a new member
on `KSession`, and a flag-guarded member there is exactly the layout split
described in correction 2: `Session.h` is on the `X2ServerProtocolLib.h` include
path, which X2Lib reaches *before* `Always.h` defines the flag, so X2Lib and
X2ServerProtocol would disagree about `sizeof(KSession)` and corrupt each other's
view of every proxy. Every call site was audited instead:

| Site | Consequence of `false` |
|---|---|
| `KSession::Tick` | returns before the heart-beat block — no `E_HEART_BEAT` is ever generated. Strictly better. |
| `ConnectedTo{Game,Channel}Server` | the "already connected" early-out never fires, so the reconnect path works. Required. |
| `CX2StateServerSelect` ctor | skips a re-entry flag reset that is a no-op on a first login (the *game* proxy is not up yet either way). |
| `X2StateLogin.cpp:1018`, `X2StateServerSelect.cpp:7377` | `IsChConnected() \|\| ConnectedToChannelServer(...)` — falls through to the connect, which is what we want. |
| `X2Main.cpp:7361`, `X2State.cpp:8254` | two `SendChID`/`SendChPacket` calls (server time, PC-bang IP/MAC) are skipped. Both are online-only extras. |
| `X2Main.cpp:11376/11400` | skips a `.kom` tamper report. Irrelevant offline. |

Nothing in the login chain needs it, and it keeps every offline edit out of
X2ServerProtocol's headers. **Keep it that way**: put offline changes in
`Session.cpp`, never in `Session.h`.

**`OfflineHook.h` is not included from `X2ServerProtocolLib.h`.** Same reason. It
is included explicitly by `Session.cpp` (where the flag comes from the vcxproj)
and by the X2Lib offline sources (where it comes from `Always.h`, already parsed
by then). The header carries a comment saying so.

**`g_pX2OfflineHook` is defined in `Session.cpp`,** not in a new
`OfflineHook.cpp`, to avoid adding a source file to `X2ServerProtocol`'s ~50
configurations for one pointer.

**A fourth hook point was needed: `~KSession`.** The plan lists three seams
(connect / outbound / inbound). Session *teardown* also has to be observed, or
the session registry leaks entries and — worse — hands out dangling `KSession*`
after the client drops a proxy, which it does routinely
(`DisconnectFromChannelServer` after `ECH_DISCONNECT_ACK`, and
`DisconnectFromGameServer` on backout). `OnSessionClose` is called from the top
of the destructor, before `End()`.

**The offline server is a process-lifetime singleton** that outlives every
`CX2ServerProtocol`. `CX2Data::ResetServerProtocol` calls `Instance()` rather
than constructing per-protocol, because the client tears down and rebuilds its
protocol object several times per session and the account/session state must
survive that.

**`OnClientSend` always returns `true`, handled or not.** An unhandled packet is
logged and dropped on the floor; letting it fall through would reach the socket
path with `m_spSockObj` unconnected. This is what made criterion 5 hold for free.

**Handlers are dispatched inside a `try { } catch( ... )`** that logs and
consumes. One malformed packet must not take the process down; phase 8 asks for
this anyway, and it costs nothing now.

**The packet log defers replies so the file reads causally.** A reply is written
from inside `Reply()`, i.e. before the request line that caused it can be
composed — its `HANDLED` / `*** UNHANDLED ***` tag is only known once the handler
returned. `DeferBegin()` / `DeferEnd()` buffer the replies and emit the request
line above them. Without it every ACK appears one line *before* its REQ, which is
actively misleading when reading a failure.

**Session kind (`GS` / `CH`) is pinned from the session's first packet,** by ID
namespace, because the proxy object cannot be asked (correction 2). Logged once
per session as a `SESSION 0x... is the GS proxy` line.

**The UDP port check is short-circuited by synthesising the ACK the client would
have built for itself** and calling its own `Handler_KXPT_PORT_CHECK_ACK`, using
the same `GetMyIPAddress()` / `GetMyPort()` values as its 10-retry give-up path
already does (`X2StateServerSelect.cpp:~878`). So the offline path takes a route
the client is known to tolerate, instead of inventing one.

**Reply field choices with a reason behind them:**

- `KECH_VERIFY_ACCOUNT_ACK::m_iChannelingCode = -1` — anything else sends
  `CX2StateLogin` into the publisher/channeling branch, where it overwrites the
  server-group ID with `code % 2` and re-picks the channel server by index.
- `m_wstrPassport` must be **non-empty**; the client gates on
  `!kEvent.m_wstrPassport.empty()` before it will advance past the login screen,
  and uses the passport in place of the password from then on.
- `m_wstrCurrentTime` must parse as `YYYY-MM-DD HH:MM:SS` — it is fed to
  `KGCMassFileManager::SetServerCurrentTime`, then `MassFileMapping()` builds a
  `CTime` from it. That is the only format `KncUtil::ConvertStringToCTime`
  accepts, hence the shared `CX2OfflineServer::NowString()`.
- `KAccountInfo::m_wstrOTP` is left **empty**, so the client keeps the passport it
  already has rather than replacing it with an OTP that means nothing offline.
- `KChannelInfo::m_wstrIP` must be non-empty: the client feeds it straight into
  `Handler_EGS_CONNECT_REQ`, which bails out on an empty string. `127.0.0.1` is
  never dialled — `Connect()` is intercepted — it just has to look like an
  address.
- The server group's `m_iServerGroupUID` and `KECH_GET_CHANNEL_LIST_NOT::
  m_iServerGroupID` are **the same value (1)** on purpose, so that clicking the
  single server-group button is a no-op instead of triggering a
  disconnect/reconnect cycle (`SSSUCM_SELECT_SERVER_GROUP` compares them).

**`MakeDefaultUnitInfo` reuses the client's own EXP table** rather than
hardcoding EXP thresholds — `g_pData->GetEXPTable()->GetEXPData( level )`, the
same call `X2StateMenu.cpp:4564` makes. This is the "reuse the client's loaded
managers" decision applied at the first opportunity; phase 2 onward should keep
using this helper as the single place a `KUnitInfo` is constructed.

**The keyboard/chat option pushes go out *after* the character-list ACK,** not on
login. They are the client's first chance to apply defaults, and sending them
earlier risks touching UI (`GetKeyPad()`, `g_pChatBox`) that is not constructed
yet at server-select time.

**Log file conventions:** truncated on every launch (one run, not history),
UTF-8 with BOM so nicknames survive and ordinary `grep` still works, and
`fflush` after every single line — a crash must not lose the last packet,
because that line is usually the cause.

### Two tools worth reusing in later phases

- **Preprocess the real translation unit instead of reading `#ifdef` nesting.**
  `cl /P` with X2Lib's `US_SERVICE` include paths and defines resolves every flag
  question definitively in one shot (`SERVER_GROUP_UI_ADVANCED`,
  `SERV_KOG_OTP_VERIFY`, `SERV_MASSFILE_MAPPING_FUNCTION`, ... are all **on**;
  `_IN_HOUSE_`, `SERV_SERVER_TIME_GET`, `CLIENT_PURPLE_MODULE` are **off**). The
  ~19MB `.i` also gives every packet struct with its conditional members already
  resolved, which is far more reliable than reading `CommonPacket.h` by eye.
- **Byte-level Python patching as the default, not the recovery path.** Every
  edit to an existing file was applied by a script that asserts its byte anchor
  is unique and reuses surrounding CP949 bytes by slicing. This caught two stale
  assumptions (a mis-typed anchor) instead of silently editing the wrong place.

### Build notes that bit

- **`X2ServerProtocol` is not a dependency of `X2_2010.vcxproj`.** Editing
  `Socket/Session.cpp` and rebuilding the client links the *stale*
  `X2Lib\X2ServerProtocol.lib` and fails on `g_pX2OfflineHook` being undefined.
  Build it explicitly first:
  `msbuild X2ServerProtocol/X2ServerProtocol_2010.vcxproj /p:Configuration=US_SERVICE /p:Platform=Win32 /p:SolutionDir=...`
- **`/p:SolutionDir` needs its trailing backslash doubled** inside a quoted
  argument (`"...\Trunk\\"`), or `cmd` escapes the closing quote and msbuild
  reports `MSB6001: ... contains an odd number of double-quote characters` with
  the rest of the command line glued into an include path.

### Operational note, extending phase 0's

`X2_offline.exe` runs at a **higher integrity level than a normal shell**, so it
cannot be driven or stopped programmatically at all: `taskkill /F /T`,
`Stop-Process -Force`, `PostMessage(WM_CLOSE)` and synthetic `Alt+F4` all fail
with `ERROR_ACCESS_DENIED` (5), and `Get-CimInstance Win32_Process` returns an
empty `ExecutablePath` / `CommandLine`. Synthetic mouse clicks do not reach it
either. Every build-run-inspect cycle needs the client closed by hand, and any
UI interaction has to be performed by a person; plan the verification around
reading the two log files, which is where the real signal is anyway.

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

### Exit test — PASSED (2026-09-01)

Every step, plus final delete, which the exit test does not ask for but which the
client offers as soon as a character is soft-deleted. Verified against
`offline_server.log` and by reading `els_db.sql` directly.

```
CREATE   'reben' class=4 -> unitUID=1                     <- create
UNITLIST 2 unit(s) (0 deleted), 3 slots                   <- after a full exe restart: still there
DELETE   'reben' (unitUID=1) pending
RESTORE  'reben' (unitUID=1) is back
SELECT   'ars' (unitUID=5, class=2, lv=1)
ENTER    unitUID=5 leaving character select               <- past character select, no error
DELETE   'ars' (unitUID=5) pending, final delete from 20:12:49
DELETE   'ars' (unitUID=5) erased for good                <- 6s later, no UI round trip
```

The last two lines are the correction-6 fix working: the character was deleted at
20:13:49 and the able-date backdated to 20:12:49, so the final-delete button was
live immediately instead of only after a trip out to channel selection and back.

The save holds the account, the surviving units, and 14 `inventory_size` rows per
unit — every category except `ST_PET`, whose base size is 0 by design — with
category 11 at 6 slots, consistent with `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`.
After a final delete, `SELECT COUNT(*) FROM inventory_size WHERE unit_uid NOT IN
(SELECT unit_uid FROM unit)` is 0, so the child-row cleanup holds.

**Where the run now stops.** `offline_packets.log` ends with exactly one kind of
`*** UNHANDLED ***` line, `EGS_CREATE_TUTORIAL_ROOM_REQ`, fired immediately after
`ENTER`. That is the intended phase boundary, not a defect: the client has
finished character select and is asking for the tutorial room, which is phase 4.
A fresh level-1 character always takes that branch, because
`KEGS_SELECT_UNIT_5_NOT`'s `SERV_ARA_FIRST_SELECT_TUTORIAL` block sets
`IsPlayingTutorial` whenever level is 1 and EXP is 0, and
`Handler_EGS_GET_MY_INVENTORY_ACK` then picks
`Handler_EGS_CREATE_TUTORIAL_ROOM_REQ` over `Handler_EGS_STATE_CHANGE_FIELD_REQ`.
**Phase 3 will not be reachable with a newly created character until phase 4's
tutorial room exists**, or until a character has non-zero EXP. Worth knowing
before phase 3 is debugged against a brand-new save.

### Corrections to this plan, found by doing it

1. **The character-select screen is dead on a fresh login until the server
   pushes `EGS_SECOND_SECURITY_INFO_NOT`.** This is the most important thing in
   phase 2 and the plan does not mention it. `SERV_SECOND_SECURITY` is on;
   `CX2StateServerSelect::m_bEnableUnitSelectBySecondSecurity` starts `false`;
   and while it is false, `SUSUCM_UNIT_BUTTON_UP`, `SUSUCM_CREATE_UNIT`,
   `SUSUCM_SELECT_UNIT` **and** `Handler_EGS_SELECT_UNIT_REQ` each `return true`
   immediately. Every click is a silent no-op: no packet, no error, no dialog,
   nothing in the log to grep for. Only two things clear the flag — this
   unprompted push with `m_bUseSecondPW = false` (what the real server sends for
   an account with no second password), and `X2StateServerSelect.cpp:339`, which
   runs only when the state is *re-entered* with the static
   `m_sbSelectedServerSet` already true.

   That second path is a trap. Creating a character bounces the client back into
   `XS_SERVER_SELECT` and unlocks the screen as a side effect, so the bug is
   invisible in the obvious test (log in, create, click it) and shows up only on
   the *next* launch, where it looks exactly like a persistence failure. It is
   not. **When a click does nothing and the packet log stays silent, suspect a
   client-side gate, not the DB.**

2. **`gup_create_unit` seeds no starting gear.** §2.4's "starting-gear seeding"
   overstates the original: `dbo.gup_create_unit` inserts no items whatsoever.
   It inserts one class skill, four `GDenyOption` rows, quest 13, and spirit.

3. **Its skill seeding is unusable in this build, and must not be copied.** The
   IDs it writes (10000 / 20030 / 30000 / 40010, classes 1-4 only) predate
   `UPGRADE_SKILL_SYSTEM_2013`, which is **on** here — they do not exist in this
   build's `CX2SkillTree::SKILL_ID`, and that snapshot knows nothing of Eve,
   Chung, Ara or Elesis. Seeding from it would write IDs the client cannot
   resolve. `unit_skill` is in the schema and stays empty until the skill tree is
   wired up. Treat everything under `DataBase/` as a *2010-era* snapshot: its
   shapes are still good, its IDs often are not.

4. **Dates are stored as INTEGER epoch, not §2.2's TEXT.** The client's currency
   is `__int64` time_t (`m_trDelAbleDate`, `m_trRestoreAbleDate`, `CTime`,
   `GetServerCurrentTime64`), so TEXT would mean parsing on every read for no
   gain. The soft-delete rule is unchanged: `del_date == reg_date` means alive.

5. **There is no three-day delete guard in the code.** The confirmation text
   (`STR_ID_16102`) says days because that string is baked into the `.kom`
   string table; it gates nothing. The only gate is
   `KUnitInfo::m_trDelAbleDate`, and changing the wording would mean repacking a
   `.kom`, which *Critical facts* #4 rules out.

6. **`m_trDelAbleDate` must be strictly in the past, not "now".**
   `CreateUnitButton()` enables the final-delete button on
   `m_trDelAbleDate < GetServerCurrentTime64()` — a **strict** comparison
   evaluated **once**, when the slots are rebuilt. A zero wait that returns the
   delete instant makes that "now < now" at the one moment it is tested, so the
   button comes up greyed and stays greyed until something else rebuilds the
   buttons; the symptom is that it only lights up after a trip out to channel
   selection and back. `GetServerCurrentTime64()` makes it worse — it is the
   login-time snapshot plus whole seconds of `GetTickCount`, so it can sit a
   second or two behind. Hence `CX2OfflineDB::DelAbleDate()`, which backdates a
   zero wait by `DELETE_CLOCK_SLACK_SECONDS`. Any later phase computing a date
   the client compares against `GetServerCurrentTime64()` needs the same care.

### Packets the plan did not predict

Four, on top of the CRUD set in §2.3. The first three all fire unprompted in the
second after `EGS_SELECT_UNIT_ACK`.

| Packet | Why it matters |
|---|---|
| `EGS_GET_MY_INVENTORY_REQ` | Despite the name it carries no inventory — the items already went out in `_1_NOT`. It is the **last step of character select**: its ACK is what calls `Handler_EGS_STATE_CHANGE_FIELD_REQ` or `Handler_EGS_CREATE_TUTORIAL_ROOM_REQ`. Unanswered, the player is stranded on the screen and every re-click repeats the whole select chain. |
| `EGS_GET_PET_LIST_REQ` | No `AddServerPacket`, so it never times out — but `CX2UIPetInfo` sets `m_bProcessPetList` and refuses to ask again until the ACK clears it. Unanswered, the pet window is wedged shut for the rest of the run. |
| `EGS_GET_RIDING_PET_LIST_REQ` | A hard 60s `AddServerPacket` wait. |
| `EGS_CHECK_BALANCE_REQ` | Sent unconditionally from `Handler_EGS_SELECT_UNIT_ACK`, and waited on. |

### What was actually built, against *Code layout to create*

```
X2Lib/Offline/
  X2OfflineDB.h/.cpp            NEW   193/809   sqlite3 wrapper, schema, migration ladder, queries
  Handlers_Unit.cpp             NEW   533       character CRUD + the five SELECT_UNIT notifications
  Handlers_Stub.cpp             NEW    59       the two pet packets - five phases earlier than planned
  X2OfflineServer.h/.cpp        EDIT  +33/+128  Dispatch moved here, EnsureAccount, MakeUnitInfoFromRow
  Handlers_Login.cpp            EDIT  -101      account onto SQLite; character list moved out
  X2Lib_2010.vcxproj            EDIT   +4       the three new sources
```

Two deviations from the planned layout:

- **`Dispatch` moved out of `Handlers_Login.cpp` into `X2OfflineServer.cpp`.** It
  was there because phase 1 had one handler file. With four, a single switch in
  the owning class keeps each `Handlers_*.cpp` a flat list of handlers.
- **`Handlers_Stub.cpp` arrived in phase 2, not phase 7.** The bar for putting a
  handler there is narrow and worth keeping: a packet belongs in the stub file
  only when *dropping it wedges the client* — it waits on the ACK, or holds an
  in-flight latch only the ACK clears. Everything else can be logged and dropped
  by the catch-all.

`X2OfflineSession.h` still does not exist; per-connection state is still the
nested `KOfflineSession`.

### Decisions made while implementing phase 2

**Text goes through SQLite's UTF-16 API** (`sqlite3_bind_text16` /
`sqlite3_column_text16`), so a `std::wstring` round-trips with no conversion of
ours, while the file itself stays UTF-8 and greppable. §2.1 asks for one
convention held consistently; this is it.

**Accounts are keyed by login ID, not hardcoded.** `EnsureAccount()` creates the
row on first sight of an ID and reuses it forever, so the schema's
`login_id UNIQUE` earns its keep and a different `LoginKey.lua` gets a different
character list instead of silently sharing one.

**Nickname uniqueness is checked among live units only**, which is how
`dbo.gup_delete_unit` behaves (it nulls `GUnitNickName` on delete). The deleted
row keeps its nickname so the character-select screen can still draw the slot.
`Handler_EGS_RESTORE_UNIT_REQ` therefore re-checks before restoring: a live
character may have taken the name in the meantime.

**Nickname validation mirrors `SERV_NICK_NAME_DOUBLE_CHECK`** — ASCII
alphanumerics, 2 to 16 characters — and returns the same `NetError` codes
(`ERR_CREATE_UNIT_04` / `_08` / `_01` / `_05`), because
`CX2StateCreateUnit::Handler_EGS_CREATE_UNIT_ACK` switches on exactly those to
pick its message. The failed ACK echoes the nickname and class back, as the
original does, because the client re-populates its form from them.

**`m_bIsRecommend` in `EGS_GET_MY_INVENTORY_ACK` is `true`.** It means "this
account has already been through the friend-recommendation flow"; false pops a
dialog asking for somebody else's nickname. There is nobody to recommend.

**`KEGS_SELECT_UNIT_4_NOT::m_kGamePlayStatus` is left empty, deliberately.** The
studio's own `ASSERT( !empty() )` says the real server always fills it, and the
live handler guards with `if( !empty() )`, so an empty one is safe. Filling it
means HP/MP/hyper values, which come from the character's stat calculation —
sending a zeroed entry would actively push MaxHP 0 into `CX2GageManager`, which
is worse than not pushing at all (and would make the battlefield branch in
`Handler_EGS_GET_MY_INVENTORY_ACK` take its "dead" path). **This is phase 3's
first job**: the village HUD is where it becomes visible.

**`BaseSlotSize()` mirrors `KInventory::GetBaseSlotSize` with the same
`#ifdef`s**, which resolve identically because `ServerDefine.h` is on X2Lib's
include path. Category 11 lands on 6 through the existing
`SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` arm rather than a second hardcoded 6.

**Migration uses `PRAGMA user_version`,** not a `schema_version` table as §2.1
suggests — it is a header field, so there is no bootstrap problem of migrating
the migration table itself. Schema is v1; add a rung to `Migrate()` rather than
a new `CREATE TABLE` block, so existing saves upgrade instead of being wiped.

### Operational notes, extending phase 0's and phase 1's

- **Windows Defender quarantines every freshly linked `x2.exe`** as
  `Trojan:Win32/Bearfoos.B!ml`, a cloud/ML false positive on the game binary. It
  takes both the build output and the copy in the game dir, then refuses to
  launch either. Folder exclusions on `<Trunk>\X2\US_SERVICE` and the game
  `data\` dir fix it permanently, but `Add-MpPreference` needs an elevated
  shell, so this cannot be self-served from the agent's session. Symptom to
  recognise: msbuild exits 0 but the exe is gone, or `Start-Process` reports
  "the file contains a virus or potentially unwanted software". Check
  `Get-MpThreatDetection` before suspecting the build. Relinking after the
  exclusion is quick — the `.obj`s survive quarantine.
- **Reading `els_db.sql` while the client holds it** works if the whole WAL set
  is copied first (`els_db.sql`, `-wal`, `-shm`) and the copy is opened. Opening
  the live file directly fights the client for the lock.

---

# Phase 3 — Village and field entry

**Goal:** a selected character loads into the village map and can walk around.

### 3.0 Two things phase 2 leaves in the way — clear these first

Both were found by running phase 2 to its exit test. Neither is optional: the
first makes the village unreachable, the second makes it render wrong.

#### 3.0.1 The tutorial gate blocks field entry entirely

`CX2StateServerSelect::Handler_EGS_GET_MY_INVENTORY_ACK`
([:4404](X2Lib/X2StateServerSelect.cpp#L4404)) forks at
[:4435](X2Lib/X2StateServerSelect.cpp#L4435):

```cpp
if ( g_pMain->GetIsPlayingTutorial() == true )
    Handler_EGS_CREATE_TUTORIAL_ROOM_REQ();     // phase 4
else
    ... Handler_EGS_STATE_CHANGE_FIELD_REQ();   // phase 3 - what we want
```

A newly created character **always** takes the tutorial arm, from two
independent sources:

- `KEGS_SELECT_UNIT_5_NOT`'s handler arms it whenever level is 1 and EXP is 0
  ([:3267](X2Lib/X2StateServerSelect.cpp#L3267), `SERV_ARA_FIRST_SELECT_TUTORIAL`,
  which is on at `ServerDefine.h:3491`);
- `CX2StateCreateUnit::Handler_EGS_CREATE_UNIT_ACK` sets it unconditionally at
  [:1185](X2Lib/X2StateCreateUnit.cpp#L1185). The
  `AuthLevel >= XUAL_OPERATOR` bypass sitting next to it — which would have gone
  straight to `XS_VILLAGE_MAP` — is `#ifndef REFORM_TUTORIAL`, and
  `REFORM_TUTORIAL` **is** defined (`KTDXLIB/Always.h:1086`), so it is dead code.
  Raising the account's auth level does nothing.

**Fix: one flag-guarded line**, immediately before the fork at `:4435`:

```cpp
#ifdef SERV_IRUHADEV_OFFLINE
    // Phase 4 owns the tutorial room; until it exists, offline always goes to
    // the field. Delete this when EGS_CREATE_TUTORIAL_ROOM_REQ is implemented.
    g_pMain->SetIsPlayingTutorial( false );
#endif SERV_IRUHADEV_OFFLINE
```

`X2StateServerSelect.cpp` is CP949 — patch it at the byte level and verify with
`file` + `git diff --stat` afterwards, per `CLAUDE.md`.

**This is a scaffold with a debt attached: deleting it is phase 4's first task.**
Until then the tutorial is unreachable, which is the point.

Three alternatives were considered and rejected; do not relitigate them:

| Alternative | Why not |
|---|---|
| Answer `EGS_CREATE_TUTORIAL_ROOM_REQ` with an error | Does not fall back. `Handler_EGS_CREATE_TUTORIAL_ROOM_ACK` ([:6237](X2Lib/X2StateServerSelect.cpp#L6237)) does everything inside `if( IsValidPacket( m_iOK ) )` and otherwise just `return false` — no village, no error path, the player simply sits on character select. |
| Seed new characters with EXP > 0 | Only silences the `_5_NOT` source. The create-unit source still fires, so the tutorial still runs every time a character is made — and a level-1 character carries a fake EXP bar forever. |
| Stop sending `_5_NOT` | Works, and is the wrong kind of clever: `_5_NOT` also carries `m_bCashShopOpen` and the shop-agency info, so it buys silence now and an invisible dead-cash-shop bug in phase 7. |

#### 3.0.2 `m_kGamePlayStatus` has to be filled now

`KEGS_SELECT_UNIT_4_NOT::m_kGamePlayStatus` is deliberately empty as of phase 2
(see phase 2's *Decisions*). The village HUD is where that first shows: without
it `CX2GageManager` never receives HP/MP and the health bar renders empty.

It needs a real max HP, which means the character's stat calculation — do it
properly rather than picking a number. It also matters for correctness one line
below the fork above: the battlefield branch tests
`pGageManager->GetMyGageData()->GetNowHp() > 0.0f` and routes a zero-HP character
to a village-return instead of the battlefield.

**That branch will not fire yet**, and that is already correct: `unit.last_pos`
defaults to 20000 = `SEnum::VMI_RUBEN` (`Enum.h:212`), while battlefields start
at `VMI_BATTLE_FIELD_RUBEN_FIELD_01 = 40000` (`Enum.h:245`). So a phase-2 save
takes the plain `Handler_EGS_STATE_CHANGE_FIELD_REQ()` path. Keep it that way
until battlefields are actually a thing.

### 3.1 The phase proper

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

Two things from phase 2 make that loop work better than it did:

- the log now ends clean, so any `*** UNHANDLED ***` line is new information;
- `Handlers_Stub.cpp` exists, and its bar still applies — a packet belongs there
  only when dropping it wedges the client (it waits on the ACK, or holds a latch
  only the ACK clears). Everything else can be logged and dropped.

### Exit test
Character select → loading → village map renders, the character is controllable,
NPCs are present, no error popups, no infinite loading.

---

# Phase 4 — Rooms, the tutorial dungeon, and dungeon results

**Goal:** the first-run flow completes — new character → tutorial stage →
village → main progression quest triggered.

### 4.0 First: delete phase 3's tutorial scaffold

Phase 3 forces `SetIsPlayingTutorial( false )` just before the fork in
`Handler_EGS_GET_MY_INVENTORY_ACK`
([:4435](X2Lib/X2StateServerSelect.cpp#L4435)), under `SERV_IRUHADEV_OFFLINE`,
because the tutorial room did not exist yet and refusing the request has no
fallback. **The tutorial cannot trigger while that
line is there.** Remove it first, or §4.2 will look broken for a reason that has
nothing to do with §4.2. See phase 3 §3.0.1 for the full reasoning.

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
