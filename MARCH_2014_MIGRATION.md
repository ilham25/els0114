# Migrating the offline mod onto the March 2014 client

Status board, per-phase execution plan, and technique reference for the
migration begun 2026-09-09. **Read this before touching any studio file.**
Every phase below is startable in a fresh conversation - see *Starting a
phase* for the preamble and the per-phase kickoff prompt. `CLAUDE.md` carries
the build commands and is the other document a fresh session should read.

| Phase | What | State |
|---|---|---|
| 0 | Baseline, build wiring, docs | **DONE** 2026-09-09 |
| 1 | Flag definitions and tuning constants | **DONE** 2026-09-09 |
| 2 | Offline build wiring + the socket seam | not started |
| 3 | Emulator standup + client revivals | not started |
| 3B | The entry-point character-select handshake | not started |
| 4 | Offline gameplay call sites | not started |
| 5 | AI party | not started |
| 6 | The QoL flags | not started |
| 7 | Content, save migration, full verification | not started |
| 8 | Documentation closeout | not started |

Phase 3B sits between 3 and 4 and is numbered with a letter, not a renumber,
because `CLAUDE.md` already cites "Phase 7 of `MARCH_2014_MIGRATION.md`" for
the save migration - shifting every later number would make that reference
wrong the moment this file changes.

## Starting a phase

Each phase below is meant to run in its own fresh conversation. A cold
session has no memory of this migration except what it reads, so give it two
things: the preamble once, then that phase's kickoff prompt.

### The shared preamble

Paste this at the start of any phase's conversation, before the kickoff
prompt:

```
This is the ProjectX2 (Elsword) client, branch mods/offline-mod-2. The client
source was replaced with a March 2014 snapshot (commit dd63297), which
overwrote every mod edit that lived inside a studio-owned file. I am
restoring them phase by phase. Read CLAUDE.md and MARCH_2014_MIGRATION.md in
full before editing anything - both are at the Trunk root.

Ground rules that apply to every phase:

- Every client edit is gated behind a #define named SERV_IRUHADEV_<NAME>,
  defined in KTDXLIB/Always.h, using the house //{{ author : date //
  description ... //}} comment block and the #endif SERV_IRUHADEV_FOO
  trailing-token style. See CLAUDE.md's top rule for the exact pattern.
- Files are pre-sorted into three lanes (MERGE, MERGE+FIX, BYTE-HAND) in this
  document's per-file inventory. Follow the assigned lane - do not improvise
  a merge on a BYTE-HAND file, and do not hand-edit a MERGE file byte by
  byte. The three-way merge bases are BASE=7b7e482 (the January import),
  OURS=c4495a3 (January + all mods, pre-upgrade), THEIRS=HEAD (March).
- After touching any file, run the verification gate in this document's
  "The verification gate" section: encoding must be unchanged (file <path>),
  diff size must match the mod not the whole file (git diff --stat), and the
  SERV_IRUHADEV_* token set must survive (the diff <(...) <(...) check).
  Check 3 must print nothing - if it does not, a base-resident flag was
  silently dropped by a clean merge (Trap 2).
- Never "fix" CP949/Korean-comment encoding. If `file <path>` changes from
  ISO-8859/Non-ISO to UTF-8 after an edit, the Edit tool corrupted the whole
  file - git checkout -- <path> (confirm with the user first) and redo the
  edit at the byte level with a Python rb/wb script instead.
- Re-locate every edit by symbol name, never by line number - MODS.md's line
  numbers are all stale. git show dd63297^:<path> is the source of truth for
  the exact text that got removed.
- Build with the sequence in CLAUDE.md's Toolchains section
  (SolutionDir must be a forward-slash path with the trailing slash inside
  the value). Judge success by X2/US_INTERNAL/x2.exe existing and being
  newly dated, not by the exit code alone.
- After editing Always.h or X2Define.h, touch X2Lib/stdafx.cpp before
  building - both sit inside every project's precompiled header and msbuild
  does not always notice the edit.
- Do not commit unless asked. When asked, use a simple message and end it
  with the Co-Authored-By line this repo's commits use.

I'm starting phase <N>. My specific instructions for this phase are below.
```

### Per-phase kickoff prompts

Append the phase's block to the shared preamble. Each names its file set,
lane assignments, and done-when condition; the full detail lives in this
file's phase sections further down.

**Phase 1 - Flag definitions and tuning constants**

