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

### Exit test — PASSED (2026-09-01)

All five criteria met. A selected character loads into Ruben, is controllable,
NPCs are present, the health bar is full, and there are no error popups or
infinite loads. Verified visually (windowed) and against both logs.

The village tail, after the phase-2 chain:

```
GS  EGS_STATE_CHANGE_FIELD_REQ/ACK          <- what leaves character select
GS  EGS_OPTION_UPDATE_REQ/ACK               <- CX2StateField ctor
GS  push-only EGS_MODULE_INFO_UPDATE_NOT
GS  EGS_FIELD_LOADING_COMPLETE_REQ/ACK      <- village is up
GS  EGS_UPDATE_PLAY_STATUS_NOT      every ~3s
GS  EGS_FIELD_UNIT_SYNC_DATA_NOT    every ~2s while moving
```

`offline_packets.log` ends with exactly one `*** UNHANDLED ***` kind,
`EGS_BILL_PRODUCT_INFO_REQ`, from opening a billing UI. That is phase 7, it hangs
nothing, and it is the intended phase boundary. A run that only logs in, enters
the village and walks around logs **no unhandled packets at all** - confirmed on
the follow-up run below - so from here on any `*** UNHANDLED ***` line is new
information, which is what makes phase 4's discovery loop cheap.

**The stat work verified itself.** Reading `els_db.sql` after the run gives
`cur_hp = 11250`, which is exactly `StatTable.lua`'s HP for Elsword/Swordman at
level 1 - so the whole round trip closed: `StatTable.lua` ->
`EGS_SELECT_UNIT_4_NOT` -> `CX2GageManager` -> the client's own 3-second push ->
SQLite. `cur_mp = 70`, a real mid-walk value, shows the write is live rather than
one-shot.

**Re-verified after packing (2026-09-01, later run).** With `StatTable.lua`
XOR-encrypted inside `data036.kom` and the loose copy deleted, the log reports
`STAT 'StatTable.lua' loaded: 50 class(es), 4000 row(s)` with no
`is NOT encrypted` note - so it really came out of the archive through the
encrypted path, not off a disk fallback - and `cur_hp` is still `11250`. Encryption
and packing round-trip the same numbers the text parser produced; the change moved
the transport, not the data. That run logged **zero** unhandled packets, and the
`STAT` line appears immediately before `UNITLIST` rather than at startup, which is
the lazy load landing exactly where it has to (after the archive mount, at the
first `KUnitInfo` that needs a stat).

**Where the run now stops:** the portal out of the village. That is phase 4, not
a phase-3 defect - see correction 4.

### Corrections to this plan, found by doing it

