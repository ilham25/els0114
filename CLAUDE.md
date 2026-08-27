# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Full source for **Elsword (internal codename "ProjectX2")** by KOG Studios — the DirectX 9 Windows game client, its five backend servers, and the in-house content tooling. This is the `EU_CN_US` trunk, roughly 2014-era. It was originally a Subversion/VSS working copy (`.vssscc` bindings remain, `.svn` metadata does not) and has since been imported into git as the working repo for local changes — see `MODS.md` for what's been changed since the import, and `VS2003_to_VS2010_Port_Guide.md` for the VS2010 server port.

**This file documents the US live build only.** The tree carries ~50 configurations for other regions and environments (EU, CN, TW/HK, JP, KR, ID, TH, BR, PH × internal/test/open-test/service). Ignore them unless explicitly asked; they change which publisher, billing, auth, and anti-cheat stack compiles in, and cross-referencing them is how you end up debugging the wrong code path.

## Rule: every client change goes behind a `SERV_IRUHADEV_` flag

**All edits to client code — `X2/`, `X2Lib/`, `KTDXLIB/` — must be wrapped in a custom `#define` named with the `SERV_IRUHADEV_` prefix.** No exceptions, including one-line fixes. This follows the studio's own practice (see *Feature flags* below) and keeps every local change revertible by undefining a single macro.