```
Phase 1. Restore all 14 SERV_IRUHADEV_* flag #defines and the tuning
constants they need, in three files, all BYTE-HAND (do not merge):

- KTDXLIB/Always.h - append 13 #define blocks. See "Phase 1" section of
  MARCH_2014_MIGRATION.md for the exact flag list.
- KTDXLIB/OnlyGlobal/Always_US.h - STATIC_AUTO_LOGIN + _ID + _PW. Before
  restoring it, check whether it is now redundant: AUTO_LOGIN_IN_HOUSE went
  live under US_INTERNAL for the first time (see "What Phase 0 established")
  and reads LoginKey.lua on its own. Decide and record the decision in this
  document's Phase 1 section.
- X2Lib/X2Define.h - the five const blocks ONLY (MP regen, party-bot rates,
  EXP/ED rates, drop draws, quest-item rate). Ignore the 53 reported merge
  conflicts and the +480/-419 diff if you try a merge - that file's real mod
  content is five small const blocks; the rest is encoding noise from the
  January CP949->UTF-8 corruption. Do not apply the full diff.

No call sites are touched yet - nothing should consume these flags this
phase, so the vanilla build must stay exactly as green as it is now. Run the
verification gate on all three files. Done when: all 14 flags exist in
Always.h, encoding is unchanged on all three files, and x2.exe still builds
and launches identically to the Phase 0 baseline.
```

**Phase 2 - Offline build wiring and the socket seam**

```
Phase 2. Wire the 55 files under X2Lib/Offline/ back into the build and
restore the one socket-layer hook the offline emulator depends on, then fix
whatever the March headers broke. Full detail is this document's "Phase 2"
section and "The compile-break table" section - read both before starting.

1. X2Lib/X2Lib_2010.vcxproj: recover the exact original text with
   git show dd63297^:X2Lib/X2Lib_2010.vcxproj and re-add the 22 ClInclude +
   32 ClCompile Offline\ entries plus the sqlite3.c entry (with its four
   per-file children: CompileAsC, PrecompiledHeader NotUsing, the four
   SQLITE_*/INFINITY defines, TurnOffAllWarnings) to plain unconditioned
   ItemGroups. Add the sqlite3 AdditionalIncludeDirectories to the
   US_INTERNAL|Win32 ItemDefinitionGroup only (January had it on
   US_SERVICE|Win32 and US_TEST|Win32, both gone now).
2. X2ServerProtocol/X2ServerProtocol_2010.vcxproj: append
   SERV_IRUHADEV_OFFLINE to the X2TOOL|Win32 PreprocessorDefinitions - that
   is the config US_INTERNAL maps to now. Always.h does not reach this
   project, so this is a second mandatory definition site.
3. X2ServerProtocol/Socket/Session.cpp (MERGE, clean): restore all five
   blocks, including the g_pX2OfflineHook DEFINITION (not just the header
   include) - X2Lib cannot link without it.
4. Build, read the real compiler errors, and fix them by category rather
   than predicting them from a static header diff. Cross-check every error
   against the six verified breaks in this document's compile-break table -
   they cover the whole shared-contract surface, so anything else is either
   a simple include-path issue or worth flagging back before guessing.
5. Regenerate X2Lib/Offline/X2OfflineLuaEnumSeed.h with
   scratchpad/gen_luaenum.py (re-run, not a hand edit) - it is numerically
   stale against March's X2Lib/X2Unit.h (UC_VALIDE_END/UC_END moved
   120/121 -> 123/124) and X2Lib/X2RidingPetManager.h (RPUI_END moved).
   Wrong seeds make server-side .lua table subscripts silently return nil.

Done when: all 55 offline sources compile, X2Lib.lib links, x2.exe links
with SERV_IRUHADEV_OFFLINE defined on both sides. Prove the flag actually
compiled in with a #pragma message inside one guard and read the compiler
output - do not conclude from re-reading the #ifdef.
```

**Phase 3 - Emulator standup and client revivals**

```
Phase 3. Restore the seam that stands the emulator up and the client-side
revivals it depends on. Files (see "Phase 3" section for detail):

- X2Lib/X2Data.cpp (MERGE+FIX, 1 conflict) - the Offline/X2OfflineServer.h
  include and the CX2OfflineServer::Instance() call inside
  ResetServerProtocol(). Locate by symbol - it moved from line 2135 to 1920.
  Leave the AI-party block alone; that is Phase 5.
- X2Lib/X2DungeonSubStage.cpp (MERGE) - the static-NPC parser revival.
- X2Lib/X2QuestManager.cpp (MERGE+FIX, 2 conflicts) and .h (MERGE) -
  m_iAfterQuestID. This field was the MOD's own addition, not a studio
  field that moved - it is not in the compile-break table for that reason.
- X2Lib/X2StateServerSelect.cpp (MERGE+FIX, 1 conflict) - the UDP port
  check. Before re-applying it, check whether NOT_USE_UDP_CHECK_INHOUSE
  (now live under US_INTERNAL, see "What Phase 0 established") has made it
  unnecessary - if CX2Game::AbuserUserCheck already returns immediately,
  restoring a second UDP bypass may be redundant. Record the decision here.
  Note this file grew 8,389 -> 12,086 lines in March; locate by symbol.

Do NOT expect character select to populate this phase - REFORM_ENTRY_POINT
replaced that handshake and the emulator does not answer it yet. That is
Phase 3B, deliberately separated so this phase's done-when is honest:
X2_offline.exe boots from the game directory, writes offline_server.log and
offline_packets.log, and creates a fresh els_db.sql. Use a throwaway save
here - the real one migrates in Phase 7.
```

