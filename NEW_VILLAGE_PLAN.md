# Adding a new village, its dungeons, its fields, and any new monster types (offline mode) — phase plan

## Context

This is a phased integration plan for adding a brand-new, full-featured village to the offline client (`SERV_IRUHADEV_OFFLINE`, branch `mods/offline-mod-2`) — a village with its own dungeons and battlefields, same as the existing 8 (Ruben, Elder, Besma, Altera, Peita, Velder, Hamel, Sander). It follows the same shape as `MARCH_2014_MIGRATION.md`/`OFFLINE_MODE_PLAN.md`: read it before starting work, and treat each phase as something you can hand to a **fresh Claude Code conversation** on its own, using the "Start this conversation with" prompt at the end of that phase's section. A fresh session should not need to re-derive anything already settled here — it just needs to read this file plus whatever files earlier phases produced or the user is supplying.

**Scope:** offline-mode only. No real GameServer/CenterServer changes, no VS2003 rebuild — the servers are unbuildable in this environment anyway, and (per *Why this is safe* below) the offline "server" for every subsystem here — village entry, dungeon entry, battlefield entry — is compiled into the same binary as the client.

**Starting materials:** the user has, from a newer Elsword client build: the village hub's world Lua script, mesh/skin/particle assets, and line-map; **and** the same for its dungeon-gate map, dungeon-lounge map, and every field map; **and** brand-new monster types (not reuses of existing `NPC_UNIT_ID`s) for the dungeons/fields, including their meshes. This is **wiring work, not content authoring** anywhere in this plan — nothing here requires `X2WorldTool`/`X2LineMapTool`/a modeling tool.

**Content shape:** the village should have multiple dungeons and/or fields (Hamel/Sander-style), not just one of each. Every phase below describes **one repeatable pattern per dungeon / per field / per monster** — run it once per item, not once total.

**Naming convention used throughout:** replace `<NAME>` with the new village's short identifier (e.g. `ARIEL`), `<DUNGEON>` with a per-dungeon identifier, `<FIELD>` with a per-field identifier, and `<MONSTER>` with a per-monster identifier — matching the existing pattern (`WI_VILLAGE_RUBEN`, `DI_EL_FOREST_WEST_NORMAL`, `VMI_BATTLE_FIELD_RUBEN_FIELD_01`, `NUI_BEE`). Pick names once and use them consistently across every phase and every enum/Lua reference — do not let a name drift between the enum and the Lua that references it, or the pieces won't link up.

## Why this is safe (read once, applies to every phase)

- `WORLD_ID` (`X2Lib/X2World.h`) is a client-only enum. No `.cpp` in this tree references any `WI_*` value by name — it exists purely as a Lua-facing symbol, populated at runtime via `CX2WorldManager::AddWorldData_LUA` (`X2WorldManager.cpp:170`). This applies equally to the village hub, its dungeon gate, its dungeon lounge, and every field — each is its own `WORLD_ID` with its own world Lua script and line-map.
- `SEnum::VILLAGE_MAP_ID`, `LOCAL_MAP_ID`, `BATTLE_FIELD_ID` (`KncWX2Server/Common/Enum/Enum.h:196-335`) and `SEnum::DUNGEON_ID` (`KncWX2Server/Common/Enum/DungeonEnum.h:5-488`) are declared in headers shared with the real servers. Normally a shared-header change means both client and server must be rebuilt together (this repo's own contract rule) — but every subsystem this plan touches (village entry, dungeon entry, battlefield entry) is emulated entirely inside `X2Lib/Offline/`, built as part of the client. There is nothing on the other side of this wire that isn't in the same binary, so skipping a server rebuild here doesn't create the drift the rule warns about.
- **`DUNGEON_ID` and `BATTLE_FIELD_ID` entries are explicit numeric literals, not auto-incremented** (confirmed: `DungeonEnum.h:487`, `DI_END = 41311`) — unlike `VILLAGE_MAP_ID`/`LOCAL_MAP_ID`, which auto-increment and must be inserted right before their `_END` sentinel. A new dungeon or field can take any unused number without any renumbering risk at all.
- `NPC_UNIT_ID` (`X2Lib/X2UnitManager.h:60-2557`) is a ~1900-entry enum with an explicit ceiling comment (keep `NUI_MAX_LIMIT` under 65536) — appending new values before that sentinel is safe and has plenty of headroom.
- **Every subsystem's client-facing data is loaded purely from Lua, with no additional C++ needed**, once the enum values exist:
  - Village: `CX2LocationManager::OpenScript` (`X2Lib/X2LocationManager.cpp:129-143`) loads `DLG_Map_World.lua`/`DLG_Map_Local.lua`/`DLG_Map_Village.lua`/`DLG_Map_House.lua`; `VillageTemplet`/`HouseTemplet`/`LocalMapTemplet` (`X2Lib/X2LocationManager.h`) are populated purely from that Lua.
  - Dungeon: `CX2DungeonManager::AddDungeonData_LUA` (`X2WorldManager.cpp` sibling, `X2DungeonManager.cpp:77-101`) reads `DungeonData.lua`; each dungeon's own stage script is parsed by `CX2DungeonSubStage::SubStageData::LoadData` (`X2DungeonSubStage.cpp:1298`).
  - Battlefield: `CX2BattleFieldManager::AddBattleFieldData_LUA` (`X2BattleFieldManager.cpp:325,339`) reads `BattleFieldData.lua`; each field's own script calls `AddSpawnMonsterGroup`.
  - Monster: `CX2Data::ResetUnitManager` (`X2Data.cpp:1790`) reads `UnitTemplet.lua`/`NPCTemplet.lua`/`NPCStat.lua`; the monster's own gameunit script (named by `NPCTemplet.lua`'s `LUA_FILE_NAME` field) is parsed by `CX2GUNPC::InitState` (`X2GUNPC.cpp:4498`) for its FSM and mesh attachment.