Define the flag in **`KTDXLIB/Always.h`**. That is the only flag file that reaches all three client projects in a `US_SERVICE` build: `KTDX.h` defines `_ALWAYS_` unconditionally ([KTDX.h:37](KTDXLIB/KTDX.h#L37)) and includes `Always.h` at [KTDX.h:151](KTDXLIB/KTDX.h#L151); `X2Lib` reaches it through `stdafx.h` → `X2Main.h` → `KTDX.h`, and `X2.exe` through `X2.cpp` → `X2/Common/dxstdafx.h` → `X2Main.h`.

Do **not** define client flags in:

- `KncWX2Server/Common/ServerDefine.h` — that is the shared/server file. It *is* visible inside `X2Lib` (via `X2ServerPacket.h` → `CommonPacket.h:16`), but it is **not** visible inside `KTDXLIB`, and defining a client flag there drags it into the server build too.
- `KTDXLIB/InHouse1.h`–`InHouse6.h`, `InHouseEtc.h` — these are excluded from `US_SERVICE` entirely, so anything defined there is dead code in this build.

Pattern to follow — append to the end of `Always.h`, using the house comment block:

```c
//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-08-24
// Description: <what this change does>
#define SERV_IRUHADEV_<SHORT_NAME>
//////////////////////////////////////////////////////////////////////////
```

(The surrounding entries in `Always.h` use Korean labels in CP949; write new ones in ASCII as above rather than pasting back mojibake.)

And at each call site, keeping the original code reachable in the `#else` branch whenever behavior is being replaced rather than added:

```c
#ifdef SERV_IRUHADEV_<SHORT_NAME>
    // new behavior
#else
    // original behavior, left intact
#endif SERV_IRUHADEV_<SHORT_NAME>
```

One flag per logical change, not one per file. Use the house `#endif SERV_IRUHADEV_FOO` trailing-token style.

If an edit touches `KncWX2Server/Common/` (packet structs, event IDs, shared enums), it is **not** a client-only change — the servers must be rebuilt (via `X2Project_Servers_2010.sln`, or `X2Project_2003.sln` if VS2003 is installed) and the flag must be defined for both sides, or the wire format desyncs silently. See *The client/server contract* below.

## Toolchains — client is VS2010; servers build under either

Historically the client and the servers were built with **different versions of Visual Studio**, because the servers didn't build under VS2010 at all. **That changed 2026-08-27**: the five servers now also build under VS2010, via a dedicated solution added by a from-scratch port (see `VS2003_to_VS2010_Port_Guide.md` for exactly what the port did and why — the process is written to be reusable on other old-toolchain codebases, not just this one). VS2010 is the toolchain actually installed in this environment and is the maintained path going forward; VS2003 remains usable for the servers only on a machine that still has it installed.

| Target | Solution | Config | Toolchain |
|---|---|---|---|
| Client, engine, tools | `X2Project_2010.sln` | `US_SERVICE` | VS2010 (`v100`), Win32, Unicode |
| The five servers (current) | `X2Project_Servers_2010.sln` | `Release_US` | VS2010 (`v100`), Win32 |
| The five servers (legacy, needs VS2003 installed) | `X2Project_2003.sln` | `Release_US` | VS2003 (`v70`), Win32 |

`X2Project_Servers_2010.sln` is a **separate solution** from `X2Project_2010.sln`, deliberately — the five server projects are *also* registered in `X2Project_2010.sln`, but under a `US_SERVICE` configuration that is still dead (wrong include paths, source list missing ~50–100 files per project; that config predates the port and was never fixed). Don't build servers from `X2Project_2010.sln`; use `X2Project_Servers_2010.sln`. `X2Project_2003.sln` was left byte-for-byte untouched by the port and remains the toolchain of record if you ever need to cross-check against a VS2003 build. (`KncWX2Server/KncWX2Server_2003.sln` is a narrower VS2003 solution holding only GameServer, CenterServer, and GameClient, with plain Debug/Release configs — unrelated to the port, predates it.)

```sh
# Client — VS2010.
msbuild X2Project_2010.sln /p:Configuration=US_SERVICE /p:Platform=Win32
msbuild X2/X2_2010.vcxproj /p:Configuration=US_SERVICE /p:Platform=Win32   # client exe only

# Servers — VS2010 (current; this is what's actually installed here).
msbuild X2Project_Servers_2010.sln /p:Configuration=Release_US /p:Platform=Win32
msbuild KncWX2Server/GameServer/GameServer_2010.vcxproj /p:Configuration=Release_US /p:Platform=Win32   # one server only

# Servers — VS2003 (legacy; devenv, msbuild cannot consume .vcproj). Requires VS2003 installed.
devenv X2Project_2003.sln /build "Release_US|Win32"
devenv X2Project_2003.sln /build "Release_US|Win32" /project GameServer
```

Build artifacts:

- `X2/US_SERVICE/X2.exe` — the client
- `X2/X2Lib.lib`, `X2/KTDXLIB.lib` — static libs; `OutDir` is `..\X2\`, only the `.obj` intermediates land in `X2Lib/US_SERVICE/` and `KTDXLIB/US_SERVICE/`
- `KncWX2Server/<Server>/Release_US/<Server>.exe` — all five servers, built by **either** toolchain into the **same** output directory (the config is named `Release_US` on both sides by design — see the port guide §0). Building with one toolchain overwrites an exe built by the other; there's no separate output path to keep them apart.

**Running a VS2010-built server**: it needs the VC10 runtime DLLs (`msvcr100.dll`, `msvcp100.dll`, `mfc100u.dll` — source from `<VS10 install>\VC\redist\x86\Microsoft.VC100.{CRT,MFC}\`) copied alongside the exe; the old VC7.1 ones already there (`msvcr71.dll` etc.) don't satisfy it. It also depends on a fix in `KncWX2Server/Common/ui/SubclassWnd.h` (a hand-rolled window-subclassing thunk that writes and executes machine code at runtime, which modern Windows' DEP blocks unless `VirtualProtect`'d executable first) — that fix is already in the tree from the port; if a server built from a *newer* checkout of this file ever regresses that, expect every server to crash instantly with `0xC0000005` on launch. Full detail in the port guide §6–7.

There is **no runnable test suite**. The one CppUnit fixture (`X2Lib/X2GameUnitTestCase.h`) is gated behind `CPPUNIT_BY_TOOL_TEAM`, commented out in `KTDXLIB/AlwaysButConditionally.h`. `Libs/InternalLib/KNCSDK/UnitTest/` is the vendored SDK's own test project, not wired into either solution.

### What `US_SERVICE` actually defines (client)

| Project | Preprocessor definitions |
|---|---|
| `X2` | `WIN32;NDEBUG;_WINDOWS;_SERVICE_;_USE_32BIT_TIME_T;CLIENT_COUNTRY_US;SERV_COUNTRY_US;AUTO_LOGIN_IN_HOUSE` |
| `X2Lib` | `WIN32;NDEBUG;_LIB;_SERVICE_;_USE_32BIT_TIME_T;_HAS_ITERATOR_DEBUGGING=0;CLIENT_COUNTRY_US;SERV_COUNTRY_US` |
| `KTDXLIB` | `WIN32;NDEBUG;_LIB;_SERVICE_;_HAS_ITERATOR_DEBUGGING=0;CLIENT_COUNTRY_US;SERV_COUNTRY_US` |
| `X2ServerProtocol`, `luaLib`, `libxml` | region-neutral (`WIN32;NDEBUG;_LIB;…`) |

Consequences worth knowing before you go looking for code that "should" be there:

- **`_IN_HOUSE_` is NOT defined.** `KTDXLIB/KTDX.h` only includes the per-developer flag files `InHouseEtc.h` and `InHouse1.h`–`InHouse6.h` under `_IN_HOUSE_`, so in `US_SERVICE` none of them apply. The only active client flag files are `Always.h` and `AlwaysButConditionally.h`. Debug cheats, test UI, and `WORLD_TOOL` overrides are all compiled out.
- **`_OPEN_TEST_` is NOT defined.** `X2Lib/define.h` therefore resolves the US patch source to `http://gamepatch.elswordonline.com/` with `PatchPath.dat`.
- **`AUTO_LOGIN_IN_HOUSE` is defined on the `X2` project only**, and every use of it (`X2Lib/X2StateLogin.cpp`, which reads ID/password out of `LoginKey.lua`) sits in `X2Lib`, which is built *without* it. The flag is inert as configured. This is a good illustration of the failure mode below — a flag set on one project and not another silently does nothing.

Server-side, `Release_US` defines only `WIN32;NDEBUG;_WINDOWS;SERV_COUNTRY_US` for all five executables. Note there is no `_SERVICE_` on the servers — live vs. test is a **runtime** choice, not a compile-time one (see below).

### Post-build event will fail here

The `X2` project's `US_SERVICE` post-build step copies the exe and Lua content to hardcoded original-studio paths (`E:\Elsword_InHouse\Data\…`, `D:\ProjectX2_SVN\resource\KR\Trunk\dat\script\…`). On any machine without those drives it errors out *after* `X2.exe` has already linked successfully. Judge the build by whether `X2/US_SERVICE/X2.exe` was produced, not by the post-build exit code.

## Architecture

### Layers

| Directory | Role |
|---|---|
| `KTDXLIB/` | The engine. `CKTDXApp` owns the D3D9 device, main loop, input, sound (FMOD), UI widget set (`KTDGUI*`), mesh/skin/particle renderers, and the Lua state. Classes are `K`-prefixed. |
| `X2Lib/` | All client gameplay — ~575 `CX2*` classes. The bulk of the codebase. |
| `X2/` | Thin client `.exe` shell: `WinMain`, anti-cheat bootstrap, crash reporting. Almost no logic. |
| `X2ServerProtocol/` | Client-side networking runtime (`KUserProxy`, `KTRUser`, `KPerformer`, ODBC, thread manager). Linked into the client; mirrors the server's actor model. |
| `KncWX2Server/` | Five server executables plus the shared `Common/` layer. |
| `KNCSDK/` | KOG's in-house SDK: serializer, Lua bindings (`lua_tinker`/`luabind`), crypto, object pools, threading. Source of the `SERIALIZE_*` / `DECL_PACKET` macros. Both client and servers use the prebuilt `.lib`s here (`Include`/`lib` for VS2003, `Include_2010`/`lib_2010` for VS2010) — not the near-duplicate at `Libs/InternalLib/KNCSDK/`, which nothing in either solution actually references. |
| `Libs/ExternalLib/` | Vendored Boost, DXSDK, Intel TBB, Lua, log4cxx, freetype, jsoncpp, cppunit. |

### Client control flow

`CX2Main` (`X2Lib/X2Main.h`, ~2000 lines) is the client god-object and top-level state machine. Its `X2_STATE` enum (`XS_LOGIN`, `XS_DUNGEON_ROOM`, `XS_DUNGEON_GAME`, `XS_PVP_GAME`, `XS_VILLAGE_MAP`, …) maps 1:1 onto the `X2State*.h` classes. `CX2Main::StateChange()` is the transition point.

Both `CX2Main` and each state derive from `CKTDXStage` — the engine's `OnFrameMove` / `OnFrameRender` / `MsgProc` / `OnResetDevice` / `OnLostDevice` interface. Actual matches derive from `CX2Game` (`X2Lib/X2Game.h`), itself a stage; `CX2DungeonGame`, `CX2PVPGame`, `CX2SquareGame` specialize it.

**In-match networking is peer-to-peer UDP, not client-server.** One player is host (`CX2Game::IsHost()`); unit, NPC, and pet state sync through `Handler_P2P_*` over `CKTDNUDP`. The servers own lobby, rooms, inventory, and persistence — not per-frame combat. Anything authoritative-looking inside a match is client-trusted.

Lua is pervasive on the client: game data and much content logic live in Lua, bound via `g_pKTDXApp->GetLuaBinder()` + `lua_tinker::decl(...)` in each manager's constructor, and loaded out of `.kom` mass-file archives at runtime. Shipped scripts are compiled with `LuaCompileTool/` (wraps `luac.exe`).

### Servers

Five executables under `KncWX2Server/`, all built on the shared `Common/` actor framework (`KActor` / `KPerformer` with per-object event queues drained by `Tick()`, FSMs declared in `*FSM*_def.h`, ODBC plus stored procedures for persistence):

- **LoginServer** — accounts, auth, guilds, friends
- **ChannelServer** — channel routing, CCU
- **GameServer** — the main simulation/lobby server (billing, quests, items, fields)
- **CenterServer** — cross-server rooms, battlefields, dungeon events
- **GlobalServer** — auto-party matchmaking and other cross-shard services

`GameClient/` is not a server — it's the robot/load-test client plus client-side proxy code.

**Runtime config selects the environment.** `KBaseServer::…` (`Common/BaseServer.cpp`) requires at least two command-line arguments: a *version tag* and a *server group ID*. It loads `<base>.ini` and then `<base>_<versionTag>.lua`, where `<base>` is fixed per executable (`config_gs`, `config_lg`, `config_ch`, `config_cn`, `config_gb`). So the US live GameServer runs as:

```
GameServer.exe US_SERVICE 1        # -> config_gs.ini + config_gs_US_SERVICE.lua
```

Missing those arguments is a hard startup failure. The US configs live in `<Server>/Config/US/`; `config_gs_US_SERVICE.lua` sets `Server:SetUseVersion( VE_SERVICE )` and registers the ODBC DSNs (`Account_US_SERVICE.dsn`, `Game01_US_SERVICE.dsn`, `log_US_SERVICE.dsn`, `Billing_US_SERVICE.dsn`). These Lua files are UTF-8 with BOM and say so in their header line — the BOM is load-bearing. SQL Server schema and stored procedures are in `DataBase/`.

### The client/server contract

The client compiles the server's headers directly — `X2Lib`'s include path contains `../KncWX2Server/Common/`. There is no generated IDL and no mirrored copy:

- **`KncWX2Server/Common/EventID_{System,Client,Server}.h`** — one packet/event ID per `_ENUM( … )` line. The X-macro is defined by the includer: `Event.h` expands it to enum values (three contiguous ranges chained via `E_*_EVENT_ID_END`), `Event.cpp` expands the same files to a `const wchar_t*` name table. Adding an ID anywhere but the end of a block renumbers the wire protocol.
- **`KncWX2Server/Common/{Client,System,Common}Packet.h/.cpp`** — payload structs via `DECL_PACKET( EGS_FOO_REQ )` (declares `struct KEGS_FOO_REQ`), with hand-written `SERIALIZE_DEFINE_PUT` / `SERIALIZE_DEFINE_GET` bodies in the `.cpp` listing fields in order. **Put and Get must stay in the same order, and every new field must be added to both.**
- **`KncWX2Server/Common/Enum/Enum.h`** — shared `SEnum::` gameplay enums.

Because the two sides are compiled separately (even now that both can use VS2010, they're still two different solutions with different macro sets), **a shared-header change means rebuilding the client *and* the servers**, with the relevant flags enabled in both. Skip one and packets desynchronize silently at runtime rather than failing to compile.

### Feature flags

Nearly every change since ~2009 sits behind a named `#define`, and there is no build-time consistency check.

- **Server plus shared:** `KncWX2Server/Common/ServerDefine.h` (~4300 lines, ~490 `SERV_*` flags). Its tail keys off `SERV_COUNTRY_US` to pull in `Common/OnlyGlobal/ServerDefine/ServerDefine_US.h`, alongside the always-on `ServerDefine_Global.h`.
- **Client:** `KTDXLIB/KTDX.h` is the hub, but under `US_SERVICE` only `Always.h` and `AlwaysButConditionally.h` are reachable (see above). `X2Lib/X2Define.h` holds client tuning constants and the `XEM_ERROR*` code list. Several blocks in `KTDX.h` are explicitly `#ifndef _SERVICE_`, i.e. deliberately absent from this build.

When adding a flag, follow the local pattern: new `#define` at the end of the owning file, commented with author / date / description, and make sure it is defined for *every* project that compiles code guarded by it. For our own client changes the owning file is always `KTDXLIB/Always.h` and the name always starts with `SERV_IRUHADEV_` — see the rule at the top of this file.

## Conventions

- Comments and identifiers are frequently **Korean in CP949/ANSI encoding**, not UTF-8 — most editors and `grep` render them as mojibake. Do not "fix" the encoding of a file you are editing; write new comments in ASCII and leave existing bytes alone.
  - **This is not just an editorial preference — Claude Code's Edit tool can silently violate it.** On at least one CP949 file, a normal Read-then-Edit round-trip (touching only a handful of lines) re-encoded the *entire file* to UTF-8 and normalized its line endings, corrupting every Korean byte sequence in it while `git diff` looked deceptively plausible at a glance. **After editing any file that carries non-ASCII bytes, verify before trusting the result or committing:** run `file <path>` and confirm the encoding line hasn't changed (e.g. `ISO-8859 text` / `Non-ISO extended-ASCII text`, not `Unicode text, UTF-8 text`), and check `git diff --stat` — a change of a few lines should not produce a diff of hundreds or thousands. If it did, `git checkout -- <path>` to restore the pristine bytes (confirm with the user first — it's a destructive command) and redo the edit at the byte level instead (e.g. a small Python script opening the file in `"rb"`/`"wb"` mode, matching your target region by exact byte content, and leaving everything else untouched — reuse existing non-ASCII byte spans verbatim via slicing rather than retyping them).
- Change blocks are marked `//{{ author : date // description` … `//}}` and are expected on non-trivial edits.
- `#endif SERV_FOO` — a trailing token after `#endif` (invalid standard C++, tolerated by MSVC) is the house style for matching a long `#ifdef`. Keep it.
- Every client edit is gated behind a `SERV_IRUHADEV_*` flag defined in `KTDXLIB/Always.h` — see the rule at the top of this file.
- Write server code (`KncWX2Server/`) as VS2003/C++98-clean anyway, even though it now also builds under VS2010: no `auto`, no range-for, no `nullptr`, no rvalue references. This isn't a hard toolchain requirement any more (see *Toolchains* above), but `X2Project_2003.sln` is kept around specifically as a cross-check, and anything in `KncWX2Server/Common/` still compiles under **both** toolchains — a VC10-only construct there would quietly break that. Genuine VS2003→VS2010 compiler-conformance fixes (bare member-function pointers needing `&`, for-scope leaks, etc.) are expected and fine; see the port guide for the recurring patterns.
- Prefix by layer: `CKTDX*` / `KTDG*` engine, `CX2*` client gameplay, `K*` server and KNCSDK.
- Wide strings (`std::wstring`, `WCHAR`) throughout; `#pragma pack(push,1)` around every packet struct.

## Tooling

Content tools are separate executables, mostly with their own solutions. C++ world/content editors: `X2WorldTool`, `X2DungeonTool`, `X2EffectTool`, `X2ItemTool`, `X2LineMapTool`, `X2ScenarioTool`, `X2EventSceneTool`, `X2Viewer`, `X2MassFileTool` (`.kom` archive packing). C# WinForms data managers over Access/Excel and SQL Server: `ScriptDBManager`, `StringDBManager`, `TextureStringManager`, `Localization/DBManager`, `TransDBManagerForWeb`, `ContentSelector`, `ReleaseTool`, `BillingTool`, `NetErrorDBManager`. Balance and table data ships as `.xls` / `.xlsm` / `.mdb` in `ScriptData/`. Client patching: `X2Patcher`.

`HShield/`, `XTRAP/`, `NexonModule/`, `Steamworks/`, `IGA/` are third-party anti-cheat, publisher, and ad SDKs, each activated by its own macro. They are the usual reason a fresh configuration fails to link.

## Cautions

- `X2Project_2010.sdf` (270 MB), `X2Project_Servers_2010.sdf`, and `X2Project_2003.ncb` (22 MB) are IntelliSense databases, and `ipch/` is precompiled-header cache. Never edit them; deleting them is safe and VS regenerates them.
- `Libs/` holds ~22,000 vendored files. Scope searches to `X2Lib`, `KTDXLIB`, `KncWX2Server`, or `X2ServerProtocol` — a repo-wide recursive grep takes minutes.
- Server config `.dsn` files contain real database hostnames and plaintext credentials from the original deployment. Don't propagate them into new files, logs, or anything published.