**Phase 3B - The entry-point character-select handshake**

```
Phase 3B. Implement the three-packet handshake that REFORM_ENTRY_POINT
requires before character select can populate - this is a hard blocker the
January-era offline code never had to deal with, since it predates the
handshake. Full detail, including exact struct member lists and the
silent-skill-wipe hazard, is this document's "Phase 3B" section - read it
in full before writing a handler.

Summary: X2Lib/X2StateServerSelect.cpp now fires
EGS_ENTRY_POINT_GET_CHANNEL_LIST_REQ, EGS_GET_CREATE_UNIT_TODAY_COUNT_REQ,
and a two-stage EGS_CHARACTER_LIST_REQ/_2ND_REQ instead of building the
button list from client-side state. None of these have an emulator handler
yet (confirm with git grep for the event names in X2Lib/Offline/). Add
handlers for all of them to X2OfflineServer's dispatch table, reusing
Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ's existing
CX2OfflineDB::LoadUnits -> CX2OfflineServer::MakeUnitInfoFromRow loop as the
model - do not write that loop twice.

Critical: when populating m_nActiveSkillPagesNumber /
m_nTheNumberOfSkillPagesAvailable in any related ACK, never send 0.
CX2UserSkillTree::SetUsingPage subtracts 1 itself; 0 underflows a USHORT.
Read this document's "Corrections ledger" before writing that value.

Gate everything behind the existing SERV_IRUHADEV_OFFLINE flag - this is new
logic inside already-flagged emulator code, not a new client flag.

Done when: character select shows the real character list from els_db.sql,
entering the village shows the correct skill tree (not wiped to level 0),
and offline_packets.log shows no UNHANDLED entries between login and
village entry.
```

**Phase 4 - Offline gameplay call sites**

```
Phase 4. Restore the SERV_IRUHADEV_OFFLINE blocks that make actual gameplay
(village, rooms, dungeons, titles) talk to the emulator instead of a socket.
Files and lanes are in this document's "Phase 4/5" section. This phase and
its per-file March churn numbers are why the verification gate matters most
here - X2Game.cpp alone carries 927 lines of mod code inside a file that
moved +2758/-1854 in the March upgrade, and it is a MERGE-lane (0 conflict)
file, which is exactly the shape Trap 2 (silent base-flag deletion) hides
in. Run the gate's check 3 on it without skipping.

Restore, in this order: X2Lib/X2Room.cpp/.h, X2Lib/X2StateBeginning.cpp,
X2Lib/X2TitleManager.cpp/.h, X2Lib/X2DungeonGame.cpp (MERGE+FIX, but only
its OFFLINE block this phase - leave the AIPARTY_PERSIST block for Phase 5),
X2Lib/X2Game.cpp/.h (again, OFFLINE block only), X2Lib/X2GageManager.cpp
(only its OFFLINE portion; MP_REGEN_BOOST is Phase 6), X2Lib/X2GageUI.h
(BYTE-HAND; only its OFFLINE portion, BUFF_DURATION_TEXT is Phase 6).

Done when: village entry, room creation, a full dungeon run and the result
screen all work end to end, and grep -E "UNHANDLED|EXCEPTION"
offline_packets.log is empty across that whole loop.
```

**Phase 5 - AI party**

```
Phase 5. Restore SERV_IRUHADEV_AIPARTY_PERSIST. Must follow Phase 4 - same
files, second pass: X2Lib/X2Game.cpp/.h, X2Lib/X2DungeonGame.cpp, and the AI
block in X2Lib/X2Data.cpp (old line 2930-2959, locate by symbol). From prior
work on this feature (see project memory / OFFLINE_MODE_PLAN.md if this
session has access to it): bots run at the player's own stat line and that
is the settled tuning verdict, not a bug to fix; the correct cast for the
party HUD is NUI_CSM_PVP_HERO_*, NUI_PVP_BOT_* is dead code and casting to
it silently does nothing.

Done when: auto-party fills empty dungeon slots with AI heroes and the party
HUD renders them correctly.
```

**Phase 6 - The QoL flags**

