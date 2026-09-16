# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Full source for **Elsword (internal codename "ProjectX2")** by KOG Studios — the DirectX 9 Windows game client, its five backend servers, and the in-house content tooling. This is the `EU_CN_US` trunk, roughly 2014-era. It was originally a Subversion/VSS working copy (`.vssscc` bindings remain, `.svn` metadata does not) and has since been imported into git as the working repo for local changes — see `MODS.md` for what's been changed since the import. **The client source was replaced with a March 2014 snapshot on 2026-09-09 (`dd63297`), which overwrote every local edit inside a studio-owned file; re-applying them is in progress — read `MARCH_2014_MIGRATION.md` first.** `VS2003_to_VS2010_Port_Guide.md` documents a VS2010 server port that the March sources have since undone; it is history, not instructions.

**This file documents the US live build only.** The tree carries ~50 configurations for other regions and environments (EU, CN, TW/HK, JP, KR, ID, TH, BR, PH × internal/test/open-test/service). Ignore them unless explicitly asked; they change which publisher, billing, auth, and anti-cheat stack compiles in, and cross-referencing them is how you end up debugging the wrong code path.

## Rule: every client change goes behind a `SERV_IRUHADEV_` flag

**All edits to client code — `X2/`, `X2Lib/`, `KTDXLIB/` — must be wrapped in a custom `#define` named with the `SERV_IRUHADEV_` prefix.** No exceptions, including one-line fixes. This follows the studio's own practice (see *Feature flags* below) and keeps every local change revertible by undefining a single macro.

