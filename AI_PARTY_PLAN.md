# AI party members in offline dungeons

**Status:** **phases 0-4 done and play-tested, with nothing owed.**
Phase 0 was a gate and it FAILED for the intended cast, so the plan is re-pointed at
`NUI_CSM_PVP_HERO_*` — see *Phase 0*. Phase 1 PASSED (2026-09-05): auto-party
puts an AI party member in the dungeon, fighting on your team, and the normal
start button still goes in alone. Phase 2 PASSED (2026-09-06) after two defects
and a redesign: three heroes drawn at random, placed on the line map's own party
start slots, and the party now *survives* a stage change rather than being
rebuilt at each one. Phase 3 PASSED (2026-09-06) on its first cycle with no
defects: the matchmaking ceremony — queue, accept popup, cancel and deny — sits
in front of it all, entirely in the offline server, with no client file touched.
Phase 4 PASSED (2026-09-06) after one defect: the party now has HP/MP bars in
the real party HUD's own slots, with portraits, names and levels. Its first
play-test produced **a bot death** - which turned out to be the studio killing
every ally at dungeon clear, and left the bars behind (*Defect 1*, fixed and
re-tested the same day). Its second closed **both** of the checks carried since
phase 2: the solo button still goes in alone, and an auto-party run pays
**exactly the same EXP** as a solo one. Phase 5 planned, nothing else
implemented.
**Read before starting a phase:** phase 1's three defects, phase 2's two and
phase 4's one, which between them change what later phases may assume about the
dungeon path - and phase 4's is the one that says what happens to the party in
the last seven seconds of a run.
Written 2026-09-05, last updated 2026-09-06.
**Flag:** none of its own. This is an extension of offline mode, so every edit
goes behind the existing **`SERV_IRUHADEV_OFFLINE`** — there is no
`SERV_IRUHADEV_AI_PARTY`. Offline mode's nine phases are done; see
`OFFLINE_MODE_PLAN.md`.

Two consequences of reusing the flag rather than adding one:

- **`SERV_IRUHADEV_OFFLINE` is defined in two places and both must stay
  toggled together** — `KTDXLIB/Always.h:2446` *and*
  `X2ServerProtocol/X2ServerProtocol_2010.vcxproj`'s `US_SERVICE`
  `PreprocessorDefinitions`. Nothing in this plan touches `X2ServerProtocol`,
  so no new work there, but do not "tidy" one side away.
- **The new client-side edits land in files offline mode has not touched
  before** — `X2Lib/X2Game.cpp` and `X2Lib/X2Room.h`. `MODS.md`'s
  `SERV_IRUHADEV_OFFLINE` row lists client files explicitly, so it gains two
  entries rather than a new row. Reverting offline mode now also reverts the AI
  party, which is the intended trade.

## Context

