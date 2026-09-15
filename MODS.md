# Local modifications

The `import-2014` commit is **not** stock 2014 code. It is stock code plus four
changes that were already applied to the working copy before git existed here,
plus mods added afterward -- see the *Since* column below. No pristine copy
exists on disk, so the import could not be split into a vanilla baseline plus
mods.

This file is the substitute for that baseline. Keep it current.

## Applied mods

The first four rows were already applied to the working copy before git existed
here; later rows are marked with the date they were added, in the *Since*
column.

| Flag | Since | Defined in | Client files | Server files |
|---|---|---|---|---|
| `SERV_IRUHADEV_SKILLTREE_NO_LOCK` | import-2014 | `KTDXLIB/Always.h:2414`, `KncWX2Server/Common/ServerDefine.h:4210` | `X2Lib/X2UISkillTreeNew.cpp:364` (added 2026-08-27), `X2Lib/X2UISkillTreeNew.cpp:5788` | `KncWX2Server/GameServer/UserSkillTree.cpp:1489` |
| `SERV_IRUHADEV_SKILL_SLOT_B_FREE` | import-2014 | `KTDXLIB/Always.h:2421`, `KncWX2Server/Common/ServerDefine.h:4217` | `X2Lib/X2UISkillTree.h:228`, `X2Lib/X2UISkillTreeNew.h:269`, `X2Lib/X2UserSkillTree.h:307`, `X2Lib/X2UserSkillTree.cpp:2078` | `KncWX2Server/GameServer/UserSkillTree.cpp` (lines 10, 766, 978) |
| `SERV_IRUHADEV_BUFF_DURATION_TEXT` | import-2014 | `KTDXLIB/Always.h:2428` | `X2Lib/X2GageUI.{h,cpp}`, `X2Lib/X2BuffTemplet.{h,cpp}`, `X2Lib/X2BuffFinalizerTemplet.h`, `X2Lib/X2GameUnit.cpp`, `X2Lib/X2PremiumBuffManager.cpp` | -- |
| `STATIC_AUTO_LOGIN` | import-2014, **retired 2026-09-09** | -- | -- | -- |
| `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` | 2026-08-27 | `KTDXLIB/Always.h:2435`, `KncWX2Server/Common/ServerDefine.h:4224` | `X2Lib/X2Unit.h:982-988`, `X2Lib/X2Unit.cpp:104-111`, `X2Lib/X2UIQuickSlot.cpp:1797-1822`, `X2Lib/X2CashShop.cpp:8306-8326` | `KncWX2Server/GameServer/Inventory.cpp:500-534`, `KncWX2Server/GameServer/Inventory.cpp:146-152` |
| `SERV_IRUHADEV_OFFLINE` | 2026-08-31 | `KTDXLIB/Always.h:2446` **and** `X2ServerProtocol/X2ServerProtocol_2010.vcxproj:1444` (`US_SERVICE` `PreprocessorDefinitions`); AI party tuning constants in `X2Lib/X2Define.h:1822` | `X2Lib/Offline/` (the whole directory: 55 sources plus `start_offline.bat`), plus seams in `X2ServerProtocol/Socket/Session.cpp` (5 blocks), `X2ServerProtocol/OfflineHook.h`, `X2Lib/X2Data.cpp:2135`, `X2Lib/X2StateServerSelect.cpp:6982`, `X2Lib/X2DungeonSubStage.cpp:1452`, `X2Lib/X2QuestManager.{h,cpp}`, `X2Lib/X2TitleManager.{h,cpp}`, `X2Lib/X2StateBeginning.cpp:1607`; and, for the AI party, `X2Lib/X2Game.h:611`, `X2Lib/X2Game.cpp:195`, `:6541`, `:6986`, `:8886`, `:8958`, `:13802`, `X2Lib/X2DungeonGame.cpp:182`, `:1304`, `:2104`, `X2Lib/X2Room.h:304`, `:332`, `X2Lib/X2Room.cpp:1087`, `X2Lib/X2GageManager.cpp:3623`, `X2Lib/X2GageUI.h:633`, `X2Lib/X2Data.cpp:2930` | -- |
| `SERV_IRUHADEV_NO_PATCHER_TOKEN` | 2026-09-04 | `KTDXLIB/Always.h:2525` | `X2/X2.cpp:805` | -- |
| `SERV_IRUHADEV_JOBCHANGE_PORTRAIT` | 2026-09-04 | `KTDXLIB/Always.h:2513` | `X2Lib/X2UIQuestNew.cpp:8`, `X2Lib/X2UIQuestNew.cpp:1516`, `X2Lib/X2UIQuestNew.cpp:2043` | -- |
| `SERV_IRUHADEV_MP_REGEN_BOOST` | 2026-09-04 | `KTDXLIB/Always.h:2538` (rate constant in `X2Lib/X2Define.h:1802`) | `X2Lib/X2GUUser.cpp:1829`, `X2Lib/X2GUUser.cpp:3654`, `X2Lib/X2GUUser.cpp:3726`, `X2Lib/X2GageManager.cpp:50` | -- |
| `SERV_IRUHADEV_AIPARTY_PERSIST` | 2026-09-06 | `KTDXLIB/Always.h:2560` (nested under `SERV_IRUHADEV_OFFLINE`) | `X2Lib/X2Game.h:660-675`, `X2Lib/X2Game.cpp:197-199`, `X2Lib/X2Game.cpp:4958-4999`, `X2Lib/X2Game.cpp:7234` (`GetOfflinePartyBotPos`), `X2Lib/X2Game.cpp:7306` (`IsOfflinePartyBotUID`), `X2Lib/X2Game.cpp:7347` (`RepositionOfflinePartyBots`), `X2Lib/X2DungeonGame.cpp:685-707`, `X2Lib/X2DungeonGame.cpp:878-885` | -- |
| `SERV_IRUHADEV_LEVEL_CAP_80` | 2026-09-08 | `KTDXLIB/Always.h:2638` (redefines the studio's `USE_MAXLEVEL_LIMIT_VAL` from `KTDXLIB/OnlyGlobal/Always_US.h:75`) | `X2Lib/X2Game.h:34` and `X2Lib/X2UIPersonalShopBoard.h:12` pick it up by macro expansion; compile-time check in `X2Lib/Offline/Handlers_Room.cpp:2711` | -- |
| `SERV_IRUHADEV_OFFLINE_FETCH_AURA_ALWAYS` | 2026-09-08 | `KTDXLIB/Always.h:2672` (nested under `SERV_IRUHADEV_OFFLINE`) | `X2Lib/Offline/Handlers_Social.cpp:1541` (the force, in `MakePetInfo`), `:1574` (`IsPetPastCrystalStage`), `:1620` (the per-pet log line), `X2Lib/Offline/Handlers_Inventory.cpp:872` (item 500720 refused), `X2Lib/Offline/X2OfflineServer.h:490` | -- |
| `SERV_IRUHADEV_FIX_CHAR_SELECT_DELETE_BUTTON` | 2026-09-11 | `KTDXLIB/Always.h:3684` | `X2Lib/X2StateServerSelect.cpp:11097-11103` (`CreateUnitButtonNew`) | -- |
| `SERV_IRUHADEV_PENDING_DELETE_UNIT_MENU` | 2026-09-11, amended 2026-09-16 (twice) | `KTDXLIB/Always.h:3728` (nested under `SERV_UNIT_WAIT_DELETE`) | `X2Lib/X2StateServerSelect.cpp:1407` (down-state reset), `:2210`, `:3203`, `:3319`, `:3434` (immediate confirm on select), `:11196`, `:11234`, `:11303`; the first 2026-09-16 amendment also touches `X2Lib/Offline/X2OfflineDB.h:401-419,512`, `X2Lib/Offline/X2OfflineDB.cpp:1119-1145,1272-1285`, `X2Lib/Offline/Handlers_Unit.cpp:420-425,552-556` | -- |

What each one does:

- **`SERV_IRUHADEV_SKILLTREE_NO_LOCK`** -- both skills in a 2-choice skill tree
  row can be learned; the unchosen one is no longer locked out. Also hides the
  "AbleChoice" select-indicator picture on the left-hand skill slot, since
  that overlay implied an either/or choice that no longer exists.
- **`SERV_IRUHADEV_SKILL_SLOT_B_FREE`** -- Skill Slot B is open by default and
  permanent. No medal purchase, and the server keeps the permanent-sentinel end
  date rather than expiring it.
- **`SERV_IRUHADEV_BUFF_DURATION_TEXT`** -- draws a remaining-seconds countdown
  over each buff/debuff icon on the status HUD.
- **`STATIC_AUTO_LOGIN`** -- retired during the March 2014 migration (Phase 1,
  2026-09-09). It used to skip the login screen and sign in with a fixed
  account. The studio's own `AUTO_LOGIN_IN_HOUSE` went live under
  `US_INTERNAL` for the first time in that migration and does the same job
  data-driven, by reading a `LoginKey.lua` next to the exe - see *Known
  deviations to clean up* below.
- **`SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`** -- all 6 equipped consumable quick
  slots are open from character creation. No cash ticket, no `LOCK` overlay,
  and the Quick Slot Expansion ticket (item 244560, plus the Ara/Elesis
  variants) is hidden from the cash shop. The server pins inventory category
  11 (`ST_E_QUICK_SLOT`) to exactly 6 slots on load, so a legacy `+3` purchase
  row already sitting in `dbo.GItemInventorySize` no longer stacks on top of
  the new base.
- **`SERV_IRUHADEV_OFFLINE`** -- makes the client playable with no servers
  running. `KSession` hands every outbound packet to an in-process emulator
  (`CX2OfflineServer`) instead of a socket, and the emulator answers on the
  same session at the point the real receive path queues into, so everything
  above the socket layer runs unmodified. All player state persists to a local
  SQLite file, `els_db.sql`, in the game directory. Client-only: no packet
  struct, event ID or shared enum is touched, so the servers do not need
  rebuilding. See `OFFLINE_MODE_PLAN.md` for the design and the phase history,
  and the *Offline mode* section of `CLAUDE.md` for the three seams.

  **It also fills a dungeon party with AI characters, and only through the
  auto-party button.** Pressing auto-party queues, matches and opens the
  dungeon with three of the game's named hero NPCs on the player's team -
  drawn at random, placed on the line map's own party start slots, fighting
  with ally AI, revived when they go down, kept across a stage change, and
  given HP/MP bars, portraits, names and levels in the real party HUD. The
  ordinary start button is untouched and still goes in alone: the entire
  scope guard is that a solo room sends no bot slots, so every client-side
  branch is a no-op on that path. An auto-party run pays exactly the same EXP
  as a solo one. The party members are `CX2GUNPC`s using the PvP-bot slot
  mechanism the studio already shipped, so a bot's moveset is its own scripted
  one rather than a player's skill tree, and no P2P peer is added for it.
  Their difficulty is three named constants in `X2Lib/X2Define.h`, applied in
  `CX2Game::SetUserSummonedNPCInfo`. See `AI_PARTY_PLAN.md` for the design and
  the six phases.

  **This flag has to be defined in two places.** `X2ServerProtocol` does not
  include `KTDX.h`, so `KTDXLIB/Always.h` alone does not reach the socket seam:
  it must *also* be in `X2ServerProtocol_2010.vcxproj`'s `US_SERVICE`
  `PreprocessorDefinitions`. Set only one and nothing fails to compile --
  `g_pX2OfflineHook` is declared in a header whose flag-conditional body the
  two projects then disagree about, and the client goes back to trying to
  reach a real server. Toggle both together, always.
- **`SERV_IRUHADEV_NO_PATCHER_TOKEN`** -- lets `x2.exe` be started directly
  instead of only through a launcher that passes the patcher token. A
  `_SERVICE_` build compares `argv[1]` against `PATCHER_RUN_ONLY` and returns
  0 out of `WinMain` if it does not match, so a bare launch (double-click, a
  debugger, a shortcut) exited instantly with no window, no message box and no
  log line. The flag supplies that same constant at the call site rather than
  reading it out of `argv`; the two tests are left standing and the token is
  still accepted when passed, so a launcher or `start_offline.bat` keeps
  working unchanged. Independent of offline mode, and useful without it.

  Verified by probe, not by reading `#ifdef`s: in `US_SERVICE` this is the
  **only** live `argv` read in the client. The other nine
  (`X2.cpp:756-778`, `X2Main.cpp:1083`, `1292-1301`, `1438-1460`, `1478`) are
  all behind flags that are off -- `LAUNCHER_COMMAND_ARGUMENT`,
  `CLOSE_ON_START_FOR_GAMEGUARD`, `CLIENT_PURPLE_MODULE*`,
  `SERV_CHANNELING_AERIA`, `SERV_COUNTRY_PH`, `_NEXON_KR_`, `ARGUMENT_LOGIN`,
  `SERV_STEAM`. Several of them read `__argv[1]`/`[2]` with no NULL guard, so
  that mattered.
- **`SERV_IRUHADEV_JOBCHANGE_PORTRAIT`** -- refreshes the top-left gage
  portrait when a quest changes the character's class. The quest path resets
  the 3D square unit but never the gage, so the HUD kept drawing the old class
  until the gage was rebuilt on a state change; every other class-change path
  in the client already refreshes it.
- **`SERV_IRUHADEV_AIPARTY_PERSIST`** -- the offline AI party survives a stage
  change instead of being rebuilt at every one of them. Requires
  `SERV_IRUHADEV_OFFLINE` and is defined under it.

  `CX2DungeonGame::StageLoading` calls `DeleteAllNPCUnit()`, which used to
  sweep the three bots away with the stage's monsters; `CreateOfflinePartyBots`
  then had to spawn all three again at `SubStageStart`, which costs a packet
  round trip plus three `CX2GUNPC` constructions and lands after the loading
  curtain has already lifted -- so the player fought alone for a moment at
  every stage and then watched three heroes pop in.

  A real multiplayer party never pays that: `CX2GUUser` units are built once in
  `CX2Game::UnitLoading` and `StageLoading` merely repositions them. This flag
  gives a bot the same lifetime. `DeleteAllNPCUnit` spares a *living* party bot
  while `m_bOfflineKeepPartyBots` is set (only around that one call), the way it
  already spares monster-card summons, and `RepositionOfflinePartyBots` places
  the survivors on the new stage's line map beside the user-unit loop. A dead
  bot is deliberately not spared -- letting it go means the ordinary spawn path
  rebuilds it whole on the new stage, which is a free revive while the stage is
  loading anyway.

  Reverting it restores per-stage respawning, which still works; the placement
  helper `GetOfflinePartyBotPos` is shared by both paths and is not gated.

- **`SERV_IRUHADEV_MP_REGEN_BOOST`** -- quality of life: raises the base MP
  regeneration rate for player units from 1 MP/s to
  `SERV_IRUHADEV_BASE_MP_REGEN_PER_SEC` (50 MP/s), the single tuning knob, in
  `X2Lib/X2Define.h`.

  The base rate is not a constant in the shipped client -- it is
  `MP_CHANGE_RATE` out of each class's Lua, read into
  `CX2GUUser::m_fOriginalMPChangeRate` at both of the two places that load it
  (`InitAndInsertToGame` and `InitComponent`) and then pushed into the gage by
  `ResetMPChangeRate()`. The flag raises the value at those two load points
  only, as a **floor** rather than an assignment, which is what keeps it from
  breaking anything downstream:

  - everything that composes on top of the base still composes -- Aisha's
    Meditation, Rena's charge states, Eve's passives, socket
    `m_fSpeedUpManaGather`, title and drag-and-set mana-recovery options all
    read `GetOriginalMPChangeRate()` and add to or scale it;
  - the PvP play-channel scale (x7 in `ResetMPChangeRate`, /7 in
    `GetNowMPChange`) is unaffected, since it wraps the base either way;
  - a class whose Lua already asks for more than the floor keeps its own
    higher value.

  Two things it deliberately does **not** touch. The socket-item MP option
  (`SetMPChangeRateValue`, `KSocketData::m_iMPChangeValue`) is a separate
  additive path and is left at its item-driven value. And the two absolute
  `ResetMPChangeRate( 3.f )` / `( 10.f )` calls behind Eve's `SMI_EVE_MEMO5`
  skill note (`X2Lib/X2GuEve.cpp:8665`) overwrite the base outright while that
  memo is equipped, so Eve regenerates *slower* than the floor in that one
  state. Both are class/item behaviour rather than the general base rate, which
  is what this flag is scoped to.

  NPC and monster MP is untouched -- `CX2GUUser` is the player-unit class. In
  online play MP is client-simulated per unit and P2P-synced, so a remote
  player running an unmodded client will regenerate at their own rate.

  **The village map needed a second site.** `XS_VILLAGE_MAP` does not regen MP
  through the unit's change rate at all. `CX2GageManager::OnFrameMove` runs a
  fixed 1.0 s timer (`m_ElapsedTimeCheckVillageBuff`) that calls
  `CX2GageSet::UpNowHpAndMpInVillage()`, which does a hardcoded
  `UpNowMp( 1.f )` straight onto the HUD's own `CX2GageData` -- the my-gage-set
  has no game unit attached in town, so `UpdateGageDataFromGameUnit()` degrades
  to a self-copy and the unit's rate never reaches the bar. That is the only
  hardcoded periodic MP tick in the client (every other literal `UpNowMp` is
  hit- or skill-driven), and it is why the first cut of this flag looked like
  it had done nothing while idle in a village but clearly worked in a dungeon.
  The tick is now worth one second of the base rate; since the period is
  exactly 1.0 s the constant goes in unscaled.

  **And dungeons and fields needed a third site, which is the one that
  mattered.** Raising the base rate had no effect there at all, and the reason
  is a stock bug: `CX2GUUser::InitComponent` sets the rate from the class Lua
  near the top, then further down -- in the `default` branch of the game-type
  switch, i.e. everything that is not PvP and not the training room -- does

  ```cpp
  CX2GageManager::GetInstance()->RestoreGageData();
  m_pGageData = CX2GageManager::GetInstance()->GetMyGageData()->GetCloneGageData();
  ResetMaxHP();
  ResetMaxMP();
  SetNowHp( ... ); SetNowMp( ... );
  ```

  The clone replaces the **whole** `CX2GageData`, so the MP change rate set
  minutes earlier in the same function is thrown away. The four calls after it
  restore the maxima and the current values; nothing restores the rate, and the
  clone carries the gage manager's, which no code ever sets -- it is the `1.0f`
  default out of `CX2GageData::Gage::Init()`. So in stock code a class's
  `MP_CHANGE_RATE` is dead in dungeons and fields, and every character
  regenerates at exactly 1 MP/s there regardless of what its Lua asks for. It
  survives only in the PvP and training branches, which do not re-clone.

  The flag re-applies `ResetMPChangeRate( GetOriginalMPChangeRate() )`
  immediately after that `ResetMaxMP()`. With the flag off the line is gone and
  stock behaviour is byte-for-byte unchanged, bug included.

  **This was found by measuring, not by reading.** Three rounds of static
  analysis got it wrong, because every writer of the rate looked correct in
  isolation -- the value was right when set and the thing that destroyed it
  does not mention MP at all. A temporary `SERV_IRUHADEV_MP_REGEN_DEBUG` flag
  logging one `[MPDBG]` line per second into `offline_server.log` settled it in
  one play-test: `InitComponent` logged `rateMp=20.000`, every per-frame sample
  a second later logged `rateMp=1.000` with `orig=20.000` still intact, which
  localises the loss to the gage object rather than the value. That asymmetry
  -- member right, gage wrong -- is also what made the class states look like
  they were working while idle regen was not: Aisha and Rena recompute from
  `GetOriginalMPChangeRate()`, which reads the member the clone never touched.
  See the *Deploying the offline client* section of `CLAUDE.md` for the
  diagnostic pattern.

### Offline fetch aura on every pet

**Defined in** `KTDXLIB/Always.h` -- `SERV_IRUHADEV_OFFLINE_FETCH_AURA_ALWAYS`,
nested under `SERV_IRUHADEV_OFFLINE`. `QUALITY_OF_LIFE.md` #7.

Every pet that is past its crystal reports the fetch aura (the pet item-pickup
skill, `PET_DROP_ITEM_PICKUP`) as already unlocked, so cash item 500720 never
has to be bought. The force is one line in `CX2OfflineServer`'s `MakePetInfo`,
which is the only builder of a `KPetInfo` the offline server ever sends -- the
pet list, the create ACK, the summon ACK and its relayed `_NOT`, and the feed
ACK all pass through it, and all eight client sites that read `m_bAutoLooting`
read one of those.

**The save is not touched.** `unit_pet.auto_looting` keeps whatever was actually
purchased, so undefining the flag restores the bought toggle exactly, with no
migration. Item 500720 is refused while the flag is on
(`Handlers_Inventory.cpp:872`, `ERR_PET_28`, item left in the bag) rather than
consumed to write a column nothing reads any more; it stays listed in the cash
shop, because `X2OfflineCashSeed.h` is a verbatim transcription of
`dbo.EB_Product` and a refusal that names its reason is more readable than a
missing product.

"Except the pet still in crystal" is the game's own test, not a guess:
`PetTemplet.lua`'s `PET_STATUS` is `0` for a crystal step (the loader's own
comment at `X2Lib/X2PetManager.cpp:1337` reads *0: egg, 1: juvenile, 2: other
(adult, perfect form)*), the pet window disables the aura button on the same
entry (`X2UIPetInfo.cpp:2019-2021`), and the live server's `ERR_PET_27` gate
means the same thing. `CX2OfflineServer::IsPetPastCrystalStage` asks
`CX2PetManager::GetPetStatus`, which already folds "no templet" and "step out of
range" into `0`. A pet born fully grown carries `PET_STATUS = { 3 }` and so is
covered, which is the live server's `IsEvolutionExceptionPet` case for free.