1. **§3.0.2 understates the job: `KUnitInfo::m_kStat` / `m_kGameStat` matter more
   than `m_kGamePlayStatus`.** The section only asks for the gage packet, which
   seeds the HUD. But `CX2GUUser::InitStat`
   ([X2GUUser.cpp:2737](X2Lib/X2GUUser.cpp#L2737)) builds the *unit's* max HP from
   `UnitData::m_GameStat` when its base HP is above zero and `m_Stat` otherwise -
   and both were zero. Filling only `m_kGamePlayStatus` would have produced a
   correct-looking HUD on a character whose in-world max HP was still 0.

2. **The client has no per-level stat table, so "reuse the client's loaded
   managers" does not apply here - the first place that decision runs out.**
   `CX2Unit::UnitTemplet` has an `m_Stat` field, but
   `CX2UnitManager::AddUnitTemplet_LUA` never populates it: it is dead on the
   client side. The real server reads base stats out of `dbo.GUnit` and adds
   `KStatTable::GetUnitStat( class, level )`
   ([GSUserFunction.cpp:4490](KncWX2Server/GameServer/GSUserFunction.cpp#L4490)),
   loaded from `StatTable.lua`. So the offline server has to own the table, and
   **`StatTable.lua` becomes a client-side data file**: copied from
   `KncWX2Server/ServerResource/US/`, XOR-encrypted, and packed into
   `data036.kom`, then read through the client's own mass-file loader - so it
   ships inside an archive like every other script rather than sitting loose in
   the game folder. See the decisions below for why it is loaded that way rather
   than parsed as text, and for the encryption step, which the packing tool does
   not do for you.
   (`UnitTemplet::m_UnitType` *is* populated, and that is what `CharAbilTypeOf`
   reuses - the decision still holds for the class/type mapping, just not for the
   numbers.)

3. **`EGS_STATE_CHANGE_FIELD_ACK` must never carry an error.** Every failure path
   in `CX2StateServerSelect::Handler_EGS_STATE_CHANGE_FIELD_ACK`
   ([:6074](X2Lib/X2StateServerSelect.cpp#L6074)) ends in
   `Handler_EGS_STATE_CHANGE_FIELD_REQ( true )`, i.e. an immediate re-send. A
   refusal there is not "the client shows an error", it is an unbounded
   request/reply loop. Contrast `EGS_JOIN_BATTLE_FIELD_ACK`, whose failure path
   just returns false - that one *can* be refused. Check which kind you have
   before returning anything but `NET_OK`.

4. **§3.0.2's "that branch will not fire yet" is right about login and wrong
   about the village.** It reasons from `unit.last_pos` being 20000 while
   battlefields start at 40000, which holds for the login path. The village
   *portal* reaches `EGS_JOIN_BATTLE_FIELD_REQ` by a completely different route:
   in this build the fields outside a village **are** `VMI_BATTLE_FIELD_*` rooms,
   so "walk out of Ruben" is phase 4 §4.1 room emulation in full - a `KRoomInfo`,
   a `KRoomSlotInfo` list, `ConnectRelayServer()` and `XS_BATTLE_FIELD`. Phase 3's
   goal ("village and field entry") therefore overlaps phase 4 more than the plan
   admits, and **phase 3 cannot deliver a walkable field.** The client also gates
   the portal on its own side: it sent the packet once and then stopped sending it
   at all, so the refusal described below is a safety net rather than something
   the player normally sees.

5. **The session FSM from §1.3 is not decoration - without the real server's
   state gate the client bounces itself out of character select.** Within 1-2ms of
   `EGS_STATE_CHANGE_FIELD_ACK` the client intermittently re-sends
   `EGS_SELECT_UNIT_REQ`. Answering it restarts the whole character-select tail
   (five `_NOT`s, the ACK, `EGS_GET_MY_INVENTORY_REQ`) whose ACK calls
   `Handler_EGS_STATE_CHANGE_FIELD_REQ` again - so leaving the village for
   character select would sometimes throw the player straight back to the village
   with the "moving to the village area" dialog. The live client never showed this,
   because `KGSUser` opens that handler with
   `VERIFY_STATE_WARN_REPEAT_FILTER( ( 1, KGSFSM::S_SERVER_SELECT ), ... )`
   ([GSUserGameCommon.cpp:800](KncWX2Server/GameServer/GSUserGameCommon.cpp#L800))
   and a successful select moves the user to `S_FIELD_MAP` immediately
   ([GSUserFunction.cpp:4823](KncWX2Server/GameServer/GSUserFunction.cpp#L4823)).
   **Any handler a later phase adds should be checked against the real one for a
   `VERIFY_STATE*` macro, and mirror it.**

### Packets the plan did not predict

Six - and none of the four things §3.1 actually lists ("field enter/leave, field
user list, NPC list, village channel/instance info") exist as packets at all. The
village, its NPCs and its line map all come out of the client's own `.kom` data,
and the field user list is a server *push* that correctly never arrives. §3.1
also predicted "~20-40 new packets, most of them trivially ACK with OK"; it was
six, and only two of those were derivable without running the game.

| Packet | Why it matters |
|---|---|
| `EGS_OPTION_UPDATE_REQ` | Sent from the `CX2StateField` constructor ([X2StateField.cpp:131](X2Lib/X2StateField.cpp#L131)), so once per village entry, and waited on. It carries the graphics detail level the real server used to size field sectors; nothing to do with it offline, but it must be answered. |
| `EGS_UPDATE_PLAY_STATUS_NOT` | The client's live HP/MP/hyper, pushed every ~3s by `CX2State::CheckAndSendingPlayStatus`. This is the **write half** of `m_kGamePlayStatus` - implementing the read side without it means HP never persists. Note it carries a bare `KGamePlayStatus`, not the `KGamePlayStatusContainer` that `_4_NOT` uses. |
| `EGS_FIELD_UNIT_SYNC_DATA_NOT` | Position sync, every ~2s while moving. The real server rebroadcasts it; offline there is nobody to broadcast to - but it is the **only** packet that reports where the character is standing, so without it `last_line_index` / `last_pos_value` are written once from the spawn point and "log back in where you left off" silently does nothing. |
| `EGS_MODULE_INFO_UPDATE_NOT` | ~3.4KB of loaded-module names, once per session, for spotting injected DLLs. Explicit ignore rather than left to the catch-all, so the packet log stays signal-only. |
| `EGS_JOIN_BATTLE_FIELD_REQ` | The village portal. Phase 4 - see correction 4. |
| `EGS_BILL_PRODUCT_INFO_REQ` | Phase 7. Deliberately left unhandled; it hangs nothing. |

### What was actually built, against *Code layout to create*

```
X2Lib/Offline/
  X2OfflineStatTable.h/.cpp     NEW    97/277   StatTable.lua parser - NOT IN THE PLAN
  Handlers_Field.cpp            NEW   261       6 handlers, not the 1 the plan implies
  X2OfflineDB.h/.cpp            EDIT  +32/+124  schema v1 -> v3, position and gauge writes
  X2OfflineServer.h/.cpp        EDIT  +29/+78   MakeGamePlayStatus, base stats, dispatch
  Handlers_Unit.cpp             EDIT  +58       m_kGamePlayStatus, the FSM gate
  Handlers_Stub.cpp             EDIT  +14       EGS_MODULE_INFO_UPDATE_NOT

X2Lib/X2StateServerSelect.cpp   EDIT  +10       the tutorial scaffold (3.0.1), verbatim
X2Lib/X2Lib_2010.vcxproj        EDIT   +3       the three new sources

StatTable.lua                   NEW            KncWX2Server/ServerResource/US/StatTable.lua,
                                               XOR-encrypted and packed into data036.kom
```

`StatTable.lua` is the one file phase 3 adds outside the source tree. It is a
copy of the GameServer's own resource, XOR-encrypted the same way every other
script is, and packed into `data036.kom`; no loose copy is left behind. A loose
plaintext copy in the game data folder also works and is convenient while
testing - `LoadDataFile` falls back to disk and the loader falls back to
`DoMemoryNotEncript` - but it is not the shipped arrangement, and it will mask a
failed repack, so delete it once the archive carries the file.

Two deviations from the planned layout:

- **`X2OfflineStatTable.{h,cpp}` is new, and is not optional** - without it every
  character has 0 max HP (correction 2).
- **`X2OfflineSession.h` still does not exist.** `KOfflineSession` grew by four
  ints (the gauge write-cache), which is not enough to earn a file.

No line was deleted anywhere. Encoding of `X2StateServerSelect.cpp` (CP949) and
`X2Lib_2010.vcxproj` (UTF-8 BOM + CRLF) verified unchanged with `file` and
`git diff --stat` after each edit.

### Decisions made while implementing phase 3

**`StatTable.lua` is loaded through the client's mass-file loader and run in the
client's Lua state.** This reverses a first attempt that parsed it as text from
the working directory, which was wrong for two reasons:

- **`_ENCRIPT_SCRIPT_` is defined for `_SERVICE_`** ([KTDX.h:86](KTDXLIB/KTDX.h#L86)),
  so every shipped script is XOR-encrypted. Text parsing only ever works on a
  loose, unencrypted copy - which means the offline build would have needed a
  plaintext game-data file forever, sitting outside the archive set the client
  actually ships.
- **`MASS_FILE_FIRST` is defined** ([KTDX.h:92](KTDXLIB/KTDX.h#L92)), and
  `KGCMassFileManager::LoadDataFile` checks the mounted archives first and then
  falls back to a loose file on disk
  ([KGCMassFileManager.cpp:665](KTDXLIB/KGCMassFileManager.cpp#L665)). So **one
  call covers both**, and going through it costs nothing while removing the
  loose-file requirement. It is also what reverses the encryption.

The loader is the three-line pattern from
`CX2UnitManager::OpenScriptFile` ([X2UnitManager.cpp:127](X2Lib/X2UnitManager.cpp#L127)):
`LoadDataFile` then `GetLuaBinder()->DoMemory`. No `luac` step is needed either
way - Lua's own loader takes source or precompiled bytecode.

**The two wrappers come off separately, and the file has to be encrypted.** This
is worth spelling out because getting it wrong fails silently:

- `LoadDataFile` unwraps the `.kom` **container only**.
- `KLuabinder::DoMemory` runs `XORDecrypt` **unconditionally**
  ([KLuabinder.h:25](luaLib/KLuabinder.h#L25)) - it does not sniff the content.
- The encryption is a **separate step on the `.lua` before packing**;
  `X2MassFileTool` does not do it. The cipher is symmetric 4-byte XOR against a
  rotating 3-key schedule (`XOR_KEY0..2`, [KTDX.h:388](KTDXLIB/KTDX.h#L388)), so
  `XOREncrypt` is literally `XORDecrypt`
  ([KTDXCommonFunc.h:715](KTDXLIB/KTDXCommonFunc.h#L715)), and `FileEncrypt()`
  right below it is the studio's own file-level helper.

So a plaintext file handed to `DoMemory` is XOR'd into garbage and simply fails to
load. Rather than make that a footgun, the loader **tries `DoMemory` first and
falls back to `DoMemoryNotEncript`**, logging a `STAT NOTE ... is NOT encrypted`
line when the fallback wins. Encrypted stays the shipped path; a forgotten
encryption step degrades to a visible warning instead of a silent drop to the
synthetic curve.

**The object is bound into Lua as `StatTable`, and the stat table is read off the
stack.** The chunk calls `StatTable:ReserveMemory(...)` and
`StatTable:SetUnitStat( class, level, { ... } )` on a global, so
`class_add` / `class_def` / `decl` register this object under that name exactly as
`KStatTable::RegisterLuaBind` does
([StatTable.cpp:131](KncWX2Server/GameServer/StatTable.cpp#L131)). The trailing
table is **not** a declared parameter: `KLuaManager`'s default table depth of 1
makes `LUA_GET_VALUE` read fields out of the table left on the stack, which is
the same convention `KStatTable::SetUnitStat` and
`CX2UnitManager::AddUnitTemplet_LUA` both use. `ReserveMemory` is a no-op here
(this side keeps a map, not a pre-sized vector) but must still be bound, or the
chunk errors on its first line.

**The table loads on first use, not at startup.** `CX2OfflineServer::Instance()`
runs around `CX2Data::ResetServerProtocol`, and the 145 `.kom` archives are not
necessarily mounted by then - `LoadDataFile` would just return NULL. Reading a
loose file from the working directory did not care about ordering; reading the
archive does. One attempt per process either way, since a retry would re-run the
chunk on every character.

**HP is truncated, not rounded.** The server reads these through `LUA_GET_VALUE`
into `UINT`/`USHORT`, so `HP = 12937.5` becomes 12937. Rounding would give an
offline character one more hit point than the same character online.

**`MAX_LEVEL` is 80, matching the server's own `ReserveMemory( class, 80 )`.**
Classes 110 and 111 carry rows up to level 99 in the US file; those are dropped
and lookups clamp to 80, which is what the server does too
(`unitLevel < mit->second.size()`).

**A missing `StatTable.lua` logs loudly and falls back to a synthetic curve**
(Elsword/Swordman's own, fitted from its level 1 and 17 rows) rather than
returning zeroes. A wrong-but-playable character with a shouting log beats an
unplayable one with a silent log.

**`m_kGameStat` is set equal to `m_kStat`.** On the real server it is base plus
equipped items plus sockets; offline there is no gear until phase 5, so the two
coincide. **When gear lands, `m_kGameStat` is the one that grows.**

**`cur_hp = 0` means "never stored", and is restored as full health.** It is also
what a character who died would carry, and either way it must not be handed back
as zero: `Handler_EGS_GET_MY_INVENTORY_ACK` routes a zero-HP character out of a
battlefield, and there is nowhere in a village to heal.

**The gauge write is change-detected, cached on the session.** The push arrives
every three seconds for the whole run; writing unconditionally would be ~1200
pointless `UPDATE`s an hour. The cache lives on `KOfflineSession`, so a reconnect
costs one redundant write - cheaper than tracking it globally and getting it wrong
when the character changes.

**The portal refusal uses `ERR_BATTLEFIELD_00`** ("not in a state that can move to
that area") because it is both true and *not* one of the two codes
`Handler_EGS_JOIN_BATTLE_FIELD_ACK` special-cases - `ERR_BATTLEFIELD_13` / `_14`
print a required level or dungeon, which would be a lie. So it falls through to
`CX2Main::IsValidPacket`'s default branch: one OK dialog, and the player stays in
the village instead of hanging on an unanswered `AddServerPacket`.

**The FSM gate replies `ERR_WRONG_STATE_00` rather than dropping the packet.** That
is what the house macro sends, and the code is deliberately in
`CX2Main::IsValidPacket`'s silent-false set
([X2Main.cpp:6936](X2Lib/X2Main.cpp#L6936)) so the client swallows it without a
dialog. Dropping instead would leave the client's wait to time out into a network
error.

**Only `EGS_SELECT_UNIT_REQ` is gated, not `EGS_GET_MY_INVENTORY_REQ`,** even
though the real server gates both (`VERIFY_STATE( ( 1, KGSFSM::S_FIELD_MAP ) )`).
The risk is asymmetric: a wrong gate on select means a click does nothing and is
visible immediately, while a wrong gate on `GET_MY_INVENTORY` strands the player
on character select permanently. Fix the cause, do not add a second gate that can
misfire. `EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ` also resets the state to
`S_SERVER_SELECT` as a belt-and-braces net, since only `CX2StateServerSelect` ever
asks for the character list.

**Migration stays a ladder, and the new rungs apply to fresh files too.**
`SCHEMA_V1` is left exactly as phase 2 wrote it; v2 (last field position) and v3
(live gauges) are additive `ALTER TABLE`s that run on a brand-new save as well as
on an upgrade, so the ladder stays the single source of truth. Both rungs were
dry-run against a copy of the real phase-2 save before shipping.

### Operational notes, extending the earlier phases'

- **"Log back in where you left off" is not observable until a character has
  EXP > 0.** `CX2StateServerSelect::Handler_EGS_STATE_CHANGE_FIELD_REQ`
  ([:5954](X2Lib/X2StateServerSelect.cpp#L5954)) forces `VMI_RUBEN` start
  position 1 whenever EXP <= 0. Both halves of the write path run and the values
  persist; only the *read* is gated. Do not debug it as broken before phase 4
  gives a character some EXP.
- **HP and MP likewise cannot change inside a village** - nothing there damages
  you. Verify both by copying the whole WAL set and reading `els_db.sql`, not by
  looking at the screen.
- **Verify the stat table from the log, not by eye.** `offline_server.log` prints
  `STAT 'StatTable.lua' loaded: <n> class(es), <n> row(s)` on the first character
  that needs a stat - 50 classes and 4000 rows is a correct US file. Anything else,
  including the synthetic-curve fallback, says so on its own line. Because the
  load is lazy, that line appears at first character select rather than at startup.
- **The Bash tool's `cat` re-renders indentation.** An anchor copied out of `cat`
  output will not match the file for a byte-level patch - a line that is really a
  tab followed by `static void Foo(` displays tab-aligned instead. Take anchors
  from `cat -A` or from `python -c "print(repr(...))"`. This cost two failed patch
  runs.

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

Phase 3 also refuses `EGS_JOIN_BATTLE_FIELD_REQ` with `ERR_BATTLEFIELD_00`
(`Handlers_Field.cpp`). Remove that too - and note that **the village portal is
the first thing §4.1 should make work**, because in this build the fields outside a
village are `VMI_BATTLE_FIELD_*` rooms, so reaching them is room emulation and not
a separate system. See phase 3 correction 4.

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

### Exit test — PARTIALLY PASSED (2026-09-02)

| Criterion | Result |
|---|---|
| Fresh character → tutorial dungeon | **pass** |
| Tutorial dungeon runs (cutscene, monsters, controllable) | **pass** |
| Normal dungeon reachable and playable | **pass** (not asked for, but it is what makes the phase useful) |
| EXP / ED awarded and persisted | **pass** — `unit_uid=3` reached `exp=252, ed=588` across runs |
| Returns to village | **pass** |
| Items awarded | **not done** — deliberately deferred, see decision 6 |
| Main progression quest offered | **not verified** — quests are phase 6; phase 4 reports exactly one completed quest and nothing else, see decision 2 |

So the flow works end to end and the numbers persist, but two of the written
criteria are outstanding, both because they belong to later phases. Recorded as
partial rather than passed.

Field ("battlefield") play came along with it, which the plan had put in no phase
at all: monsters spawn, respawn, and escalate to a middle boss.

### Corrections to this plan, found by doing it

1. **§4.3 is wrong about where EXP and ED come from.** "Accept the client's
   reported EXP, ED, drops and clear rank" — the client reports no such thing.
   `KEGS_NPC_UNIT_DIE_REQ` carries the dead NPC's UID, who killed it and how;
   `KEGS_MY_USER_UNIT_INFO_TO_SERVER_REQ` carries combat *scores*, not rewards.
   EXP and ED were computed server-side per kill out of two tables and pushed back
   down in `EGS_NPC_UNIT_DIE_NOT`'s `m_EXPList`, which is what
   `CX2Game::ProcessExpListByNpcUnitDie` adds to the unit. So the offline server
   has to own those tables — hence `CX2OfflineDropTable`, the second instance of
   phase 3's "the client has no copy of this data" problem.

2. **§4.1's packet list is mostly the multiplayer list.** `EGS_JOIN_ROOM_REQ/ACK`,
   `EGS_JOIN_ROOM_NOT`, `EGS_LEAVE_ROOM_NOT` and `EGS_CHANGE_TEAM_REQ/ACK` never
   fire for a solo player — there is nobody to join, be told about, or swap teams
   with. What actually runs is a different set entirely; see *Packets the plan did
   not predict*.

3. **"and the game-start packet" is singular and it is a chain of ten.** Starting a
   dungeon is `EGS_STATE_CHANGE_GAME_START_REQ/ACK` → `_NOT` → four
   `EGS_GAME_LOADING_REQ` rounds → `EGS_GAME_LOADING_ALL_UNIT_OK_NOT` →
   `EGS_PLAY_START_NOT` → the per-stage chain (`STAGE_LOAD`,
   `STAGE_LOAD_COMPLETE`, `STAGE_START`, `SUB_STAGE_OPEN`, `SUB_STAGE_GO_NEXT`,
   `SUB_STAGE_LOAD_COMPLETE`, `SUB_STAGE_START`). Miss any one `_NOT` and the
   dungeon renders but never starts.

4. **The plan's largest single miss: `CX2Game::m_AllowFrameCount`.** No packet in
   any phase list refills it, and without a refill the client **stops reading the
   keyboard about seven seconds into every dungeon** — silently, with no popup, no
   log line and no packet. `CX2Game::KeyProcess` gates all input gathering on
   `GetEnableAllKeyProcess() && m_AllowFrameCount > 0`; the counter is seeded to
   `(int)( 85 * 5.3 )` = 450 and decremented once per `OnFrameMove`
   ([X2Game.cpp:2741](X2Lib/X2Game.cpp#L2741)). It is a speed-hack detector, and
   `EGS_REMAINING_PLAY_TIME_NOT` is its only refill — the real server broadcasts
   one every `ms_fPlayTimeNotifyGap` = 5.0 s against that 5.3 s allowance
   ([Room.cpp:551](KncWX2Server/CenterServer/Room.cpp#L551)).

   This cost three wrong fixes before the cause was found, because everything
   about it looks like something else: the character stands in `USI_WAIT` while
   NPCs animate, the UI responds, and the client's own three-second status push
   keeps arriving. It also explains an asymmetry that looked like two separate
   bugs — the switch at [X2Game.cpp:2733](X2Lib/X2Game.cpp#L2733) exempts
   `XS_TRAINING_GAME` and `XS_BATTLE_FIELD` from the decrement, which is why
   fields became freely walkable while dungeons kept freezing.

   **Generalise it:** the server's *periodic pushes* are load-bearing, not
   telemetry. A phase that answers every request correctly can still be unplayable.

5. **§4.1's "in-match traffic is P2P UDP and never reaches the offline server at
   all, which is why trust the client costs nothing" is half wrong, in both
   directions.** Monster creation and death (`EGS_NPC_UNIT_CREATE_REQ`,
   `EGS_NPC_UNIT_DIE_REQ`), the world triggers, the stage chain and the results
   all come over TCP to the server, and the server *owns* NPC UIDs — the client
   sends `-1` and reads the real UID back out of the broadcast. Meanwhile the
   P2P half is not free either: the client must be registered as its **own** peer
   or `ConnectTestResult()` returns false on an empty peer map,
   `GameLoadingReq(100)` never fires, and loading never completes. Self-as-peer is
   the studio's own pattern (`X2SquareGame.cpp:353` does it for the village).

6. **A `_NOT` broadcast is frequently the only thing that performs an action —
   even for the host.** Hit three times in this phase: `CX2Game::CreateNPCReq`
   only sends, and the sole live `CreateNPC` call is in
   `Handler_EGS_NPC_UNIT_CREATE_NOT` ([X2Game.cpp:6988](X2Lib/X2Game.cpp#L6988));
   `Handler_EGS_WORLD_TRIGGER_RELOCATION_NOT`
   ([:14813](X2Lib/X2Game.cpp#L14813)) holds the only live
   `CX2World::ActiveTrigger()` call, the local one being
   `#if defined(WORLD_TOOL) || defined(X2TOOL)`. **Never assume the host already
   did it locally** — grep for the local call and check what it is `#ifdef`'d
   behind.

7. **The static stage monster list has exactly one source, and the client's own
   parser for it is dead code.** `SubStageData::m_NPCDataList` is filled only by
   `CX2Dungeon::SetStageStaticNPC` from `EGS_DUNGEON_STAGE_LOAD_NOT::m_mapNPCData`.
   The client *has* the dungeon script and a `LoadNPCData` to parse it, but the
   call site is `#ifdef X2TOOL`. Phase 4 revives it behind the flag (gated on
   `bIsNpcLoad`, so only the throwaway read-only dungeon parses monsters and the
   live one cannot double-spawn) and rebuilds the packet from it.

8. **§4.2 looks in the wrong place for the tutorial gate.** Not `TutorSystem.lua`
   and not the unit data: it is `CX2Main::SetIsPlayingTutorial`, and the room
   request is sent by `CX2StateBeginning` / `CX2StateField`, not by
   `CX2StateServerSelect`. Phase 3's scaffold suppressed exactly that flag.

9. **The dungeon *menu* is not in this plan at all, and nothing else in phase 4
   matters without it.** The village's party dialog holds the dungeon button, the
   local map, and every route into a dungeon, and it hides itself outright unless
   `CX2PlayGuide::GetShowDungeonMenu()` is true
   ([X2PartyUI.cpp:1319](X2Lib/X2PartyUI.cpp#L1319)). That flag has two sources,
   both in `CX2QuestManager::SetUnitQuest`
   ([X2QuestManager.cpp:596](X2Lib/X2QuestManager.cpp#L596)), and both read the
   completed-quest vector of `EGS_SELECT_UNIT_2_NOT` — which phase 2 sends empty
   and labels "phase 6". So an empty quest list does not merely hide quests, it
   removes dungeons.

10. **Solo dungeon entry needs no party subsystem.** An earlier reading of this
    (recorded here because it was wrong for two rounds) said dungeon entry runs
    through `CX2PartyManager` and therefore needed the party packets. It does not:
    `CX2PartyUI::GameStartCurrentMember` takes its branch when
    `DoIHaveParty()` is *false* just as readily
    ([X2PartyUI.cpp:4984](X2Lib/X2PartyUI.cpp#L4984)), and picking a dungeon on the
    local map with no party sends nothing at all — `CX2LocalMapUI` writes
    `GetMyPartyData()->m_iDungeonID` locally. One packet does the whole job:
    `EGS_QUICK_START_DUNGEON_GAME_REQ` → ACK → `EGS_PARTY_GAME_START_NOT`, whose
    handler builds the room and sends `XGM_STATE_CHANGE -> XS_DUNGEON_GAME`
    directly, with no room screen in between.

11. **Leaving a dungeon needs two packets, not one.**
    `CX2StateDungeonGame::OnFrameMove`
    ([X2StateDungeonGame.cpp:894](X2Lib/X2StateDungeonGame.cpp#L894)) acts only
    when `m_bReceive_KEGS_LEAVE_ROOM_ACK` **and**
    `m_bReceive_KEGS_UNIT_INFO_UPDATE` are both set, and only
    `EGS_UPDATE_UNIT_INFO_NOT` sets the second. With just the ACK, pressing Leave
    does nothing — no error, no transition. The tutorial hides this: its ESC
    dialog branches on `GetIsPlayingTutorial()` and only the tutorial arm sets
    `m_bLeaveRoomAtTutorial`, which is the alternative to the second flag.

12. **Field monster tables *are* shipped with the client — the note in phase 3
    correction 4 and my own first two readings of this were wrong.** The claim was
    that `CXSLBattleField`'s `NPC_TABLE_FILE_NAME` data never reached players and
    would have to be recreated. It is all in `data036.kom`: 24 per-field scripts
    (`Ruben_Field_00.lua`, …), and decrypting them turns up `AddSpawnMonsterGroup`
    in 24 and `AddLine` in none, so they are the monster tables rather than the
    line maps. Only `BattleFieldServerData.lua` — the tuning constants — is
    genuinely absent, because it is server tuning and was never shipped.

13. **"Boss groups" in this build means the *middle* boss.**
    `CXSLBattleField::GetBattieFieldBossMonsterInfo` is entirely inside an `#else`
    under `SERV_BATTLEFIELD_MIDDLE_BOSS`, which this build defines — the shipping
    game replaced the full boss with the middle boss. Consistent with the client's
    `BattleFieldData.lua` carrying `BATTLE_FIELD_MIDDLE_BOSS_INFO` and no
    `BATTLE_FIELD_RISK_INFO` at all.

### Packets the plan did not predict

Room entry, all three routes into one single-occupant room:

```
EGS_CREATE_TUTORIAL_ROOM_REQ/ACK      the tutorial
EGS_QUICK_START_DUNGEON_GAME_REQ/ACK  every normal dungeon, solo
EGS_PARTY_GAME_START_NOT              what actually moves the player
EGS_JOIN_BATTLE_FIELD_REQ/ACK         the village portal out to a field
EGS_CREATE_ROOM_REQ/ACK               PvP lobby only in this build; implemented anyway
```

The dungeon run:

```
EGS_REMAINING_PLAY_TIME_NOT           every <=5s or input dies - correction 4
EGS_DUNGEON_STAGE_LOAD_REQ/ACK + _NOT          _NOT carries the static monsters
EGS_DUNGEON_STAGE_LOAD_COMPLETE_REQ/ACK + _NOT
EGS_DUNGEON_STAGE_START_NOT
EGS_DUNGEON_SUB_STAGE_OPEN_REQ/ACK + _NOT
EGS_DUNGEON_SUB_STAGE_GO_NEXT_REQ/ACK + _ALL_NOT
EGS_DUNGEON_SUB_STAGE_LOAD_COMPLETE_REQ/ACK
EGS_DUNGEON_SUB_STAGE_START_NOT
EGS_DUNGEON_SUB_STAGE_CLEAR_REQ/ACK
EGS_DUNGEON_KILLALLNPC_CHECK_REQ/ACK  m_bResult MUST be true - see decision 4
EGS_NPC_UNIT_CREATE_REQ/ACK + _NOT    server owns the UIDs
EGS_NPC_UNIT_DIE_REQ/ACK + _NOT       where EXP and ED are actually earned
EGS_WORLD_TRIGGER_RELOCATION_REQ/ACK + _NOT
EGS_USER_UNIT_DIE_REQ/ACK, EGS_USER_UNIT_DIE_COMPLETE_REQ
EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ/ACK, EGS_STOP_DUNGEON_CONTINUE_TIME_REQ/ACK
EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ/ACK
EGS_GET_ITEM_REQ/ACK
EGS_CHECK_MACHINE_ID_REQ/ACK
```

Ending it:

```
EGS_END_GAME_REQ/ACK
EGS_END_GAME_DUNGEON_RESULT_DATA_NOT
EGS_UPDATE_UNIT_INFO_NOT              also required to LEAVE - correction 11
EGS_END_GAME_NOT
EGS_STATE_CHANGE_RESULT_REQ/ACK, EGS_RESULT_SUCCESS_REQ/ACK
EGS_LEAVE_GAME_REQ/ACK, EGS_LEAVE_ROOM_REQ/ACK
EGS_START_REWARD_BOX_SELECT_REQ/ACK, EGS_SELECT_REWARD_BOX_REQ/ACK
```

Fields:

```
EGS_BATTLE_FIELD_NPC_LOAD_NOT          the field's opening population
EGS_NPC_UNIT_CREATE_NOT                respawn reuses the dungeon packet
EGS_NPC_UNIT_CREATE_MIDDLE_BOSS_NOT    the middle boss
EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT   the only report of position in a field
```

Answered as explicit ignores so the packet log stays signal-only:
`EGS_MODULE_INFO_UPDATE_NOT`, `EGS_DUNGEON_PLAY_INFO_TO_SERVER_NOT`,
`EGS_FRAME_AVERAGE_REQ`, `EGS_CLIENT_QUIT_REQ`,
`EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT`.

Compiled out of this build and therefore unreachable, checked rather than assumed:
`EGS_CHANGE_DUNGEON_GET_ITEM_TYPE_REQ` (`NOT_USE_DICE_ROLL`), and
`EGS_SKILL_USE_REQ`'s wait (`AddServerPacket` commented out at its only sender).

### What was actually built, against *Code layout to create*

New, beyond the plan's list:

```
X2Lib/Offline/Handlers_Room.cpp          rooms, the dungeon run, results, fields
X2Lib/Offline/X2OfflineDropTable.h/.cpp  NpcExpTable.lua + DropTable.lua
X2Lib/Offline/X2OfflineBattleField.h/.cpp  BattleFieldData.lua + <Field>.lua
                                           + BattleFieldServerData.lua
```

Client edits, all behind `SERV_IRUHADEV_OFFLINE`:

```
X2Lib/X2StateServerSelect.cpp   phase 3's tutorial scaffold removed (§4.0)
X2Lib/X2DungeonSubStage.cpp     SubStageData::LoadNPCData revived - correction 7
```

Data the user packs into `data036.kom`, XOR-encrypted:

```
NpcExpTable.lua              per-level NPC EXP
DropTable.lua                per-monster EXP/ED gates
BattleFieldServerData.lua    field tuning; the only one not already shipped
```

`unit_dungeon` gained rows through `AddDungeonClear`; no schema bump was needed —
phase 2's v3 already had the table.

### Decisions made while implementing phase 4

1. **The offline server owns NPC UIDs**, exactly as `KRoomMonsterManager` does
   (counter from 1, monotonic for the life of the room). The client sends `-1`.
   Getting this wrong the first time keyed every reward under `-1`.

2. **One completed quest is reported: 11005 (`TQI_CHASE_THIEF`).** It is the
   narrowest lever that turns the dungeon menu on (correction 9) and it is the
   studio's own unlock. 11030 would additionally switch the novice guide off,
   which is not phase 4's call. This is an unlock, not a quest system — quest
   state stays phase 6, and the code says so.

3. **Sub-stage clear falls back to `CT_GAME` with `NET_OK`** when the client's
   `m_vecNextStage` cannot be read, rather than the server's `ERR_ROOM_51`. An
   error there hangs the dungeon with no way out; continuing is recoverable.

4. **`EGS_DUNGEON_KILLALLNPC_CHECK_ACK.m_bResult` is always true.** False makes
   the client report *itself* as a hacker
   ([X2StateDungeonGame.cpp:3530](X2Lib/X2StateDungeonGame.cpp#L3530)).

5. **The 30% clear bonus is applied; the rank bonus is not.**
   `ResultProcess.cpp:2054` is the source for the former. The rank tables are
   server data with no client copy, and inventing multipliers would be worse than
   omitting them visibly.

6. **No item drops.** `KDropTable::NormalNpcDropItem`'s item lottery is
   deliberately not ported: an item has to land in an inventory, and the inventory
   does not round-trip through SQLite until phase 5. Dropping items that a relog
   would silently eat is worse than dropping none.

7. **Resurrection is refused** (`ERR_RESURRECT_00`) — no stones until phase 5.

8. **`m_bGameEnd` is false when leaving a dungeon mid-run**, true only on a real
   end. True would make `OnFrameMove` fire `StateChangeResultReq()` and race the
   village transition with a results screen for an abandoned run.

9. **The dungeon-leave fix is gated on `GetIsExitingTutorial()`** so the tutorial
   keeps the branch it already leaves correctly through, rather than being
   silently moved onto the other one.

10. **Two separate drop-table maps, not one.** Merging `AddMonsterDropInfo` and
    `AddBattleFieldNpcDropInfo` under one key looked right — real IDs never
    collide, dungeons being 30000+ and battlefields 40000+ — but *both* use 0 as
    their wildcard, and six monsters (393, 662, 665, 1115, 3003, 3005) appear
    under both with different rewards. The battlefield rows, all `Exp = 0`, were
    overwriting the dungeon rows and costing those monsters their EXP.

11. **`Exp` in `DropTable.lua` is a gate, not an amount.** Any value above zero
    means "this monster grants EXP", and the amount comes from
    `NpcExpTable.lua` keyed by the monster's level. `ED` is a per-level
    multiplier gated by an `EDProperty` percentage roll. Arithmetic is
    `KDropTable::NormalNpcDropItem`'s verbatim.

12. **Field position and village position share one pair of columns**, because the
    server keeps one `m_kLastPos` and the client's login path reads that single
    slot — re-joining a field when the stored map is a `VMI_BATTLE_FIELD_*` and
    placing the player in a village otherwise
    ([X2StateServerSelect.cpp:4444](X2Lib/X2StateServerSelect.cpp#L4444)). The two
    encodings of `m_usLastPosValue` differ (village `floatToHalf(ratio)`, field
    `sqrtf(dist) * 100`) and that is safe precisely because `last_pos` says which
    map the row belongs to.

13. **Field respawn and the middle boss ride the client's own pushes**, because
    the offline server has no timer — it only runs when a packet arrives. Respawn
    and the boss use `EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT`; the frame-count
    refill uses the three-second `EGS_UPDATE_PLAY_STATUS_NOT`. The field's
    limitation is stated rather than hidden: stand perfectly still and the field
    stops refilling.

14. **The field's opening population is sent on the first
    `EGS_UPDATE_BATTLE_FIELD_USER_POS_NOT`, not on the join ACK.** The client
    builds that push from `GetMyUnit()->GetLastTouchLineIndex()`, so its arrival
    *proves* the unit exists — which is the exact condition the client's NPC-load
    handler branches on. Sending from the ACK races the state change.

15. **Boss groups are kept by ID but excluded from respawn.** A middle boss must
    never come back on a timer, only by earning the danger value again.

16. **Middle-boss pacing is tuned in the Lua, not in code.**
    `SetDangerousValueEventRate` 4 → 32 puts the first boss at 25 kills instead of
    200 and scales all five tiers uniformly, leaving
    `GET_MIDDLE_BOSS_MONSTER_DROP_RATE`'s curve untouched. The original value and
    the restore instruction are comments in the file, so re-tuning needs no
    rebuild. Tuning in the data keeps the emulator faithful and the knob visible.

17. **Elite / attribute-enchanted monsters are omitted**, and `m_mapAttribNpcInfo`
    goes out empty. Their stat rolls come from `AttribNpcTable.lua`, also
    server-only; the client reads an empty map as "ordinary monster".

### Operational notes, extending the earlier phases'

- **There are two `KLuaManager` classes in this tree with the same method names
  and different return types.** The server's
  (`KncWX2Server/Common/Lua/KLuaManager.h`) returns `HRESULT`; the client's
  (`luaLib/KLuaManager.h`) returns `bool`. Since `S_OK` is `0`, porting the
  server's `== S_OK` idiom into client code **inverts every test** — and it
  compiles, because `bool` converts to `int`. An inverted `if` skips a block; an
  inverted `while( S_OK == BeginTable( i ) )` spins forever. That is exactly what
  it did: the field's BGM kept playing while the game stopped responding, the hang
  being on the main thread inside a Lua chunk. **When porting a server parser into
  `X2Lib`, convert every `== S_OK` to `== true` and every `== E_FAIL` to
  `== false`.**
- **A `.kom` archive can be read from the outside, and it is the fastest way to
  answer "is this data shipped?".** The file carries a plaintext XML manifest —
  `grep -a '<File Name="' data036.kom` lists every member with its sizes. To see
  *contents*, scan for zlib streams (`0x78` followed by `0x01/0x5E/0x9C/0xDA`),
  decompress, then XOR-decrypt with the three rotating keys at
  [KTDX.h:388](KTDXLIB/KTDX.h#L388). Members are compiled Lua (`\x1bLuaQ`), so
  grep the string constants rather than expecting source. This turned "we would
  have to recreate the field monster tables by hand" into "all 24 are already
  here" in about ten minutes, and it should be the *first* move next time a phase
  claims data is missing.
- **Client-side state can usually be inspected from the offline server, without
  touching a CP949 file.** Chasing the input freeze needed
  `m_bCanNotInputAndPauseNPCAI`, which has no getter — but the same call that sets
  it also calls `SetAIEnable(false)` on every NPC, and `GetAIEnabled()` is public,
  so "every NPC has AI off" was a faithful proxy read entirely from
  `Handlers_Field.cpp`. Reach for a proxy through existing public accessors before
  byte-patching a Korean-encoded header. When a getter really is unavoidable, add
  it behind the flag, keep it to one line, and delete it afterwards — three such
  getters were added and reverted with `git checkout --` once the cause was found,
  which is only safe because those files contained nothing else.
- **`EGS_UPDATE_PLAY_STATUS_NOT` is the emulator's clock.** It arrives every
  ~3.01 s in villages, dungeons and fields alike, and it is the only fixed cadence
  a run has. Anything periodic should ride it rather than invent a timer.
- **A modal message box is what a server-packet timeout looks like.**
  `CX2State::UIServerTimeOutProc` pops `KTDGUIOKMsgBox` after
  `m_fServerTimeOut` = 5.0 s. Worth knowing, but check the packet log before
  blaming it: in the one case it was suspected, every REQ had in fact been
  answered and the real cause was correction 4.
- **Cross-check waits mechanically, not by reading.**
  `grep -rho 'AddServerPacket( *[A-Z_0-9]*' X2Lib/*.cpp` gives all 333 armed
  waits; diffing that against the ACKs the offline server sends is a few seconds
  and finds the hangs a phase will otherwise discover one play-test at a time.

### Rule: server Lua that the client needs is the user's to pack

Three phases have now hit the same shape — the client is missing a data file that
only ever existed server-side — and the handling is settled:

1. **Say what is needed and stop.** Name the exact file
   (`KncWX2Server/ServerResource/US/<name>.lua`), say it must be XOR-encrypted and
   packed into `data036.kom`, and let the user do it. Do not write loose copies
   into the game directory: `MASS_FILE_FIRST` means `LoadDataFile` falls back to a
   loose file, which works and then **masks a failed repack**, so the next person
   cannot tell whether the archive is right.
2. **Load it the shipped way** — `LoadDataFile()` for the container, `DoMemory()`
   for the XOR, with a `DoMemoryNotEncript()` fallback that logs a note when it
   fires. Same shape as `X2OfflineStatTable`, `X2OfflineDropTable` and
   `X2OfflineBattleField`.
3. **No band-aid logic.** Do not hardcode a table, invent a curve, or approximate
   a function to paper over a file that is not packed yet. If the data is absent,
   the feature is off and the log says which file is missing and what to do about
   it — the way the middle boss does, since its spawn chance is a Lua *function*
   and no constant can stand in for it. The single exception is a value that can
   be read out of the repo and cited as such: `BattleFieldServerData.lua`'s
   respawn window and factors are carried as fallbacks with a comment naming their
   source, so an unpacked install behaves like the live server instead of like
   nothing. A fallback is honest only when it is the real number, is labelled, and
   is announced in the log.

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

### Exit test — PASSED (2026-09-03)

*Equip items, learn skills, rearrange the bag, restart — every change persisted
exactly.* Confirmed at the keyboard; the client runs at a higher integrity level
than the agent's shell and cannot be driven programmatically (phase 1's
operational note), so every round of this was a person playing. The save was
backed up first as `els_db.sql.bak-pre-phase5` plus its `-wal` and `-shm`,
because the phase migrates the schema to v4.

**It took six rounds, and the faults were not in the inventory or skill code.**
Everything §5 actually asked for worked on the first build. What did not work
was the surrounding game — the parts phase 4 had built but never had a reason to
exercise end to end, which only a real session reaches:

| Round | Symptom | Actually |
|---|---|---|
| 1 | new character had no gear | `GBase_Item`, not `gup_create_unit` — corrections 10–13 |
| 1 | monsters "dropped nothing" | drop rate was right; ED had no coins — decision 8, reversed |
| 1 | ESC would not leave a dungeon | sticky client tutorial flag — correction 9 |
| 2 | stuck after the boss died | missing `..._KILLALLNPC_CHECK_NOT` — correction 14 |
| 3 | stamina bar filled the screen | `m_iSpiritMax = 0` → NaN — correction 16 |
| 4 | could not level up | EXP paid at room exit, and a field has none — correction 17 |
| 4 | every NPC threw an error dialog | `EGS_TALK_WITH_NPC_REQ` unanswered — correction 18 |
| 5 | level-up had no effect | `EGS_CHAR_LEVEL_UP_NOT` never sent — correction 19 |

The pattern across all eight: **not one was a wrong value, and not one produced
an error.** Six were a packet or a payout that never happened, and the client's
response to nothing happening is to wait quietly — a full EXP bar that never
turns over, a cleared dungeon that never ends, a bar drawn at NaN width. The
packet log was decisive in exactly one of them (correction 18, an `UNHANDLED`
line); everywhere else the log looked healthy and the last line before the
silence was the clue.

Round 6 was test data rather than a fault: `reyaa` set to level 50 with 999 SP
directly in `unit`, so the skill half could be exercised without grinding. EXP
was set to 38,676,300 — level 50's cumulative total from `ScriptData/ExpTable.xls`
— rather than left where it was, because the EXP bar is drawn against the
level's own base and a value below it draws a negative fill; and `cur_hp` to 0,
the schema's "never stored" marker, which resolves to full HP at the new level
instead of a level-2 sliver.

Re-running it later, this is what the log should say:

| Step | Expect in `offline_server.log` |
|---|---|
| Launch | `DB schema upgraded to v4 (item detail, skill points)`, once |
| Create a character | `SKILL seeded N default skill(s) for class C`, then `ITEM promotional costume for class N: 5 worn` and `ITEM beginner equipment for class N: 5 worn` |
| Enter the village | `ITEM inventory loaded for unitUID=...: N item(s)`, `SKILL loaded ... row(s)` |
| Kill anything | `GAME -> +N exp`, then `REWARD unitUID=... +N exp (N total)` — the second line is the one correction 17 added |
| Level up | `REWARD unitUID=... LEVEL UP a -> b` and `+N SP`, with the effect on screen |
| Clear a dungeon | `GAME all NPCs dead - dungeon clear broadcast sent`, then the result screen ~7s later |
| Talk to an NPC | `GAME talked to NPC N` |
| Learn a skill | `SKILL skill ... level 0 -> 1` |
| Equip a weapon | `ITEM move 1/0 -> 9/9, 2 slot(s) changed` |
| Drag items around the bag | one `ITEM move` line per drag |
| Sell something | `SHOP sold 1 x item ... for N ED` |
| Restart, reselect | the same item count, the same skills, the weapon still worn |

`grep UNHANDLED offline_packets.log` should show only `EGS_SKILL_USE_REQ`
(correction 15 — fire-and-forget, expected). Anything else is a real gap.

### Corrections to this plan, found by doing it

1. **§5's `EGS_GET_MY_INVENTORY_REQ/ACK` and `EGS_SELECT_UNIT_INVENTORY_INFO_NOT`
   are both the wrong packets.** `EGS_GET_MY_INVENTORY_ACK` carries no inventory
   at all — phase 2 already found that and answers it as the last step of
   character select — and `EGS_SELECT_UNIT_INVENTORY_INFO_NOT` is the overflow
   packet for an inventory too large for one message, which a solo save never
   reaches. The inventory arrives in `EGS_SELECT_UNIT_1_NOT`'s `m_mapItem`, and
   the packet that does the work every other time is
   `EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ`.

2. **"Equip/unequip, `m_mapEquippedItem` in `KUnitInfo`" is half of it, and the
   missing half is the one that makes gear *stick*.**
   `CX2Unit::UnitData::SetKUnitInfo` builds `m_NowEqipItemUIDList` from
   `m_mapEquippedItem` ([X2Unit.cpp:3252](X2Lib/X2Unit.cpp#L3252)) and
   `ResetEqip()` then resolves those UIDs against the inventory — so an equipped
   item has to appear **twice**, once in `KUnitInfo::m_mapEquippedItem` and once
   inside `m_mapItem` at its `ST_E_EQUIP` slot. Send only the first and the
   equip list points at items the inventory does not have; send only the second
   and nothing is worn. The real server sends both as well
   (`KInventory::GetEquippedItem` and `GetInventoryInfo`), which is what made
   this findable rather than guessable.

3. **There is no "split" packet.** §5 lists move/swap/split/stack as four
   operations; the protocol has one, `EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ`, whose
   four cases inside `KInventory::MoveItem` are empty-destination move, same-ID
   quantity merge, plain swap, and relocate-the-blocker. Splitting a stack is
   not among them — this build's client has no split UI.

4. **The skill-tree relaxations §5 asks to mirror need almost no offline code.**
   `SERV_IRUHADEV_SKILLTREE_NO_LOCK` is entirely client-side in
   `X2UISkillTreeNew.cpp`; its server half (`UserSkillTree.cpp:1489`) sits inside
   `CheckGetNewSkill`'s either/or test, which the offline `LearnSkills` does not
   port at all. `SERV_IRUHADEV_SKILL_SLOT_B_FREE` needs one thing and it is data,
   not logic: the permanent sentinel end date `2049-12-31 23:59:00` that
   `KUserSkillTree::ExpandSkillSlotB` writes under that flag, reported in
   `KUnitSkillData::m_wstrSkillSlotBEndDate` alongside `SSBES_PERMANENT`. Same
   for `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`: phase 2's
   `CX2OfflineDB::BaseSlotSize` already carried its `return 6`, so category 11
   has been six slots wide since the schema was first seeded.

5. **The client cannot be asked what the equipped gear is worth.**
   `CX2Unit::GetEqipStat()` is the obvious source for `m_kGameStat`'s gear half
   and it is a trap: it can only answer for `GetMyUser()->GetSelectUnit()`, and
   `MakeUnitInfoFromRow` runs for *every* character in the list, so the second
   slot would be handed the first slot's weapon. The right seam is one level
   down — `g_pData->GetItemStatCalculator().CalculateItemStat( ..., pTemplet )`,
   which is what `CX2Item::GetStat` itself calls under
   `SERV_NEW_ITEM_SYSTEM_2013_05` ([X2Item.cpp:245](X2Lib/X2Item.cpp#L245)) and
   which takes a templet rather than a unit.

6. **A quick-slot item consumed by the ACK alone heals nothing.** Phase 4's
   correction 6 for a fourth time:
   `CX2UIQuickSlot::Handler_EGS_USE_QUICK_SLOT_NOT` holds the only calls to
   `UseItemSpecialAbility` and `ApplyBuffFactorToGUUser`
   ([X2UIQuickSlot.cpp:1032](X2Lib/X2UIQuickSlot.cpp#L1032)), so the `_NOT` is
   the packet that does the work. Its `default:` arm is an
   `ASSERT( !L"Can not use this item in this State" )`, so it must be sent only
   inside a room — which is also the only place there is a `CX2Game` to apply it
   to.

7. **`EGS_SORT_CATEGORY_ITEM_ACK` does not carry slot infos.** Every other
   inventory ACK carries `std::vector< KInventoryItemInfo >`; this one carries
   `std::vector< UidType >`, and `CX2Inventory::UpdateCategorySlotList` takes the
   *index* in that vector as the slot
   ([X2Inventory.cpp:1252](X2Lib/X2Inventory.cpp#L1252)). Filling it the usual
   way compiles and silently sorts nothing.

8. **`DropTable.lua` rows with no EXP and no ED still matter.** Phase 4 skipped
   them, correctly for phase 4, which only read EXP and ED — and keeping that
   filter would have thrown away every row whose only reward is an item. The
   test has to become "no EXP *and* no ED *and* no item cases".

Corrections 9–12 came out of play-testing rather than reading, and 10–12 out of
the live `Game01` stored procedures, which the user has access to and this
snapshot does not carry in full.

9. **Leaving a dungeon has to be gated on server state, not on the client's
   tutorial flags.** `CX2StateDungeonGame` sends `EGS_LEAVE_ROOM_REQ` and then
   waits for *two* things before it will unwind: the ACK, and an
   `EGS_UPDATE_UNIT_INFO_NOT`. Phase 4 suppressed the second one for tutorial
   rooms and picked `GetIsExitingTutorial()` to detect them — which is wrong in
   the worst way, because that flag is set when the tutorial is *left*
   ([X2StateDungeonGame.cpp:1448](X2Lib/X2StateDungeonGame.cpp#L1448)) and
   cleared only on the way back to character select, PvP lobby or unit select.
   So it reads true for the rest of the session and every subsequent dungeon
   becomes inescapable — six `EGS_LEAVE_ROOM_REQ`/`ACK` pairs in the packet log
   with no `_NOT` between them. `GetIsPlayingTutorial()` is no better; it is
   cleared one line *before* the REQ is sent (:1445). The room is the server's
   own object, so the offline server tracks `KOfflineRoom::m_bTutorial` when it
   creates one and asks that instead. **The general rule: never branch offline
   server behaviour on a client flag whose lifetime the client owns.**

10. **`dbo.gup_create_unit_set_promotion` creates nothing.** The name says
    otherwise and it is the procedure the create path calls right after
    `gup_create_unit` ([GSGameDBThread.cpp:7723](KncWX2Server/GameServer/GSGameDBThread.cpp#L7723)),
    but its whole body is one INSERT into `GItemPeriod` — `7, DATEADD(DD,7,…)` —
    for the rows already at `InventoryCategory = 9 AND SlotID NOT IN (1,3,5,7,9)`.
    It *dates* the costume; it does not grant it. The starting items come from
    inside `gup_create_unit` itself, copied out of a table:

    ```sql
    INSERT INTO dbo.GItem (UnitUID, ItemID, InventoryCategory, SlotID, UsageType, Quantity, Endurance, Inserted, RegDate)
    SELECT @iUnitUID, ItemID, InventoryCategory, SlotID, UsageType, Quantity, Endurance, 125, @sdtNow
    FROM dbo.GBase_Item WITH(NOLOCK) WHERE UnitClass = @iUnitClass_
    IF @@ERROR <> 0 OR @@ROWCOUNT <> 10   -- @iOK = -28
    ```

    Exactly ten rows per class, and all ten are positioned — a new character is
    created with everything already worn. The earlier claim in this record that
    "the real `gup_create_unit` inserts no items at all" was read off the copy
    in `DataBase/`, which predates the `GBase_Item` refactor.

11. **Equip slot IDs are `CX2Unit::NESI_*`, and the odd/even split is the
    fashion layer.** ([X2Unit.h:223](X2Lib/X2Unit.h#L223))

    | Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
    |---|---|---|---|---|---|---|---|---|---|---|
    | | BODY | BODY_FASHION | LEG | LEG_FASHION | HAND | HAND_FASHION | FOOT | FOOT_FASHION | WEAPON | WEAPON_FASHION |

    So `GBase_Item`'s ten rows are five gear pieces at 9/1/3/5/7 and five
    costume pieces at 10/2/4/6/8, and `set_promotion`'s `NOT IN (1,3,5,7,9)`
    is "the costume half" — which is what makes the 7-day period a costume
    rental rather than a limit on the gear.

12. **A UseLevel the character has not reached is not a reason to move an item
    to the bag.** The beginner set is UseLevel 2 and a new character is level 1,
    and the first version of this shipped it into the bag on the strength of
    `CX2Unit::ResetEqip` → `IsPossibleAddEqip` → `EqipAbility` →
    `CanEquipAsParts( .., m_Level )` rejecting it
    ([X2Unit.cpp:255](X2Lib/X2Unit.cpp#L255)). That chain is real, but it only
    governs `m_NormalEqipItemUIDList` — the stat and render lists. **The
    equipment window is drawn from inventory category 9**, so an item in an
    equip slot shows there whether or not `ResetEqip` accepted it. Live ships
    the same UseLevel-2 data into the same slots at level 1, so putting it in
    the slots reproduces live exactly: visible from the start, inert until the
    first level-up. Reasoning from one validation path to "therefore it cannot
    go there" skipped the question of what actually draws the UI.

13. **The item IDs are `dbo.GBase_Item`'s, and the two available sources
    disagree.** Before the live table was available this record derived the
    beginner gear from `ScriptData/ItemTemplet.xlsx`'s 2013-reform block —
    `<class>002000` for the weapon and `+50/+140/+230/+320` for the armour,
    whose equip positions do line up one for one (`2 = EP_WEAPON_HAND`,
    `8 = EP_DEFENCE_BODY`, `9 = LEG`, `10 = HAND`, `11 = FOOT`). The live
    `GBase_Item` names a different set entirely — `131641..` / `111094..`, and
    `112700..` for Ara — and **none of those IDs appear anywhere in this tree's
    `ItemTemplet.xlsx`**, checked by raw byte search across all 33 of its parts,
    not just the column the earlier lookup read. Neither source is wrong; the
    spreadsheet in this snapshot and the database the client was served by are
    simply from different points in the game's life, and which one an install
    resolves depends on what is packed in its `.kom`.

    So the seeder tries `GBase_Item` first — it is what actually built
    characters — falls back to the xlsx block for classes 1–5 where it exists,
    and **logs which source each piece came from**. Both are real, citable data;
    neither is derived, which is what keeps this inside the project's fallback
    rule rather than outside it. If the log reports pieces as `unresolvable`,
    that is the install's packed item table differing from live, and it is
    visible rather than silent.

    The table also settles the classes the repo could not: Chung is
    `111114..111118` / `133125..133129`, and **classes 7, 8 and 9 all carry
    Ara's set verbatim** — Elesis appears to have been given Ara's row and never
    corrected, and class 9 is unreachable because `gup_create_unit` rejects any
    class outside 1–8. Transcribed as-is rather than "fixed", with a log line
    when a class is handed a set that is not its own.
14. **Clearing a dungeon needs `EGS_DUNGEON_KILLALLNPC_CHECK_NOT`, not just the
    ACK — and getting it wrong hangs the run with no error anywhere.** Found by
    play-testing: the boss dies, the clear effect plays, and then the client
    sits in the finished room heartbeating forever. Nothing appears as
    `UNHANDLED`, because the client is not waiting on a packet — it is waiting on
    a timer that was never started.

    The chain: `KDungeonRoom` answers the host's check with
    `ERM_DUNGEON_KILLALLNPC_CHECK_ACK` **and then**, when the result is true,
    `BroadCastID( ERM_DUNGEON_KILLALLNPC_CHECK_NOT )` to every member
    ([CenterServer/DungeonRoom.cpp:8886](KncWX2Server/CenterServer/DungeonRoom.cpp#L8886)),
    which each GameServer forwards to its own client as
    `EGS_DUNGEON_KILLALLNPC_CHECK_NOT`
    ([GSUserSecurity.cpp:1722](KncWX2Server/GameServer/GSUserSecurity.cpp#L1722)).
    Only the `_NOT` handler calls `CX2DungeonSubStage::ClearDungeonGame()`, and
    that is what sets `m_fEndGameDelay = 7.0f` and starts the ending event
    ([X2DungeonSubStage.cpp:1223](X2Lib/X2DungeonSubStage.cpp#L1223)). With
    `m_fEndGameDelay` left at 0 the countdown block in `OnFrameMove` never runs
    at all, and that countdown is the **only** thing that ever calls
    `EndDungeonGameReq()` — so `EGS_END_GAME_REQ` is never sent and the result
    screen never comes.

    What makes this findable only by reading the flags: the client's *own* ACK
    handler used to call `ClearDungeonGame()`, and that call is
    `#ifndef SERV_FIX_NONE_NPC_DUNGEON_LINES`
    ([X2StateDungeonGame.cpp:3555](X2Lib/X2StateDungeonGame.cpp#L3555)). The flag
    **is** defined — `ServerDefine.h:2645`, reachable from `X2Lib` through
    `X2ServerPacket.h` → `CommonPacket.h` — so that path is compiled out and the
    2013 refactor moved the work onto the broadcast. Reading the ACK handler
    alone shows a clear that appears to happen locally.

    Two more traps in the same function, for whoever reads it next: the block at
    `X2DungeonSubStage.cpp:607` that also starts the ending event is inside an
    `#if 0`, so it is not a second path; and the end-game countdown gates on
    `GetDungeonEndingEvent()->m_bEndChapter1` and re-arms itself with
    `m_fEndGameDelay = 1.f` when it is false, which is a second, independent way
    for the same symptom to appear.

    **Phase 4's correction 6 for the fifth time.** The running score for `_NOT`
    packets that do real work rather than merely informing: `EGS_DROP_ITEM_NOT`,
    `EGS_USE_QUICK_SLOT_NOT`, `EGS_UPDATE_UNIT_INFO_NOT`,
    `EGS_GET_ITEM_REALTIME_NOT`, and now `EGS_DUNGEON_KILLALLNPC_CHECK_NOT`.
    **Treat "the ACK is enough" as the null hypothesis to disprove, not the
    default** — for any REQ whose real-server handler is in a room or FSM class,
    grep the CenterServer room for a `BroadCast*` on the same event before
    calling the handler done.

15. **`EGS_SKILL_USE_REQ` is fire-and-forget and correctly left unhandled.** It
    shows up in `offline_packets.log` as `UNHANDLED` once per skill cast, which
    looks alarming and is not: `CX2GUUser::Send_SKILL_USE_REQ` has its
    `AddServerPacket( EGS_SKILL_USE_ACK, 60.f )` commented out
    ([X2GUUser.cpp:33725](X2Lib/X2GUUser.cpp#L33725)), so the client never waits
    for a reply. On the live server it feeds statistics and skill-use quest
    counters; offline it has nothing to feed.

16. **A zeroed `m_iSpiritMax` is a NaN, and a NaN is a stretched bitmap.** The
    stamina gauge rendered as a bar spanning most of the screen with a
    `-1.$%` tooltip. Phase 3 sent `m_iSpiritMax = 0` and `m_iSpirit = 0` as
    placeholders, and every consumer computes `m_iSpirit / (float)m_iSpiritMax`
    with no guard at all — `0.0f/0.0f` is NaN, `NaN * originalSize.x` is the
    width handed to `SetSizeX`, and `StringCchPrintfW( …, L"%.1f%%", … )` renders
    NaN as exactly `-1.$` under MSVC
    ([X2StateMenu.cpp:3378](X2Lib/X2StateMenu.cpp#L3378)). The tooltip text was
    the thing that identified it: a garbled float is a much better clue than a
    misdrawn bar.

    **The lesson is about zero as a placeholder, not about spirit.** Phase 3
    zeroed every field it had no data for, which is right for a count and wrong
    for a denominator. Worth a sweep of the other placeholder zeroes for the
    same shape: anything the client divides by, or uses as a `%` numerator's
    base, cannot be left at 0.

    **That sweep was done** — every float-cast member denominator in `X2Lib`'s
    gameplay code, not just the ones with `Max` in the name:

    ```sh
    grep -rnoE "/ *\( *float *\) *\(?[A-Za-z_][A-Za-z0-9_]*(->|\.)[A-Za-z0-9_]+" X2Lib/*.cpp \
      | sed 's/^[^:]*:[0-9]*://' | sort | uniq -c | sort -rn
    ```

    It comes back clean. Besides `m_iSpiritMax` (11 sites once the long
    `g_pData->GetMyUser()->…` spellings are counted) the only server-fed
    denominator is `channelInfo.m_iMaxUser`, which phase 2 already sends as 100
    ([Handlers_Login.cpp:107](X2Lib/Offline/Handlers_Login.cpp#L107));
    `pSquareSlot->m_JoinMaxNum` is unreachable because offline never builds a
    square slot, and the `pkItemTemplet->…Endurance` divisions read the client's
    own item table rather than anything the offline server sends.

17. **EXP is paid per kill, not at the end of the run — and paying it at the end
    meant field EXP was never paid at all.** Phase 4 accumulated every kill into
    `KOfflineRoom::m_iRewardEXP` and applied the total at `EGS_END_GAME_REQ` or
    `EGS_LEAVE_ROOM_REQ`. That is fine for a dungeon and silently broken for a
    field: **a field is never ended and never left.** The player walks out
    through `EGS_STATE_CHANGE_FIELD_REQ`, so the accumulator was simply
    discarded, and a character could grind a field indefinitely without gaining
    a single point.

    It presented as "the EXP bar is full but I don't level up", which is worth
    unpacking because the two halves have different causes. The bar filled
    because **the client adds the EXP to its own unit data as each kill
    notification arrives** — `pMyUnitData->m_EXP += iExp` in
    `CX2Game::ProcessExpListByNpcUnitDie`
    ([X2Game.cpp:7498](X2Lib/X2Game.cpp#L7498)) — so the display was live and
    local. The level never moved because the client never levels itself; only
    the server grants a level, and ours was never asked to. The save confirmed
    it: `level 1, exp 468`, with no `REWARD` line anywhere in the log.

    The real server does it per kill. `KGSUser`'s `ERM_NPC_UNIT_DIE_NOT` handler
    runs `SendPacket( EGS_NPC_UNIT_DIE_NOT, … )`, then
    `m_kEXP.AddExp( SumEXP() )`, then `CheckCharLevelUp()`, on **every monster**
    ([GSUserRoomCommon.cpp:1959](KncWX2Server/GameServer/GSUserRoomCommon.cpp#L1959)).
    `ApplyDungeonReward` now runs from the NPC-die handler, and the two former
    payout sites were corrected rather than merely left alone:
    `EGS_LEAVE_ROOM_REQ` pays nothing at all now, and `EGS_END_GAME_REQ` pays
    **only the 30% clear bonus** — paying `iAddEXP` again there would have
    doubled every dungeon. The result screen's `m_nOldEXP` also had to become
    `kBefore.m_iEXP - iAddEXP`, the same correction the ED line already carried,
    or the bar animates from the finished total to itself.

    **The shape to watch for: a reward model that assumes an end.** Anything
    accumulated in `KOfflineRoom` and settled on exit is wrong for a field, and
    the field will not complain. `m_iKillNPCNum` and `m_iRewardED` are still
    accumulators, but both are display-only on the result screen, which a field
    never shows.

18. **`EGS_TALK_WITH_NPC_REQ` has to be answered or every village NPC throws an
    error dialog.** Clicking any NPC sends it and registers a wait
    ([X2TFieldNpc.cpp:2198](X2Lib/X2TFieldNpc.cpp#L2198)); with no ACK the wait
    expires and the client raises its own "no reply from the server" popup — the
    `E_SYSTEM_*` code seen on screen. The conversation itself is entirely
    client-side, so the ACK was all that was missing. `KGSUser` answers
    `NET_OK` and then runs the quest manager and title manager
    ([GSUserGameCommon.cpp:1660](KncWX2Server/GameServer/GSUserGameCommon.cpp#L1660));
    neither system exists offline, so neither is faked. Its only refusal,
    `ERR_ITEM_14`, is for talking mid-trade, which one player cannot do — so the
    answer is unconditional.

19. **A level-up is silent unless `EGS_CHAR_LEVEL_UP_NOT` is sent.** Correction
    17 made levels actually happen; this is the half that makes them *look* like
    they happened. The client has no other route to the effect: the packet's
    handler is what assigns the new level and both stats, refills HP and MP,
    calls `DisplayLevelUpEffect`, and sets `CX2Unit::SetIsLevelUp( true )`
    ([X2StateMenu.cpp:4576](X2Lib/X2StateMenu.cpp#L4576)). That flag is then read
    in two more places — the village replays the effect off it on its next frame
    ([X2TFieldGame.cpp:1932](X2Lib/X2TFieldGame.cpp#L1932)), and the dungeon
    result screen keys its level-up animation off it
    ([X2StateDungeonResult.cpp:1121](X2Lib/X2StateDungeonResult.cpp#L1121)) — so
    one missing packet costs three separate pieces of presentation.

    Sent from both places a level can be gained: the per-kill path, and
    `EGS_END_GAME_REQ` when the 30% clear bonus is what tips it over. Unlike
    `EGS_UPDATE_UNIT_INFO_NOT` it is safe to send from anywhere — the village,
    the dungeon and the battlefield all dispatch it
    (`X2StateField.cpp:1138`, `X2StateDungeonGame.cpp:1902`,
    `X2StateBattleField.cpp:361`) and none of them touch the dungeon leave gate.
    It carries **both** stats, base and game: the handler assigns each over the
    unit's own and rebuilds max HP from the game one, so sending only the base
    stat would strip the character's gear bonuses at the moment it levelled.

    `ScriptData/ExpTable.xls` is the EXP curve, in the clear: three columns,
    level / required EXP / cumulative total, 80 rows. Useful for reading a
    level's threshold without touching the packed client tables — level 2 is 600
    and level 50 is 38,676,300 — and it is the table the client's own
    `GetEXPData( n ).m_nTotalExp` mirrors. Reading it needs `xlrd`; the file is
    the old binary `.xls` format, not `.xlsx`.

    Fixed in `KUnitInfo` only, and that is sufficient:
    `SERV_DELETE_ROOM_USER_INFO_DATA` is defined in this build, so
    `KRoomUserInfo` no longer carries the pair and the block in
    `CX2Unit::UnitData::SetKRoomUserInfo` that used to copy it
    ([X2Unit.cpp:3446](X2Lib/X2Unit.cpp#L3446)) is compiled out. Entering a room
    cannot clobber what `SetKUnitInfo` put on the unit, and the dungeon room's
    own gauge reads the unit rather than the slot
    ([X2StateDungeonRoom.cpp:3029](X2Lib/X2StateDungeonRoom.cpp#L3029)) — the
    first attempt at this fix set the fields on `KRoomUserInfo` too and failed
    to compile, which is how the flag was found.


### Packets the plan did not predict

Answered, with the rules ported from `KInventory` / `KUserSkillTree`:

```
EGS_CHANGE_INVENTORY_SLOT_ITEM_REQ/ACK   moving, swapping, equipping, stacking
EGS_CHANGE_EQUIPPED_ITEM_IN_FIELD_NOT    redraws the village character
EGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT     redraws the in-dungeon character
EGS_DELETE_ITEM_REQ/ACK                  discarding
EGS_SORT_CATEGORY_ITEM_REQ/ACK           the sort button
EGS_USE_ITEM_IN_INVENTORY_REQ/ACK        right-click use, GetCanUseInventory only
EGS_USE_QUICK_SLOT_REQ/ACK + _NOT        the consumable bar
EGS_BUY_ED_ITEM_REQ/ACK                  the ED shops
EGS_SELL_ED_ITEM_REQ/ACK                 selling back
EGS_REPAIR_ITEM_REQ/ACK                  the blacksmith
EGS_GET_SKILL_REQ/ACK                    learning and upgrading
EGS_RESET_SKILL_REQ/ACK                  one skill back down
EGS_INIT_SKILL_TREE_REQ/ACK              the whole tree
EGS_CHANGE_SKILL_SLOT_REQ/ACK            the eight equipped slots
```

The drop chain, which phase 4 listed only as `EGS_GET_ITEM_REQ`:

```
EGS_DROP_ITEM_NOT                        puts the item on the floor
EGS_GET_ITEM_ACK                         clears the client's 60s wait
EGS_GET_ITEM_NOT                         draws the pickup, removes the sprite
EGS_GET_ITEM_REALTIME_NOT                the packet that adds it to the bag
```

Refused rather than left unhandled, because each arms an `AddServerPacket` wait
and a dropped one becomes a modal network error five seconds later:

```
EGS_ENCHANT_ITEM_REQ/ACK    ERR_ENCHANT_ITEM_00
EGS_SOCKET_ITEM_REQ/ACK     ERR_SOCKET_ITEM_00
```

Neither implemented nor refused, so they will appear as `UNHANDLED` in the
packet log if an NPC reaches them: identify, evaluate, resolve, synthesis,
manufacture, item exchange, seal/unseal, attribute enchant, random-item cubes,
titles, the bank, pet inventories, the temp inventory, trade and the personal
shop. All need server tables with no client copy, or belong to phase 7. Ten
speculative refusals with guessed error codes would be worse than a log line
naming exactly which packet the player reached.

### What was actually built, against *Code layout to create*

New, and the plan predicted one of the four:

```
X2Lib/Offline/Handlers_Inventory.cpp     predicted
X2Lib/Offline/Handlers_Skill.cpp         the skill half needed its own file
X2Lib/Offline/X2OfflineInventory.h/.cpp  KInventory, reduced to one character
X2Lib/Offline/X2OfflineSkill.h/.cpp      KUserSkillTree, likewise
```

Extended:

```
X2OfflineDB           schema v4, item and skill CRUD, SaveSkillPoint
X2OfflineDropTable    the item lottery - DropItemList, DropGroupList, AddToGroup
X2OfflineServer       MakeGameStat, equipped items and skills in every KUnitInfo
Handlers_Room         drops on a kill, the pickup chain, SP on level-up
Handlers_Unit         SELECT_UNIT_1_NOT filled, default skills seeded on create
```

No client-side edits at all this phase, and no new `SERV_IRUHADEV_` flag — every
rule this phase needed already had a client-side accessor.

Schema v4 is additive (eleven `ALTER TABLE`s on `item`, one on `unit`, one on
`unit_skill`), so a v3 save upgrades rather than being wiped. The migration and
every new statement were dry-run against a copy of the live `els_db.sql` before
the build was deployed.

### Decisions made while implementing phase 5

1. **Write-through, no flush.** Every mutation writes its row to SQLite before
   the handler returns. The real server batches because it has thousands of
   users and a DB round trip; here a drag is one `UPDATE` on a local file at
   human speed, and the exit test is *every change persisted exactly* — which
   write-through is the only shape that cannot get wrong.

2. **Equippability is not ported, it is called.** `KInventory::IsAbleToEquip`
   needs `CompareLevel` plus `CompareUnitClass`, and the latter needs the
   class-compatibility table that `X2Lib` exposes nowhere — except inside
   `CX2Unit::CanEquipAsParts`, which is static, is the client's own check, and
   already contains both halves. `IsSuitableSlot` *is* ported, because it is
   pure templet arithmetic with no client equivalent.

3. **Item UIDs are SQLite row ids.** The client keys its own inventory by item
   UID and addresses items that way in `EGS_DELETE_ITEM_REQ`, so they have to
   survive a relog; `AUTOINCREMENT` gives that for free and keeps the save file
   hand-readable at the same time.

4. **Cash skill points are always zero and always expired.** Not a stub: it is
   the branch `KUserSkillTree::GetNecessarySkillPoint` takes for an account with
   no cash-skill ticket, so every cost is paid in plain SP through the studio's
   own code path rather than a special case of ours.

5. **SP per level comes from the client's Lua, not from a table of ours.**
   `CX2SkillTree::GetCalcInitSkillPoint` already sums
   `CalcLevelUpIncreaseSkillPoint( level )` over the levels
   ([X2SkillTree.cpp:4044](X2Lib/X2SkillTree.cpp#L4044)), which is the same
   function `CXSLSkillTree::GetCalcLevelUpIncreaseSkillPoint` read on the
   server. A level-up grants the difference between two calls to it.

6. **A single-skill reset does not require the scroll; a full tree reset does.**
   The real server refuses both without the consumable.
   `EGS_INIT_SKILL_TREE_REQ` *names* an item UID — the client only sends it when
   the player uses that scroll — so requiring it changes nothing.
   `EGS_RESET_SKILL_REQ` names no item, and the scroll is a cash-shop item with
   no offline source until phase 7, so requiring it would make a feature this
   phase is supposed to deliver impossible to use. It consumes a scroll when
   there is one and logs which of the two happened every time.

7. **Drops use the drop table's own probability, unmodified.**
   `KDropTable::NpcDropItem` multiplies it by contribution, dungeon factor,
   level factor and a party bonus; all four are 1.0 here, which is the same
   simplification phase 4 already made for EXP and ED — contribution is 1.0 for
   a solo player who did all the damage, the party bonus is 1.0 for a party of
   one, and the other two are room state the offline server does not model.
   Consistency with phase 4 matters more than picking a different guess.

8. **ED drops as coins on the floor, and only the pickup credits it.**
   *Reversed 2026-09-02 — the original decision was "no ED coins", on the
   grounds that phase 4 already credits a kill's ED out of `GetNpcReward` and
   dropping coins as well would pay twice. It does not double-pay, but it does
   make the money invisible: `EDProperty = 100` in `DropTable.lua` means the
   live game drops a coin on essentially every kill, and a player watching a
   monster die and leave nothing behind reads "drops are broken" — which is
   exactly what happened in the play-test.* The kill no longer credits ED
   directly. Instead `PushNpcDrop` splits the reward into `(rand()%2)+2` coins
   of `iED/iCoinNum`, records them in `KOfflineRoom::m_mapDropED`, and
   `EGS_GET_ITEM_REQ` moves each into `m_iCollectedED` as it is picked up, with
   `EGS_GET_ITEM_NOT`'s `m_mapGetED` filled so the client animates it. The
   result screen then reports `m_iCollectedED`, so **ED left on the floor is
   ED not earned** — same as live. Coin tier follows the amount (`EDCoinImage`
   bronze / silver / gold at ≤50 / ≤150 / above).

9. **Drops come out at +0 and identified.** `RandomEnchant` and
   `ItemSealProcess` read the enchant-event and seal tables, which are server
   data with no client copy, and `m_cItemState` is set to `IS_NORMAL` so nothing
   needs the identify NPC that is not implemented. An invented enchant curve
   would be exactly the band-aid the project rule names.

10. **A full bag loses the drop, and says so.** The real server parks it in a
    temp inventory with its own screen and packets; that is a subsystem, not a
    line of code. The pickup logs `the temp inventory is not implemented
    offline, so it is lost` rather than silently doing nothing.

11. **Gear never wears out, and repair is implemented anyway.** Endurance decay
    per stage is real behaviour and is *not* implemented — wearing gear out
    without a working repair path would be strictly worse than neither. Repair
    is implemented because it is exact from client data alone (the per-point
    cost is on the templet) and because clicking Repair at the blacksmith has to
    answer rather than time out. It will report zero cost until decay lands.

12. **`m_kGameStat` is base plus gear everywhere, through one function.**
    `MakeGameStat` loads the row's *own* inventory first, so the three places
    that send a game stat — `KUnitInfo`, `KRoomUserInfo` and
    `EGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT` — cannot drift or pick up the wrong
    character's weapon. Socket and enchant contributions are left out because
    nothing offline can produce either.

13. **A skill slot pointing at a level-0 skill is dropped on load.** A skill
    reset to level 0 keeps its `unit_skill` row on purpose — the client draws
    level 0 differently from never-learned — and the row keeps its slot column.
    Honouring that slot would put an unusable skill on the bar.

14. **An item that cannot be placed on load is kept, not deleted.** Stock
    `KInventory::Init` relocates a wrong-position item rather than dropping it;
    so does `CX2OfflineInventory::Load`, and when there is nowhere to relocate
    it the row stays in the save file, out of the slot grid, with a log line.
    The one thing that produces this is a slot-count change between builds —
    reverting `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`, say, which `MODS.md` already
    warns about.

15. **Spirit — the stamina gauge — is not modelled, and reports full.** Nothing
    offline consumes it, nothing regenerates it, and no dungeon is refused for
    lack of it. Since every consumer displays only the ratio
    `m_iSpirit / m_iSpiritMax`, reporting the two *equal* renders the full gauge
    that "not modelled" calls for, and the value itself never reaches the
    screen. `CX2OfflineServer::FillSpirit` sends both as `SHRT_MAX`, chosen
    because `dbo.GSpirit.Spirit` is a `smallint` — so it is the largest spirit
    the shipped schema can hold, and therefore cannot be smaller than any
    `DungeonData::m_RequireSpirit` the real system could have set. That makes it
    a bound read out of the repo rather than a number picked to look plausible,
    and it is named `SPIRIT_FULL` rather than `MAX_SPIRIT` so it cannot be
    mistaken for the real table value.

    **To model it properly, two things are needed and both are the user's to
    supply.** The maximum and the per-level curve live in the GameServer's
    `SpiritTable.lua`, which defines `MAX_SPIRIT` plus `{ LEVEL, SPIRIT }` rows
    read by `KSpiritTable::SetMaxSpirit_LUA` / `AddSpiritTable_LUA`
    (`KncWX2Server/GameServer/SpiritTable.cpp`) — and that file **is not in this
    tree at all**, only the C++ that loads it, so it would have to come off the
    live server's GameServer resource directory and then be XOR-encrypted and
    packed into `data036.kom` like every other script the client reads. The
    creation value is `dbo.GResurrectionStoneCnt.StartSpirit`, one row, which
    `gup_create_unit` reads into `GSpirit`. `KSpiritTable`'s constructor never
    initialises `m_iMaxSpirit`, so there is no in-repo default to fall back on
    and none was invented.

16. **The promotional costume is permanent here, and live rents it for 7 days.**
    `gup_create_unit_set_promotion` gives every costume piece a `GItemPeriod`
    row of seven days (correction 10), so on live it expires. Implementing that
    means item-period expiry — a clock, a sweep, an expiry notification and a
    UI that shows remaining days — which is a subsystem and not in this phase.
    A costume that silently vanished a week into a save would read as a
    persistence bug, i.e. as the very thing this phase's exit test is checking,
    so the divergence is deliberate and the creation log says so out loud:
    `(permanent here; live gives it a 7-day period)`.

### Operational notes, extending the earlier phases'

- **Dry-run a schema migration against a copy of the live save before
  deploying.** Thirteen `ALTER TABLE`s and eight new prepared statements went in
  this phase; running them through Python's `sqlite3` against a copy of
  `els_db.sql` took about a minute and would have caught a mistyped column name
  that no C++ compiler can see. Copy the whole WAL set, as phase 2's note says.
- **`grep -rho 'AddServerPacket( *[A-Z_0-9]*' X2Lib/*.cpp` is the phase-5
  checklist, not just a debugging aid.** Filtering its 333 armed waits down to
  the ones with ITEM / SKILL / INVEN / QUICK / SHOP in the name is how the
  repair and sort packets got found before a play-test hit them, and also how
  the list of things deliberately left unhandled got written down rather than
  discovered.
- **An ACK shaped like `KPacketOK` does not mean the reply is trivial.**
  `EGS_GET_ITEM_ACK` is a bare `m_iOK`, and the pickup it acknowledges takes
  three more packets to complete. Read who consumes the `_NOT`s before assuming
  the ACK is the whole story.
- **`MakeUnitInfoFromRow` runs once per character in the list, so anything it
  loads must be keyed by that row's unit UID.** Two of this phase's near-misses
  (correction 5, decision 12) were the same shape: a helper that reads "the
  current character" is wrong inside a loop over all of them.
- **Known risk, carried over rather than fixed:** `SkillPointForLevel` calls
  into the client's Lua state from the session worker thread, as phase 3's stat
  table and phase 4's dungeon-script reader already do. It fires on level-up,
  when the main thread is on the results screen. If a level-up ever corrupts the
  Lua state, this is the first place to look, and the fix is to cache the curve
  at load rather than call per level-up.

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

### Exit test — RUN, PASSED (2026-09-04)

**Status: phase 6 complete.** The story chain plays through, quests accept /
progress / complete / reward, titles and missions arrive, and a job advancement
really does change the class and survive a restart. Two loose ends are
deliberately deferred to the last phase rather than held against this one; they
are named at the bottom.

Everything below is in the deployed `X2_offline.exe`; the play-test was the
user's, as every phase's has been (phase 1's operational note: the client runs
at a higher integrity level than the agent's shell and cannot be driven
programmatically). The save was backed up first as `els_db.sql.bak-pre-phase6`
plus its `-wal` and `-shm`, and again as `-pre-v6` and `-pre-grant` at the two
later points where the schema or the data changed.

What to expect in `offline_server.log`:

| Step | Expect |
|---|---|
| Launch | `DB schema upgraded to v5 (quests, titles, missions)`, once; then `v6` on the next build |
| Select a character | `QUEST loaded for unitUID=...: N in progress, M completed`, then `TITLE loaded ...` and usually `TITLE N new mission(s) opened` |
| Take a quest from an NPC | `QUEST accepted <id> '<title>' (N sub-quest(s), from NPC <n>)` |
| Talk to the NPC it names | `QUEST <id> sub <s> NPC_TALK done (NPC <n>)` |
| Kill something it counts | `QUEST <id> sub <s> NPC_HUNT 3/10 (NPC <n>)` |
| A collection quest | `QUEST <id> sub <s> quest item <item> drops (2/5 held, rate ...)` per drop |
| Walk into the village it names | `QUEST <id> sub <s> VISIT_VILLAGE done (map ...)` |
| Clear the dungeon it names | `QUEST <id> sub <s> dungeon clear type 8 -> 1, done=1` |
| Spend a skill point | `QUEST <id> sub <s> USE_SKILL_POINT skill <n> -> 1/1, done=1` |
| Hand it in | `QUEST complete <id> took 5/5 x item ...`, `gave ...`, then `QUEST completed <id> '<title>': +N exp, +N ED, lv a -> b` |
| A job-advancement quest | `QUEST complete <id> CLASS CHANGE unitUID=... 3 -> 13`, then `SKILL seeded N default skill(s) for class 13` |
| A finished title mission | `TITLE mission <id> '<name>' complete -> title <id> awarded` |
| Wear a title | `TITLE unitUID=... wears title <id>` |
| Restart, reselect | the same `QUEST loaded ...` counts, the same class, the same titles |

`grep UNHANDLED offline_packets.log` should now show **nothing**:
`EGS_SKILL_USE_REQ`, the one entry phase 5 left there (its correction 15), is
handled as of correction 17.

#### What the play-test actually took: seven rounds

Worth recording as a shape, not as a list of bugs. Every single failure was
silent — nothing errored, nothing appeared in a log, and in five of the seven
the symptom surfaced in a different system from the cause. The phase was
finished in one build; it took six more to be *correct*, and the diagnostics
added along the way did more work than the fixes.

| # | Reported as | Actually was | Written up |
|---|---|---|---|
| 1 | no quest at all after the tutorial | only player-initiated accept existed; the four epic auto-open sweeps were missing entirely | corr. 12 |
| 2 | "half working", and the quest completed instantly | the instant completion was *correct* studio behaviour; the real gap was chain continuation, invisible because `CheckAutoOpen` was silent | corr. 13 |
| 3 | Lowe should speak automatically | my own phase-2 placeholder — a faked "11005 completed" in `_2_NOT` — was telling the client the second story quest was already done | decision 11, reversed |
| 4 | Thief Pursuit never starts | the client never parses `m_iAfterQuestID`; the chain field the story runs on was being dropped at load | corr. 14 |
| 5 | 2nd Ruben dungeon stays locked | two independent causes: the result packet claimed the player died, and `m_mapDungeonClear` was never populated | corr. 15 |
| 6 | must leave and re-enter a village for its quest step to tick | the step's trigger fired before the step existed — and again one level down, through sub-quest group staging | corr. 16 |
| 7 | press K, spend SP, quest does not complete | the whole feature-use family of clear types had no driver; they are ticked from each feature's own packet handler, not from anything quest-shaped | corr. 17 |
| 8 | class changes but the top-left portrait does not | a gap in the **shipped client**, not in the emulation — the quest path refreshes the 3D model and not the gage | corr. 18 |

Two of those are worth separating from the rest, because they are not emulation
bugs at all and the reflex in both cases was to go looking for a packet I had
failed to send:

- **#4** was a *client* parser gap: the field is in the script the client
  already loads, and only the client's loader ignores it.
- **#8** was a *client* UI gap: the GameServer sends only
  `EGS_QUEST_COMPLETE_ACK` for a quest-driven class change, and four other
  class-change paths in the client refresh the portrait while the quest path
  does not.

Both are fixed behind flags in `Always.h` (`SERV_IRUHADEV_OFFLINE` and
`SERV_IRUHADEV_JOBCHANGE_PORTRAIT`), which is the point of the flag rule: the
local changes to studio code are one `#undef` away from being reverted.

#### What the census says about the remaining gaps

The one-shot census that goes into the log on first quest load now reports, per
sub-quest clear type, how many *reachable* steps use it and — when nothing
offline can advance it — one sentence saying why. Measured against the shipped
US script (1395 quest templets, all 1395 visible through the `ADD_SERVER_GROUP`
filter, client group 1):

```
driven                              not driven offline
  1 NPC_TALK                109        9 PVP_PLAY                 4
  2 NPC_HUNT                417       10 PVP_WIN                  7
  3 ITEM_COLLECTION         123       11 PVP_KILL                 5
  4 QUEST_ITEM_COLLECTION   693       13 ITEM_ENCHANT             1
  5 DUNGEON_TIME             37       14 ITEM_SOCKET              1
  6 DUNGEON_RANK             88       15 ITEM_ATTRIB              1
  7 DUNGEON_DAMAGE           14       16 ITEM_RESOLVE             1
  8 DUNGEON_CLEAR_COUNT      78       28 PVP_PLAY_ARRANGE         1
 18 USE_SKILL_POINT           1       32 SUITABLE_LEVEL_CLEAR     1
 24 VISIT_VILLAGE            50                                ----
 25 VISIT_FIELD              51                                  22
 26 VISIT_DUNGEON           205
 27 FIND_NPC                 47
                          -----
                           1913
```

**22 reachable steps out of 1935 cannot be advanced offline, and 17 of the 22
are PvP.** Nothing load-bearing for a story playthrough is unreachable.

Two things the census settled that guesswork would not have:

- **`clearType=18` — spend a skill point — has exactly one reachable step in
  the entire script**, and it is the one the user hit. The single most annoying
  bug of the phase was a one-of-a-kind piece of data.
- **Types 22, 23 and 36 do not appear at all.** Item use, skill use and
  learn-new-skill were all wired in correction 17 and have nothing to do. That
  work was speculative and produced nothing; the census is what makes that
  visible instead of leaving three drivers of unknown value in the tree.

#### Deferred to the last phase, by agreement

- **The formal exit test from level 1 has not been run end to end.** What was
  tested is the same ground by a different route: the story chain from the
  tutorial through Ruben on a fresh character, and the class change on a level
  50 one, with restarts in between. A clean level-1-to-job-change run is worth
  doing once more before the project is called finished.
- **`auto-open by the after-quest list of 12005: 1 candidate(s), 0 opened` /
  `refused with error 1074`, on every login.** The only recurring refusal left
  in the log. Almost certainly legitimate — a quest the character genuinely
  cannot take — but "almost certainly" is exactly the phrasing that produced
  rounds 1 through 5, so it gets checked rather than assumed.

### Corrections to this plan, found while implementing it

1. **`XS_CLASS_CHANGE` and `X2StateClassChange.cpp` are dead code, exactly like
   `X2StateBeginning.cpp`.** The phase text above points at them; nothing
   reaches them. `X2StateClassChange.cpp` sends and receives no packets at all,
   and every `SendGameMessage( XGM_STATE_CHANGE, XS_CLASS_CHANGE, ... )` in the
   tree is commented out ([X2UIQuestNew.cpp:1503](X2Lib/X2UIQuestNew.cpp#L1503),
   `:1616`, `X2UIQuestReceive.cpp:898`, `:980`). A class change is *entirely* a
   quest reward: the client reads the new class off `EGS_QUEST_COMPLETE_ACK`'s
   `m_kUpdateUnitInfo.m_kUnitInfo`, resets the skill-tree UI and shows
   `GetClassChangePopup()`. There is no class-change state and no class-change
   packet to implement.

2. **`EGS_UPDATE_QUEST_REQ` is dead on both sides.**
   `CX2State::Handler_EGS_UPDATE_QUEST_REQ`
   ([X2State.cpp:4235](X2Lib/X2State.cpp#L4235)) has no callers anywhere in
   `X2Lib`, and the GameServer has no handler for the packet either. It is
   answered anyway - it costs eight lines, and an unanswered REQ arms a wait -
   but it will never appear in the log.

3. **The client's quest templet has no `m_mapAcceptRewardItem`.** The server's
   does, and pays an item just for accepting a quest
   ([UserQuestManager.cpp:686](KncWX2Server/GameServer/UserQuestManager.cpp#L686)).
   `CX2QuestManager::AddQuestTemplet_LUA` never reads such a field, so there is
   nothing offline to read the list from and `EGS_NEW_QUEST_NOT`'s
   `m_vecUpdatedInventorySlot` goes out empty. A genuine divergence from live,
   not a decision - the data is not in the client's copy of the script.

4. **`EXCEPTION_EPIC_QUEST_TEMPLET` is off**, so epic quests (`m_eQuestType >=
   5`) do load on the client. It is commented out at `ServerDefine.h:370`; had
   it been on, `AddQuestTemplet_LUA` would return before storing them and the
   epic chain would have no templets at all.

5. **A sub-quest instance vector is POSITIONAL, and nothing enforces it.**
   `KQuestInstance::m_vecSubQuestInstance[i]` describes
   `QuestTemplet::m_vecSubQuest[i]`; every server handler indexes it that way
   and so does `CX2QuestManager::SetKQuestInstance`. Nothing in the struct says
   so, and a vector built in SQLite row order rather than templet order would
   look right in the save file and advance the wrong steps. `CX2OfflineQuest`
   therefore rebuilds it from the templet on every load and stores no ordering
   of its own.

6. **A mission's ID and the ID of the title it awards are the same number**, and
   the real server relies on that rather than on `MissionTemplet::m_iTitleID`:
   `CheckNewMission` looks a *mission* up in the *title* map to decide whether
   it is already done
   ([UserTitleManager.cpp:325](KncWX2Server/GameServer/UserTitleManager.cpp#L325)),
   and `CheckCompleteMission` logs an error if the two ever disagree (`:1335`).

7. **`Reward::m_wstrName` and `Reward::m_iBuff` are dead fields.** The quest
   templet carries a reward title name and a reward buff; `grep` finds no reader
   for either in `X2Lib` or `KncWX2Server`. Quests do not award titles.

8. **Editing a header that lives inside the precompiled header does not rebuild
   the PCH.** Adding the accessor to `X2Lib/X2TitleManager.h` produced
   `error C2039: 'GetMapTitleMission' : is not a member of 'CX2TitleManager'`
   from a translation unit that had just included it - and a
   `#ifndef ... #error` probe inserted next to the accessor did *not* fire,
   which is what identified the cause: the whole region came from a stale
   `US_SERVICE\X2Lib.pch`. Deleting the `.pch` does not help either (msbuild
   still thinks it is up to date, and every file then fails with C1083).
   **`touch X2Lib/stdafx.cpp` and rebuild.**

9. **The story chain is not offered by anyone — the server hands it out, and
   phase 6's first build did not.** Found by the first play-test: the tutorial
   finished, the character walked into Ruben, and no quest appeared. Only
   *player-initiated* accept was implemented (`EGS_NEW_QUEST_REQ`, sent when the
   player clicks a quest on an NPC), and an epic quest is never clicked. The
   GameServer accepts on the player's behalf from four places, all in
   `KUserQuestManager`:

   | Trigger | Function | Condition it reads |
   |---|---|---|
   | Enter a village | `CheckEpicQuest_SuccessStateChange` (`:5970`) | `m_Condition.m_setEnableVillage` |
   | Enter a dungeon | `CheckEpicQuest_EnterDungeon` (`:7363`) | `m_setEnableDungeon` |
   | Enter a field | `CheckEpicQuest_EnterBattleField` (`:7428`) | `m_setEnableBattleField` |
   | Login, and after every completion | `CheckEpicQuest_NewQuestByBeforeQuest` (`:6034`, called from `SetUnitQuest:365` and `DBE_QUEST_COMPLETE_ACK:5175`) | `m_Condition.m_vecBeforeQuestID` all complete |

   All four call `Handler_EGS_NEW_QUEST_REQ` with `IsAfterQuest = true`, i.e.
   they skip the start-NPC check, and with `m_iTalkNPCID = NUI_NONE`. The two
   prebuilt lists they walk (`m_vecEpicQuestTemplet`,
   `m_vecExistBeforeEpicQuestTemplet`, built at
   [XSLQuestManager.cpp:336-404](KncWX2Server/Common/X2Data/XSLQuestManager.cpp#L336))
   are only filters over the templet map, so `CX2OfflineQuest::CheckAutoOpen`
   applies them inline over the client's own `GetMapQuestTemplet()`.

   Two asymmetries in the original are transcribed rather than tidied: a quest
   with an **empty** before-list is deliberately not opened by the
   prerequisite sweep (`:6068` — it is the first link of a chain and belongs to
   a place), and the village sweep does **not** check `m_iPlayLevel` or an empty
   sub-quest list while the dungeon and field sweeps do (compare `:5978` with
   `:7375`).

   The login sweep runs inside `PushSelectUnitNotifications` *before*
   `EGS_SELECT_UNIT_2_NOT` is built, which is where the server does it too — so
   a quest opened at login arrives as part of the list and needs no packet of
   its own. The other three push one `EGS_NEW_QUEST_NOT` each.

10. **`ADD_SERVER_GROUP` can hide every quest in the game, silently.** It is on
    (`Always.h:1707`), and `CX2QuestManager::GetQuestTemplet` returns NULL for
    any quest whose `m_iServerGroupID` is neither -1 nor
    `g_pInstanceData->GetServerGroupID()`
    ([X2QuestManager.cpp:831](X2Lib/X2QuestManager.cpp#L831)). The offline login
    reports server group 1 and nothing offline sets the client's own idea of it,
    so the two can disagree — and if they do, quests vanish from this code *and*
    from the client's own NPC lists, with no error anywhere.

    **Checked and ruled out.** `CX2OfflineQuest::LogTempletCensus` was added to
    tell this apart from correction 9 in one run, and it reported
    `1395 templet(s) loaded, 1395 visible through the server-group filter
    (client group=1)`. Nothing is hidden: every quest in the script carries
    `m_iServerGroupID` -1 or 1. The census stays in the build anyway - it is one
    line per process, and it converts the single most confusing possible failure
    into a fact.

    The same line established the shape of the auto-open surface, which is worth
    recording because it is much smaller than expected: of 229 epic quests, only
    **8 open in a village, 2 in a dungeon and none in a field**. Every other link
    of the chain opens on a finished prerequisite, so
    `CheckAutoOpen( AOP_BEFORE_QUEST )` after each completion is not a corner
    case - it is the main mechanism.

    **This is the shape phase 5's exit test kept producing** (six rounds, and
    "not one produced an error"): the failure is something that never happened,
    and the only way to see it is a line that says it did not.

11. **A quest that completes the instant it is given is the studio's own
    behaviour, not a bug.** The first play-test after correction 9 reported it:
    quest 11000 'Suspicious Movements' opened on entering the tutorial dungeon
    and completed on arriving in Ruben, 56 milliseconds after its step ticked.
    The packet log shows the `EGS_QUEST_COMPLETE_REQ` coming *from the client* -
    `SERV_ENTER_FIELD_QUEST_CLEAR` makes it hand in visit-a-village quests by
    itself (`CX2State::CompleteQuestToVisitVillageList`,
    [X2State.cpp:11498](X2Lib/X2State.cpp#L11498), and the batch request built at
    [X2QuestManager.cpp:4769](X2Lib/X2QuestManager.cpp#L4769)). 11000's only
    sub-quest *is* "visit Ruben", and the player was walking into Ruben. Nothing
    to fix.

12. **Silence is not an acceptable answer from a sweep that found candidates.**
    After correction 9 the auto-open worked and the chain still did not
    continue, and the logs could not say why: `CheckAutoOpen` was quiet by
    design, so "no epic quest names this place", "the sweep never ran" and
    "eleven matched and all were refused on level" all looked identical. It now
    writes one summary line whenever it had at least one candidate - how many it
    considered, how many opened, and a tally of the refusal codes - and stays
    quiet only when nothing matched at all.

    Two more diagnostics went in with it, both answering questions that
    otherwise produce no packet and therefore no trace:
    `CountWaitingOn( iQuestID )` logs, after every hand-in, how many quests in
    the whole script list it as a prerequisite (a zero says the chain does not
    continue automatically and the next step must be clicked on an NPC); and the
    talk handler logs what the client's own quest manager says that NPC offers,
    which is the only way to see an NPC that offers nothing.

    The census additionally dumps the whole early chain - the four IDs the
    client itself names in `CX2PlayGuide::TUTORIAL_QUEST_ID` - with each quest's
    type, level gates, prerequisites, opening village and dungeon, start scene
    and every sub-quest's clear type. Read out of the client's loaded templets,
    deliberately, and **not** out of `ScriptData/QuestTable.xls`: that
    spreadsheet is in the tree and would be easier, and it is exactly the stale
    snapshot the *`DataBase/` and `ScriptData/`* rule was written about.

13. **The automatic NPC conversation is a quest's start scene, not a separate
    system.** The player reported that on the real client Lowe speaks to them
    immediately after the tutorial, with the NPC portrait dialogue. That is
    `CX2UIQuestNew::Handler_EGS_NEW_QUEST_NOT` calling
    `g_pData->GetEventScene()->PlayEventScene( pQuestTemplet->m_wstrStartScene )`
    ([X2UIQuestNew.cpp:1823](X2Lib/X2UIQuestNew.cpp#L1823)) - so the missing
    conversation is not a missing dialogue feature, it is a missing
    `EGS_NEW_QUEST_NOT`. Worth writing down because the symptom points at the
    scene system and the cause is entirely in quest granting.

14. **The story chain lives in `m_iAfterQuestID`, and the stock client does not
    read it.** This is the finding that made phase 6 actually work, and it took
    three play-tests to reach because every earlier guess was about the wrong
    mechanism.

    The census dump settled it in one run. Printing the four quests the client
    itself names as the tutorial chain
    (`CX2PlayGuide::TUTORIAL_QUEST_ID`) gave:

    ```
    CHAIN 11000 'Suspicious Movements': before=[-] village=[20000] dungeon=[39600..39605]
    CHAIN 11005 '[Field] Thief Pursuit': before=[-] village=[-]     dungeon=[-]
    CHAIN 11010 '[Dungeon] Protect the El': before=[-] village=[-]  dungeon=[30000]
    CHAIN 11030 '[Dungeon] William the Prankster': before=[-] village=[-] dungeon=[-]
    ```

    **11005 has no prerequisite, no opening village and no opening dungeon.**
    Neither the place sweeps nor the prerequisite sweep from correction 9 can
    ever start it, and the same is true of 11030. The only remaining mechanism
    is `KUserQuestManager::CheckEpicQuest_NewQuestByAfterQuest` (`:6102`), which
    opens the quests listed in the *finished* quest's `m_vecAfterQuestID`.

    And that field is the trap. The Lua key is `m_iAfterQuestID` and it holds a
    **table**; the server parses it into a vector
    ([XSLQuestManager.cpp:156](KncWX2Server/Common/X2Data/XSLQuestManager.cpp#L156)),
    while the client declares a bare `int m_iAfterQuestID` on its own
    `QuestTemplet` and **never reads the key at all** - the member is dead
    weight. So the chain was in the script the client had already loaded, and
    only the client's parser was throwing it away.

    Fixed by transcribing the server's loader into
    `CX2QuestManager::AddQuestTemplet_LUA` behind `SERV_IRUHADEV_OFFLINE`, into
    a new `std::vector< int > m_vecAfterQuestID`, and adding `AOP_AFTER_QUEST`
    to `CheckAutoOpen`. Nothing needed repacking: this is the one case where
    data that looked server-only turned out to be sitting in the client's own
    `.kom` already.

    Three details transcribed rather than reinvented:

    - The after-quest sweep does **not** filter on `QT_EPIC`. The place sweeps
      walk the server's epic-only prebuilt lists; this one opens whatever the
      finished quest names, and with `bForce` - which a completion always passes
      - it skips the type check entirely (`:6136-6152`).
    - It runs **before** the prerequisite sweep on completion, and at login it
      runs once per already-completed quest before the prerequisite sweep
      (`SetUnitQuest:358` then `:365`). The login pass is the recovery path: a
      chain link missed for any reason is handed over on the next login rather
      than being lost for good.
    - `> 0` guards each entry, which is what skips the padding zeroes the tables
      are written with.

    **What this run also confirmed about correction 11.** With the fake 11005
    completion gone, Lowe's conversation appeared - that is 11005's
    `m_wstrStartScene` (`'11005_01'`) playing, exactly as correction 13
    predicted. The quest itself still did not start, because the after-quest
    link was missing; the guide arrow to Lake Noah appeared anyway, because the
    play guide is driven client-side and does not wait for a quest instance. Two
    independent systems producing one symptom, which is why the log mattered
    more than the screen.

15. **A cleared dungeon unlocks nothing, twice over.** Reported after the chain
    started working: Banthus dies, the next quest points at Ruben's second
    dungeon, and that dungeon is still locked. Two independent causes, one
    symptom - and the save file proved the clear itself was fine
    (`unit_dungeon` held `(16, 30000, 1, 0)`).

    **(a) The result packet said the player died.** `KDungeonUnitResultInfo::
    m_bIsDie` was computed as `m_kPlayResult.m_fHP <= 0.0f`, and
    `m_kPlayResult` is filled by `EGS_MY_USER_UNIT_INFO_TO_SERVER_REQ` - which
    **arrives after** the result is sent. Measured in the packet log, not
    guessed: `EGS_END_GAME_REQ` at `20:39:57.425`, the play result at
    `20:39:57.438`. So the struct was still `Clear()`ed, `m_fHP` was 0, and
    every successful run reported a death.

    That flag is load-bearing on the client:
    `CX2StateDungeonGame::Handler_EGS_END_GAME_DUNGEON_RESULT_DATA_NOT` records
    the clear only when `m_bIsWin && false == bDieMyUnit`
    ([X2StateDungeonGame.cpp:3005](X2Lib/X2StateDungeonGame.cpp#L3005)) - so
    the client silently skipped `AddClearDungeon`, `CX2Unit::m_mapDungeonClear`
    stayed empty, and `CX2DungeonManager::IsActiveDungeon` refused every
    dungeon gated on this one. The fix is to believe the play result only when
    one actually arrived: absent it there is no evidence of a death, and a run
    that reached `EGS_END_GAME_REQ` with a win is evidence of the opposite.

    **(b) `KUnitInfo::m_mapDungeonClear` was never populated.** Clears have been
    written to `unit_dungeon` since phase 4 and read by nobody. Even with (a)
    fixed the client only knows about dungeons cleared *this session*, so a
    relog re-locks everything - which the exit test ("progression intact across
    restarts") would have caught even if the play-test had not.
    `MakeUnitInfoFromRow` now fills the map from the table.

    **(c) The difficulty belongs in the key.** The client unlocks against
    `m_iDungeonID + m_cDifficulty` and looks prerequisites up by that same
    number; phase 4 stored the base ID alone. It only ever agreed because every
    run so far has been on difficulty 0. Now stored the way the client reads it.

    Schema v6 adds `max_score` and `clear_date` to `unit_dungeon`:
    `KDungeonClearInfo` carries both and the local-map UI draws them, and an
    empty clear-time string would have reached the client's date parsing.

    The pattern is phase 5's again, for the third time in this phase: **nothing
    errored.** A boolean was wrong, the client quietly declined to record
    something, and the consequence surfaced two systems away as a locked door.

16. **A step whose trigger already fired never fires again.** Reported as: the
    previous quest completes, the next one wants a village or field you are
    *already standing in*, and it sits unticked until you walk out and back in.

    The visit check existed, and ran in two places: `Handler_EGS_NEW_QUEST_REQ`
    (the player clicked a quest on an NPC) and `QuestAutoOpen` (a sweep opened
    an epic quest). The second one only ever checked **the place that triggered
    the sweep**. That is fine for the three place sweeps, whose trigger *is* a
    place - and wrong for the two that carry the story. `AOP_AFTER_QUEST` and
    `AOP_BEFORE_QUEST` open a quest wherever the player happened to hand the
    last one in, and got no visit check at all.

    The real server has none of this trouble because it re-runs the check off
    the back of every accept, against wherever the character actually is
    ([GSUserGameCommon.cpp:1608-1649](KncWX2Server/GameServer/GSUserGameCommon.cpp#L1608)).
    Both paths now call one `QuestCheckHereAndNow`, which asks the same
    dungeon / field / village question every other handler in the file asks.

    **The second half was group staging, and it is the part that would have
    come back.** A quest's sub-quests are grouped, and `CheckBeforeGroup`
    refuses a step while any earlier group is outstanding. So "talk to Lowe,
    then go to Elder" refuses the Elder step on the first pass; finishing the
    talk is what makes it eligible, and by then the visit has already happened.
    Same bug, one level down, and not fixed by checking on accept. So
    `QuestCheckHereAndNow` runs three passes rather than one - bounded, not
    `while( changed )` - and is also called after an NPC talk and after a
    hand-in, the two events that finish a group. The same shape as
    `CX2OfflineTitle::ReCheckPassive`, for the same reason.

17. **The feature-use sub-quests were never wired at all.** Reported as: the
    quest says press K and spend a skill point, you spend one, nothing happens.

    `SQT_USE_SKILL_POINT` is ticked on live from `DBE_INSERT_SKILL_ACK` - the
    *database reply* handler, three levels of nesting in
    ([GSUserGameCommon.cpp:4639](KncWX2Server/GameServer/GSUserGameCommon.cpp#L4639)).
    Offline there is no DB round trip, so that handler does not exist, and the
    port of `EGS_GET_SKILL_REQ` carried the feature without the quest tick that
    was buried inside it. That is the general shape of this whole group: each
    one lives in the packet handler for its own feature, none of them look like
    quest code, and a port that goes feature by feature loses all of them.

    Wired this round, with the driver each one actually has:

    | Type | Driver | Note |
    |---|---|---|
    | `SQT_USE_SKILL_POINT` (18) | `EGS_GET_SKILL_REQ` | once per **point**, so 1→3 on one skill counts twice |
    | `SQT_LEARN_NEW_SKILL` (36) | `EGS_GET_SKILL_REQ` | once per **skill**, from the request's own `m_vecNowLearnSkill` |
    | `SQT_SKILL_USE` (23) | **new** `EGS_SKILL_USE_REQ` handler | the client was already sending it; nothing was listening |
    | `SQT_ITEM_USE` (22) | `EGS_USE_ITEM_IN_INVENTORY_REQ`, `EGS_USE_QUICK_SLOT_REQ` | two call sites on live too |
    | `SQT_DUNGEON_DAMAGE` (7) | dungeon clear | `m_iDungeonDamage` is a **ceiling**, not a floor |

    Two of these read almost identically and mean the opposite thing, in the
    same server file a hundred lines apart: an empty `m_setSkillID` means "any
    skill" for `SQT_SKILL_USE` and "can never complete" for
    `SQT_LEARN_NEW_SKILL`. Neither is safe to infer from the other.

    **What is deliberately still not driven, and why.** A census line now
    reports, per clear type, how many reachable steps use it and - when nothing
    can advance it - one sentence saying what is missing. The reasons are not
    all "not implemented yet":

    - `SQT_ITEM_ENCHANT`, `SQT_ITEM_SOCKET` - both features are *refused*
      offline with the item left intact, because the success-rate and
      socket-option tables are server data with no client copy. A driver would
      be dead code.
    - `SQT_ITEM_ATTRIB` - the attribute system is compiled out of this build.
    - `SQT_SUITABLE_LEVEL_DUNGEON_CLEAR` - "was this cleared at an appropriate
      level" arrives at the GameServer *already decided*, as
      `m_mapSuitableLevelInfo` on the room server's end-of-game packet. Nothing
      in the client sends or receives that field. "Am I inside the dungeon's
      level band" is a different rule that would agree sometimes and drift the
      rest of the time.
    - `SQT_ITEM_EQUIP_DUNGEON_CLEAR` - **the shipped server's own check is
      unreachable code.** It looks the quest up in `mapOngoingQuest` and then
      dereferences the iterator on the `== end()` branch
      ([UserQuestManager.cpp:2234](KncWX2Server/GameServer/UserQuestManager.cpp#L2234)).
      There is no working behaviour to copy.
    - `SQT_FEED_PET`, `SQT_USER_DIE`, `SQT_HYPER_MODE_USE`, `SQT_PVP_TAG_COUNT` -
      **the shipped GameServer never ticks these either.** `XSLQuestManager`
      parses the first two with empty case bodies and a pair of TODO comments
      ([XSLQuestManager.cpp:1206](KncWX2Server/Common/X2Data/XSLQuestManager.cpp#L1206))
      and nothing anywhere moves them. "Implement the missing driver" is the
      wrong instinct: there is nothing to port.
    - every PvP type - there is no PvP offline.

    Worth stating plainly because it changes what the census means: a type in
    that list is not a TODO. Four of them are notes about the original game.

    Two incidental repairs made on the way through:

    - **`X2OfflineQuest.cpp` contained a literal NUL byte.** A `L'\0'` in the
      census formatter had been written as `L'` + an actual 0x00 + `'`. MSVC
      accepted it, so it built and ran; `file` called the .cpp `data`, `grep`
      called it a binary, and `sed` rendered the byte as a space - which is how
      an anchored patch came to look correct and match nothing.
    - **A symbol present in a header is not a declaration.** The skill-use
      dungeon exclusions were transcribed from `CXSLDungeon`'s predicates and
      checked with `grep -c` against `X2Lib/X2Dungeon.h`; every name "existed".
      Six of them were **commented out** of the client's enum, and the compiler
      rejected them one at a time. This client's `DUNGEON_ID` is a much shorter
      list than the server's - one Henir space rather than six, one tutorial
      rather than fourteen - so a dungeon the server excludes may simply not be
      enterable here. Check for `\bNAME\b\s*=` on a line that is not commented,
      not for the name.

18. **The HUD portrait after a quest job change — a gap in the shipped client,
    not in the emulation.** Reported as: the class really does change and
    everything says so, but the character image top-left keeps showing the old
    class until the character is re-selected.

    Worth writing down mainly for how it was decided, because the reflex was to
    go looking for a packet I had failed to send. The GameServer sends **only**
    `EGS_QUEST_COMPLETE_ACK` for a quest-driven class change - `SetUnitClass` /
    `ResetStat` and then the same ACK every other completion gets
    ([UserQuestManager.cpp:5038-5142](KncWX2Server/GameServer/UserQuestManager.cpp#L5038)).
    There is no second notification to have missed. The whole class-change UI is
    client-side, off that one ACK.

    And that client code is incomplete.
    `CX2UIQuestNew::Handler_EGS_QUEST_COMPLETE_ACK` resets the skill tree, shows
    the popup, and calls `ResetUnitViewerInFieldSquare` - which rebuilds the 3D
    square unit and nothing else
    ([X2State.cpp](X2Lib/X2State.cpp), `ResetUnitViewerInFieldSquare`). The
    top-left portrait is a different thing entirely: it belongs to
    `CX2MyGageUI`, whose class is cached in `m_eOwnerGameUnitClass` and only set
    by `CreateMyGageUI` / `SetCharacterImage`. The gage is created per state -
    `InitMyGageWhenMovingToVillage` - which is exactly why re-selecting the
    character fixed it.

    Every *other* class-change path in the client does refresh it:

    | Path | Refreshes the portrait |
    |---|---|
    | `EGS_CHANGE_MY_UNIT_INFO_NOT` ([X2State.cpp:12958](X2Lib/X2State.cpp#L12958)) | yes |
    | admin change, `EGS_ADMIN_CHANGE_UNIT_CLASS_ACK` (:5083) | yes |
    | jumping character, `EGS_JUMPING_CHARACTER_ACK` (:13174) | yes - the comment there is literally "refresh the character portrait" |
    | cash item, `EGS_CHANGE_JOB_CASH_ITEM_NOT` (:8388) | yes, in `XS_BATTLE_FIELD` |
    | **quest completion** | **no** |

    So the fix is a client change behind `SERV_IRUHADEV_JOBCHANGE_PORTRAIT`
    (`Always.h`), applied at **both** live completion handlers -
    `Handler_EGS_QUEST_COMPLETE_ACK` and
    `Handler_EGS_ALL_COMPLETED_QUEST_COMPLETE_ACK`, which carry the same
    `QT_CHANGE_JOB` block and the same omission. The third copy of that block is
    inside `#else SERV_DAILY_QUEST` and does not compile; it was left alone.

    The guard is `GetMyGageData()`, not `GetInstance()` alone:
    `CX2GageManager::SetCharacterImage` dereferences `m_ptrMyGageSet` with no
    null check, unlike nearly every neighbour in that header, and there is no
    gage at all in some states.

    Two process notes:

    - **`m_ucClearData`-style substring counting bites in patch anchors too.**
      The first attempt located the two call sites by counting
      `b"\t\t\t\t" + call`, which reported three - because four tabs is a prefix
      of the five-tab site. Anchor a byte patch on `\n` + the indentation, never
      on the indentation alone.
    - `X2UIQuestNew.cpp` is CP949. Verified after the edit: `file` still reports
      `ISO-8859 text` and `git diff --stat` shows 68 insertions and 0 deletions.

    **The test jig this was found with, and why it is gone.** Walking a level-1
    character to a job-advancement quest takes hours, so the class change was
    tested with a temporary skip: a marker file `offline_grant.txt` next to the
    executable, read at character login, granting either the character's next
    `QT_CHANGE_JOB` quest or a named quest ID with every step already satisfied.

    It had to live **inside the client**, which is the part worth remembering.
    The quest script is in `data036.kom`, so nothing outside a running client
    knows which quest advances which class; writing rows into `unit_quest` from
    a SQL script would have meant taking the IDs from
    `ScriptData/QuestTable.xls`, the stale snapshot that has already produced
    two confidently wrong answers in this project. Reading the templets from the
    process that already has them loaded was the only source that could not be
    wrong.

    Removed once the test passed - implementations, declarations and the login
    hook - so nothing in the shipped path reads a marker file or can grant a
    quest that was never earned. If it is needed again, it was four pieces:
    `SubQuestTargetCount`, `LogJobChangeQuests`, `FindJobChangeQuest` and
    `ForceGrantComplete` on `CX2OfflineQuest`, plus a block in
    `Handlers_Unit.cpp` immediately before `GetQuestInstances` fills
    `EGS_SELECT_UNIT_2_NOT`. Two details that were not obvious and would have to
    be rediscovered: a collection step cannot be forced with a flag because the
    bag *is* the state on both sides, so the items have to actually be inserted;
    and the grant has to run before the packet is built, or the quest does not
    appear until the next login.

### Packets the plan did not predict

Answered, with the rules ported from `KUserQuestManager` / `KUserTitleManager`:

```
EGS_NEW_QUEST_REQ/ACK + _NOT             accepting, and the quest it hands back
EGS_QUEST_COMPLETE_REQ/ACK               handing in, the reward, the class change
EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ/ACK the "hand in everything" button
EGS_GIVE_UP_QUEST_REQ + ACK              abandoning one
EGS_GATHER_GIVE_UP_QUEST_REQ             abandoning several
EGS_UPDATE_QUEST_REQ/ACK                 dead on both sides; answered anyway
EGS_UPDATE_QUEST_NOT                     every progress change
EGS_EQUIP_TITLE_REQ/ACK                  wearing a title
EGS_NEW_MISSION_NOT                      a title mission opening
EGS_UPDATE_MISSION_NOT                   its progress
EGS_REWARD_TITLE_NOT                     the title it pays
```

Deliberately not implemented, and they will show as `UNHANDLED` if reached:
`EGS_ADMIN_QUEST_COMPLETE_REQ` (admin cheat), `EGS_CHANGE_RANDOM_QUEST_NOT` and
`EGS_EVENT_QUEST_INFO_NOT` (both are *pushes* the offline server would have to
originate, and both need a live event schedule it does not have), and
`EGS_TITLE_EXPIRATION_NOT` (nothing offline issues a rented title).

### What was actually built, against *Code layout to create*

New, and the plan predicted one of the three:

```
X2Lib/Offline/Handlers_Quest.cpp         predicted
X2Lib/Offline/X2OfflineQuest.h/.cpp      KUserQuestManager, reduced to one character
X2Lib/Offline/X2OfflineTitle.h/.cpp      KUserTitleManager, likewise
```

Extended:

```
X2OfflineDB          schema v5, quest / sub-quest / completed-quest CRUD,
                     missions, titles, SaveUnitClass, SaveEquippedTitle
X2OfflineInventory   CountItemByID and ConsumeByID - the bag is the state for
                     every collection sub-quest, on both sides
X2OfflineServer      m_iTitleID in KUnitInfo, and the event bridges the other
                     handler files call
Handlers_Unit        _2_NOT from unit_quest, _3_NOT from unit_mission/unit_title
Handlers_Room        talk, kill, spawn, dungeon-clear, level-up and pickup hooks,
                     plus quest-item drops inside PushNpcDrop
Handlers_Field       the village-entry hook
Handlers_Inventory   the use-item and buy hooks
```

One client-side edit, behind the existing `SERV_IRUHADEV_OFFLINE`: a read-only
`GetMapTitleMission()` accessor on `CX2TitleManager`. `GetMissionInfo` can only
answer for an ID that is already known, and deciding *which missions a character
has become eligible for* means walking the whole templet map.

Schema v5 is additive (five new tables, one `ALTER TABLE` on `unit`) and v6 adds
two more columns to `unit_dungeon`, so a v4 save upgrades rather than being wiped. The migration and all eighteen new
statements were dry-run against a copy of the live `els_db.sql` before the build
was deployed, including the `UNIT_COLUMNS` select and the final-delete sweep
over all ten child tables.

### Decisions made while implementing phase 6

1. **The completion ACK goes out before anything the completion sets off.** The
   client holds an `AddServerPacket` wait on `EGS_QUEST_COMPLETE_ACK`, and its
   `_NOT` handlers run against a character it has not been told about yet - so
   the level-up effect, the quest updates and the title work are stashed in
   `KQuestAfter` and replayed by `AfterQuestComplete` once the ACK is on the
   wire. The batch hand-in stashes one per quest and replays them all after its
   single ACK. This is the shape the live pair of servers ends up with, where
   the reward is paid in a DB reply handler that sends the ACK first.

2. **Collected items are handed in BEFORE the room check, and rolled back if the
   reward will not fit.** Checking for room first refuses a perfectly valid
   completion whenever the bag is full - which is exactly when a quest that
   takes ten items and gives one is most likely to be handed in. The real server
   has no such problem because `KInventory::DeleteAndInsert` does both halves in
   one call. Putting the items back cannot fail: the slots they came out of are
   still free.

3. **A collection sub-quest has no counter at all - the bag is the state.** That
   is not a simplification, it is what the client does:
   `CX2QuestManager::SubQuestInst::IsComplete`
   ([X2QuestManager.cpp:2716](X2Lib/X2QuestManager.cpp#L2716)) answers
   `SQT_ITEM_COLLECTION` by calling `GetNumItemByTID` on its own inventory.
   `CX2OfflineInventory::CountItemByID` mirrors that call's exclusions exactly -
   worn gear, the quick-slot bar and both banks do not count - so the two can
   never disagree about a quest the player is holding items for.

4. **Quest items are rolled per active quest, not out of the drop table.** On
   live they come from `KRoomUser::GetQuestDropItemInDungeon`, which is why the
   same monster drops a quest item for one player and nothing for another. The
   four live multipliers (comeback user, Gaia server, two events) are all 1.0
   here - the same simplification phases 4 and 5 made for EXP, ED and the
   ordinary drop table - and the roll stops once the bag holds enough.

5. **A dungeon's rank is reported as `RT_NONE`, so rank sub-quests do not tick.**
   The offline server does not compute a rank: the thresholds are server-side
   Lua this project does not reproduce, which is the same reason phase 4 left
   the result screen's rank bonus out. The divergence is one-directional and
   visible - a sub-quest asking for rank D or better simply will not complete -
   whereas passing an invented rank would hand out a reward the run did not
   earn.

6. **Timed-event quests and daily random quests are allowed rather than
   refused.** Both are gated on live against a table the GameServer reseeds (the
   event schedule, and the day's random rotation); neither table exists offline
   and neither can be derived from client data. Refusing them would make every
   event quest permanently untakeable, which is a bigger divergence than
   allowing one, and each is logged by name when it is allowed through.

7. **`Reward::m_iSP` is paid as skill points.** The live server routes it to
   `AddAPoint` under `SERV_PVP_NEW_SYSTEM`, which is an arena point; that system
   does not exist offline and PvP is out of scope, so it is paid as SP - the
   meaning the field has in the client's own `Reward` struct and in the pre-PvP
   server code.

8. **A class change seeds the *whole* default skill list for the new class.**
   The live path under `SERV_UPGRADE_SKILL_SYSTEM_2013` grants only the two
   defaults the advancement unlocks, read out of `GetUnitClassDefaultSkill`.
   `SeedDefaultSkills` grants the class's whole list, which is the same set: the
   base skills are shared down the class tree, and re-upserting one at level 1
   is a no-op for anything already learned higher.

9. **A quest whose templet is missing is left on disk, not deleted.** The row
   stays in `unit_quest`, out of the in-memory map, with a log line naming it -
   so a build that has the templet again picks the quest back up. Deleting it
   would destroy a save to tidy up an in-memory structure.

10. **A quest with no sub-quest groups is treated as a flat checklist.** The
    real server returns false from `CheckCompleteSubQuest_BeforGroup` when it
    cannot find a group, which is safe for it because every shipped quest is
    grouped. Offline the same script is read, so the case should not arise;
    treating it as "no staging" rather than "never advances" means a quest that
    somehow lacks groups is playable instead of permanently stuck, and the log
    says which one it was.

11. **The quest-11005 dungeon-menu unlock from phase 2 is REMOVED.**
    *This reverses the decision this entry originally recorded, which was to
    keep it behind a guard. The guard made it worse, not better.*

    Phase 2 reported quest 11005 as completed in `EGS_SELECT_UNIT_2_NOT`, purely
    so `CX2QuestManager::SetUnitQuest` would call `SetShowDungeonMenu( true )`
    and stop the village party dialog from hiding the dungeon button
    ([X2QuestManager.cpp:601](X2Lib/X2QuestManager.cpp#L601)). With no quest
    system that was honest and it worked.

    With phase 6 it is actively harmful, and the guard aimed it at exactly the
    wrong character. **11005 is the second story quest** - `TQI_CHASE_THIEF` in
    the client's own `CX2PlayGuide::TUTORIAL_QUEST_ID`, the one Lowe gives on
    arriving in Ruben - so telling the client it is already finished means Lowe
    has nothing to offer, its `m_wstrStartScene` (the automatic conversation the
    player expects) never plays, and every quest chained behind 11005 is stuck
    with the client believing it is done while the save says it is not. The
    guard fired precisely when the real completion count was zero, i.e. for the
    new character whose story it then broke.

    The button is now earned the way the real game grants it. Nothing is faked,
    and the two sides agree about where the player is in the story.

12. **Titles get the full mission engine rather than an empty `_3_NOT`.** The
    phase bullet asks only that titles and missions reach
    `EGS_SELECT_UNIT_3_NOT`, and persistence alone would satisfy it literally -
    but a title system that can never award a title leaves that packet
    permanently empty, which is indistinguishable from the phase-2 stub. The
    mission loop is the quest loop with different field names, so it rides on
    the same events. What is *not* modelled, because each needs a subsystem that
    does not exist offline: every PvP clear type, pet feeding, party-size
    conditions, resurrection-stone counts and "die a certain way". Missions
    using those are still tracked and still shown; they simply never advance.

13. **`CheckCompletable` returns `ERR_QUEST_12` where the server falls through
    with `NET_OK`.** The real handler's epic-quest level check is a bare
    `goto error_proc` that leaves the error at `NET_OK`, so the client is told
    "OK" and given nothing - a hang rather than a message. `ERR_QUEST_12` is the
    level-too-low code the accept path uses and puts a readable reason on
    screen. This is the one place phase 6 deliberately does not transcribe the
    server.

### Operational notes, extending the earlier phases'

- **`touch X2Lib/stdafx.cpp` after editing any header inside the PCH.** See
  correction 8 - the failure looks like the edit did not happen, and a `#error`
  probe placed next to the edit does not fire either, because the probe is in
  the stale region too.
- **`grep -rn "SendPacket( EGS_" X2Lib/*.cpp` filtered to QUEST / TITLE /
  MISSION is the phase-6 checklist**, the same way `AddServerPacket` was phase
  5's. It is what found `EGS_GATHER_GIVE_UP_QUEST_REQ` and
  `EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ` before a play-test hit them, and what
  established that `EGS_UPDATE_QUEST_REQ` has no sender at all.
- **Three quest UIs dispatch the same packets and only one is built.**
  `CX2UIQuestNew`, `CX2UIQuestReceive` and `CX2UIQuest` are all wired into
  `CX2UIManager::UIServerEventProc`; which one exists decides who handles
  `EGS_NEW_QUEST_NOT`. It does not matter for correctness - all three drive the
  same `CX2QuestManager` - but reading the wrong one wastes time.

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

### Exit test — RUN, PASSED with two defects found (2026-09-04)

Every menu opens, nothing hangs, and `offline_packets.log` came back with **zero
`UNHANDLED` entries** across a full session. Cash-shop purchases work and
persist: buy, deposit, claim, relog, item still there.

Two defects the play-test found, both fixed and both written up in the addendum
below: costumes could be bought but never claimed ("select the item socket
option"), and the catalog was being built from the wrong file.

---

### 7.1 What the cash shop turned out to be

**The plan's "catalog from the client's own cash-shop script data" was wrong,
and the correction is the most important thing in this phase.** There is no
client-side catalog. `CashShopItemList.lua` only marks which items get a NEW /
HOT / RECOMMEND badge; `CashShopCategory.lua` only maps a tab to a category
number. The catalog itself is `KBillProductInfo` rows out of a **separate
billing database** behind the LoginServer, which this tree does not contain and
which is not among the two databases (`Account`, `Game01`) the live-DB rule
covers.

What this tree *does* contain is `KncWX2Server/ServerResource/US/CashItemPrice.lua`
— 854KB, 10,224 `AddCashItemPrice( itemID, price )` lines, the real US cash
price of every cash item, loaded by the GameServer's own
`CXSLCashItemManager`. So the catalog is that file, and it is the user's to pack,
exactly like `StatTable.lua` and `DropTable.lua` before it. `CX2OfflineCashShop`
is a transcription of `CXSLCashItemManager`'s three Lua entry points, registered
under the same global name (`g_pCashItemManager`) the script calls.

Three fields of `KBillProductInfo` have no source anywhere and get a **stated
default rather than an invented value**:

| Field | Offline | Why not a number |
|---|---|---|
| `m_cPeriod` | 0 (permanent) | Rental lengths lived only in the billing DB. 0 is not a guess at the real length — it is a *different offer*, and the shop prints it: with `m_cPeriod` 0 the client's own `GetPeriod()` renders the quantity instead of a day count, so nobody is shown a made-up "30 days". |
| `m_cQuantity` | 1 | One purchase, one item. |
| `m_cCategoryNo` | derived | It decides only which tab an item appears under. `CategoryFor()` maps the item's own templet — its fashion flag, type and equip position — onto the `CSSC_*` range. That is a presentation mapping over client data, not game data being invented. |

`m_iProductNo` is the item ID: a surrogate key (the live product numbers are
billing-DB identities with no meaning here) chosen because it is stable across
runs, which matters because a deposit row stores it.

### 7.2 Corrections to this plan, found by doing it

1. **The plan's "solo party of one" is the wrong shape; no party is right.**
   The client has no party until an `EGS_REGROUP_PARTY_NOT` says so, and every
   button in the party UI then acts on members that do not exist. A player alone
   on live has no party either, and the whole phase-4 dungeon flow already runs
   without one. So each party packet answers `ERR_PARTY_09` ("no party") and the
   party window stays the empty one the client starts with.

2. **`EGS_CHECK_BALANCE_ACK` was silently broken since phase 1, and nothing had
   noticed.** It filled `m_ulBalance`, which under `SERV_SUPPORT_SEVERAL_CASH_TYPES`
   — on for every region — the client never reads: its handler routes
   `m_bOnlyType == false` to `SetGlobalCash( m_GlobalCashInfo )` and ignores the
   scalar entirely ([X2State.cpp:5645](X2Lib/X2State.cpp#L5645)). The wallet
   would have shown zero however much the save file said. Fixed by filling
   `m_GlobalCashInfo.m_ulCash[GCT_PUBLISHER_CASH]`, which is the slot the US
   client's own buy popup defaults to ([X2CashShop.cpp:11170](X2Lib/X2CashShop.cpp#L11170)).
   This is the phase-0 "a flag set on one project and not another silently does
   nothing" failure in a new costume: a field written and never read.

3. **Chat is not a stub, and treating it as one would have broken it.** The
   plan's table says "echo locally, no broadcast". The client does not echo
   locally — `CX2ChatBox::Handler_EGS_CHAT_REQ` hands the packet over and stops;
   what puts the line on screen is `EGS_CHAT_NOT` coming back. Drop it and the
   chat box swallows everything the player types. The offline server broadcasts
   it back to the one player.

4. **Eighteen handlers were written and then deleted because their packet
   structs do not exist in `US_SERVICE`.** The whole local-ranking system
   (`SERV_LOCAL_RANKING` off — so the plan's "Ranking / profile →
   `X2ProfileManager.cpp:185`" row is for a system that is not in this build),
   the whole relationship/marriage system, the spirit-reward ladder, the
   jumping-character event, the `.kom` integrity report, both halves of the
   returning-player retention offer, both recruit-a-friend packets, and the
   VER2 lag check. **The textual `SendPacket`/`AddServerPacket` scan cannot see
   `#ifdef`s, and neither can the plan's table** — the compiler is the only
   honest filter, and the fastest way to use it is to write the handler and let
   the build delete it.

5. **The catalog loader must not go through `CX2OfflineInventory::Templet`.**
   That helper logs a warning line per unresolvable item, which is right when a
   *save file* names an item the client does not ship and very wrong over ten
   thousand script rows in one go. `AddCashItemPrice_LUA` calls
   `GetItemTemplet` directly and reports one summary count.

6. **A cash purchase lands in the deposit, not the bag** — `cash_order` is a
   real table, not a shortcut. The client has a whole window for the deposit and
   asks for it by name (`EGS_BILL_INVENTORY_INQUIRY_REQ`); skipping it would
   leave that window permanently empty and its "get item" button inert.

### Packets the plan did not predict

Answered — 91 new handlers across the two files:

```
cash shop   BILL_PRODUCT_INFO, GET/MODIFY_WISH_LIST, BILL_INVENTORY_INQUIRY,
            BUY_CASH_ITEM, BILL_GET_PURCHASED_CASH_ITEM,
            GET_PURCHASED_PACKAGE_CASH_ITEM, PRESENT_CASH_ITEM,
            VISIT_CASH_SHOP_NOT, CHECK_PRESENT_CASH_INVENTORY_NOT, APPLY_COUPON
guild       21 packets - creation, the three boards, applications, invitations,
            member management, guild skills
party       13 - invite, leave, the six change-* packets, game start, PvP and
            auto-party matchmaking
friends     24 - the community list, the eight friend verbs, friend groups, the
            messenger serial, community options, unit search and watch
mail        8  - the letter list, read/delete/send, attachments, the black list
chat        5  - CHAT_REQ (a real echo), the option write, the black list,
            the megaphone
ranking     1  - GET_RANKING_INFO (Henir / dungeon / PvP boards, all empty)
tutor       3, bank 1, temp inventory 2, trade board 1, checksum 1
pets        13 - the two lists out of SQLite, hatching, summon, feed, commands,
            evolution, rename, auto-feed, and the four riding-pet packets
menu        6  - the two channel-change packets, the warp button, the 2013 event
            mission, the skill-note memo, the random box
security    6  - the second-password pad
```

Deliberately not implemented, and they will show as `UNHANDLED` if reached:
every `EGS_ADMIN_*` cheat (auth-gated, and the auth level is never raised
offline), the personal shop and personal trade, PvP rooms and lobbies, the
training school, the item crafting family phase 5 already refuses (manufacture,
synthesis, resolve, identify, seal, evaluate, convert), and the pre-global
billing packets (`EGS_CASH_PRODUCT_INFO_REQ`, `EGS_GET_PURCHASED_CASH_ITEM_REQ`,
`EGS_PURCHASED_CASH_ITEM_LIST_REQ`) that `SERV_GLOBAL_BILLING` replaces.

The blocking-REQ checklist went from **209 unhandled to 110**, with the dispatch
table at 203 cases.

### What was actually built, against *Code layout to create*

```
X2Lib/Offline/Handlers_Shop.cpp        predicted
X2Lib/Offline/Handlers_Social.cpp      NEW - the plan folded these into Handlers_Stub
X2Lib/Offline/X2OfflineCashShop.h/.cpp NEW - CXSLCashItemManager, reduced to a catalog
```

Extended:

```
X2OfflineDB        schema v7: cash_order, wish_list, settings, unit_pet,
                   unit_riding_pet, plus the wallet accessors. account.cash_balance
                   has been in the schema since v1 and was never written until now.
Handlers_Login     EGS_CHECK_BALANCE_REQ now pays the real wallet - see correction 2
Handlers_Stub      the two pet-list handlers moved out, now that pets have a table
```

No client-side edit was needed this phase — no new `SERV_IRUHADEV_` flag, and
`SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` already hides the quick-slot expansion
ticket the plan asked about (`CX2CashShop::GetCahBuyExpandQuickSlotItem` returns
false under it, [X2CashShop.cpp:8306](X2Lib/X2CashShop.cpp#L8306)).

Schema v7 is additive — five new tables and no `ALTER TABLE` at all — and its
migration also tops up an existing account's zero balance from the new
`settings.cash_start` row, or a v6 save could open the shop and never afford
anything. The migration and all sixteen new statements were dry-run against a
copy of the live `els_db.sql` before the build was deployed: v6 → v7, the
existing account `shadow_x` went 0 → 100000, and every insert/update/delete the
handlers use round-tripped.

### Decisions made while implementing phase 7

1. **A list fetch answers with a well-formed EMPTY list; an action that needs
   somebody else answers with a refusal that is TRUE.** That is the whole rule
   the social half follows. An empty guild list is what a player with no guild
   sees on live; an error box is not, and silence is a spinner that never stops.
   And when a friend request fails, the honest reason is "that nickname does not
   exist" (`ERR_MESSENGER_04`) — because the only character in the world is the
   one sending it — not a generic failure.

2. **A refusal never consumes the item it was given.** Guild creation, the guild
   skill reset, the megaphone, the random box, the pet egg and the mount ticket
   all name an item by UID that a *success* would destroy. Every one of those
   handlers refuses before touching the inventory, and says so in the log. The
   one place an item IS consumed is the pet rename, which succeeds.

3. **Five cash items are refused at purchase rather than sold and then broken.**
   The six inventory-expansion tickets, their six event twins, and the
   resurrection stone (127030) all exist to cause a side effect the offline
   server does not model — `m_mapExpandedCategorySlot` is always empty and the
   resurrection count has been 0 since phase 4. Selling one would take the cash
   and hand back an item that does nothing. Every ID in that list is a named
   constant in the client's own `X2Define.h` or is named in the client's own
   purchase handler; none is guessed.

4. **The claim path finds the deposit line, checks for room, deletes, then
   inserts — in that order.** Checking room first means a full bag leaves the
   line in the deposit to be claimed later; deleting before inserting means a
   `DeleteCashOrder` that comes back false (the line was not there) cannot mint
   an item out of nothing. `DeleteCashOrder` returns false on zero
   `sqlite3_changes` precisely so the caller can tell those apart.

5. **One deposit line per ordered unit, not one line carrying a count.**
   `KBillOrderInfo` has no quantity of its own — the count it shows comes from
   its embedded product's `m_cQuantity` — so ordering two of something has to be
   two rows or the second is lost.

6. **The second-password pad authenticates whatever is typed and refuses to set
   a new one.** Authenticating is right: there is nothing to protect (the
   account is a row in a local file the player owns) and refusing would lock
   them out of their own save. Refusing to *set* one is right for the same
   reason accepting would be wrong — it would tell the player they have a PIN
   that is not stored anywhere, and the next login would silently not ask.

7. **The shared bank opens empty with a size of zero rather than opening onto
   slots.** `PRIVATE_BANK` is on so the button is there, but nothing offline
   stores a bank and `CX2OfflineInventory` has no `ST_BANK` category behind it —
   a non-zero size would give the player slots that swallow items.

8. **The catalog is sorted by item ID and paged at 400 products.** Sorting is
   for reproducibility: the same product lands on the same page every run, so a
   log line naming a page means something. 400 is a packet-size choice — ten
   thousand products at ~50 bytes each would be one 500KB event.

9. **Guild-board reads succeed where guild-board writes fail.** Opening the
   guild skill board answers `NET_OK` with an empty `KGuildSkillInfo`, because a
   failure pops a dialog on a window the player only opened to look at; joining,
   inviting or renaming answers `ERR_GUILD_19` ("not in a guild"). Same split in
   the friends list: deleting or denying something that is not there *succeeds*,
   because the list ends in the state the player asked for.

### Operational notes, extending the earlier phases'

- **The `cl /P` flag probe from phase 1 is worth keeping as a script.** A probe
  TU that is just `#define _ALWAYS_` + `Always.h` + `AlwaysButConditionally.h` +
  `ServerDefine.h` and one `#pragma message` per flag, compiled with `/Zs` and
  the `US_SERVICE` defines, answers thirty flag questions in one second. Use
  `/Zs`, **not** `/EP` — with `/EP` the pragmas are passed through to the output
  instead of being executed, and nothing prints.
- **The VC toolchain is on `D:`, not under either `Program Files`.**
  `C:\Program Files (x86)\Microsoft Visual Studio 10.0` has no `VC` directory at
  all; `where cl` finds nothing until `Common7\Tools\vsvars32.bat` has been run,
  which points at `D:\Program Files\VS\Microsoft Visual Studio 10.0\VC\bin\`.
  Anything invoking `cl` directly has to go through that .bat.
- **Dry-run a migration by extracting the SQL from the `.cpp` rather than
  retyping it.** `re.findall(r'"([^"]*)"', ...)` over the `SCHEMA_V*` block gives
  the exact string the client will execute; a retyped copy tests a different
  statement than the one that ships.
- **`grep -rn "SendPacket( EGS_"` + `AddServerPacket` is still the checklist,
  and it over-reports.** It found all 91 of this phase's packets — and 18 more
  whose structs are compiled out. Treat its output as a candidate list, not a
  work list.

### Addendum (2026-09-04): the catalog was coming from the wrong file

Shipped, play-tested, and then corrected. Three findings, in the order they
turned up.

**1. The costume socket bug.** Buying a costume and sending it to the inventory
failed with "select the item socket option", and the attribute dropdown never
appeared. `MakeOrderInfo` was sending `m_vecSocketOption` empty, and that field
is not a list of socket IDs - it is a list of socket *group* IDs, which both
halves of the client expand through
`CX2SocketItem::GetSocketIdListForCashAvatar`: the combo box to draw the
options, and `GetSelectedOptionList` to turn the pick back into a socket. Empty
list, no combo, no selection, and
`GetSelectedOptionListAndGroupID` returns false for anything
`GetIsPossibleSocketItemByOnlyItemType` says can hold a socket - so a costume
could be bought and never claimed.

Fixed with the client's own fallback numbers, 2000 for a weapon and 1000 for
defence ([X2SocketItem.cpp:2214-2231](X2Lib/X2SocketItem.cpp#L2214)), which is
what it uses for a group ID it does not recognise. The claim now reads
`m_mapSocketForCash`, validates the pick back through the same function, and
passes it into `InsertItem` so it persists on the item row.

The first cut of that fix validated the socket *after* `DeleteCashOrder`, which
would have destroyed the deposit line on a rejected pick. Order is now room ->
socket -> delete -> insert.

**2. `m_cCategoryNo` is the billing category, not the client enum.** From
`CX2CashShop::GetItemByCategory`:

```cpp
if ( (int)subCateID.x == (int)subCategoryID )   // x = CSSC_* client enum
    if ( m_cCategoryNo == (int)subCateID.y )    // y = the REAL billing number
```

`CategoryFor()` had been returning the `CSSC_*` enum - a different number space
entirely. Worse, the field is a **signed char**, so the values it returned for
`IT_SPECIAL` and the default case (`CSSC_INSTALL_ETC` 152, `CSSC_PET_PET` 200)
wrapped to -104 and -56 and could never match anything. Those tabs were empty
and nobody had noticed.

**3. The catalog itself came from the wrong file.** The plan said "catalog from
the client's own cash-shop script data"; phase 7 corrected that to
`CashItemPrice.lua` on the grounds that it was the only price list in the tree.
Both were wrong. `CashItemPrice.lua` is the GameServer's **item-resolve** price
lookup - its only two readers are `Inventory.cpp:11939` and `:18992`, the
decompose value - and it prices 9,947 items where the shop sold 2,342. Built
from it, the shop showed roughly 3,000 items that were never purchasable.

The real catalog is `dbo.EB_Product` in the **billing** database, which is what
`EGS_BILL_PRODUCT_INFO_ACK` exists to deliver. Its 2360 rows are now transcribed
into `X2Lib/Offline/X2OfflineCashSeed.h` and seeded into `cash_product` by the
schema v9 migration, so the shop is queryable and editable in the save file.

Numbers that settled the open questions, all measured rather than assumed:

| question | answer | how |
|---|---|---|
| real KOG prices for display? | no - only 789 of 2342 shop items have one, so two thirds would show 0 | intersected `EB_Product` against `CashItemPrice.lua` |
| use `EB_ProductAttribute` for socket groups? | no - all 1996 rows carry the same value, and gating the dropdown on row *presence* would reintroduce bug 1 for the 364 products with no row | `COUNT(DISTINCT NO_ATTRIBUTE1)` = 1 |
| key the catalog by item ID? | no - 18 items are sold as two products each, and the client keeps a vector of products per item | `COUNT(*) - COUNT(DISTINCT NO_PRODUCTID)` = 18 |
| do categories fit a signed char? | yes - 22 values, 11..63 | `MAX(CD_CATEGORYNO)` = 63 |
| carry period / level / show / sale / gift? | no - constant across all 2360 rows | `COUNT(DISTINCT ...)` = 1 on each |

The category scheme confirms itself against the client's enum: 11-16 is the six
fashion sub-tabs, 21-27 the seven accessory ones, 31-34 the four consume ones,
then install, pet and event. Tens digit is the tab.

**The wallet is now cosmetic.** `settings.cash_start` is 999999 and a purchase
deducts nothing; the affordability check stays only because it makes the knob
mean something (set it to 0 and the shop is read-only). `account.cash_balance`
is no longer consulted - a per-account copy of a constant is just a second value
to drift.

`CashItemPrice.lua` is no longer needed in `data036.kom` for the shop. Nothing
reads it any more.

### Operational notes from the addendum

- **The game's SQLite has an uncheckpointed WAL, and reading the main file alone
  gives a stale snapshot.** Copying `els_db.sql` on its own showed schema v6 and
  a zero cash balance while the live save was v7 with six deposit lines. Always
  copy `-wal` and `-shm` alongside it.
- **`sqlcmd` v100 (the SQL 2008 tools, and what is on PATH here) cannot reach a
  modern SQL Server** - Native Client 10.0 tops out at TLS 1.0 and the
  connection is closed on it. The container's own client works:
  `docker exec mssql2022 /opt/mssql-tools18/bin/sqlcmd -S localhost -U sa -P ... -C`.
  Run it from PowerShell, not Git Bash, which rewrites `/opt/...` into a Windows
  path before Docker sees it.
- **Profile a table before dumping it.** Five `COUNT(DISTINCT ...)` subqueries
  in one row cut this ask from 2360x15 columns to 2360x4, killed a second table
  outright, and answered two design questions that would otherwise have been
  guesses.

---

---

# Phase 7b — Actionable items: cubes, pets, mounts, class change

Phase 7 built the shop. This is what happens after you buy something that is not
just a costume: the items that have to *do* something when you use them. All of
them were refused, three of them by a phase-7 decision that said the data did
not exist. It does exist — it was just never packed.

## What was actually wrong

Five kinds of item were reported broken. They are not one bug:

| kind | packet | was | now |
|---|---|---|---|
| box / cube | `EGS_OPEN_RANDOM_ITEM_REQ` | refused: "contents table is server data with no client copy" | real, out of `RandomItemTable.lua` |
| pet egg | `EGS_CREATE_PET_REQ` | refused: needs `PetData.lua` | real |
| mount stone | `EGS_CREATE_RIDING_PET_REQ` | refused: needs `RidingPetData.lua` | real |
| class change | *(none — it is claim-time)* | **silently landed in the bag and did nothing, ever** | intercepted at claim, changes class |
| package | `EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ` | refused | still refused, and correctly — see below |

The class-change one is the interesting failure, because nothing logged and
nothing errored. On live, claiming one of those 42 items is intercepted before
the inventory insert (`GSUserCashShop.cpp:2887`) and turned into a class change;
the item never exists. Offline it was an ordinary item ID with no handler
attached to it, so it claimed into the bag and sat there. There was no
`UNHANDLED` packet to notice, because the packet that would have been
unhandled — `EGS_UNLIMITED_SECOND_CHANGE_JOB_NOT` — is a *different* item, and
that one is not in the catalog at all. A silent success is worse than an error
and this is why.

**Packages do not exist in this data.** `EB_ProductPackage` in `ES_BILLING` has
zero rows, and `EB_Product` has no product-kind column, so nothing the shop
sells is a package and `m_byteProductKind` is 0 for every line. The refusal is
unreachable rather than wrong, and it stays. What the report called a "package"
is a box, and boxes are cubes.

## 7b.1 The four files the user has to pack

Same rule as `StatTable.lua` and `DropTable.lua`: XOR-encrypt and pack into
`data036.kom`, no loose copies left behind.

```
KncWX2Server/ServerResource/US/RandomItemTable.lua   8.8 MB   the cube contents
KncWX2Server/ServerResource/US/RandomItemData.lua    220 KB   seal / announce / charm
KncWX2Server/ServerResource/US/PetData.lua            18 KB   egg -> pet
KncWX2Server/ServerResource/US/RidingPetData.lua     5.8 KB   stone -> mount
```

`RandomItemMapping.lua` is deliberately **not** on that list: every call in the
US copy is commented out, so the live US server applies no timed group swap
either. Its two entry points are still bound — as no-ops that log if they ever
fire — because an unbound call aborts the whole chunk at that line.

`RandomItemTable.lua` is eleven times the size of `DropTable.lua`, which is why
the load is lazy. Nothing touches it until the first cube is opened, and that
first open will hitch for a moment.

## 7b.2 The thing that made all three loadable: the enum tables

These files are not self-contained. They index globals by name:

```lua
m_UseCondition = USE_CONDITION["UC_ANYONE"]
m_cUnitClass   = UNIT_CLASS["UC_NONE"]
g_pPetManager:AddPetCreateItemInfo( 500000, PET_UNIT_ID["PUI_PETTE_PPORU"], -1 )
```

On the server those globals exist because the server built them from its own
enums. In the client's Lua state **nothing defines them at all** — every
subscript would come back `nil`, arrive as 0, and the files would load into a
table of zeroes that parses perfectly and is completely wrong. That is the worst
failure shape available, so `CX2OfflineLuaEnum::Publish()` is a hard
prerequisite: if it fails, the loaders refuse to run the scripts rather than
load garbage.

The values are the **client's own** enums, extracted by a generator
(`scratchpad/gen_luaenum.py` → `X2OfflineLuaEnumSeed.h`) and then diffed against
the server's independent copy of each enum, which is a genuine cross-check
rather than two views of one source:

| enum | client | server | shared | mismatched |
|---|---|---|---|---|
| `PET_UNIT_ID` | 103 | 103 | 103 | **0** |
| `RIDING_PET_UNIT_ID` | 6 | 6 | 6 | **0** |
| `UNIT_CLASS` | 60 | 65 | 58 | **0** |

`USE_CONDITION` is the one exception — the client has no copy of it, so its four
values are `CXSLItem::USE_CONDITION` verbatim.

### The bug this nearly shipped with

`True` and `False` are globals too — `m_bGiveAll = True` appears in all 4360
cube blocks, and 1734 of them say `False`. The obvious implementation is
`lua_pushboolean`. It is wrong. `KLuaManager` reads an int field through
`GET_BY_NAME`, which gates on `lua_isnumber` (`KLuaManager.cpp:672`), and in
Lua 5.1 **`lua_isnumber` is false for a boolean**. A boolean `True` would have
read as "field absent", fallen back to the init value, and turned all 2626
give-all cubes into single-item draws — a wrong result with no error anywhere.
They are pushed as the numbers 1 and 0, which read back correctly and are still
truthy to any `if True then`.

Caught by reading `GET_BY_NAME` rather than by testing, which is the only way it
would have been caught — the symptom is "this cube gave me one item" and nobody
knows how many it should have given.

## 7b.3 The cube draw

`CXSLRandomItemManager::GetResultItem` transcribed
(`XSLRandomItemManager.cpp:756`):

* resolve the item group by `m_UseCondition` — `UC_ANYONE` takes entry [0];
  `UC_ONE_UNIT` collapses the class to its base class first; `UC_ONE_CLASS`
  matches exactly. All three occur: 3161 / 926 / 273 blocks.
* `m_bGiveAll` false → one `KLottery` draw over the group; true → the whole group
* plus the charm bonus item, for the 76 cubes that have one

Base-grade collapse needed `CXSLUnit::GetUnitClassBaseGrade`, which the client
has no copy of — and does not need one. The eight base classes are numbered 1..8
and `CX2Unit::UNIT_TYPE` is numbered `UT_ELSWORD`=1..`UT_ELESIS`=8 to match, so
the unit type *is* the base-grade class value, and `CX2OfflineStatTable::UnitTypeOf`
already reads it off the client's own unit templet.

Deliberately not carried over, each for a checked reason:

* **the timed group swap** — mechanism on, US data empty (see above)
* **the enchant level on cube weapons/armour** — `SERV_NEW_ITEM_SYSTEM_2013_05`
  is ON and `#else`s that block out of the single-draw path. (The give-all path
  is not guarded the same way. That asymmetry is in the studio's source; it is
  left alone rather than "corrected".)
* **resurrection / stamina counters** — carried on the packet and passed
  through, but nothing offline consumes them

**ED is a trap on this packet.** `SERV_CUBE_OPEN_ED_CONDITION` is on, and the
client does not treat `m_iED` as a delta — it *assigns* it over the character's
ED (`X2UIInventory.cpp:9121`). A default-constructed ACK carries 0. Every reply
out of the handler, refusals included, fills it, or opening a cube empties the
wallet.

Ordering, as everywhere else in this project: draw → room check → consume cube →
consume key → spend ED → insert. A cube that draws nothing, or does not fit,
leaves the bag exactly as it found it.

Error codes came from `NetError_def.h:288-294`, which does not read the way the
numbering suggests: 00 not in the inventory, 01 not a cube, 02 wrong class,
**04** not enough keys (there is no 03), 05 generic, 07 no room, 13 not enough
ED. The first pass guessed and used a code that does not exist.

## 7b.4 Pets and mounts, and the gap that will bite

Hatching and mount creation are straightforward — the storage
(`unit_pet`, `unit_riding_pet`) was already there from phase 7. New pets start at
`PetData.lua`'s own `SetNewPetInfo` values rather than at invented defaults.
A timed pet is permanent, because `unit_pet` has no expiry column and only 4 of
120 rows are timed (none sold offline); a timed **mount** really does expire,
because `unit_riding_pet` does have one, and 20 of the 38 stone rows are timed.

**The shipped `ServerResource` .lua files are newer than this source tree.** They
name enum values this build does not have:

| file | names | this build has | in the shop |
|---|---|---|---|
| `RidingPetData.lua` | 11 mounts | **4** | 7 sold, **3 unrenderable** |
| `PetData.lua` | 105 pets | 98 | 11 sold, all fine |

The three are the Hamelings — items 550040 / 550041 / 550042 — and the client's
own `RIDING_PET_ITEM_ID` array offers "use" on them regardless, so this is
reachable in ordinary play, not a theoretical edge. Those refuse with a log line
naming the item and **keep the ticket**. Inventing an ID would burn the ticket on
a mount the client cannot draw.

This is worth stating plainly: it is not a bug in this phase and there is no fix
short of extending `CX2RidingPetManager::RIDING_PET_UNIT_ID` and shipping the
models, which is a different project.

## 7b.5 Class change

42 cash items, intercepted at claim before the room check — the item never
enters the bag, so a full inventory is no reason to refuse it.

The map is generated (`gen_classchange.py` → `X2OfflineClassChangeSeed.h`) from
two sources that had to agree and did: the IDs from the client's
`UNIT_CLASS_CHANGE_*_ITEM_ID` constants, the targets from
`CXSLItem::GetClassChangeCashItem`'s switch. 42 IDs, present in both, none on
only one side.

Resolving which branch of that switch counts was the subtle part. Every case
sits inside an `#ifdef` naming a transcendence class with an `#else` naming the
older `UC_*_2` placeholder. All seven of those flags are ON in `US_SERVICE`
(compiler probe), so the live case is the one naming a class the client has —
and the `#else` names a `UC_*_2` this build does not compile. Reading the switch
without resolving that gives six items the wrong target class, which is exactly
what the first pass produced.

What it does: `SaveUnitClass`, then the full skill-tree reset the live path also
does (`InitSkillTree` — clear, restore all SP, re-grant the new class's
defaults). Then `EGS_BILL_GET_PURCHASED_CASH_ITEM_ACK` **first** and
`EGS_BUY_UNIT_CLASS_CHANGE_NOT` **second**, because the client is holding an
`AddServerPacket` wait on the ACK and the NOT rebuilds the character sheet
against a unit it has not been told about yet. Same ordering rule
`AfterQuestComplete` follows.

### The second bug this nearly shipped with

`KEGS_BUY_UNIT_CLASS_CHANGE_NOT` carries `m_vecChangeInProgressQuest` and
`m_vecChangeCompleteQuest`, and the names lie: they are not "the quests that
changed". `CX2State`'s handler feeds them straight into
`CX2QuestManager::SetUnitQuest`, which opens with `ClearUnitQuest()` and rebuilds
from whatever it was handed. Sending the empty vectors the packet is born with
would have **blanked the player's entire quest log** the moment they changed
class, until the next relog put it back.

They now carry the lists exactly as they stand. On live they would be the
*remapped* lists, out of `ClassChangeQuest.lua`; that file is not packed, so no
remap happens and unremapped-but-intact is the honest answer.

Not carried, and narrow enough to state rather than hide: the memo remap, the
item remap and the quest remap (`ClassChangeMemo.lua`, `ClassChangeItem.lua`,
`ClassChangeQuest.lua`, none packed). Class-specific skill notes and completed
job quests keep their old class's IDs. The class change itself — which is what
the item is for — is complete.

`EGS_UNLIMITED_SECOND_CHANGE_JOB_NOT` is still unhandled, and stays that way:
its item (`153000168`) is not in the catalog, so it cannot be bought.

## 7b.6 What is new

```
X2Lib/Offline/X2OfflineLuaEnum.h/.cpp          the four enum tables + True/False
X2Lib/Offline/X2OfflineLuaEnumSeed.h           GENERATED from the client's enums
X2Lib/Offline/X2OfflineRandomItem.h/.cpp       cubes
X2Lib/Offline/X2OfflinePetData.h/.cpp          egg -> pet, stone -> mount
X2Lib/Offline/X2OfflineClassChangeSeed.h       GENERATED, 42 rows
```

rewritten: `Handler_EGS_OPEN_RANDOM_ITEM_REQ`, `Handler_EGS_CREATE_PET_REQ`,
`Handler_EGS_CREATE_RIDING_PET_REQ`, and the class-change interception inside
`Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ`.

### Exit test

Pack the four files. Then, watching `offline_server.log`:

* `LUA published enum tables: UNIT_CLASS(60) ...` on the first cube or egg
* `CUBE loaded: N cube(s) ... M case(s) across G group(s)` — a nonzero N
* `PET loaded: N egg(s) -> pet, M stone(s) -> mount` with a `NOTE ... 7 stone
  row(s)` skip line
* open a cube, hatch a pet, summon a mount, claim a class-change item
* **check ED did not change** after opening a cube — that is the regression this
  phase is most exposed to
* **check the quest log still has its quests** after a class change

Anything refused says which file is missing and what to do about it. Nothing
guesses.

### Exit test — RUN, in progress (2026-09-04)

The four files were packed. Class-change claim confirmed working end to end:
item claimed, class changed, quest log intact. Two defects turned up before
the cube/pet/mount checks could be signed off, and neither is fixed yet.

**1. Opening a cube emptied the wallet.** This is despite
`Handler_EGS_OPEN_RANDOM_ITEM_REQ` ([Handlers_Social.cpp:1907](X2Lib/Offline/Handlers_Social.cpp#L1907))
setting `kAck.m_iED = kUnit.m_iED` before every reply path, exactly as the
assign-not-delta warning in this phase's own code comments says it must. Every
saved backup of the character's ED only ever went up (2783 -> 27127 -> 41145),
so the value in SQLite was never zeroed - whatever went wrong happened between
the ACK and the client showing it, or somewhere not yet found. Not root-caused.

**2. A class of consumables is silently voided, wider than cubes/pets/mounts.**
Found while testing item 78894 ("Elixir" -
`CXSLItem::SI_THE_GATE_OF_DARKNESS_ELIXIR_GIANT_POTION`,
[XSLItem.h:934](KncWX2Server/Common/X2Data/XSLItem.h#L934)) and two more items
(270970-270972) used from the bag. `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`
([Handlers_Inventory.cpp:187](X2Lib/Offline/Handlers_Inventory.cpp#L187))
treats the client's `GetCanUseInventory()==true` flag as blanket permission to
delete the item and reply success. On live, that flag only means "the UI lets
you double-click this" - `GSUserInventory.cpp`'s handler (the live path starts
at line 4940) runs a ~700-line item-ID switch first: warp teleport, buff
activation via `KGSUser::ActivateItemBuff` / `CXSLBuffManager`
([GSUserFunction.cpp:15978](KncWX2Server/GameServer/GSUserFunction.cpp#L15978)),
skill unseal, nickname change, guild/bank/quickslot expansion. Offline has none
of it. Confirmed concretely:

  - `kAck.m_iWarpPointMapID` is hardcoded to 0
    ([Handlers_Inventory.cpp:199](X2Lib/Offline/Handlers_Inventory.cpp#L199))
    and never set afterward, so a warp scroll used from the bag disappears
    with no teleport.
  - The Elixir's "blessing" buff is entirely a server + login-server
    bookkeeping system - no `CXSLBuffManager`/`BTI_BUFF_*` symbol appears
    anywhere under `X2Lib/`, so there is nothing client-side to trigger. The
    item is consumed and nothing happens.

  Items 270970-270972 have no `CXSLItem::SI_*`/`EI_*` enum entry at all, so
  their effect is data-driven (`ItemTemplet::m_iBuffFactorID` /
  `m_SpecialAbilityList`) rather than a hardcoded case - not yet identified by
  name. Per the live-DB rule, `ScriptData/ItemTemplet.xlsx` is not trusted for
  this; the planned next step is a diagnostic log line reading the name and
  ability counts straight off the client's own loaded item templet, not a
  guess from the spreadsheet.

Scope for the fix - how much of the live item-use switch is worth porting for
a single-player save - is an open decision, not yet made.

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
