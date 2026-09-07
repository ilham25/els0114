# Offline Elsword — phases 36-45: "box" items, end to end

`ISSUES_2.md` #7:

> i need you to cover all "box" item (the item that need to be opened)
> functionality because right now i feel like it's half baked. dont make
> assumption and refer on how the real server behaviour does (i can access the
> real live database data when you need something)

Plus a priority the user set while this document was being written: **the
Philosopher's Scroll first**, then everything else.

**Written 2026-09-08, from the tree and from the shipped client scripts.** Every
line reference below was read while writing; check them rather than
re-deriving them. Where a claim came from decrypting a `.lua` out of the game
directory, the recipe is in §0.9 so you can reproduce it.

Four decisions taken with the user, 2026-09-08:

- **Scope is all four families**: openable containers, the hand-in/trade tables
  (`ITEM_EXCHANGE`, `ITEM_CONVERT`), the end-of-dungeon treasure box, and the
  item workshop (`ITEM_MANUFACTURE`).
- **Sealed cubes match live, with the unseal path landing first** — phase 39
  before phase 40. §0.6 explains why this ordering turned out to be load-bearing
  for a different reason than the one it was chosen for.
- **Item 60004276 is refused offline, not transcribed.** It is a real live-server
  bug (§0.2); reproducing it would consume a player's item for nothing, and the
  standing rule that an item consumed for nothing is unrecoverable outranks
  "transcribe, don't tidy" here. Phase 36 owns it.
- This document **supersedes step 4 of phase 34** in
  `OFFLINE_MODE_PHASE29_PLAN.md`, which folded "cover all box item function" in
  as an audit. See §0.8.

---

## §0 — Read this before any phase in this batch

Nine findings. Each one would otherwise cost a phase conversation an hour, and
three of them contradict the obvious assumption.

### 0.1 "Box" is not one system — it is three routes, and the client picks