- **The offline entry handshakes for all three are already fully generic** — no per-ID whitelist anywhere, so **no offline-mode C++ changes are needed** for any of this:
  - Village: `Handler_EGS_STATE_CHANGE_FIELD_REQ`/`Handler_EGS_FIELD_LOADING_COMPLETE_REQ` (`X2Lib/Offline/Handlers_Field.cpp:62-164`).
  - Dungeon: `OpenRoom`/`BuildStageNpcData`/the full stage-load chain (`X2Lib/Offline/Handlers_Room.cpp:482,1669-1879,1902-2149`) — a missing dungeon script fails cleanly with `"no dungeon script for ID=%d"` (`:1688`) rather than hardcoding anything.
  - Battlefield: `Handler_EGS_JOIN_BATTLE_FIELD_REQ` and the respawn/danger-value tick (`Handlers_Room.cpp:1241-1328,891-998`).
- **One confirmed gap, not a blocker:** there is no offline-mode-specific loader for monster stat/mesh data (unlike `X2OfflineStatTable`), because `NPCTemplet.lua`/`NPCStat.lua`/`UnitTemplet.lua` load once at client startup via the same client-native path real gameplay always used (`X2StateStartUp.cpp:186`), not per-dungeon like the `X2Offline*` loaders. This should work without any offline-specific code — but treat it as something to verify in Phase 8, not assume.
- Drop/reward data for new monsters goes into files that **already exist as source** in this tree — `KncWX2Server/ServerResource/US/DropTable.lua`, `StaticDropTable.lua`, `NpcExpTable.lua` — so that piece is pure addition, no unseen-file merge risk. `BattleFieldServerData.lua` (also already in-tree) needs **no edit at all** — it's global tuning shared across every field, not per-field data.
- `Enum.h`, `DungeonEnum.h`, `X2World.h`, and `X2UnitManager.h` all carry CP949 (Korean) bytes in their comments. Every edit to any of them must be followed by the encoding gut-check this repo's `CLAUDE.md` mandates: `file <path>` should report the same encoding as before, and `git diff --stat` should show a handful of changed lines, not the whole file. If it doesn't, `git checkout -- <path>` (confirm with the user first) and redo the edit at the byte level.

## Phase 0 — Handoff: gather every source file

**Goal:** get every asset this integration needs into a known location before any code changes, for the village hub, its dungeon gate/lounge, every dungeon, every field, and every new monster.

**Inputs needed from the user:**

