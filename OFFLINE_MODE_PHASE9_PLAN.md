# Offline Elsword — phases 9-27: the play-test defects

Continuation of `OFFLINE_MODE_PLAN.md`. Phase 8 shipped a client that boots,
plays, and persists; the phase-8 exit criterion was "play everything and drain
the `UNHANDLED` bucket". Doing that produced `ISSUES.md` — 19 defects — and this
document turns each one into a phase that can be run as its own conversation.

**Written 2026-09-05, from the logs of the play session that produced
`ISSUES.md`** (`offline_packets.log` 31,419 lines, `offline_server.log` 2,188
lines, both timestamped 23:19-00:06 on 2026-09-04/05). Every "Evidence" block
below is quoted from those two files or from the tree; nothing in them is
inferred. The "Diagnosis" blocks say explicitly whether they are **CONFIRMED**
or a **HYPOTHESIS**, because four of the nineteen are not yet pinned down and
pretending otherwise is how a phase loses a day.

---

## 0. The finding that reframes the whole list

The phase-8 core loop was `grep UNHANDLED offline_packets.log`. Run against this
session it returns **three lines**:

```
2  EGS_SQUARE_LIST_REQ          (id=82)
1  EGS_GET_MY_BANK_INFO_REQ     (id=630)
```

and the `IGNORED` bucket returns **two**:

```
2  EGS_RESOLVE_ITEM_REQ         (id=460)
2  EGS_ENCHANT_ATTACH_ITEM_REQ  (id=557)
```

There are **zero** `EXCEPTION` lines.

So sixteen of the nineteen defects are *not* missing dispatch entries. They fall
into three other buckets, and **the bucket determines the debugging technique** —
this is the single most important thing to carry into each conversation:

| Bucket | How many | What it looks like in the log | How you debug it |
|---|---|---|---|
| **A. Deliberately refused** | 6 | handler ran, logged a refusal sentence, returned an error code | read the refusal, decide whether the data it wanted can now be sourced honestly |
| **B. Client never sent the packet** | 6 | the event name is *absent* from the `C->S` census entirely | client-side gate in `X2Lib`; the offline server is not involved at all |
| **C. Handled, replied OK, still wrong** | 4 | handler logged success; the screen disagrees | an ACK field is unset or a `KUnitInfo` member is never populated |
| **D. Genuinely unhandled** | 3 | `*** UNHANDLED ***` | the phase-8 loop; write a handler |

**Bucket B is the trap.** For issues 2, 3, 4, 9, 11 and 12 the client never put a
byte on the wire. Grepping the packet log for them returns nothing, which reads
exactly like "I haven't reproduced it yet" and will send you round the play-test
loop three times. Before touching `X2Lib/Offline/` for any of those, confirm the
event name's absence from the `C->S` census and go straight to the client UI code
that would have sent it.

The one-line check that tells you which bucket an issue is in:

```sh
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"
grep -a "C->S" "$DATA/offline_packets.log" | grep -c "EGS_WHATEVER_REQ"
```

Zero means bucket B. Non-zero, then
`grep -a "EGS_WHATEVER_REQ" "$DATA/offline_packets.log" | tail -3`
to see whether it was `HANDLED`, `IGNORED` or `UNHANDLED`.

## 0.1 A bucket-A refusal message can be stale. Verify it before you repeat it.

Every bucket-A defect is diagnosed from a sentence a handler logged about itself.
Those sentences were written in the phase that added the handler and **are not
re-checked when a later phase supplies the thing they say is missing.** At least
one is provably wrong today:

> `PET  feeding refused - the satiety table lives in .../PetData.lua, which is not packed`

`PetData.lua` **is** packed. It loads 118 egg rows in the same session, twenty
minutes before that line is printed. The refusal was written in phase 7; phase 7b
then added `CX2OfflinePetData`, which loads the file, binds
`AddPetCashFeedItemInfo` — and discards both of its arguments. Nobody updated the
sentence. Acting on it would have sent the user to repack a file that was already
packed. See phase 10.

So for phases 10, 12, 23, 24, 25, 26 and the warp note in Appendix A, before
telling the user something is missing:

1. **Check the loader actually failed.** Most of these subsystems log a distinct
   warning when their file is absent (`PET WARNING no egg -> pet rows`,
   `DROP loaded: ... row(s)`, `STAT 'StatTable.lua' loaded`). Absence of that
   warning means the file arrived.
2. **Check the binder records what it receives.** A `_LUA` binder with its
   parameters commented out is data loading successfully into a bin.
3. **Check the handler reads its request at all.** A signature of
   `const KEvent& /*kEvent*/` means the handler is a hardcoded refusal that cannot
   distinguish a case it could serve from one it could not.

Only if all three still point at missing data is the ask-the-user rule in play.

---

## 1. Shared preamble — paste this into every phase conversation

Each phase below is meant to open a fresh conversation. That conversation gets
`CLAUDE.md` automatically and this file is in the repo, so it does not need
pasting — one line starts a phase:

> Read §0, §0.1, §1 and the Phase N section of `OFFLINE_MODE_PHASE9_PLAN.md`, then
> do Phase N. Don't read the other phase sections — the file is ~1200 lines.

Substitute the phase number. Naming the sections matters: §0 is the bucket
triage, §0.1 is the stale-refusal check, §1 is this preamble, and skipping them is
how a phase gets debugged with the wrong technique.

Two useful variants:

- **To plan before building:** append *"Investigate and confirm the diagnosis
  first — don't edit anything until you've shown me what you found."* Worth doing
  for the phases marked HYPOTHESIS (14, 17, 18, 27).
- **To carry a finding forward:** phases 19-22 and 23-26 share root causes, so
  start the follow-up with *"Phase 19 found <the gate>; check whether Phase 20 is
  the same gate before implementing anything."*

The five standing facts each conversation needs, and which §1 exists to supply:

1. **Branch is `mods/offline-mod-2`.** All work is client-only unless the phase
   says otherwise; nothing here should touch `KncWX2Server/Common/`.
2. **Every edit goes behind a new `SERV_IRUHADEV_*` flag in `KTDXLIB/Always.h`**,
   one flag per phase, house comment block, `#endif SERV_IRUHADEV_FOO` style.
   The phase sections below each name the flag to use.
3. **Build, deploy, run** — the loop from `CLAUDE.md`, in full:
   ```sh
   TRUNK="f:/elsword stuff/elsword_2014/els_2014/ElswordFiles/ready_and_built/ProjectX2_SVN/ProjectX2_SVN/source/EU_CN_US/Trunk"
   DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"
   touch X2Lib/stdafx.cpp        # after ANY header edit, or the PCH eats it
   msbuild X2Lib/X2Lib_2010.vcxproj -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   msbuild X2/X2_2010.vcxproj       -p:Configuration=US_SERVICE -p:Platform=Win32 "-p:SolutionDir=$TRUNK/"
   ls -la X2/US_SERVICE/X2.exe   # judge by the artifact; post-build ALWAYS fails here
   cp X2/US_SERVICE/X2.exe "$DATA/X2_offline.exe"
   ```
   The game directory holds **fourteen** files, eight of them `X2*.exe` from
   earlier mods. The target is `X2_offline.exe` and nothing else. Verify the copy
   landed by reading the name and mtime programmatically, not by eyeballing `ls`.
4. **Reading the result is the verification; there is no test suite.**
   ```sh
   cd "$DATA"
   grep -aE "UNHANDLED|EXCEPTION" offline_packets.log | sort | uniq -c | sort -rn
   grep -a "CENSUS" offline_server.log
   tail -60 offline_server.log
   ```
   Note: the session that produced `ISSUES.md` ended without a `CENSUS` line — it
   was not a clean exit. If you want the census, quit through the menu.
5. **The two data rules from `CLAUDE.md` bind hardest in this batch.** Up to five
   of these nineteen are blocked on data that only ever lived on the server. Do
   not invent a success curve, a socket-option table or a warp table. Name the
   file, stop, and wait — the phase sections say exactly which file and exactly
   what to say.

   **But verify the block before asserting it — see §0.1.** Phase 10 was drafted
   as blocked on an unpacked `PetData.lua` and is not blocked at all.