To include crystal-stage pets as well, drop the `IsPetPastCrystalStage` test in
`MakePetInfo` -- the pickup itself would work (`X2GUUser.cpp:2874` is not gated
on pet status), but the pet window's aura button stays greyed out for them, so
the UI and the behaviour would disagree.

### Offline "any difficulty" quest and title steps

**Gated by** `SERV_IRUHADEV_OFFLINE`; no new flag. `ISSUES_2.md` #2, phase 30.

Six one-line reads added to two client script parsers, so `m_bUpperDifficulty`
-- the only "any difficulty" mechanism this build has -- is actually loaded:

| file | clear type | what it fixes |
|---|---|---|
| `X2Lib/X2QuestManager.cpp:2453` | `SQT_VISIT_DUNGEON` (26) | 88 of 121 sub-quests |
| `X2Lib/X2QuestManager.cpp:2493` | `SQT_FIND_NPC` (27) | 39 of 47 sub-quests |
| `X2Lib/X2QuestManager.cpp:2574` | `SQT_ITEM_USE` (22) | neither of its 2 sets the flag; parity only |
| `X2Lib/X2TitleManager.cpp:587` | `TMCT_DUNGEON_TIME` | 5 of 17 sub-missions |
| `X2Lib/X2TitleManager.cpp:611` | `TMCT_DUNGEON_RANK` | 9 of 32 sub-missions |
| `X2Lib/X2TitleManager.cpp:634` | `TMCT_DUNGEON_DAMAGE` | none of its 11 sets it; parity only |