*Maps (one world Lua script + one line-map + referenced mesh/skin/particle assets, per item):*
1. Village hub.
2. Dungeon gate map.
3. Dungeon lounge map.
4. Each field map (one per `<FIELD>`).
   (Individual dungeon *stage* maps are usually described inside the dungeon's own stage script rather than as separate standalone world maps — confirm this against the actual files once in hand; if a dungeon does use distinct per-stage world maps, treat each the same as a field map above.)

*Location/identity Lua — ideally the whole current file for each, not a diff, since none exist as source in this tree and merging blind against a file nobody can see risks the same mistake this repo's `CLAUDE.md` warns about elsewhere (the `ItemTemplet.xlsx`-vs-live-DB story):*
5. `DLG_Map_World.lua`, `DLG_Map_Local.lua`, `DLG_Map_Village.lua`, `DLG_Map_House.lua` — as in the village-only version of this plan. (A structurally-identical reference copy of the `Village{...}` shape already lives at `KncWX2Server/GameClient/DLG_Map_Village.lua`.)
6. `DungeonData.lua` (master dungeon registry) — one new entry per `<DUNGEON>`.
7. Each `<DUNGEON>`'s own per-instance stage script (`STAGE_LIST`/`STAGE`/`DUNGEON_MAP`/`SUB_STAGE`/`NPC_GROUP`).
8. `BattleFieldData.lua` (master field registry) — one new entry per `<FIELD>`.
9. Each `<FIELD>`'s own monster-spawn script (`AddSpawnMonsterGroup` calls).
10. `UnitTemplet.lua`, `NPCTemplet.lua`, `NPCStat.lua` — one new entry per `<MONSTER>` in each.
11. Each `<MONSTER>`'s own gameunit script (FSM + mesh/animation attachment).

*Optional:*
12. New village's `MapData:AddMapData{...}`/`AddLocalMapInfo(...)` lines, if Cobo Express fast-travel should reach it. `KncWX2Server/ServerResource/US/MapData.lua` already exists as source — only the new lines are needed.

**Not needed from the user** (already in this tree, only new lines get added): `KncWX2Server/ServerResource/US/DropTable.lua`, `StaticDropTable.lua`, `NpcExpTable.lua`. **Never needs editing:** `BattleFieldServerData.lua`.

**Exit criteria:** every file above is saved somewhere the next phases can read it, and every `<NAME>`/`<DUNGEON>`/`<FIELD>`/`<MONSTER>` identifier is decided and written down once (not re-decided per phase).

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then help me with Phase 0: I'm adding a new village named `<NAME>` with dungeons `<DUNGEON1>, <DUNGEON2>, ...`, fields `<FIELD1>, <FIELD2>, ...`, and new monsters `<MONSTER1>, <MONSTER2>, ...`. Here are the files I have from a newer client build: [list what you're attaching / where they are]. Tell me if anything on the Phase 0 checklist is still missing before we move to Phase 1.

## Phase 1 — Enum and flag wiring (C++)

**Goal:** give every new identity (village, gate, lounge, each dungeon, each field, each monster) a symbol the rest of the build can reference.

