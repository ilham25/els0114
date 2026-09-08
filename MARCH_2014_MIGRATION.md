# Migrating the offline mod onto the March 2014 client

Status board and technique reference for the migration begun 2026-09-09.
**Read this before touching any studio file.** Every phase below is startable in
a fresh conversation; `CLAUDE.md` carries the build commands.

| Phase | What | State |
|---|---|---|
| 0 | Baseline, build wiring, docs | **DONE** 2026-09-09 |
| 1 | Flag definitions and tuning constants | not started |
| 2 | Offline build wiring + the socket seam | not started |
| 3 | Emulator standup + client revivals | not started |
| 4 | Offline gameplay call sites | not started |
| 5 | AI party | not started |
| 6 | The QoL flags | not started |
| 7 | Content, save migration, full verification | not started |
| 8 | Documentation closeout | not started |

## What happened

Commit `dd63297` ("update client from jan 2014 source to march 2014 source",
2,350 files) replaced the client source tree with a March 2014 snapshot by
**overwriting**, not merging. Every mod edit that lived inside a studio-owned
file is gone; the mod's own files survived untouched.

`SERV_IRUHADEV` now appears in **zero studio files**. Lost:

- All 13 flag `#define`s in `KTDXLIB/Always.h`, plus `STATIC_AUTO_LOGIN` in
  `KTDXLIB/OnlyGlobal/Always_US.h`.
- Every call site in 41 studio files.
- `X2Lib/X2Lib_2010.vcxproj`'s 54 `Offline\` entries, the `sqlite3.c` compile
  entry and its include path.
- `X2ServerProtocol_2010.vcxproj`'s `SERV_IRUHADEV_OFFLINE` (its whole
  `US_SERVICE` config is gone).
- The `g_pX2OfflineHook` **definition** (was `Session.cpp:15`). `X2Lib` cannot
  link without it, even with `OfflineHook.h` intact.

Survived intact: all 55 files in `X2Lib/Offline/` (~39,300 lines),
`X2ServerProtocol/OfflineHook.h`, `X2Lib/Offline/start_offline.bat`, all of
`X2CashShopTool/`, every `.md`.

Arithmetic of the restore set: **40** files lost `SERV_IRUHADEV`, **+1**
`Always_US.h`, **+1** `X2Lib/X2Lib_2010.vcxproj` (holds no flag text, only the
source list) = **42**, minus **3** server files now out of scope =
**39 files to restore**.

## Scope decisions (taken 2026-09-09)

- **All 14 flags** are restored.
- **The server side is dropped entirely** - `KncWX2Server/Common/ServerDefine.h`,
  `GameServer/UserSkillTree.cpp`, `GameServer/Inventory.cpp` and the VS2010
  server port. Offline mode needs no servers, and the client halves of those
  three flags take their definitions from `Always.h`. Recoverable any time from
  `git show c4495a3:<path>`.
- **The existing save is migrated** with a backup (Phase 7).

## The two traps

Three-way merge base is `BASE=7b7e482` (January import), `OURS=c4495a3`
(January + mods), `THEIRS=HEAD` (March).

**Trap 1 - encoding divergence re-corrupts CP949.** For 18 files `OURS` is
UTF-8 while `HEAD` is CP949 (`ISO-8859` / `Non-ISO`) - the corruption
`CLAUDE.md` warns about, already committed in the January lineage. Merging
`OURS` into `HEAD` writes mojibake over clean Korean bytes. **March holds the
good bytes.**

Worst case is `X2Lib/X2Define.h`: 53 conflicts and a +480/-419 mod diff, but its
*real* mod content is five small `const` blocks (was `:1801-1858`). Everything
else is encoding noise. **Do not apply 480 lines to it.**

**Trap 2 - three flags live in the merge BASE, so a clean merge silently
deletes them.** `SKILLTREE_NO_LOCK`, `SKILL_SLOT_B_FREE` and
`BUFF_DURATION_TEXT` were already in the import commit, so `git merge-file` sees
"unchanged in base, removed in theirs" and drops them **with zero conflicts**.
Ten files are marked `base-only` in the table below.