Offline mode plays every dungeon solo. The client's auto-party button still
works — it sends `EGS_AUTO_PARTY_DUNGEON_GAME_REQ` — and the offline server
refuses it with `ERR_PARTY_23` because there is nobody to match with
([Handlers_Social.cpp:498](X2Lib/Offline/Handlers_Social.cpp#L498)). Dungeons
past the early game are tuned for a party, and a solo player has no way to fill
one.

The goal: **make auto-party fill the remaining three slots with AI-controlled
characters instead of searching for humans.** They should read as party members
— named, levelled, on your team, fighting the dungeon with you — not as
summoned pets.

## Scope: auto-party only. The solo button stays solo.

Bots appear **only** when the player presses auto-party. Picking a dungeon and
starting it normally must behave exactly as it does today — one slot, one
character, no companions. Anyone who wants a solo run must still be able to have
one, and that is the button for it.

Two facts make this a clean separation rather than a conditional threaded
through the feature:

- **The two entry points are already separate handlers.**
  `EGS_QUICK_START_DUNGEON_GAME_REQ` is the solo button
  ([Handlers_Room.cpp:639](X2Lib/Offline/Handlers_Room.cpp#L639));
  `EGS_AUTO_PARTY_DUNGEON_GAME_REQ` is the auto-party button
  ([Handlers_Social.cpp:498](X2Lib/Offline/Handlers_Social.cpp#L498)). Nothing
  in this plan edits the first one. `MakeRoomSlots` gains an optional bot count
  that defaults to zero, so the solo call site keeps its current meaning without
  being touched.
- **`KEGS_AUTO_PARTY_DUNGEON_GAME_REQ` is a `typedef` of
  `KEGS_QUICK_START_DUNGEON_GAME_REQ`** ([ClientPacket.h:7804](KncWX2Server/Common/ClientPacket.h#L7804))
  — same four fields, same dungeon id, difficulty, get-item type and mode. So
  the auto-party handler can be built by copying the solo one and adding bots,
  with no new plumbing and no risk of the two diverging on what a dungeon
  request means.

**The client-side widening needs no scope guard of its own.** All three gates in
`CX2Game` are driven by bot slots *existing* — `DeleteNpcSlot()` iterates
`m_bNpc` slots, and the spawn loop iterates `m_vecNpcSlot`. A solo room sends no
bot slots, so all three are no-ops on that path automatically. Do not add a
"was this auto-party?" test to `CX2Game`; the room already says.

## The discovery that shapes this whole plan

**KOG already built this.** It is the PvP AI-opponent system, and it is
compiled into this build:

```
PVP_BOT                KTDXLIB/Always.h:2174                    (client)
SERV_PVP_NEW_SYSTEM    KncWX2Server/Common/ServerDefine.h:1188  (shared, reaches X2Lib)
```

A "PvP NPC" is a room *slot* occupied by an AI character rather than a player.
The entire pipeline exists and is wired end to end. Every piece below was read
in this tree, not assumed:

| Concern | Where it already works |
|---|---|
| Wire format | `KRoomUserInfo::m_bIsPvpNpc` + `m_iRating` — **unconditional** fields, the `#ifdef` around them in `CommonPacket.h` is commented out |
| Slot carries bot data | `CX2Room::SlotData::m_bNpc / m_iNpcUid / m_iNpcId / m_iNpcRating / m_wstrNpcName` ([X2Room.h:166-170](X2Lib/X2Room.h#L166)) |
| Slot ingest | [X2Room.cpp:2323-2331](X2Lib/X2Room.cpp#L2323) — `m_bNpc = m_kRoomUserInfo.m_bIsPvpNpc`, destroys the 3D unit viewer, keeps `m_pUnit` |
| **P2P exclusion** | [X2Room.cpp:265-268](X2Lib/X2Room.cpp#L265) — `NetworkProcess()` skips `m_bNpc` slots, so **no UDP peer is added for a bot** |
| Player-unit exclusion | [X2Game.cpp:1861-1866](X2Lib/X2Game.cpp#L1861) — `AddUserUnit()` skips NPC slots |
| Slot to in-game bot | `CX2Room::DeleteNpcSlot()` ([X2Room.cpp:1070](X2Lib/X2Room.cpp#L1070)) moves each bot slot into `m_vecNpcSlot` as a `RoomNpcSlot`, carrying name, level, rating and the five combat stats off `m_pUnit->GetUnitData()->m_GameStat` |
| Spawn | ~~[X2Game.cpp:8093-8117](X2Lib/X2Game.cpp#L8093), inside `CX2Game::Handler_EGS_PLAY_START_NOT` — base class, not `CX2PVPGame`~~ — **wrong for dungeons, and it cost a build/play cycle.** That handler is PvP-and-room only; the dungeon hook is `CX2DungeonGame::SubStageStart()` ([X2DungeonGame.cpp:1258](X2Lib/X2DungeonGame.cpp#L1258)), beside the studio's own `CreateAllyEventMonster`. See *Defect 1* under phase 1 |
| "This NPC is a bot" | [X2GUNPC.cpp:2151-2157](X2Lib/X2GUNPC.cpp#L2151) and [:2529-2535](X2Lib/X2GUNPC.cpp#L2529) — `m_UnitUID < -1 && g_pX2Room->IsNpcSlot( m_UnitUID )` |
| Stat override | [X2GUNPC.cpp:3915-3921](X2Lib/X2GUNPC.cpp#L3915) — a bot's HP/atk/def come from its `RoomNpcSlot`, not from the NPC stat table |
| Team HP bars | `InsertPvpMemberUI` / `UpdatePvpMemberGageData` ([X2Game.cpp:6438](X2Lib/X2Game.cpp#L6438), [X2PVPGame.cpp:176](X2Lib/X2PVPGame.cpp#L176)) |
| Revive | [X2Game.cpp:6174-6205](X2Lib/X2Game.cpp#L6174) — `RebirthUserUnit` already takes a bot branch |
| The characters | ~~`NUI_PVP_BOT_ELSWORD / _AISHA / _RENA / _RAVEN / _EVE / _CHUNG`~~ — **dead in this build, phase 0 proved it.** Use `NUI_CSM_PVP_HERO_*` ([X2UnitManager.h:1627-1636](X2Lib/X2UnitManager.h#L1627)); see *Phase 0* |
| Follow-and-fight AI | `CX2AllyNPCAI : CX2NPCAI` ([X2AllyNPCAI.h](X2Lib/X2AllyNPCAI.h)) — targeting, chase, dash/walk, random jump, Lua-tuned; selected by `CX2NPCAI::NAT_ALLY` at [X2GUNPC.cpp:1780](X2Lib/X2GUNPC.cpp#L1780) |

**Nothing in `KncWX2Server/Common/` has to change.** The offline mod stays
client-only, exactly as `MODS.md` claims for `SERV_IRUHADEV_OFFLINE`.

### The one thing that makes this safe

Offline mode survives only because the client becomes its own P2P peer over
loopback — `CX2StateDungeonGame` will not finish loading while
`CKTDNUDP::ConnectTestResult()` is false, and it reads the peer map that
`CX2Room::NetworkProcess()` fills from occupied slots (see the header comment in
[Handlers_Room.cpp:34-46](X2Lib/Offline/Handlers_Room.cpp#L34)). Three more
*ordinary* occupied slots would add three unreachable peers and hang the dungeon
at 80% forever.

Bot slots do not, because `NetworkProcess()` skips `m_bNpc`. This is the reason
to use the PvP-bot mechanism rather than fake players, and it is not negotiable.

### The other thing that makes this safe

`CX2Game::LiveActiveNPCNum()` — what `CC_KILL_ALL_ACTIVE_NPC` counts
([X2DungeonSubStage.cpp:770](X2Lib/X2DungeonSubStage.cpp#L770)) — only counts
NPCs whose team is `TN_MONSTER`. Bots spawned on `TN_RED` (the dungeon player
team, which is what `MakeRoomSlots` already sets) **do not block sub-stage
clear**. Spawn them on any other team and every room becomes uncompletable.

## The three gates that currently make it PvP-only

> **Corrected by phase 1, and this is the single most misleading thing the plan
> said.** Gates 1 and 2 below are real, but they are **not** the mechanism,
> because a dungeon never calls the function they live in. Being in `CX2Game`,
> the shared base class, is not the same as being on the dungeon's path:
> `CX2StateDungeonGame` routes `EGS_PLAY_START_NOT` to its own `PlayStartNot()`
> ([X2StateDungeonGame.cpp:1692](X2Lib/X2StateDungeonGame.cpp#L1692)), which
> calls `GameStart()` directly and never touches
> `CX2Game::Handler_EGS_PLAY_START_NOT`. The working hook is
> `CX2DungeonGame::SubStageStart()`. Both gates were left in as a guard rather
> than removed; see *Defect 1* under phase 1 for the whole story, and treat any
> other "it is in the base class, so the dungeon runs it" claim in this file as
> unverified. Gate 3 *is* reachable — it is inside `CX2Game::CreateNPC`, which
> the dungeon does reach.

All three are in `CX2Game`, the base class both `CX2DungeonGame` and
`CX2PVPGame` derive from. All three are one `#ifdef SERV_IRUHADEV_OFFLINE`
each, with the studio's original code kept reachable in the `#else` branch —
these are behaviour *replacements*, which is exactly the case `CLAUDE.md` asks
to be written that way.

1. **Conversion is gated on the PvP channel.**
   [X2Game.cpp:8044-8048](X2Lib/X2Game.cpp#L8044):
   `if( g_pMain->GetConnectedChannelID() == KPVPChannelInfo::PCC_OFFICIAL ) g_pX2Room->DeleteNpcSlot();`
   In a dungeon this is false, so bot slots never reach `m_vecNpcSlot`, never
   get spawned, and — worse — stay in `m_SlotDataList` as slots that
   `AddUserUnit` skips and nothing else claims.

2. **The spawn loop is built for a PvP arena.**
   [X2Game.cpp:8093-8117](X2Lib/X2Game.cpp#L8093) places bots at
   `pLineMap->GetBlueTeamStartPosition( index )` on `CX2Room::TN_BLUE` with
   `CX2NPCAI::NAT_NORMAL` — the enemy side of a versus map. A dungeon line map
   has no blue team start, and `TN_BLUE` would make the bots hostile *and*
   uncountable for stage clear.

3. **The HUD insert is gated on game type.**
   [X2Game.cpp:6437-6451](X2Lib/X2Game.cpp#L6437), inside `CX2Game::CreateNPC`:
   `else if ( GetGameType() == CX2Game::GT_PVP )` is the only branch that calls
   `InsertPvpMemberUI`. Dungeons take the keycode/start-state branch instead.

## What the offline server has to supply

`MakeRoomSlots` ([Handlers_Room.cpp:165](X2Lib/Offline/Handlers_Room.cpp#L165))
builds a one-element slot vector today, with `MAX_SLOT = 1` and a comment saying
the client is the only occupant. It becomes a 1..4-element vector where slots
1..3 are bots.

Four concrete requirements, each of which will silently do nothing if missed:

- **`m_bIsPvpNpc = true`** on the bot's `KRoomUserInfo`. This is the only signal.
- **`m_iOwnerUserUID = 0`** — a fifth requirement the plan missed, found in
  phase 1. `CX2Room::SlotData::Set_KRoomSlotInfo` compares that field against
  `g_pData->GetMyUser()->GetUID()` to choose between
  `Set_KRoomSlotInfoOfMine` and `..OfOthers`, and the "mine" branch runs
  against the player's own live `CX2Unit`.
- **A negative unit UID, at most -2.** `IsPvpBot()` tests `m_UnitUID < -1`
  ([X2GUNPC.cpp:2153](X2Lib/X2GUNPC.cpp#L2153)). `-2, -3, -4` is the obvious
  choice. A positive UID produces a bot that spawns, fights, and is invisible to
  every piece of bot-aware code in the client.
- **A filled `m_kGameStat`.** `DeleteNpcSlot` copies HP/atk/def straight off
  `m_pUnit->GetUnitData()->m_GameStat`, and `CX2GUNPC` then overrides its NPC
  stat table with those five numbers. `MakeGameStat` already exists and
  `CX2OfflineStatTable` already knows per-level base stats.
- **The NPC template id must be derived client-side.**
  `KEGS_PARTY_GAME_START_NOT` — the packet that actually moves the player into a
  dungeon ([Handlers_Room.cpp:704](X2Lib/Offline/Handlers_Room.cpp#L704),
  handled at [X2PartyManager.cpp:1397](X2Lib/X2PartyManager.cpp#L1397)) — has
  **no `m_mapPvpNpcInfo`**; only `EGS_GAME_START_PVP_MATCH_NOT` carries it. So
  `SlotData::m_iNpcId` stays 0 on the dungeon path.
  **Do not add a field to the packet.** Map `KRoomUserInfo::m_cUnitClass` to
  an NPC id in the widened spawn loop instead. The class is already on every
  slot, and this keeps the mod client-only. (Phase 0 changed *which* ids that
  table maps to — `NUI_CSM_PVP_HERO_*`, not `NUI_PVP_BOT_*` — but not the
  mechanism: slot ingest at [X2Room.cpp:2323-2331](X2Lib/X2Room.cpp#L2323)
  reads only UID, nickname and rating for a bot slot, so `m_cUnitClass` is
  free for this.)

Then `Handler_EGS_NPC_UNIT_CREATE_REQ`
([Handlers_Room.cpp:1613](X2Lib/Offline/Handlers_Room.cpp#L1613)) must hand a
bot spawn its **room-slot UID** rather than the next value of
`m_kRoom.m_iNextNpcUID`. Recognise it by `m_NPCID` being one of the bot-cast
ids, and pop the next unused negative UID from the room's bot list.
Get this wrong and the NPC spawns with a positive UID, `IsPvpBot()` is false,
the stat override does not fire, and the bot appears at NPC-table stats with no
HUD entry — a failure that looks like the AI being broken.

## Two designs, and why this is the one

**Rejected: fake player slots.** Three extra `KRoomUserInfo` with
`m_bIsPvpNpc = false`, producing three real `CX2GUUser` units driven by a new
input-synthesising AI. It gives genuinely full character movesets and a real
party HUD for free. It also adds three unreachable P2P peers (dungeon hangs at
80%), needs a from-scratch AI that drives `CX2GUUser` through combos and MP
skills — a system that does not exist anywhere in this client — and every
`GetUserUnit` consumer in `X2Lib` starts seeing units nobody is driving. Much
larger, and it fights the client instead of using it.

**Chosen: PvP bot slots.** Everything in the table above already works; the
change is three `#ifdef`s in `CX2Game` plus room-building in the offline server.
Its cost is honest and worth stating up front: a bot is a `CX2GUNPC`, so its
moveset is whatever its `.lua` gives it — a scripted approximation of that
character, not the player's own skill tree. That is what KOG shipped these NPCs
to be. Phase 0 sharpened this cost: the cast is not the six playable-character
bots but the game's *named NPC heroes*, so a party reads as Lowe, Lime and Edan
fighting alongside you rather than as a second Elsword.

---

## Running a phase in a fresh conversation

Each phase is meant to be one conversation. `CLAUDE.md` and the memory index
load automatically, and the memory already points here, so the prompt only has
to name the phase and forbid re-exploration:

```
Read AI_PARTY_PLAN.md and do Phase 1.

The plan's evidence table has the file:line refs already - trust it and verify
by reading those specific lines, don't re-explore the codebase from scratch.
Build and deploy per the plan's Verification section when you're done, then
stop so I can play-test.
```

Substitute the phase number. Three things to hold to across all of them:

1. **Phases are ordered and phase 0 is a gate.** Do not start phase 1 before
   phase 0 has run *in the game* and its result is written into this file. Which
   of the six `NUI_PVP_BOT_*` exist decides what phase 1 and 2 can do, and it is
   not knowable from the repo.
2. **Write the outcome back into this file at the end of every phase**, in
   `OFFLINE_MODE_PLAN.md`'s shape: an `### Exit test — PASSED/PARTIAL (date)`
   table, a *Corrections to this plan, found by doing it* section, and
   *Decisions made while implementing phase N*. The next conversation reads that
   and nothing else of the previous one. A phase that changed the plan's
   assumptions and did not say so costs the following phase more than it saved.
3. **Only the play-test closes a phase.** There is no test suite; a phase ends
   when the exit test has been run in the client and the result recorded, not
   when it compiles. Expect to hand the build back and forth — the model builds
   and deploys, you play, you paste what happened.

If a conversation runs long mid-phase, the cheapest resume is a fresh one with:
*"Read AI_PARTY_PLAN.md. Phase N is in progress — `git diff` shows what's done.
Continue from there."*

---

# Phase 0 — Probe: do the bot NPCs exist in this client's data?

**This is a gate, not a formality.** Everything downstream assumes
`NUI_PVP_BOT_ELSWORD` and friends resolve to a real NPC templet with a mesh, a
Lua state machine and a stat row. Those come from Lua packed in the `.kom`
archives (`CX2UnitManager::AddNPCTemplet_LUA`,
[X2UnitManager.cpp:636](X2Lib/X2UnitManager.cpp#L636)), which is data this repo
does not contain. The enum existing proves nothing — see the
`client-enums-are-half-commented-out` memory for the general form of this trap,
and `serverresource-lua-newer-than-tree` for the version-skew form.

Do:

1. Build a diagnostic under `SERV_IRUHADEV_OFFLINE` like everything else: after
   unit-manager load, call `GetNPCUnitInfo()` for all six ids and log id, name,
   `m_LuaFileName`, and whether an NPC stat row exists, via
   `CX2OfflineLog::Server( L"AIPARTY  ..." )`.
   `CLAUDE.md` suggests a separate short-lived `*_DEBUG` flag for temporary
   diagnostics; deliberately not doing that here — one flag for the whole
   feature. What makes it removable instead is that it is **one contiguous block
   in one function**, tagged `AIPARTY`, and phase 5 deletes it. Keep it that
   way: do not scatter probe logging across files, or there will be nothing to
   grep for when it is time to take it out.
2. Run one dungeon, `grep AIPARTY offline_server.log`.

Exit test: all six resolve, **or** a known subset does. Record which.

If none resolve, stop and re-plan around one of these before writing any more
code:

- `NUI_CSM_PVP_HERO_*` ([X2UnitManager.h:1627-1636](X2Lib/X2UnitManager.h#L1627))
  — the card-summoned PvP hero NPCs, the "summon an NPC character" consumable
  the idea started from. Same `RoomNpcSlot` mechanism, different cast.
- `NUI_PVP_HERO_LOW / _PENENSIO / _NOA / _SPIKA / _LIME / _AMELIA / _EDAN /
  _BALAK` ([X2UnitManager.h:1264-1274](X2Lib/X2UnitManager.h#L1264)).
- The ally-NPC-by-monster path, `CX2GUNPC::CreateAllyNpcByMonster_LUA`
  ([X2GUNPC.cpp:25628](X2Lib/X2GUNPC.cpp#L25628)), which already spawns a
  follow-and-fight ally through `CreateNPCReq( ..., NAT_ALLY, ownerUID, ... )`.
  This one is proven live in this build (`CREATE_ALLY_NPC_BY_MONSTER`,
  `Always.h:1161`) but produces a *monster* ally, not a party member — the
  fallback of last resort.

### Exit test — FAILED for the intended cast, re-planned (2026-09-05)

Probe built at the end of `CX2Data::ResetUnitManager`
([X2Data.cpp:2007](X2Lib/X2Data.cpp#L2007)), one contiguous `AIPARTY`-tagged
block. It runs at client start-up, from `CX2StateStartUp`, so **no dungeon is
needed** — reaching the login screen is enough. It probes 27 ids: the six
intended bots plus both fallback casts, so one run answered the gate *and* the
re-plan question.

| Cast | Result |
|---|---|
| `NUI_PVP_BOT_*` (6) | **0/6 usable.** Templet row and stat row both present; the `.lua` loads to nothing — `skinMesh=0 states=0` on all six |
| `NUI_PVP_HERO_*` + `NUI_PVP_RUNE_GUARD` (11) | 11/11 usable |
| `NUI_CSM_PVP_HERO_*` (10) | 10/10 usable |

The six bots are **orphan rows**: `NPCTemplet.lua` and `NPCStat.lua` still carry
them (`PVP_BOT_ELSWORD.lua`, hp 30800, atkP 474, ...) but the state-machine
`.lua` they name is not in this client's archives. Two independent signs it is
data rot rather than a probe artefact:

- All 21 other ids came back fully populated through the *same* call in the
  *same* run — `GetNPCUnitInfo()`, the call the `CX2GUNPC` creation path itself
  makes. The loader is fine; these six files are not there.
- Their templet `NAME` string ids resolve to the wrong strings. All six read as
  Gliter Alchemist event NPCs ("new / suspicious / supporter", cycling) — the
  three enum entries immediately *preceding* `NUI_PVP_BOT_ELSWORD`. Whatever
  removed the bots left the rows behind and the string table skewed past them.

Not settled, and deliberately not chased: whether the `.lua` is absent from the
`.kom` or present-but-failing. `LoadLuaManager_ErrorCode()` distinguishes the
two (1 = load failure, 2 = execution failure) and the probe could log it, but it
costs a build/play-test cycle and changes nothing — the files are in neither the
archives nor this repo, so there is no path to this cast either way. A raw byte
scan of the `.kom` set was tried and is **not** evidence: `CSM_PVP_HERO_LOW`
loads perfectly and its name appears in no archive's plaintext, so absence of
the string proves nothing.

### Decisions made while implementing phase 0

- **The cast becomes `NUI_CSM_PVP_HERO_*`** — Lowe, Penensio, Noah, Speka, Lime,
  Amelia, Edan, Valak, Code:Q-Proto_00, Apple. Chosen over the otherwise
  equivalent `NUI_PVP_HERO_*` because the CSM set is the *card-summoned* one:
  those NPCs were shipped to be summoned onto the player's side, which is
  exactly the job here. Ten is more than the three slots need.
- **`NUI_PVP_RUNE_GUARD` is excluded.** It loads, but `states=8` and
  `defP=defM=0` — it is a stationary guard object, not a fighter. (Its name
  string is also wrong, reading "Apple".)
- **The probe was left in** rather than deleted now. Phase 5 removes it; until
  then it is the standing check that this cast still resolves, and it is one
  `grep AIPARTY` away from proving a future data change broke something.
- **The probe covers the fallbacks too**, not just the six ids the gate was
  about. That is what made this one play-test instead of two.

### Corrections to this plan, found by doing it

1. **"Run one dungeon" was wrong** — the probe sits in start-up data loading and
   fires before login. Cheaper than the plan assumed.
2. **"The enum existing proves nothing" was right, and stronger than written.**
   The failure mode here was not a commented-out enum or a missing templet: the
   templet *and* a full stat row are both present and look completely healthy.
   Only the state machine is missing. Anything that had checked for a templet
   row — the obvious check — would have passed, and phase 1 would have shipped a
   bot that spawns as nothing.
3. **Phase 2 step 1 no longer means what it says.** "Classes chosen to
   complement the player's, never a duplicate of the player's class" assumed the
   bots *were* the six playable characters. The heroes have no `UNIT_CLASS`, so
   the rule becomes: pick three distinct heroes, and there is nothing to
   complement. Their captions come from the slot nickname while their model
   comes from the client-side id mapping, so keep those two tables in step or a
   bot will be named as someone it is not.
4. **Phase 5's stat scaling matters more than the plan assumed.** These heroes
   carry boss-scale rows — hp 159k-285k, atk 1.3k-2.4k, against a low-level
   player's few thousand. The `RoomNpcSlot` override
   ([X2GUNPC.cpp:3915-3921](X2Lib/X2GUNPC.cpp#L3915)) is what makes them usable
   at all, so it is load-bearing rather than a tuning nicety: if it does not
   fire, a "party member" arrives with 250k HP. That is the same failure the
   plan's table already predicts for a positive NPC UID — it will just be far
   more obvious than "immortal-looking" suggests.
5. **`m_cUnitClass` is confirmed free for a bot slot.** Ingest at
   [X2Room.cpp:2323-2331](X2Lib/X2Room.cpp#L2323) reads only `m_nUnitUID`,
   `m_wstrNickName` and `m_iRating` when `m_bIsPvpNpc` is set, so phase 1 step 6
   can carry the hero selector there as planned — only the mapping's target
   changes.

---

# Phase 1 — One bot, through auto-party

> **Done and play-tested 2026-09-05 (exit test below).** The six numbered
> steps are the plan **as written beforehand** and three of them did not
> survive contact: step 3's defaulted bot count, and steps 5 and 6, which
> name a handler the dungeon never calls. They are kept unedited so the
> corrections underneath have something to correct. **To read what the code
> actually does, skip to *Three defects found by play-testing phase 1* and
> *Decisions made while implementing phase 1*.**

Smallest thing that proves the pipeline. Because bots must never appear on the
solo button, this phase has to reach the dungeon through auto-party rather than
quick-start — so it builds the *shortest legal* auto-party path and leaves the
matchmaking ceremony (queue, popup, cancel) to phase 3.

**Do not test this by temporarily adding a bot to the quick-start handler.** It
is the obvious shortcut and it is how the solo path ends up shipping with a
companion nobody asked for: the temporary edit compiles, works, and looks
exactly like the permanent one in a diff two weeks later.

1. **No flag work.** `SERV_IRUHADEV_OFFLINE` is already defined on both sides;
   this phase adds nothing to `Always.h`. If you edit any header anyway (step 6
   adds a field to `X2Room.h`, which is not in the PCH, but `X2Define.h` in
   phase 5 is), `touch X2Lib/stdafx.cpp` before building —
   `pch-hides-header-edits`.
2. **`Handlers_Social.cpp`** — replace the `ERR_PARTY_23` refusal in
   `Handler_EGS_AUTO_PARTY_DUNGEON_GAME_REQ` with the solo handler's own shape.
   The request is a typedef of the quick-start one, so it is the same four
   fields: build the same `KRoomInfo` seed, call the same `OpenRoom( ...,
   RT_DUNGEON, ... )`, set `RS_LOADING` and `m_dwPlayStartTick` the same way,
   and send `EGS_AUTO_PARTY_DUNGEON_GAME_ACK` with `NET_OK` followed by
   `EGS_PARTY_GAME_START_NOT` — but with **one** bot slot.
   Factor the shared middle out of
   `Handler_EGS_QUICK_START_DUNGEON_GAME_REQ` rather than copying it, so the two
   buttons cannot drift; the only difference between them should be the bot
   count and which ACK goes out. Read the solo handler's comments first — the
   room-state and `SS_LOADING` notes there are load-bearing and apply verbatim.
3. **`Handlers_Room.cpp`** — `MakeRoomSlots` grows an optional bot count.
   `MAX_SLOT` in the anonymous namespace goes from 1 to 4, and `MakeRoomInfo`'s
   `m_JoinSlot` follows the real occupancy. Add `MakeBotRoomUserInfo( class,
   level, nickname, botUID, OUT KRoomUserInfo& )` next to `MakeRoomUserInfo`:
   `m_bIsPvpNpc = true`, `m_nUnitUID = botUID` (at most -2), `m_cUnitClass`,
   `m_ucLevel`, `m_wstrNickName`, and a `MakeGameStat`-derived `m_kGameStat`. No
   equipped items, no skill data, no IP/port — a bot has no P2P identity and
   `Set_KRoomSlotInfo` skips `ResetEqip()` for it anyway.
   Record the bot UIDs on `m_kRoom` for the create-req handler.
   **The bot count parameter defaults to 0**, which is what keeps the solo call
   site correct without editing it.
4. **`Handlers_Room.cpp`** — `Handler_EGS_NPC_UNIT_CREATE_REQ` returns the
   room's bot UID when `m_NPCID` is a bot-cast id, and does **not** write
   `m_mapNpcLevel` / `m_mapNpcID` for it (those price kill rewards).
5. **`X2Game.cpp` gate 1** — widen the `DeleteNpcSlot()` condition to also fire
   when `GetGameType() == GT_DUNGEON`, keeping the original `PCC_OFFICIAL`
   branch in `#else`.
6. **`X2Game.cpp` gate 2** — in the spawn loop, when `GT_DUNGEON`: derive the
   NPC id from the slot's unit class, spawn on `CX2Room::TN_RED` with
   `CX2NPCAI::NAT_ALLY` and `m_iAllyUID = GetMyUnit()->GetUnitUID()`, and take
   the start position from the player's own start position offset sideways, not
   from `GetBlueTeamStartPosition`. Note `DeleteNpcSlot` has already run by the
   time this loop executes, so `m_cUnitClass` must be captured into
   `RoomNpcSlot` first — add a field to that struct (a client-side struct, not a
   packet, so this is free).

Exit test:

- Press **auto-party** on a normal dungeon. One AI character spawns beside you
  on your team, follows you, and attacks monsters.
- **Press the normal start button on the same dungeon. Nobody spawns.** This is
  the regression check for the whole feature and it belongs in every phase's
  exit test from here on — it is one run and it is the thing most likely to be
  broken by a careless refactor of the shared middle.
- Sub-stages still clear on both paths. If they do not on the auto-party path,
  the bot is not on `TN_RED`.
- `offline_packets.log` has no new `UNHANDLED` and no `EXCEPTION`.
- Both dungeons still finish loading — no 80% hang. If one hangs, a bot slot is
  reaching `NetworkProcess()` without `m_bNpc` set.

### Exit test — PASSED (2026-09-05)

Confirmed in play after three build/play cycles. Evidence, from the run at
22:58–23:04 in `offline_server.log`:

| Check | Result |
|---|---|
| Auto-party fills a slot | `ROOM dungeon room 1001 … 1 bot(s)`, `AIPARTY bot slot 1: uid=-2 class=1 level=50 name="Lowe"` |
| The bot spawns, on your team, and follows | on screen; `AIPARTY spawning bot "Lowe" npcID=1102 level=50 slotUID=-2 … ally of 12` |
| **Its hits damage monsters** | on screen, after defect 2 below |
| **No entrance animation on stage change** | on screen, after defect 3 below |
| Sub-stages clear, the dungeon finishes | 6 stage starts, then `GAME all NPCs dead - dungeon clear broadcast sent` and a result screen (score=27131, 12 items) |
| **The normal start button spawns nobody** | `ROOM dungeon room 1002 … 0 bot(s)`, and nobody on screen |
| No new failures | `grep -cE "UNHANDLED\|EXCEPTION" offline_packets.log` → 0 |
| No 80% hang on either button | both rooms loaded into the dungeon |

A second dungeon (`dungeonID=38100 dif=0 mode=2`) also spawned its bot, so the
path is not specific to the one map it was developed against.

The bot spawned four times across that six-stage run, which is the per-**stage**
rebuild described under defect 3 — not once per sub-stage, and not once per run.

**One thing to know before grepping these logs again:** the two `AIPARTY` spawn
lines now appear in the opposite order to what you would expect —
`bot spawn "…" given slot uid=-2` (the offline server) prints *before*
`spawning bot "…"` (the client). Since defect 2's fix, `CreateNPCReq` sends its
packet immediately rather than batching into a later flush, and the emulator
answers it synchronously inside the same call. Not a fault; it just means the
server line is not evidence that the client line was skipped.

### Three defects found by play-testing phase 1, and what each taught

Written 2026-09-05, after three build/play cycles. Each is worth keeping because
each was invisible to code-reading and each has a general form.

#### Defect 1 — nothing spawned at all: the plan named a handler the dungeon never calls

The plan's evidence table put the spawn in `CX2Game::Handler_EGS_PLAY_START_NOT`
and stressed that it is the **base class**, not `CX2PVPGame`. That is true and it
is beside the point: **a dungeon never reaches that handler.**
`CX2StateDungeonGame` routes `EGS_PLAY_START_NOT` to its own `PlayStartNot()`
([X2StateDungeonGame.cpp:1692](X2Lib/X2StateDungeonGame.cpp#L1692)), which calls
`m_pDungeonGame->GameStart()` directly. Both gates sat in dead code: no
`DeleteNpcSlot()`, so `m_vecNpcSlot` stayed empty, so `CreateOfflinePartyBots()`
returned before it could even log a failure.

Even reachable it would have been too early — at play start the first sub-stage
has not loaded, so there is no placed player to spawn beside.

**The hook is `CX2DungeonGame::SubStageStart()`**
([X2DungeonGame.cpp:1258](X2Lib/X2DungeonGame.cpp#L1258)), immediately after
`CreateAllyEventMonster()` — the studio's own "fill this party out to four with
`NAT_ALLY` NPCs at `pUser->GetPos()`" feature. When the tree already contains the
feature you are building, follow *its* call site, not the one that merely
contains the code you want to reuse.

Consequences, both now in the code:

- `CreateOfflinePartyBots()` runs **once per sub-stage** and so must be
  idempotent. It is: a bot whose NPC is already in the world is skipped, and it
  calls `DeleteNpcSlot()` itself (also idempotent) rather than relying on the
  dead gate.
- The offline server matches a bot spawn by **hero id**, not by "next unclaimed
  slot". A bot really is re-created — once per **stage**, because a stage change
  tears the world down — so a one-shot claim flag would have handed the second
  spawn an ordinary monster UID and `IsPvpBot()` would have stopped seeing it.
  This is why every bot must be a *distinct* hero.

The two `Handler_EGS_PLAY_START_NOT` gates were left in place as a guard rather
than a mechanism: if that handler is ever reached in a dungeon it now takes the
dungeon branch instead of spawning bots on `TN_BLUE`.

#### Defect 2 — the bot attacked monsters and did no damage: a packet field's default

Lowe chased monsters and swung at them, and they took nothing. Not the AI, not
the stats, not the negative UID — **one field of `KNPCUnitReq` that
`PushCreateNPCReq` does not set.**

`KNPCUnitReq::Init()` defaults `m_cAllyTeam = 2`, i.e. `CX2Room::TN_MONSTER`
([CommonPacket.h:3633](KncWX2Server/Common/CommonPacket.h#L3633)).
`CX2Game::PushCreateNPCReq` sets `m_cTeamNum`, `m_cAIType` and `m_iAllyUID` but
never `m_cAllyTeam`, so the default survived to the client. And
`CX2DamageManager` skips any hit whose NPC attacker has an ally team equal to
the defender's team ([X2DamageManager.cpp:1347](X2Lib/X2DamageManager.cpp#L1347),
`SERV_TRAPPING_RANGER_TEST`):

```cpp
if( pAttacker->GetGameUnitType() == CX2GameUnit::GUT_NPC &&
    pAttacker->GetAllyTeam() == pDefender->GetTeam() )
    continue;
```

So every monster in the game counted as the bot's own side. The attack animation
plays either way, which is exactly what made it read as an AI bug.

**The fix is `CreateNPCReq` instead of `PushCreateNPCReq`.** It takes `eAllyTeam`
and defaults it to `TN_NONE`, which is why *both* of the studio's ally spawns use
it and pass `TN_NONE` explicitly — `CreateAllyEventMonster`
([X2DungeonGame.cpp:3531](X2Lib/X2DungeonGame.cpp#L3531)) and
`CreateAllyNpcByMonster_LUA` ([X2GUNPC.cpp:25680](X2Lib/X2GUNPC.cpp#L25680)).
One packet per bot rather than a batch; `CreateAllyEventMonster` does the same in
its own loop.

The general form is worth stating plainly: **`PushCreateNPCReq` is not
`CreateNPCReq` with batching.** It fills in fewer fields, and the ones it omits
keep packet defaults that were chosen for monsters. Reading its body is what
answers this; reading its signature is not, because the field is not a parameter
of it at all.

#### Defect 3 — the bot played an entrance animation at every stage change

Every NPC is put into its lua `START` state when it is built
([X2GUNPC.cpp:2621](X2Lib/X2GUNPC.cpp#L2621)), and for this cast that state is a
card-summon entrance. Because a stage change tears the world down, a bot is
rebuilt once per stage, so the entrance played each time — which reads as being
re-summoned rather than as a party member who was there all along.

The bot is now forced into `GetCommonState().m_Wait` at creation, in
`CX2Game::CreateNPC` right after `SetUserSummonedNPCInfo`, guarded on
`IsPvpBot()` and `GT_DUNGEON`. The ally AI takes over on its next tick. This is
the same intent the studio already applies one level up: `SubStageStart` forces
`GetStartState()` on every NPC **except** `NAT_ALLY` ones
([X2DungeonGame.cpp:1116](X2Lib/X2DungeonGame.cpp#L1116)) — an exemption that
does not extend to creation, which is the gap this fills.

Note what the logs proved along the way, since it constrains phase 2: a bot
persists across **sub-stages** (`DeleteAllNPCUnit` only removes the sub-stage's
own NPC list) but not across **stages**. The `AIPARTY spawning bot` lines came
once per stage, not once per sub-stage, against a stage 0 of two sub-stages and a
stage 1 of three.

### Decisions made while implementing phase 1

- **The bot count lives on the room, not on `MakeRoomSlots`.** The plan asked
  for an optional bot-count parameter defaulting to zero. That cannot work, and
  finding out why is the most useful thing this phase learned: **the slot list
  is built three times for one run, and only the first of those knows which
  button was pressed.** `EGS_PARTY_GAME_START_NOT` comes from the entry handler,
  but `EGS_STATE_CHANGE_GAME_START_NOT` and — the one that actually matters —
  `EGS_PLAY_START_NOT` are both sent from `Handler_EGS_GAME_LOADING_REQ`, which
  is shared by both buttons and has no idea. `EGS_PLAY_START_NOT` is the packet
  `CX2Game::Handler_EGS_PLAY_START_NOT` turns into `m_vecNpcSlot`, so a
  defaulted argument would have been correct at the call site that does not
  matter and unanswerable at the one that does. `KOfflineRoom::m_vecBot` gives
  the same guarantee the default was for — `OpenRoom` clears it, only
  `MakePartyBots` fills it, and only the auto-party handler calls that — while
  making every call site right without any of them having to ask.
- **`m_cUnitClass` carries a real `UNIT_CLASS`, not a bare index.** A bot slot
  still constructs a `CX2Unit` (`Set_KRoomSlotInfoOfOthers` →
  `new CX2Unit( kRoomUserInfo )` → `Init()`, which looks the class up in the
  unit templet table), so an out-of-range value is a needless risk for nothing.
  Phase 1's one bot is `UC_ELSWORD_SWORDMAN` → `NUI_CSM_PVP_HERO_LOW` ("Lowe").
  The class is never seen: the model is the hero NPC's and the caption is the
  slot nickname.
- **The two tables are named after each other.** `BOT_CAST` in
  `Handlers_Room.cpp` decides a bot's *name*; the `switch` in
  `CX2Game::CreateOfflinePartyBots` decides its *model*; the unit class is the
  only thing joining them. Both carry a comment naming the other, and
  `Handlers_Room.cpp` has a compile-time check that `BOT_CAST` and `BOT_NPC_ID`
  are the same length. The server needs `BOT_NPC_ID` for one thing only:
  recognising a bot spawn in `EGS_NPC_UNIT_CREATE_REQ`.
- **The shared middle is two functions, not one.** `OpenDungeonGameRoom` (build
  the `KRoomInfo` seed, open the room) and `SendDungeonGameStartNot` (advance to
  `RS_LOADING`, start the clock, `ClearPlayRun`, send
  `EGS_PARTY_GAME_START_NOT`). Splitting there is what lets each handler send
  its own ACK in between, in the same order the solo handler always did. The
  only line that differs between the two buttons is `MakePartyBots`.
- **Gate 1's `else` branch was left where it was.** The plan's "widen the
  condition, keep the original in `#else`" would have moved dungeons out of the
  `SetCanUseEscFlag( true )` branch as a side effect — that branch is what every
  non-PvP-channel game, dungeons included, has always taken. The `#ifdef` now
  carries two separate conditions rather than one if/else, so ESC handling on
  the solo path is byte-identical to before.
- **Gate 2 is a new method, not an edited loop.** `CreateOfflinePartyBots()`
  sits beside `PushCreateNPCReq` in `X2Game.cpp`; the studio's PvP arena loop is
  untouched behind an `else`. A PvP match still takes it.
- **Bots are spawned behind the player, 60 units apart**, from
  `GetMyUnit()->GetPos()` rather than from the line map. A dungeon line map has
  no team start positions to read, and the player is standing on valid ground by
  definition at play start. Phase 2's fan-out along the start line supersedes
  this.

### Corrections to this plan, found by doing it

1. **`MakeRoomSlots`'s "optional bot count defaulting to 0" is wrong** — see the
   first decision above. The scope guard that keeps the solo button solo is
   `m_vecBot` being empty, not an argument default.
2. **`NAT_ALLY` + `iAllyUID` means the client re-stats the bot from the player,
   and phase 5 needs to know.** `CX2Game::CreateNPC` calls
   `SetUserSummonedNPCInfo` for any ally with an owner
   ([X2Game.cpp:6483](X2Lib/X2Game.cpp#L6483)), and its `default:` branch does
   `pNPC->SetNPCStat( ... )` from **the player's own** HP/atk/def
   ([X2Game.cpp:12900](X2Lib/X2Game.cpp#L12900)) — after the `RoomNpcSlot`
   override at [X2GUNPC.cpp:3915](X2Lib/X2GUNPC.cpp#L3915) has already run. So
   the last word on a bot's stats is the player's stat line, not the slot's.
   Two consequences: the 160k–285k HP hero rows cannot reach the field even if
   the slot override failed (belt and braces, which is welcome), and **phase 5's
   "scale the five `RoomNpcSlot` stats" knob will do nothing on this path**. The
   knob phase 5 actually has is `SetHardLevel` / the `unitID` switch in
   `SetUserSummonedNPCInfo`, or dropping `iAllyUID` and losing the follow
   behaviour. Decide that in phase 5 with a play-test, not from the code.
3. **The bot's `m_kGameStat` is base-table only, with no gear.**
   `MakeGameStat( const KOfflineUnitRow& )` loads *that row's* inventory, which a
   bot does not have; calling it with the player's row would have given the bot
   the player's equipment bonuses. `MakeBotRoomUserInfo` calls
   `CX2OfflineStatTable::GetUnitStat` directly instead.
4. **`m_iOwnerUserUID` must stay 0 on a bot slot.** Not in the plan's list of
   four requirements, and it belongs there:
   `CX2Room::SlotData::Set_KRoomSlotInfo` picks between
   `Set_KRoomSlotInfoOfMine` and `..OfOthers` by comparing that field against
   `g_pData->GetMyUser()->GetUID()`, and the "mine" branch runs against the
   player's own live `CX2Unit`.
5. **A cast id that spawns with no bot slot left is treated as an ordinary
   NPC**, with a log line. The same heroes are summonable by monster card, so
   "this NPC id means bot" is not true on its own — only "this NPC id *and* an
   unclaimed bot slot" is.

---

# Phase 2 — Three bots, placement, death and revive

> **Implemented and closed 2026-09-06.** Defect 2 is fixed, and the per-stage
> respawn it lived on is gone entirely - the party now survives a stage change.
> Two items are still unexercised rather than unbuilt: a bot death and the
> EXP/ED comparison. See *The party now survives a stage change* below. The
> five numbered steps below are the plan **as written beforehand**, and steps 2,
> 4 and 5 did not survive contact - one precedent it names does not exist, one
> check turned out to be a build, and one needed no code at all. They are kept
> unedited so the sections underneath have something to correct. **To read what
> the code actually does, skip to *Two defects found by play-testing phase 2*,
> *What reading settled before the play-test* and *Decisions made while
> implementing phase 2*.** In particular the cast is now all ten heroes drawn at
> random, not the three fixed rows the preamble describes.

**What phase 1 already did, so phase 2 does not redo it.** Step 3 is done — a
bot is built at the player's level and its `m_kGameStat` comes from
`CX2OfflineStatTable` at that level. Step 5's prevention is in and is stronger
than the plan's wording: the offline server writes no `m_mapNpcLevel` /
`m_mapNpcID` row for a bot at all, so `Handler_EGS_NPC_UNIT_DIE_REQ` finds
nothing to price. It still wants the before/after `els_db.sql` check.

**Raising the count to three is one constant plus two table rows.**
`AUTO_PARTY_BOT_NUM` in `X2OfflineServer.h` goes from 1 to `MAX_BOT`; `BOT_CAST`
in `Handlers_Room.cpp` already carries three entries (Lowe, Lime, Edan) and the
`switch` in `CX2Game::CreateOfflinePartyBots` already maps all three classes.
Adding a fourth means a row in **both**, and the compile-time check next to
`BOT_CAST` only catches a length mismatch against `BOT_NPC_ID`, not a class the
client `switch` has no case for — that one logs `maps to no hero` at runtime.

**Every bot must stay a distinct hero.** The offline server matches a spawn back
to its slot by NPC id (`FindPartyBotByNpcID`), because a bot is genuinely
re-created once per stage and a one-shot claim would break the second spawn. Two
bots sharing a hero would collide on that lookup.

1. ~~Three bots, classes chosen to complement the player's.~~ Superseded twice
   over: the cast is NPC heroes with no `UNIT_CLASS` (phase 0), and the unit
   class on a bot slot is only a selector for the client's id mapping (phase 1).
   Pick three distinct heroes and keep `BOT_CAST` and the client `switch` in
   step.
2. Spawn placement: fan out along the player's start line via
   `CKTDGLineMap::GetLineData`, the way the field spawner in
   `X2OfflineBattleField.cpp` already picks positions. Phase 1 spawns them
   behind the player at 60-unit intervals off `GetMyUnit()->GetPos()`, which is
   fine for one and will read as a queue for three.
3. ~~Level: match the player's level.~~ Done in phase 1.
4. Death and revive. `CX2Game::RebirthUserUnit` already takes the bot branch
   ([X2Game.cpp:6174](X2Lib/X2Game.cpp#L6174)); confirm what sends it in a
   dungeon and whether `CX2Game::IsAllUserDead()`
   ([X2Game.cpp:6108](X2Lib/X2Game.cpp#L6108)) counts bots — if it does, a wiped
   party of bots would end the run while the player is alive.
   **Do not take "it is in `CX2Game`, the base class" as proof the dungeon runs
   it.** That is exactly the reasoning that produced defect 1: the dungeon has
   its own state class and its own `CX2DungeonGame`, and it routes around base
   handlers more often than the class hierarchy suggests. Find the caller and
   check it is on the dungeon path before building on it.
   Worth knowing already: a bot survives sub-stage changes but is destroyed and
   re-created at every **stage** change, so "died and came back" and "was
   rebuilt by the next stage" look identical from the outside. Distinguish them
   in the log before concluding revive works.
5. **Verify the reward path is not polluted.** A dying bot must not produce
   EXP/ED through `Handler_EGS_NPC_UNIT_DIE_REQ`
   ([Handlers_Room.cpp:1692](X2Lib/Offline/Handlers_Room.cpp#L1692)). Confirm
   with a run: `sqlite3 els_db.sql "select unit_uid, exp, ed from unit;"` before
   and after.

Exit test: three bots fight a full dungeon start to finish, die and come back,
and EXP/ED for the auto-party run matches a solo run of the same dungeon. The
solo button still spawns nobody.

### Exit test - PARTIAL, ONE DEFECT OPEN (2026-09-06)

Two build/play cycles so far. The cast, the count and the draw are right; the
spawn *plumbing* is not finished, and phase 2 is **not closed**.

| Check | Result |
|---|---|
| Three bots, three different heroes, redrawn per press | **PASS.** `AIPARTY bot slot 1..3` names a different trio on each auto-party press - `Valak/Noah/Lowe`, then `Apple/Lowe/Valak`, then `Noah/Valak/Code: Q-Proto_00` |
| They fan out rather than queue up | **PASS**, and now superseded: placement comes from the line map's party start slots rather than an offset off the player |
| No stutter at a stage change | **PASS**, and by a different route than this row assumed. The stagger is still there but a stage change no longer spawns anything at all - see *The party now survives a stage change* |
| Sub-stages clear, dungeon finishes | **PASS** - the 05:29 run reached `SHUTDOWN clean` through a full dungeon |
| Six bots instead of three | **WAS FAILING, now fixed** - defect 1 |
| **Bots present at stage start** | **PASS** (2026-09-06). Defect 2 is closed twice over: the stale-position fix, and then the removal of the per-stage spawn it lived on |
| A dead bot comes back | **UNPROVEN, and assumed working by decision.** No `was down` line in any run - the mobs at this level cannot kill a bot carrying the player's stat line, so it could not be provoked. See *Revive is unproven and coupled to defect 2* below before relying on it |
| **The normal start button spawns nobody** | **PASS** (2026-09-06). The standing regression check, re-run after the count went to three |
| Reward path is clean | **NOT YET RUN.** Still wants the before/after `els_db.sql` comparison |
| No new failures | **PASS for this feature.** One `UNHANDLED`, `EGS_SECRET_STAGE_LOAD_REQ` (id=1197), unrelated to the AI party - it is a dungeon feature offline mode has never handled |

### Picking phase 2 up in a new conversation (as of 2026-09-06)

> **Superseded.** Both changes this section says to start by testing were tested
> and passed, and the per-stage spawn it is written around no longer exists.
> Kept for the reasoning; for the current shape read *The party now survives a
> stage change* below.

**Everything below the line is committed and the exe in the game directory is
current** (`X2_offline.exe`, 14,336,000 bytes, 06:14). Two changes are in that
build and have **not been play-tested**: defect 2's fix (spawn at line-map start
slots) and the staggered one-at-a-time spawn. Start there.

Open the next conversation with:

```
Read AI_PARTY_PLAN.md, phase 2. The last build fixed defect 2 and made the
party spawn one bot at a time; neither is play-tested yet. <what you saw>
```

What one run answers, in order of importance:

1. **Are you alone at a stage change?** You should not be. If you still are,
   defect 2's fix did not take and the next step is a log line in
   `CX2Game::CreateNPC` for an `IsPvpBot()` unit, printing the UID it was built
   with - not more reading.
2. **`grep "never arrived" offline_server.log` should be empty.** Anything there
   means a spawn is still being dropped.
3. **`grep "NOTE line map has" offline_server.log` should be empty too.** If it
   fires, that map has fewer than two start slots and the bots fell back to the
   player-offset placement, which is the path the old bug lived on.
4. Is the stage-change stutter gone, and do the three arrive in sequence?

Still not run at all, from the original exit test: **a bot death** (nothing at
this level hits hard enough - see *Revive is still unproven*), and the
**EXP/ED comparison** between an auto-party run and a solo run
(`sqlite3 els_db.sql "select unit_uid, exp, ed from unit;"` before and after).
Both are play-test work, not code work.

Known-good and not worth re-checking: the random draw, three distinct heroes per
press, the solo button spawning nobody, and a full dungeon completing.

### Two defects found by play-testing phase 2

#### Defect 1 - six bots in the room: a spawn is not synchronous, and the idempotence check assumed it was

Three heroes, two of each. The cause was not the random draw and not the
raised count; it was the per-frame `TickOfflinePartyBots` this phase added,
meeting an assumption phase 1 had got away with.

`CreateOfflinePartyBots` decided "already spawned" with
`NULL != GetNPCUnitByUID( slotUID )`. **That is not a test for "already
asked for".** `CreateNPCReq` only sends `EGS_NPC_UNIT_CREATE_REQ`; the unit is
built later, in `Handler_EGS_NPC_UNIT_CREATE_NOT`, off the broadcast. The
offline server answers inside the same call - which is why its
`bot spawn ... given slot uid` line prints *before* the client's
`spawning bot` line, as phase 1 already noted - but it answers by **queueing**
the NOT onto the session, so the NPC does not exist until the client next pumps
its packets. Every call landing in that window sees NULL and asks again.

The log is unambiguous: four identical batches of three, 10 ms apart.

```
[05:16:17.988] AIPARTY  3 bot spawn request(s) sent
[05:16:17.998] AIPARTY  3 bot spawn request(s) sent
[05:16:18.008] AIPARTY  3 bot spawn request(s) sent
[05:16:18.036] AIPARTY  3 bot spawn request(s) sent
```

Phase 1 never hit it because nothing called the function per frame; once per
sub-stage, the window had long closed.

The fix is a second half to the idempotence - a per-slot grace timer, armed
when a request goes out, counted down by the tick, dropped the moment the NPC
turns up, and retried with a log line if it lapses. **It lives in
`CreateOfflinePartyBots`, not in the tick**, so the next caller gets it too;
putting it in the caller would have fixed this instance and left the trap.

The general form, and it is worth stating because it will recur:
**"the object is not there" and "I have not asked for it" are different
questions, and in this client every spawn, every item grant and every state
change goes through a packet round-trip that makes them different for several
frames.**

#### Defect 2 - the first spawn of every stage was silently thrown away

**Root cause found and fixed 2026-09-06. The fix is built and deployed but NOT
play-tested.**

The symptom: from stage 1 onwards, three spawn requests went out at
`SubStageStart`, the offline server acknowledged and broadcast them, and no bot
appeared. A few seconds later the retry fired and they did. Confirmed in play as
"I always solo for a mere seconds at a stage change, and then the bots appear",
which is also what **ruled out the dangerous reading**: under the wrong-UID
explanation those first three would have existed and been fighting, so the
player would never have been alone. Three bots, not six.

The cause is a silent early return in `CX2Game::CreateNPC`
([X2Game.cpp:6405](X2Lib/X2Game.cpp#L6405)):

```cpp
pNPC->Init();
if( false == pNPC->SetPosition( vPos, bRight ) )
{
    ASSERT( !"CreateNPC SetPosition error" );   // compiled out in release
    SAFE_DELETE_KTDGOBJECT( pNPC );
    return;                                     // no log, no trace
}
```

**If the requested position has no ground under it, the NPC is built and then
thrown away.** And `CreateOfflinePartyBots` was deriving its positions by
offsetting from `GetMyUnit()->GetPos()` at `SubStageStart` — where **the player
has not been moved to the new stage yet**, so that is still a point on the
*previous* stage's map. Every bot landed nowhere and every one was discarded.
The retry worked only because the player had been placed by then.

The same cause explains the collapsed fan-out recorded under *Decisions* below:
positions like `(-2833, 898, -285)` for all three, then `y = 658` and properly
spread 48 ms later. Both are one bug — a stale position — wearing two faces.

**The fix: spawn at the line map's own start slots**, 1..3, with the player on
slot 0. A start slot is on the map by definition, so `SetPosition` cannot fail
on it, and it is exactly where a party of four is placed anyway. Read the slots
through `CKTDGLineMap::GetStartPosMap()` and **not** `GetStartPosition( 0..n )`:
`m_mapStartPos` is a `map`, and `GetStartPosition` answers a missing key with
`(0, 0, 0)` — a position `SetPosition` would reject, putting the bug straight
back. The old player-offset fan-out survives as a fallback for a map with fewer
than two start slots, and now logs `AIPARTY NOTE line map has %d start slot(s)`
when it fires.

The retry grace came down from 3 s to 1.5 s at the same time. **It should now
never fire at all** — `never arrived` appearing in the log again means something
regressed.

The general form is the one to carry forward: **this client fails silently by
deleting things.** `CreateNPC` has three such exits (`m_bLastKillCheck`, a
NULL templet, this one) and only one of them is reachable with a log statement
anywhere near it. When something does not appear and nothing says why, look for
a `SAFE_DELETE` on the failure path before looking for a missing call.

#### Staggered spawn - one party member at a time

Also 2026-09-06, also built, deployed and **not play-tested**. Asked for after
the stage-change hitch survived defect 2's fix: building a `CX2GUNPC` loads the
hero's skin meshes and its lua state machine (74-101 states for this cast), and
three of those on one frame is a visible stutter.

`CreateOfflinePartyBots` now sends **one** request per call and `break`s, arming
`m_fOfflineBotSpawnCooldown` (0.5 s); `TickOfflinePartyBots` counts it down and
calls back for the next. `SubStageStart` only starts the sequence. The cooldown
is tested inside `CreateOfflinePartyBots` as well as in the tick, because
`SubStageStart` is a caller too.

**Not done with `CreateNPCReq`'s own `fDelayTime`, and this is worth keeping.**
That parameter really does defer the whole creation — `CreateNPC`'s `else`
branch queues a `CreateNPCData` and builds it later — so it looks like exactly
the right tool, one argument instead of a cooldown. But `CreateNPCData`
([X2Game.h:121](X2Lib/X2Game.h#L121)) carries **no ally-team and no `bNoDrop`
field**. A bot routed through it would be rebuilt without either: it would drop
loot like a monster, and its hits would pass through every enemy again. That is
phase 1 defect 2 exactly — a struct that quietly does not carry a field — and it
would have read as the AI being broken. The reasoning sits in a comment on
`OFFLINE_BOT_SPAWN_INTERVAL` so it is not "simplified" back later.

### The party now survives a stage change, so most of the above stops applying

**2026-09-06. Built, deployed, played, PASSED - and this is what closed defect
2.** Asked for after the staggered spawn: *"why can't we use the same logic as
multiplayer party spawn"* -- the observation being that the loading screen stays
up for a while after the stage BGM has started, so the party could be put in
place under it.

The observation is right and the code says exactly where the window is:
`StageStart` plays the BGM and then calls `StartFadeIn( 99999 )`
([X2DungeonGame.cpp:874](X2Lib/X2DungeonGame.cpp#L874)), which pins the fade
alpha at 1.0 -- fully black -- indefinitely. What lifts it is a second
`StartFadeIn()` with its 0.5 s default, on the line **after** `SubStageStart()`
in `Handler_EGS_DUNGEON_SUB_STAGE_START_NOT`
([X2DungeonGame.cpp:2028](X2Lib/X2DungeonGame.cpp#L2028)). So the spawn was
being fired at the exact instant the curtain began to rise, and a spawn costs a
packet round trip plus three `CX2GUNPC` builds. It could never land in time.

**But the multiplayer party spawn it was compared to does not exist as a
per-stage burst.** Real party members are `CX2GUUser`, built **once** at dungeon
entry in `CX2Game::UnitLoading` ([X2Game.cpp:1677](X2Lib/X2Game.cpp#L1677)) ->
`AddUserUnit()` -> `CreateGUUser`, synchronously, inside the 10/30/80 % loading
screen and with no packet involved. At a stage change they are not re-created at
all: `StageLoading` deletes the world, calls `DeleteAllNPCUnit()`
([X2DungeonGame.cpp:685](X2Lib/X2DungeonGame.cpp#L685)) -- which is what killed
our bots -- and then just walks `m_UserUnitList` calling `InitPosition( true )`
([X2DungeonGame.cpp:814](X2Lib/X2DungeonGame.cpp#L814)). **Multiplayer pays a
reposition per stage, never a construction.** That is why it has no hitch and no
gap to hide in the first place.

So the answer was not to burst inside the window but to stop respawning.
`SERV_IRUHADEV_AIPARTY_PERSIST` (`KTDXLIB/Always.h:2560`, nested under
`SERV_IRUHADEV_OFFLINE`) does three things:

1. **`DeleteAllNPCUnit` spares a living party bot**
   ([X2Game.cpp:4958](X2Lib/X2Game.cpp#L4958)) while `m_bOfflineKeepPartyBots`
   is set, which `StageLoading` sets around that one call and clears
   immediately after. Not a blanket exemption: every other caller still means
   all of them. The precedent is directly below it in the same loop -- the
   studio already spares monster-card summons from the same sweep, and those
   are the same class with the same ally AI, so outliving `m_pWorld` is a path
   it already relies on.
2. **`RepositionOfflinePartyBots`** ([X2Game.cpp:7347](X2Lib/X2Game.cpp#L7347))
   places the survivors on the new stage's line map, resets the AI's target and
   drops them into the wait state -- deliberately a mirror of the
   `m_UserUnitList` loop it is called beside. It runs from `StageLoading` after
   the new world exists, which is load-bearing: `SetPosition` re-derives the
   unit's line index from whatever line map is current, so any earlier and it
   would place the party on a map about to be deleted.
3. **Placement moved into `GetOfflinePartyBotPos`**
   ([X2Game.cpp:7234](X2Lib/X2Game.cpp#L7234)), shared by the spawn and the
   reposition so a kept bot lands exactly where a fresh one would.

**A dead bot is deliberately not spared.** Letting the corpse go means the
ordinary spawn path rebuilds it whole on the new stage -- the cheapest revive
available, and free while the stage is loading anyway.

Why this is safe against the obvious worry, a unit outliving the world it was
standing in: nothing it holds across the boundary is a raw pointer into the old
stage. The AI's target and attacker are `KObserverPtr`
([X2NPCAI.h:145](X2Lib/X2NPCAI.h#L145)) and so is the grab list
(`SetUserGrapReset`), so the monsters being deleted around it null themselves
out; and units read the world through `GetWorld()` on demand rather than caching
a pointer.

What this changes about everything written above:

- **The staggered spawn and the 1.5 s retry grace stay, and stop mattering at
  stage changes.** What is left for `CreateOfflinePartyBots` is dungeon entry, a
  bot that died, and a bot that could not be placed on a new stage.
- **Defect 2 cannot recur.** There is no stage-change spawn left to place at a
  stale player position. The `GetOfflinePartyBotPos` fallback and its
  `NOTE line map has %d start slot(s)` line are still there for the entry spawn.
- **Bots keep their HP across a stage** now, as a real party member does,
  instead of being silently restored by being rebuilt. This is a behaviour
  change and worth watching in the play-test: a party that arrives at stage 3
  half dead is correct, not a bug.

**The one risk worth having worried about, now answered: bots do not fall
through the disabled-line window.** `StageStart` calls `DisableAllLineData()`
right after `StageLoading` returns and the sub-stage's line set is not enabled
again until `SubStageOpen`, so a bot spends a few frames standing on a disabled
line -- exactly as a user unit does. The play-test says it survives that, as
users do. Recorded because the reasoning still holds if anything here moves:
placing the party at `SubStageStart` *instead* is not an option, because with
every line disabled `SetPosition` can fail outright and this code deletes a bot
it cannot place. `StageLoading` is the only moment the whole line map is
available, which is exactly why the studio places the user units there too.

#### What the run said

One full dungeon, 06:49-06:51, six stage changes. The counts are the whole
result and they are exactly the predicted signature:

| `grep` in `offline_server.log` | Expected | Got |
|---|---|---|
| `spawning bot` | 3, once per **dungeon** | **3** -- at 06:49:09, :09.5, :10, the 0.5 s stagger, and never again |
| `kept across the stage change` | 3 per stage change | **18** = 6 x 3 |
| `could not be placed` | none | **0** |
| `never arrived` | none | **0** |
| `NOTE line map has` | none | **0** |
| `UNHANDLED` / `EXCEPTION` in `offline_packets.log` | none | **0** -- including the `EGS_SECRET_STAGE_LOAD_REQ` the previous run hit |
| `SHUTDOWN clean` | 1 | **1** |

The placement lines show three distinct, spread positions at every stage
(`(-1082, 117, -580)`, `(-972, 60, -580)`, `(-865, 5, -580)` and so on), so the
line map's start slots are being read correctly on every map in the dungeon,
not just the first.

The three spawn lines being the *only* three is the point of the whole change:
before this, that block repeated at every stage, and the repeat was what the
player saw as fighting alone and then having three heroes pop in.

**Still never exercised, and still play-test work rather than code work:** a bot
death (nothing at this level hits hard enough -- `was down` is still empty
across every run) and the EXP/ED comparison between an auto-party run and a solo
run.

### Revive is still unproven

Recorded because "assume it works" is a reasonable call to make and a bad thing
to forget having made.

Nothing at the player's level hits hard enough to kill a bot, so the revive path
has never run: `grep "was down" offline_server.log` is empty across every run.
The decision (2026-09-06) is to assume it works and move on, which is fine as
far as it goes — but **it is not independent of defect 2, and that is the part
worth remembering.**

`TickOfflinePartyBots` finds each bot with exactly the lookup defect 2 shows
failing:

```cpp
CX2GUNPC* pNpc = GetNPCUnitByUID( (int)npcSlot.m_iNpcUid );
if( NULL == pNpc ) { ...treat as missing, do not check its HP... }
```

That coupling mattered while defect 2 was open, because a bot living under a UID
the room did not know would have made revive unreachable rather than merely
untested. **Defect 2 is now settled and it was not that case** — the bots do
live at their slot UIDs, so the lookup above is sound and revive is simply
unexercised. It still wants one death in a real run before anyone calls it
working.

If it does need provoking later, the cheap ways are a dungeon several levels
above the character, or temporarily dropping `RESPAWN_DELAY` and letting a boss
do the work — not a code path that kills the bot artificially, which would
prove the respawn and not the detection.

### One lead ruled out on defect 2, so nobody re-walks it

`CX2Game::CreateNPC` opens with a silent early return:

```cpp
if( m_bLastKillCheck == true )
    return;          // no log, no trace
```

It is exactly the right shape for defect 2 — a stage-scoped flag that suppresses
NPC creation with nothing written anywhere — and it is **not** the cause.
`CX2DungeonGame::SubStageStart()` sets `m_bLastKillCheck = false` at its very
top ([X2DungeonGame.cpp:1068](X2Lib/X2DungeonGame.cpp#L1068)), and
`CreateOfflinePartyBots()` is called at the *end* of that same function, so the
flag is already clear by the time the request goes out, and it only becomes true
again when the sub-stage is cleared. The log agrees: the stage's own monsters
were alive and one was killed between the failed request and the retry.

The remaining suspects are all downstream of `CreateNPC` being entered at all,
which is why the diagnostic under defect 2 asks for the UID *the NPC was built
with* rather than for another guard.

### What reading settled before the play-test

Three of phase 2's five steps were questions rather than work, and the answers
changed what had to be built.

1. **`IsAllUserDead()` cannot see a bot, so a wiped party of bots cannot end the
   run.** It iterates `m_UserUnitList` ([X2Game.cpp:6114](X2Lib/X2Game.cpp#L6114)),
   which holds `CX2GUUser*` only; a bot is a `CX2GUNPC`. Step 4's worry does not
   arise and no code was needed for it.
2. **Nothing in an offline dungeon revives a bot - step 4 was a build, not a
   check.** `RebirthUserUnit`'s bot branch is real but unreachable on this path:
   its only two callers are
   `CX2Game::Handler_EGS_RESURRECT_TO_CONTINUE_DUNGEON_NOT`
   ([X2Game.cpp:11792](X2Lib/X2Game.cpp#L11792)) and the PvP state
   ([X2StatePVPGame.cpp:2375](X2Lib/X2StatePVPGame.cpp#L2375)). The offline
   server *refuses* `EGS_RESURRECT_TO_CONTINUE_DUNGEON_REQ` with
   `ERR_RESURRECT_00` (no resurrection stones offline), so the NOT is never
   sent. And the dead bot does not quietly disappear and get re-spawned either:
   the per-frame dead-NPC cleanup **explicitly skips PvP bots**
   ([X2Game.cpp:3014](X2Lib/X2Game.cpp#L3014)) - which is what the PvP revive
   depends on - so the corpse stays in the world at 0 HP, `GetNPCUnitByUID`
   keeps answering, and `CreateOfflinePartyBots`'s idempotence check skips it
   forever. Left alone, a bot that dies is gone until the next stage change.
3. **The plan's placement precedent does not exist.** Step 2 says to fan out
   "the way the field spawner in `X2OfflineBattleField.cpp` already picks
   positions" - that file contains no line-map call at all. The real precedent
   is `CX2GUNPC::InitPosition`'s own fallback, `GetLandPosition`, and the
   studio's `CreateAllyEventMonster`, which spawns its allies at exactly
   `pUser->GetPos()` with no offset whatsoever
   ([X2DungeonGame.cpp:3556](X2Lib/X2DungeonGame.cpp#L3556)).

### Decisions made while implementing phase 2

- **Revive is delete-and-respawn, not `RebirthUserUnit`.**
  `CX2Game::TickOfflinePartyBots( float )` runs every frame from
  `CX2DungeonGame::OnFrameMove` while the dungeon is in `GS_PLAY`, counts how
  long each bot has been at 0 HP in `m_mapOfflineBotDeadTime`, and after 8
  seconds calls `DeleteNPCUnitByUID` and then `CreateOfflinePartyBots()`.
  Reusing the studio's bot branch was rejected on two specifics: it calls
  `InitPosition( false, -1 )`, which in a dungeon picks a **random** line-map
  start position ([X2GUNPC.cpp:3792](X2Lib/X2GUNPC.cpp#L3792)) rather than
  anywhere near the player, and it forces `GetStartState()`, which for this cast
  is the card-summon entrance phase 1 defect 3 removed. Delete-and-respawn goes
  down the path that already runs at every stage change, so the bot returns at
  the player, at slot stats, with its UID handed back by the offline server, and
  in the wait state.
- **Per frame, not per sub-stage.** A bot dies mid-fight and `SubStageStart` -
  where `CreateOfflinePartyBots` hangs - may not come round for minutes. The
  tick is free on the solo path: it returns on the first line when the room has
  no bot slots.
- **A three-second spawn grace period, in `CreateNPC` beside the phase 1
  entrance fix.** The respawn puts the bot back **at the player**, which is very
  often the exact spot and moment that killed it; without this a bot can
  respawn into the same boss attack and die on arrival, forever. Three rather
  than the studio's five because this also fires on the ordinary per-stage
  spawn, where nothing is threatening it yet.
- **The cast is all ten heroes and the draw is random, without replacement.**
  Asked for during the phase. `BOT_CAST` / `BOT_NPC_ID` grew from three rows to
  the ten `NUI_CSM_PVP_HERO_*` that phase 0 probed and found fully populated;
  `MakePartyBots` does a partial Fisher-Yates over the cast **indices**. Without
  replacement is a correctness requirement, not a preference - two bots sharing
  a hero would collide in `FindPartyBotByNpcID`, and the second would be handed
  an ordinary monster UID and stop being a bot. `NUI_PVP_RUNE_GUARD` stays out:
  it loads, but with `defP = defM = 0` it is a stationary guard object.
- **Ten distinct unit classes had to be found**, because the class is the only
  channel the hero selection has to the client. They are the six base classes
  plus `UC_ELSWORD_KNIGHT`, `UC_ELSWORD_MAGIC_KNIGHT`, `UC_LIRE_COMBAT_RANGER`
  and `UC_LIRE_SNIPING_RANGER` - all unconditional enum entries, all real
  classes with templet rows, since a bot slot still constructs a `CX2Unit`. The
  class is never seen: the model is the hero NPC's and the caption is the slot
  nickname (`CX2GUNPC::GetUnitName`, [X2GUNPC.cpp:25618](X2Lib/X2GUNPC.cpp#L25618)).
- **Placement fans out and is snapped to the line map.** 60 behind, 80 ahead,
  100 behind, relative to the player's facing, each run through
  `GetLandPosition( vPos, LINE_RADIUS, &iLineIndex )` so a bot offset past the
  edge of a narrow platform lands on it rather than beside it.
- **Step 5 needed no code.** The offline server already writes no
  `m_mapNpcLevel` / `m_mapNpcID` row for a bot, so `EGS_NPC_UNIT_DIE_REQ` finds
  nothing to price. It still wants the before/after `els_db.sql` check in the
  play-test, which is in the exit table above.
- **A land-snap that lands nowhere near what was asked for is discarded.**
  Found in the same log as defect 1: the early bursts placed all three bots at
  *exactly* the same point, `(-2833, 898, -285)`, while a later one in the same
  stage fanned them out correctly at `y = 658`. With no line under the point -
  which is the case at a stage start, before the player has been placed -
  `CKTDGLineMap::GetLandPosition` answers with a far-away fallback, and it
  answers with the **same** fallback for every input, so the fan-out collapses
  into a stack. The snap is now taken only if it lands within 200 units of the
  requested position; otherwise the raw offset stands, which is the player's own
  position - what the studio's `CreateAllyEventMonster` uses unmodified anyway.
  This is the reason to use a real precedent rather than a plausible-sounding
  API: `GetLandPosition` is right for the platform-edge case the fan-out
  introduced and wrong for the not-loaded-yet case, and only the log told them
  apart.
- **A three-second retry, not a one-shot request.** A consequence of defect 1's
  grace timer that is worth having on its own: a spawn request the offline
  server never answers now produces
  `AIPARTY bot "X" (uid=-N) never arrived after 3s - asking again` and is sent
  again, rather than leaving a slot silently empty for the rest of the stage.
  It is what surfaced defect 2 within one run.

### No new packets, on either side

Worth recording because the plan left it open and phases 3-5 will want to know:
**phase 2 added no packet, no packet field and nothing under
`KncWX2Server/Common/`.** Three bots travel through the same
`KRoomSlotInfo` / `KRoomUserInfo` list phase 1 already widened, and the revive
is entirely client-side - it is a `DeleteNPCUnitByUID` followed by the ordinary
`EGS_NPC_UNIT_CREATE_REQ` that every spawn uses. The mod stays client-only and
the servers still never need rebuilding for it.

The one packet the run turned up, `EGS_SECRET_STAGE_LOAD_REQ` (id=1197,
`*** UNHANDLED ***`), has **nothing to do with the AI party** - it is a dungeon
feature offline mode has never implemented. It belongs to offline mode's own
backlog, not to this plan.

---

# Phase 3 — The matchmaking ceremony

> **Done and play-tested 2026-09-06, first cycle, no defects.** The whole
> ceremony is offline-server work - no client file was touched, and no packet
> or packet field was added on either side. The paragraphs below are the plan
> **as written beforehand** and are kept unedited; for what the code does, read
> *Exit test*, *What phase 3 actually built* and *Decisions made while
> implementing phase 3* underneath. One thing the plan got backwards is worth
> knowing before reading it: `EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT` is sent
> **by the client**, not to it - see correction 1. Two checks are still owed
> and are not phase 3's: the solo-button regression went unexercised this run,
> and phase 2's bot death and EXP/ED comparison have still never happened.

Auto-party already reaches the dungeon after phase 1; what it does not do is
*look* like matchmaking. Right now the ACK is followed immediately by
`EGS_PARTY_GAME_START_NOT`, so the player presses a button and teleports. The
client's own auto-party flow expects, in order:

```
EGS_AUTO_PARTY_DUNGEON_GAME_REQ / _ACK
EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT     queued; UI switches to the auto-party panel
EGS_AUTO_PARTY_MAKING_SUCCESS_NOT            matched; the accept popup
EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT      everyone accepted
EGS_PARTY_GAME_START_NOT                     move into the dungeon
```

plus `EGS_CANCEL_AUTO_PARTY_MAKING_REQ/ACK`, `EGS_UNREG_AUTO_PARTY_WAIT_LIST_NOT`
and `EGS_AUTO_PARTY_CLOSE_NOT` for the cancel path. Read `CX2PartyManager`'s
handlers for each before deciding how much of the ceremony to reproduce — a
short fake queue (1-2 s) that then "matches" reads far better than an instant
jump, and it is the only thing that gives the cancel path somewhere to happen.

The queue needs a timer the offline server owns. `TickField` and
`PushRemainingPlayTime` ([Handlers_Room.cpp:400](X2Lib/Offline/Handlers_Room.cpp#L400),
[:571](X2Lib/Offline/Handlers_Room.cpp#L571)) are the existing precedent for
deferred server-side work; reuse that shape rather than inventing a second one.

Decide and record: does offline auto-party always fill to 4, or offer a count?
Default to 4.

Exit test: press auto-party in the village, watch it queue and match, land in
the dungeon with three bots. Cancel mid-queue and end up back in the village
with the UI reset. Start the same dungeon with the normal button and go in
alone.

### Exit test — PASSED (2026-09-06)

One build/play cycle, no defects. All three ways out of the ceremony — deny,
cancel, accept — were exercised in a single 18-second stretch of the same run,
which is what makes the trace below worth keeping: it is the whole state
machine on one page.

| Check | Result |
|---|---|
| Auto-party queues instead of teleporting; the panel switches to the waiting view with a running clock | **PASS** |
| It "matches" a few seconds later: success sting, accept popup naming the dungeon and **4** members | **PASS** — `matched - 4 member(s), accept popup up` |
| Accept lands in the dungeon with three bots, as before | **PASS** — Edan, Apple, Lowe at uid −2/−3/−4, staggered 0.5 s apart |
| Deny puts the party panel back and auto-party works again | **PASS** — `declined`, then `closed, reason=1042`, and the very next press queued normally |
| Cancel mid-queue returns to the village with the UI reset, and auto-party can be pressed again | **PASS** — cancelled 1.15 s into the queue, before the match could fire |
| No new failures | **PASS** — `UNHANDLED` and `EXCEPTION` are **0** across the whole file, including the `EGS_SECRET_STAGE_LOAD_REQ` the previous run hit |
| Clean exit | **PASS** — one `SHUTDOWN clean` |
| **The normal start button still spawns nobody** | **NOT RUN this cycle.** No `EGS_QUICK_START_DUNGEON_GAME_REQ` in the log at all. Phase 3 does not touch that handler, but the standing regression check went unexercised and should ride along with the next run |

Also not exercised, and still carried forward from phase 2: **a bot death** and
the **EXP/ED comparison** against a solo run. This run left the dungeon early
(`leaving room 1001 (reason=114)`) and never changed stage, so
`kept across the stage change` is 0 — phase 2's persistence was proven in the
06:49 run, not this one.

#### What the run said

The full ceremony, three presses, straight out of `offline_packets.log`:

```
07:37:09.995  C->S  EGS_AUTO_PARTY_DUNGEON_GAME_REQ
07:37:09.995  S->C  EGS_AUTO_PARTY_DUNGEON_GAME_ACK
07:37:09.995  S->C  EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT
07:37:12.509  S->C  EGS_AUTO_PARTY_MAKING_SUCCESS_NOT          <- 2.514 s of queue
07:37:17.422  C->S  EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT    <- denied
07:37:17.422  S->C  EGS_AUTO_PARTY_CLOSE_NOT

07:37:19.269  C->S  EGS_AUTO_PARTY_DUNGEON_GAME_REQ            <- 1.8 s later, so the deny
07:37:19.269  S->C  EGS_AUTO_PARTY_DUNGEON_GAME_ACK               really did unlatch the UI
07:37:19.269  S->C  EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT
07:37:20.418  C->S  EGS_CANCEL_AUTO_PARTY_MAKING_REQ           <- cancelled mid-queue
07:37:20.418  S->C  EGS_CANCEL_AUTO_PARTY_MAKING_ACK

07:37:21.671  C->S  EGS_AUTO_PARTY_DUNGEON_GAME_REQ            <- 1.3 s later, likewise
07:37:21.671  S->C  EGS_AUTO_PARTY_DUNGEON_GAME_ACK
07:37:21.671  S->C  EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT
07:37:24.484  S->C  EGS_AUTO_PARTY_MAKING_SUCCESS_NOT          <- 2.813 s of queue
07:37:27.732  C->S  EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT    <- accepted
07:37:27.732  S->C  EGS_PARTY_GAME_START_NOT
```

The two follow-up presses at :19.269 and :21.671 are the real result of the
deny and cancel rows, and they are worth more than the screenshot: the client
will not send `EGS_AUTO_PARTY_DUNGEON_GAME_REQ` again while
`SetProcessDungeonMatch` is latched, so a second press arriving at all is proof
that the close NOT and the cancel ACK each unlatched it. That was the one thing
in this phase that could have silently half-worked — the panel looking right
while the flag stayed set, locking the party window for the rest of the
session.

`reason=1042` in `offline_server.log` is `NOT_LEAVE_AUTO_PARTY_REASON_03`
resolved to its enum value; there is no name table for `NetError` on this side.

**Every one of the six `S->C` packets above lands on the same millisecond as
the `C->S` that provoked it, except the two `MAKING_SUCCESS_NOT`s, which land
on the same millisecond as an `EGS_UPDATE_PLAY_STATUS_NOT`.** That is the
emulator's whole timing model made visible: it runs only when the client sends
something, and the queue is counted on the one packet that arrives whether the
player does anything or not.

#### The queue latency, measured

The status push was rock-steady at 2.99 s across the whole run
(`00.428, 03.524, 06.521, 09.519, 12.509, 15.500, 18.492, 21.492, 24.484,
27.479`). Both presses landed early in a window, so both queues came in near
the 2.5 s floor:

| Press | Previous beat | δ into the window | Queue actually taken |
|---|---|---|---|
| 07:37:09.995 | 09.519 | 0.476 s | **2.514 s** |
| 07:37:21.671 | 21.492 | 0.179 s | **2.813 s** |

That is the lucky branch, and the arithmetic should be written down before
somebody is surprised by the other one. With a 3.0 s beat and a 2.5 s floor,
the deadline falls inside the *next* window only when δ ≤ 0.5 s — a 1-in-6
slice — and the wait is then 2.5-3.0 s. Otherwise the deadline slips past the
next beat and the wait is **3.0-5.5 s**. Both observed presses hit the 1-in-6
branch; a five-second search is normal, not a regression. If that ever reads as
too long, the fix is to lower `AUTO_PARTY_QUEUE_MS`, which shrinks the long
branch and never the short one — not to add a second timer.

### What phase 3 actually built

The whole ceremony is offline-server work; **no client file was touched at
all**, and no packet or packet field was added, on either side. Every one of
the five packets already exists and `CX2PartyManager` already draws all of
them — phase 1 simply skipped four of them.

The flow now, with the client-side handler that reacts to each:

```
C->S  EGS_AUTO_PARTY_DUNGEON_GAME_REQ
S->C  EGS_AUTO_PARTY_DUNGEON_GAME_ACK              (NET_OK; nothing else happens on it)
S->C  EGS_REG_AUTO_PARTY_WAIT_LIST_SUCCESS_NOT     X2PartyManager.cpp:2749 - panel -> waiting,
                                                   SetProcessDungeonMatch( true ) latches
        ...2.5 s of fake queue, counted on the status push...
S->C  EGS_AUTO_PARTY_MAKING_SUCCESS_NOT            X2PartyManager.cpp:2828 - success sting,
                                                   10 s OK/Cancel popup
C->S  EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT      the player's answer
   accept -> S->C EGS_PARTY_GAME_START_NOT         room opened here, bots made here
   deny   -> S->C EGS_AUTO_PARTY_CLOSE_NOT         reason 03
and at any point in the queue:
C->S  EGS_CANCEL_AUTO_PARTY_MAKING_REQ
S->C  EGS_CANCEL_AUTO_PARTY_MAKING_ACK             X2PartyManager.cpp:2795 - unlatches the flag
```

New in `X2Lib/Offline/`: `CX2OfflineServer::KAutoPartyMatch` (a three-state
machine plus the request), `TickAutoPartyMatch`, `StartAutoPartyDungeon`,
`CloseAutoPartyMatch`, and handlers for the two client packets that had never
been sent before. `Handler_EGS_AUTO_PARTY_DUNGEON_GAME_REQ` shrank to
"validate, ACK, queue".

### Decisions made while implementing phase 3

- **The room is not opened until the player accepts, and this is the decision
  the rest of the shape hangs off.** The obvious build — open the room at
  request time, hold it, send `EGS_PARTY_GAME_START_NOT` on accept — is a trap:
  `OpenDungeonGameRoom` -> `OpenRoom` calls `m_kRoom.Clear()`, sets
  `m_bActive`, and puts the session in `S_ROOM`
  ([Handlers_Room.cpp:458](X2Lib/Offline/Handlers_Room.cpp#L458)), and there is
  no path back out of that short of opening another room. A cancelled or denied
  match would leave a live dungeon room behind a player standing in the
  village. So `KAutoPartyMatch` keeps the *request* whole and replays it, and
  `OpenRoom` clears any pending match on the way in — which makes every other
  entry into a room (the solo button, a battlefield, the tutorial) abandon a
  stale queue for free, without any of those handlers being edited.
- **The queue is counted on `EGS_UPDATE_PLAY_STATUS_NOT`, not on a timer.** The
  emulator has no clock — it only runs when the client sends something — and
  the plan named `TickField` / `PushRemainingPlayTime` as the precedent to
  reuse rather than inventing a second mechanism. Both ride that packet, and it
  turns out to be exactly as regular in a **village** as in a dungeon: it is
  pushed by `CX2State::CheckAndSendingPlayStatus` off
  `m_TimerForSendingPlayStatus`, constructed at `3.0f`
  ([X2InstanceData.cpp:78](X2Lib/X2InstanceData.cpp#L78)), from
  `CX2StateMenu::OnFrameMove` ([X2StateMenu.cpp:161](X2Lib/X2StateMenu.cpp#L161)).
  The previous run's log confirms it: 24 consecutive pushes 3.00-3.05 s apart
  while standing in the village.
- **2.5 s of queue, which is really 2.5-5.5 s.** The three-second beat
  quantises the wait upward, so `AUTO_PARTY_QUEUE_MS` is a floor and the beat
  adds up to 3 s of jitter on top. Picked low deliberately: the failure mode
  worth avoiding is collapsing back into phase 1's instant teleport, and 2.5 s
  cannot — the first eligible beat is at least that late. The variance reads as
  matchmaking rather than as a bug.
- **A client-side per-frame tick was considered and rejected.** Phase 2 already
  added `TickOfflinePartyBots` to `CX2DungeonGame::OnFrameMove`, so ticking the
  offline server from the village's frame move would have been an easy 200 ms
  resolution. It would also have called `Reply` -> `KSession::QueueingEvent`
  from the render thread rather than the `KSession::Run` thread, outside
  `m_csDispatch` and outside the per-packet SQLite savepoint — i.e. it would
  break both of offline mode's two stated invariants at once to buy two seconds
  of precision on a fake queue.
- **Four members, always.** The plan left this open. `m_iMemberCount` is only
  what the accept popup reads out, and it has to agree with
  `AUTO_PARTY_BOT_NUM` or the popup promises a party the room will not contain,
  so it is written as `1 + AUTO_PARTY_BOT_NUM` rather than as a second
  constant. Offering a count would need UI that does not exist.
- **Deny closes with `NOT_LEAVE_AUTO_PARTY_REASON_03`; success closes with
  nothing.** `Handler_EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT( false )` puts
  the client's own buttons back but does **not** clear
  `SetProcessDungeonMatch` ([X2PartyManager.cpp:2857](X2Lib/X2PartyManager.cpp#L2857)),
  so something has to. `EGS_AUTO_PARTY_CLOSE_NOT` is that something, and
  reason 03 — "a party member did not agree to the auto-party game start" — is
  both the branch that clears the flag and the honest description: the party
  member who declined is the player. It is **not** sent on the success path,
  because every branch of that handler falls through to
  `Battle_Atena_Fail.ogg` ([X2PartyManager.cpp:2937](X2Lib/X2PartyManager.cpp#L2937))
  and `EGS_PARTY_GAME_START_NOT` already clears the flag itself
  ([X2PartyManager.cpp:1457](X2Lib/X2PartyManager.cpp#L1457)) — so it would buy
  nothing and play a failure sting over a successful match.
- **The popup cannot hang the state machine.** It is armed with
  `SMUCM_DUNGEON_MATCH_GAME_DENY` as its `timeOutMsg`
  ([X2PartyManager.cpp:2853](X2Lib/X2PartyManager.cpp#L2853)), and
  `AddTimedMessagePopup`'s third command id *is* the timeout one
  ([X2Main.h:1607](X2Lib/X2Main.h#L1607)), so a player who walks away denies by
  default after ten seconds and the reply still arrives. There is no way out of
  that popup that leaves `APS_WAIT_REPLY` waiting forever, which is why no
  server-side popup timeout was built.
- **`EGS_UNREG_AUTO_PARTY_WAIT_LIST_NOT` is deliberately never sent.** The plan
  listed it with the cancel path, but it is the *server-initiated* unregister —
  the player-initiated one is answered by the cancel ACK, and offline nothing
  ever kicks a player out of a queue of one. Its client handler
  ([X2PartyManager.cpp:2813](X2Lib/X2PartyManager.cpp#L2813)) does exactly what
  the cancel ACK path already does.
- **A match is tagged with the character that made it.** `TickAutoPartyMatch`
  drops the match if `kSes.m_nSelectedUnitUID` no longer matches. Without that,
  queueing, going back to character select and picking a different character
  would leave the match armed, and the next status push would drop somebody
  else into a dungeon they never asked for.
- **A reply arriving at an idle match is logged and ignored, not an error.**
  The panel's cancel button and the accept popup can be live at the same
  moment; cancelling first leaves the popup on screen to time out and deny into
  `APS_IDLE`. Acting on it would re-open a match the player has already closed.

### Corrections to this plan, found by doing it

1. **`EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT` is client-to-server, not
   server-to-client.** The plan's ordered list annotates it "everyone
   accepted", which reads as a broadcast the server sends. It is the opposite:
   `CX2PartyManager::Handler_EGS_AUTO_PARTY_MAKING_SUCCESS_REPLY_NOT( bool )`
   ([X2PartyManager.cpp:2857](X2Lib/X2PartyManager.cpp#L2857)) is a *sender*,
   called from the accept popup's OK, Cancel and timeout commands. So the
   ceremony has one more inbound packet than the plan implies and one fewer
   outbound, and the offline server has to *handle* it rather than push it.
   Both it and `EGS_CANCEL_AUTO_PARTY_MAKING_REQ` had never once been sent
   before this phase, because the client gates both on
   `GetProcessDungeonMatch()`, which only phase 3 ever latches.
2. **The plan's `TickField` / `PushRemainingPlayTime` line refs are stale**
   (`Handlers_Room.cpp:400` and `:571`); they are at
   [:669](X2Lib/Offline/Handlers_Room.cpp#L669) and
   [:840](X2Lib/Offline/Handlers_Room.cpp#L840). The shape they demonstrate is
   the right one and was reused; only the numbers had drifted.
3. **Nothing needed a scope guard for the solo button, again.** The whole
   ceremony hangs off `EGS_AUTO_PARTY_DUNGEON_GAME_REQ`, which only the
   auto-party button sends, and `m_kAutoParty` is cleared by any room opening.
   `Handler_EGS_QUICK_START_DUNGEON_GAME_REQ` is untouched for the third phase
   running.
4. **"The cancel path" in the plan is three different exits, not one.** The
   plan lists `EGS_CANCEL_AUTO_PARTY_MAKING_REQ/ACK`,
   `EGS_UNREG_AUTO_PARTY_WAIT_LIST_NOT` and `EGS_AUTO_PARTY_CLOSE_NOT`
   together as "for the cancel path". They are not interchangeable: the ACK
   answers a cancel *during the queue*, the close NOT answers a *deny at the
   popup*, and the UNREG NOT is server-initiated and is never sent offline at
   all. Each clears `SetProcessDungeonMatch` down a different branch, and
   picking the wrong one for a given exit leaves the flag latched - which
   looks like nothing at all until the player tries to press the button a
   second time. The play-test's two follow-up presses are what proved the
   right ones were picked.
5. **`EGS_REQUEST_GET_AUTO_PARTY_BONUS_INFO_NOT` (id=903) is part of this flow
   and needed nothing.** It arrives in pairs, several times, while the party
   panel is open - eight times in the eighteen seconds of the play-test. It
   was already stubbed in `Handlers_Stub.cpp` from offline mode's own work and
   decorates the hero-recruit banner. Worth naming only so the next reader of
   the packet log does not mistake it for part of the ceremony.

### If any of this regresses later, where to look

Written before the play-test and kept because none of it fired - which means
none of these symptoms has ever been seen and each row is a prediction, not a
recollection.

| Symptom | Where to look |
|---|---|
| The button teleports as before, no queue | `grep "queued for dungeonID" offline_server.log` — if it is there, the client ignored the NOT; if not, the ACK failed and the handler returned early |
| Queue never resolves, panel sits at 00:0x forever | `grep "matched -" offline_server.log`. Empty means `TickAutoPartyMatch` is not being reached — check that `id=905` is still arriving in `offline_packets.log` while the panel is up |
| Accept popup appears, accepting does nothing | `grep "accepted - opening" offline_server.log`. Missing means the reply NOT is not being dispatched; look for it as `*** UNHANDLED ***` |
| Party window will not open again after a cancel or deny | `SetProcessDungeonMatch` is still latched — the cancel ACK or the close NOT did not arrive. Both are logged |
| Bots stop appearing | The accept path is the only one that calls `MakePartyBots` now. `grep "bot slot" offline_server.log` |

---

# Phase 4 — Make them look like party members

Cosmetic, and the phase that decides whether this feels like a party or like
three pets.

1. **Gate 3** — widen the `GT_PVP` HUD branch at
   [X2Game.cpp:6437](X2Lib/X2Game.cpp#L6437) so a dungeon bot gets
   `InsertPvpMemberUI` / `UpdatePvpMemberGageData` too. Cheapest route to
   on-screen HP bars, but it uses the PvP team-list widget.
   Unlike gates 1 and 2, this one **is** on the dungeon path — it is inside
   `CX2Game::CreateNPC`, which the dungeon reaches through the offline server's
   `EGS_NPC_UNIT_CREATE_NOT` broadcast; phase 1's entrance-animation fix already
   lives a few lines below it. Confirm that rather than assume it, though: gates
   1 and 2 were in `CX2Game` too and the dungeon reaches neither.
2. The dungeon party HUD proper is `InsertPartyMemberUI( const KPartyUserInfo&,
   CX2GameUnit* )` ([X2GageManager.cpp:395](X2Lib/X2GageManager.cpp#L395)),
   driven entirely by `CX2PartyManager` off `EGS_PARTY_*` packets
   ([X2PartyManager.cpp:458](X2Lib/X2PartyManager.cpp#L458)). Using it means the
   offline server also fabricating a party — a `KPartyUserInfo` per bot, and
   periodic `KPartyMemberStatus` pushes for their HP/MP. Strictly better
   looking, meaningfully more work. Pick one *after* seeing option 1 on screen.
3. Names. `RoomNpcSlot::m_wstrNpcName` comes from the slot's nickname, so the
   offline server picks it. Character names (Elsword, Aisha, ...) read better
   than invented ones and match what the NPC actually is.
4. Result screen: `X2StatePVPGame.cpp:1705` and `:3052` already read
   `GetNpcSlot()` for the PvP result. Check what the dungeon result screen does
   with a party of one versus four, and whether bots should appear there at all.

### Exit test — PASSED (2026-09-06), after one defect and a second cycle

Two build/play cycles. The bars worked first time; one defect at the very end
of the run needed the second. **The re-test run also closed both of the checks
that had been carried since phase 2**, so phase 4 ends with nothing owed.

| Check | Result |
|---|---|
| Each AI party member has an HP/MP bar, in the left column | **PASS** |
| Portrait, name and level read as a party member's | **PASS** |
| The bar tracks the fight | **PASS** |
| Bars survive a stage change | **PASS** — two stage changes in the 08:10 run, `kept across the stage change` ×3 each time |
| A dead party member's bar empties, and there is one bar per slot after the respawn | **PASS** — the first bot deaths this project has ever seen in play, three at once at 08:11:20, all three respawned and no duplicate bar |
| **Bars are gone by the result screen** | **FAIL** on cycle 1 — *Defect 1* below. **PASS** on cycle 2: `AIPARTY dungeon paid out - party stood down (3 of 3 still in the world)` at 08:38:54, one line after the clear |
| The end-of-dungeon respawn spam is gone | **PASS** — `never arrived` is **0** across the whole file, against nine in four seconds before the fix |
| No new failures | **PASS** — `UNHANDLED` and `EXCEPTION` are 0 |
| **The normal start button still spawns nobody** | **PASS at last.** Owed since phase 2 and skipped by every run since. `dungeon room 1002 dungeonID=30080 dif=2 mode=0 for unitUID=12, 0 bot(s)`, against `room 1001 … 3 bot(s)` for the auto-party run into the same dungeon ninety seconds earlier |
| **EXP and ED are not reduced by having a party** | **PASS.** Same dungeon, same difficulty, back to back:<br>auto-party `CLEARED in 85s: 31 kill(s), +972 exp (+291 clear bonus), +1656 ED`<br>solo `CLEARED in 94s: 32 kill(s), +972 exp (+291 clear bonus), +2106 ED` |

**The EXP figure is exactly equal and that is the point of the row.** A bot
takes no share: it deals damage, and the kill credit and the clear bonus are
the player's either way. The ED differs because ED is picked up off the ground
— the solo run had one more kill and better drop luck — and drop RNG is the
only thing separating the two numbers. So an AI party is a pure help: faster
clears at identical reward. Whether that is *too* generous is a phase 5
question, and now it is a phase 5 question with a measurement behind it.

The 08:37-08:41 session had no `SHUTDOWN clean` because the client was still
open when the logs were read, not because it fell over.

### Defect 1 — the party's HP bars outlived the party

**What the player saw.** At the end of the dungeon the bots vanish, and their
three bars stay on screen with empty gauges until the result screen replaces
everything. Invisible before phase 4, because there were no bars to be left
behind — which is what makes this a phase 4 defect and not a phase 2 one.

**What the logs said, and it is a better story than the symptom.** The client
kills its own ally NPCs at dungeon clear. Three lines of `offline_packets.log`
have the whole thing:

```
08:11:12.555  S->C  EGS_DUNGEON_KILLALLNPC_CHECK_NOT     <- the dungeon is cleared
08:11:12.584  C->S  EGS_NPC_UNIT_DIE_REQ  x3             <- ...and the client kills all three bots
08:11:19.618  S->C  EGS_END_GAME_DUNGEON_RESULT_DATA_NOT <- 7 s later, the reward screen
```

with `offline_server.log` naming the reason: `NPC uid=-2 id=0 lv=0 died:
KILL_SELF`, three times. Nothing in this mod asked for that; it is the studio
tidying the field once there is nothing left to fight.

So the empty bars were only half of it. `TickOfflinePartyBots` did exactly what
it is built to do with a dead party member — waited out `RESPAWN_DELAY`,
deleted it, asked for it back — and there is no longer a sub-stage to spawn
into, so **nine spawn requests went out in four seconds and not one of them
arrived**, until the state change stopped the tick. That loop has been running
since phase 2 and nobody saw it, because a bot that never comes back looks
identical to a dungeon that has ended.

**The fix** is one new method, `CX2Game::EndOfflinePartyBots()`, called from
`CX2DungeonGame::Handler_EGS_END_GAME_DUNGEON_RESULT_DATA_NOT`
([X2DungeonGame.cpp:2100](X2Lib/X2DungeonGame.cpp#L2100)). It deletes whatever
is left of the party, removes every bar, and latches `m_bOfflinePartyOver`,
which is the entire thing `TickOfflinePartyBots` needs to stop.

**Why that packet, out of the four the end of a dungeon offers** — this was the
only real decision in the fix, and the user picked it: the party should leave
when the rewards come up.

- `EGS_DUNGEON_KILLALLNPC_CHECK_NOT` (the clear) is **too early**. It opens a
  seven-second victory cinematic the party should still be standing in.
- `EGS_END_GAME_NOT` and `EGS_END_GAME_DUNGEON_RESULT_DATA_NOT` land 19 ms
  apart, and of the two it is the second that fills the reward screen in — its
  handler's first line is `g_pData->ResetDungeonResultInfo( kPacket )`.
- `EGS_STATE_CHANGE_RESULT_NOT` is **too late**: that is the state change
  itself, and `~CX2Game`'s `ClearPvpMemberUI` already covers it.

**The paths that do not reach it need nothing.** Quitting a dungeon early or
dying out of one never produces a result packet — and never needs to, because
the game object is destroyed on the way out and `~CX2Game` clears every bar
([X2Game.cpp:775](X2Lib/X2Game.cpp#L775)). That is also why the defect was
confined to a *successful* clear.

**What this teaches, in general form:** a UI element and the thing it describes
have separate lifetimes, and the client is under no obligation to end them
together. Phase 4 added an element whose owner is deleted by code phase 4 never
looked at — and the same run showed the reverse, a tick still tending an object
the game had finished with. Both were silent. When adding a display for
something the client owns, find who deletes that thing, not just who creates it.

### What phase 4 built

Option 1 of the plan's four items, plus the two things it turned out to need,
plus an answer to item 4 that needed no code. Four client files, all behind the
existing `SERV_IRUHADEV_OFFLINE`.

1. **The bar itself** — `CX2Game::CreateNPC` now inserts a PvP-member gage for
   a dungeon bot. Not by widening the `GT_PVP` arm the plan pointed at
   ([X2Game.cpp:6493](X2Lib/X2Game.cpp#L6493)): that arm is the `else` of an
   `if ( GT_DUNGEON )`, so a dungeon can never reach it, and widening it would
   have meant restructuring the studio's if/else. The insert went into phase
   1's existing `IsPvpBot() && GT_DUNGEON` block a hundred lines below instead —
   same condition, already written, and *after* `SetUserSummonedNPCInfo`, so
   the first frame of the bar is drawn from the stats the bot actually fights
   with.
   **The plan's caution was right to be there, and the answer is yes**: unlike
   gates 1 and 2, `CreateNPC` really is on the dungeon path. The proof is not
   that it sits in `CX2Game` — that is exactly the reasoning that failed for
   the other two — but that phase 1's entrance-animation fix is in the same
   function and has been firing in play since 2026-09-05.
2. **The left column, not the right** —
   `CX2GageManager::InsertPvpMemberUI( const RoomNpcSlot& )` gained an offline
   branch that pushes the bar onto `m_vecGageSetPvpMyTeam`. The studio's
   version pushes every NPC onto the *other* team's list and has the my-team
   half commented out with the note that an NPC has no allies — true in PvP,
   where a bot is the opponent, and backwards here.
   That branch also has to tell the widget which side it is on, because
   `CX2PVPPlayerGageUI`'s `RoomNpcSlot` constructor hardcodes
   `m_uiMyTeam = TN_BLUE` ([X2GageUI.h:584](X2Lib/X2GageUI.h#L584)) and
   `SetPosition` picks the column by comparing that against the player's team.
   Hence the one header edit: `SetOfflinePartyBotTeam`, which must be called
   before `InitUI()` because `InitUI` is what calls `SetPosition`.
   **The left column is not a preference, it is the party HUD's own address.**
   `CX2PVPPlayerGageUI::SetPosition`'s my-team case and
   `CX2PartyMemberGageUI::SetPosition` both resolve to `(6, 121 + i*44)`. A
   bot's bar therefore lands in the slot a human party member's bar would have
   occupied, which is the whole of what phase 4 was for. It also fills MP in:
   `UpdatePvpMemberGageData` zeroes MP on the other-team list unless the viewer
   bought the show-opponent-MP cash item.
3. **The portrait** — `CX2Data::GetPvpNpcImageName` knew the ten
   `NUI_PVP_HERO_*` ids and none of the ten `NUI_CSM_PVP_HERO_*` ones, so every
   bot fell through to `default`, came back with empty strings, and drew
   whatever `DLG_PVP_Game_Other_State_NEW.lua` ships with. **This was invisible
   until the bar existed** — nothing else in the client asks for a bot's
   portrait on the dungeon path — and it would have read as "phase 4
   half-worked".
   Fixed as a remap at the top of the function rather than ten extra `case`
   labels: the two groups are the same ten characters (phase 0 re-pointed the
   cast at the card-summoned variants because the plain ones have no templet in
   this build), so the id is translated on the way in and the studio's table
   stays one entry per portrait.
4. **Keeping it honest per frame** — a PvP gage set is *not* driven by
   `CX2GageManager::UpdateGageDataFromGameUnit`, which walks only the my-gage
   and the party-member lists. Whoever owns the game type has to feed it, and
   `CX2PVPGame::OnFrameMove` does that in a loop over the room's NPC slots
   ([X2PVPGame.cpp:176](X2Lib/X2PVPGame.cpp#L176)). The dungeon's equivalent
   went into `CX2Game::TickOfflinePartyBots`, which was already walking every
   bot slot every frame with the `CX2GUNPC` in hand — one loop instead of two,
   and no new call site in `CX2DungeonGame::OnFrameMove`.
5. **Names needed nothing.** The plan's item 3 was already satisfied by phase 1:
   `BOT_CAST` ([Handlers_Room.cpp:118](X2Lib/Offline/Handlers_Room.cpp#L118))
   names each bot after the hero it actually is — Lowe, Lime, Edan, Penensio,
   Noah, Speka, Amelia, Valak, Code: Q-Proto_00, Apple. Phase 4 only made those
   names visible somewhere other than the log.

### No new packets, and the four that phase 4 turned out to depend on

**Phase 4 is the mirror image of phase 3.** Phase 3 was entirely offline-server
work and touched no client file; phase 4 is entirely client work and touched no
file under `X2Lib/Offline/` at all. Between them they add **no packet and no
packet field on either side**, and `KncWX2Server/Common/` is still untouched by
the whole feature.

What phase 4 *did* need was four packets the plan never named, three of them
only discovered by reading a log at the end of a run. Worth listing because
none of them is inferable from the phase 4 section as written:

| Packet | What it turned out to be |
|---|---|
| `EGS_NPC_UNIT_DIE_REQ` (id=180) ×3 | **The one that mattered.** At 29 ms after the clear the client kills its own ally NPCs, reason `KILL_SELF`. Nothing in this mod asks for it and nothing in the plan predicted it; it is the whole cause of *Defect 1* |
| `EGS_DUNGEON_KILLALLNPC_CHECK_NOT` (id=1087) | The clear itself, and the obvious hook that is **wrong** — it opens a seven-second victory cinematic the party should still be standing in |
| `EGS_END_GAME_NOT` (id=201) | The near-miss: 19 ms before the one that was chosen, and it does not carry the result |
| `EGS_END_GAME_DUNGEON_RESULT_DATA_NOT` (id=202) | The hook that was chosen — the packet that fills the reward screen in, and so the moment the party leaves |

**And a warning about how the defect hid.** The end-of-dungeon respawn loop sent
nine `EGS_NPC_UNIT_CREATE_REQ` in four seconds, and the offline server answered
every one of them correctly: nine `HANDLED` lines, no `UNHANDLED`, no
`EXCEPTION`. The packet-log census — the core debugging loop this whole project
runs on — **cannot see a bug made of well-formed packets**. It caught nothing
here, and the thing that did was `grep AIPARTY offline_server.log`, where the
same four seconds read as `never arrived after 2s - asking again` over and
over. Keep logging decisions, not just packets.

### Decisions made while implementing phase 4

- **The dead bot's bar stays on screen, empty, for the whole 8-second
  respawn** - mid-dungeon. At the *end* of a dungeon it does not, and that
  distinction cost a build cycle: see *Defect 1*. `TickOfflinePartyBots` revives by deleting the NPC and letting the
  spawn path rebuild it, so the obvious move is to drop the bar at the same
  time. Leaving it is better: a downed party member reads as *down*, and an
  empty bar that refills is the clearest possible signal that the respawn timer
  is running. What that costs is a duplicate-bar hazard, closed by the
  remove-before-insert in `CreateNPC` — `RemovePvpMemberUIByUserUid` on the
  slot UID immediately before `InsertPvpMemberUI`. Without it the rebuild
  stacks a second bar on the same UID, and `UpdatePvpMemberGageData` updates
  *every* match it finds, so the duplicate would have tracked the fight
  correctly and never looked wrong — just eaten a row.
- **The level is shown, not hidden.** `SetLevelString( -1 )` is the "hide it"
  value and the `RoomNpcSlot` constructor passes exactly that, which is right
  for a PvP bot whose level means nothing. The offline server gives every bot
  the player's own level, so the branch re-sets it *after* `InitUI()` — setting
  it before is overwritten by the constructor's -1.
- **`SetPartyMemberGameUnit` is deliberately not called**, breaking symmetry
  with both of the studio's branches. The overload taking an index reaches into
  `m_vecGageSetPartyMember`, **not** into the vector the bar was just pushed
  onto, so both PvP branches have been writing the owner unit into a different
  and usually empty list for years. Nothing on this path needs it: the bar is
  handed `UpdatePvpMemberGageData( uid, unit )` explicitly every frame.
- **No `SetShow` change, and nothing new clears the bars at the end of a run.**
  `CX2GageManager::SetShow` touches only the my-gage and party-member lists, so
  the state-change hook that hides the HUD for `XS_DUNGEON_RESULT` does not
  reach a PvP bar. It does not need to: `CX2Main::StateChange` calls
  `RemoveStage( m_pNowState, true )` **before** constructing the new state
  ([X2Main.cpp:4867](X2Lib/X2Main.cpp#L4867)), which runs
  `~CX2StateDungeonGame` → `~CX2Game` → `ClearPvpMemberUI`
  ([X2Game.cpp:775](X2Lib/X2Game.cpp#L775)). The bars are gone before the
  result screen exists. If they ever are not, that ordering is what changed.
- **Option 2 was not built, and the plan said not to build it yet.**
  Fabricating a `KPartyUserInfo` per bot plus periodic `KPartyMemberStatus`
  pushes is strictly better looking and meaningfully more work; the instruction
  was to pick after seeing option 1 on screen, and that has not happened yet.
  Two things the option-1 run should settle: whether the PvP widget's
  cosmetics — the hidden rank emblem, the buff-icon strip — read wrong next to
  a real party HUD, and whether the offline server would then also have to fake
  party membership everywhere else `CX2PartyManager` looks.

### Corrections to this plan, found by doing it

1. **The plan's `X2Game.cpp:6437` is not a branch a dungeon can be "widened"
   into.** It is the `else if ( GT_PVP )` of an `if ( GT_DUNGEON )`, so the
   dungeon already has its own arm and the two are mutually exclusive. The work
   is an *addition* on the dungeon side, not a widened condition. The line is
   also now at [:6493](X2Lib/X2Game.cpp#L6493), phase 1 having grown the
   function.
2. **Item 4's answer is "the dungeon result screen needs nothing, and bots
   should not appear on it".** It counts *player* slots — `GetSlotNum()` with
   `pSlotData->m_pUnit != NULL`
   ([X2StateDungeonResult.cpp:961](X2Lib/X2StateDungeonResult.cpp#L961) and
   [:1351](X2Lib/X2StateDungeonResult.cpp#L1351)) — to pick the
   `Reward_Box_<n>_<i>` layout, and `DeleteNpcSlot()` has already moved every
   bot out of `m_SlotDataList` before the dungeon even starts. An offline
   auto-party run therefore shows the identical solo result screen, which is
   also the honest one: a bot earns no EXP, no ED and no reward box.
3. **Phase 5's MODS.md list is now eight client files, not five.** Phase 4 adds
   `X2Lib/X2GageManager.cpp`, `X2Lib/X2GageUI.h` and `X2Lib/X2Data.cpp` to
   `X2Game.h`, `X2Game.cpp`, `X2DungeonGame.cpp`, `X2Room.h`, `X2Room.cpp`.
   Note `X2Data.cpp` is on that list twice over — phase 0's probe is still in
   it and phase 5 plans to delete that; the `GetPvpNpcImageName` remap stays,
   so the file does **not** come off the list when the probe goes.
4. **`X2Lib/X2GageUI.h` is UTF-8 with LF endings**, unlike almost everything
   else in `X2Lib`, which is CP949 with CRLF. Match what the file has rather
   than what the directory usually has — run `file` on it before editing, as
   `CLAUDE.md` says.
5. **`msbuild "/p:SolutionDir=$TRUNK\"` fails from PowerShell** with
   `MSB6001: ... contains an odd number of double-quote characters`: the
   trailing backslash escapes the closing quote and the rest of the command
   line is swallowed into an include path. The plan's own snippet uses a
   forward slash for exactly this reason; from PowerShell, build the value with
   `(Get-Location).Path.Replace('\','/')` and a trailing `/`.


6. **The plan never once mentions that the client kills its own allies at
   dungeon clear**, and every phase before this one got away with it. The
   evidence table's *Revive* row even points at `RebirthUserUnit`'s bot
   branch as though something would bring them back. Phase 2 already found
   that branch unreachable offline; phase 4 found the other half - they are
   killed on purpose, by `EGS_NPC_UNIT_DIE_REQ` with `KILL_SELF`, the
   moment the last monster dies. Anything a later phase hangs on a bot
   being alive has to answer what happens in those last seven seconds.
7. **The `#endif SERV_IRUHADEV_AIPARTY_PERSIST` block inside phase 4's
   changes is untouched and stays that way.** `EndOfflinePartyBots` deletes
   the party deliberately and does *not* go through the persist exemption,
   because this is the one stage teardown the party is not meant to survive.

---

# Phase 5 — Tuning and honesty

- Bot difficulty. **The plan's knob does not work on this path** — see phase 1
  correction 2. `RoomNpcSlot`'s five stats are overridden a second time, after
  the slot override, by `SetUserSummonedNPCInfo`
  ([X2Game.cpp:12900](X2Lib/X2Game.cpp#L12900)), which re-stats any ally with an
  owner from **the player's own** HP/atk/def. So a bot currently has the
  player's stat line, which is a defensible default and is why nothing looked
  wrong in phase 1. Scaling `RoomNpcSlot` would change nothing visible.
  The knobs that do exist: a multiplier applied where
  `SetUserSummonedNPCInfo` writes the stats (a `unitID` case, as the Nasod Watch
  and Wally entries already are), `SetHardLevel`, or dropping `iAllyUID` and
  losing the follow behaviour with it. Whichever is chosen, the named constant
  in `X2Lib/X2Define.h` is still the right shape. Decide with a play-test.
- `CX2AllyNPCAI` is Lua-tuned (`LoadAIDataFromLUA`): `allyLostRange`,
  `targetRange`, `dashRange`, jump rates. If those need changing, the file is
  the studio's and the packing is the user's job — name it and stop
  (`server-lua-packing-is-users-job`).
- Delete phase 0's probe block — `grep AIPARTY` across `X2Lib/` finds it, and it
  should be one block in one function. Phase 0 is the only thing that needed it.
- Update `MODS.md` by **extending the existing `SERV_IRUHADEV_OFFLINE` row** —
  add a sentence about AI party members to its description, saying explicitly
  that they are auto-party only. No new row: there is no new flag.
  (`X2Lib/Offline/` is already listed as a whole directory, so
  `Handlers_Social.cpp` and `Handlers_Room.cpp` need no separate mention.)
  The *Client files* column gains **five** entries, not the two the plan
  guessed, because the hook moved to the dungeon game object and both room
  files were touched:
  `X2Lib/X2Game.h`, `X2Lib/X2Game.cpp`, `X2Lib/X2DungeonGame.cpp`,
  `X2Lib/X2Room.h`, `X2Lib/X2Room.cpp`. `X2Lib/X2Data.cpp` is there too, from
  phase 0's probe — remove it from the list in the same edit that deletes the
  probe, if nothing else in offline mode has claimed that file by then.
- Write the phase history and every correction back into this file, the way
  `OFFLINE_MODE_PLAN.md` does. That record is worth more than the plan was.

---

## Verification, at any point

```sh
TRUNK="F:/elsword stuff/.../source/EU_CN_US/Trunk"
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"

touch X2Lib/stdafx.cpp                      # after ANY header edit
msbuild X2Lib/X2Lib_2010.vcxproj -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
msbuild X2/X2_2010.vcxproj       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
ls -la X2/US_SERVICE/X2.exe                 # judge by the artifact, not the exit code
cp X2/US_SERVICE/X2.exe "$DATA/X2_offline.exe"
cd "$DATA" && ./start_offline.bat

grep -E "UNHANDLED|EXCEPTION" offline_packets.log | sort | uniq -c | sort -rn
grep "AIPARTY" offline_server.log
sqlite3 els_db.sql "select unit_uid, nickname, level, exp, ed from unit;"
```

## The failure modes to expect, and what each looks like

| Symptom | Cause |
|---|---|
| Dungeon hangs at 80% loading | A bot slot reached `NetworkProcess()` without `m_bIsPvpNpc` — three unreachable P2P peers |
| Bots spawn but at wrong stats, no HUD, immortal-looking | Positive NPC UID, so `IsPvpBot()` is false. Fix the create-req handler |
| Sub-stage never clears, camera waits forever | Bots spawned on a team other than `TN_RED`; `LiveActiveNPCNum()` counts them |
| Bots do not appear at all, or appear as placeholders | An NPC id whose `.lua` is not in this client's `.kom`. Phase 0 lists the 21 that are; anything outside that list must be probed the same way before it is used |
| **Bots appear on the normal start button** | The solo path reached `MakePartyBots`, or the solo handler was edited "temporarily". `m_kRoom.m_vecBot` being empty is the entire scope guard. Out of scope by design — see *Scope* |
| **Bots spawn, chase and swing, and monsters take no damage** | `KNPCUnitReq::m_cAllyTeam` left at its `Init()` default of `TN_MONSTER`, so `CX2DamageManager` treats every monster as the bot's own side ([X2DamageManager.cpp:1347](X2Lib/X2DamageManager.cpp#L1347)). Spawn through `CreateNPCReq`, which defaults it to `TN_NONE`; `PushCreateNPCReq` never sets it. Phase 1 defect 2 |
| **Nothing happens at all, and no log line says why** | The code was hung off a `CX2Game` handler the dungeon does not call. Verify the caller is on the dungeon path, and log the early-return branches. Phase 1 defect 1 |
| Bots play an entrance animation on every stage change | Normal — a stage change destroys every NPC and the bot is rebuilt. Force it past its lua `START` state at creation. Phase 1 defect 3 |
| **Twice as many bots as slots, all the right heroes** | A spawn was requested more than once inside the several frames between `CreateNPCReq` and the NPC actually existing. `GetNPCUnitByUID( uid ) == NULL` does **not** mean “not asked for” — the create is a packet round-trip. Guard with the in-flight timer in `CreateOfflinePartyBots`. Phase 2 defect 1 |
| **Bots fan out correctly in one stage and stack on one spot in another** | `GetLandPosition` returned its far-away fallback — the same one for every input — because no line was loaded under the point yet. Accept a snap only if it lands near what was asked for. Phase 2 decisions |
| **`AIPARTY … never arrived after 3s`** | A spawn request the offline server answered never produced an NPC. This was phase 2 defect 2 — a stale spawn position at a stage change, silently deleted by `CreateNPC` — and it is fixed twice over: the position now comes from the line map's start slots, and there is no per-stage spawn left at all. The line reappearing means something regressed |
| **The party window will not open again, and the P key does nothing** | `SetProcessDungeonMatch` is still latched from an auto-party that was queued and never closed. Only three packets clear it — the cancel ACK, `EGS_AUTO_PARTY_CLOSE_NOT` and `EGS_PARTY_GAME_START_NOT` — and each belongs to a different exit. Phase 3 correction 4 |
| **Empty party HP bars linger after a dungeon clear** | `EndOfflinePartyBots` did not run. It hangs off `EGS_END_GAME_DUNGEON_RESULT_DATA_NOT`; `grep "stood down" offline_server.log` says whether it fired. Phase 4 defect 1 |
| **`AIPARTY … never arrived` repeating once a second at the END of a run** | Same cause: the tick is still tending a party the dungeon has finished with, deleting and re-requesting into a world with no sub-stage left. Every one of those requests is answered normally, so `offline_packets.log` shows nothing wrong — only `offline_server.log` does. Phase 4 defect 1 |