**Steps:**
1. Before touching any enum file, confirm how `WORLD_ID` names reach Lua (grep for a `WORLD_ID`-equivalent of `CX2OfflineLuaEnum`, which is confirmed to publish `VILLAGE_MAP_ID` to Lua by reading `Enum.h` directly — see `X2Lib/Offline/X2OfflineMapData.h`'s header comment). Don't assume `DLG_Map_World.lua`/`DungeonData.lua`/`BattleFieldData.lua` can reference a `WORLD_ID` by symbol until this is confirmed; they may need a raw integer instead.
2. Add one flag to `KTDXLIB/Always.h`, using the house comment block:
   ```c
   //////////////////////////////////////////////////////////////////////////
   // Author: Iruha
   // Date: <today>
   // Description: New village <NAME> (+dungeons/fields/monsters) - enum/identity wiring
   #define SERV_IRUHADEV_VILLAGE_<NAME>
   //////////////////////////////////////////////////////////////////////////
   ```
   (One flag for the whole village's worth of new content is enough — this is one logical change, per this repo's "one flag per logical change" rule. Split into sibling flags only if some pieces genuinely need independent revert points.)
3. `X2Lib/X2World.h`, `WORLD_ID` enum — append, all guarded by the flag: `WI_VILLAGE_<NAME>`, `WI_DUNGEON_GATE_<NAME>`, `WI_DUNGEON_LOUNGE_<NAME>`, and one per field/stage map if Phase 0 confirmed those need distinct world IDs.
4. `KncWX2Server/Common/Enum/Enum.h`:
   - `VILLAGE_MAP_ID` — `VMI_<NAME>` immediately before `VMI_VILLAGE_MAP_END`; `VMI_DUNGEON_GATE_<NAME>` in the `DUNGEON GATE` block; `VMI_DUNGEON_LOUNGE_<NAME>` in the `DUNGEON LOUNGE` block — all currently the last entry in their block, so appending doesn't renumber anything after it.
   - `LOCAL_MAP_ID` — `LMI_<NAME>` after `LMI_SANDER` (currently last).
   - `BATTLE_FIELD_ID` — `BFI_<NAME>_<N>` per field, any unused explicit number before `BFI_END`.
   All guarded by the flag.
5. `KncWX2Server/Common/Enum/DungeonEnum.h`, `DUNGEON_ID` — `DI_<NAME>_<DUNGEON>_<DIFFICULTY>` per dungeon×difficulty combo, any unused explicit number before `DI_END = 41311`. Guarded by the flag.
6. `X2Lib/X2UnitManager.h`, `NPC_UNIT_ID` — `NUI_<MONSTER>` per new monster, before `NUI_MAX_LIMIT`, staying under the documented 65536 ceiling. Guarded by the flag.
7. Run the encoding gut-check from *Why this is safe* on every edited header (`X2World.h`, `Enum.h`, `DungeonEnum.h`, `X2UnitManager.h`).

**Files touched:** `KTDXLIB/Always.h`, `X2Lib/X2World.h`, `KncWX2Server/Common/Enum/Enum.h`, `KncWX2Server/Common/Enum/DungeonEnum.h`, `X2Lib/X2UnitManager.h`.

**Exit criteria:** every enum value from the list above exists and is guarded; encoding checks pass on all four edited headers.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 1 for village `<NAME>` (dungeons: `<...>`, fields: `<...>`, monsters: `<...>`). Add the `SERV_IRUHADEV_VILLAGE_<NAME>` flag to `Always.h`, then every enum value Phase 1 lists across `X2World.h`, `Enum.h`, `DungeonEnum.h`, and `X2UnitManager.h`, all guarded by that flag. First confirm how `WORLD_ID` names reach Lua before assuming a symbol lookup works. Run the CP949 encoding check from this repo's `CLAUDE.md` on every header you edit.

## Phase 2 — Village Lua integration

**Goal:** merge the village hub's own data into the four location-manager Lua files.

**Steps:**
1. Merge the village's `AddWorldData` entry into `DLG_Map_World.lua`.
2. Merge its `LocalMapTemplet` grouping into `DLG_Map_Local.lua` — at this point, also add `m_DungeonList` (the dungeon IDs from Phase 1) and `BattleFieldIDList`/`BattleFieldRestData` (the field IDs from Phase 1); see Phases 3-4 below for the rest of what those subsystems need.
3. Merge its `LocationManager:Village{ VillageMapID = VILLAGE_MAP_ID["VMI_<NAME>"], BaseLocalStateID = LOCAL_MAP_ID["LMI_<NAME>"], START_POS = {...} }` block into `DLG_Map_Village.lua`, using `KncWX2Server/GameClient/DLG_Map_Village.lua` as a formatting reference.
4. Merge its houses/NPCs into `DLG_Map_House.lua`, making sure every house has a `m_vecCommonPos` entry for `WI_VILLAGE_<NAME>` specifically — required, or the assert in `X2TFieldGame.cpp:3186` fires.

**Files touched:** the four `DLG_Map_*.lua` files.

**Exit criteria:** all four files reference the same `<NAME>` IDs consistently, with the village linking to every dungeon and field ID from Phase 1.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 2 for village `<NAME>` (enum values already added in Phase 1). Here are the four location Lua files and the village's source data: [attach/point to Phase 0's outputs]. Merge the village's entries into all four, including its dungeon list and battlefield list in `DLG_Map_Local.lua`, cross-checking every ID against Phase 1.

## Phase 2b — Cobo Express wiring (optional)

**Goal:** make the village reachable via paid fast-travel.