Both traps land on nearly the same set, so one handling covers both.

## The three lanes

| Lane | Files | How |
|---|---|---|
| **MERGE** | 17 | `git merge-file -p --diff3 ours base theirs`, then write the result. |
| **MERGE+FIX** | 6 | Same, then resolve 1-2 conflict hunks by hand. |
| **BYTE-HAND** | 18 | **Never merge.** Extract the mod blocks from `OURS` (ASCII by house rule) and insert them into `HEAD`'s bytes with a Python `rb`/`wb` script anchored on ASCII context. Where a mod has an `#else` branch holding original studio code, take that code from **`HEAD`**, not from `OURS`. |

Lane assignment was **validated empirically**: all 23 MERGE / MERGE+FIX files
were merged and their `SERV_IRUHADEV_*` token sets compared before and after -
none loses a flag (`X2Game.cpp` keeps all 5, `X2GageManager.cpp` all 3). Trap 2
is fully contained in the BYTE-HAND lane.

## The verification gate - run per file, every phase

```sh
# 1. encoding must still match the March bytes
file <path>                     # ISO-8859 / Non-ISO, NOT "UTF-8"
# 2. diff size must match the mod, not the file
git diff --stat -- <path>
# 3. Trap-2 detector: the flag-token set must survive. Must print nothing.
diff <(git show c4495a3:<path> | grep -o 'SERV_IRUHADEV_[A-Z0-9_]*' | sort -u) \
     <(grep -o 'SERV_IRUHADEV_[A-Z0-9_]*' <path> | sort -u)
```

`MODS.md`'s line numbers are **all stale** - `ResetServerProtocol` alone moved
2135 to 1920. **Re-locate every edit by symbol name, never by line number.**
`git show dd63297^:<path>` is the source of truth for the removed text.

## Per-file inventory

`Confl` = conflict hunks from a `--diff3` merge. `Mod size` = `git diff
BASE..OURS`; `base-only` means the mod arrived with the import (Trap 2).
`March churn` = `git diff BASE..THEIRS`, i.e. how much the upgrade moved the
file. Phase `out` = out of scope.