`REFORM_QUEST` is on, so `ClearCondition` has no difficulty field and no "any"
sentinel: difficulty is the last digit of the dungeon ID, and "any difficulty"
means `m_bUpperDifficulty == true` plus the Normal ID in `m_setDungeonID`, read
as "this dungeon at difficulty >= that digit". `CX2OfflineQuest::IsExistDungeonInSub`
already honoured the flag correctly -- nothing ever set it, because the client
never needed it. **Offline, the client's parse *is* the server's**, and the
server's `CXSLQuestManager` / `CXSLTitleManager` read the key at exactly these
six places. Each addition matches the server's line and its placement, which
brings the client to the server's own count exactly: 11 reads in the quest
parser and 6 in the title parser, where it had 8 and 3.

Because sub-quest groups are staged (`CheckBeforeGroup`), one stuck group-0
`VISIT_DUNGEON` step also blocks the later `DUNGEON_CLEAR_COUNT` step in the
same quest -- which is why a whole quest read as Normal-only even though its
clear-count branch parsed the flag correctly.

**`LUA_GET_VALUE_RETURN` hard-fails on a missing key** (`luaLib/KLuaManager.h:143`,
`goto error_proc`), which would drop the templet and, for the quest parser, take
every quest with it. The key's presence was therefore proved rather than
assumed, against the exact script the client loads: `FieldSubQuest.lua` and
`SubTitleMission.lua` were XOR-decrypted out of `data036/` and their Lua 5.1
bytecode disassembled and register-simulated back into the 1,672 sub-quest and
640 sub-mission tables they build. All 121 + 47 + 2 records of the three quest
types carry `m_bUpperDifficulty` inside `m_ClearCondition`; the types that do
*not* carry it are exactly the ones whose parser branch does not read it, on
both sides. The title reads use the non-fatal `LUA_GET_VALUE`, as the server's
do. The disassembler was scratch, not committed; it is a Lua 5.1 chunk reader
plus a register simulator over `SETTABLE`/`SETLIST`, which is all it takes to
turn any of these bind-a-table scripts back into readable rows.

