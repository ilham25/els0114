# Offline Elsword — phases 29-35: the `ISSUES_2.md` defects

Continuation of `OFFLINE_MODE_PHASE9_PLAN.md`. That document turned
`ISSUES.md`'s nineteen defects into phases 9-28, and all of them are shipped.
A second round of play produced `ISSUES_2.md` — six more defects. This document
turns each one into a phase that can be run as its own conversation.

**Written 2026-09-07, from the tree.** Unlike the 9-28 batch, this one had no
usable logs to work from — see §0.2 — so **every Diagnosis block below was
reached by reading code, and every one of the six is CONFIRMED.** No phase here
starts from a hypothesis and none is blocked on the user for data. The line
references are quoted, not remembered; check them rather than re-deriving them.

Two decisions taken with the user, 2026-09-07:

- **The rank work is split in two.** Phase 32 wires the real rank engine
  (all-F → real letters) accepting a monster weight that is known to be low;
  phase 33 then revives the monster grade to correct it.
- **The fetch aura becomes `QUALITY_OF_LIFE.md` #7 instead of a persistence
  fix.** Phase 35 forces the aura on for every hatched pet rather than making
  the purchased toggle persist. Consequence the user accepted: the toggle path
  itself stays unfixed. Consequence the phase must still handle: item 500720
  would otherwise be consumed to do nothing, so phase 35 refuses it.

---

## §0 — Read this before any phase in this batch

Five findings from the investigation that produced this document. Each one
would otherwise cost a phase conversation an hour.

### 0.1 `X2StateBeginning.cpp` is dead code, and phase 16's client-side fix never ran