| # | File | Flags | Lane | Confl | Mod size | March churn | Phase |
|---|---|---|---|---|---|---|---|
| 1 | `KTDXLIB/Always.h` | AIPARTY_PERSIST, BUFF_DURATION_TEXT, JOBCHANGE_PORTRAIT, LEVEL_CAP_80, MP_REGEN_BOOST, NO_PATCHER_TOKEN, OFFLINE, OFFLINE_DROP_BOOST, OFFLINE_EXP_BOOST, OFFLINE_FETCH_AURA_ALWAYS, QUICK_SLOT_FULL_FREE, SKILLTREE_NO_LOCK, SKILL_SLOT_B_FREE | BYTE-HAND | 1 | +189/-0 | +2191/-1235 | 1 |
| 2 | `KTDXLIB/OnlyGlobal/Always_US.h` | STATIC_AUTO_LOGIN | BYTE-HAND | 0 | base-only | +261/-246 | 1 |
| 3 | `X2Lib/X2Define.h` | MP_REGEN_BOOST, OFFLINE, OFFLINE_DROP_BOOST, OFFLINE_EXP_BOOST | BYTE-HAND | 53 | +480/-419 | +389/-142 | 1 |
| 4 | `X2ServerProtocol/Socket/Session.cpp` | OFFLINE | MERGE | 0 | +56/-0 | +38/-2 | 2 |
| 5 | `X2ServerProtocol/X2ServerProtocol_2010.vcxproj` | OFFLINE | MERGE+FIX | 1 | +3/-2 | +312/-551 | 2 |
| 6 | `X2Lib/X2DungeonSubStage.cpp` | OFFLINE | MERGE | 0 | +29/-0 | +207/-120 | 3 |
| 7 | `X2Lib/X2QuestManager.cpp` | OFFLINE | MERGE+FIX | 2 | +50/-0 | +302/-239 | 3 |
| 8 | `X2Lib/X2QuestManager.h` | OFFLINE | MERGE | 0 | +18/-0 | +16/-10 | 3 |
| 9 | `X2Lib/X2StateServerSelect.cpp` | OFFLINE | MERGE+FIX | 1 | +27/-0 | +4795/-1071 | 3 |
| 10 | `X2Lib/X2Data.cpp` | OFFLINE | MERGE+FIX | 1 | +46/-1 | +710/-566 | 3 + 5 |
| 11 | `X2Lib/X2Room.cpp` | OFFLINE | MERGE | 0 | +10/-0 | +211/-200 | 4 |
| 12 | `X2Lib/X2Room.h` | OFFLINE | MERGE | 0 | +15/-0 | +15/-21 | 4 |
| 13 | `X2Lib/X2StateBeginning.cpp` | OFFLINE | MERGE | 0 | +15/-0 | +204/-178 | 4 |
| 14 | `X2Lib/X2TitleManager.cpp` | OFFLINE | MERGE | 0 | +24/-0 | +261/-249 | 4 |
| 15 | `X2Lib/X2TitleManager.h` | OFFLINE | MERGE | 0 | +11/-0 | +16/-12 | 4 |
| 16 | `X2Lib/X2DungeonGame.cpp` | AIPARTY_PERSIST, OFFLINE | MERGE+FIX | 1 | +77/-0 | +199/-179 | 4 + 5 |
| 17 | `X2Lib/X2Game.cpp` | AIPARTY_PERSIST, OFFLINE | MERGE | 0 | +927/-2 | +2758/-1854 | 4 + 5 |
| 18 | `X2Lib/X2Game.h` | AIPARTY_PERSIST, OFFLINE | MERGE | 0 | +84/-0 | +223/-78 | 4 + 5 |
| 19 | `X2Lib/X2GageManager.cpp` | MP_REGEN_BOOST, OFFLINE | MERGE | 0 | +66/-0 | +239/-235 | 4 + 6 |
| 20 | `X2Lib/X2GageUI.h` | BUFF_DURATION_TEXT, OFFLINE | BYTE-HAND | 0 | +18/-0 | +260/-133 | 4 + 6 |
| 21 | `X2/X2.cpp` | NO_PATCHER_TOKEN | MERGE | 0 | +24/-0 | +154/-97 | 6 |
| 22 | `X2Lib/X2BuffFinalizerTemplet.h` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +168/-107 | 6 |
| 23 | `X2Lib/X2BuffTemplet.cpp` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +357/-209 | 6 |
| 24 | `X2Lib/X2BuffTemplet.h` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +331/-153 | 6 |
| 25 | `X2Lib/X2CashShop.cpp` | QUICK_SLOT_FULL_FREE | MERGE+FIX | 1 | +6/-0 | +2104/-879 | 6 |
| 26 | `X2Lib/X2GUUser.cpp` | MP_REGEN_BOOST | MERGE | 0 | +25/-0 | +7257/-3898 | 6 |
| 27 | `X2Lib/X2GageUI.cpp` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +3837/-3470 | 6 |
| 28 | `X2Lib/X2GameUnit.cpp` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +2726/-1913 | 6 |
| 29 | `X2Lib/X2PremiumBuffManager.cpp` | BUFF_DURATION_TEXT | BYTE-HAND | 0 | base-only | +274/-205 | 6 |
| 30 | `X2Lib/X2UIQuestNew.cpp` | JOBCHANGE_PORTRAIT | BYTE-HAND | 0 | +68/-0 | +420/-239 | 6 |
| 31 | `X2Lib/X2UIQuickSlot.cpp` | QUICK_SLOT_FULL_FREE | MERGE | 0 | +14/-0 | +94/-140 | 6 |
| 32 | `X2Lib/X2UISkillTree.h` | SKILL_SLOT_B_FREE | BYTE-HAND | 0 | base-only | +27/-35 | 6 |
| 33 | `X2Lib/X2UISkillTreeNew.cpp` | SKILLTREE_NO_LOCK | BYTE-HAND | 1 | +6/-0 | +2211/-1526 | 6 |
| 34 | `X2Lib/X2UISkillTreeNew.h` | SKILL_SLOT_B_FREE | BYTE-HAND | 0 | base-only | +113/-95 | 6 |
| 35 | `X2Lib/X2Unit.cpp` | QUICK_SLOT_FULL_FREE | MERGE | 0 | +6/-0 | +1020/-416 | 6 |
| 36 | `X2Lib/X2Unit.h` | QUICK_SLOT_FULL_FREE | MERGE | 0 | +6/-0 | +1037/-172 | 6 |
| 37 | `X2Lib/X2UserSkillTree.cpp` | SKILL_SLOT_B_FREE | BYTE-HAND | 0 | base-only | +801/-356 | 6 |
| 38 | `X2Lib/X2UserSkillTree.h` | SKILL_SLOT_B_FREE | BYTE-HAND | 0 | base-only | +195/-121 | 6 |
| 39 | `KncWX2Server/Common/ServerDefine.h` | QUICK_SLOT_FULL_FREE, SKILLTREE_NO_LOCK, SKILL_SLOT_B_FREE | BYTE-HAND | 1 | +7/-0 | +1621/-1260 | out |
| 40 | `KncWX2Server/GameServer/Inventory.cpp` | QUICK_SLOT_FULL_FREE | MERGE | 0 | +15/-1 | +788/-222 | out |
| 41 | `KncWX2Server/GameServer/UserSkillTree.cpp` | SKILLTREE_NO_LOCK, SKILL_SLOT_B_FREE | BYTE-HAND | 1 | +14/-8 | +1999/-624 | out |