`TMCT_NPC_HUNT` looks like a seventh site -- 4 of its 177 sub-missions set the
flag -- but the **server does not read it there either**, so ignoring it is
shipped behaviour and was left alone.

Three diagnostics went in alongside, all inside `X2Lib/Offline/X2OfflineQuest.cpp`:

- `DungeonReqString()` renders a sub-quest's requirement as `30070 (upperDiff=1,
  ...)`, since neither the packed key nor the flag is readable without the other.
- The per-sub-quest census line (`QUEST CHAIN sub`) now prints it, and the
  aggregate line (`QUEST CENSUS clearType=`) now prints how many reachable steps
  carry `upperDiff=1`. A zero there for 26 or 27 means the read has regressed.
- The three dungeon-filter rejections that were a bare `continue`
  (`OnVisitDungeon`, `OnFindNPC`, `OnUseItem`) now log the requirement and the
  key the clear actually arrived with.

Baseline to compare against, from the run before the change: **1,395 templets,
1,395 visible**, clearType 26 = 205 steps, 27 = 47, and 22 absent entirely. A
templet count below 1,395 means a parse is now failing.

### Offline 3x EXP and 3x drop rate

**Defined in** `KTDXLIB/Always.h` -- `SERV_IRUHADEV_OFFLINE_EXP_BOOST` and
`SERV_IRUHADEV_OFFLINE_DROP_BOOST`, both nested under `SERV_IRUHADEV_OFFLINE`.
**Rates** live together at the tail of `X2Lib/X2Define.h`: `_EXP_RATE`,
`_ED_RATE`, `_QUEST_ITEM_RATE` (all `3.0f`) and `_DROP_DRAWS` /
`_STATIC_DROP_DRAWS` (both `3`). That is the one place to retune -- set a rate
to `1.0f` or a draw count to `1` to switch that half off without touching the
flags.

Quality of life for solo play. Offline has none of the live server's rate
bonuses: party, PC bang, premium, event and channel EXP are all either
display-only or not compiled in offline, and `X2OfflineDropTable.h:99` records
that the four multipliers the real server folds into a drop are hard-coded to
1.0 here. So the curve is the full retail one with nothing on top of it.

Covered: monster EXP and ED, dungeon-clear bonus EXP (for free -- it is 30% of
the run total), quest reward EXP and ED, monster and static/place item drops,
and quest collection items. Not covered, deliberately: cube/box contents
(`X2OfflineRandomItem`), and the display-only bonus fields (`m_iPartyEXP`,
`m_iSocketOptEXP`, `m_nPremiumBonusEXP`, ...) -- filling those would make the
EXP bar disagree with the database.

Two things here are not the obvious implementation, and both matter:

- **EXP is multiplied where the reward is *minted*, not where it is stored.**
  `Handlers_Room.cpp` scales `iEXP` immediately after the battlefield factor
  and before `m_kRoom.m_iRewardEXP` accumulates, because every figure the
  player sees and every figure that is stored derives from that one local: the
  `ApplyDungeonReward` write, the `m_EXPList` number the client adds to its own
  bar as `EGS_NPC_UNIT_DIE_NOT` arrives, the clear bonus, and the result
  screen's `m_nOldEXP` / `m_nEXP`. Multiplying inside `ApplyDungeonReward`
  instead -- the one-line change, and the tempting one -- would triple the
  stored total while still telling the client the unboosted number, and the
  result screen's bar can then animate **backwards**.

- **Drops repeat the draw; they do not scale the probability.**
  `CX2OfflineDropTable::Decide` is a weighted *single pick* over one
  accumulated list, not a per-item coin flip. Tripling `m_fProb` would not give
  3x: real rows already sum to ~82% (`DropTable.lua:1686`), so tripling pins
  the total at 100% -- about 1.2x actual -- and silently makes every case
  listed after the accumulator passes 100 unreachable. Repeating the draw is
  what the studio's own drop-rate event does
  (`KncWX2Server/CenterServer/KDropTable.cpp:1284`, under
  `SERV_ITEM_DROP_EVENT`), and it multiplies the expected item count exactly
  while leaving every rarity ratio alone. The **inner group draw stays at one**
  -- a group is a near-uniform selector (group 1 is eight rows of 12.5), not a
  rarity gate, so repeating it would saturate the group and pin every group win
  to its first item.

  The quest collection-item roll is the exception: that one *is* a true
  per-item roll, so it is scaled directly and clamped to 100.

The monster row and the static (place) row have separate draw counts because
they are different kinds of loot -- the monster row is where gear comes from,
the static row is where the ordinary consumables come from (Aqua has a static
row in nearly every dungeon at 5-10%). Drop `_STATIC_DROP_DRAWS` back to `1`
on its own if the potion clutter gets tiresome.

`offline_server.log` gets a `BOOST` line at startup naming all five values. It
is printed unconditionally -- with the vanilla numbers when the flags are off
-- because the question it answers is "did my rebuild actually take", and a
line that only appears when the boost is compiled in cannot tell a disabled
boost apart from a stale PCH.

Client-only; no server rebuild. Nothing under `KncWX2Server/Common/` is
touched, so the wire format is unchanged. Reverting is safe at any time: only
future gains are scaled, nothing migrates, and the character is untouched.


### Level cap 80

**Defined in** `KTDXLIB/Always.h` -- `SERV_IRUHADEV_LEVEL_CAP_80`. It does one
thing: `#undef`/`#define` the studio's own `USE_MAXLEVEL_LIMIT_VAL` macro, which
`KTDXLIB/OnlyGlobal/Always_US.h:75` sets to `const int g_iMaxLevel = 67;` for
US. The redefinition sits at the tail of `Always.h`, i.e. after the
`OnlyGlobal` include at line 2492 and before any `X2Lib` header is parsed, so
both places that expand the macro pick up 80: `X2Lib/X2Game.h:34`
(`_CONST_X2GAME_`) and `X2Lib/X2UIPersonalShopBoard.h:12`
(`_CONST_UIPERSONALSHOPBOARD_INFO_`). Two headers, one edit, and the studio's
line is left in place as the value the flag replaces.

