# AI party members in offline dungeons

**Status:** planned, nothing implemented. Written 2026-09-05.
**Depends on:** `SERV_IRUHADEV_OFFLINE` (all nine phases done — see
`OFFLINE_MODE_PLAN.md`).
**Proposed flag:** `SERV_IRUHADEV_AI_PARTY`, defined in `KTDXLIB/Always.h` per
the rule at the top of `CLAUDE.md`.

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
| Spawn | [X2Game.cpp:8093-8117](X2Lib/X2Game.cpp#L8093), inside `CX2Game::Handler_EGS_PLAY_START_NOT` — **base class, not `CX2PVPGame`** |
| "This NPC is a bot" | [X2GUNPC.cpp:2151-2157](X2Lib/X2GUNPC.cpp#L2151) and [:2529-2535](X2Lib/X2GUNPC.cpp#L2529) — `m_UnitUID < -1 && g_pX2Room->IsNpcSlot( m_UnitUID )` |
| Stat override | [X2GUNPC.cpp:3915-3921](X2Lib/X2GUNPC.cpp#L3915) — a bot's HP/atk/def come from its `RoomNpcSlot`, not from the NPC stat table |
| Team HP bars | `InsertPvpMemberUI` / `UpdatePvpMemberGageData` ([X2Game.cpp:6438](X2Lib/X2Game.cpp#L6438), [X2PVPGame.cpp:176](X2Lib/X2PVPGame.cpp#L176)) |
| Revive | [X2Game.cpp:6174-6205](X2Lib/X2Game.cpp#L6174) — `RebirthUserUnit` already takes a bot branch |
| The characters | `NUI_PVP_BOT_ELSWORD / _AISHA / _RENA / _RAVEN / _EVE / _CHUNG` ([X2UnitManager.h:1258-1263](X2Lib/X2UnitManager.h#L1258)) |
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

All three are in `CX2Game`, the base class both `CX2DungeonGame` and
`CX2PVPGame` derive from. All three are one `#ifdef SERV_IRUHADEV_AI_PARTY`
each.

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
  `NUI_PVP_BOT_*` in the widened spawn loop instead. The class is already on
  every slot, and this keeps the mod client-only.

Then `Handler_EGS_NPC_UNIT_CREATE_REQ`
([Handlers_Room.cpp:1613](X2Lib/Offline/Handlers_Room.cpp#L1613)) must hand a
bot spawn its **room-slot UID** rather than the next value of
`m_kRoom.m_iNextNpcUID`. Recognise it by `m_NPCID` being one of the six
`NUI_PVP_BOT_*`, and pop the next unused negative UID from the room's bot list.
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
moveset is whatever `NUI_PVP_BOT_<char>.lua` gives it — a scripted approximation
of that character, not the player's own skill tree. That is what KOG shipped
these NPCs to be.

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

1. Build a throwaway diagnostic behind `SERV_IRUHADEV_AI_PARTY_DEBUG`: after
   unit-manager load, call `GetNPCUnitInfo()` for all six ids and log id, name,
   `m_LuaFileName`, and whether an NPC stat row exists, via
   `CX2OfflineLog::Server( L"AIPARTY  ..." )`.
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

---

# Phase 1 — One bot, in a solo dungeon

Smallest thing that proves the pipeline. Keep auto-party refused; use the
existing quick-start path.

1. **`KTDXLIB/Always.h`** — define `SERV_IRUHADEV_AI_PARTY` with the house
   comment block. `touch X2Lib/stdafx.cpp` afterwards (`pch-hides-header-edits`).
2. **`Handlers_Room.cpp`** — `MakeRoomSlots` grows an optional bot count.
   `MAX_SLOT` in the anonymous namespace goes from 1 to 4, and `MakeRoomInfo`'s
   `m_JoinSlot` follows the real occupancy. Add `MakeBotRoomUserInfo( class,
   level, nickname, botUID, OUT KRoomUserInfo& )` next to `MakeRoomUserInfo`:
   `m_bIsPvpNpc = true`, `m_nUnitUID = botUID` (at most -2), `m_cUnitClass`,
   `m_ucLevel`, `m_wstrNickName`, and a `MakeGameStat`-derived `m_kGameStat`. No
   equipped items, no skill data, no IP/port — a bot has no P2P identity and
   `Set_KRoomSlotInfo` skips `ResetEqip()` for it anyway.
   Record the bot UIDs on `m_kRoom` for the create-req handler.
3. **`Handlers_Room.cpp`** — `Handler_EGS_NPC_UNIT_CREATE_REQ` returns the
   room's bot UID when `m_NPCID` is a `NUI_PVP_BOT_*`, and does **not** write
   `m_mapNpcLevel` / `m_mapNpcID` for it (those price kill rewards).
4. **`X2Game.cpp` gate 1** — widen the `DeleteNpcSlot()` condition to also fire
   when `GetGameType() == GT_DUNGEON`, keeping the original `PCC_OFFICIAL`
   branch in `#else`.
5. **`X2Game.cpp` gate 2** — in the spawn loop, when `GT_DUNGEON`: derive the
   NPC id from the slot's unit class, spawn on `CX2Room::TN_RED` with
   `CX2NPCAI::NAT_ALLY` and `m_iAllyUID = GetMyUnit()->GetUnitUID()`, and take
   the start position from the player's own start position offset sideways, not
   from `GetBlueTeamStartPosition`. Note `DeleteNpcSlot` has already run by the
   time this loop executes, so `m_cUnitClass` must be captured into
   `RoomNpcSlot` first — add a field to that struct (a client-side struct, not a
   packet, so this is free).

Exit test:

- Enter any normal dungeon solo. One AI character spawns beside you on your
  team, follows you, and attacks monsters.
- Sub-stages still clear. If they do not, the bot is not on `TN_RED`.
- `offline_packets.log` has no new `UNHANDLED` and no `EXCEPTION`.
- The dungeon still finishes loading — no 80% hang. If it hangs, a bot slot is
  reaching `NetworkProcess()` without `m_bNpc` set.

---

# Phase 2 — Three bots, placement, death and revive

1. Three bots, classes chosen to complement the player's (never a duplicate of
   the player's class unless all six are exhausted).
2. Spawn placement: fan out along the player's start line via
   `CKTDGLineMap::GetLineData`, the way the field spawner in
   `X2OfflineBattleField.cpp` already picks positions.
3. Level: match the player's level, so `MakeGameStat` gives a bot the stats a
   real party member of that level would have.
4. Death and revive. `CX2Game::RebirthUserUnit` already takes the bot branch
   ([X2Game.cpp:6174](X2Lib/X2Game.cpp#L6174)); confirm what sends it in a
   dungeon and whether `CX2Game::IsAllUserDead()`
   ([X2Game.cpp:6108](X2Lib/X2Game.cpp#L6108)) counts bots — if it does, a wiped
   party of bots would end the run while the player is alive.
5. **Verify the reward path is not polluted.** A dying bot must not produce
   EXP/ED through `Handler_EGS_NPC_UNIT_DIE_REQ`
   ([Handlers_Room.cpp:1692](X2Lib/Offline/Handlers_Room.cpp#L1692)); step 3 of
   phase 1 is what prevents it. Confirm with a run:
   `sqlite3 els_db.sql "select unit_uid, exp, ed from unit;"` before and after.

Exit test: three bots fight a full dungeon start to finish, die and come back,
and EXP/ED for the run matches a solo run of the same dungeon.

---

# Phase 3 — Wire it to the auto-party button

Replace the refusal at
[Handlers_Social.cpp:498](X2Lib/Offline/Handlers_Social.cpp#L498). The client's
own auto-party flow expects, in order:

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
jump, and it exercises the cancel path.

Decide and record: does offline auto-party always fill to 4, or offer a count?
Default to 4.

Exit test: press auto-party in the village, watch it queue and match, land in
the dungeon with three bots. Cancel mid-queue and end up back in the village
with the UI reset.

---

# Phase 4 — Make them look like party members

Cosmetic, and the phase that decides whether this feels like a party or like
three pets.

1. **Gate 3** — widen the `GT_PVP` HUD branch at
   [X2Game.cpp:6437](X2Lib/X2Game.cpp#L6437) so a dungeon bot gets
   `InsertPvpMemberUI` / `UpdatePvpMemberGageData` too. Cheapest route to
   on-screen HP bars, but it uses the PvP team-list widget.
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

---

# Phase 5 — Tuning and honesty

- Bot difficulty. `RoomNpcSlot`'s five stats are the only knob and the offline
  server owns them outright — scale them off `CX2OfflineStatTable` with a factor
  that is one named constant in `X2Lib/X2Define.h`, the way
  `SERV_IRUHADEV_BASE_MP_REGEN_PER_SEC` is.
- `CX2AllyNPCAI` is Lua-tuned (`LoadAIDataFromLUA`): `allyLostRange`,
  `targetRange`, `dashRange`, jump rates. If those need changing, the file is
  the studio's and the packing is the user's job — name it and stop
  (`server-lua-packing-is-users-job`).
- Update `MODS.md` with the flag row and a description.
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

## The four failure modes to expect, and what each looks like

| Symptom | Cause |
|---|---|
| Dungeon hangs at 80% loading | A bot slot reached `NetworkProcess()` without `m_bIsPvpNpc` — three unreachable P2P peers |
| Bots spawn but at wrong stats, no HUD, immortal-looking | Positive NPC UID, so `IsPvpBot()` is false. Fix the create-req handler |
| Sub-stage never clears, camera waits forever | Bots spawned on a team other than `TN_RED`; `LiveActiveNPCNum()` counts them |
| Bots do not appear at all, or appear as placeholders | Phase 0 was skipped and `NUI_PVP_BOT_*` has no templet in this client's `.kom` |