`X2Lib/X2Lib_2010.vcxproj` is the 42nd file and is not in the table: it carries
no flag text, only the 54 `Offline\` source-list entries plus `sqlite3.c`. It is
Phase 2's work.

## What Phase 0 established

### The toolchain actually installed here

VS2010 with the C++ compiler lives on **`D:`**, not `C:`:
`D:\Program Files\VS\Microsoft Visual Studio 10.0\` (registry
`HKLM\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\SxS\VC7` -> `10.0`).
`C:\Program Files (x86)\Microsoft Visual Studio 10.0\` holds only shell pieces
and has **no `VC` directory** - `cl.exe` is not there.

**VS2003 is not installed.** There is no `SxS\VS7` entry for `7.1`. So although
the servers are now VS2003-of-record, they cannot be built in this environment
at all. That is fine, since servers are out of scope, but do not plan around
building them here.

### `US_INTERNAL` resolved by preprocessor, not by reading

Reading nested `#ifdef`s got this wrong twice. A naive grep for active
`#define`s in `InHouse*.h` reported 84, but `MASS_FILE_FIRST`
(`InHouseEtc.h:296`, inside a dead `#ifdef LOADREALFILE_ERROR_LOG`) and
`WORLD_TOOL` (`InHouseEtc.h:410`, inside a dead
`#if defined(EFFECT_TOOL) || defined(X2VIEWER)`) are both **false positives**.

The reliable method is a probe that `#include`s `KTDX.h` and emits a quoted
marker per flag, run through `cl /EP` with the project's real include paths and
defines. Quote the flag names, or an `ON` result expands to nothing:

```sh
cl /nologo /EP /I. /I.\DXUT /I..\X2ServerProtocol /I..\X2ServerProtocol\FSM \
   /I..\X2ServerProtocol\Socket /I.\CxImage /I..\libxml \
   /I<Trunk>\Libs\ExternalLib\DXSDK\Include /I<Trunk>\Libs\ExternalLib\Boost\boost_1_44_0 \
   /I<Trunk>\KNCSDK\Include_2010 /I<Trunk>\KOGGamePerformanceCheck\include \
   /I<Trunk>\Libs\ExternalLib\freetype-2.4.11\include /I<Trunk>\XTRAP\Client \
   /I<Trunk>\Steamworks \
   /DWIN32 /DNDEBUG /D_LIB /DSERV_COUNTRY_US /DCLIENT_COUNTRY_US /D_IN_HOUSE_ \
   /D_HAS_ITERATOR_DEBUGGING=0 /D_USE_32BIT_TIME_T /D_CONVERT_VS_2010 \
   /DUNICODE /D_UNICODE probe.cpp
```