Define the flag in **`KTDXLIB/Always.h`**. That is the flag file that reaches all three client projects unconditionally in any US client build (`US_SERVICE`, `US_INTERNAL`, ...): `KTDX.h` defines `_ALWAYS_` unconditionally ([KTDX.h:37](KTDXLIB/KTDX.h#L37)) and includes `Always.h` at [KTDX.h:190](KTDXLIB/KTDX.h#L190); `X2Lib` reaches it through `stdafx.h` → `X2Main.h` → `KTDX.h`, and `X2.exe` through `X2.cpp` → `X2/Common/dxstdafx.h` → `X2Main.h`.

Do **not** define client flags in:

- `KncWX2Server/Common/ServerDefine.h` — that is the shared/server file. It *is* visible inside `X2Lib` (via `X2ServerPacket.h` → `CommonPacket.h:16`), but it is **not** visible inside `KTDXLIB`, and defining a client flag there drags it into the server build too.
- `KTDXLIB/InHouse1.h`–`InHouse6.h`, `InHouseEtc.h` — as of the March 2014 snapshot these **are** compiled in under `US_INTERNAL` and (for now — see *Toolchains*) `US_SERVICE` too, both of which define `_IN_HOUSE_` — so a flag put there would take effect, but they are the studio's per-developer files, they carry ~84 apparent defines whose real state needs the preprocessor to settle, and a flag of ours buried among them is unfindable. Keep ours in `Always.h`.

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

If an edit touches `KncWX2Server/Common/` (packet structs, event IDs, shared enums), it is **not** a client-only change — the servers must be rebuilt (via `X2Project_2003.sln`, which needs a machine with VS2003 — it is **not** installed here) and the flag must be defined for both sides, or the wire format desyncs silently. See *The client/server contract* below.

## Rule: server-side Lua the client needs is the user's to pack — never band-aid around it

Some data only ever existed on the server: per-level stat tables, drop tables,
field tuning. When client-side code needs one of those files, it has to be
XOR-encrypted and packed into `data036.kom` alongside every other client script,
and **the user does that step.** Do not stop and wait for confirmation before
building the feature that needs it — implement it now, on the assumption that
the file will be (or already is) packed, and name the file when you report the
work as done rather than before you start it. The user has said explicitly not
to block work on this: packing is fast on their end, and the degrade path below
means the feature just stays off with a clear log line until it's in — nothing
is lost by finishing the implementation first.

Do:

1. **Implement the whole feature in one pass**: the loader, the logic that
   consumes the table, and the degrade-if-missing path (step 3 below) all
   together. Do this even when some other document (a phase plan, an issue
   writeup, your own earlier diagnosis) frames "refuse and log why" as an
   acceptable stopping point for the *current* task — a feature that only
   "can't be done" for lack of a packed file is not the same as a feature that
   doesn't exist, and finishing it costs nothing thanks to the degrade path.
2. **Load it the shipped way.**
   `g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( name )` for
   the `.kom` container, then `GetLuaBinder()->DoMemory( ... )` for the XOR, with a
   `DoMemoryNotEncript( ... )` fallback that logs a note when it fires. Copy the
   shape from `X2Lib/Offline/X2OfflineStatTable.cpp`,
   `X2OfflineDropTable.cpp` or `X2OfflineBattleField.cpp`.
3. **Degrade visibly.** If the file is not there, turn the feature off and log
   which file is missing and what to do about it. A feature that is off and says
   so is debuggable; one that silently guesses is not. This is also what makes
   rule 1 safe: build against the assumption the file is packed, and an
   unpacked file fails into this path instead of into a guess.
4. **Name the file when you report the work as done, not before.** Give the
   exact path (`KncWX2Server/ServerResource/US/<name>.lua`) and say it needs
   XOR-encrypting and packing into `data036.kom` if it isn't already. If the
   file turns out to already be packed, confirm that from the log (the loader's
   own "loaded N row(s)" line) rather than assuming — don't report success on
   the strength of the code compiling alone.

Do not:

- **Do not write a loose copy into the game directory.** Under `US_INTERNAL`
  this *works*, and works harder than it used to — and then masks a failed
  repack, leaving nobody able to tell whether the archive is actually right.
  Delete loose copies once the file is packed.
  The precedence inverted with the March snapshot, so old notes on this are
  wrong in both directions. `KGCMassFileManager::LoadDataFile`
  ([KGCMassFileManager.cpp:878-941](KTDXLIB/KGCMassFileManager.cpp#L878), `#else`
  at `:915`) branches on `MASS_FILE_FIRST`, which `US_INTERNAL` does **not**
  define: `:916` calls `LoadRealFile` **unconditionally first**, so a loose file
  beats the archive every time. With the macro (the old January build) the
  `.kom` came first and the loose fallback was reached only if the file was
  absent *and* `_SERVICE_` was undefined (`:890`) — which the old `US_SERVICE`
  build did define, so back then a loose copy did nothing at all. A miss logs
  `KEM_ERROR136` now, `KEM_ERROR135` then.
- **Do not hardcode the table, invent a curve, or approximate a function** to
  paper over a file that has not been packed yet. That is the band-aid: it looks
  like progress, it drifts from the real data, and it hides the missing file.
- **Do not edit the studio's `.lua` to make code simpler.** Editing that data to
  *tune gameplay* is fine and is the right place for it (see
  `BattleFieldServerData.lua`'s solo-play comment) — but keep the original value
  and the restore instruction in comments, and preserve the file's UTF-8 BOM and
  CRLF byte-for-byte.

The one permitted fallback is a constant that can be **read out of the repo and
cited**: `BattleFieldServerData.lua`'s respawn window and reward factors are
carried in code as fallbacks with a comment naming their source, so an unpacked
install behaves like the live server rather than like nothing. A fallback is
honest only when it is the real number, is labelled with where it came from, and
is announced in the log. Anything that cannot be sourced that way — a Lua
*function*, for instance — gets no fallback at all.

## Rule: `DataBase/` and `ScriptData/` are a stale snapshot — the live DB is the source of record

The user has access to the **original SQL Server databases** (`Account`,
`Game01`) this client was served by. Everything under `DataBase/` and
`ScriptData/` in this tree is a *checkout from some point in the game's life*,
and it has been proven to disagree with what actually shipped. So:

**When behaviour depends on a stored procedure or a server-side data table, ask
the user to pull it from the live DB before building on the copy in this tree.**
Name the procedure or table, give the query, and wait — the same shape as the
Lua-packing rule above.

```sql
-- procedure bodies
SELECT OBJECT_NAME(object_id) AS proc_name, definition
FROM   sys.sql_modules WHERE OBJECT_NAME(object_id) IN ( 'gup_...', … );

-- when the live names may have drifted from this snapshot's
SELECT name FROM sys.procedures WHERE name LIKE 'gup_%' ORDER BY name;
```

The two failures that produced this rule, both from character creation:

- **`DataBase/`'s `dbo.gup_create_unit` predates a refactor.** The copy here
  inserts no items; the live one copies ten positioned rows out of
  `dbo.GBase_Item` per class and aborts with `-28` if there are not exactly ten.
  Reading the local copy line by line produced the confident, wrong conclusion
  that a new character starts with nothing.
- **`ScriptData/ItemTemplet.xlsx` disagrees with the live item table.** Every
  beginner-gear ID in `GBase_Item` (`131641..`, `111094..`, `112700..`) is
  absent from that spreadsheet, which carries a later reform block instead.
  Deriving IDs from the spreadsheet produced a plausible, checkable, wrong
  answer — and *it agreed with the item names the user read off their own
  client*, so it survived a sanity check that should have caught it.

Notes that matter in practice:

- **A procedure missing from `DataBase/` is not evidence it does nothing.**
  `dbo.gup_create_unit_set_promotion` has no body anywhere in this tree; it is
  called on every character creation.
- **A local copy that reads consistently is still not corroboration.** Both
  failures above came from sources that were internally coherent. Two sources
  agreeing only counts when they are genuinely independent — the client's own
  arrays and a stored procedure are; a spreadsheet and a pattern derived from it
  are not.
- **Commented-out code in a live procedure is history, and history is still
  useful.** The dead per-class blocks in `gup_create_unit` were worthless as a
  source of IDs and were the thing that revealed the slot *layout*
  (`InventoryCategory 9`, odd slots real / even slots fashion). Read it for
  structure, never for values.
- **Transcribe, don't tidy.** The live table gives classes 7, 8 and 9 the same
  set, which looks like an unfinished row. It goes in verbatim with a log line
  saying so — "correcting" studio data on the way in makes the code disagree
  with the server for a reason nobody can later reconstruct.
- **Treat the dump as confidential.** Ask for it as plain text in the
  conversation, not as a file written into the game tree, and keep server names,
  linked servers and logins out of anything committed or logged — the same
  handling the `.dsn` files get (see *Cautions*).

## Toolchains — the client is VS2010/`US_SERVICE`; the servers are VS2003 and unbuildable here

**This changed on 2026-09-09, when the client source was replaced with a March 2014 snapshot (`dd63297`).** Two things moved at once: the client's only US configuration became `US_INTERNAL` (`US_SERVICE` was gone from every `.vcxproj` and `.sln`), and the servers reverted to VS2003 — the VS2010 server port's source fixes were overwritten by the March sources, so `X2Project_Servers_2010.sln` can no longer build and `VS2003_to_VS2010_Port_Guide.md` is now history rather than instructions.

**A new `US_SERVICE` client configuration was added back on 2026-09-16** (`X2/X2_2010.vcxproj`, `X2Lib/X2Lib_2010.vcxproj`, `KTDXLIB/KTDXLIB_2010.vcxproj` — not the `.sln`, see below), cloned from `US_INTERNAL` with the `ClCompile` optimization settings changed to match the current `EU_SERVICE`/`BR_SERVICE` profile (`Optimization=Full`, `IntrinsicFunctions=true`, `EnableFiberSafeOptimizations=true`, `FavorSizeOrSpeed=Speed`, `OmitFramePointers=true`, `FunctionLevelLinking=false`) — `US_INTERNAL` compiled essentially unoptimized (`Optimization=Disabled`, or no `Optimization` tag at all in `X2Lib`'s case), which was the cause of a reported inventory-hover/tab-switch stutter. **Build the client as `US_SERVICE` by default from now on; only use `US_INTERNAL` if explicitly asked for it** (e.g. to reach for the in-house cheat/dev-UI surface, or to compare against the pre-optimization baseline). As of this writing `US_SERVICE` still defines `_IN_HOUSE_` identically to `US_INTERNAL` — stripping it is a separate, not-yet-done step (tracked as "stage 2" against this same config; see the *What `US_INTERNAL`/`US_SERVICE` actually define* table below for what that would change) — so the two configs currently differ **only** in the optimization flags, nothing else.

| Target | Solution | Config | Toolchain |
|---|---|---|---|
| Client, engine, tools (default) | `X2/X2_2010.vcxproj`, `X2Lib/X2Lib_2010.vcxproj`, `KTDXLIB/KTDXLIB_2010.vcxproj` | **`US_SERVICE`** | VS2010 (`v100`), Win32, Unicode |
| Client, engine, tools (only if asked) | (same three `.vcxproj`s) | `US_INTERNAL` | VS2010 (`v100`), Win32, Unicode |
| `luaLib`, `luajitLib` | (same solution) | **`Release`** | VS2010 (`v100`), Win32 |
| `X2ServerProtocol` | (same solution) | **`X2TOOL`** | VS2010 (`v100`), Win32 |
| The five servers | `X2Project_2003.sln` | `Release_US` | VS2003 (`v70`), Win32 — **not installed here** |

`US_SERVICE` was added directly to the three `.vcxproj` files, not to `X2Project_2010.sln`'s configuration list — that's cosmetic (only affects opening the solution in Visual Studio) since the build recipe below never goes through the `.sln`. If you need to select `US_SERVICE` inside Visual Studio itself, the solution-level config list needs the same addition first.

**The VS2010 C++ compiler is on `D:`, not `C:`** — `D:\Program Files\VS\Microsoft Visual Studio 10.0\` (registry `HKLM\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\SxS\VC7` → `10.0`). The `C:\Program Files (x86)\Microsoft Visual Studio 10.0\` tree holds only shell pieces and has no `VC` directory. **VS2003 is not installed at all** (no `SxS\VS7` entry for `7.1`), so the servers cannot be built in this environment — plan around that rather than discovering it mid-task.

**`msbuild` is not on `PATH`** in a Git Bash shell here — use the full path,
`/c/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe`
(`C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe`). Pass
`-m -p:MultiProcessorCompilation=true` on every invocation — the machine has
16 logical cores and none of the `.vcxproj`s set `MultiProcessorCompilation`
themselves, so without the property override `cl.exe` compiles one
translation unit at a time. `-m` alone does nothing extra here since each
command below builds a single project, not a solution graph — the property
override is what actually turns on `/MP`. Verified: a full `X2Lib` rebuild
(forced via `touch X2Lib/stdafx.cpp` to invalidate the PCH) completed in
~4 minutes at 16-way with zero errors, same output size as without the flag.

```sh
MSBUILD="/c/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe"
TRUNK="F:/elsword stuff/.../source/EU_CN_US/Trunk"   # must end in a slash when passed
MP="-m -p:MultiProcessorCompilation=true"

# Dependencies first, in this order. luajitLib and luaLib are Release; X2ServerProtocol is X2TOOL.
# Client config is US_SERVICE by default - substitute US_INTERNAL only if explicitly asked.
"$MSBUILD" luajitLib/luajitLib.vcxproj                    -p:Configuration=Release    -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" luaLib/luaLib_2010.vcxproj                     -p:Configuration=Release    -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" X2ServerProtocol/X2ServerProtocol_2010.vcxproj -p:Configuration=X2TOOL     -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" KTDXLIB/KTDXLIB_2010.vcxproj                   -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" X2Lib/X2Lib_2010.vcxproj                       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" X2/X2_2010.vcxproj                             -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
```

`SolutionDir` **must** be passed, with a trailing slash: every `IncludePath` entry for Boost, DXSDK, KNCSDK and freetype is written as `$(SolutionDir)Libs...`, so without it the build dies on `fatal error C1083: Cannot open include file: 'boost/shared_ptr.hpp'` or `'d3dx9.h'`. Use **forward slashes**, and put the trailing slash inside the value (`SolutionDir=F:/.../Trunk/`). A backslash path is an active trap: a trailing `\` before a closing quote escapes the quote, msbuild gets a mangled `SolutionDir`, and the build fails with `C1083: Cannot open include file: 'ImportKncSerializer.h'` — which reads like a missing KNCSDK include path in the project and is not.

Three things about that command list are load-bearing:

- **Never build the client from `X2Project_2010.sln`.** Its `US_INTERNAL` build sweeps in ~15 dead tool/server projects and never runs `X2_2010.vcxproj`, so it does not produce an exe. (`US_SERVICE` was never added to the `.sln` at all — see above — so this applies to it too, doubly.)
- **`X2ServerProtocol` is not a `ProjectReference` of `X2Lib`** (only `KTDXLIB` is), so it is never rebuilt implicitly — build it explicitly or you link a stale `X2Lib/X2ServerProtocol.lib`. It has **no `US_*` configuration at all**; the solution maps `US_INTERNAL` → `X2TOOL|Win32` (there is no `US_SERVICE` mapping since it's not in the `.sln` — irrelevant anyway, since the recipe above always builds `X2ServerProtocol` with an explicit `-p:Configuration=X2TOOL`, never through a solution-config mapping), whose `OutDir` is already `..\X2Lib\`.
- **`luajitLib` is required, not optional.** `_USE_LUAJIT_` is defined unconditionally (`KTDX.h:392-397`, via `X2OPTIMIZE_APPLY_UNPACK_HACK_PREVENTION`) and `KTDXApp.cpp:38` auto-links `luajitLib.lib` with a `#pragma comment`. Because `X2OPTIMIZE_REMOVE_LUA_INTERPRETER_MODULE` is *not* defined under `US_INTERNAL` or `US_SERVICE`, it must be the **full interpreter** build — `Release`, never `Release_NoInterpreter`. Its `Release` config outputs `..\KTDXLIB\luajitLib.lib`. `luaLib` still exists and still builds alongside it.

Build artifacts (default, `US_SERVICE`; substitute `US_INTERNAL` for the other config):

- `X2/US_SERVICE/x2.exe` — the client. Note the **lowercase** name: `TargetName` is `x2`.
- `X2/X2Lib.lib`, `X2/KTDXLIB.lib` — static libs; `OutDir` is `..\X2\`, only the `.obj` intermediates land in `X2Lib/US_SERVICE/` and `KTDXLIB/US_SERVICE/`. These are config-specific — a `US_INTERNAL` build and a `US_SERVICE` build each leave their own `.lib`, and the later build wins if you don't check which one you last ran.
- `KTDXLIB/luajitLib.lib`, `KTDXLIB/luaLib.lib`, `X2Lib/X2ServerProtocol.lib`.

**The post-build event no longer fails.** `X2/X2_2010.vcxproj:642` sets `PostBuildEventUseInBuild` to `false` for `US_INTERNAL` and (cloned identically) `US_SERVICE`, so the old copy-to-`E:\Elsword_InHouse` step never runs for either. The exit code is now meaningful — but still confirm `X2/US_SERVICE/x2.exe` was produced and is newly dated.

There is **no runnable test suite**. The one CppUnit fixture (`X2Lib/X2GameUnitTestCase.h`) is gated behind `CPPUNIT_BY_TOOL_TEAM`, commented out in `KTDXLIB/AlwaysButConditionally.h`. `Libs/InternalLib/KNCSDK/UnitTest/` is the vendored SDK's own test project, not wired into either solution.

### What `US_INTERNAL`/`US_SERVICE` actually define (client)

**As of this writing `US_SERVICE`'s `PreprocessorDefinitions` are identical to `US_INTERNAL`'s** (below) — the 2026-09-16 addition of `US_SERVICE` only changed the `ClCompile` optimization tags (see *Toolchains*), nothing in this table. `_IN_HOUSE_` is still defined under `US_SERVICE` too, so everything this section says about the in-house cheat/dev-UI surface currently applies to both configs equally. Stripping `_IN_HOUSE_` from `US_SERVICE` (and adding `AUTO_LOGIN_IN_HOUSE` directly, since it would otherwise stop arriving through `InHouse2.h`) is a deliberately separate, not-yet-done step — if and when that lands, update this section to describe the two configs' definitions separately.

| Project | Preprocessor definitions |
|---|---|
| `X2` | `WIN32;NDEBUG;_WINDOWS;SERV_COUNTRY_US;CLIENT_COUNTRY_US;_IN_HOUSE_;_USE_32BIT_TIME_T;_HAS_ITERATOR_DEBUGGING=0;_CONVERT_VS_2010` |
| `X2Lib` | `WIN32;NDEBUG;_LIB;_USE_32BIT_TIME_T;_IN_HOUSE_;SERV_COUNTRY_US;CLIENT_COUNTRY_US;_HAS_ITERATOR_DEBUGGING=0;_CONVERT_VS_2010` |
| `KTDXLIB` | `WIN32;NDEBUG;_LIB;SERV_COUNTRY_US;CLIENT_COUNTRY_US;_IN_HOUSE_;_HAS_ITERATOR_DEBUGGING=0;_USE_32BIT_TIME_T;_CONVERT_VS_2010` |
| `X2ServerProtocol` (`X2TOOL`) | `X2TOOL;WIN32;NDEBUG;_LIB;_USE_32BIT_TIME_T;_HAS_ITERATOR_DEBUGGING=0` |
| `luaLib`, `luajitLib`, `libxml` (`Release`) | region-neutral; `luajitLib` adds `_USE_LUAJIT_` |

All three client projects are `Unicode`, so `UNICODE`/`_UNICODE` come from `CharacterSet` rather than the definitions list — a hand-rolled `cl` invocation must pass them or `DXUT.h:11` raises an `#error`.

**`_IN_HOUSE_` is defined (under both `US_INTERNAL` and, for now, `US_SERVICE`), and that is the single biggest behavioural change from the old (pre-March-2014-snapshot) `US_SERVICE` build**, which did not define it. `KTDX.h:493` therefore includes all seven per-developer flag files (`InHouseEtc.h`, `InHouse1.h`–`InHouse6.h`), which had been dead code for this build's entire prior history. **Do not reason about these flags by reading the nested `#ifdef`s** — a naive grep for active `#define`s in those files reports 84, of which at least `MASS_FILE_FIRST` and `WORLD_TOOL` are false positives sitting inside dead conditionals. Resolve them with `cl /EP` on a probe that includes `KTDX.h`; the exact recipe and the full verdict table are in `MARCH_2014_MIGRATION.md`.

The results that matter:

- **`MASS_FILE_FIRST` is NOT defined**, and `WORLD_TOOL` is not either (so `KTDX.h:503-575`'s big `#undef` block does not fire). See the Lua rule above for what the missing `MASS_FILE_FIRST` does to loose-file precedence.
- **`_SERVICE_` and `_OPEN_TEST_` are NOT defined.** `X2Lib/define.h` still resolves the US patch source to `http://gamepatch.elswordonline.com/` with `PatchPath.dat`. `CX2Data::ResetServerProtocol` passes `bIsSERVICE = false`.
- **`AUTO_LOGIN_IN_HOUSE` is now live in `X2Lib`.** It used to be set on the `X2` project only, while every use site sits in `X2Lib` — so it was inert. It now arrives through `InHouse2.h:5` → `KTDX.h:493` → *every* client project, and `CX2StateLogin::ReadIDAndPassword` (`X2Lib/X2StateLogin.cpp:817`) really does read `LoginKey.lua` and auto-log-in.
- **`NOT_USE_UDP_CHECK_INHOUSE` is defined**, making `CX2Game::AbuserUserCheck` (`X2Lib/X2Game.cpp:16136`) return immediately.
- **`NO_GAMEGUARD` is defined**; `HACK_SHIELD` and `BUG_TRAP` are not.
- **The in-house cheat and dev-UI surface is compiled in** (`CHEAT_CLASS_CHANGE`, `ITEM_VIEW_CHEAT`, `MONSTER_TEST_DUNGEON`, `ADD_CREATE_CHARACTER_BUTTON`, `SHOW_ONLY_MY_DAMAGE`, `HIDE_SYSTEM_MESSAGE`, and ~15 more). Two that are *not* hazards despite their names: `RATE_MODIFIER_INHOUSE_TEST` has zero use sites in client code, and `DAMAGE_ZERO` is only `SetZeroDamage( NOWSTATE_FLAG( SET_ZERO_DAMAGE ) )` in `X2GUNPC.cpp`, i.e. driven by the NPC state table.

Server-side, `Release_US` defines only `WIN32;NDEBUG;_WINDOWS;SERV_COUNTRY_US` for all five executables. Note there is no `_SERVICE_` on the servers — live vs. test is a **runtime** choice, not a compile-time one (see below).

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
- **Client:** `KTDXLIB/KTDX.h` is the hub. `Always.h` and `AlwaysButConditionally.h` are always reachable, and under `US_INTERNAL` and (for now) `US_SERVICE` the seven `InHouse*.h` files are reachable too (see *Toolchains*). `X2Lib/X2Define.h` holds client tuning constants and the `XEM_ERROR*` code list. Several blocks in `KTDX.h` are explicitly `#ifndef _SERVICE_`, i.e. deliberately absent from this build.

When adding a flag, follow the local pattern: new `#define` at the end of the owning file, commented with author / date / description, and make sure it is defined for *every* project that compiles code guarded by it. For our own client changes the owning file is always `KTDXLIB/Always.h` and the name always starts with `SERV_IRUHADEV_` — see the rule at the top of this file.

### Offline mode

`SERV_IRUHADEV_OFFLINE` makes the client playable with **no servers running**: an in-process emulator answers the packets the five servers used to, and all player state persists to a local SQLite file, `els_db.sql`, in the game directory. It is client-only — it touches nothing under `KncWX2Server/Common/`, so the servers never need rebuilding for it.

**As of 2026-09-09 this mode is mid-migration and does not build.** The March 2014 snapshot (`dd63297`) overwrote every call site and dropped the 54 `Offline\` entries from `X2Lib/X2Lib_2010.vcxproj`, so the 55 files under `X2Lib/Offline/` are on disk but orphaned from the build, and the `g_pX2OfflineHook` definition is gone. `SERV_IRUHADEV` being absent from a studio file is therefore *expected*, not a bug. `MARCH_2014_MIGRATION.md` holds the phase plan, the per-file inventory and the re-application technique; the description below is of the design as it will be once restored. Design, phase history, and every correction the phases produced are in `OFFLINE_MODE_PLAN.md`; what has been changed and how to revert is in `MODS.md`. **Phase 3B (the `REFORM_ENTRY_POINT`/`SERV_ENTRY_POINT` character-select handshake) is done** — the emulator answers all three of its request packets, and character creation and character select work. A separate bug in the tutorial auto-connect that follows character creation was found and fixed 2026-09-15 (built and deployed, play-test pending); see `MARCH_2014_MIGRATION.md`'s Phase 3B section, "Auto-connect after character creation".

**The flag is defined in two places and both must be toggled together**: `KTDXLIB/Always.h` (the usual place) *and* `X2ServerProtocol/X2ServerProtocol_2010.vcxproj`'s **`X2TOOL|Win32`** `PreprocessorDefinitions` — that project has no `US_*` configuration, and the solution maps `US_INTERNAL` onto `X2TOOL`. `X2ServerProtocol` does not include `KTDX.h`, so `Always.h` alone does not reach the socket seam, and setting only one side fails silently rather than at compile time.

The code is all under `X2Lib/Offline/` — `X2OfflineServer.{h,cpp}` (the emulator and its dispatch table, ~200 packets), `Handlers_*.cpp` (the handlers, one file per subsystem), `X2OfflineDB.{h,cpp}` (SQLite; a versioned schema with a migration ladder), and one module per subsystem it had to reimplement (`X2OfflineInventory`, `X2OfflineQuest`, `X2OfflineTitle`, `X2OfflineSkill`, `X2OfflineCashShop`, `X2OfflineStatTable`, `X2OfflineDropTable`, `X2OfflineBattleField`, `X2OfflineRandomItem`, `X2OfflinePetData`).

Three seams, and there are only three:

1. **`X2ServerProtocol/Socket/Session.cpp`** — the one that matters. `KSession` hands every outbound `KEvent` to `g_pX2OfflineHook->OnClientSend()` instead of serializing it to a socket, and the hook answers by calling `KSession::QueueingEvent()` on the same session — the exact point the real receive path queues into. Everything above the socket layer therefore runs completely unmodified. The hook interface is `X2ServerProtocol/OfflineHook.h`, declared there so the socket layer does not depend on `X2Lib`.
2. **`X2Lib/X2Data.cpp:2136`** — `ResetServerProtocol()` stands the emulator up and registers it before any proxy can connect. It outlives every `CX2ServerProtocol` on purpose: the client drops and rebuilds its proxies several times per session.
3. **A handful of client-side revivals** — code the shipping client contains but never calls, because the *server* used to do that job. `X2DungeonSubStage.cpp` (the static-NPC parser), `X2QuestManager.cpp` (`m_iAfterQuestID`, which is a table the client never read), `X2StateServerSelect.cpp` (the UDP port check has no server to echo off). These are revivals, not rewrites.

**Launching it**: `X2_offline.exe` plus `start_offline.bat` in the game directory. `SERV_IRUHADEV_NO_PATCHER_TOKEN` (a separate flag, independent of offline mode) means the exe no longer needs the `PATCHER_RUN_ONLY` token in `argv[1]` and can be started directly — without it a bare launch returns 0 out of `WinMain` with no window and no log. The `.bat` is still preferred because it does `cd /d "%~dp0"`: the working directory must be the game data directory, since `X2Main` mounts the `.kom` archives through a `"./"` prefix and the offline server writes `els_db.sql` and both logs there.

**Debugging it is done by reading two log files**, both written into the game directory and both flushed after every line: `offline_server.log` (lifecycle, errors, and one line per interesting decision) and `offline_packets.log` (one line per packet, both directions, tagged `HANDLED` / `--- IGNORED ---` / `*** UNHANDLED ***` / `*** EXCEPTION - ROLLED BACK ***`). The core loop for any new work is: play until something breaks, `grep UNHANDLED offline_packets.log`, implement, repeat. Packets that are deliberately not implemented are labelled `IGNORED` with a reason from `X2OfflineIgnore.cpp`, so the `UNHANDLED` bucket stays empty and any line in it is new information. On a clean exit the same file's census is written to `offline_server.log`, naming every event ID the dispatch declined and how often.

Two invariants worth knowing before changing anything in here:

- **One packet is one transaction.** `OnClientSend` opens a SQLite savepoint before dispatch and commits it only if the handler returned without faulting, and it runs the whole dispatch under `m_csDispatch` because there is one `KSession::Run` thread per proxy. Anything inside `X2OfflineDB` that runs during play must use `Begin()`/`Commit()`/`Rollback()`, never a literal `BEGIN` — the literals left in `Migrate()` are safe only because it runs from `Open()`, before the first packet.
- **`sizeof(KSession)` differs between `X2Lib` and `X2ServerProtocol`**, because `ADD_COLLECT_CLIENT_INFO_PROTOCOL` is defined in `X2ServerProtocol/X2ServerProtocol.h:8` and `X2ServerProtocolLib.h:9`, which `X2ServerProtocol` sees before the header while `X2Lib` reaches it earlier, without the macro. Never read a `KActorProxy`/`KUserProxy` member from `X2Lib`; every member reads 4 bytes low. `CX2OfflineServer::KindFromEventID` exists specifically to avoid needing to.

### Deploying the offline client

**This branch (`mods/offline-mod-2`) exists to build and run the offline client.** A change is not finished when it compiles; it is finished when the exe in the game directory has it and the logs show it working. The whole loop:

```sh
MSBUILD="/c/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe"         # not on PATH here
TRUNK="F:/elsword stuff/.../source/EU_CN_US/Trunk"                   # this repo
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/24965799/data"   # the game directory
MP="-m -p:MultiProcessorCompilation=true"                            # 16 cores, see *Toolchains*

# 1. build - dependencies, then X2Lib, then the exe. See *Toolchains* for the
#    full ordered list and why each config is what it is. NEVER build via
#    X2Project_2010.sln: its US_INTERNAL build sweeps in ~15 dead tool/server
#    projects and never runs X2_2010.vcxproj, so it produces no exe (US_SERVICE
#    isn't even in the .sln's config list at all - see *Toolchains*). Client
#    config is US_SERVICE by default - substitute US_INTERNAL only if asked.
"$MSBUILD" X2ServerProtocol/X2ServerProtocol_2010.vcxproj -p:Configuration=X2TOOL      -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" KTDXLIB/KTDXLIB_2010.vcxproj                   -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" X2Lib/X2Lib_2010.vcxproj                       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP
"$MSBUILD" X2/X2_2010.vcxproj                             -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/" $MP

# 2. confirm the artifact. The post-build event is disabled under US_SERVICE (and
#    US_INTERNAL), so the exit code is meaningful now - but still check the file and its date.
ls -la X2/US_SERVICE/x2.exe

# 3. deploy - the game directory names it X2_offline.exe
cp X2/US_SERVICE/x2.exe "$DATA/X2_offline.exe"

# 4. run, then read the logs it wrote next to itself
cd "$DATA" && ./start_offline.bat
```

Five things that go wrong at deploy time, all of them silently:

- **Deploy onto the exact name already in the game directory, and confirm it rather than assuming it.** It is `X2_offline.exe`, which is also the name `start_offline.bat` looks for. The source artifact is `x2.exe` **lowercase** out of `X2/US_SERVICE/` (or `X2/US_INTERNAL/` if that config was built instead - the two do not share an output directory, so check which one you actually built), and the old game directory holds half a dozen other `X2_*.exe` builds from earlier mods, so a near-miss name silently leaves the old exe in place and the next play-test measures stale code. `ls` in a terminal is not proof — read the name programmatically (`python -c "import os; print(os.listdir(DATA))"`) before claiming anything about it, and check the size and mtime of what you copied. A build that did not land is indistinguishable from a change that did not work.
- **A stale PCH silently discards header edits.** `Always.h` and `X2Define.h` sit inside every project's precompiled header, and msbuild does not always notice. The flag is then simply absent at the call site with no error — the `#else` branch compiles and the change looks like it did nothing. After editing any header, `touch X2Lib/stdafx.cpp` before building, and if a change appears to have no effect, **prove the code compiled in** rather than re-reading the `#ifdef`s: put a `#pragma message` inside the guard, rebuild that one project, and read the compiler output.
- **Windows Defender quarantines fresh builds.** A newly linked `x2.exe` trips a Bearfoos ML false positive. If the copy or the launch fails with no obvious reason, check Protection History; the fix is folder exclusions for the build output and the game directory, which needs an admin.
- **The working directory must be the game directory.** `X2Main` mounts the `.kom` archives through a `"./"` prefix, and the offline server writes `els_db.sql` and both logs relative to the cwd. Launching from anywhere else finds no content, or quietly starts a second empty save somewhere surprising. `start_offline.bat` does `cd /d "%~dp0"` for exactly this; a shortcut with a different *Start in* field does not.
- **The save file is real player data.** `els_db.sql` (plus `-wal`/`-shm`, and the `els_db.sql.bak` written on a clean exit) lives in the game directory and is the only copy of the character. Never delete it to "start clean" without asking, and never assume a schema change is reversible — `X2OfflineDB` migrates forward only. The pre-March save still sits in the **old** directory, `...\237311\22191271\data\els_db.sql`; migrating it is Phase 7 of `MARCH_2014_MIGRATION.md`.

**Reading the result** is the actual verification; there is no test suite. Both logs are in the game directory and flushed per line:

```sh
cd "$DATA"
grep -E "UNHANDLED|EXCEPTION" offline_packets.log | sort | uniq -c | sort -rn
grep "CENSUS" offline_server.log          # clean-exit summary of every declined id
tail -50 offline_server.log
sqlite3 els_db.sql "select unit_uid, nickname, level from unit;"
```

**When a gameplay change cannot be verified by reading code**, add a temporary diagnostic rather than reasoning further: `CX2OfflineLog::Server( L"..." )` writes a printf-style wide line into `offline_server.log`, is flushed immediately, and is available anywhere in `X2Lib`. Gate it behind its own short-lived `SERV_IRUHADEV_*_DEBUG` flag so it is one `#define` to remove, tag the lines with a grep-able prefix, and throttle per-frame logging to about once a second. This is how a rate that "should" have applied gets settled in one play-test instead of three.

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