**This is data the build already shipped**, which is why it is only a constant:

- `ExpTable.lua` (packed in `data036.kom`; decrypts to `luac` with the 12-byte
  XOR key) carries rows all the way to `LEVEL = 80`, `TOTAL_EXP = 986793900` --
  comfortably inside `int`.
- `KncWX2Server/ServerResource/US/StatTable.lua` does
  `ReserveMemory( class, 80 )` and has a `SetUnitStat` row at level 80 for every
  player class, and `CX2OfflineStatTable::MAX_LEVEL` was already 80 to match.
- `SkillData.lua`'s `CalcLevelUpIncreaseSkillPoint` is a formula
  (`level / 10 + 4`), not a table, so `CX2OfflineSkill::SkillPointForLevelUp`
  keeps paying out past 67 without any change.

Everything that reads the cap follows automatically -- the full list is
`Handlers_Room.cpp`'s `ApplyDungeonReward` (stops levelling at it),
`Handlers_Inventory.cpp`'s level-up scroll (refuses at it), `X2Game.cpp:8313`
(the EXP bar's max), `X2InstanceData.cpp:80` (the seed for `m_iMaxLevel`) and
`X2UIPersonalShopBoard.cpp`'s level filter. `X2Define.h`'s `LIMIT_MAX_LEVEL`
(65) is dead -- nothing but comments references it -- and is deliberately left
alone.

`Handlers_Room.cpp` carries a compile-time check next to `ApplyDungeonReward`:
a negative array bound if `_CONST_X2GAME_::g_iMaxLevel != 80` while the flag is
defined. That is aimed squarely at the stale-PCH failure mode -- `Always.h` is
inside every project's precompiled header, so a dropped edit would otherwise
show up as a cap that silently stayed at 67.

A **live** server would need more than this: `SiKGameSysVal()->GetLimitsLevel()`
reads `GameSysValTable.lua`'s `MAXLevel`, which is what the comment beside the
studio's `#define` is warning about. Offline has no `GameSysVal` and never sends
`EGS_UPDATE_MAX_LEVEL_NOT`, so `CX2InstanceData::m_iMaxLevel` just keeps its
`g_iMaxLevel` seed and there is nothing else to change.

Client-only; no server rebuild. Nothing under `KncWX2Server/Common/` is touched.
Reverting is safe for a character at or below 67; a character already past it
keeps its stored level and EXP, but the EXP bar clamps and further gains stop.

### Character deletion: restore-only pending-delete flow

**Defined in** `KTDXLIB/Always.h` -- `SERV_IRUHADEV_FIX_CHAR_SELECT_DELETE_BUTTON`
and `SERV_IRUHADEV_PENDING_DELETE_UNIT_MENU`, both introduced 2026-09-11 and
undocumented until now.

Vanilla US deletion is two-stage under the studio's own `SERV_UNIT_WAIT_DELETE`:
deleting a character reserves it rather than removing it -- it stays listed,
greyed, and recoverable -- and two dedicated buttons on the character-select
dialog, `restore_unit` and `final_delete_unit`, restore it or make the deletion
permanent after a wait period. **Neither button exists in this build's shipped
`DLG_UI_Character_Selection_Back_New.lua`** -- confirmed from the running
client's own `ErrorLog.txt`, which logs a failed `GetControl` for both names at
every character-list load. So on this build, true vanilla can reserve a
deletion but can reach neither restore nor final-delete from the UI at all; the
network flow the studio wired (`Handler_EGS_RESTORE_UNIT_REQ`,
`Handler_EGS_FINAL_DELETE_UNIT_REQ`, `X2StateServerSelect.cpp:10343-10385`) is
real and complete, just unreachable.

`SERV_IRUHADEV_FIX_CHAR_SELECT_DELETE_BUTTON` is an unrelated, pure bug fix:
`CreateUnitButtonNew()` never hid slot control index 5 (`ButtonDeleteUnit`,
`DLG_UI_Character_Selection_Slot_New.lua`'s 6th per-slot button), which
intercepted every click before the real select button, silently dropping it.

`SERV_IRUHADEV_PENDING_DELETE_UNIT_MENU` repurposes that now-hidden button as
a grey pending-delete overlay, and gives restore a UI entry point by way of the
slot click. **Its original 2026-09-11 version was wrong**: clicking a pending
slot opened an Ok/Cancel box whose *Cancel* button fired
`SUSUCM_FINAL_DELETE_UNIT`, permanently destroying the character with no
confirmation of its own. Amended 2026-09-16:

- the slot click now shows a single Ok/Cancel box worded as vanilla's own
  restore confirm (`STR_ID_16106`, the exact call vanilla already makes at
  `X2StateServerSelect.cpp:1687`) and sends `SUSUCM_RESTORE_UNIT_CHECK` on Ok;
  Cancel just closes it. Nothing destructive is reachable from this menu.
- there is deliberately **no final-delete UI** -- a pending character stays
  recoverable indefinitely, matching what true vanilla actually permits on
  this build. `SUSUCM_RESTORE_UNIT` and the two vanilla button call sites are
  left in place, unreachable, so either lights up for free if the shipped
  dialog ever gains the missing controls.
- `X2Lib/Offline/X2OfflineDB.cpp`'s `IsNickNameTaken` used to free a pending
  unit's nickname immediately on soft delete (`del_date = reg_date` filter),
  diverging from vanilla, which holds the name until final delete. With
  restore now the only exit from the pending state, that divergence was a real
  hazard -- delete "Bob", create a new "Bob", and the old "Bob" could never be
  restored again. It now takes an `iExcludeUnitUID` parameter so the name stays
  held while pending, except against the pending unit's own row (restore would
  otherwise refuse itself). One consequence of combining "hold the name" with
  "no final-delete UI": the name is held **permanently** once a character is
  deleted and never restored, since nothing releases it. Acceptable at solo-save
  scale; noted here so it isn't mistaken for a bug later.
- the wait period (`DELETE_WAIT_SECONDS`, `X2OfflineDB.h`) is 5 minutes rather
  than vanilla's hardcoded 1 day (`GSGameDBThread.cpp`'s `iDelableDay = 1` for
  every region). With no final-delete UI this only governs the date shown in
  `STR_ID_16103` and the ack field a final-delete packet would check if one
  ever arrived; it is not something a player waits out on a button.