**Steps:** add the village's `AddMapData`/`AddLocalMapInfo` lines to `KncWX2Server/ServerResource/US/MapData.lua`.

**Exit criteria:** entry present; `CX2OfflineMapData::CheckCOBOExpressTicketMapID` returns a non-zero zone for the village's map ID once built (verified in Phase 8).

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 2b for village `<NAME>`: add its Cobo Express entries to `KncWX2Server/ServerResource/US/MapData.lua`, matching the shape already used for the other 8 villages.

## Phase 3 — Dungeon integration

**Goal:** register each new dungeon and give it real stage content.

**Steps (repeat per `<DUNGEON>`):**
1. Merge a new entry into `DungeonData.lua` — `DUNGEON_ID` (from Phase 1), `m_DataFileName` (the per-instance stage script's filename), level/item requirements, boss drop list, ending speech.
2. Add the dungeon's per-instance stage script (from Phase 0) to the game data — `STAGE_LIST`→`STAGE<i>`→`DUNGEON_MAP`→`SUB_STAGE<j>`, each with `CLEAR_COND`/`NEXT_STAGE`, `PORTAL_LINE_INDEX`, `START_POS`, and `NPC_GROUP` referencing the monster IDs from Phase 1 (see Phase 5 — the monsters referenced here must already exist as `NPC_UNIT_ID`s before this phase's content can spawn anything real).
3. Confirm the dungeon already appears in `DLG_Map_Local.lua`'s `m_DungeonList` for this village (added in Phase 2) — if using an unlock chain between this village's own dungeons, populate `m_vecRequireClearDungeonID` too.

**Files touched:** `DungeonData.lua`, each dungeon's stage script, (cross-check only) `DLG_Map_Local.lua`.

**Exit criteria:** every `<DUNGEON>` has a `DungeonData.lua` entry, a stage script, and is reachable from the village's gate list.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 3 for village `<NAME>`'s dungeons: `<DUNGEON1>, <DUNGEON2>, ...` (IDs already added in Phase 1, linked from the village in Phase 2). Here are `DungeonData.lua` and each dungeon's stage script: [attach/point to Phase 0's outputs]. Merge each dungeon's registration and stage content, double-checking every `NPC_GROUP` monster reference matches a Phase 1 `NUI_` symbol.

## Phase 4 — Battlefield integration

**Goal:** register each new field and give it real monster-spawn content.

**Steps (repeat per `<FIELD>`):**
1. Merge a new entry into `BattleFieldData.lua` — `BATTLE_FIELD_ID`/`VMI_BATTLE_FIELD_<NAME>_<N>` (from Phase 1), `NPC_TABLE_FILE_NAME` (the field's own monster-spawn script filename).
2. Add the field's monster-spawn script (from Phase 0) — `AddSpawnMonsterGroup` calls referencing monster IDs from Phase 1 (same dependency on Phase 5 as dungeons above).
3. Confirm the field already appears in `DLG_Map_Local.lua`'s `BattleFieldIDList`/`BattleFieldRestData` for this village (added in Phase 2).
4. Do **not** edit `BattleFieldServerData.lua` — it's shared global tuning, not per-field data.

**Files touched:** `BattleFieldData.lua`, each field's monster-spawn script, (cross-check only) `DLG_Map_Local.lua`.

**Exit criteria:** every `<FIELD>` has a `BattleFieldData.lua` entry, a monster-spawn script, and is reachable from the village's field list.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 4 for village `<NAME>`'s fields: `<FIELD1>, <FIELD2>, ...` (IDs already added in Phase 1, linked from the village in Phase 2). Here are `BattleFieldData.lua` and each field's monster-spawn script: [attach/point to Phase 0's outputs]. Merge each field's registration and spawn content, double-checking every monster reference matches a Phase 1 `NUI_` symbol. Confirm `BattleFieldServerData.lua` is left untouched.

## Phase 5 — New monster type integration

**Goal:** give every new `NPC_UNIT_ID` real stats and a real mesh/FSM.