`UNICODE` / `_UNICODE` are required or `DXUT.h:11` raises an `#error`.

Verdict for a `US_INTERNAL` client build:

| Flag | State | Why it matters |
|---|---|---|
| `_IN_HOUSE_` | **ON** | all seven `InHouse*.h` are live for the first time |
| `_SERVICE_`, `_OPEN_TEST_` | OFF | |
| `_ALWAYS_` | ON | `Always.h` + `AlwaysButConditionally.h` are still the flag home |
| `MASS_FILE_FIRST` | **OFF** | loose-file precedence inverted, see below |
| `WORLD_TOOL` | OFF | so `KTDX.h:503-575`'s big `#undef` block does **not** fire |
| `_USE_LUAJIT_` | ON | `KTDXApp.cpp:38` auto-links `luajitLib.lib` |
| `X2OPTIMIZE_REMOVE_LUA_INTERPRETER_MODULE` | OFF | so the **full** interpreter build of luajit is needed: `Release`, not `Release_NoInterpreter` |
| `_ENCRIPT_SCRIPT_` | ON | scripts are always XOR-encrypted |
| `NO_GAMEGUARD` | ON | `HACK_SHIELD` OFF, `BUG_TRAP` OFF |
| `AUTO_LOGIN_IN_HOUSE` | **ON** | see below |
| `NOT_USE_UDP_CHECK_INHOUSE` | **ON** | see below |
| `RATE_MODIFIER_INHOUSE_TEST` | ON but **inert** | zero use sites in `X2Lib` / `X2` / `KTDXLIB`, so it does *not* collide with our EXP/drop boost |
| `DAMAGE_ZERO` | ON but harmless | only in `X2GUNPC.cpp`, as `SetZeroDamage( NOWSTATE_FLAG( SET_ZERO_DAMAGE ) )` - driven by the NPC state table, not a blanket zero |
| cheats / dev UI | ON | `CHEAT_CLASS_CHANGE`, `ITEM_VIEW_CHEAT`, `ERASE_BUFF_CHEAT`, `ITEM_CHEAT_POPUP_TEST`, `ADD_CREATE_CHARACTER_BUTTON`, `EXPAND_DEVELOPER_SCRIPT`, `REALTIME_SCRIPT_PATCH`, `FREE_SELECT_PVP_MAP`, `MONSTER_TEST_DUNGEON`, `BATTLE_FIELD_TEST`, `SHOW_ONLY_MY_DAMAGE`, `HIDE_SYSTEM_MESSAGE`, `ADD_SHORT_CUT_KEY_IN_HOUSE`, `CHEAT_COMMAND_SHORT_CUT_KEY_TEST`, `AUTH_DELETE_ITEM_ALL`, `AUTH_AUTO_SELL_ITEM`, `JUNK_AVATAR`, `FIRST_SCREEN_TEST`, `NOT_RENDER_NPC_GAME_EDIT`, `SHOW_ATTACK_BOX_DUMMY`, `DROP_FILE_TEST`, `DISCONNECT_DISENABLE_DLGUI` |

### Three `_IN_HOUSE_` consequences that touch the mod work

1. **`AUTO_LOGIN_IN_HOUSE` is live in `X2Lib` for the first time.**
   `CLAUDE.md` used to say it was inert, because it was set on the `X2` project
   only while its use sites live in `X2Lib`. It now arrives via
   `InHouse2.h:5` -> `KTDX.h:493` -> every client project.
   `CX2StateLogin::ReadIDAndPassword` (`X2Lib/X2StateLogin.cpp:817`) reads
   `LoginKey.lua` through the mass-file manager and auto-logs in.
   **Phase 1 must decide whether `STATIC_AUTO_LOGIN` is still wanted** - it may
   now be redundant, and `MODS.md` already lists it as a deviation to clean up.