```
Phase 6. Restore the remaining 8 flags, almost entirely BYTE-HAND. This is
where Trap 2 (base-resident flags silently dropped by a clean merge) lives -
10 of the files below merged clean in validation but would have silently
lost their flag had the merge been trusted instead of byte-hand-edited.
Group by flag and do the full "Phase 6" section's file list; do not skip the
verification gate on any file in this phase even though most report 0
conflicts.

Flags: BUFF_DURATION_TEXT, SKILL_SLOT_B_FREE, SKILLTREE_NO_LOCK,
QUICK_SLOT_FULL_FREE, MP_REGEN_BOOST (its X2GageManager.cpp/X2GUUser.cpp
portion only - the OFFLINE portion was Phase 4), JOBCHANGE_PORTRAIT,
NO_PATCHER_TOKEN. LEVEL_CAP_80, the EXP/drop boosts, and
OFFLINE_FETCH_AURA_ALWAYS need no studio call sites at all - already covered
by Phase 1's Always.h restore and X2Lib/Offline/'s own surviving code.

Done when: the flag-token verification gate passes on all files in this
phase's list, encoding is unchanged on every one, and each flag is confirmed
working in a play-test (buff duration text shows, skill slot B is free,
skill tree has no lock, quick slots are free, MP regen is boosted, the
job-change portrait shows, and the offline exe launches without the patcher
token).
```

**Phase 7 - Content, save migration, and full verification**

```
Phase 7. This phase has a user-owned step (do not block on it - implement
around it and name the file when reporting done, per CLAUDE.md's Lua rule)
and then a verification pass. Full file list and the new
X2OPTIMIZE_ENCRYPT_AFTER_COMPRESS runtime risk are in this document's
"Phase 7" section.

1. Sixteen server-side .lua files need XOR-encrypting and repacking into
   data036.kom (currently stock, 5.4MB where the pre-upgrade one was 23MB).
   Name them (list is in "Phase 7 owes the user a repack" below) and move on
   - do not hardcode or approximate any of the tables they hold.
2. Re-apply the solo-play tuning edit to
   KncWX2Server/ServerResource/US/BattleFieldServerData.lua, which dd63297
   overwrote - preserve its UTF-8 BOM and CRLF byte-for-byte.
3. Migrate the save: back up 22191271/data/els_db.sql into db_backup/, copy
   it into 24965799/data/, and verify the character rows load before playing
   on it (sqlite3 els_db.sql "select unit_uid, nickname, level from unit;").
   Never delete a save to start clean without asking.
4. Deploy: copy X2/US_INTERNAL/x2.exe to 24965799/data/X2_offline.exe -
   confirm the exact name programmatically (python -c "import os;
   print(os.listdir(DATA))"), not by eyeballing ls. Update the stale
   US_SERVICE msbuild hint text inside start_offline.bat.
5. Play-test and read both logs. If a Lua loader reports 0 rows on a file
   you are sure is packed, check X2OPTIMIZE_ENCRYPT_AFTER_COMPRESS
   (KTDXLIB/KTDX.h:407, new in March) before re-suspecting the pack step -
   it changed the .kom decrypt/decompress order that every offline loader
   goes through.

Done when: the migrated character loads, a full dungeon run completes, and
grep -E "UNHANDLED|EXCEPTION" offline_packets.log is empty.
```

**Phase 8 - Documentation closeout**

```
Phase 8. Re-check CLAUDE.md and MODS.md against what phases 1-7 actually
found, and fix anything wrong. Specifically: regenerate MODS.md's whole flag
table (every file:line in it went stale in the upgrade); fix any
CLAUDE.md phase-number cross-references that changed shape because of
Phase 3B; close out this document's phase table to all-DONE; and archive the
VS2010 server port properly - delete X2Project_Servers_2010.sln and mark
VS2003_to_VS2010_Port_Guide.md historical (its source fixes were overwritten
by the March snapshot, so the solution can no longer build; keep the guide
for its reusable process notes).
```

## Risk re-weighting

The phase table above looks flat, but the actual cost is not evenly spread,
and it is not where an early read of the plan suggests.

**Phase 2 (the compile) is cheap.** A full symbol-level audit of the shared
contract - every packet struct referenced from `X2Lib/Offline/`, every
`SEnum::` value the offline code reads - turned up **six real compile
breaks**, all mechanical, all in "The compile-break table" below. Zero event
IDs were renumbered out from under existing handlers and zero packet structs
were removed wholesale. Budget it as roughly a day, not as the hardest phase.

**Phase 3B is the actual blocker, and it did not exist as a phase until this
document's rewrite.** `REFORM_ENTRY_POINT` and `SERV_ENTRY_POINT` are both
on and replaced the character-select handshake outright - the `#else`
branch the January emulator was written against is dead code, and nothing
in `X2Lib/Offline/` answers any of the three new request packets. Every
later phase depends on getting past character select, so this is the one
piece of genuinely new implementation work in the whole migration, not a
restoration.