- `CX2OfflineDB::FinalDeleteUnit`'s child-table cascade (`szTables`) was missing
  four tables added after the list was last extended (`unit_pet`,
  `unit_riding_pet`, `unit_skill_unsealed`, `unit_skill_note`). Fixed, though
  currently latent -- nothing in the UI reaches final delete.

**Corrected 2026-09-16, same day, from play-testing:** the first cut of the
single-confirm slot click above looked like it did nothing. Two separate
defects, both in `X2StateServerSelect.cpp`, both pre-dating this flag rather
than introduced by it:

- **The click only registers on a slot that is already selected.**
  `UnitButtonUp( CX2Unit* pUnit )` only reaches the `m_bDeleted` check inside
  its `if( m_pSelectUnit == pUnit )` branch -- the first click on a pending
  slot takes the *other* branch (a plain reselect: `m_pSelectUnit = pUnit`,
  `CreateServerSelectUnitViewerUI( pUnit )`, no dialog), so from the player's
  side a first click looked exactly like "nothing happened, just highlighted."
  A live character has the same two-click structure, but the first click's
  reselect is a real action there (it swaps which character's model is
  shown); for a pending character it wasn't, so the requirement was more
  visible. Fixed by raising the same restore confirm immediately in that
  reselect branch too, gated on `pUnit->AccessUnitData().m_bDeleted`, closing
  any dialog already open first (`SAFE_DELETE_DIALOG( m_pDLGDeleteUnitCheck )`)
  in case a different pending slot's confirm was still up. The original
  second-click path (`if( m_pSelectUnit == pUnit )`) is left in place and
  still fires if a player clicks an already-selected pending slot again --
  harmless, since re-raising the same confirm on a unit already confirmed
  pending is a no-op state-wise.