**`els_db.sql` in the game directory is real player data.** It currently holds
three characters — `reyaa` (uid 12, lv 50, 241,388 ED), `wewswe` (uid 15, lv 1),
`caswe` (uid 16, lv 5). Never delete it to start clean; `X2OfflineDB` migrates
forward only.

---

## 2. Phase index

Ordered for execution, not by `ISSUES.md` number. Cheap-and-confirmed first,
because each one that lands shrinks the surface the harder ones are debugged
against. Phases within a group are independent of each other unless the notes
below say otherwise.

| Phase | `ISSUES.md` | Defect | Bucket | Confidence | Blocked on user? |
|---|---|---|---|---|---|
| **9**  | 14 | Expansion card "Item cannot be purchased" | A | CONFIRMED | no |
| **10** | 15 | Cannot feed pet | A | CONFIRMED | **no — the file IS packed; see below** |
| **11** | 7  | Cannot sort inventory | C | CONFIRMED (server side works) | no |
| **12** | 6  | Cannot dismantle equipment | A | CONFIRMED | maybe — resolve tables |
| **13** | 16 | Cobo Express "You cannot enter the village" | D | CONFIRMED | no |
| **14** | 1  | Pet summon/unsummon "Failed to create the pet" | A/C | HYPOTHESIS | no |
| **15** | 13 | Title image missing in field/dungeon | C | CONFIRMED | no |
| **16** | 19 | PvP rank not drawn in character list | C | CONFIRMED | no |
| **17** | 18 | Result screen shows no reward | C | HYPOTHESIS | no |
| **18** | 8  | "Fetch aura" emptied my wallet | C | HYPOTHESIS — **and the ED is intact** | no |
| **19** | 2  | Elixir cannot be used | B | CONFIRMED (bucket) | no |
| **20** | 11 | Stamina potion not working | B | CONFIRMED (bucket) | no |
| **21** | 9  | "Camilla's secret manual" unusable | B | CONFIRMED (bucket) | no |
| **22** | 12 | Skill notebook not working | B | CONFIRMED (bucket) | no |
| **23** | 4  | Cannot enhance equipment | A + B | CONFIRMED | **yes — enchant tables** |
| **24** | 3  | Cannot socket equipment | A + B | CONFIRMED | **yes — socket tables** |
| **25** | 5  | Cannot use magic amulet | A | CONFIRMED | **yes — attach tables** |
| **26** | 10 | Cannot add equipment attributes | A | CONFIRMED | **yes — attrib tables** |
| **27** | 17 | Regular drop ("Aqua") never drops | C | HYPOTHESIS | no |

**Phases 19-22 share one root cause and probably one flag.** They are four
symptoms of the same bucket-B gate. Run **phase 19 first**; it is the one that
identifies the gate. Phases 20-22 then become "does the same fix cover this item
too?" and may collapse into a single follow-up conversation. They are numbered
separately so that if they turn out to be four different gates you have four
slots.

**Phases 23-26 are one feature** — the item workshop — split four ways because
each needs a different table from the user. **Run phase 23 first**; it establishes
whether the tables can be sourced at all. If the answer is no, phases 24-26
become "improve the refusal so the player knows why", which is a real and much
smaller job, and they should be merged into one conversation at that point.

---

# Phase 9 — Expansion card cannot be bought (`ISSUES.md` #14)

**Flag:** `SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND`

### Symptom
Buying an inventory expansion card in the cash shop shows "Item cannot be
purchased".

### Evidence
```
[23:57:12.520] CASH  refused item 200751 - its effect (an inventory expansion or the
                     resurrection-stone counter) is not modelled offline, so the
                     purchase would take the cash and do nothing
```
Three times, at 23:57:12, 23:57:18 and 23:57:34.

### Diagnosis — CONFIRMED
Not a bug. `Handlers_Shop.cpp:270` refuses on purpose, via `IsNotModelled()`
against the `NOT_MODELLED[]` table at `Handlers_Shop.cpp:60-75`. Item 200751 is
one of the `INVENTORY_SLOT_ADD_ITEM_*` constants. The table's own comment says
why: the real server answers an expansion by returning
`m_mapExpandedCategorySlot`, and offline "nothing here changes the
`inventory_size` rows after creation".

This is the *right* refusal for phase 7 and the *wrong* one now — the data it
needs is not server-only. Inventory size is a row in the offline SQLite DB.

### What to do
1. Read the `inventory_size` schema in `X2Lib/Offline/X2OfflineDB.h`, and how
   `CX2OfflineInventory` seeds and reads it.
2. Find where the real server filled `m_mapExpandedCategorySlot` — grep
   `KncWX2Server/` — to learn the map's key/value meaning (category -> extra
   slots) and the per-card increment. **This is a read of server source for
   *structure*, which is allowed and is what `CLAUDE.md` means by "read it for
   structure, never for values."**
3. Implement: on purchase of an `INVENTORY_SLOT_ADD_ITEM_*` id, bump the matching
   `inventory_size` row, and populate `m_mapExpandedCategorySlot` on every ACK
   that carries it (character load included, or the expansion is forgotten on
   relog).
4. Remove only the `INVENTORY_SLOT_ADD_ITEM_*` entries from `NOT_MODELLED[]`.
   **Leave 127030** — the resurrection stone is a character counter, a separate
   and unrelated job.

### Trap
The expansion must survive a relog. If you bump the row but do not send
`m_mapExpandedCategorySlot` at character load, the slots come back on purchase and
vanish on restart, which looks like the DB write failed and is not.

### Exit test
Buy an expansion card. It succeeds; the category shows the extra slots; quit and
relaunch; the slots are still there; `select * from inventory_size` shows the
bumped row.

### What actually happened

No new packets were involved - this was bucket A exactly as filed, a single
handler refusing on purpose. What the plan got right and wrong once the server
source was actually read for structure:

- **The per-card increment (8) is a real, citable constant, not a value to
  invent.** `CXSLInventory::SLOT_COUNT_ONE_LINE` at
  `KncWX2Server/Common/X2Data/XSLInventory.h:54` is a compile-time `= 8` in a
  shared header, not data behind a stored procedure - so unlike the phase 9-27
  batch's DB-blocked phases, this one needed no ask-the-user step at all. It is
  hardcoded in `Handlers_Shop.cpp` with the file:line citation, the same shape
  as the `BattleFieldServerData.lua` fallback `CLAUDE.md` describes.