2. **`NOT_USE_UDP_CHECK_INHOUSE` neuters the UDP abuser kick.**
   `CX2Game::AbuserUserCheck` (`X2Lib/X2Game.cpp:16136`) returns immediately.
   This is adjacent to, but not the same site as, the offline mod's UDP
   port-check revival in `X2StateServerSelect.cpp`. **Phase 3 should check
   whether that revival is still needed** before re-applying it blindly.

3. **The in-house cheat surface is now compiled in.** Not a hazard, but it
   changes what a play-test sees versus the old `US_SERVICE` build. Keep it in
   mind when a Phase 4-6 verification looks wrong: `SHOW_ONLY_MY_DAMAGE` and
   `HIDE_SYSTEM_MESSAGE` in particular alter what is on screen.

### Loose-file precedence has inverted

From `KGCMassFileManager::LoadDataFile`
(`KTDXLIB/KGCMassFileManager.cpp:878-941`, `#else` at `:915`):

- **With** `MASS_FILE_FIRST` (January): the `.kom` map first, then
  `LoadRealFile` only if absent **and** `_SERVICE_` is undefined (`:890`). The
  old `US_SERVICE` build *did* define `_SERVICE_`, so the loose-file fallback
  was **compiled out entirely** - contrary to what `CLAUDE.md` claimed.
- **Without** it (March / `US_INTERNAL`): `:916` calls `LoadRealFile`
  **unconditionally first**. A loose file on disk beats the archive, always.

So the Lua rule's hazard is strictly worse now: a loose `.lua` beside the exe
silently overrides a stale or failed repack. A miss logs `KEM_ERROR135` under
`MASS_FILE_FIRST` and `KEM_ERROR136` without it.

### Build wiring changed in Phase 0

- `.gitignore`: added `!/luajitLib/` (the March snapshot's new project was
  untracked while the committed `.sln` referenced it), plus `US_INTERNAL/`,
  `X2TOOL/`, `X2TOOL_Release/` and `X2TOOL_Debug/` to the build-output ignores.
  Note `X2/US_INTERNAL/x2.exe.manifest` and `.intermediate.manifest` were
  already committed by `dd63297` and stay tracked.
- `X2Project_2010.sln`: luajitLib `{7F92974D-...}` `US_INTERNAL|Win32` remapped
  from `X2TOOL_Release|Win32` to **`Release|Win32`** (`ActiveCfg` + `Build.0`,
  and the `Any CPU` `ActiveCfg`). It was building `luajit.exe` into
  `C:\buildtool\nant-0.94\bin` instead of `..\KTDXLIB\luajitLib.lib`.
  `Mixed Platforms` already pointed at `Release`, which corroborates the intent.

### Build commands verified, and one shell trap

Two of the commands in `CLAUDE.md`'s *Toolchains* list were actually run in
Phase 0 rather than merely written down, because both configs are new:

- `X2ServerProtocol` at `Configuration=X2TOOL` builds clean (16 sources) and
  outputs to `X2Lib\X2ServerProtocol.lib`, which is where `X2Lib` expects it.
- `luajitLib` at `Configuration=Release` builds clean and outputs to
  `KTDXLIB\luajitLib.lib` (2,941,644 bytes). Only a `sprintf` deprecation
  warning in `lj_str.c:180`. The previous artifact was 2,895,220 bytes from the
  Aug 30 build, so the library is not bit-reproducible across builds - do not
  treat a size change as a problem on its own.