- **Multiple pending slots could show "pressed" at once**, unlike a live
  slot, where clicking a new one always released the last one. Root cause:
  `SUSUCM_UNIT_BUTTON_UP`'s `FIX_BUTTON_CRASH` reset loop (`Always.h`,
  studio code, unconditionally on) walks every slot and resets control index
  0's `SetDownStateAtNormal` -- but a pending slot's clickable control is
  index 5 (`ChangeUnitButtonInfo`, this flag), which the loop never touches.
  So a pending slot's pressed state, once set, was never cleared by clicking
  a different slot afterward. Fixed by resetting index 5 in the same loop,
  gated on this flag since index 5 is only a real interactive button under
  it.

**A third defect, found the same day after the above two shipped, is what had
actually been silently defeating both of them:** clicking a pending slot did
nothing at all -- no model swap, no dialog, restore and (the still-hypothetical)
delete both unreachable. Diagnosed with a temporary logging pass
(`SERV_IRUHADEV_PENDING_DELETE_DEBUG`, removed once this was confirmed fixed;
see the *Deploying the offline client* diagnostic pattern in `CLAUDE.md`), which
showed the clicked button's own name had silently become `SLOT_BUTTON_<uid>100`
-- three extra digits nobody wrote.

Root cause: `ChangeUnitButtonInfo` (`X2StateServerSelect.cpp`) names
`pSelectUnitButton` and `pDeleteUnitButton` **identically**,
`SLOT_BUTTON_<uid>`, since both represent the same slot and only one is ever
shown. It used `SetName` for both. `SetName` registers the name in the
dialog's `m_ControlsMap` for `GetControl(L"name")` lookups, and
`CKTDGUIDialog::ReNameControl` -- studio code -- silently disambiguates any
collision by appending a counter starting at **100**. Since
`pSelectUnitButton` claims the name first, `pDeleteUnitButton` collided and
was quietly renamed to `SLOT_BUTTON_<uid>100`. `SUSUCM_UNIT_BUTTON_UP`'s
`tempButtonName.substr(12)` then parsed a UID with that spurious suffix,
matching no real character, so `GetUnitByUID()` returned `NULL` and the
click silently did nothing -- on *every* pending slot, unconditionally,
regardless of the two fixes above.