**Phases 4/5 are the largest re-application risk by volume.** These are the
files where mod code has to go back into bodies that moved the most in the
March upgrade - `X2Game.cpp` alone is +2758/-1854 lines of March churn
around 927 lines of mod code, `X2StateServerSelect.cpp` is +4795/-1071, and
`X2GUUser.cpp` (Phase 6, but the same shape of risk) is +7257/-3898. None of
that is exotic work, but it is exactly the shape - a clean, wide, confident
merge - that Trap 2 hides inside, so the verification gate is not optional
on these files even when the merge reports zero conflicts.

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
`git show dd63297^:<path>` is the source of truth for the exact removed text.

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

## The compile-break table

Six verified breaks across the whole shared contract (`KncWX2Server/Common/`)
that the offline emulator's 55 files depend on. Located by symbol-level cross
reference between `X2Lib/Offline/`'s use sites and the March packet structs,
not predicted from a header diff - each row below cites the exact emulator
line and the exact March struct shape. Zero event IDs were renumbered and
zero packet structs were removed wholesale; every break here is a field that
moved, was renamed, or was conditionally compiled out.

| # | Emulator site | What moved | Fix |
|---|---|---|---|
| 1 | `Handlers_Unit.cpp:566` | `KEGS_SELECT_UNIT_1_NOT::m_vecSkillAcquired` -> `m_vecUserSkillPageData` under `SERV_SKILL_PAGE_SYSTEM` (on; `ServerDefine.h:4314`). `CommonPacket.h:8153-8157` is a real `#ifdef`/`#else`: only the page-based member exists now. It is a restructure, not a rename - `KUserSkillPageData` (`CommonPacket.h`) wraps the old flat `std::vector<KUserSkillData>` (as `m_vecUserSkillData`) plus `m_aEquippedSkill[]`, `m_aEquippedSkillSlotB[]`, `m_usSkillPoint`, `m_usCashSkillPoint`. | Wrap `CX2OfflineSkill::GetAcquiredSkills()`'s flat vector into a single-element `std::vector<KUserSkillPageData>` with that one page holding the flat vector and the skill-point totals. Equipped slots stay sourced from `m_kUnitInfo.m_UnitSkillData`, not from the page - confirmed at `X2StateServerSelect.cpp:3670`, which reads `SetEquippedSkill` from the unit-info struct, not from `kEvent.m_vecUserSkillPageData`. |
| 2-4 | `Handlers_Room.cpp:1070,1083,1084,1121,1125,1126` | `KEGS_JOIN_BATTLE_FIELD_REQ`'s `m_iBattleFieldID` / `m_StartPosIndex` / `m_bMoveForMyParty` moved inside `m_kBattleFieldJoinInfo` (`KBattleFieldJoinInfo`, `CommonPacket.h`) under `SERV_OPTIMIZE_MOVE_TO_BATTLEFIELD_LOGIC_FIX` (on; `ServerDefine.h:3860`). Note the rename inside the sub-struct: `m_StartPosIndex` -> `m_iStartPosIndex`. | Re-point every read/write through `kReq.m_kBattleFieldJoinInfo.m_iBattleFieldID` etc. Two genuinely new fields exist on the sub-struct and default safely (`m_bNowBattleFieldPositionInfoStartPosition = false`, `m_usBattleFieldPositionValue = 0`) - no emulator change needed for those two, just don't clobber them. |
| 5 | `Handlers_Room.cpp:1111` | `KEGS_JOIN_BATTLE_FIELD_ACK::m_iLastTouchIndex` is now `#ifndef SERV_OPTIMIZE_MOVE_TO_BATTLEFIELD_LOGIC_FIX` - i.e. compiled out, since that flag is on. | Delete the `kAck.m_iLastTouchIndex = kRow.m_iLastLineIndex;` assignment. Nothing downstream needs replacing - the flag's whole point was retiring this field. |
| 6 | `Handlers_Room.cpp:1652` | `m_iItemDropEventProbCount` (int, a draw count) is gone; `ClientPacket.h:1622-1627` is a real `#ifdef SERV_PC_BANG_DROP_EVENT` / `#ifdef SERV_DROP_EVENT_RENEWAL` (on; `ServerDefine.h:4126`) / `#else` - only `float m_fItemDropEventProbRate` (a probability) exists now. | Change `kNot.m_iItemDropEventProbCount = 0;` to `kNot.m_fItemDropEventProbRate = 0.0f;`. Both are currently zeroed (the offline PC-bang event is off), so this is a type/name fix with no behavior change, not a design decision. |
| 7 | `Handlers_Inventory.cpp:311` | `KEGS_CHANGE_INVENTORY_SLOT_ITEM_ACK::m_iOK` removed outright - the struct is now only `std::vector<KInventoryItemInfo> m_vecInventorySlotInfo`. | Delete `kAck.m_iOK = NetError::NET_OK;`. The handler's existing comment about "the default-constructed ACK reads as nothing-changed" still holds - an empty `m_vecInventorySlotInfo` is what signals no-op now, there is no separate status field to also set. |