`ELSWORD_NEW_BEGINNING` is **not** defined ([InHouse3.h:275](KTDXLIB/InHouse3.h#L275),
commented out). [X2Main.cpp:4930-4944](X2Lib/X2Main.cpp#L4930) only constructs
`CX2StateServerSelect`; the `XS_BEGINNING` / `CX2StateBeginning` arm is
unreachable. So:

- The character list is drawn by **`CX2StateServerSelect::CreateUnitButton()`**
  ([X2StateServerSelect.cpp:1993-2007](X2Lib/X2StateServerSelect.cpp#L1993)),
  not by `X2StateBeginning.cpp`.
- Phase 16's `GetPVPEmblem()` → `GetPvpRank()` swap at
  [X2StateBeginning.cpp:1595-1616](X2Lib/X2StateBeginning.cpp#L1595) compiles
  and does nothing. `X2StateServerSelect.cpp:1999` already had the
  type-correct `GetPvpRank()` call in the shipped source. The half of phase 16
  that actually worked was the server-side
  `kOut.m_cRank = PVPRANK_RANK_ARRANGE` in `MakeDefaultUnitInfo`.
- **That phase-16 status note in `OFFLINE_MODE_PHASE9_PLAN.md` is therefore
  known-wrong**, and phase 31 owns correcting it. It is the thirteenth entry in
  that document's §0.1 stale-claim count.

Lesson to carry: **this tree has two implementations of several screens, one of
them flag-dead.** Before editing a UI file, check the state is actually
constructed in `X2Main.cpp`.

### 0.2 The logs on disk do not cover these six defects

`offline_packets.log` and `offline_server.log` are overwritten per run. The
copies in the game directory are 14 KB / 7.5 KB from a 19:45-19:46 cash-shop
session on 2026-09-07 — **zero `UNHANDLED`, zero `IGNORED`, zero `EXCEPTION`** —
and contain nothing from the play that produced `ISSUES_2.md`.

So the phase-8 loop (`grep UNHANDLED`) has nothing to say about this batch, and
neither does the §0 bucket table in the phase-9 doc. Where a phase wants log
evidence it has to reproduce first. **Do not read the absence of a packet from
these logs as evidence of anything.**

Current state of the save (`els_db.sql`, schema v11):

| unit_uid | nickname | level | ED |
|---|---|---|---|
| 12 | reyaa | 50 | 283,051 |
| 18 | Freils | 17 | 423,579 |

`unit_pet` holds 5 rows; `auto_feed` and `auto_looting` are **0 on every one**.
A pet is summoned on login (petUID 5, `Shioo`) — phase 28's restore works.

### 0.3 The client treats every `m_iED` as an absolute, and there is no setter

There is no `CX2Unit::SetED`. Roughly twenty packet handlers do
`g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_ED = kEvent.m_iED;`
raw, and `CX2Inventory::UpdateInventorySlotList` then repaints the wallet from
that member ([X2Inventory.cpp:285](X2Lib/X2Inventory.cpp#L285)). `DECL_PACKET`
is a plain struct with no value-init, so an ACK whose constructor does not list
`m_iED` ships **indeterminate stack bytes**, not zero.

`IsValidPacket( m_iOK )` ([X2Main.cpp:6914](X2Lib/X2Main.cpp#L6914)) returns
true only for `NET_OK` and four special codes, so an ED left at 0 on a
*refusal* path is harmless — the client never reads it. **Only success paths
matter.** That is what makes the phase-34 audit small and precise.

### 0.4 The offline emulator inherits the *client's* quest/title parsers, and they are less complete than the server's

`CX2OfflineQuest` reuses `CX2QuestManager`'s templets rather than parsing the
script itself ([X2OfflineQuest.h:11-14](X2Lib/Offline/X2OfflineQuest.h#L11)).
The client's `LoadClearCondition` reads `m_bUpperDifficulty` for 8 clear types;
the server's `CXSLQuestManager` reads it for 11. The three the client omits are
exactly the ones phase 30 is about. The same asymmetry exists in the title
loader — 3 sites client-side against 6 server-side.

Generalisation worth applying to any future offline defect: **when offline
behaviour depends on a field of a client-parsed templet, diff the client's
parser against `KncWX2Server/Common/X2Data/XSL*Manager.cpp` before assuming the
field is populated.** The client never needed those fields because the server
decided; offline, the client's parse *is* the server.

### 0.5 An `OUT` vector parameter in `X2OfflineInventory` may clear itself

`CX2OfflineInventory::InsertItem` opens with `vecChanged.clear()`
([X2OfflineInventory.cpp:1436-1441](X2Lib/Offline/X2OfflineInventory.cpp#L1436)),
while `ConsumeByID` only `push_back`s. Any caller that passes one accumulator
into both, or into `InsertItem` twice, silently loses everything gathered
earlier. That is phase 29's whole bug, and it is latent in one more place
([Handlers_Quest.cpp:416](X2Lib/Offline/Handlers_Quest.cpp#L416), the rollback
vector, never sent).

The correct house pattern is a fresh local per call, appended to the ACK —
[Handlers_Social.cpp:2838-2847](X2Lib/Offline/Handlers_Social.cpp#L2838) is the
reference; `Handlers_Inventory.cpp:2369`, `:551` and `Handlers_Room.cpp:2616`
do the same. **When adding a call to `InsertItem`, grep the other call sites
before choosing where the out-vector comes from.**

---

## §1 — Shared preamble

`OFFLINE_MODE_PHASE9_PLAN.md`'s §1 applies unchanged and is not repeated here.
One line opens a phase:

> Read §0 and the Phase N section of `OFFLINE_MODE_PHASE29_PLAN.md`, plus §1 of
> `OFFLINE_MODE_PHASE9_PLAN.md`, then do Phase N. Don't read the other phase
> sections.

The five standing facts, in short:

1. Branch is `mods/offline-mod-2`; everything here is client-only. Nothing in
   this batch touches `KncWX2Server/`, so the servers never need rebuilding.
2. **Every edit is gated by `SERV_IRUHADEV_OFFLINE`**, the flag already in
   `KTDXLIB/Always.h`. Do not mint a per-phase flag — the phase-9 doc's §1
   rule 2 explains why the four that were minted had to be migrated away. A
   throwaway `SERV_IRUHADEV_*_DEBUG` logging flag is the only exception.
3. Build / deploy loop, from `CLAUDE.md`:
   ```sh
   TRUNK="f:/elsword stuff/elsword_2014/els_2014/ElswordFiles/ready_and_built/ProjectX2_SVN/ProjectX2_SVN/source/EU_CN_US/Trunk"
   DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"
   touch X2Lib/stdafx.cpp        # after ANY header edit, or the PCH eats it
   msbuild X2Lib/X2Lib_2010.vcxproj -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   msbuild X2/X2_2010.vcxproj       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   ls -la X2/US_SERVICE/X2.exe   # judge by the artifact; post-build ALWAYS fails here
   cp X2/US_SERVICE/X2.exe "$DATA/X2_offline.exe"
   ```
   Verify the copy landed by reading the name, size and mtime
   programmatically. The directory holds eight other `X2*.exe`.
4. Reading the logs is the verification; there is no test suite.
5. `els_db.sql` is real player data. `X2OfflineDB` migrates forward only.

**Files in this batch that carry non-ASCII bytes**: `X2QuestManager.cpp`
(phase 30 edits it), `X2TitleManager.cpp` (phase 30 may), plus the offline
`Handlers_*.cpp` which are ASCII. `CLAUDE.md`'s CP949 rule binds: after editing
any file with non-ASCII bytes, run `file <path>` and `git diff --stat`, and if a
10-line change came out as hundreds, restore and redo as a byte-level Python
patch. This has already destroyed a file twice in this project
(see phase 16's process note).

---

## §2 — Phase index

Ordered for execution: confirmed-and-cheap first, so each landing shrinks the
surface the big ones are debugged against.

| Phase | `ISSUES_2.md` | Defect | Size | Confidence | Blocked? |
|---|---|---|---|---|---|
| **31** | 3 | PvP emblem turns into a black box after entering a room | 1 line + an audit | **DONE 2026-09-07** - the audit found a second clobber | no |
| **29** | 1 | Quest-reward item invisible until character re-select | ~6 lines | CONFIRMED | no |
| **34** | 5a | Using any item from the bag zeroes the displayed ED | 3 handlers + helper | CONFIRMED | no |
| **35** | 6 | Fetch aura → QoL #7: every hatched pet has it | ~3 lines | CONFIRMED | no |
| **30** | 2 | "Any difficulty" dungeon quest only advances on Normal | 3 lines + verify | CONFIRMED | no |
| **32** | 4 | Result screen is F rank for every unit | large | CONFIRMED | needs a file packed |
| **33** | — | Monster grade, so the ranks in 32 are accurate | large | CONFIRMED | no |

**Run 34 before 35.** Both edit `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`
([Handlers_Inventory.cpp:190-410](X2Lib/Offline/Handlers_Inventory.cpp#L190));
either order works, but running them as parallel conversations will conflict.

**Phase 33 depends on 32** and is only worth running once 32's ranks are on
screen — its whole purpose is to correct their bias.

`ISSUES_2.md` #5 has two halves. **5a** (the ED display) is phase 34 and is
confirmed. **5b** ("cover all box item function") was an audit step inside phase
34; it is now `ISSUES_2.md` #7 and owns its own document,
`OFFLINE_MODE_PHASE36_PLAN.md` (phases 36-45). See the note in phase 34.

---

# Phase 29 — Quest-reward item invisible until character re-select (`ISSUES_2.md` #1)

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
An equipment reward from a quest is in the inventory in the save, but does not
appear in the inventory UI until the player goes back to the character list and
re-selects the character.

### Diagnosis — CONFIRMED
`CX2OfflineInventory::InsertItem` **clears the caller's out-vector on entry**
([X2OfflineInventory.cpp:1436-1441](X2Lib/Offline/X2OfflineInventory.cpp#L1436)),
and `CompleteOneQuest` passes one shared accumulator into it once per reward
item:

| line | what it does |
|---|---|
| [Handlers_Quest.cpp:374](X2Lib/Offline/Handlers_Quest.cpp#L374) | `std::vector< KInventoryItemInfo > vecChangedSlot;` |
| `:381` | `ConsumeByID( ..., vecChangedSlot )` — **appends** the handed-in items |
| `:434` | `InsertItem( ..., vecChangedSlot, iInserted )` — **clears it**, once per reward |
| `:547` | `kAck.m_kUpdateUnitInfo.m_vecKInventorySlotInfo = vecChangedSlot;` |

So the ACK carries only the slots touched by the **last** `mapReward` entry —
`mapReward` is a `std::map<int,int>`, so that is the highest item ID.
Everything else is dropped: the other reward items, and *always* the handed-in
condition items. The items themselves are committed to SQLite by `InsertItem`
(`X2OfflineInventory.cpp:1522`), which is exactly "in the data but not in the
UI", and re-select rebuilds the whole bag from the DB
(`PushSelectUnitNotifications`,
[Handlers_Unit.cpp:513-592](X2Lib/Offline/Handlers_Unit.cpp#L513)) — which is
why that "fixes" it.

The client side is correct and needs no change. `SERV_DAILY_QUEST` is defined
([ServerDefine.h:236](KncWX2Server/Common/ServerDefine.h#L236)), so the live
handler is [X2UIQuestNew.cpp:1442](X2Lib/X2UIQuestNew.cpp#L1442) — not the
`#else` copy at `:1582` — and it calls `UpdateInventorySlotList` on both the
model and the UI grid. `CX2Unit::Reset` ([X2Unit.cpp:155](X2Lib/X2Unit.cpp#L155))
does not touch the inventory; only the equipped-item list, which
`MakeUnitInfoFromRow` fills.

### What to do
1. In `CompleteOneQuest`, give `InsertItem` a **fresh local vector per reward
   item** and append it to `vecChangedSlot`, copying
   [Handlers_Social.cpp:2838-2847](X2Lib/Offline/Handlers_Social.cpp#L2838)
   verbatim in shape.
2. Same treatment for the no-room rollback loop at `Handlers_Quest.cpp:416`
   (`vecUndo`). Latent today because that vector is never sent, but it is the
   same mistake, and leaving it is how it comes back.
3. `Handler_EGS_ALL_COMPLETED_QUEST_COMPLETE_REQ`
   ([Handlers_Quest.cpp:663](X2Lib/Offline/Handlers_Quest.cpp#L663)) reuses
   `CompleteOneQuest`, so the batch hand-in button is fixed by the same change —
   confirm that, don't re-implement it.
4. **Audit every other `InsertItem` caller** before closing the phase.
   `Handlers_Shop.cpp:749` passes `kAck.m_vecInventorySlotInfo` straight in;
   that is currently harmless (it is the only fill) but is one added line away
   from the same bug — either leave it with a comment saying why it is safe, or
   normalise it.
5. Optional root fix: `InsertItem`'s `clear()` serves no caller. Removing it
   makes the whole class impossible — but it changes behaviour for ~8 call sites
   at once, so if you do it, read all of them first and say so in the commit.

### Trap
The symptom names *equipment*, which invites a hunt through the equipped-item
path (`SetKUnitInfo`, `m_NowEqipItemUIDList`, `ResetEqip`). That path is fine.
The reward is an ordinary bag insert; equipment is just what the user happened
to be given. Do not start in `X2Unit.cpp`.

### Exit test
Hand in a quest that pays an equipment reward **and** takes collected items. The
reward appears in the inventory immediately, the handed-in items disappear
immediately, and no re-select is needed. A multi-item reward shows every item.

---

# Phase 30 — "Any difficulty" dungeon quest only advances on Normal (`ISSUES_2.md` #2)

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
A dungeon quest whose text says it counts at any difficulty only progresses
when the dungeon is cleared on Normal.

### Diagnosis — CONFIRMED
`REFORM_QUEST` is on ([Always.h:1088](KTDXLIB/Always.h#L1088)), so
`ClearCondition` has **no difficulty field and no "any" sentinel** —
`m_cDifficulty` exists only in the `#else` arm
([X2QuestManager.h:194-206](X2Lib/X2QuestManager.h#L194)). Difficulty is the
last digit of the dungeon ID (`base*10 + DL_NORMAL|DL_HARD|DL_EXPERT`,
[X2Dungeon.h:40-45](X2Lib/X2Dungeon.h#L40), `:87-92`), and the only "any
difficulty" mechanism is `m_bUpperDifficulty == true` with the Normal ID in
`m_setDungeonID` — "this dungeon at difficulty ≥ 0".

`CX2OfflineQuest::IsExistDungeonInSub`
([X2OfflineQuest.cpp:594-622](X2Lib/Offline/X2OfflineQuest.cpp#L594)) is a
faithful port of the server's `XSLQuestManager.cpp:2137-2172` and honours the
flag correctly. **The flag is never set.** The client's quest-script parser
omits `m_bUpperDifficulty` for exactly three clear types, where the server's
reads it:

| clear type | server `XSLQuestManager.cpp` | client `X2QuestManager.cpp` |
|---|---|---|
| `SQT_NPC_HUNT` (2) | `:882` ok | `:1941` ok |
| `SQT_*_ITEM_COLLECTION` (3, 4) | `:941` ok | `:2042` ok |
| `SQT_DUNGEON_TIME` (5) | `:1011` ok | `:2094` ok |
| `SQT_DUNGEON_RANK` (6) | `:1035` ok | `:2130` ok |
| `SQT_DUNGEON_DAMAGE` (7) | `:1061` ok | `:2170` ok |
| `SQT_DUNGEON_CLEAR_COUNT` (8) / `SQT_SUITABLE_LEVEL_DUNGEON_CLEAR` (32) | `:1089` ok | `:2210` ok |
| `SQT_WITH_DIF_SERV_USER` (12) | `:1135` ok | `:2277` ok |
| `SQT_ITEM_EQUIP_DUNGEON_CLEAR` (17) | `:1193` ok | `:2356` ok |
| **`SQT_ITEM_USE` (22)** | **`:1427` ok** | **`:2523-2569` MISSING** |
| **`SQT_VISIT_DUNGEON` (26)** | **`:1307` ok** | **`:2434-2449` MISSING** |
| **`SQT_FIND_NPC` (27)** | **`:1342` ok** | **`:2450-2481` MISSING** |

The default is `false` ([X2QuestManager.h:285](X2Lib/X2QuestManager.h#L285)), so
`IsExistDungeonInSub` takes the exact-`find()` branch and only the literal
Normal ID matches. This build's census has **205 sub-quests of type 26 and 47 of
type 27** (`offline_server.log`, `QUEST CENSUS clearType=26/27`). And because
sub-quest groups are staged (`CheckBeforeGroup`,
[X2OfflineQuest.cpp:644](X2Lib/Offline/X2OfflineQuest.cpp#L644)), a stuck
group-0 `VISIT_DUNGEON` step also blocks the later `DUNGEON_CLEAR_COUNT` step in
the same quest — which is why the whole quest reads as Normal-only even though
the clear-count branch parses the flag correctly.

Under `REFORM_QUEST` the visible quest text is hand-written
(`m_wstrDescription`); all the auto-generated difficulty wording is in
`#ifndef REFORM_QUEST` dead branches. So "any difficulty" is only a *string* in
the script and the enforceable half is `m_bUpperDifficulty` alone. That is
exactly why dropping the flag is silent.

### What to do
1. Add the three missing reads to `CX2QuestManager::LoadClearCondition`,
   matching the server's lines, each behind `SERV_IRUHADEV_OFFLINE` with the
   original reachable in `#else`.
2. **`LUA_GET_VALUE_RETURN` hard-fails (`goto error_proc`) when the key is
   absent** (`luaLib/KLuaManager.h:143-150`). The server uses the same macro on
   the same script and does not fail, so the key is present for these types —
   but *prove it against this build's packed `Quest.lua`/`SubQuest.lua`* before
   shipping, or a missing key kills the templet load and takes every quest with
   it. Two ways: decrypt the script out of `data036/` with the XOR-key method
   phase 19 established and grep, or land the change and read the census line
   in `offline_server.log` (it prints **1,395 templets** today — a drop means
   the parse is failing).
3. Add the flag to `CX2OfflineQuest::LogTempletCensus`'s per-sub-quest line
   ([X2OfflineQuest.cpp:60-405](X2Lib/Offline/X2OfflineQuest.cpp#L60)), which
   currently prints neither `m_setDungeonID` nor `m_bUpperDifficulty`. This is
   what makes the exit test readable instead of guesswork.
4. The dungeon filter rejects with a bare `continue` and no log
   (`X2OfflineQuest.cpp:1646`, `:1700`, `:2049`). One line naming the
   sub-quest, the required set and the packed key the clear arrived with turns
   the next report of this class into a one-play-test diagnosis.

### Also in scope, same shape — decide explicitly
The **title** loader has the identical asymmetry: the client reads
`m_bUpperDifficulty` 3× (`X2TitleManager.cpp:635`, `:712`, `:722`) where the
server reads it 6× (`XSLTitleManager.cpp:398`, `:419`, `:439`, `:459`, `:565`,
`:576`) — missing for `TMCT_DUNGEON_TIME`, `TMCT_DUNGEON_RANK`,
`TMCT_DUNGEON_DAMAGE`. Titles do have a real `-1` "any" sentinel
([X2TitleManager.h:154](X2Lib/X2TitleManager.h#L154)) which
`CX2OfflineTitle::DungeonMatches` honours
([X2OfflineTitle.cpp:261-267](X2Lib/Offline/X2OfflineTitle.cpp#L261)), so "any"
still works there and only "≥ this difficulty" is broken. Fix it here or record
it in the appendix — but do not leave it undecided.

### The reproduction the user actually hit
An earlier revision of `ISSUES_2.md` named it, before the entry was generalised
(`git show HEAD:ISSUES_2.md`):

> the first pet tainted crystal quest cannot be completed even though i already
> complete shadow forest dungeon, turns out i need to play the normal diff
> (even though the quest said any difficulty is fine)

**Start there.** Find that quest's templet ID, log its sub-quests' clear types,
`m_setDungeonID` and `m_bUpperDifficulty`, and confirm the step in question is
one of the three types in the table above with the Shadow Forest **Normal** ID
in its set. That turns the diagnosis from "confirmed by reading two parsers"
into "confirmed against the case that was reported", and it gives the exit test
a specific quest instead of a category.

### Exit test
The first pet (tainted crystal) quest completes after clearing Shadow Forest on
**Hard or Very Hard** — no Normal run needed. More generally: clear a dungeon on
Hard or Very Hard that has an "any difficulty" `VISIT_DUNGEON` step in progress
and the step ticks. The census line shows `upperDiff=1` for that sub-quest, and
the templet count is still 1,395.

---

# Phase 31 — PvP emblem turns into a black box after entering a room (`ISSUES_2.md` #3)

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
Choose a character, play, come back to the character list: **that** character's
PvP emblem is now a small black box. The others are fine.

### Diagnosis — CONFIRMED
`MakeRoomUserInfo` starts with `kOut = KRoomUserInfo();` and **never sets
`m_cRank`** ([Handlers_Room.cpp:171-205](X2Lib/Offline/Handlers_Room.cpp#L171));
the default is 0 ([CommonPacket.h:2154](KncWX2Server/Common/CommonPacket.h#L2154),
`:2319`). The client then writes that zero straight onto **my own** `CX2Unit`:

- [X2Room.cpp:2129](X2Lib/X2Room.cpp#L2129) — `m_pUnit` is literally the object
  in `MyUser`'s list, the one the character list reads
- [X2Room.cpp:2180](X2Lib/X2Room.cpp#L2180) —
  `SetKRoomUserInfo( kRoomSlotInfo_.m_kRoomUserInfo )`
- [X2Unit.cpp:3356](X2Lib/X2Unit.cpp#L3356) — `m_cRank = data.m_cRank;`,
  unconditionally

`PVPRANK_NONE = 0` is never registered by `PVPEmblem_Season2.lua` (its lowest
key is `PVPRANK_RANK_ARRANGE = 1`), so `GetPVPEmblemData` returns NULL
([X2PVPEmblem.cpp:41-54](X2Lib/X2PVPEmblem.cpp#L41)), `SetTex` is skipped on a
picture that was just `SetShow(true)`, and an untextured quad is the black box.

The list is only rebuilt from a fresh packet on the **first** visit:
`CX2StateServerSelect`'s constructor short-circuits on the static
`m_sbSelectedServerSet`
([X2StateServerSelect.cpp:323-330](X2Lib/X2StateServerSelect.cpp#L323)) and
re-reads the cached `CX2Unit` objects instead. So the clobber survives to the
character list. Texture lifetime is **not** involved — the emblem textures are
held for the life of the app ([X2PVPEmblem.cpp:126-128](X2Lib/X2PVPEmblem.cpp#L126),
released only in the destructor) and no `OnLostDevice` path touches them.

Every `KUnitInfo` the offline server emits is already correct
(`MakeDefaultUnitInfo`, [X2OfflineServer.cpp:446](X2Lib/Offline/X2OfflineServer.cpp#L446));
`KRoomUserInfo` is the one struct that is not.

**Read §0.1 first** — the phase-16 fix that was supposed to cover this lives in
a file this build never constructs.

Entry points that reach the clobber (`MakeRoomSlots` → `MakeRoomUserInfo`,
[Handlers_Room.cpp:403-418](X2Lib/Offline/Handlers_Room.cpp#L403)) include
`EGS_CREATE_TUTORIAL_ROOM` (`:532`), which fires immediately after a brand-new
character is chosen — so a new character can show the black box before its
first dungeon.

### What to do
1. Set `kOut.m_cRank = (char)CX2PVPEmblem::PVPRANK_RANK_ARRANGE;` in
   `MakeRoomUserInfo`, next to the `m_iTitleID` line, sourcing the value the
   same way `MakeDefaultUnitInfo` does so there is one source of truth.
2. `KRoomUserInfo::m_cRankForServer` (`CommonPacket.h:2155`) is likewise unset.
   Decide whether it matters here and say so — do not set it blindly.
3. **This is the same failure the title fix already documented and missed on the
   field next door.** The comment at
   [Handlers_Room.cpp:188-200](X2Lib/Offline/Handlers_Room.cpp#L188) explains,
   for `m_iTitleID`, that a zeroed field here "does not merely omit the title in
   the dungeon, it wipes it off the character for the rest of the session".
   So: **audit every field of `KRoomUserInfo` that `SetKRoomUserInfo`
   ([X2Unit.cpp:3343-3400](X2Lib/X2Unit.cpp#L3343)) assigns unconditionally**
   and check each one is filled. That audit is the phase; the one line is not.
4. The same zero also blanks the in-game emblem at
   [X2GageManager.cpp:1234](X2Lib/X2GageManager.cpp#L1234) and
   [X2GageUI.cpp:3131](X2Lib/X2GageUI.cpp#L3131); both are fixed by the same
   change. Confirm in play.
5. Correct the phase-16 status note in `OFFLINE_MODE_PHASE9_PLAN.md`. Leave the
   dead `X2StateBeginning.cpp` edit in place or revert it — but say which, and
   why.

### Exit test
Choose a character, enter and clear a dungeon, return to the character list. Its
emblem is the "Arranging" emblem, not a black box, and so are the other
characters'. Check a brand-new character too, because of the tutorial room.


### Status, 2026-09-07 — done, built and deployed; awaiting the exit test

The diagnosis held exactly as written. Three things changed.

**1. The one line, with one source of truth.** A new
`CX2OfflineServer::DefaultPvpRank()` ([X2OfflineServer.cpp](X2Lib/Offline/X2OfflineServer.cpp),
just above `MakeDefaultUnitInfo`) now owns the value and the whole explanation;
`MakeDefaultUnitInfo` and `MakeRoomUserInfo` both call it. The two sites drifting
apart *was* this defect, so the shared constant is the actual fix and the
assignment is the smaller half.

`PVPRANK_RANK_ARRANGE` is not a stand-in, it is the live value:
`KGSUser::GetPvpRankForClient` ([GSUserFunction.cpp:16689-16692](KncWX2Server/GameServer/GSUserFunction.cpp#L16689))
returns it for any character with fewer than ten official season matches.

**2. `m_cRankForServer` is deliberately left at 0.** The live GameServer does set
it — `m_cRank = GetPvpRankForClient()` / `m_cRankForServer = GetPvpRank()`, the
unsuppressed rank, at
[GSUserFunction.cpp:10786-10787](KncWX2Server/GameServer/GSUserFunction.cpp#L10786)
— but there is **not one read of `m_cRankForServer` anywhere in `X2Lib` or
`X2ServerProtocol`**. It is serialized for the CenterServer's matchmaking, which
offline does not have, and its constructor already zeroes it
([CommonPacket.h:2320](KncWX2Server/Common/CommonPacket.h#L2320)), so there are
no indeterminate bytes on the wire either. Setting it would be inventing a
second, differently-derived rank that nothing reads.

**3. The audit found a second, invisible clobber, and it is fixed too.**
`KUnitSkillData::m_vecSkillNote` was never filled by
`CX2OfflineSkill::FillUnitSkillData`, and `SetEqipSkillMemo` is a plain
assignment ([X2UserSkillTree.h:362](X2Lib/X2UserSkillTree.h#L362)) called
unconditionally by both `SetKUnitInfo` ([X2Unit.cpp:3267](X2Lib/X2Unit.cpp#L3267))
and `SetKRoomUserInfo` ([X2Unit.cpp:3441](X2Lib/X2Unit.cpp#L3441)). So entering a
dungeon room erased the skill-note memos `EGS_REG_SKILL_NOTE_MEMO_ACK` had just
registered, and `CX2GUUser::IsEquipSkillMemo`
([X2GUUser.cpp:5845](X2Lib/X2GUUser.cpp#L5845)) — what makes a memo's effect
apply in combat — went false in the dungeon the memo was registered for.
Silently: the skill-note UI reads `CX2Unit::m_mapSkillNote`
([X2Unit.cpp:1811](X2Lib/X2Unit.cpp#L1811)), a different member fed by a
different packet, so the pages kept showing their memos. `FillUnitSkillData` now
fills the vector from `CX2OfflineSkill::GetSkillNotes()`, which fixes the login
path (`X2OfflineServer.cpp:954`) and the room path together.

### The audit — every field `SetKRoomUserInfo` assigns unconditionally

`SetKRoomUserInfo` is [X2Unit.cpp:3343-3485](X2Lib/X2Unit.cpp#L3343). Flags
resolved against this build: `SERV_PVP_NEW_SYSTEM`, `PVP_SEASON2`,
`SERV_INTEGRATION`, `SERV_SKILL_NOTE`, `SERV_DELETE_ROOM_USER_INFO_DATA`,
`TITLE_SYSTEM`, `SERV_TITLE_DATA_SIZE`, `GUILD_MANAGEMENT`,
`SERV_RELATIONSHIP_SYSTEM`, `SERV_GATE_OF_DARKNESS_SUPPORT_EVENT` **on**;
`NEW_MESSENGER` and `SERV_GROW_UP_SOCKET` **off** (the latter appears only
commented out, `ServerDefine_Global.h:888`).

| `KRoomUserInfo` field | → `UnitData` | filled? | verdict |
|---|---|---|---|
| `m_nUnitUID`, `m_iOwnerUserUID`, `m_cUnitClass`, `m_wstrNickName`, `m_ucLevel` | identity | yes | — |
| `m_wstrIP` / `m_usPort` / `m_wstrInternalIP` / `m_usInternalPort` | P2P | yes | — |
| `m_kGameStat` | `m_GameStat` | yes (`MakeGameStat`) | — |
| `m_iTitleID` | `m_iTitleId` | yes | `ISSUES.md` #13 |
| `m_mapEquippedItem` | inventory | yes | and skipped for my own unit anyway (UID gate) |
| `m_UnitSkillData` slot-B state/date, equipped skills | skill tree | yes (`FillUnitSkillData`) | — |
| **`m_cRank`** | **`m_cRank`** | **NO** | **the defect. Fixed.** |
| **`m_UnitSkillData.m_vecSkillNote`** | `SetEqipSkillMemo` | **NO** | **second clobber, invisible. Fixed — see 3 above.** |
| `m_bMale` / `m_ucAge` | `m_bMan` / `m_Age` | no | harmless. Read only at [X2StateDungeonRoom.cpp:430](X2Lib/X2StateDungeonRoom.cpp#L430) and [X2StatePVPRoom.cpp:637](X2Lib/X2StatePVPRoom.cpp#L637), both behind `AuthLevel >= XUAL_SPECIAL_USER`, which offline never grants (`m_cAuthLevel = XUAL_NORMAL_USER`). `SetKUnitInfo` never sets them either, so there is nothing to clobber. |
| `m_uiKNMSerialNum` | `m_iNMKSerialNum` | no | harmless. Nexon Messenger only (`X2Community.cpp`), and the `KUnitInfo` path sends 0 too — same value, no clobber. |
| `m_bIsGameBang` | `m_bIsGameBang` | no (false) | correct. There is no PC bang offline. |
| `GetBonusRate( BT_PREMIUM_EXP_RATE )` | `m_fAddExpRate` | no (0) | correct. Init is 0, and the only reads are `> 0.f` guards on the PvP result screen. |
| `m_wstrGuildName` / `m_ucMemberShipGrade` | guild name / grade | no | consistent. Guild creation is refused offline (`Handlers_Social.cpp:62`) and the `KUnitInfo` path sends an empty name, so the room agrees with login. **Revisit if guilds are ever implemented** — this would then wipe the guild name off the character on room entry. |
| `m_cWeddingStatus` / `m_iLoverUnitUID` | wedding state | no (0) | correct. `SEnum::WS_NONE == 0` (`Enum.h:1174`), and offline has no relationship system. |
| `m_iGateOfDarknessSupportEventTime` | same | no (0) | correct. Offline never sets it in the `EGS_SELECT_UNIT` ACK either, and 0 means the event is off. |

**One adjacent field, reported and deliberately not changed.**
`KRoomUserInfo::m_iRidingPetUID` / `m_usRidingPetID` are not read by
`SetKRoomUserInfo` but by its caller: `Set_KRoomSlotInfoOfMine` runs
`SetOrClearRidingPetInfo` ([X2Room.cpp:2183](X2Lib/X2Room.cpp#L2183)), so a zero
calls `ClearRidingPetInfo()` on my own unit. The live server *does* fill them,
from `GetSummonedRidingPetInfo` ([GSUserFunction.cpp:11167](KncWX2Server/GameServer/GSUserFunction.cpp#L11167)),
and offline mounts are otherwise implemented. Left alone on purpose: that field
is what makes `CX2GUUser` start a run mounted
([X2GUUser.cpp:1498-1518](X2Lib/X2GUUser.cpp#L1498)), so filling it would change
dungeon-start behaviour for a mounted player — a gameplay change nobody asked
for — and doing it correctly needs ride-state tracking the offline server does
not keep (`KOfflineSession` tracks `m_nSummonedPetUID` but has no riding
equivalent, and `MakeRoomUserInfo` has no session in scope). The current effect
is that mounting and then opening a dungeon room dismounts you. Worth a phase of
its own if it ever bothers anyone; it is not `ISSUES_2.md` #3.

**`m_vecPet` is a non-issue**: the live server fills it (`GetSummonedPetInfo`,
`GSUserFunction.cpp:11165`) but no client site anywhere in `X2Lib` reads
`KRoomUserInfo::m_vecPet`, so the offline omission costs nothing.

### Phase-16 note corrected
`OFFLINE_MODE_PHASE9_PLAN.md`'s phase-16 status block now carries a
**CORRECTION, 2026-09-07** paragraph: `X2StateBeginning.cpp` is dead code, the
`X2StateServerSelect.cpp` call was already correct in the shipped source, only
the `MakeDefaultUnitInfo` half ever worked, and it was incomplete. The dead
`X2StateBeginning.cpp` edit is **left in place** — reverting it means another
byte-level patch of a CP949 file for no behavioural gain, and the guarded
version is the type-correct one.

### Build and deploy
`X2Lib` then `X2.exe`, 0 errors (the post-build event fails on the studio drive
letters, as always). Deployed to `X2_offline.exe`, 14,341,632 bytes, verified by
reading the name, size and mtime programmatically. **Not yet play-tested** — the
exit test needs a human, and the skill-note fix wants one too: register a memo,
enter a dungeon, confirm its effect still applies.

---

# Phase 32 — Result screen is F rank for every unit (`ISSUES_2.md` #4)

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
Every rank stamp on the dungeon result screen reads F, for every unit, every
run.

### Diagnosis — CONFIRMED
`KDungeonUnitResultInfo`'s constructor sets all five rank fields to **`1`, which
is `RT_F`** ([ClientPacket.h:935-963](KncWX2Server/Common/ClientPacket.h#L935)) —
not `RT_NONE`. `SendDungeonResultData`
([Handlers_Room.cpp:2761-2794](X2Lib/Offline/Handlers_Room.cpp#L2761)) sets the
four *scores* and never assigns any `m_c*Rank`; nothing in `X2Lib/Offline/`
does. The client draws the letter straight from the packet
(`X2StateDungeonResult.cpp:549`, `:577`, `:621`, `:651`, `:686`, `:774`) and
`Get_UI_RANK_SCORE`'s `default:` is F
([X2StateDungeonResult.cpp:2864](X2Lib/X2StateDungeonResult.cpp#L2864)), so
`RT_NONE` would render F as well.

This is an acknowledged omission, documented three times
(`Handlers_Room.cpp:1380`, `:2840`, `:2877`) — and the third one matters beyond
the screen: `QuestOnDungeonClear` is called with `RT_NONE` (`:2891`), so **rank
sub-quests and rank title missions can never tick.** Their comparisons are
correct (`X2OfflineQuest.cpp:1454`, `X2OfflineTitle.cpp:630`); they are starved
of input.

The engine is fully reproducible from the tree. `SERV_DUNGEON_RANK_NEW` is on
([ServerDefine.h:845](KncWX2Server/Common/ServerDefine.h#L845)), and:

- **`KncWX2Server/ServerResource/US/ResultData_new.lua`** (537 lines, in the
  tree) holds all five functions and their thresholds: `DUNGEON_TIME_RESULT`
  :85, `DUNGEON_DAMAGE_RESULT` :152, `DUNGEON_COMBO_RESULT` :224,
  `DUNGEON_TECHNICAL_RESULT` :321, `DUNGEON_TOTAL_RANK_RESULT` :439, plus
  `WEIGHT_BY_DUNGEONID` :33 and a `RANK_TYPE` table that matches the C++ enum.
  `ResultData.lua` beside it is the pre-`SERV_DUNGEON_RANK_NEW` version this
  build does **not** use (`CnSimLayer.cpp:345`).
- **`KncWX2Server/CenterServer/ResultProcess.cpp:1938-2035`** is the reference
  orchestration: one `lua_tinker::call` per rank, each function reporting back
  through `SetResultData( bonus, rank )` which C++ reads via
  `spRoomUser->GetPercent()` / `GetRank()`, then `DUNGEON_TOTAL_RANK_RESULT`
  over the four, clamped into `[RT_F, RT_SS]`. `:55-95` is the complete binding
  list — `KResultProcess`, `KRoomUser` (18 getters) and
  `KDungeonMonsterManager` (5 die counts plus `PrintLuaLog`).
- **Every per-unit input already arrives.** `KDungeonPlayResultInfo`
  ([CommonPacket.h:2457-2475](KncWX2Server/Common/CommonPacket.h#L2457)) carries
  `m_iTotalGivenDamamge`, `m_iTotalAttackedDamage`, `m_iTotalSumNpcHP`,
  `m_iTotalRecoveryHP`, `m_iUsingKindOfSkill`, `m_iCommandTechScore` and
  `m_iSkillTechScore` under `SERV_DUNGEON_RANK_NEW`; the client fills them at
  `X2Game.cpp:9449-9452` from `CX2GUUser` accumulators, and the offline server
  already stores the whole struct (`Handlers_Room.cpp:1385`,
  `X2OfflineServer.h:236`) and clears it per run (`ClearPlayRun`, `:973`).

### What to do
1. **A new loader, `CX2OfflineResultTable`**, modelled on
   `X2OfflineBattleField.cpp` — which already runs a server `.lua` out of the
   client `.kom` via `LoadDataFile` + `GetLuaBinder()->DoMemory` with a
   `DoMemoryNotEncript` fallback. Copy that shape, not `X2OfflineStatTable`'s:
   this file defines *functions* to be called later, not rows to be parsed, so
   the loader's job is to run the chunk into the Lua state and keep it.
2. **Test the payload, not the handle.** `if( NULL == kInfo )` is dead code —
   `MASSFILE_MEMBERFILEINFO_POINTER` is a struct by value carrying
   `operator const T*` that returns `this`. Use
   `if( NULL == kInfo->pRealData || kInfo->size <= 0 )`, per the phase-9 doc's
   §0.3 point 3. Getting this wrong makes an unpacked file report as a corrupt
   one, which sends the user to fix the wrong thing.
3. **Bind every method the file names, including the ones inside `--[[ ]]`
   comment blocks and behind other regions' flags** — phase-9 §0.3 point 4.
   `lua_tinker` turns a call to an unbound method into an error that abandons
   the rest of the chunk, and the total-rank function is at the very end of this
   file.
4. **Publish `DUNGEON_ID`.** `WEIGHT_BY_DUNGEONID` subscripts it by name for
   eleven Henir and secret dungeons. `X2OfflineLuaEnum` already publishes
   `UNIT_CLASS`, `PET_UNIT_ID`, `RIDING_PET_UNIT_ID`, `ITEM_TYPE`,
   `ITEM_GRADE` and `VILLAGE_MAP_ID`; add `DUNGEON_ID` the same way, generated
   from the **client's** `X2Dungeon.h` by the existing generator, not retyped.
   Many `DI_*` names in that header are commented out, so the generator must
   read only the live ones and the Lua must tolerate a `nil` subscript (it
   already falls through to `fConstDungeon = 1.0`).
5. **Adapt the arguments offline has no party for.** `nEndNumMember` and
   `iPartyTotalGivenDamage` are party figures; solo they are 1 and the player's
   own total damage. `GetBaseHP` has to come from `CX2OfflineStatTable` plus
   gear — the same source phases 3 and 5 built for the village unit. Write down
   what each substitution is and why; this is the part a later phase will need
   to re-read.
6. **Wire all five ranks into `SendDungeonResultData`**, then pass the real
   total rank to `QuestOnDungeonClear` instead of `RT_NONE` (`:2891`) so rank
   quests and rank titles start working, and take their clear types off
   `X2OfflineQuest.cpp`'s undriven list — as phases 23 and 24 had to.
7. **Fix the `best_rank` comparison, which becomes wrong the moment this phase
   lands.** [X2OfflineDB.cpp:1409-1418](X2Lib/Offline/X2OfflineDB.cpp#L1409)
   keeps the *lowest* number on the stated grounds that "rank 1 is S and the
   client counts down", citing a `CX2Dungeon::RANK_TYPE` that does not exist.
   The real enum is `CX2DungeonRoom::RANK_TYPE` with `RT_F=1 … RT_SS=8` —
   **higher is better** — so as written it would pin `best_rank` to F forever.
   Same wrong comment at `X2OfflineDB.h:319`. `m_cMaxTotalRank`
   (`X2OfflineServer.cpp:927`) is read by no client code today, so this is
   currently invisible; it stops being invisible here.
8. **Degrade visibly.** With the file unpacked, log which file is missing and
   what to do about it, and leave the ranks at their constructor default rather
   than guessing. Per `CLAUDE.md`, a Lua *function* gets **no** fallback — do
   not transcribe the thresholds into C++ as a stopgap, and do not invent a
   curve.
9. **Name the file when the work is done, not before**:
   `KncWX2Server/ServerResource/US/ResultData_new.lua`, which needs
   XOR-encrypting and packing into `data036.kom` if it is not already. Then
   confirm from the loader's own log line that it loaded, rather than from the
   build succeeding.

### Traps
- `Handlers_Room.cpp` is 132 KB. Grep to `SendDungeonResultData`; do not read it
  through.
- The rank **EXP bonus** (`fComboBonus` and friends, 0.01-0.05) is deliberately
  out of scope, as it was for phases 4 and 17. Say so; do not quietly add it.
- The dungeon ID the Lua expects is `iDungeonID + cDifficulty` — the packed
  form, not the base ID (`ResultProcess.cpp:1948`). Passing the base ID makes
  every `WEIGHT_BY_DUNGEONID` lookup miss silently and return 1.0.
- `fMonsterWghtTime` is a **denominator**. If it is ever 0 the result is `inf`
  and the rank is F — indistinguishable from the bug being unfixed. Guard it and
  log when the guard fires.

### Exit test
Clear a dungeon. The five stamps show a spread of letters that changes with how
the run went — a fast clear improves Time, taking no damage improves Damaged —
and `offline_server.log` prints the inputs and the five outputs for the run.
Expect the letters to be **pessimistic**; that is phase 33, not a failure here.

---

# Phase 33 — Monster grade, so phase 32's ranks are accurate

**Flag:** `SERV_IRUHADEV_OFFLINE`

Not an `ISSUES_2.md` item. Split out of phase 32 with the user, 2026-09-07:
phase 32 produces real ranks with a wrong weight, and this phase corrects the
weight. **Run it only once 32's ranks are on screen.**

### The gap
Three of the four rank functions divide by a graded monster weight:

```
normal*1 + lowElite*2 + highElite*3 + middleBoss*4 + boss*12
```

Offline every kill is a normal NPC, so the weight is just the kill count. A
dungeon of 40 trash plus one boss weighs 52 on live and 41 offline — the
denominator is ~27% low, the quotient is high, and Time / Combo / Technical all
skew toward F. `DUNGEON_DAMAGE_RESULT` is the exception: it computes a weight
and never uses it, so the Damaged rank is already accurate after phase 32.

### Why the grade is missing
- [Handlers_Room.cpp:1564-1568](X2Lib/Offline/Handlers_Room.cpp#L1564) says it:
  `m_cMonsterGrade` stays `MG_NORMAL_NPC` because the client's `NPCData` only
  carries a grade under `X2TOOL`.
- The parse is [X2DungeonSubStage.cpp:1651](X2Lib/X2DungeonSubStage.cpp#L1651),
  `LUA_GET_VALUE_ENUM( ..., L"MONSTER_GRADE", ... )`, and the enum itself is
  `X2TOOL`-only ([X2DungeonSubStage.h:46-53](X2Lib/X2DungeonSubStage.h#L46)).
- **The client enum has three values** (`MG_NORMAL_NPC`, `MG_BOSS_NPC`,
  `MG_MIDDLE_BOSS_NPC`); the server weight wants five. The two elite tiers have
  no client-side name at all. That asymmetry is the real work of this phase, and
  the reason it is not a one-liner.

### What to do
1. Revive the enum and the parse under `SERV_IRUHADEV_OFFLINE` — a revival in
   the sense of the three `CLAUDE.md` already lists for offline mode: code the
   shipped client contains but never runs.
2. Decide, and write down, how the client's three grades map onto the server's
   five. `CommonPacket.h:3637`'s comment names the server side as
   `CXSLUnitManager::MT_NORMAL_NPC`; find that enum and map by name, not by
   ordinal. If the elite tiers genuinely have no client-side source, say so and
   fold them into the nearest grade **with a log line naming the choice** — do
   not silently pick.
3. Count kills by grade per run on the room (`m_iKillNPCNum` is the current
   single counter) and expose the five counts to Lua as
   `KDungeonMonsterManager`'s five `Get*DieCount` methods.
   `KncWX2Server/CenterServer/DungeonMonsterManager.{h,cpp}` is the reference
   for the struct, `ResultProcess.cpp:89-93` for the binding names.
4. `ClearPlayRun()` must clear the new counters too, for the reason phase 17
   added it: a room outlives its match.

### Exit test
Clear the same dungeon before and after. The graded counts appear in
`offline_server.log`, the boss is counted as a boss, and the Time / Combo /
Technical letters improve relative to phase 32's run.

---

# Phase 34 — Using an item from the bag zeroes the displayed ED (`ISSUES_2.md` #5)

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
Opening a "box" item empties the displayed ED. The saved value is intact; only
the UI is wrong. (The user's wider ask — cover **all** box items — is
`ISSUES_2.md` #7 and lives in `OFFLINE_MODE_PHASE36_PLAN.md`, not here.)

### Diagnosis — CONFIRMED
It is not specific to boxes: **every successful use of any item from the bag
does it.** `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` sets `kAck.m_iED = 0` at
[Handlers_Inventory.cpp:202](X2Lib/Offline/Handlers_Inventory.cpp#L202) and never
assigns it again on any path, including the `NET_OK` path at `:308-314`. The
client assigns it raw:

```cpp
// X2UIInventory.cpp:8919, behind IsValidPacket( m_iOK )
g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_ED = kEvent.m_iED;   // <- 0
```

and `UpdateInventorySlotList` then repaints the wallet from it
([X2Inventory.cpp:285](X2Lib/X2Inventory.cpp#L285)). Read §0.3 for why refusal
paths are harmless and only success paths matter. The real server sources the
field from `GetED()` (`GSUserInventory.cpp:4544`, `:5941`).

Boxes reach this handler rather than the cube handler because the client checks
`GetCanUseInventory()` **first** (`X2UIInventory.cpp:5917`, default branch
`:6902-6905` → `USE_ITEM`) and only falls through to `IsRandomItem` →
`EGS_OPEN_RANDOM_ITEM_REQ` when that is false (`:6927-6929`).
`Handler_EGS_OPEN_RANDOM_ITEM_REQ` is **already correct** — phase 20 fixed it and
left the comment naming this exact hazard
([Handlers_Social.cpp:2633-2637](X2Lib/Offline/Handlers_Social.cpp#L2633)).

The audit found **three** broken handlers, not one:

| handler | file:line | ED status |
|---|---|---|
| `EGS_USE_ITEM_IN_INVENTORY_REQ` | `Handlers_Inventory.cpp:190` | **always 0** (`:202`) — the reported defect |
| `EGS_RESOLVE_ITEM_REQ` (dismantle) | `Handlers_Inventory.cpp:2236` | **always 0** (`:2249`); the success path at `:2386` ships it |
| `EGS_ATTRIB_ENCHANT_ITEM_REQ` | `Handlers_Inventory.cpp:1513` | **never set at all**; `KEGS_ATTRIB_ENCHANT_ITEM_ACK` ([ClientPacket.h:4871](KncWX2Server/Common/ClientPacket.h#L4871)) has no constructor, so it ships **indeterminate stack bytes** — a random wallet, not an empty one |

Everything else was checked and is clean: `BUY_ED_ITEM`, `SELL_ED_ITEM`,
`REPAIR_ITEM`, `ENCHANT_ITEM`, `SOCKET_ITEM`, `WARP_BY_BUTTON`, `GET_ITEM`
(drop), `ApplyDungeonReward`, `PushSelectUnitNotifications`,
`MakeUnitInfoFromRow`. The letter and guild-ad handlers set 0 but are pure stubs
that always reply an error, so the client never reads it.

### What to do
1. Fix the three. Set the real ED **on the initial error value**, the way
   `Handler_EGS_OPEN_RANDOM_ITEM_REQ` does at `:2637` — that is what makes it
   impossible for a later `return Reply(...)` to ship a zero.
2. **Add the missing helper.** There is no shared "money changed" fill; every
   handler open-codes `LoadUnit` → adjust → `SaveProgress` →
   `kAck.m_iED = kUnit.m_iED`, which is precisely why three of them forgot. A
   `bool CX2OfflineServer::FillAckED( KOfflineSession&, OUT int& iED )` called
   right after `LoadUnit` makes the class structurally impossible. Add it and
   use it in the three; converting the clean handlers is optional and belongs in
   a separate commit if done at all.
3. Give `KEGS_ATTRIB_ENCHANT_ITEM_ACK`'s new initialisation a comment noting the
   struct has no constructor, so the next reader does not assume 0. Worth a
   sweep for other constructor-less `DECL_PACKET`s the offline server replies
   with.
4. **`ISSUES_2.md` #5b is no longer this phase's job.** It became `ISSUES_2.md`
   #7 and is planned in full as `OFFLINE_MODE_PHASE36_PLAN.md`, phases 36-45.
   Two findings from writing that document change what this phase should do:
   - **Phase 36 fixes the ED on `EGS_USE_ITEM_IN_INVENTORY_REQ`**, because the
     Philosopher's Scroll (item 160267) cannot be verified while the same reply
     zeroes the wallet. So this phase keeps only `EGS_RESOLVE_ITEM_REQ` and
     `EGS_ATTRIB_ENCHANT_ITEM_REQ` — and whichever of 34/36 runs second inherits
     the `FillAckED` helper rather than adding it twice.
   - **34, 35 and 36 all edit this handler.** Do not run them as parallel
     conversations.
5. `Handler_EGS_RESOLVE_ITEM_REQ` also contains the phase-29 pattern at `:2369`
   and gets it right — worth reading as the counter-example.

### Exit test
Note the ED, then: use a potion from the bag, open a box, dismantle an item, add
an attribute. The displayed ED matches `select ed from unit` after each one.
Whether the box *delivers* anything is phase 36/37's exit test, not this one —
here it only has to leave the wallet alone.

---

# Phase 35 — Fetch aura: every hatched pet has it (`ISSUES_2.md` #6 → QoL #7)

**Flag:** `SERV_IRUHADEV_OFFLINE`

**Run after phase 34** — both edit `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`.

### The decision this phase implements
`ISSUES_2.md` #6 reports the fetch aura not persisting. `QUALITY_OF_LIFE.md` #7
asks for "all pets (except the pet that still in crystal) always have the fetch
aura unlocked so no need to buy fetch aura anymore". **The user chose QoL #7
instead of the persistence fix**, 2026-09-07.

Recorded so the next reader is not confused by the difference: the underlying
defect is that nothing ever *writes* the flag, and this phase does not fix that
— it makes the flag's stored value irrelevant. If the purchased toggle is ever
wanted back, the fix is in "Not done" below.

### Diagnosis — CONFIRMED
"Fetch aura" is the pet's drop-item pickup skill, under `PET_DROP_ITEM_PICKUP`
(defined, [Always.h:1003](KTDXLIB/Always.h#L1003)) — **not** `PET_AURA_SKILL`,
which is the intimacy stat buff. There is no dedicated toggle packet; it rides
`EGS_USE_ITEM_IN_INVENTORY_REQ` carrying item
`ACTIVATION_DROP_ITEM_PICKUP_SKILL = 500720`
([X2Define.h:1356](X2Lib/X2Define.h#L1356)) with the pet UID in `m_iTempCode`:

- request built at `X2UIPetInfo.cpp:818-862` → `X2UIInventory.cpp:8879-8886`
- ACK read at [X2UIInventory.cpp:8985-8990](X2Lib/X2UIInventory.cpp#L8985) →
  `SetPetAutoLooting( kEvent.m_iTempCode )` → `:14578-14606`, which flips the
  flag **client-side only**, on `CX2PET`, `CX2UIPetInfo` and the unit

The offline handler echoes `m_iTempCode`
([Handlers_Inventory.cpp:203](X2Lib/Offline/Handlers_Inventory.cpp#L203)) and
returns `NET_OK`, so the aura lights up for the session — and **no `unit_pet`
row is ever touched.** Nothing in `X2Lib/Offline/` writes `m_bAutoLooting` true;
the only writes are `false` at pet creation
([Handlers_Social.cpp:1672](X2Lib/Offline/Handlers_Social.cpp#L1672)). The column
is otherwise fully wired: loaded (`X2OfflineDB.cpp:2912`), saved (`:2951`,
`:2994`) and reported (`MakePetInfo`, `Handlers_Social.cpp:1506-1508`).
`select * from unit_pet` shows `auto_looting = 0` on all five rows today.

**No schema change is needed** — the column has existed since `SCHEMA_V7`.

"Except the pet still in crystal" needs no special handling: a `unit_pet` row
exists only once an egg has been hatched by `Handler_EGS_CREATE_PET_REQ`
([Handlers_Social.cpp:1660-1680](X2Lib/Offline/Handlers_Social.cpp#L1660)); an
unhatched crystal is an inventory item. **Confirm that in code before relying on
it.**

### What to do
1. Force the flag where the client reads it, not in the database:
   `kOut.m_bAutoLooting = true;` in `MakePetInfo`
   ([Handlers_Social.cpp:1506](X2Lib/Offline/Handlers_Social.cpp#L1506)), behind
   `SERV_IRUHADEV_OFFLINE` with the original in `#else`. One line, one place,
   covering existing pets and new ones alike, and leaving the saved data
   untouched so the flag stays the revert switch. Do **not** migrate the DB for
   this.
2. **Check `MakePetInfo` is the only builder of a pet info the client reads the
   flag from.** `X2PetManager.cpp:2401` and `:2643`, `X2GUUser.cpp:1565` and
   `:2070` read it from `KPetInfo`, which `MakePetInfo` builds — but
   `X2Room.cpp:2439` and `X2SquareUnit.cpp:640` read an auto-looting flag off
   *room* and *square* structs. Find what fills those offline, or establish that
   nothing does, before declaring the phase done — otherwise the aura works in
   the village and not in a dungeon, which is where it matters.
3. **Refuse item 500720 rather than consuming it.** With the aura always on the
   item has nothing to do, and the generic path would still eat it. Refuse with
   the item intact and a log line, in the shape the handler already uses at
   `Handlers_Inventory.cpp:216-233`. This is a direct consequence of the
   decision above, not extra scope.
4. Leave the item in the cash shop or pull it — but decide. It is seeded at
   `X2OfflineCashSeed.h:405` (`{ 349, 500720, 62, 1, 0 }`), and it is the item
   `ISSUES.md` #8 was about.

### Not done, and how to do it later
The persistence fix, if it is ever wanted: in
`Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`, after a successful `ConsumeOne` of
500720, `LoadPets`, set `m_bAutoLooting = true` on the row whose
`m_nPetUID == kReq.m_iTempCode` (falling back to `kSes.m_nSummonedPetUID` when
the temp code is 0), and `SavePet`. `Handler_EGS_SET_AUTO_FEED_PETS_REQ`
([Handlers_Social.cpp:2180-2205](X2Lib/Offline/Handlers_Social.cpp#L2180)) is the
exact load / mutate / `SavePet` template.

### Exit test
Summon a pet that has never had the aura bought. Its pet-menu button reads
active (`PET_LOOTING_ACTIVE`, `X2UIPetInfo.cpp:2010-2045`) and dropped items are
picked up automatically in a dungeon. Relog and check both again. Buying 500720
is refused with the item still in the bag.

---

# Appendix — latent bugs found while writing this document

Recorded so they are not rediscovered. None is in `ISSUES_2.md`; each is
assigned to the phase that makes it live.

1. **`best_rank` keeps the wrong extreme.**
   [X2OfflineDB.cpp:1409-1418](X2Lib/Offline/X2OfflineDB.cpp#L1409) and the
   comment at `X2OfflineDB.h:319` both claim rank 1 is S and the client counts
   down, citing a `CX2Dungeon::RANK_TYPE` that does not exist. It is
   `CX2DungeonRoom::RANK_TYPE`, `RT_F=1 … RT_SS=8`. Invisible today because
   ranks are never written and `m_cMaxTotalRank` is read by no client code.
   **Assigned to phase 32.**
2. **The title loader has phase 30's asymmetry.** The client reads
   `m_bUpperDifficulty` 3× (`X2TitleManager.cpp:635`, `:712`, `:722`), the
   server 6× — missing `TMCT_DUNGEON_TIME`, `TMCT_DUNGEON_RANK`,
   `TMCT_DUNGEON_DAMAGE`. "Any" still works via the `-1` sentinel; "≥ this
   difficulty" does not. **Assigned to phase 30, decide explicitly.**
3. **`KEGS_ATTRIB_ENCHANT_ITEM_ACK` has no constructor**
   ([ClientPacket.h:4871-4880](KncWX2Server/Common/ClientPacket.h#L4871)) and the
   offline handler never touches `m_iED`, so it ships stack garbage.
   **Assigned to phase 34**, along with a sweep for other constructor-less
   `DECL_PACKET`s the offline server replies with.
4. **`InsertItem`'s `clear()` serves no caller** and is one added line away from
   re-creating phase 29 anywhere. **Noted in phase 29 as an optional root fix.**
5. **Phase 16's status note in `OFFLINE_MODE_PHASE9_PLAN.md` is wrong** — it
   credits a fix in a file this build never constructs. **Assigned to phase
   31**, and it is the thirteenth entry in that document's §0.1 stale-claim
   count.

---

# Verification, across the batch

There is no test suite; reading the logs is the verification.

```sh
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"
cd "$DATA"

# the phase-8 loop - should stay empty
grep -aE "UNHANDLED|EXCEPTION" offline_packets.log | sort | uniq -c | sort -rn

# per-phase evidence
grep -a "QUEST" offline_server.log | tail -30            # 29, 30
grep -a "result screen\|RANK" offline_server.log         # 32, 33
grep -a "ITEM     used item\|CUBE" offline_server.log     # 34
grep -a "PET" offline_server.log                         # 35

# the save, before and after
python -c "import sqlite3;c=sqlite3.connect(r'els_db.sql');\
print(c.execute('select unit_uid,nickname,level,ed from unit').fetchall());\
print(c.execute('select pet_uid,name,auto_looting from unit_pet').fetchall())"
```

Quit through the menu rather than killing the process, or there is no `CENSUS`
line and no `els_db.sql.bak`.

**When a change cannot be verified by reading code, add a diagnostic rather than
reasoning further** — `CX2OfflineLog::Server( L"..." )` is flushed per line and
available anywhere in `X2Lib`. Gate it behind a short-lived
`SERV_IRUHADEV_*_DEBUG` flag, tag the lines with a grep-able prefix, and throttle
per-frame logging to about once a second. Phases 32 and 33 will need this; the
other five should not.

Finally, per this project's own habit: **each phase updates `MODS.md`** and
appends a "what actually happened" section here, *including the places this
document turned out to be wrong*. That record is what made phases 19-27 cheap.