Nothing looks `pDeleteUnitButton` up by name anywhere (only by index,
`GetControl(5)`), so the fix is `SetNameByForce` instead of `SetName` --
same string, but it only sets the control's own name and never touches the
map or its collision check. Not a workaround: `SetNameByForce` is the
correct call for a control nothing ever looks up by name, and it was
available already (`CKTDGUIControl::SetNameByForce`, used elsewhere in the
engine for exactly this reason).

Client-only; no server rebuild -- the offline emulator files above are part of
`SERV_IRUHADEV_OFFLINE`, not `KncWX2Server/Common/`. Reverting
`SERV_IRUHADEV_PENDING_DELETE_UNIT_MENU` returns to the studio's own dead-end OK
box (`STR_ID_30401`) on a pending slot; the character stays recoverable only by
undeleting through direct DB access, since there is still no in-game
final-delete path either way.


## Reverting to stock

Comment out every `#define` listed in the *Defined in* column and rebuild.
No git operation is needed -- the flag system is the revert switch. That is the
main reason a reconstructed vanilla baseline was not worth building.

Reverting `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` specifically can move player
data: any item sitting in quick slot 4, 5, or 6 falls outside the stock 3-slot
category size, so on next login `KInventory::Init()` routes it through the
same `mapWrongPosItem` path stock code already uses for any item found outside
its category's current bounds -- it is not deleted, just relocated out of the
quick-slot bar.

## Rebuild requirements

`SERV_IRUHADEV_SKILLTREE_NO_LOCK`, `SERV_IRUHADEV_SKILL_SLOT_B_FREE`, and
`SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` all touch
`KncWX2Server/Common/ServerDefine.h`, which compiles into **both** sides.
Toggling any of them requires rebuilding the VS2010 client *and* the five
servers, with the flag set consistently in both. Rebuild only one side and the
wire format desynchronizes silently at runtime instead of failing to compile.

The remaining five -- `SERV_IRUHADEV_BUFF_DURATION_TEXT`,
`SERV_IRUHADEV_OFFLINE`, `SERV_IRUHADEV_NO_PATCHER_TOKEN`,
`SERV_IRUHADEV_JOBCHANGE_PORTRAIT` and `SERV_IRUHADEV_MP_REGEN_BOOST` -- are
client-only. None of them touches anything under `KncWX2Server/Common/`, so
the servers never need rebuilding for any of them. Two caveats:

- `SERV_IRUHADEV_OFFLINE` needs **both** of its definition sites toggled
  together, and all three client libs plus the exe rebuilt (`KTDXLIB`,
  `X2ServerProtocol`, `X2Lib`, then `X2`).
- Toggling anything in `KTDXLIB/Always.h` -- which is every
  `SERV_IRUHADEV_*` flag -- means rebuilding **all four** projects, because
  `Always.h` sits inside each one's precompiled header. msbuild does not
  always notice, and a stale PCH makes the flag simply absent at the call
  site with no error: the `#else` branch compiles and the change appears to
  have done nothing. If in doubt, prove it rather than assume it -- put a
  `#pragma message` inside the `#ifdef` at the call site, rebuild that one
  project, and read the compiler output.

## Known deviations to clean up

**Resolved 2026-09-09 (March migration, Phase 1):** `STATIC_AUTO_LOGIN` used
to break the project rule in CLAUDE.md two ways - it wasn't
`SERV_IRUHADEV_`-prefixed, and it lived in `KTDXLIB/OnlyGlobal/Always_US.h`
(pulled in only for `CLIENT_COUNTRY_US`, so the change was silently US-only)
instead of `KTDXLIB/Always.h` - and it stored a plaintext account password in
a header that compiled into the shipped executable. Rather than fix those
three problems, the flag was retired: the studio's own `AUTO_LOGIN_IN_HOUSE`
went live under `US_INTERNAL` for the first time in this migration and does
the same job (skip the login screen, sign in as a fixed account) by reading
an external `LoginKey.lua`, which has none of the three problems. See
`MARCH_2014_MIGRATION.md`'s "What Phase 1 established" for the full reasoning.