`QuestTemplet::m_vecAfterQuestID` (referenced in Phase 3's `X2QuestManager`
restore) is **not** in this table on purpose: it was the mod's own added
field in January, not a studio field the March upgrade moved. It gets
restored by the normal Phase 3 merge, not fixed as a compile break.

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

## What Phase 1 established

All 13 `SERV_IRUHADEV_*` flags now exist in `KTDXLIB/Always.h`, appended
byte-hand (extracted in ASCII from `c4495a3`, no merge) after the country-select
`#include` chain that was already the file's tail. The five tuning `const`
blocks that back `MP_REGEN_BOOST`, `AIPARTY_PERSIST` and the two offline reward
flags went into `X2Lib/X2Define.h` the same way. No call site references any of
them yet - `git grep SERV_IRUHADEV_ -- X2Lib X2 KTDXLIB X2ServerProtocol`
(source only, per the *Stale dead-config* grep hazard above) turns up nothing
outside the already-orphaned `X2Lib/Offline/` tree and
`X2ServerProtocol/OfflineHook.h` - so this phase could not have changed the
vanilla build's behavior, and didn't: `X2Lib`, `KTDXLIB` (rebuilt transitively
as a project reference) and `X2` all rebuilt clean, and
`X2/US_INTERNAL/x2.exe` came out freshly dated with the same warning set as
before, no new errors.

**`STATIC_AUTO_LOGIN` is retired, not restored.** `KTDXLIB/OnlyGlobal/Always_US.h`
is untouched this phase. Reasoning: `AUTO_LOGIN_IN_HOUSE` is now live under
`US_INTERNAL` for the first time (see *What Phase 0 established*) and its
`CX2StateLogin::ReadIDAndPassword` runs unconditionally on entering the login
state - it reads `LoginKey.lua` (or, with a confirmation dialog,
`LoginKeyEx.lua`) through the mass-file manager for an `ID`/`PASSWORD` pair and
signs in with it, which is the same "skip the login screen, sign in as a fixed
account" behavior `STATIC_AUTO_LOGIN` provided. Restoring `STATIC_AUTO_LOGIN` on
top would be a second, redundant mechanism, and it carries the two defects
`MODS.md` already flagged it for - it isn't `SERV_IRUHADEV_`-prefixed, and it
lives in a `CLIENT_COUNTRY_US`-only file rather than `Always.h` - plus a third:
its account password ships in plaintext inside a compiled, tracked header,
where `LoginKey.lua` is an external data file that never has to be. To get the
same auto-login today, drop a `LoginKey.lua` next to the exe with a `LOGIN`
table's `ID` and `PASSWORD` strings - loose files now beat the `.kom` archive
unconditionally (see *Loose-file precedence has inverted* above), so no
repacking is needed for this one. `MODS.md`'s `STATIC_AUTO_LOGIN` row and
*Known deviations to clean up* section have been updated to match.

Two comments were corrected while restoring, both our own text rather than
studio bytes, so fixing them isn't a Trap-1 violation:

- The `SERV_IRUHADEV_OFFLINE` block's build note used to say the flag must
  also be set in `X2ServerProtocol_2010.vcxproj`'s "`US_SERVICE`
  PreprocessorDefinitions" - that config name is gone (Phase 0). It now says
  `X2TOOL|Win32`, which is what `US_INTERNAL` actually maps onto for that
  project, and points at Phase 2 rather than repeating the toggle-both-sides
  detail inline.
- The `SERV_IRUHADEV_LEVEL_CAP_80` block cited `Always_US.h:75` and a retail
  cap of `67`; the March snapshot moved `USE_MAXLEVEL_LIMIT_VAL` to
  `Always_US.h:126` and raised the studio's own cap to `70`. It also cited
  `X2UIPersonalShopBoard.h:12` as a second expansion site - that header no
  longer references the macro at all under March; only `X2Game.h:34` does, and
  `X2UIPersonalShopBoard.cpp` reads the resulting `_CONST_X2GAME_::g_iMaxLevel`
  directly. The comment now says so instead of citing a line that no longer
  exists. `X2Define.h`'s dead `LIMIT_MAX_LEVEL` is `70` now too (studio's
  change, coincidentally matching the new floor) but is still unreferenced by
  any code, so it's still left alone.

Verification gate ran clean on both files: `file` reports the same encoding as
before the edit for each (`Non-ISO extended-ASCII` for `Always.h`, `ISO-8859`
for `X2Define.h`); `git diff --stat` shows insertions only, no deletions, on
both; and the Trap-2 flag-token diff printed nothing for either file.

## Phase 3B - the entry-point handshake, in full

This is the one piece of genuinely new implementation in the whole migration -
every other phase is restoring code that already existed in some form. It did
not exist as a separate phase before this rewrite because the January-era
offline mod predates the handshake entirely: `REFORM_ENTRY_POINT` is a March
addition.