**The shell trap, which cost one false failure:** passing
`"/p:SolutionDir=%T%\"` from a `.bat` where `%T%` is a backslash path makes the
trailing `\` escape the closing quote, so msbuild receives a mangled
`SolutionDir` and the build dies on `fatal error C1083: Cannot open include
file: 'ImportKncSerializer.h'`. That error looks exactly like a missing KNCSDK
include path in the config - it is not. Pass a **forward-slash** path with the
trailing slash inside the value (`SolutionDir=F:/.../Trunk/`), which is what
`CLAUDE.md` recommends and what both verified commands use.

### Stale dead-config build output: a grep hazard, not a link hazard

1.4 GB of intermediates survive for configurations that no longer exist -
`X2/US_SERVICE` (471M), `X2Lib/US_SERVICE` (650M), `KTDXLIB/US_SERVICE` (135M),
`X2ServerProtocol/US_SERVICE` and `US_TEST` (78M each) - plus `ipch/`. They are
from the last pre-upgrade build and still contain the string `SERV_IRUHADEV`
inside `.pch`, `.obj` and `.pdb` files, including 32 offline `.obj`s in
`X2Lib/US_SERVICE`.

**They cannot corrupt a build.** All three client projects set
`IntDir = $(Configuration)\` for `US_INTERNAL`, so a `US_INTERNAL` build reads
and writes only `US_INTERNAL/`; the `US_SERVICE/` tree is unreachable. The two
artifacts that *do* live at a config-independent path, `X2/X2Lib.lib` and
`X2/KTDXLIB.lib` (`OutDir` is `..\X2\` for every config), are both dated
Aug 30 and therefore come from the March `US_INTERNAL` build, not from a modded
one. So there is no stale-object link risk and no reason to purge the tree
before starting.

What they *do* break is **a repo-wide grep**. `grep -r SERV_IRUHADEV` returns
these binaries, so "the flag still appears in the tree" is not evidence the mod
is present. Scope flag greps to source, e.g.
`git grep SERV_IRUHADEV` or `--include=*.cpp --include=*.h`. The per-file
verification gate above is already safe because it greps named files.

The PCH hazard that *is* real is the ordinary one `CLAUDE.md` documents, and it
concerns the **live** config's precompiled headers - `X2Lib/US_INTERNAL/X2Lib.pch`
(286 MB), `KTDXLIB/US_INTERNAL/KTDXLIB.pch`, `X2/US_INTERNAL/x2.pch`. After
editing `Always.h` or `X2Define.h`, `touch X2Lib/stdafx.cpp` and prove the flag
compiled in with a `#pragma message`.

Deleting the dead `US_SERVICE`/`US_TEST` output directories and `ipch/` is safe
housekeeping (all gitignored and regenerable) and would reclaim ~1.4 GB, but it
is not a prerequisite for any phase.

### Known-good baseline

`X2/US_INTERNAL/x2.exe` - 20,498,432 bytes, 2026-08-30 18:20 - is
byte-identical to the `x2.exe` deployed in
`F:\elsword stuff\elsword_2014\els_2014\237311\24965799\data`. The vanilla March
client builds and runs. **Any breakage after Phase 0 is ours.**

## Phase 7 owes the user a repack

The new game directory's `data036.kom` is stock - 5,411,981 bytes (Aug 27)
against the old directory's 23,106,615. Every server-side table the offline mode
reads is therefore absent and will degrade-to-off with a log line. The offline
loaders name these 16 files:

`StatTable.lua`, `NpcExpTable.lua`, `DropTable.lua`, `StaticDropTable.lua`,
`BattleFieldServerData.lua`, `BattleFieldData.lua`, `EnchantTable.lua`,
`AttribAttachTable.lua`, `AttribEnchantTable.lua`, `ResolveTable.lua`,
`SocketItemTable.lua`, `RandomItemTable.lua`, `RandomItemData.lua`,
`PetData.lua`, `RidingPetData.lua`, `MapData.lua`

The server-owned ones live in `KncWX2Server/ServerResource/US/` and need
XOR-encrypting and packing. Confirm from each loader's own "loaded N row(s)"
line, never from the build succeeding. Also re-apply the solo-play tuning edit
to `KncWX2Server/ServerResource/US/BattleFieldServerData.lua`, which `dd63297`
overwrote - preserving its UTF-8 BOM and CRLF byte-for-byte.