**Steps (repeat per `<MONSTER>`):**
1. Merge a new entry into `UnitTemplet.lua` (whatever that file's role is for this `NPC_UNIT_ID` — confirm against the user's copy; report back if it turns out unnecessary for monster-type NPCs specifically, since its exact scope wasn't nailed down in this conversation's research).
2. Merge a new entry into `NPCTemplet.lua` — `NPC_ID` (from Phase 1), `CLASS_TYPE`, name/desc, and `LUA_FILE_NAME` naming the monster's own gameunit script.
3. Merge a new entry into `NPCStat.lua` — `MAX_HP`, `ATK_PHYSIC`/`ATK_MAGIC`, `DEF_PHYSIC`/`DEF_MAGIC`, elemental defenses, `SCAN_RATE`, `ACCURACY`/`AVOIDANCE`, keyed by the same `NPC_ID`.
4. Add the monster's own gameunit script (from Phase 0) — its `INIT_STATE` FSM and mesh/animation attachment tables (`MESH_PLAYER`, `ATTACH_MESH_NAME`), parsed by `CX2GUNPC::InitState`.
5. Add the monster's rows to the already-in-tree `KncWX2Server/ServerResource/US/DropTable.lua` (keyed by `(DungeonID/BattleFieldID, MonsterID)`), `StaticDropTable.lua` (keyed by dungeon/field ID alone, if this monster contributes a static/guaranteed drop), and `NpcExpTable.lua` (keyed by NPC level) — these three do not need to be sourced from the user, only edited here.

**Files touched:** `UnitTemplet.lua`, `NPCTemplet.lua`, `NPCStat.lua`, each monster's gameunit script, `DropTable.lua`, `StaticDropTable.lua`, `NpcExpTable.lua`.

**Exit criteria:** every `<MONSTER>` has a templet entry, a stat entry, a gameunit script, and drop/exp rows.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 5 for monsters `<MONSTER1>, <MONSTER2>, ...` (IDs already added in Phase 1, referenced from Phase 3/4 spawn tables). Here are `UnitTemplet.lua`, `NPCTemplet.lua`, `NPCStat.lua`, and each monster's gameunit script: [attach/point to Phase 0's outputs]. Merge each monster's registration, stats, and mesh/FSM script, then add its rows to the in-tree `DropTable.lua`/`StaticDropTable.lua`/`NpcExpTable.lua`.

## Phase 6 — Build

**Goal:** produce a client binary with the new village, its dungeons, its fields, and its monsters all compiled in.

**Steps:**
1. `touch X2Lib/stdafx.cpp` — `Always.h`, `Enum.h`, `DungeonEnum.h`, and `X2UnitManager.h` all sit inside the precompiled header, and msbuild does not reliably notice header-only edits otherwise.
2. Run the standard `US_SERVICE` build order from `CLAUDE.md`'s *Toolchains* section (`luajitLib` → `luaLib` → `X2ServerProtocol` → `KTDXLIB` → `X2Lib` → `X2`, each with `-p:SolutionDir=.../Trunk/` and `-m -p:MultiProcessorCompilation=true`).

**Exit criteria:** `X2/US_SERVICE/x2.exe` is freshly dated and the build produced zero errors.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 6: build the client with village `<NAME>`'s changes from Phases 1-5(+2b) included, following the `US_SERVICE` build recipe in this repo's `CLAUDE.md`. Confirm `X2/US_SERVICE/x2.exe`'s timestamp updated.

## Phase 7 — Iterate with loose files, then verify

**Goal:** confirm the village, its dungeons, its fields, and its monsters all actually work in-game before committing to a repack.