### Why it exists

`X2Lib/X2StateServerSelect.cpp:437-441`:

```c
#ifdef REFORM_ENTRY_POINT	 	// 13-11-11,    kimjh
    Handler_EGS_ENTRY_POINT_GET_CHANNEL_LIST_REQ ();
    Handler_EGS_GET_CREATE_UNIT_TODAY_COUNT_REQ ();
    Handler_EGS_CHARACTER_LIST_REQ ();
#else  // REFORM_ENTRY_POINT	// 13-11-11,    kimjh
    ClearUnitButton();
    CreateUnitButton();
#endif // REFORM_ENTRY_POINT	// 13-11-11,    kimjh
```

`REFORM_ENTRY_POINT` is defined unconditionally at `KTDXLIB/Always.h:472`, and
its server-side counterpart `SERV_ENTRY_POINT` is defined unconditionally at
`KncWX2Server/Common/ServerDefine.h:4304`. Both are on in every March build -
there is no configuration where the `#else` branch the January emulator relies
on is reachable. The three `Handler_EGS_*` calls each send a request and wait
for the emulator to answer; none of the three currently has an emulator-side
handler (confirmed by `git grep` across `X2Lib/Offline/` for each event name -
zero matches for all three).

### It is five packets, not three

`KncWX2Server/Common/EventID_Client.h:2243-2256` shows the character list is a
**two-stage** handshake, not a single request/reply:

```
EGS_CHARACTER_LIST_REQ  ->  EGS_CHARACTER_LIST_1ST_ACK
                             (client may send EGS_CHARACTER_LIST_2ND_REQ)
                         ->  EGS_CHARACTER_LIST_ACK
```

and `EGS_GET_CREATE_UNIT_TODAY_COUNT` has the same `_1ST_ACK` shape. One
mercy: `ClientPacket.h:10953` is
`typedef KEGS_CHARACTER_LIST_ACK KEGS_CHARACTER_LIST_1ST_ACK;` - the two
stages share one payload type, so one builder function serves both replies.

### The payloads, read directly out of `ClientPacket.h`

`KEGS_ENTRY_POINT_GET_CHANNEL_LIST_ACK` (`:300-314`):

```c
DECL_PACKET( EGS_ENTRY_POINT_GET_CHANNEL_LIST_ACK )
{
    std::map< int, KChannelInfo >       m_mapSolesChannelList;
    std::map< int, KChannelInfo >       m_mapGaiaChannelList;
    std::map< int, KChannelBonusInfo >  m_mapSolesChannelBonusList;
    std::map< int, KChannelBonusInfo >  m_mapGaiaChannelBonusList;
};
```

`KEGS_CHARACTER_LIST_ACK` (`:10937-10950`):

```c
DECL_PACKET( EGS_CHARACTER_LIST_ACK )
{
    int                                       m_iOK;
    std::map< int, int >                      m_mapServerGroupUnitSlot;
    std::map< int, std::vector<KUnitInfo> >   m_mapServerGroupUnitInfo;
    std::wstring                              m_strUserID;
};
```

`KEGS_GET_CREATE_UNIT_TODAY_COUNT_ACK` (`:10999-11011`):

```c
DECL_PACKET( EGS_GET_CREATE_UNIT_TODAY_COUNT_ACK )
{
    UidType               m_iUserUID;
    std::map< int, int >  m_mapCreateCharCountToday;
};
```

### The reuse that makes this tractable

This is not new logic - it is the existing single-server-group handler
reshaped into a per-server-group map. `Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ`
(`X2Lib/Offline/Handlers_Unit.cpp:61-100`) already does exactly the required
work:

```c
std::vector< KOfflineUnitRow > vecRow;
CX2OfflineDB::Instance()->LoadUnits( kSes.m_nUserUID, vecRow );
...
for( size_t i = 0; i < vecRow.size(); ++i )
{
    KUnitInfo kUnit;
    MakeUnitInfoFromRow( kUnit, vecRow[i] );      // X2OfflineServer.cpp:880
    kAck.m_vecUnitInfo.push_back( kUnit );
    ...                                             // soft-deleted units go in too
}
```

`CX2OfflineServer::MakeUnitInfoFromRow` (declared `X2OfflineServer.h:520`) is
already the single source of truth for turning a DB row into a `KUnitInfo`.
The new `EGS_CHARACTER_LIST_REQ` handler should call the same
`LoadUnits` -> `MakeUnitInfoFromRow` loop and write the resulting vector into
`kAck.m_mapServerGroupUnitInfo[ serverGroupID ]` instead of a flat vector, with
`kAck.m_mapServerGroupUnitSlot[ serverGroupID ] = m_iUnitSlots`. Do not
duplicate the loop - factor it into a helper both handlers call, or have the
new handler call the existing one's inner logic directly. Offline mode has
exactly one server group, so the map will always have one entry; that is fine,
the client only reads the map by key.