This is the single most important fact in the document. `CX2UIInventory`'s
right-click path decides which packet a "box" produces, and it checks
`GetCanUseInventory()` **first**
([X2UIInventory.cpp:5917](X2Lib/X2UIInventory.cpp#L5917)):

| route | condition | packet | offline status |
|---|---|---|---|
| **A** | `GetItemTemplet()->GetCanUseInventory() == true` | `EGS_USE_ITEM_IN_INVENTORY_REQ` — the `default:` arm at [`:6896`](X2Lib/X2UIInventory.cpp#L6896) of a ~120-case switch | **consumed, delivers nothing** unless it is a skill-unseal book or the skill note |
| **B** | else, and the item is in the client's random-item registry (`IsRandomItem`, [`:6929`](X2Lib/X2UIInventory.cpp#L6929)) | `EGS_OPEN_RANDOM_ITEM_REQ` | **works** — phase 20 built it properly |
| **C** | else | nothing is sent; the click is dead | n/a |

So the boxes that feel half baked are **route A**, not the cube path everyone
assumes. That one fact explains both halves of the complaint:

- `ISSUES_2.md` #6 ("opening a box empties my ED") is
  `kAck.m_iED = 0` at [Handlers_Inventory.cpp:202](X2Lib/Offline/Handlers_Inventory.cpp#L202),
  never reassigned on any path including success at `:308-314`. The client
  *assigns* that field over the character's ED
  ([X2UIInventory.cpp:8919](X2Lib/X2UIInventory.cpp#L8919)).
- "half baked" is the same handler calling `ConsumeOne` and then doing nothing
  ([Handlers_Inventory.cpp:304-314](X2Lib/Offline/Handlers_Inventory.cpp#L304)).

The cube handler by contrast fills ED on **every** reply, refusals included
([Handlers_Social.cpp:2637](X2Lib/Offline/Handlers_Social.cpp#L2637)) — copy
that shape, not the other one.

**Before writing code for any item, establish which route it takes.** §0.9 is
how.

### 0.2 The live item-use handler is at `GSUserInventory.cpp:3944`. The copy at `:4940` is a trap

`SERV_UPGRADE_SKILL_SYSTEM_2013` is defined
([ServerDefine.h:3749](KncWX2Server/Common/ServerDefine.h#L3749)), so the live
pair is:

- `IMPL_ON_FUNC( EGS_USE_ITEM_IN_INVENTORY_REQ )` — **`:3944`**, the validation switch
- `IMPL_ON_FUNC( DBE_USE_ITEM_IN_INVENTORY_ACK )` — **`:4535`**, the effect switch

The second pair at `:4940` / `:5931` is inside the `#else` **and** wrapped in
`/* … */` (opened at `:4939`, closed at `:6737`). It is dead twice over.

The two copies are *not* identical, which is what makes this dangerous rather
than merely wasteful. `EL_CHAR_LEVEL_UP_ITEM2` (60004276) has a case in the dead
effect switch at `:6066` and **no case** in the live one at `:4655` — so on the
live US server that item passes validation, is consumed, and does nothing.

**Rule for this batch: quote `:3944` and `:4535`. If a line number you are about
to cite is above 4938, you are reading dead code.**

### 0.3 `SERV_*` flag states that matter here

Checked, not assumed:

| flag | state | where |
|---|---|---|
| `SERV_UPGRADE_SKILL_SYSTEM_2013` | **on** | `ServerDefine.h:3749` |
| `SERV_CHAR_LEVEL_UP_ITEM` | **on** | `ServerDefine.h:1285` |
| `SERV_CHAR_LEVEL_UP_ITEM_EVENT_RENA` | **on** (global) | `ServerDefine_Global.h:559` |
| `SERV_SEALED_RANDOM_ITEM` | **on** | `ServerDefine.h:391` |
| `SERV_CUBE_OPEN_ED_CONDITION` | **on** | `ServerDefine.h:3412` |
| `SERV_CUBE_IN_ITEM_MAPPING` | **on** | `ServerDefine.h:94` |
| `SERV_NEW_ITEM_SYSTEM_2013_05` | **on** | `ServerDefine.h:3709` |
| `SERV_USE_ITEM_DB_UPDATE_FIX` | **on** | `ServerDefine.h:1405` |
| `SERV_LIMIT_LEVEL_NOT_USE_ITEM_MSG` | **off** — JP only | `ServerDefine_JP.h:225` |
| `SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT` | **off** | not defined anywhere |
| `SERV_TREASURE_BOX_ITEM` | **off** | not defined anywhere |
| `SERV_OPEN_CUBE_REALTIME_DB_UPDATE` | **off** | not defined anywhere |

Two consequences worth carrying: the US refusal code for a max-level scroll is
`NetError::ERR_UNKNOWN`, not `ERR_NOT_USE_01`; and `CheckCharLevelUp()` takes no
argument in this build.

### 0.4 Nothing in this batch is blocked on packing a file

Unusual for this project, and worth stating because the reflex is to stop and
ask. The game directory has a loose `data036/` beside the `.kom`s (187 files),
`MASS_FILE_FIRST` resolves it, and every table these phases need is already
there. All decrypt with the key in §0.9:

| file | Lua entry point | what it is |
|---|---|---|
| `RandomItem.lua` | `g_pItemManager:AddRandomItemTemplet( id, keyID, keyCount, requiredED )` | the **client's** cube registry — this is what decides route B ([X2ItemManager.cpp:1592](X2Lib/X2ItemManager.cpp#L1592)) |
| `RandomItemTable.lua` (7.9 MB) | `g_pRandomItemManager:AddRandomItemTemplet` / `AddRandomItemGroup` | cube **contents** and odds; already loaded by `CX2OfflineRandomItem` |
| `RandomItemData.lua` | `AddSealRandomItemInfo`, `AddNotifyResultItemInfo`, `AddCharmItem`, `AddAttractionItemInfo` | seal / announce / charm presentation |
| `ItemConvertTable.lua` | `g_pItemManager:AddItemConvertInfo` / `AddItemConvertGroup` — fields `m_ConvertType`, `m_TargetItemID`, `m_ResultItemID`, `m_Commission` | phase 42 |
| `ItemExchangeTable.lua` | `g_pItemManager:AddItemExchangeData` — fields include `m_iSourceQuantity`, `m_iServerGroupID` | phase 41 |
| `ManufactureItem.lua` | the workshop recipes (`CX2ItemManager::ManufactureData`, [X2ItemManager.h:69](X2Lib/X2ItemManager.h#L69)) | phase 44 |
| `PackageItemData.lua` | `g_pCashItemManager:AddPackageItemData` | cash-package contents |
| `AddCubePackageData.lua` | `g_pCX2CubePackageManager:AddCubePackageData` | cube-package grouping |
| `CubeOpenImageInfo.lua` | `g_pCubeOpenImageManager:AddImageInfo_LUA` | the cube-open animation |
| `ItemTrans.lua` | flat `id, name, desc` constant run | ~43,577 item id → live US English name |

`CX2OfflineRandomItem::RunScript` ([X2OfflineRandomItem.cpp:68-95](X2Lib/Offline/X2OfflineRandomItem.cpp#L68))
is the loader shape to copy: `LoadDataFile` → `DoMemory` → `DoMemoryNotEncript`
fallback that logs, and a degrade-if-missing path.

### 0.5 The offline item row already has every column these phases need

`KOfflineItemRow` ([X2OfflineDB.h:50-66](X2Lib/Offline/X2OfflineDB.h#L50))
carries `m_iSealData`, `m_iItemState`, `m_iPeriod`, `m_iUsageType`,
`m_iEnchantLevel`, three attribs and both socket vectors.

**No phase in this batch needs a schema migration.** If you find yourself
writing one, stop and re-read this section — `X2OfflineDB` migrates forward
only, and a migration written by mistake is not reversible.

### 0.6 `m_ucSealData` is one byte holding two different things — and `IS_SEALED` is dead

The obvious reading of "sealed cube" is `CXSLItem::ITEM_STATE::IS_SEALED`. That
is wrong, and following it wastes a phase.

`IS_SEALED` is **declared and never referenced** — one hit in the entire tree,
its own definition at [XSLItem.h:55](KncWX2Server/Common/X2Data/XSLItem.h#L55).
Zero uses in `KncWX2Server/`, zero in `X2Lib/`. Likewise
`KInventory::Seal` / `Unseal` ([Inventory.cpp:3772](KncWX2Server/GameServer/Inventory.cpp#L3772),
`:3789`) are called from nowhere but each other's declaration.

The real mechanism is `KItemInfo::m_ucSealData`
([CommonPacket.h:481](KncWX2Server/Common/CommonPacket.h#L481)), a single
`unsigned char` encoding two facts at once
([CommonPacket.h:556-563](KncWX2Server/Common/CommonPacket.h#L556)):

```cpp
bool IsSealedItem() const { return ( m_ucSealData >= 100 ); }  // >=100 means bound
void UnsealItem()         { m_ucSealData -= 100; }             // unsealing subtracts 100
unsigned char GetSealCount() const                             // the low part is the stamp count
{
    if( m_ucSealData >= 100 )
        return ( m_ucSealData - 100 );
    …
}
```

So: **the hundreds digit is "bound", the remainder is the Phoru-stamp count.**
A cube seals its output by setting `m_ucSealData = 100`
([GSGameDBThread.cpp:3381](KncWX2Server/GameServer/GSGameDBThread.cpp#L3381)),
i.e. "bound, zero stamps".

Two consequences:

1. **A cube-sealed item really is stranded until it is unsealed.** So the
   39-before-40 ordering the user chose is not merely tidy, it is required —
   phase 40 without phase 39 creates unusable player items.
2. `Query_UpdateSealItem` **skips any item with a period**
   ([GSGameDBThread.cpp:3377](KncWX2Server/GameServer/GSGameDBThread.cpp#L3377)):
   `if( mit->second.m_sPeriod > 0 ) continue;`. Rental items out of a cube are
   never sealed. Phases 39 and 40 interact here; neither may ignore it.

The client side agrees: `m_ucTimesToBeSealed = data.m_kItemInfo.GetSealCount()`
([X2Item.h:627](X2Lib/X2Item.h#L627)) and the inventory UI gates on
`GetMaxSealCount() - m_ucTimesToBeSealed`
([X2UIInventory.cpp:7932](X2Lib/X2UIInventory.cpp#L7932), `:8027`).

### 0.7 What is and is not implemented offline today

| event | offline | note |
|---|---|---|
| `EGS_OPEN_RANDOM_ITEM_REQ` | handled ([Handlers_Social.cpp:2616](X2Lib/Offline/Handlers_Social.cpp#L2616)) | route B; fidelity gaps in phase 40 |
| `EGS_USE_ITEM_IN_INVENTORY_REQ` | handled but nearly empty ([Handlers_Inventory.cpp:190](X2Lib/Offline/Handlers_Inventory.cpp#L190)) | route A; phases 36 and 37 |
| `EGS_GET_PURCHASED_PACKAGE_CASH_ITEM_REQ` | handled (`Handlers_Shop.cpp`) | cash packages |
| `EGS_SEAL_ITEM_REQ` / `EGS_UNSEAL_ITEM_REQ` | **IGNORED** ([X2OfflineIgnore.cpp:63-64](X2Lib/Offline/X2OfflineIgnore.cpp#L63)) | phase 39 |
| `EGS_ITEM_EXCHANGE_REQ` | **IGNORED** (`:68`) | phase 41 |
| `EGS_ITEM_CONVERT_REQ` | **IGNORED** (`:67`) | phase 42 |
| `EGS_ITEM_MANUFACTURE_REQ` | **IGNORED** (`:60`) | phase 44 |
| `EGS_SYNTHESIS_ITEM_REQ`, `EGS_SYNTHESIS_SOCKET_GROUPID_REQ` | **neither handled nor ignored** → logs `*** UNHANDLED ***` | phase 45 |
| `EGS_START_REWARD_BOX_SELECT_*` / `EGS_SELECT_REWARD_BOX_*` | stubs ([Handlers_Room.cpp:3228-3249](X2Lib/Offline/Handlers_Room.cpp#L3228)) | phase 43 — **and the live server has no handler either**, see that phase |

**One stale ignore entry to clean.** `EGS_RESOLVE_ITEM_REQ` sits in the ignore
list ([X2OfflineIgnore.cpp:61](X2Lib/Offline/X2OfflineIgnore.cpp#L61)) *and* is
dispatched to a real handler (`Handlers_Inventory.cpp:2236`). Dispatch wins, so
behaviour is right and the list is lying — and an ignore rule is a promise that
nothing is silently dropped. Whichever phase first opens `X2OfflineIgnore.cpp`
deletes that line. The four beside it (`EGS_IDENTIFY_ITEM_REQ`,
`EGS_ITEM_EVALUATE_REQ`, `EGS_RESTORE_ITEM_REQ`,
`EGS_RESTORE_ITEM_EVALUATE_REQ`) were checked and are genuinely unhandled —
leave those alone.

### 0.8 Overlap with `OFFLINE_MODE_PHASE29_PLAN.md`

Phase 34 there owns `ISSUES_2.md` #5 (the zeroed ED) and folds
"cover all box item function" in as its **step 4, an audit**. That step is
superseded by this document; correct it to say so when phase 34 runs.

Phases 34, 35 and 36 all edit `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`. Because
the scroll is the user's priority, and because a level-up cannot be verified
while the same reply is zeroing the wallet, **phase 36 absorbs phase 34's ED fix
for that one handler** (the `FillAckED` helper). Phase 34 then keeps only
`EGS_RESOLVE_ITEM_REQ` and `EGS_ATTRIB_ENCHANT_ITEM_REQ`, and phase 35 (fetch
aura) should run after 36 to avoid a conflict.

### 0.9 Naming an item, and finding its route

Every phase here starts by identifying a real item. Do not pattern-match on
names, and do not use `ScriptData/ItemTemplet.xlsx` — it is a stale KR snapshot
that disagrees with what shipped (`CLAUDE.md`, the live-DB rule).

The scripts in the game directory's `data036/` are Lua 5.1 bytecode
XOR-encrypted with a repeating 12-byte key:

```
02 AA F8 C6 DC AB 47 26 EF BB 00 98
```

XOR the whole file with `key[i % 12]` and you get plain `luac` output starting
`\x1bLuaQ`. A crude constant-pool scan (type byte `3` = 8-byte double, type byte
`4` = `u32` length-including-NUL then bytes) is enough:

- **`ItemTrans.lua`** — a flat `TNUMBER id, TSTRING name, TSTRING desc` run,
  ~43,577 entries. This is how "the Philosopher's Scroll" became `160267`.
- **`RandomItem.lua`** — if the item's ID is **not** among its constants, the
  item is not a cube and cannot take route B. Its only two string constants are
  `AddRandomItemTemplet` and `g_pItemManager`, so a numeric hit is meaningful.
- **`Item.lua`** (34 MB) — the strings around an item's ID constant give its
  icon and its `BFI_BUFF_*` / special-ability names.

When code reading cannot settle a question, add a temporary diagnostic rather
than reasoning further: `CX2OfflineLog::Server( L"..." )` is flushed per line and
available anywhere in `X2Lib`. Gate it behind a short-lived
`SERV_IRUHADEV_*_DEBUG` flag, tag the lines with a grep-able prefix, and remove
it before the phase closes. For this batch the highest-value diagnostic is one
line in `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` printing the item ID,
`GetItemType()`, `GetCanUseInventory()`, `GetNumSpecialAbility()` and
`GetNumBuffFactorPtr()` — it settles §0.1's route question for any item in one
play-test.

---

## §1 — Shared preamble

`OFFLINE_MODE_PHASE9_PLAN.md`'s §1 applies unchanged. One line opens a phase:

> Read §0 and the Phase N section of `OFFLINE_MODE_PHASE36_PLAN.md`, plus §1 of
> `OFFLINE_MODE_PHASE9_PLAN.md`, then do Phase N. Don't read the other phase
> sections.

The standing facts, in short:

1. Branch is `mods/offline-mod-2`. Everything in this batch is **client-only** —
   nothing touches `KncWX2Server/`, so the servers never need rebuilding.
2. **Every edit is gated by `SERV_IRUHADEV_OFFLINE`**, the flag already in
   `KTDXLIB/Always.h`. Do not mint a per-phase flag; the phase-9 doc's §1 rule 2
   explains why the four that were minted had to be migrated away. A throwaway
   `SERV_IRUHADEV_*_DEBUG` logging flag is the only exception.
3. Build and deploy:
   ```sh
   TRUNK="f:/elsword stuff/elsword_2014/els_2014/ElswordFiles/ready_and_built/ProjectX2_SVN/ProjectX2_SVN/source/EU_CN_US/Trunk"
   DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"
   touch X2Lib/stdafx.cpp        # after ANY header edit, or the PCH eats it
   msbuild X2Lib/X2Lib_2010.vcxproj -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   msbuild X2/X2_2010.vcxproj       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   ls -la X2/US_SERVICE/X2.exe   # judge by the artifact; post-build ALWAYS fails here
   cp X2/US_SERVICE/X2.exe "$DATA/X2_offline.exe"
   ```
   Verify the copy landed by reading the name, size and mtime programmatically.
   The directory holds eight other `X2*.exe`.
4. Reading the logs is the verification; there is no test suite.
5. `els_db.sql` is real player data, and `X2OfflineDB` migrates forward only.
   Per §0.5, no phase here should need a migration at all.

**Files in this batch that carry non-ASCII bytes**: the offline
`Handlers_*.cpp` and `X2Offline*.{h,cpp}` are ASCII, but `X2UIInventory.cpp`,
`X2ItemManager.cpp` and `X2StateDungeonResult.cpp` are **CP949** and phases 43
and 45 may touch them. `CLAUDE.md`'s rule binds: after editing any file with
non-ASCII bytes, run `file <path>` and `git diff --stat`, and if a ten-line
change came out as hundreds of lines, restore and redo it as a byte-level Python
patch. This has already destroyed a file twice in this project.

### Cross-cutting rules — every phase obeys all five

1. **Never consume an item for nothing.** The refusal shape is already in the
   handler ([Handlers_Inventory.cpp:215-223](X2Lib/Offline/Handlers_Inventory.cpp#L215)):
   reply an error, leave the item, log the ID and why. A refusal is a bug
   report; a consumed item is unrecoverable player data.
2. **Fill `m_iED` on the initial error value**, the way the cube handler does at
   `Handlers_Social.cpp:2637` — that is what makes it impossible for a later
   `return Reply(...)` to ship a zero.
3. **Name the item before writing code for it** (§0.9), and establish its route
   (§0.1).
4. **Read the live handler, not the commented one** (§0.2).
5. **Fresh out-vector per `InsertItem` call.** `InsertItem` clears the caller's
   vector on entry ([X2OfflineInventory.cpp:1436](X2Lib/Offline/X2OfflineInventory.cpp#L1436));
   the reference shape is [Handlers_Social.cpp:2838-2847](X2Lib/Offline/Handlers_Social.cpp#L2838).

### Live-DB asks

Per `CLAUDE.md`'s live-DB rule these are asks with a named query, never
assumptions. **None of them blocks phase 36.** Ask as plain text in the
conversation, never as a file written into the game tree, and keep server names,
linked servers and logins out of anything committed or logged.

```sql
SELECT OBJECT_NAME(object_id) AS proc_name, definition
FROM   sys.sql_modules
WHERE  OBJECT_NAME(object_id) IN ( 'gup_insert_item', 'gup_update_item_seal',
                                   'gup_get_item_list_seal' );

-- when the live names may have drifted from this snapshot's
SELECT name FROM sys.procedures WHERE name LIKE 'gup_%item%' ORDER BY name;
```

| proc | wanted by | question it answers |
|---|---|---|
| `gup_insert_item` | phase 40 | which columns a cube-inserted row is actually born with — period, enchant, seal |
| `gup_update_item_seal` | phases 39, 40 | confirms `m_ucSealData = 100` is stored verbatim |
| `gup_get_item_list_seal` | phase 39 | how seal state is read back at login |
| `gup_%item_exchange%`, `gup_%convert%`, `gup_%manufacture%` | phases 41, 42, 44 | the names first; bodies once known |

---

## §2 — Phase index

| Phase | Subject | Size | Confidence | Blocked? |
|---|---|---|---|---|
| **36** | **Philosopher's Scroll (160267): instant level-up**, + the ED fix for this handler | small | **CONFIRMED** | no |
| **37** | Route A: the item-use effect switch, transcribed from `GSUserInventory.cpp:4162`/`:4655` | large | CONFIRMED-empty | no |
| **38** | Cube coverage: client registry vs. contents table, and the refusal census | investigation | — | no |
| **39** | `EGS_UNSEAL_ITEM_REQ` / `EGS_SEAL_ITEM_REQ` | medium | CONFIRMED | no |
| **40** | Cube fidelity: rental period, and the seal phase 39 made safe | medium | CONFIRMED | no |
| **41** | `EGS_ITEM_EXCHANGE_REQ` | medium | CONFIRMED | no |
| **42** | `EGS_ITEM_CONVERT_REQ` | medium | CONFIRMED | no |
| **43** | Dungeon reward box — **confirm it is dead on live, then decide** | small | CONFIRMED | no |
| **44** | `EGS_ITEM_MANUFACTURE_REQ`: the item workshop | medium | CONFIRMED | no |
| **45** | `EGS_SYNTHESIS_ITEM_REQ` + drive the `UNHANDLED` bucket back to empty | small | CONFIRMED | no |

Ordering that matters:

- **36 first.** The user's stated priority, and self-contained.
- **39 strictly before 40.** Phase 40 seals cube output; without 39 that output
  is unusable (§0.6).
- **38 informs 37 and 40** but blocks neither. It is investigation; it can run
  alongside 37 provided they do not both edit `Handlers_Inventory.cpp`.
- **36, 37 and phase-34/35 of the other document all edit the same handler.**
  Do not run them as parallel conversations.
- 41, 42, 44, 45 are independent of everything and of each other.

---

# Phase 36 — Philosopher's Scroll (160267): instant level-up

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
The Philosopher's Scroll is right-clicked in the inventory. Offline it is
consumed and nothing happens, and the displayed ED drops to zero. The item is
gone.

### Diagnosis — CONFIRMED

**The item.** `ItemTrans.lua` (decrypted per §0.9) gives:

> **160267** — "Philosopher's Scroll" — *"A super rare scroll that will
> instantly grant one level up increase from your current level.
> (Right-click to use)"*

The server names it `CXSLItem::EI_CHAR_LEVEL_UP_ITEM = 160267`
([XSLItem.h:713](KncWX2Server/Common/X2Data/XSLItem.h#L713)),
behind `SERV_CHAR_LEVEL_UP_ITEM`, which is on (§0.3).

**Its route is A, not B** (§0.1), established two ways rather than assumed:
160267 is absent from the client's `RandomItem.lua` registry, and it has no
`case` of its own in `X2UIInventory.cpp`'s switch — it falls through to
`default:` at [`:6894`](X2Lib/X2UIInventory.cpp#L6894), which sends
`EGS_USE_ITEM_IN_INVENTORY_REQ`. The client only reaches that switch at all when
`GetCanUseInventory()` is true, so the templet flag must be true or the live game
could never have used the item either.

**The live behaviour is two switch arms.**

Validation, [GSUserInventory.cpp:4162-4180](KncWX2Server/GameServer/GSUserInventory.cpp#L4162):

```cpp
case CXSLItem::EI_CHAR_LEVEL_UP_ITEM:
case CXSLItem::EL_CHAR_LEVEL_UP_ITEM2:          // SERV_CHAR_LEVEL_UP_ITEM_EVENT_RENA, on
    {
        if( SiKGameSysVal()->GetLimitsLevel() == GetLevel() )
        {
            KEGS_USE_ITEM_IN_INVENTORY_ACK kPacket;
            kPacket.m_iOK = NetError::ERR_UNKNOWN;   // US: SERV_LIMIT_LEVEL_NOT_USE_ITEM_MSG is JP-only
            SendPacket( EGS_USE_ITEM_IN_INVENTORY_ACK, kPacket );
            return;
        }
    }
    break;
```

Effect, [GSUserInventory.cpp:4655-4671](KncWX2Server/GameServer/GSUserInventory.cpp#L4655):

```cpp
case CXSLItem::EI_CHAR_LEVEL_UP_ITEM:            // note: 60004276 is NOT here, see below
    {
        const int iNextLevel = GetLevel() + 1;
        const int iTotalExpByNextLevel = SiKExpTable()->GetRequireTotalExpbyLevel( iNextLevel );
        m_kEXP.AddExp( iTotalExpByNextLevel - GetEXP() );
        CheckCharLevelUp();                       // no-arg; SERV_DUNGEON_CLEAR_PAYMENT_ITEM_EVENT is off
    }
    break;
```

So the scroll does not grant a flat amount of EXP — it tops the character up to
**exactly** the total required for the next level. A character at 40% of the way
through level 30 ends at 0% of level 31, not 40%.

**The ED.** `kAck.m_iED = 0` at
[Handlers_Inventory.cpp:202](X2Lib/Offline/Handlers_Inventory.cpp#L202) and is
never reassigned. The real server sources it from `GetED()`
(`GSUserInventory.cpp:4544`). This is why the phase owns the ED fix: the exit
test cannot distinguish "level-up worked" from "everything is broken" while the
wallet is being zeroed on the same packet.

**Everything needed already exists offline.** Reuse, do not rewrite:

| need | existing code |
|---|---|
| exp → level, skill-point award, persist | `CX2OfflineServer::ApplyDungeonReward` ([Handlers_Room.cpp:2711](X2Lib/Offline/Handlers_Room.cpp#L2711)) |
| the EXP table | `g_pData->GetEXPTable()->GetEXPData( n ).m_nTotalExp` ([Handlers_Room.cpp:2740](X2Lib/Offline/Handlers_Room.cpp#L2740)) — the client's own table, so the two cannot disagree |
| `EGS_CHAR_LEVEL_UP_NOT` + both stat blocks | `CX2OfflineServer::PushLevelUp` ([X2OfflineServer.cpp:976](X2Lib/Offline/X2OfflineServer.cpp#L976)) |
| level-up quest and title steps | `CX2OfflineServer::QuestOnLevelUp` ([Handlers_Quest.cpp:1023](X2Lib/Offline/Handlers_Quest.cpp#L1023)) |
| the level cap | `_CONST_X2GAME_::g_iMaxLevel` = **67** for US ([Always_US.h:75](KTDXLIB/OnlyGlobal/Always_US.h#L75)) |

On the cap: the server uses `SiKGameSysVal()->GetLimitsLevel()`, read from
`GameSysValTable.lua`. `g_iMaxLevel` is a legitimate stand-in rather than a
guess — the studio's own comment beside the `#define` says the constant and
`GameSysValTable.lua`'s `MAXLevel` must be changed together, and
`ApplyDungeonReward` already caps against it (`Handlers_Room.cpp:2737`). Cite
that comment where you use it.

**Item 60004276.** `EL_CHAR_LEVEL_UP_ITEM2` appears in the live *validation*
switch (`:4164`) but not in the live *effect* switch (`:4655`) — its only effect
case is at `:6066`, inside the commented-out `#else` (§0.2). On the live US
server it is validated, consumed, and does nothing. **Offline refuses it**, per
the user's decision; see step 4.

### What to do

1. **Add the ED fix first**, so the rest is verifiable. Set `kAck.m_iED` from the
   loaded unit row on the **initial error value**, replacing the `= 0` at
   `Handlers_Inventory.cpp:202`, exactly as `Handlers_Social.cpp:2637` does. If
   you add the `FillAckED` helper that phase 34 proposes, note in the commit that
   phase 34's scope shrinks accordingly (§0.8).
2. **Add the max-level refusal**, before anything is consumed:
   refuse when `kUnitRow.m_iLevel >= (int)_CONST_X2GAME_::g_iMaxLevel` with
   `NetError::ERR_UNKNOWN`, leaving the item in the bag, and log the level. Put
   it beside the existing pre-consume refusals at `Handlers_Inventory.cpp:238-302`
   so it shares their shape.
3. **Add the effect**, after `ConsumeOne` succeeds. Transcribe `:4655`:
   ```
   iNextLevelTotalExp = GetEXPTable()->GetEXPData( level + 1 ).m_nTotalExp
   iAddEXP            = iNextLevelTotalExp - kUnitRow.m_iEXP
   ApplyDungeonReward( unitUID, iAddEXP, 0, &iOldLevel )
   ```
   then `PushLevelUp` and `QuestOnLevelUp` **only if the level actually rose**,
   copying the guard at [Handlers_Room.cpp:2880-2884](X2Lib/Offline/Handlers_Room.cpp#L2880).
   `ApplyDungeonReward` already awards the skill points and saves; do not
   duplicate either.
4. **Refuse 60004276** with the item intact and a log line naming the studio
   source — that `GSUserInventory.cpp:4655` has no case for it and `:6066`
   does but is inside the commented-out `#else`. This is a deliberate divergence
   from live and the log line is what makes it reconstructable later.
5. Add a `//{{ Iruha : 2026-09-08 // …` change block around the new code, per the
   house convention.

### Trap

- **`ApplyDungeonReward`'s name.** It is not dungeon-specific — it is the only
  exp/ED/level/SP path offline has, and `Handler_EGS_END_GAME_REQ` is merely its
  first caller. Reusing it is right. Writing a second level-up path beside it is
  how the client and the save start disagreeing about what level the character
  is.
- **Do not add a flat EXP amount.** `GetRequireTotalExpbyLevel` returns a
  *total*, and the server subtracts the current EXP from it. Adding
  `GetRequireNeedExpbyLevel` instead would over-pay a character mid-level and
  could grant two levels.
- **`ApplyDungeonReward` caps EXP at the max level** (`Handlers_Room.cpp:2747`),
  so a scroll used at 66 lands on 67 cleanly — but only step 2's refusal stops
  a scroll being eaten at 67 for nothing.
- The `m_iTempCode` field on the ACK is the pet-auto-looting echo
  (`Handlers_Inventory.cpp:203`); leave it alone.

### Exit test

Note level, EXP and ED. Then, on a character below 67:

- Right-click the Philosopher's Scroll. The character gains **exactly one**
  level; the level-up effect plays; skill points rise by the curve's amount; the
  displayed ED is **unchanged** and equals `select ed from unit`.
- `offline_server.log` carries the `REWARD unitUID=… LEVEL UP n -> n+1` line and
  a `+n SP` line.
- `select level, exp from unit` shows the EXP sitting exactly on the new level's
  `m_nTotalExp`, not above it.

Then take a character to 67 and confirm the scroll is refused **with the item
still in the bag** and a log line saying why. Same for 60004276 at any level.

---

# Phase 37 — Route A: the item-use effect switch

**Flag:** `SERV_IRUHADEV_OFFLINE`

**Run after phase 36** — same handler.

### Symptom
Most items that can be right-clicked from the bag are consumed and do nothing.
The skill-unseal books and the skill note work; everything else is eaten.

### Diagnosis — CONFIRMED
`Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` implements exactly three effects: the
unseal book (`:238-260`, `:359-369`), the skill note (`:272-302`, `:375-386`),
and the in-room buff/ability `_NOT` (`:339-352`). Everything else falls to
`ConsumeOne` and a bare ACK (`:304-314`).

The live server's equivalent switches are the validation arm at
[GSUserInventory.cpp:3944](KncWX2Server/GameServer/GSUserInventory.cpp#L3944)
(cases running from roughly `:4000` to `:4530`) and the effect arm at
[`:4535`](KncWX2Server/GameServer/GSUserInventory.cpp#L4535) (cases to `:4935`).
Read only those (§0.2).

**Scope comes from the client, not from item names.** `X2UIInventory.cpp`
enumerates ~120 route-A item IDs by named constant, in two switches:
[`:5994-6113`](X2Lib/X2UIInventory.cpp#L5994) (the pre-check) and
[`:6214-6898`](X2Lib/X2UIInventory.cpp#L6214) (the dispatch). Families present:
`WARP_ITEM_*`, `NICKNAME_CHANGE_CARD_ITEM_ID`, `INIT_SKILL_TREE_*`,
`RURIEL_RESET_SKILL_ITEM`, `ITEM_FOR_SEAL_*` (the Phoru stamps),
`SKILL_POINT_*` / `SKILL_PLUS_ITEM_ID`, `GOLD_TICKET_*`,
`INVENTORY_SLOT_ADD_ITEM_*_EVENT`, `PROTECTION_OF_*`, `BELSSING_OF_*`,
`ELIXIR_*`, `NASOD_SCOPE_*`, `CHANGE_PET_NAME_ITEM_ID`, `GUILD_CREATE_ITEM_ID`,
`INIT_GUILD_SKILL_TREE_ITEM_ID`, the wedding/couple items,
`ACTIVATION_DROP_ITEM_PICKUP_SKILL`, `HALLOWEEN_TRANSFORM_POSION`,
`UNLIMITED_SECOND_CHANGE_JOB_ITEM_ID`, `RECOMMEND_TICKET_ID`.

Note several of those already have a client-side case that does **not** reach
`EGS_USE_ITEM_IN_INVENTORY_REQ` — the warp scrolls open a village menu, the
elixirs go through the quick-slot `_NOT`. Cross the two lists before assuming a
constant needs server work.

### What to do

1. **Census first.** Add the §0.9 diagnostic line to the handler, play for a
   session using whatever the save actually holds, then
   `grep "ITEM     used item" offline_server.log | sort | uniq -c | sort -rn`.
   Name every ID that appears via `ItemTrans.lua`. That list — not the full 120
   — is the phase's real scope.
2. **Group by what the effect needs**, and do the groups that need nothing new
   first. In rough order of cheapness:
   - **Inventory expansion** (`INVENTORY_SLOT_ADD_ITEM_*_EVENT`,
     `CI_EXPAND_INVENTORY_*`) — `CX2OfflineInventory` already has per-category
     maxima; this is a stored counter and an ACK field.
   - **Skill-point items** (`SKILL_POINT_*`) — `CX2OfflineSkill` already tracks
     CSP; the live path is `DBE_INSERT_CASH_SKILL_POINT_REQ` with an
     `m_iCSPoint` and an `m_iPeriod`.
   - **Skill reset / init** (`INIT_SKILL_TREE_*`, `RURIEL_RESET_SKILL_ITEM`) —
     `CX2OfflineSkill` owns the tree already.
   - **Nickname / pet-name change** — a single column write.
   - **The Phoru stamps** (`ITEM_FOR_SEAL_*`) — these belong to phase 39; leave
     them and say so.
   - Anything needing a system that does not exist offline (guild creation,
     weddings) — **refuse with the item intact** and a log line naming the
     system. That is a finished outcome for this phase, not a deferral.
3. For each group, transcribe the live validation case *and* the live effect
   case. A validation case without its effect is how an item gets consumed for
   nothing.
4. Keep the switch ordered and commented to mirror the server's, so a future
   reader can diff them.

### Trap
- The handler currently refuses everything with `GetCanUseInventory() == false`
  (`:215-223`). That check is **correct and stays** — the client never sends
  route A for such an item, so anything hitting it arrived by another door.
- Several live cases end in `SendToGameDB(...)` and `return` — their real effect
  is in the `DBE_*_ACK` handler, not the `EGS_*_REQ` one. Follow the DB round
  trip before concluding a case does nothing.
- Do not batch this into one commit. One group per commit, each with its own
  play-test.

### Exit test
For each group implemented: use the item, see the effect, and confirm
`select … from unit` / the relevant offline table changed. For each group
refused: the item is **still in the bag** afterwards and the log names it.
`grep "used item" offline_server.log` should have no entry that is neither
implemented nor explicitly refused.

---

# Phase 38 — Cube coverage: registry vs. contents

**Flag:** `SERV_IRUHADEV_OFFLINE` (likely no code change at all)

### Symptom
Some cubes refuse to open with
`CUBE refused - item %d has no row in RandomItemTable.lua`.

### Diagnosis — investigation, not yet confirmed
Route B has **two** tables and they are different files (§0.4):

- `RandomItem.lua` — the **client's** registry. Decides whether the client will
  even send `EGS_OPEN_RANDOM_ITEM_REQ` ([X2UIInventory.cpp:6927](X2Lib/X2UIInventory.cpp#L6927)).
- `RandomItemTable.lua` — the **contents**, loaded by `CX2OfflineRandomItem`.

An item in the first but not the second is a cube the client offers and the
offline server refuses. Whether that set is empty is unknown and is this phase's
question.

### What to do
1. Decrypt both (§0.9). Extract the cube IDs from each — for `RandomItem.lua`
   the rows are `AddRandomItemTemplet( id, keyID, keyCount, requiredED )`
   ([X2ItemManager.cpp:1592](X2Lib/X2ItemManager.cpp#L1592)); note the constant
   pool de-duplicates, so a naive number scan mixes IDs with key IDs and counts —
   treat a set difference as a lead, not a verdict.
2. Cross-check the leads against `CX2OfflineRandomItem::GetCube` at runtime
   rather than on paper, and name each one via `ItemTrans.lua`.
3. Check the load actually succeeded: `grep "CUBE" offline_server.log` for the
   loader's own row-count line. `RandomItemTable.lua` is 7.9 MB and the load is
   lazy — nothing touches it until the first cube is opened, so an empty log
   means "not yet exercised", not "fine".
4. **If the difference is empty, the phase's deliverable is that finding**, added
   to this document. Do not invent work.
5. If it is not empty, the honest options are: confirm the missing cubes are
   region-dead (compare against `KncWX2Server/ServerResource/*/RandomItemTable.lua`
   for the other regions), or leave them refused with a clearer log line. **Do
   not fabricate contents for a cube.**

### Trap
`SERV_CUBE_IN_ITEM_MAPPING` is on, but `RandomItemMapping.lua` is 100% comments
in the US branch, so the live US server swaps nothing either — the offline
no-op is correct and is already documented at
[X2OfflineRandomItem.h:49-53](X2Lib/Offline/X2OfflineRandomItem.h#L49). Do not
"fix" it.

### Exit test
A written answer with numbers: how many cubes the client registry lists, how
many the contents table covers, and the named list of any difference. Plus a
play-test opening at least one cube of each shape the save can reach
(single-draw, give-all, key-required, ED-required).

---

# Phase 39 — `EGS_UNSEAL_ITEM_REQ` / `EGS_SEAL_ITEM_REQ`

**Flag:** `SERV_IRUHADEV_OFFLINE`

**Must land before phase 40.**

### Symptom
Both packets are declined (§0.7), so a bound item can never be unbound and a
Phoru's Foot Stamp can never be used.

### Diagnosis — CONFIRMED
Read §0.6 first; the seal field is not what it looks like.

The GameServer handlers are thin wrappers:
[`EGS_SEAL_ITEM_REQ` at `:7381`](KncWX2Server/GameServer/GSUserInventory.cpp#L7381)
and [`EGS_UNSEAL_ITEM_REQ` at `:7476`](KncWX2Server/GameServer/GSUserInventory.cpp#L7476).
Both refuse while trading or personal-shopping (`GetTradeUID() > 0 ||
GetPersonalShopUID() > 0` → `ERR_ITEM_14`) and while the inventory is locked
(`ERR_INVENTORY_LOCK_00`), then delegate:

- `KInventory::SealItem( iDestItemUID, iSealItemUID, ucBeforeSealCount, ucSealResult, vecUpdated )`
  — [Inventory.cpp:16330](KncWX2Server/GameServer/Inventory.cpp#L16330)
- `KInventory::UnsealItem( iDestItemUID, ucSealResult, vecUpdated )`
  — [Inventory.cpp:16761](KncWX2Server/GameServer/Inventory.cpp#L16761)

`SealItem` is the **grade-matched stamp**: it picks the required stamp item from
the target's grade ([Inventory.cpp:16502-16509](KncWX2Server/GameServer/Inventory.cpp#L16502)) —
`IG_UNIQUE → CI_SEAL_ITEM_UNIQUE`, `IG_ELITE → CI_SEAL_ITEM_ELITE`,
`IG_RARE → CI_SEAL_ITEM_RARE`, `IG_NORMAL → CI_SEAL_ITEM_NORMAL` — then accepts
the several event variants of each (`CI_EVENT_SEAL_ITEM_*`,
`CI_RURIEL_EVENT_SEAL_ITEM_*`, `CI_ARIEL_EVENT_SEAL_ITEM_*`, `:16550-16590`),
refuses an item held in the bank (`IsExistInBank`, `:16535`), and consumes the
stamp.

The client already gates on the count:
`GetMaxSealCount() - m_ucTimesToBeSealed` at
[X2UIInventory.cpp:8027](X2Lib/X2UIInventory.cpp#L8027) and `:7932`.

`KOfflineItemRow::m_iSealData` already exists (§0.5) — no migration.

### What to do
1. Remove both entries from `X2OfflineIgnore.cpp` (`:63-64`), and delete the
   stale `EGS_RESOLVE_ITEM_REQ` line at `:61` while you are in the file (§0.7).
2. Add a `SetSealData( nItemUID, iSealData, OUT KInventoryItemInfo& )` to
   `CX2OfflineInventory`, in the shape of the existing `SetEnchantLevel` /
   `SetEndurance` setters ([X2OfflineInventory.h:153-168](X2Lib/Offline/X2OfflineInventory.h#L153)).
   One setter, absolute value — the caller does the arithmetic, as with enchant.
3. `Handler_EGS_UNSEAL_ITEM_REQ`: refuse unless `IsSealedItem()`
   (`m_iSealData >= 100`); otherwise subtract 100, persist, and reply with the
   updated slot in `m_vecInventorySlotInfo`. Mirror
   `DBE_UNSEAL_ITEM_ACK`'s ACK fill ([GSUserInventory.cpp:7556-7559](KncWX2Server/GameServer/GSUserInventory.cpp#L7556)).
4. `Handler_EGS_SEAL_ITEM_REQ`: transcribe the grade→stamp mapping and the event
   variants from `Inventory.cpp:16502-16590` **verbatim**, refuse a mismatch
   with the stamp intact, consume the stamp on success, and add 1 to the low part
   of `m_iSealData` — never letting it cross 100 by accident. Refuse at
   `GetMaxSealCount()`.
5. Both handlers: trading and personal shop do not exist offline, so those two
   refusals are unreachable — say so in a comment rather than porting dead
   checks.

### Trap
- **`IS_SEALED` and `KInventory::Seal`/`Unseal` are dead code** (§0.6). If you
  find yourself writing to `m_iItemState`, you are on the wrong path.
- **The `>= 100` encoding means arithmetic, not a boolean.** `m_iSealData = 1`
  is "stamped once, not bound"; `= 100` is "bound, never stamped"; `= 101` is
  both. Setting it to 0 or 1 to mean "unsealed" destroys the stamp count.
- `GetMaxSealCount()` is a **templet** property, not an item one — read it off
  `CX2OfflineInventory::Templet( itemID )`.
- The Phoru stamps are also route-A items in phase 37's list. They belong here;
  make sure phase 37 left them alone.

### Exit test
On an item with `m_iSealData >= 100` in the save: unseal it, watch the seal
badge clear in the inventory tooltip, and confirm
`select seal_data from item where item_uid = …` dropped by exactly 100.
Then apply a grade-matched Phoru stamp: the stamp is consumed, the count rises
by one, and a grade-*mismatched* stamp is refused **with the stamp still in the
bag**. Zero `UNHANDLED` for both event IDs.

---

# Phase 40 — Cube fidelity: rental period and the seal

**Flag:** `SERV_IRUHADEV_OFFLINE`

**Run after phase 39.**

### Symptom
Cubes pay out, but timed rewards last forever and bound rewards arrive unbound —
offline is quietly more generous than live.

### Diagnosis — CONFIRMED

**Period is dropped.** `CX2OfflineRandomItem::KResult` carries `m_iPeriod`
([X2OfflineRandomItem.h:111](X2Lib/Offline/X2OfflineRandomItem.h#L111)), but
`CX2OfflineInventory::InsertItem`'s third parameter is `iEnchantLevel`, not a
period ([X2OfflineInventory.h:145](X2Lib/Offline/X2OfflineInventory.h#L145)),
and the cube call site passes `0`
([Handlers_Social.cpp:2843](X2Lib/Offline/Handlers_Social.cpp#L2843)). Every
row is born with `m_iPeriod = 0` ([X2OfflineInventory.cpp:1511](X2Lib/Offline/X2OfflineInventory.cpp#L1511)),
i.e. permanent. `KOfflineItemRow` has the column (§0.5).

**Seal is logged but not applied.** `IsSealedCube` is used in exactly one place —
a log line ([Handlers_Social.cpp:2895](X2Lib/Offline/Handlers_Social.cpp#L2895)).
Live, `EGS_OPEN_RANDOM_ITEM_REQ` fills `m_setSealRandomItem`
([GSUserInventory.cpp:2183-2196](KncWX2Server/GameServer/GSUserInventory.cpp#L2183))
from `IsSealRandomItemCube( cubeID )` ∧ `IsCanSealForRandomItem( resultID )`,
and the DB layer applies `m_ucSealData = 100`
([GSGameDBThread.cpp:3381](KncWX2Server/GameServer/GSGameDBThread.cpp#L3381)).

**The two interact.** `Query_UpdateSealItem` skips any item with a period:
`if( mit->second.m_sPeriod > 0 ) continue;`
([GSGameDBThread.cpp:3377](KncWX2Server/GameServer/GSGameDBThread.cpp#L3377)).
A rental reward is never sealed. Implement period first, then let the seal
respect it — doing them in the other order hides the interaction.

**The reference for the insert half is `KInventory::OpenRandomItem`**
([Inventory.cpp:8455](KncWX2Server/GameServer/Inventory.cpp#L8455) / `:8470`,
called from [GSUserInventory.cpp:1859](KncWX2Server/GameServer/GSUserInventory.cpp#L1859)).
`CX2OfflineRandomItem` ported only the *draw* half
(`CXSLRandomItemManager::GetResultItem`), which is why these two fields have no
home yet.

`SERV_TREASURE_BOX_ITEM` is **off** (§0.3), so the `IsTreasureBox` arm of the
seal condition at `:2189` does not compile in — do not port it.

### What to do
1. Give `InsertItem` an optional `iPeriod` (default 0) and thread
   `KResult::m_iPeriod` through from `Handlers_Social.cpp:2843`. Audit the other
   `InsertItem` callers before changing the signature — they all pass positional
   arguments.
2. Set `m_iUsageType` consistently with the period. A timed item is
   `CX2Item::PT_*` — read what `MakeInventoryItemInfo` expects
   ([X2OfflineInventory.cpp:636](X2Lib/Offline/X2OfflineInventory.cpp#L636))
   rather than guessing, or the client will draw the wrong tooltip.
3. Apply the seal: for a cube where `IsSealedCube( cubeID )` is true, set
   `m_iSealData = 100` on each inserted row whose item ID is sealable — **and
   only when that row's period is 0**, per `:3377`.
4. `CX2OfflineRandomItem` needs the `IsCanSealForRandomItem( resultID )` half of
   the condition; `RandomItemData.lua`'s `AddSealRandomItemInfo` is where it
   comes from. If that data turns out to be cube-level only, say so and seal at
   cube granularity — do not approximate silently.
5. Ask for `gup_insert_item` and `gup_update_item_seal` (§1) to confirm the
   column semantics before committing. This is the one phase where the live DB
   genuinely settles something.

### Trap
- **Do not land this before phase 39.** A sealed reward with no unseal path is a
  dead item in a player's bag, and `els_db.sql` is real data.
- `InsertItem`'s third argument is enchant level. Adding a period as a fourth
  positional argument next to the optional `pvecSocket` is easy to get wrong —
  check every call site compiles *and* still means what it did.
- Phase 39's `m_iSealData` arithmetic applies here too: set 100, never 1.

### Exit test
Open a cube known to pay a timed item: the item shows its remaining days in the
tooltip and `select period, usage_type from item …` is non-zero. Open a sealed
cube: the reward carries the bound marker, `seal_data = 100`, and phase 39's
unseal then clears it. A cube paying a **timed** reward out of a **sealed** cube
leaves that row with `seal_data = 0` — the `:3377` rule.

---

# Phase 41 — `EGS_ITEM_EXCHANGE_REQ`

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
The exchange NPC's confirm button does nothing; the packet is declined
([X2OfflineIgnore.cpp:68](X2Lib/Offline/X2OfflineIgnore.cpp#L68)).

### Diagnosis — CONFIRMED
Live entry point: [`EGS_ITEM_EXCHANGE_REQ` at GSUserInventory.cpp:6949](KncWX2Server/GameServer/GSUserInventory.cpp#L6949),
with the DB round trip at [`DBE_ITEM_EXCHANGE_ACK`, `:7278`](KncWX2Server/GameServer/GSUserInventory.cpp#L7278)
and a limit check at `:8085`. The logic is in `KInventory`, which has a
**check** function and four `ItemExchange` overloads — read them before picking
one:

| function | line |
|---|---|
| `ItemExchangeCheck( iHouseID, … )` | [Inventory.cpp:15275](KncWX2Server/GameServer/Inventory.cpp#L15275) |
| `ItemExchange( iHouseID, … )` ×3 | `:15473`, `:15746`, `:15764` |
| `ItemExchange( iItemID, iHouseID, iSelectedItemID, mapInsertedItem, vecUpdated, vecNewItem )` | [`:15985`](KncWX2Server/GameServer/Inventory.cpp#L15985) |

The `iSelectedItemID` parameter on the last one is the tell: **this is the
"choose one of N" box**, which is a distinct shape from the cube's random draw
and from a fixed bundle.

The data is client-side: `ItemExchangeTable.lua` →
`g_pItemManager:AddItemExchangeData`, carrying at least `m_iSourceQuantity` and
`m_iServerGroupID` (§0.4). `CX2ItemManager` already parses it, so the offline
server can read the client's own parsed table rather than re-loading the file —
check `X2ItemManager.h` for the accessor before writing a loader.

### What to do
1. Establish which overload the US build's call site actually reaches, by
   reading `:6949` under the §0.3 flag states. Do not port all four.
2. Prefer reading `CX2ItemManager`'s already-parsed exchange data over adding a
   second loader; fall back to the `CX2OfflineRandomItem::RunScript` shape only
   if there is no accessor.
3. Implement: validate the source items and quantity, validate the selection is
   in the group, consume, insert, reply with every touched slot. Fill `m_iED` if
   the ACK carries one, and take the commission if the table has one.
4. Remove the ignore entry.

### Trap
- `m_iServerGroupID` gates rows by server. Offline there is one server; read
  what the client passes rather than assuming 0.
- The "choose one of N" shape means the request names a **selection**. Validating
  that the selection belongs to the group is the whole security of the feature —
  offline it is the whole correctness of it.

### Exit test
Exchange at the NPC: source items leave the bag, the chosen reward arrives, ED
moves by the commission, everything updates without a character re-select. A
selection that is not in the group is refused with the source items intact.

---

# Phase 42 — `EGS_ITEM_CONVERT_REQ`

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
Conversion does nothing; the packet is declined
([X2OfflineIgnore.cpp:67](X2Lib/Offline/X2OfflineIgnore.cpp#L67)).

### Diagnosis — CONFIRMED
Live: [`EGS_ITEM_CONVERT_REQ` at GSUserInventory.cpp:9064](KncWX2Server/GameServer/GSUserInventory.cpp#L9064),
DB round trip at [`DBE_ITEM_CONVERT_ACK`, `:9126`](KncWX2Server/GameServer/GSUserInventory.cpp#L9126).

Data is client-side: `ItemConvertTable.lua` →
`g_pItemManager:AddItemConvertInfo` / `AddItemConvertGroup`, with fields
`m_ConvertType`, `m_TargetItemID`, `m_ResultItemID`, `m_Commission` (§0.4).
`m_ConvertType` implies more than one conversion mode — enumerate the distinct
values in the decrypted table before writing the switch, rather than assuming
one.

### What to do
1. Read `:9064` and `:9126` together; the effect is likely in the DB ACK.
2. Enumerate `m_ConvertType`'s distinct values from the table, and implement each
   the server implements. Refuse any type you do not implement, item intact,
   with the type in the log.
3. Charge `m_Commission` and fill `m_iED` on the ACK from the loaded row (rule 2).
4. Remove the ignore entry.

### Trap
Convert and exchange look alike and are different tables with different packets.
Do not share an implementation between phases 41 and 42 before reading both
live handlers — a shared helper that fits one and is bent to fit the other is
how the wire format drifts.

### Exit test
Convert an item: the target leaves, the result arrives, ED drops by the
commission, `select … from item` agrees with the UI without a re-select.

---

# Phase 43 — Dungeon reward box: confirm it is dead, then decide

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
The end-of-dungeon treasure-box pick never appears. The offline handlers are
phase-3 stubs whose comments say the feature is waiting on inventory work that
has since landed ([Handlers_Room.cpp:3228-3249](X2Lib/Offline/Handlers_Room.cpp#L3228)).

### Diagnosis — CONFIRMED, and it is not what the stubs assume

**The live server has no handler for these packets at all.** Searching the whole
of `KncWX2Server/` for `REWARD_BOX` returns only:

- the six event IDs ([EventID_Client.h:297-303](KncWX2Server/Common/EventID_Client.h#L297)),
- their serializers ([ClientPacket.cpp:2459-2492](KncWX2Server/Common/ClientPacket.cpp#L2459)),
- an unrelated UI enum, `NUI_MI_REWARD_BOX`.

No `IMPL_ON_FUNC` for `EGS_START_REWARD_BOX_SELECT_REQ` or
`EGS_SELECT_REWARD_BOX_REQ` exists in any of the five servers. So the live US
server never sent `EGS_START_REWARD_BOX_SELECT_NOT` either.

The client sends the request unconditionally from the result screen
([X2StateDungeonResult.cpp:950-954](X2Lib/X2StateDungeonResult.cpp#L950)) and
only advances to the box-pick animation when
`Handler_EGS_START_REWARD_BOX_SELECT_NOT` sets `m_bStartRewardBoxSelectNot`
([`:2021-2026`](X2Lib/X2StateDungeonResult.cpp#L2021)). No `_NOT`, no box —
on live and offline alike.

**So offline's stub reproduces live behaviour exactly**, and the phase-3 comment
("nothing in the boxes until item drops land in phase 5") is a stale guess, not
a finding.

### What to do
1. **Re-verify the negative** before acting on it — a grep proving absence is
   weaker than one proving presence. Check the CenterServer and GlobalServer FSM
   registration tables as well as `IMPL_ON_FUNC`, and check whether any
   `SERV_*` flag would compile a handler in for another region.
2. If it holds: **change nothing in the handlers.** Replace the stale phase-3
   comments with the finding and its references, correct the claim in
   `OFFLINE_MODE_PLAN.md` if it repeats there, and record it in this document.
   A feature that is off *and matches live* is finished.
3. If a handler does turn up somewhere, this becomes a real implementation phase
   and the box contents come from that handler's table — reopen it then.

### Trap
The temptation is to build the box because it is called a box and the stubs
invite it. Building it would make offline **diverge** from live in the player's
favour, inventing a reward source the real game did not have. That is the
band-aid this project's rules name, wearing a feature's clothes.

### Exit test
A written answer with references: the feature is dead on live, the stub is
correct, here is where that is proven. Plus one dungeon run confirming the
result screen completes normally and neither event ID appears in the
`UNHANDLED` bucket.

---

# Phase 44 — `EGS_ITEM_MANUFACTURE_REQ`: the item workshop

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
The workshop's craft button does nothing; the packet is declined
([X2OfflineIgnore.cpp:60](X2Lib/Offline/X2OfflineIgnore.cpp#L60)).

### Diagnosis — CONFIRMED
Live: [`EGS_ITEM_MANUFACTURE_REQ` at GSUserInventory.cpp:2681](KncWX2Server/GameServer/GSUserInventory.cpp#L2681),
DB round trip at [`DBE_ITEM_MANUFACTURE_ACK`, `:2813`](KncWX2Server/GameServer/GSUserInventory.cpp#L2813).
The DB thread inserts with `SEnum::GIR_MANUFACTURE`
([GSGameDBThread.cpp:9896](KncWX2Server/GameServer/GSGameDBThread.cpp#L9896)) —
the same `Query_InsertItemList` the cube uses, so phase 40's period/seal work
applies here too if it landed first.

Recipes are client-side: `ManufactureItem.lua`, parsed into
`CX2ItemManager::ManufactureData` (`m_ManufactureType`, `m_ResultGroupID`, and a
`MaterialData` list of `m_MaterialItemID` / `m_MaterialCount`,
[X2ItemManager.h:63-75](X2Lib/X2ItemManager.h#L63)).

### What to do
1. Read `:2681` and `:2813`; the material consumption and the result insert are
   split across them.
2. Read the recipe out of `CX2ItemManager`'s parsed table rather than adding a
   loader, if an accessor exists.
3. Implement: validate materials and ED cost, consume, insert the result, reply
   with every touched slot and the real ED. `m_ResultGroupID` suggests the result
   may itself be a group draw — check whether it routes through
   `CXSLRandomItemManager`, and if so reuse `CX2OfflineRandomItem` rather than
   writing a second draw.
4. Remove the ignore entry.

### Trap
If the result is a group draw, the odds live in `RandomItemTable.lua` and the
existing draw code already implements `m_bGiveAll` and the class conditions.
Writing a second, simpler draw beside it is how the two start disagreeing.

### Exit test
Craft a recipe the save has materials for: materials leave, the result arrives,
ED drops by the cost. A recipe with missing materials is refused with everything
intact.

---

# Phase 45 — `EGS_SYNTHESIS_ITEM_REQ`, and an empty `UNHANDLED` bucket

**Flag:** `SERV_IRUHADEV_OFFLINE`

### Symptom
Synthesis is neither handled nor ignored, so it logs `*** UNHANDLED ***` — the
one bucket this project keeps empty on purpose, because any line in it is
supposed to be new information.

### Diagnosis — CONFIRMED
Live: [`EGS_SYNTHESIS_SOCKET_GROUPID_REQ` at GSUserInventory.cpp:8259](KncWX2Server/GameServer/GSUserInventory.cpp#L8259)
(no param), [`EGS_SYNTHESIS_ITEM_REQ` at `:8276`](KncWX2Server/GameServer/GSUserInventory.cpp#L8276),
and [`DBE_OPEN_SYNTHESIS_ITEM_ACK` at `:8358`](KncWX2Server/GameServer/GSUserInventory.cpp#L8358).
The `OPEN_` in the DB ACK's name says this is another draw-shaped feature, and
`SocketItemTable.lua` is in `data036/`.

### What to do
1. Read the three handlers. Decide honestly between implementing and ignoring —
   if the socket-synthesis system has no other offline support, an `IGNORED`
   entry with a real reason is a legitimate finished outcome.
2. Either way, the packet must leave the `UNHANDLED` bucket: add a handler, or
   add an `X2OfflineIgnore.cpp` entry naming what is missing.
3. **Close the batch.** Run a long session exercising every phase in 36-44, then:
   ```sh
   grep -E "UNHANDLED|EXCEPTION" offline_packets.log | sort | uniq -c | sort -rn
   grep "CENSUS" offline_server.log
   ```
   Every remaining declined ID should have a reason in `X2OfflineIgnore.cpp`.
4. Update `ISSUES_2.md` #7 and `MODS.md` with what landed and how to revert it.

### Trap
An `IGNORED` entry is a promise that nothing is silently dropped. Do not use it
to make a symptom go away — phase 38's stale `EGS_RESOLVE_ITEM_REQ` line is what
that looks like a year later.

### Exit test
`grep UNHANDLED offline_packets.log` is empty after a session that opened cubes,
used bag items, sealed and unsealed, exchanged, converted, crafted and cleared a
dungeon. The clean-exit census names every declined ID with a reason.