- **There is a second, unrelated implementation of this same item ID in the
  tree, and it is not the one that applies.**
  `GSUserCashShop_Global.cpp:769-881` (the CN/Global variant) expands the
  category the instant `EGS_BUY_CASH_ITEM_REQ` is handled - no deposit, no
  claim step. That is not the flow phase 7 built offline around (US goes
  through the two-step deposit/claim `X2CashShop.cpp:3580` describes), so
  reading `_Global.cpp` first would have produced a purchase-time
  implementation that never matches what the client actually waits on. The
  real match is `GSUserFunction.cpp:13523-13639`, inside the handler for the
  **claimed** item ID (`m_usEventID = EGS_GET_PURCHASED_CASH_ITEM_REQ`) - same
  file that also confirmed the six categories `INVENTORY_SLOT_ADD_ITEM_EQUIP`
  etc. expand individually while bare `INVENTORY_SLOT_ADD_ITEM` (200750, the
  "all categories" bundle card the plan didn't call out by name) expands all
  six from one card, each by the same 8.

- **The relog trap in the plan doesn't apply, and no extra send was needed.**
  The plan assumed the fix would need to hand `m_mapExpandedCategorySlot` back
  to the client at character load or the DB-vs-client picture would desync on
  restart. Reading `Handlers_Unit.cpp:519-537` (`KEGS_SELECT_UNIT_1_NOT`, phase
  5's own char-load packet) shows the client is never told slot counts as
  deltas at login at all - `kNot.m_mapInventorySlotSize` there is
  `pInven->GetSlotSizes()`, the *absolute* size map, freshly loaded from the
  same `inventory_size` table the purchase writes to. So persisting the bump
  is sufficient by construction; there was nothing further to wire up, and the
  trap the plan warned about was already closed by phase 5's own design.

- **Both `INVENTORY_SLOT_ADD_ITEM_*` families were removed from
  `NOT_MODELLED[]`, not just the plan's six.** The plan said "remove only the
  `INVENTORY_SLOT_ADD_ITEM_*` entries" without saying whether that covered the
  `_EVENT` twins (`INVENTORY_SLOT_ADD_ITEM_EQUIP_EVENT` etc., IDs
  60002281-60002286). The claim-time code added here maps both families to the
  same category identically, so both were unrefused rather than leaving the
  `_EVENT` six as a narrower follow-up; 127030 (the resurrection stone) is
  still refused, unchanged from the plan.

- **Implementation shape**: `CX2OfflineDB::ExpandInventorySize` (bump the row,
  capped at `INVENTORY_SLOT_MAX_NUM`) under `CX2OfflineInventory::
  ExpandCategorySlot` (also grows the in-memory slot vector, mirroring
  `KInventory::ExpandSlot`'s append-only resize so existing slot indices don't
  move), called from the claim handler the same way the existing class-change
  branch already claims-without-carrying: take the deposit line, don't touch
  the bag, fill `m_mapExpandedCategorySlot` in the ACK.

- **Build**: `X2Lib_2010.vcxproj` then `X2_2010.vcxproj`, both `US_SERVICE`,
  0 errors. Deployed to `X2_offline.exe`; confirmed by size/mtime against the
  seven other `X2_*.exe` builds already in the game directory.

- **Not yet done**: the exit test itself is a real play-test (buy a card,
  relog, check the DB row) that needs a human at the client - not run as part
  of this phase.

---

# Phase 10 — Cannot feed pet (`ISSUES.md` #15)

**Flag:** `SERV_IRUHADEV_OFFLINE_PET_FEED`

### Symptom
Feeding a pet fails. The dialog reads "Failed to create the pet", which is
misleading — see below.

### The refusal message is FALSE. `PetData.lua` is packed and loading.
The user confirmed the pack, and the log corroborates it:
```
[23:23:18.992] PET  loaded: 118 egg(s) -> pet, 28 stone(s) -> mount, 1 random egg(s)
[23:23:18.992] PET  hatched item 84001273 into pet 99 'Pertaa' (petUID=1)
```
Those 118 rows *are* `PetData.lua` executing. `CX2OfflinePetData::EnsureLoaded`
logs `WARNING no egg -> pet rows ... until PetData.lua is packed` when the file is
missing, and that warning never fired.

**Do not ask the user to pack anything in this phase.** The earlier draft of this
plan did, on the strength of the refusal string alone; the string is a phase-7
artifact that phase 7b invalidated and nobody updated.

### Evidence
```
[23:58:31.644] PET  feeding refused - the satiety table lives in
                    KncWX2Server/ServerResource/US/PetData.lua, which is not packed
[23:58:44.328] PET  feeding refused - ...
```
`EGS_FEED_PETS_REQ` x2, both `HANDLED`.

### Diagnosis — CONFIRMED, and the cause is two stale pieces of phase ordering

**1. The handler never reads the request.** `Handlers_Social.cpp:1501`,
written in phase 7:
```c
// Feeding needs the feed table - which item raises satiety by how much -
// and that is AddPetCashFeedItemInfo in PetData.lua, server-side. Refusing
// rather than guessing a number keeps the food in the bag.
CX2OfflineLog::Server( L"PET  feeding refused - the satiety table lives in ..." );
kAck.m_iOK     = NetError::ERR_PET_00;
kAck.m_sSatiety = 0;
```
It is an unconditional refusal. It was correct in phase 7, when nothing loaded
`PetData.lua` at all.

**2. Phase 7b loads the table and then discards it.** `X2OfflinePetData.cpp`:
```c
void CX2OfflinePetData::AddPetCashFeedItemInfo_LUA( int /*iItemID*/, int /*iIncreaseSatiety*/ )
{
    // Recorded nowhere on purpose - see the header. Bound so the call does not
    // abort PetData.lua partway through.
}
```
Both parameters are commented out. There is no `m_mapFeedItem` member. The same
is true of `AddSpecialFeedItemID_LUA`, `AddPetSatietyDecreaseFactor_LUA`,
`IncreaseSpecialFeedIntimacyRate_LUA` and `DecreaseSpecialFeedIntimacyRate_LUA` —
all bound purely so the chunk does not abort, all empty.

`X2OfflinePetData.h`'s header comment claims these are "Recorded but unused: the
offline feed handler already works off the item the client hands it". **That
sentence is wrong** and is what makes the bug invisible on a read-through — the
feed handler works off nothing at all. Fix the comment as part of this phase.

### The data, in full — it is five rows and it is already in the packed file
From `KncWX2Server/ServerResource/US/PetData.lua`:
```lua
SetNewPetInfo( 0, 3840, 0, 0, 0 )              -- a new pet starts at satiety 3840

AddPetCashFeedItemInfo( 500030,    468 )       -- El tree seed
AddPetCashFeedItemInfo( 500040,    936 )       -- El tree fruit
AddPetCashFeedItemInfo( 83000010,  936 )       -- (CN) El tree fruit
AddPetCashFeedItemInfo( 210000074, 1404 )      -- SERV_PET_EVENT_EVOLUTION_ITEM
AddPetCashFeedItemInfo( 84001302,  1404 )      -- (CN) bamboo

AddSpecialFeedItemID( 500030 ) ... ( 84001302 )   -- the same five
AddPetSatietyDecreaseFactor( 0, 1 ) ( 1, 1 ) ( 2, 1 ) ( 3, 0.5 )
IncreaseSpecialFeedIntimacyRate( 0.0201 )
DecreaseSpecialFeedIntimacyRate( 0.0501 )
```
Nothing here needs guessing, and nothing here needs the user.

### The packets are tiny
```c
DECL_PACKET( EGS_FEED_PETS_REQ )  { UidType m_iItemUID; };
DECL_PACKET( EGS_FEED_PETS_ACK )  { int m_iOK; short m_sSatiety;
                                    std::vector< KInventoryItemInfo > m_vecInventorySlotInfo; };
```
**Note what the REQ does *not* carry: a pet UID.** It names only the food item, so
the server feeds *the currently summoned pet*. The handler therefore needs to know
which pet is summoned — see the dependency below.

### What to do
1. Add `std::map< int, int > m_mapFeedItem` and `std::set< int > m_setSpecialFeed`
   to `CX2OfflinePetData`; record in the two `_LUA` binders instead of discarding;
   add getters. Keep the decrease factors and intimacy rates too — they are four
   and two more lines and the next pet phase will want them.
2. Correct the header comment that says the feed handler already works.
3. Rewrite `Handler_EGS_FEED_PETS_REQ`: read `m_iItemUID`, look up the item in the
   bag, reject with the food intact if it is not one of the five, raise the
   summoned pet's satiety by the table value, clamp at the maximum, consume one
   food, persist, and fill all three ACK fields — including
   `m_vecInventorySlotInfo`, or the bag will not redraw (this is the same class of
   bug as phase 11).
4. Log the item, the amount and the new satiety, so the next play-test can read it.

### Dependency — check before starting
Feeding targets the summoned pet, and **phase 14 is about summoning being broken**.
If no pet can be summoned, feeding may have nothing to target and this phase
cannot reach its exit test. Check how the offline server tracks the summoned pet
first; if it does not track one, do phase 14 before this one.

### Also fix here — the dialog string
"Failed to create the pet" for a *feeding* failure is the client mapping
`NetError::ERR_PET_00` onto the wrong string. Whatever else this phase does, a
refusal that names the wrong operation should stop. **This overlaps phase 14** —
the same dialog appears there, and whichever phase runs second inherits a narrower
problem.

### Exit test
Feed a summoned pet an El tree seed: satiety rises by 468, the seed count drops by
one in a bag that redraws, and the new satiety survives a relog. No `PetData.lua`
message anywhere in `offline_server.log`.

### What actually happened

No new packets either - still bucket A, but the plan's "What to do" undersold
the actual gap. Feeding needed one piece of state the plan never named, and the
"fix the dialog string" line turned out to have a real, already-built fix
sitting in the shared header:

- **The dependency check resolved clean, without needing phase 14.** The plan
  flagged "check how the offline server tracks the summoned pet first; if it
  does not track one, do phase 14 before this one" as an open question.
  Reading `Handler_EGS_SUMMON_PET_REQ` (`Handlers_Social.cpp`) answered it:
  summoning a pet by a real, non-zero UID already works today (it finds the
  row, updates `m_tLastSummonDate`, replies `NET_OK`) - only *unsummoning* is
  broken, and that is a narrower, separate bug (below). Since the exit test only
  needs a pet summoned, not un-summoned, phase 10 did not need phase 14 first.

- **"Tracks the summoned pet" was the real missing piece, and the plan didn't
  say how to build it because nothing existed to build on.**
  `EGS_FEED_PETS_REQ` carries only the food item's UID, never a pet UID, so the
  server has to already know which pet is out. Before this phase,
  `KOfflineSession` had no such field at all - not even something to read and
  find broken. Added `UidType m_nSummonedPetUID`, written by
  `Handler_EGS_SUMMON_PET_REQ` on every successful summon and read by the feed
  handler to look up the live `KOfflinePetRow`. This is new state, not a bug
  fix to existing state, and it is the piece the plan's "What to do" section
  skipped over by assuming the summoned pet would just be "the summoned pet."

- **`EGS_SUMMON_PET_REQ` with UID 0 means "put the pet away", confirmed from
  the client, not guessed.** `CX2PetManager::Handler_EGS_SUMMON_PET_REQ( 0 )`
  is the exact call `PCUM_SUMMON_CANCEL` makes
  (`X2PetManager.cpp:319`). The offline handler's match loop can never find a
  pet row with UID 0, so every unsummon request already fails with
  `ERR_PET_00` today - and worse, the client's own ACK handler
  (`X2PetManager.cpp:2253`) shows the *correct* success shape for that case is
  `iOK = NET_OK` with `m_kSummonedPetInfo.m_iPetUID == 0`, not an error at all.
  **Left alone on purpose** - this is squarely phase 14's declared symptom
  ("Pet summon/unsummon `Failed to create the pet`"), and fixing it here would
  have widened phase 10 into phase 14's scope for no gain the exit test needs.
  Whoever runs phase 14 should start from this paragraph rather than
  rediscovering it.

- **The dialog-string fix the plan asked for already has the right error codes
  sitting unused in the shared enum - no `Common/` edit, no server rebuild.**
  `KncWX2Server/Common/NetError_def.h` already defines `ERR_PET_06` ("no
  summoned pet"), `ERR_PET_10` ("satiety full"), `ERR_PET_11` (generic feed
  failure) and `ERR_PET_18` ("not a food item this pet can eat") - a whole
  family of pet-specific codes nothing in the offline handlers had ever
  referenced. `IsValidPacket()` (`X2Main.cpp:6914`) has no special case for any
  of them, so each one falls through to the generic popup that prints
  `NetError::GetErrStrF(enumID)` and returns `false` - exactly the "show the
  right refusal, don't touch the ack" shape every other refusal in this file
  already uses. Swapped the feed handler's `ERR_PET_00` for `ERR_PET_06` (no
  pet out), `ERR_PET_18` (wrong item) and `ERR_PET_10` (already full), instead
  of inventing a new code or leaving the wrong-operation string in place.

- **The satiety cap (4800) is a real, citable client constant, the same shape
  `CLAUDE.md`'s permitted-fallback rule describes.**
  `CX2PetManager::MAX_OF_SATIETY` at `X2Lib/X2PetManager.cpp:14` is a
  compile-time `= 4800.0f` the client itself divides every satiety bar by.
  Cited in a comment at the clamp site rather than pulled in via a
  `CX2PetManager` include, since `X2Lib/Offline` doesn't otherwise depend on
  the client UI/manager layer and this is one `int` literal, not a live
  dependency.

- **Recorded, not yet wired: satiety decay and the "special" feed gate.**
  `AddPetSatietyDecreaseFactor` and the two
  `Increase`/`DecreaseSpecialFeedIntimacyRate` calls are now captured (per the
  plan's "next pet phase will want them") with getters, but nothing in this
  phase reads them back - satiety decay over time and `ERR_PET_12` ("special"
  pets eating only tree fruit/seed) are still a later phase's job, consistent
  with `SERV_TRANSFORM_PET` being off in this build regardless.

- **Build toolchain note for future phases**: `msbuild` is not on `PATH` in
  this environment - use
  `C:\Windows\Microsoft.NET\Framework\v4.0.30319\MSBuild.exe` directly (PowerShell,
  not the bash `msbuild X2Lib/...` form `CLAUDE.md`'s loop shows). More
  importantly, **a `SolutionDir` value ending in a bare `\` right before the
  closing quote breaks silently**: PowerShell re-quotes an argument containing
  spaces for the native argv parser, and a trailing `\"` in that reconstructed
  command line is read as an *escaped quote*, not backslash-then-quote - the
  build still runs, `SolutionDir` is simply wrong, and the result is the same
  `cannot open include file: 'd3dx9.h'` error `CLAUDE.md` already warns about
  for a missing `SolutionDir`, which reads exactly like the flag wasn't passed
  at all even though it was. Forward slashes throughout (`"/p:SolutionDir=$TRUNK/"`
  with `$TRUNK` itself built from forward slashes) sidestep the whole class of
  bug, matching the advice `CLAUDE.md` already gives for the opposite reason
  (fighting a trailing `\` through a shell).

- **Not yet done**: the exit test is a real play-test (feed a summoned pet,
  watch the satiety bar and the bag, relog) that needs a human at the client -
  not run as part of this phase. `X2Lib_2010.vcxproj` then `X2_2010.vcxproj`
  both built clean under `US_SERVICE`, deployed to `X2_offline.exe`, confirmed
  by size/mtime.

---

# Phase 11 — Cannot sort inventory (`ISSUES.md` #7)

**Flag:** `SERV_IRUHADEV_OFFLINE_INVEN_SORT`

### Symptom
Pressing sort does nothing visible.

### Evidence
```
[23:53:40.773] ITEM  sorted category 1
[23:53:41.594] ITEM  sorted category 1
[23:53:41.917] ITEM  sorted category 1
[23:53:42.496] ITEM  sorted category 1
```
`EGS_SORT_CATEGORY_ITEM_REQ` x4, all `HANDLED`. Four presses in three seconds —
the player pressing it again because nothing happened.

### Diagnosis — CONFIRMED that the server half runs
`Handlers_Inventory.cpp` `Handler_EGS_SORT_CATEGORY_ITEM_REQ` calls
`CX2OfflineInventory::SortCategory()`, which returned `true` every time, and
replied `NET_OK`. So either the sort happened in the DB and the client did not
redraw, or `SortCategory()` reports success without moving anything.

Two candidates, in order of likelihood:

1. **`kAck.m_vecUpdatedInventorySlot` is empty or wrong.** The client applies the
   sort by walking that vector; an empty one is a no-op that reports success.
   Read what `SortCategory()` pushes into it, and compare against what the
   client's `EGS_SORT_CATEGORY_ITEM_ACK` handler in `X2Lib` expects — in
   particular whether it wants *every* slot in the category or only the moved
   ones, and whether the slot indices are category-relative or absolute.
2. **`SortCategory()` returns `true` without moving anything.**

### How to tell them apart without a rebuild
Query the DB directly between two sort presses. If the rows reorder, it is
candidate 1 (the ACK); if they do not, candidate 2 (the sort).

### Trap
`ITEM move 9/13 -> 2/6, 2 slot(s) changed` at 23:35:27 shows the *manual* slot
move path builds its updated-slot vector correctly and the client redraws from
it. That handler is the working reference — diff the two.

### Exit test
Press sort; the category visibly reorders; the order survives a relog.

---

# Phase 12 — Cannot dismantle equipment (`ISSUES.md` #6)

**Flag:** `SERV_IRUHADEV_OFFLINE_ITEM_RESOLVE`

### Symptom
Dismantling equipment does nothing.

### Evidence
```
2  EGS_RESOLVE_ITEM_REQ  (id=460)  --- IGNORED ---
```

### Diagnosis — CONFIRMED
`X2OfflineIgnore.cpp` carries `{ L"EGS_RESOLVE_ITEM_REQ", L"the item workshop is
not implemented offline" }`, added in phase 5. The dispatch has no handler, so
`Reason()` matched and the packet was declined with that sentence. The client got
**no reply at all** and sat there.

### What to do
1. Find what dismantling actually yields. Grep `KncWX2Server/` for the resolve
   path and find out whether the output materials come from a **server table**, a
   **field on `ItemTemplet`** the client already has, or the item's own recipe.
   This determines whether this is a real implementation or another
   ask-the-user-to-pack.
2. If the yield is client-side data: implement `Handler_EGS_RESOLVE_ITEM_REQ`,
   remove the ignore rule, consume the item, insert the materials, and reply with
   the updated slots.
3. If it is server data: **keep the ignore rule** but make the client show
   something. An `IGNORED` packet produces no reply, and the UI hangs waiting; a
   refusal ACK with an error code at least closes the dialog. That is a real
   improvement and a legitimate outcome for this phase.

### Trap
The `NOT DRIVEN OFFLINE` census line at 23:19:42 flags two title sub-quests
(`clearType=13`, `clearType=14`) as depending on enhancement and socketing. Do
not widen this phase to chase them — that is phases 23-24.

### Exit test
Dismantle an item: either it yields materials and the item is gone, or a dialog
says dismantling is unavailable offline and the item is untouched. Silence is the
failure.

---

# Phase 13 — Cobo Express: "You cannot enter the village" (`ISSUES.md` #16)

**Flag:** `SERV_IRUHADEV_OFFLINE_SQUARE_LIST`

### Symptom
Using Cobo Express shows "You cannot enter the village".

### Evidence
```
[23:43:43.680] C->S  GS  EGS_SQUARE_LIST_REQ  (id=82, 5 bytes)  *** UNHANDLED ***
[23:43:44.104] C->S  GS  EGS_SQUARE_LIST_REQ  (id=82, 5 bytes)  *** UNHANDLED ***
```
Two attempts, 400ms apart. This is one of only three `UNHANDLED` events in the
entire session.

### Diagnosis — CONFIRMED as the packet; verify the causal link
`EGS_SQUARE_LIST_REQ` gets no reply, so the client's village-entry flow times out
into that dialog. Confirm by reproducing and matching the timestamp to the moment
the dialog appears — 5 bytes of request and a 400ms retry is the shape of a UI
that asked twice and gave up.

### What to do
`EGS_SQUARE_LIST_ACK` (`ClientPacket.h:2411`) is small:
```c
int                          m_iOK;
UINT                         m_nTotalPage;
UINT                         m_nViewPage;
std::vector< KSquareInfo >   m_vecSquareInfo;
```
A square is a shared village instance. Offline there is exactly one player, so the
honest answer is **one square, population 1, page 1 of 1** — not an empty list,
which the client may read as "no village available" and produce the same dialog.
Read `KSquareInfo`'s fields and fill every one; look at how phase 3's field-entry
handlers built their equivalents.

Then check whether `EGS_JOIN_SQUARE_REQ` / `EGS_LEAVE_SQUARE_REQ` /
`EGS_SQUARE_UNIT_SYNC_DATA_REQ` are handled — if entering the square becomes
reachable, they are the next thing the client will send, and none was exercised in
this session because entry failed first. **Expect this phase to uncover a second
round of `UNHANDLED`.** That is success, not scope creep; finish the entry path.

### Related, same area, do NOT fold in
```
[23:59:57.476] WARP  button warp 20000 refused - the warp table is server data;
                     use the world map, which goes through EGS_STATE_CHANGE_FIELD_REQ
```
Four of these. A separate deliberate refusal, not in `ISSUES.md`, blocked on the
server warp table. Note it and leave it.

### Exit test
Cobo Express opens the village; the character is in it; `grep SQUARE
offline_server.log` shows the entry; no `UNHANDLED` remains for the square family.

---

# Phase 14 — Pet summon/unsummon "Failed to create the pet" (`ISSUES.md` #1)

**Flag:** `SERV_IRUHADEV_OFFLINE_PET_SUMMON`

### Symptom
The pet summon/unsummon button misbehaves and shows "Failed to create the pet".

### Evidence
```
[23:23:18.992] PET  hatched item 84001273 into pet 99 'Pertaa' (petUID=1)
[23:51:01.546] PET  hatched item 500610 into pet 29 'hihoo'  (petUID=2)
[23:23:18.992] PET  loaded: 118 egg(s) -> pet, 28 stone(s) -> mount, 1 random egg(s)
[23:23:18.992] PET  NOTE 7 egg row(s) and 10 stone row(s) name a pet or mount this
                    client build has no enum for - the .lua files shipped in
                    ServerResource are newer than the tree
```
Census: `EGS_COMMANDS_FOR_PETS_REQ` **x12,876**, `EGS_SUMMON_PET_REQ` x13,
`EGS_GET_PET_LIST_REQ` x13, `EGS_CREATE_PET_REQ` x2. All `HANDLED`.

### Diagnosis — HYPOTHESIS, three candidates
Creation works; both hatches succeeded and the DB has the rows. So the failure is
downstream, and the dialog string is probably wrong (same as phase 10). Ranked:

1. **`EGS_COMMANDS_FOR_PETS_REQ` at 12,876 requests is the loudest signal in the
   whole log** — roughly 41% of every packet sent. That is a client retry storm.
   Something in the summoned-pet loop is asking every frame and never being
   satisfied. Start here: find what the client does with
   `EGS_COMMANDS_FOR_PETS_ACK` and which field, unset, would make it ask again.
2. **The summon ACK is fine but the client cannot build the pet.** Pet 99
   `'Pertaa'` may be one of the rows the `NOTE` warns about — newer
   `ServerResource` data naming a pet this 2014 build has no enum or mesh for.
   Cross-check pet id 99 against the client's pet enum. If it is absent, the
   *correct* fix is to refuse hatching that egg with a clear message, not to
   summon a pet that cannot be drawn — "skip, never guess".
3. **The error string is mismapped**, as in phase 10.

### How to settle it in one play-test
Add a temporary `SERV_IRUHADEV_PET_SUMMON_DEBUG` flag logging, once per second
(throttled — 12,876 unthrottled lines will bury the log): the pet id and uid on
each `EGS_COMMANDS_FOR_PETS_REQ`, and every field of the summon ACK. Then summon
`'hihoo'` (pet 29) and `'Pertaa'` (pet 99) in turn — if one works and the other
does not, it is candidate 2 and the enum is the answer.

### Trap
Do **not** start by rewriting the summon handler. Two of the three candidates are
not in it.

### Exit test
Summon a pet: it appears, follows, and unsummons; `EGS_COMMANDS_FOR_PETS_REQ`
drops from ~12,876 to a sane rate over a comparable session.

---

# Phase 15 — Title image missing in field/dungeon (`ISSUES.md` #13)

**Flag:** `SERV_IRUHADEV_OFFLINE_TITLE_IN_FIELD`

### Symptom
The equipped title does not render above the character in field or dungeon. It
presumably does render elsewhere, since the title system otherwise works.

### Evidence
Titles load and award correctly:
```
[23:19:42.737] TITLE  loaded for unitUID=12: 127 mission(s), 4 title(s)
[23:33:16.024] TITLE  mission 30080 complete -> title 30080 awarded
[23:49:48.056] TITLE  mission 30170 complete -> title 30170 awarded
```
But `grep -E "Title" X2Lib/Offline/Handlers_Unit.cpp` returns only the phase-6
mission/title *list* block (lines 681-696). Nothing anywhere in `X2Lib/Offline/`
sets a title id on a unit-info struct.

### Diagnosis — CONFIRMED
`KUnitInfo` and its siblings carry the equipped title in `m_iTitleID` /
`m_sTitleID` (`CommonPacket.h:1427`, `:1429`, `:2198`, `:2641`, `:4059`), and the
offline server never populates any of them. The field renderer draws from that
struct, so it draws no title.

### What to do
1. Find which struct the field/dungeon renderer reads the title from — the five
   sites above belong to different packets, and only one or two matter. Trace
   back from the client's title-drawing code rather than forward from the packet.
2. Find where the equipped title is stored in `X2OfflineDB` (the title system is
   phase 6; whether the *equipped* one is persisted needs checking).
3. Populate `m_iTitleID` on every path that builds a unit-info for field entry,
   character load, and the P2P unit sync.

### Trap
There are at least five title members across `CommonPacket.h`, two of them `short`
and one commented out (`:2200`). Setting the wrong one compiles and does nothing.
Confirm by reading the client's consumer, not by picking the best-matching name.

### Exit test
Equip a title; enter a dungeon; the title renders above the character; it survives
a relog.

---

# Phase 16 — PvP rank not drawn in the character list (`ISSUES.md` #19)

**Flag:** `SERV_IRUHADEV_OFFLINE_PVP_RANK`

### Symptom
In the character-select list, the PvP rank shows as a small black box to the left
of the character name instead of a rank emblem.

### Evidence
`grep -i pvp X2Lib/Offline/Handlers_Unit.cpp X2Lib/Offline/Handlers_Login.cpp`
returns **nothing**. The relevant members exist and are never written:
`m_iPVPEmblem` (`CommonPacket.h:1370`, `:3934`), `m_cPVPEmblem` (`:4045`),
`m_uiPVPRanking` (`:3937`), `m_iPvPPoint` (`:576`).

### Diagnosis — CONFIRMED
The character-list ACK leaves the PvP emblem at its constructor default (0), and
the client renders emblem 0 as an empty/black swatch rather than as "no rank".

### What to do
Decide what "no PvP rank" means to this client and set the emblem to *that*, not
to 0. Two ways to find it:
- Read the client's emblem-to-texture lookup and see which value maps to the
  no-rank case or to a skipped draw.
- Grep `KncWX2Server/` for where `m_cPVPEmblem` is assigned, to see what the real
  server sent for a character that never played PvP. **Structure, not values** —
  but a default is structure.

Then set it on every path that builds the character-list rows, and persist
`m_iPvPPoint` in the DB if the emblem is derived from it.

### Scope
PvP itself stays off — `X2OfflineIgnore.cpp` refuses the whole `PVP` family with
"no PvP offline; a match is peer-to-peer and there is no peer", and that stands.
This phase is *only* about the list not drawing a black box.

### Exit test
The character list shows a proper unranked emblem, or nothing, for all three
characters.

---

# Phase 17 — Result screen shows no reward (`ISSUES.md` #18)

**Flag:** `SERV_IRUHADEV_OFFLINE_RESULT_REWARD`

### Symptom
After a dungeon, the result screen's reward panel is blank. EXP gained, damage
dealt and combo are correct; everything else is not.

### Evidence
`EGS_RESULT_SUCCESS_REQ` x4, `EGS_STATE_CHANGE_RESULT_REQ` x4, `EGS_END_GAME_REQ`
x4, all `HANDLED`. The rewards themselves are real and are being credited:
```
[23:20:03] REWARD  unitUID=12 +90 exp (38727329 total), +0 ED (56270 total), lv=50
[23:20:03] DROP    monster 80 dropped item 99610 (dropUID=7)
[23:20:11] DROP    picked up item 99610 (dropUID=18)
```
745 `DROP` lines and 197 `REWARD` lines in one session.

### Diagnosis — HYPOTHESIS, but a narrow one
The economy works and the screen does not, so this is an ACK-field problem in
`Handler_EGS_RESULT_SUCCESS_REQ` (`X2OfflineServer.cpp:537` -> `Handlers_Room.cpp`).
The fields that *do* show — EXP, damage, combo — are exactly the ones the client
accumulates locally during the match. The fields that do not are exactly the ones
the server was supposed to report back. That split is the tell.

### What to do
1. Read `KEGS_RESULT_SUCCESS_ACK` in `KncWX2Server/Common/ClientPacket.h` field by
   field and check each against what the handler sets. Expect several left at
   their constructor defaults.
2. The per-run totals already exist — `GAME -> +85 exp, +84 ED (key=40000
   battlefield, run total 403 exp / 416 ED)` shows the handler tracks a run total.
   Wire the tracked totals and the picked-up item list into the ACK.
3. `DROP picked up item 99610 (dropUID=18)` means the acquired-item list is
   available too.

### Trap
`Handlers_Room.cpp` is 105 KB, the largest file in `X2Lib/Offline/`. Grep to the
handler; do not read it top to bottom.

### Exit test
Clear a dungeon; the result screen lists ED earned, items acquired and EXP, and
they match the `REWARD`/`DROP` lines for that run.

---

# Phase 18 — "Fetch aura" emptied my wallet (`ISSUES.md` #8)

**Flag:** `SERV_IRUHADEV_OFFLINE_WALLET_DISPLAY`

### START HERE: the money is not gone
`els_db.sql` right now:

| unit_uid | nickname | level | ED |
|---|---|---|---|
| 12 | reyaa | 50 | **241,388** |
| 15 | wewswe | 1 | 132 |
| 16 | caswe | 5 | 14,302 |

241,388 is the same value `reyaa`'s `REWARD` line trail ends on. **No ED was
lost.** The cash wallet is likewise intact — every purchase in the session logged
`wallet unchanged at 999999`, thirteen times.

So this is most likely a **display** defect, and the first job of this phase is to
establish *which* wallet appeared empty and *on which screen*. Ask the user before
implementing anything; "emptied my wallet" and "showed zero" need different fixes
and only one of them is a real problem.

### Evidence
```
[23:23:03.093] CASH  bought 1 deposit line(s) listed at 1 cash; wallet unchanged at 999999
[23:23:10.355] CASH  claimed line 20: 1 x item 84001273 into the bag
[23:34:19.948] CASH  claimed line 22: 1 x item 75000600 into the bag
```
The offline cash wallet is deliberately not charged. There is no ED-spend log line
anywhere in the session.

### Diagnosis — HYPOTHESIS
Most likely: whatever "fetch aura" is (a random box? a gacha?) opens a UI whose
ACK carries a balance field the offline server leaves at 0, and the UI shows that
instead of the real balance. Candidates, in order:
1. `X2Lib/Offline/X2OfflineRandomItem.cpp` — the random-item/box path.
2. Whichever ACK that UI reads its balance from, unset.
3. A genuine ED debit path with no log line — least likely, since the DB
   disagrees, but rule it out.

### What to do
Reproduce with the DB open. Read the ED before, open the fetch aura, read it
after. If the number is unchanged, this is cosmetic and the fix is to fill the
balance field. If it *does* change, this becomes a data-loss bug and jumps to the
front of the queue — **stop and tell the user immediately.**

### Exit test
Open the fetch aura; the displayed balance matches `select ed from unit`, before
and after.

---

# Phase 19 — Elixir cannot be used (`ISSUES.md` #2)

**Flag:** `SERV_IRUHADEV_OFFLINE_ITEM_USE_GATE`

**Run this phase before 20, 21 and 22 — it is the one that finds the gate.**

### Symptom
Elixirs cannot be used.

### Evidence — this is the important part
`EGS_USE_ITEM_IN_INVENTORY_REQ` appears **once** in the entire session, and that
one succeeded:
```
[23:24:09.139] ITEM  used item 500720 from the bag
```
There is **no** `ITEM refused use of item ...` line anywhere — and the handler
logs one on every refusal (`Handlers_Inventory.cpp`, the `GetCanUseInventory()`
branch). Quick-slot use works fine and often:
```
[23:27:30.645] ITEM  quick slot 1 used item 130166
[23:30:29.469] ITEM  quick slot 0 used item 135402
```
30 `EGS_USE_QUICK_SLOT_REQ`, all handled.

### Diagnosis — CONFIRMED bucket, unknown gate
**The client never sent a packet.** The offline server is not involved. Something
in `X2Lib`'s inventory UI refused the click before it reached the wire.

### What to do
1. Find the client-side use path: the inventory item's double-click / use handler
   in `X2Lib`, upstream of where `EGS_USE_ITEM_IN_INVENTORY_REQ` is sent. Grep for
   the send site and walk backwards through every early return.
2. Each early return is a candidate gate. Likely shapes, given what offline leaves
   unset: a cooldown table the server used to own; a `CanUse` check against a
   state the offline unit-info never fills; a village/field-state restriction; or
   a per-item-type branch that routes elixirs to a *different* packet the dispatch
   may not handle.
3. **Prove which gate fires** — do not reason about it. Add a
   `SERV_IRUHADEV_ITEM_USE_DEBUG` line at each early return naming itself, build,
   click the elixir once, read `offline_server.log`. One play-test settles it.

### Trap
Item 500720 used successfully from the bag, so the path is not globally broken.
Whatever gates the elixir does not gate that item — diff their `ItemTemplet` rows.

### Hand off
Whatever you learn here is the input to phases 20-22. Write the gate's identity
into `MODS.md` or a note the next conversation can read.

### Exit test
An elixir is used from the inventory, its effect applies, the count drops, and
`offline_server.log` shows `ITEM used item <id> from the bag`.

---

# Phase 20 — Stamina potion not working (`ISSUES.md` #11)

**Flag:** reuse phase 19's flag if it is the same gate; otherwise
`SERV_IRUHADEV_OFFLINE_STAMINA`

### Symptom
The stamina potion does not work properly.

### Evidence
Bucket B — no stamina-related event appears in the `C->S` census. There is no
`EGS_*STAMINA*` or `EGS_*VITAL*` event id in `EventID_Client.h` at all, which
means stamina is either a client-local counter or is carried inside another
packet.

### Diagnosis — CONFIRMED bucket, cause unknown
Do phase 19 first. If the gate found there covers this item too, this phase is a
verification, not an implementation.

If it is not the same gate, the likely story is that stamina (vitality) is a
per-day counter the real server owned and offline never modelled — in which case
the potion has nothing to restore. Check whether `X2OfflineDB` has a stamina
column and whether anything decrements it.

### What to do
1. Re-run the bucket check for whatever event the potion would send.
2. Find where the client reads the stamina value it displays, and trace it to the
   ACK field that fills it.
3. If nothing models stamina: **decide whether to model it or to disable the
   potion honestly.** Both are acceptable; silently consuming the potion is not.

### Exit test
Either the potion restores stamina and the bar moves, or using it is refused with
a message and the potion is not consumed.

---

# Phase 21 — "Camilla's secret manual" cannot be used (`ISSUES.md` #9)

**Flag:** reuse phase 19's, or `SERV_IRUHADEV_OFFLINE_SKILL_BOOK`

### Symptom
The book item cannot be used.

### Evidence
Bucket B — no packet sent. See phase 19.

### Diagnosis — CONFIRMED bucket
A "secret manual" is a skill-unlock book: using it grants or unlocks a skill. That
is a server-side effect, so even past the client gate it will need a handler.

Note `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`'s own comment: it only allows items
the client marks `GetCanUseInventory()`, because "anything else - a warp ticket, a
cube, a package - has a server-side effect this phase does not implement, and
consuming it to do nothing would destroy the item." **The manual is very likely
exactly that case, and the current behaviour is a deliberate safety refusal.**

### What to do
1. Confirm the item's `GetCanUseInventory()` value from `ItemTemplet`.
2. Grep `KncWX2Server/` for the skill-book path, for *structure*.
3. Decide: implement the skill grant against `X2OfflineSkill`, or refuse with a
   message that names the reason. The skill system already works
   (`SKILL skill 3011: level 0 -> 6`, `SKILL spent 18 SP, 29 left`), so
   implementing is plausible — the missing piece is the item-to-skill mapping,
   which may be server data.

### Trap
Do not consume the item until the grant is proven to work. Order the handler so
the skill lands first and the item is consumed only on success.

### Exit test
Use the manual: the skill appears in the skill window and survives a relog; or the
use is refused with a message and the item is intact.

---

# Phase 22 — Skill notebook not working (`ISSUES.md` #12)

**Flag:** `SERV_IRUHADEV_OFFLINE_SKILL_NOTE`

### Symptom
The skill notebook does not work.

### Evidence
Bucket B. The skill-note events exist in `EventID_Client.h`:
```
:1324  EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT
:1325  EGS_REG_SKILL_NOTE_MEMO_REQ
:1326  EGS_REG_SKILL_NOTE_MEMO_ACK
```
None appears in the `C->S` census. The skill system itself works:
```
[23:28:35.258] SKILL  skill 3011: level 0 -> 6
[23:28:35.258] SKILL  spent 18 SP, 29 left
[23:28:38.823] SKILL  slot 5 <- skill 3011 (slot -1 <- skill 3012)
```
`EGS_CHANGE_SKILL_SLOT_REQ` x1 and `EGS_GET_SKILL_REQ` x1, both handled.

### Diagnosis — CONFIRMED bucket
The notebook is the saved skill-preset page. `EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT`
is a **server-originated push** telling the client how many pages it owns — and
nothing offline originates it, so the client likely believes it has **zero pages**
and disables the UI. That is the strongest single hypothesis in this phase and is
cheap to check.

### What to do
1. Find where the client reads its skill-note page count and what it does with 0.
2. If it is the page count: send `EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT` at character
   load with the default page count a live account had. **Find that default in the
   client's own constants or in `KncWX2Server/`; do not pick a number.**
3. Then implement `EGS_REG_SKILL_NOTE_MEMO_REQ` against a new `X2OfflineDB` table
   and expect further note-family packets once the UI unlocks.

### Exit test
The notebook opens, a preset saves, and it is still there after a relog.

---

# Phase 23 — Cannot enhance equipment (`ISSUES.md` #4)

**Flag:** `SERV_IRUHADEV_OFFLINE_ENCHANT`

**Run this before phases 24-26.** It decides whether the item workshop can be
built at all.

### Symptom
Equipment cannot be enhanced.

### Evidence — two independent failures stacked
1. **Bucket B:** `EGS_ENCHANT_ITEM_REQ` never appears in the `C->S` census. The
   client did not send it.
2. **Bucket A:** even if it had, `Handler_EGS_ENCHANT_ITEM_REQ`
   (`Handlers_Inventory.cpp`) refuses unconditionally:
   ```c
   kAck.m_iOK = NetError::ERR_ENCHANT_ITEM_00;
   CX2OfflineLog::Server( L"ITEM  refused an enhancement - the enchant tables are server data"
       L" with no client copy, so enhancement is not implemented" );
   ```
   The handler takes `const KEvent& /*kEvent*/` — it does not even read the
   request.

The consequence is already visible in the title system:
```
[23:19:42.735] QUEST  CENSUS  clearType=13: 1 sub-quest(s)  ** NOT DRIVEN OFFLINE:
                      enhancement and socketing are refused offline (server-only tables)
[23:19:42.735] QUEST  CENSUS  clearType=14: 1 sub-quest(s)  ** NOT DRIVEN OFFLINE: ...
```
Two title sub-quests are permanently uncompletable because of this.

### The blocking question, and it goes to the user first
The handler's comment says the success rates and stat multipliers have no client
copy. Before writing any code, establish whether they can be sourced honestly:

- **Is there a server Lua for it?** `ls KncWX2Server/ServerResource/US/` and look
  for an enchant table. If there is, this is the pack-a-file rule: name it, stop,
  wait.
- **Is it in the live DB?** This is the `CLAUDE.md` live-DB rule. The user has
  `Account` and `Game01`. Ask for the enchant rate table and any `gup_*enchant*`
  procedure:
  ```sql
  SELECT name FROM sys.procedures WHERE name LIKE '%enchant%' ORDER BY name;
  SELECT name FROM sys.tables     WHERE name LIKE '%enchant%' ORDER BY name;
  ```
  Ask for it as plain text in the conversation, not as a file in the game tree.

**Do not invent a success curve.** The handler's own comment names the reason:
"inventing a success curve would silently destroy items." That refusal was correct
and stays correct until real numbers arrive.

### If the data arrives
Load it the shipped way (`LoadDataFile` -> `DoMemory` with a `DoMemoryNotEncript`
fallback, or a DB-transcribed table — **transcribe, don't tidy**), implement the
handler, then chase the bucket-B half: find why the client never sent the request.
The gate is probably the blacksmith NPC UI, and it may be the same class of gate
as phase 19.

### If it does not
Then the deliverable is smaller and still real: make the refusal reach the player.
Right now the client shows nothing useful. Fold phases 24-26 into this one and
ship one honest "the item workshop is unavailable offline" path.

### Exit test
Either an item enhances with real rates and the level persists, or the attempt is
refused with a message naming why and the item is untouched.

---

# Phase 24 — Cannot socket equipment (`ISSUES.md` #3)

**Flag:** `SERV_IRUHADEV_OFFLINE_SOCKET`

### Symptom
Equipment cannot be socketed.

### Evidence
Identical shape to phase 23. `EGS_SOCKET_ITEM_REQ` is absent from the `C->S`
census, and `Handler_EGS_SOCKET_ITEM_REQ` refuses unconditionally with
`NetError::ERR_SOCKET_ITEM_00` and the log line "refused a socketing - the socket
option tables are server data with no client copy".

### Diagnosis — CONFIRMED
Blocked on the socket **option** tables: which stat rolls a magic stone can
produce, and at what weights.

### What to do
Do phase 23 first and reuse its answer about sourcing. Then the same three steps:
source the table, implement, chase the client-side gate.

### Trap
Socketing and enhancement are different tables and different rolls. Do not assume
one file covers both because they share a UI.

### Exit test
A magic stone sockets into an item and the rolled option is real and persisted, or
the attempt is refused with a message and the stone is not consumed.

---

# Phase 25 — Cannot use magic amulet (`ISSUES.md` #5)

**Flag:** `SERV_IRUHADEV_OFFLINE_ENCHANT_ATTACH`

### Symptom
The magic amulet cannot be used.

### Evidence
```
2  EGS_ENCHANT_ATTACH_ITEM_REQ  (id=557)  --- IGNORED ---
```
`X2OfflineIgnore.cpp` carries `{ L"EGS_ENCHANT_ATTACH_ITEM_REQ", L"the item
workshop is not implemented offline" }` from phase 5. No handler, therefore **no
reply at all** — the client's dialog hangs.

### Diagnosis — CONFIRMED
Unlike phases 23 and 24, this one does not even send an error ACK, which is worse
for the player: a refusal at least closes the dialog.

### What to do
1. Establish what an amulet attach actually does — a deterministic effect (attach
   this specific option to this item) or a roll? A deterministic attach may be
   implementable from the amulet's own `ItemTemplet`, with no server table at all.
   **Check this before assuming it is blocked**; it is the one member of the
   workshop family that might be free.
2. If deterministic: implement it, remove the ignore rule.
3. If it needs a table: keep the rule, but add a handler that replies with an
   error so the dialog closes.

### Exit test
The amulet attaches its option, or the attempt is refused with a message and the
amulet is intact. In neither case does the dialog hang.

---

# Phase 26 — Cannot add equipment attributes (`ISSUES.md` #10)

**Flag:** `SERV_IRUHADEV_OFFLINE_ATTRIB`

### Symptom
Equipment attributes cannot be added.

### Evidence
Bucket B/A. No `EGS_ATTRIB_*` event appears in the `C->S` census, and
`X2OfflineIgnore.cpp` refuses both `EGS_ATTRIB_ENCHANT_ITEM_REQ` and
`EGS_ATTRIB_ATTACH_ITEM_REQ` — plus `EGS_ADD_ON_STAT_REQ`, which may be part of
the same UI — with "the item workshop is not implemented offline".

### Diagnosis — CONFIRMED
Three ignored event ids behind one screen, and the client is gating before it
sends any of them.

### What to do
Same shape as phase 25: determine which of the three the UI would send, whether
the attribute values are deterministic or rolled, source the table if rolled, and
in every case replace silence with a reply.

### Trap
`EGS_ADD_ON_STAT_REQ` matches the ignore list by exact name. If it turns out to
belong to a *different* feature than attributes, this phase should not touch it —
check before removing any rule.

### Exit test
An attribute is added and persists, or the attempt is refused with a message and
nothing is consumed.

---

# Phase 27 — Regular drops such as "Aqua" never drop (`ISSUES.md` #17)

**Flag:** `SERV_IRUHADEV_OFFLINE_DROP_REGULAR`

### Symptom
Ordinary consumable drops — the example given is "Aqua" — never appear.

### Evidence
The drop system is emphatically alive: 745 `DROP` lines in one session.
```
[23:20:03.352] DROP  loaded: 80 npc-exp row(s), 2435 monster row(s),
                     19128 item case(s) in 459 group(s)
[23:20:03.610] DROP  monster 80 dropped item 99610 (dropUID=7)
[23:20:47.860] DROP  monster 106 dropped item 10080021 (dropUID=35)
[23:20:50.038] DROP  monster 21 dropped item 77360 (dropUID=43)
[23:20:06.307] QUEST 9500 sub 9470 quest item 99610 drops (0/4 held, rate 100.00)
```
Items *do* drop. So this is not "drops are broken" but "this class of drop is
missing".

### Diagnosis — HYPOTHESIS, three candidates
1. **Aqua is not in the loaded table for those monsters.** 19,128 item cases in
   459 groups came from somewhere; find the source and grep it for Aqua's item id.
2. **It is in the table but the roll never hits.** Check the rate is read
   correctly — the quest-drop line prints `rate 100.00`, so rates are at least
   parsed on that path.
3. **It drops and is filtered on pickup.** The log distinguishes `dropped` from
   `picked up`; if Aqua appears as `dropped` and never as `picked up`, the failure
   is in the pickup path, not the table.

### What to do
1. Get Aqua's item id (from `ItemTemplet`, or ask the user what the client shows).
2. `grep -a "<id>" offline_server.log` — which of the three falls straight out of
   whether it appears at all, appears as `dropped` only, or never.
3. Read `X2Lib/Offline/X2OfflineDropTable.cpp` for where the 19,128 cases come
   from and what it filters.

### Trap — check this first, it may end the phase in five minutes
`CLAUDE.md`'s live-DB rule exists because `ScriptData/` and `DataBase/` disagree
with what shipped. If the drop table was built from a spreadsheet in
`ScriptData/`, missing items are exactly the expected failure, and the fix is to
ask the user for the live drop table rather than to debug the loader. Establish
the table's provenance **before** debugging anything.

### Exit test
Aqua drops from a monster that should drop it, at a plausible rate, and picking it
up puts it in the bag.

---

# Appendix A — Found in the logs, not in `ISSUES.md`

Three more real defects turned up while gathering evidence. They are not in the
user's list, so they are not phases; they are recorded here so they are not
rediscovered from scratch.

1. **`EGS_GET_MY_BANK_INFO_REQ` (id=630) is `UNHANDLED`**, once, at 23:41:27. The
   bank is not implemented. `EGS_EXPAND_BANK_SLOT_NOT` exists too
   (`EventID_Client.h:1112`). A clean phase-8-style job whenever the user wants
   the bank.

2. **Button warps are refused**, four times:
   ```
   [23:59:57.476] WARP  button warp 20000 refused - the warp table is server data;
                        use the world map, which goes through EGS_STATE_CHANGE_FIELD_REQ
   ```
   Warp ids 1000, 20000 and 20004. A deliberate refusal blocked on the server warp
   table — another live-DB / pack-a-file candidate.

3. **388 cash products are silently dropped from the catalog:**
   ```
   [23:22:53.590] CASH  catalog: 1972 product(s) from 2360 cash_product row(s);
                        388 dropped for having no item templet
   ```
   16% of the shop is invisible. Some of what the user cannot buy may simply not
   be listed. Worth a look if more "cannot buy X" reports arrive.

---

# Appendix B — The commands that produced this document

Reusable at the start of any phase above.

```sh
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"

# which bucket is this issue in?
grep -a "C->S" "$DATA/offline_packets.log" \
  | grep -oE "E(GS|CH|LG|CN|GB)_[A-Z0-9_]+" | sort | uniq -c | sort -rn

# the three buckets the phase-8 loop watches
grep -a "UNHANDLED" "$DATA/offline_packets.log" | grep -oE "E[A-Z_]+ +\(id=[0-9]+" | sort | uniq -c
grep -a "IGNORED"   "$DATA/offline_packets.log" | grep -oE "E[A-Z_]+ +\(id=[0-9]+" | sort | uniq -c
grep -a "EXCEPTION" "$DATA/offline_packets.log"

# what the server thought it was doing, by subsystem
for T in PET CASH ITEM REWARD TITLE SKILL WARP DROP QUEST; do
  echo "=== $T ==="
  grep -aE "^\[[0-9:.]+\] $T " "$DATA/offline_server.log" | sort -u | head -25
done

# the save file
python -c "import sqlite3; c=sqlite3.connect(r'$DATA/els_db.sql'); \
  print(c.execute('select unit_uid,nickname,level,ed from unit').fetchall())"
```