**Steps:**
1. Deploy `X2/US_SERVICE/x2.exe` to the game directory as `X2_offline.exe` (verify the copy landed by name/size/mtime, don't just assume it).
2. Drop every edited/new file from Phases 2-5 as **loose files** in the game directory — this build loads a loose file before the `.kom` archive, so no packing/XOR step is needed yet.
3. Run `start_offline.bat`, then play through each surface in turn:
   - **Village entry** — via `DLG_Map_Village.lua`'s `START_POS`/`DLG_Map_Local.lua`'s grouping. Check `offline_packets.log` for `HANDLED` `EGS_STATE_CHANGE_FIELD_REQ`/`EGS_FIELD_LOADING_COMPLETE_REQ`.
   - **Each dungeon** — enter via the village's dungeon gate, run at least one full stage. Check for `HANDLED` on the room-open and `EGS_DUNGEON_STAGE_LOAD_*` chain, and that `NPC_GROUP` monsters actually spawn.
   - **Each field** — enter via the village's field access. Check for `HANDLED` `EGS_JOIN_BATTLE_FIELD_REQ` and that `AddSpawnMonsterGroup` monsters spawn and respawn.
   - **Each monster** — confirm it renders with the correct mesh, animates through its FSM, and has sane combat stats (HP drains at a sane rate, deals damage) — this is the item flagged as unverified in *Why this is safe* (no offline-specific loader for `NPCTemplet.lua`/`NPCStat.lua`), so don't skip it.
4. `grep -E "UNHANDLED|EXCEPTION" offline_packets.log` should have no new hits tied to any of this village's IDs; `offline_server.log` should show no assert (e.g. the `X2TFieldGame.cpp:3186` NPC-position assert) or missing-file line.

**Exit criteria:** village, every dungeon, every field, and every monster all work, with clean logs.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 7 for village `<NAME>`: deploy the Phase 6 build, drop all of Phases 2-5's files as loose files, and play through the village, every dungeon (`<...>`), every field (`<...>`), and every new monster (`<...>`) in turn. Read `offline_packets.log`/`offline_server.log` after each and report exactly what they show — including whether monster stats/mesh actually took effect, since that path has no offline-specific loader to fall back on.

## Phase 8 — Pack and ship

**Goal:** move from loose-file testing to the real, packed artifacts.

**Steps:**
1. Confirm which `.kom` archive number houses each file category for the existing 8 villages/dungeons/fields/monsters (not visible from source in this tree — check against how those files are currently packed, or ask the user). There may be more than one archive involved now (location Lua, dungeon/field scripts, and unit/monster Lua may not all live in the same `.kom`).
2. XOR-encrypt and pack every finalized file from Phases 2-5 into its correct archive (per this repo's standing rule: this is the user's step to run, not something to band-aid around with a permanent loose copy).
3. Delete every loose copy dropped in Phase 7.
4. Re-run Phase 7's full verification checklist against the packed build only — a loose-file success does not prove every pack succeeded, and a miss on any one file surfaces as `KEM_ERROR136` in the logs rather than a build error.

**Exit criteria:** everything from village entry through every dungeon, field, and monster works identically with loose copies removed; no `KEM_ERROR136` lines tied to this village's files.

**Start this conversation with:**
> Read `NEW_VILLAGE_PLAN.md` in full, then do Phase 8 for village `<NAME>`: confirm which `.kom` archive(s) should hold each category of new file, pack them (or confirm the user has packed them — name the exact files and ask them to confirm packing per this repo's `CLAUDE.md` rule if not done yet), delete the Phase 7 loose copies, and re-verify against `offline_packets.log`/`offline_server.log` with the packed build only.

## Risks / open items carried across phases

- **Struct/table drift.** The source client's Lua (location files, dungeon/field data, `NPCTemplet.lua`/`NPCStat.lua`) is from a newer build than this tree's C++ parsers. If any file uses fields this build's parser doesn't recognize, the parse may silently ignore them or error outright — read the relevant parser (`VillageParsing`/`HouseParsing` in `X2LocationManager.cpp:566-568`, `CX2DungeonManager::AddDungeonData_LUA`, `CX2BattleFieldManager::AddBattleFieldData_LUA`, `AddNPCTemplet_LUA`/`AddNPCStat_LUA` in `X2UnitManager.cpp`) once each real file is in hand, rather than assuming compatibility.
- **`UnitTemplet.lua`'s exact role for monster-type NPCs is not fully pinned down** by this conversation's research — Phase 5 flags this as something to confirm once the actual file is in hand, rather than guessing its shape in advance.
- **Monster stat/mesh load path is unverified in offline mode specifically** (no dedicated `X2Offline*` loader exists for it) — Phase 7 makes this an explicit checklist item rather than an assumption.
- **Archive naming/packing convention** for every new file category isn't visible from source in this tree — resolve in Phase 8 only; no earlier phase should block on it, since Phases 2-7 all work against loose files first.
- **Dependency order matters across phases**, since monsters (Phase 5) are referenced by both dungeon stage scripts (Phase 3) and field spawn scripts (Phase 4) — do Phase 5 before or alongside Phases 3-4, not strictly after, if a fresh conversation is tackling them out of order; cross-check `NPC_UNIT_ID` symbols carefully either way.