Keep the existing `Handler_EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_REQ` handler in
place rather than deleting it - `SERV_ELISIS_PREVIOUS_SIS_EVENT` at
`X2StateServerSelect.cpp:426` can still call it down a different path.

### The silent skill-tree wipe trap

The mechanism, precisely, from `X2Lib/X2UserSkillTree.cpp`'s
`SKILL_PAGE_SYSTEM` overload of `SetAcquiredSkill`:

```c
void CX2UserSkillTree::SetAcquiredSkill( const std::vector<KUserSkillPageData>& vecSkillList )
{
    m_vecSkillDataMap.clear();
    for ( UINT i = 0; i < m_usOpenedPage; ++i )
        m_vecSkillDataMap.push_back( SkillDataMap() );

    int iPage = 0;
    for each page in vecSkillList:
        for each skill in page.m_vecUserSkillData:
            if ( m_vecSkillDataMap.empty() || iPage >= m_vecSkillDataMap.size() )
                continue;                          // <-- silently dropped, no log
            m_vecSkillDataMap[iPage][ skillID ] = ...;
        m_arrSkillPoint[iPage] = page.m_usSkillPoint;
        ++iPage;
}
```

`m_vecSkillDataMap` is sized from `m_usOpenedPage`, which is set by
`SetOpenedPage()` (`X2StateServerSelect.cpp:3663`,
called **before** `SetAcquiredSkill()` at `:3668` - the ordering is correct in
the client and must not be reversed by anything the emulator does). If
`m_nTheNumberOfSkillPagesAvailable` sent in the ACK is `0`, `m_usOpenedPage`
is `0`, `m_vecSkillDataMap` stays empty, and every single skill in the ACK's
payload is discarded by the `continue` - **with no error, no log line, and no
crash**. `m_arrSkillPoint` still gets written from `page.m_usSkillPoint`
regardless, so a player would see full skill points and an empty tree, which
reads as a bug in the point display rather than as missing data.

**The contract to implement:** send `m_nTheNumberOfSkillPagesAvailable >= 1`
and a non-empty `m_vecUserSkillPageData`. See "Corrections ledger" below for
why `m_nActiveSkillPagesNumber` must never be `0` either - that one is a
different, non-silent failure mode (integer underflow), not the wipe.

### Done when

Character select shows the real character list read from `els_db.sql`,
entering the village shows the correct skill tree state (not wiped to empty),
and `offline_packets.log` shows no `UNHANDLED` entries anywhere between login
and village entry.

## Corrections ledger

Two findings from an earlier pass at this analysis were wrong or irrelevant,
and are recorded here so a later phase does not re-adopt them from an older
note or transcript.

**`SetUsingPage`'s off-by-one.** An earlier draft recommended sending
`m_nActiveSkillPagesNumber = 0` to "reset" the active page. This is inverted
and destructive: `CX2UserSkillTree::SetUsingPage` (`X2Lib/X2UserSkillTree.h:457`)
is `{ m_usUsingPage = usValue_ - 1; }` - it already does the 1-based-to-0-based
conversion itself. Sending `0` makes `m_usUsingPage` underflow a `USHORT` to
`65535`, which is a different and worse failure than the skill-wipe trap
above (that one degrades to an empty tree; this one is an out-of-range index
the first time anything reads `m_vecSkillDataMap[m_usUsingPage]`). The
constructor defaults on the wire struct
(`KncWX2Server/Common/CommonPacket.h:1103-1104`, both `1`) are already
correct and should be left alone; send `1`, never `0`, for
`m_nActiveSkillPagesNumber`.

**`SEnum::LOCAL_MAP_ID` re-pointing is a non-issue for this migration.**
`KncWX2Server/Common/Enum/Enum.h:327-332` does show IDs 10000-10002
re-assigned from `LMI_VELDER_*` to `LMI_RUBEN`/`LMI_ELDER`/`LMI_BESMA` (with
`LMI_VELDER` itself moved to 10005), which is real churn in the enum. But
`X2Lib/Offline/` was checked directly (`git grep` for `LMI_` and for bare
`1000[0-9]` literals) and has **zero use sites that reference a local map ID
by name or by a hardcoded value in that range**. No phase needs to touch
anything for this.

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

**New runtime risk for this phase:** `X2OPTIMIZE_ENCRYPT_AFTER_COMPRESS` is
new at `KTDXLIB/KTDX.h:407`, unconditionally defined, and changes the order
`.kom` entries are decrypted versus decompressed. Every one of the nine
offline Lua loaders goes through this path. If a loader reports 0 rows on a
file confirmed to be packed correctly, check this flag's effect before
re-suspecting the packing step - it is new behavior this build has that the
January build did not.
