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

> **Correction, 2026-09-05: bucket B is a trap in the other direction too.** Four
> of those six — issues 2, 9, 11 and 12, phases 19-22 — were **not** bucket B.
> They rode on packets whose names nobody thought to grep for, or sent the
> expected packet successfully in a session the census does not cover. All four
> are now done. **Read §0.2 before acting on any bucket call in this table.**

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

## 0.2 What phases 19-22 actually taught — read this before 23-27

Written 2026-09-05, after all four shipped and were verified in play. Six things,
in the order they will cost you time.

### 1. Bucket B is a weak test. Absence of *a* packet is not absence of *the* packet.

§0 classified issues 2, 9, 11 and 12 as "the client never sent a byte" because no
`EGS_USE_ITEM_*` / `EGS_*STAMINA*` name appears in the `C->S` census. **All four
were wrong.** Two rode on a packet nobody thought to grep for — the stamina potion
is a *cube* (`EGS_OPEN_RANDOM_ITEM_REQ`) and the skill notebook is a *cash claim*
(`EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ`) — and the other two sent exactly the
expected packet and got `NET_OK` back, in an earlier session than the one the
census covered.

Before trusting a bucket-B reading:

- **grep the census for the item id, not the event name.** The offline log prints
  item ids on most lines.
- **check the item's own counts.** A stack that has gone down proves the packet
  was sent and succeeded, whatever the census for one session says. This is what
  revealed both the elixir (100 → 94) and the stamina potion (100 → 96).
- **remember the census is one session.** `ISSUES.md` covers weeks of play.

### 2. The ACK is bookkeeping; the `_NOT` is the effect. This is now five for five.

Every "the item is consumed and nothing happens" defect in this project has been
the same shape, and it caught phases 19 and 21 as well:

| effect | ACK does | the `_NOT` that actually does it |
|---|---|---|
| potion / elixir buff | bag, ED, sort order | `EGS_USE_QUICK_SLOT_NOT` → `ApplyBuffFactorToGUUser` |
| skill unseal | bag | `EGS_UNSEAL_SKILL_NOT` → `AddSkillUnsealed` |
| skill-note pages | nothing | `EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT` → `SetSkillNoteMaxPage` + `HideSkillNote(false)` |

**Method:** find the client function that performs the effect, then find its
*callers*. If the only caller is a `_NOT` handler, the ACK alone will never work,
no matter how correct it looks. Do not trace forward from the request.

### 3. A field named `m_iRestoreX` may be an absolute, not a delta.

`KEGS_OPEN_RANDOM_ITEM_ACK::m_iRestoreSpirit` is the **new total**, because the
server assigns it from a function that returns the post-clamp value. Passing the
table value through set stamina to 4.4% and printed "-95%". The same shape is
still live and unfixed on `m_iRessurectionCount` beside it, harmless only while
the character owns no resurrection stones.

**Before copying any ACK field from a data table, read the server's assignment to
it**, not just the field name and not just the client's read of it.

### 4. Name the item before theorising. Both tools were available all along.

- **`data036/` in the game directory** holds the client's own scripts, XOR'd with
  a repeating 12-byte key (see phase 19). Decrypting `ItemTrans.lua` gives ~43,577
  **item id → live English name** pairs; `Item.lua` gives an item's icon and its
  `BFI_BUFF_*` / ability names; `SkillData.lua` gave phase 21 its entire
  item→class→skill mapping. `ScriptData/ItemTemplet.xlsx` contains **none** of the
  ids in this batch.
- **`db_backup/` in the game directory** holds dated saves. The one from 20:03 on
  2026-09-04 — three hours before the session that produced `ISSUES.md` — named
  both phase 19's and phase 20's items in a single diff against the live save.

### 5. Resolve ids; do not pattern-match names.

Phase 22's first pass declared "the cash shop seeds no memo cards" after grepping
2,360 product names for "Note"/"Memo"/"Manual". There are fifteen, products
184-198 — **named after the skill they teach** (`603002 Phoenix's Affinity`), not
after what they are. The play-test found them in under a minute.

A name search answers "is there an item called X". That is a different question
from "is there an item that does X".

### 6. Some cash items are "claimed, not carried".

`Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ` now has three branches that apply
an effect and never create an inventory item: the class-change ticket, the
inventory-expansion card, and (phase 22) the cash skill note. The tell is that the
live server handles the id inside `EGS_GET_PURCHASED_CASH_ITEM_REQ`
(`GSUserCashShop.cpp`) rather than in the use-item switch (`GSUserInventory.cpp`).

**If a cash item ends up sitting inertly in the bag, check which of those two
files handles it before assuming the bag path is broken.** Note that some ids
exist in both forms — `99600 SI_SKILL_NOTE_ITEM` is a real bag item,
`221600 CI_CASH_SKILL_NOTE_ITEM` is claim-time only, and they do the same thing.

### And the §0.1 count is now nine

Stale refusal comments found and corrected in this batch alone: the bag path's
"its effect is not implemented offline" (it was, via the `_NOT`), the cube's
"carried through untouched … so dropping them would empty those two cubes" (it
was destroying the gauge), `m_vecSkillUnsealed`'s "Nothing offline seals or
unseals anything" (nothing did, then something did), and
`EGS_REG_SKILL_NOTE_MEMO_REQ`'s "the memo IDs come from a server table" (they are
`GetItemID( itemUID )`; there was never a table). Phase 23 then added two more:
`EGS_ENCHANT_ITEM_REQ`'s "the enchant tables are server data with no client copy"
(the stat multipliers and the stone IDs both have a client copy, and always did —
only the success rates do not), and the copy of that same sentence that had
propagated into `X2OfflineQuest.cpp`'s undriven-clear-type list, where it was
keeping a title sub-quest permanently unreachable. **§0.1 is not a caveat about
one pet message. Assume every self-describing refusal in `X2Lib/Offline/` is as
old as the phase that wrote it — and that a wrong one may have been copied
somewhere else.**

---

## 0.3 What phases 23-26 actually required — read this before 27

Written 2026-09-05, after all four shipped and were played. The item workshop was
the batch the plan was least sure about — four phases marked **"blocked on the
user"**, three of them with a "name the file, stop, wait" instruction. None of
them was blocked. Seven things, in the order they will cost you time.

### 1. The "blocked on user" column was wrong four times out of four

The whole question was answered by one `ls KncWX2Server/ServerResource/US/`:

| phase | the plan said it needed | where it actually was |
|---|---|---|
| 23 enhancement | "enchant tables", maybe a live-DB query | `EnchantTable.lua`, in the tree |
| 24 socketing | "socket option tables" | `SocketItemTable.lua`, in the tree |
| 25 amulet | "attach tables" | **already parsed** — inside `EnchantTable.lua` |
| 26 attributes | "attrib tables" | `AttribEnchantTable.lua` + `AttribAttachTable.lua`, in the tree |

No live-DB query was needed for any of them. **Do the `ls` before writing the
ask.** The `CLAUDE.md` rule is "name the file, then implement anyway" — it is not
"assume the file does not exist".

### 2. Ask what the CLIENT already has before asking what the user must supply

Every one of the four refusals over-claimed. In each case a meaningful part of
what was called server-only had a client copy that had been there all along:

| phase | claimed server-only | actually client-side |
|---|---|---|
| 23 | "success rates **and stat multipliers**" | `ENCHANT_STAT_SCALE` is `SetEnchantRate`'s 21 values byte for byte; so are the stone IDs and the level banding |
| 24 | "the socket option tables" | the **cost** (`CX2UISocketItem::CalculateNewSocketCost`) and the **slot count** (`CX2SocketItem::SOCKET_COUNT`) |
| 26 | "the item attribute system" | `AttribEnchantRequire.lua` is loaded by the *client*; so are `ATI_*`, `ESI_SLOT_*`, `ACT_*`, `ENCHANT_TYPE`, `CanEnchantAttribute` and `GetItemID` |

Only one thing in each phase genuinely had no client copy: the **probability
tables**. That is a much smaller ask than "the enchant tables", and it is also
what makes a packed file self-checking — phase 23's loader compares its
`SetEnchantRate` rows against the client's `ENCHANT_STAT_SCALE` and logs
`21 stat rate(s) match`, which is a free proof that the right region's file got
packed.

### 3. `NULL == kInfo` is dead code, and it was in every offline loader

**This is the one to carry forward.** Every loader in `X2Lib/Offline/` tested a
missing packed file with:

```c
KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
kInfo = ...->LoadDataFile( name );
if( NULL == kInfo )            // <-- never true
```

`MASSFILE_MEMBERFILEINFO_POINTER` is **a struct by value**, not a pointer, and it
carries `operator const MASSFILE_MEMBERFILEINFO*() const { return this; }`
(`KTDXLIB/KGCMassFileManager.h`). Comparing it against NULL takes the address of a
local, which is never null. So a file that was never packed fell through to the
`DoMemory` / `DoMemoryNotEncript` pair and was reported as

```
ATTRIB   ERROR 'AttribAttachTable.lua' failed to run, encrypted or plaintext.
```

— i.e. "your packed file is corrupt" when the truth was "you did not pack it".
That is the single diagnosis the degrade path exists to give, and it was the one
it could not give. Caught only because phase 26 shipped with one of its two files
unpacked and the log accused the wrong thing.

Fixed at all **11 sites across 9 loaders** (`Enchant`, `Socket`, `Attrib`,
`Resolve`, `Drop`, `Stat`, `PetData`, `MapData`, `RandomItem`, `BattleField`×2) to
test the payload instead:

```c
if( NULL == kInfo->pRealData || kInfo->size <= 0 )
```

which is what the client's own loader does
([X2ItemManager.cpp:231](X2Lib/X2ItemManager.cpp#L231)). The bug came from the
phase-12 loader and was copied forward three times without anyone reading the
type. **When a degrade path fires, check that it says the right thing — a wrong
diagnosis is worse than none, because it sends the user to fix the wrong thing.**

### 4. Bind every method the packed file names, including the dead ones

lua_tinker turns a call to an unbound method into an error that **abandons the
rest of the chunk**. `EnchantTable.lua` calls 23 distinct methods, several inside
its own `--[[ ]]` block comments and several behind TW/HK-only flags, and the
probability rows this batch needed are at the very *end* of the file. Binding only
the methods phase 23 read would have silently cost every row after the first
unbound call.

So phase 23 bound all 23 and parsed all of them into storage. **That is why phase
25 was nearly free**: `AddAttachItemInfo` — the magic amulet's entire table — had
already been loading since the day `EnchantTable.lua` was packed, and phase 25
turned out to be two accessors and a handler, with no new file, no new loader and
nothing to ask the user for.

### 5. Two of the four loaders had to borrow a Lua global and give it back

`AttribEnchantTable.lua` and `AttribAttachTable.lua` are written against
`g_pCX2EnchantItem`, and `SocketItemTable.lua` against `g_pCX2SocketItem`. Both of
those names are published by the **client's own** managers (`X2EnchantItem.cpp:33`,
`X2SocketItem.cpp:69`), and neither client manager binds the methods those files
call. So both loaders point the global at themselves for the duration of the
`DoMemory` calls and restore `g_pData->GetEnchantItem()` / `GetSocketItem()`
immediately afterwards, logging a warning if they cannot. The client republishes
those globals only from `OpenScriptFile`, which runs once at start-up, long
before either loader.

Worth checking before adding a fifth: `EnchantTable.lua`'s `EnchantItemManager`
and `ResolveTable.lua`'s `g_pResolveItemManager` are names the client does **not**
use, so those two could simply take them.

### 6. A long flag-wrapped server switch gets extracted by machine, not by hand

`CXSLSocketItem::GetSocketDataType` is a ~450-line switch over magic stone IDs
wrapped in six different `SERV_` flags. It was extracted with a small
preprocessor applying this build's flag set and flattened into one 58-row table,
and `IsMagicStoneItemID` was extracted the same way. **The two agreeing —
identical 58-stone sets, arrived at independently — is the check that says the
extraction was right.** The `SOCKET_DATA_TYPE` and `ENCHANT_TYPE` enum seeds were
generated from the client headers the same way, and verified by resolving all 117
enum names the packed `SocketItemTable.lua` subscripts.

**One asymmetry this turned up:** the client's socket UI offers 16 Luriel stones
(`85003840..`, `152000699..`) that the server never accepted, because
`SERV_LURIEL_MAGIC_STONE` is not defined **anywhere** in `KncWX2Server/Common`.
The client array is not flag-guarded; the server switch is. The server wins, and
those stones are refused with a log line naming the stone.

### 7. Packets and work the phase sections did not mention

- **`EGS_SOCKET_ITEM_REQ` is plural.** Its `m_mapSocketInfo` is
  slot index -> magic stone UID, so one request fills several slots. The phase-24
  exit test says "a magic stone sockets into an item", singular; the handler has
  to price, validate and apply a whole batch, and the real server does it in two
  passes so a request that cannot be paid for changes nothing. The play-test
  exercised exactly this: two empty slots in one request, then repeated single-
  slot replacements.
- **Two quest hooks nobody scheduled.** Phases 23 and 24 each had to add a driver
  (`OnEnchantItem`, `OnSocketItem`) and take their clear type off
  `X2OfflineQuest.cpp`'s undriven list. Phase 23's section mentions the two stuck
  title sub-quests as a *consequence* of the refusal, not as work to do.
- **`EGS_ADD_ON_STAT_REQ` is not part of phase 26** — the trap in that section was
  right. It is an in-match relay to the room server
  ([GSUserRoomCommon.cpp:4122](KncWX2Server/GameServer/GSUserRoomCommon.cpp#L4122))
  whose answer the client applies with `CX2GUUser::SetAddOnStat`. It stays
  ignored; only its wrong reason was fixed.
- **Phase 23 also had to touch the stat sum.** `AddEquippedStat` did not apply
  `ENCHANT_STAT_SCALE`, because until phase 23 no offline item could have a
  non-zero enchant level. Left alone, a +7 weapon would have been worth nothing to
  the character's HP in a dungeon, since `m_kGameStat` is what
  `CX2GUUser::InitStat` prefers.

### Decisions taken that the plan did not specify

- **The GM socket cheat is refused.** `KEGS_SOCKET_ITEM_REQ::m_bCheat` lets the
  request carry chosen option IDs straight into the item, gated on `UAL_GM` on the
  real server. Offline there is no operator and the auth level is whatever the
  login handed out, so honouring it would let a modified client write arbitrary
  socket options. `ERR_VERIFY_12` and a log line.
- **The enchant amulet does NOT feed the enchant quest hook**, because the real
  server does not: `DBE_ENCHANT_ATTACH_ITEM_ACK` has no `Handler_OnEnchantItem`
  call, only the enhancement path does. A step that asks the player to *enhance*
  an item is not satisfied by buying the level.
- **A random attribute that rolls `ET_NONE` is refused, not applied.** Left alone
  it would silently turn "add an attribute" into "clear the slot".
- **`GetAttribCountType` counts non-empty slots, not leading ones** — an item with
  slot 0 empty and slot 1 filled is `ACT_SINGLE`. That decides both the price and
  which lottery runs, and reading it as "how far along the array am I" is wrong.
- **The attribute amulet overwrites all three slots**, `ET_NONE` included, because
  the server's loop walks `ESI_SLOT_1..MAX` unconditionally.
- **Rows for flags this build does not compile are parsed and stored anyway**
  (EnchantPlus, DestroyGuard, the identify table, the drop/cube random enchant
  lotteries). They cost nothing, they keep the chunk alive per point 4, and the
  drop/cube ones are what phase 27 will want.

### And the §0.1 count is now eleven

This batch found four more stale refusals: `EGS_ENCHANT_ITEM_REQ`'s "the enchant
tables are server data with no client copy" (two thirds of it had a client copy),
`EGS_SOCKET_ITEM_REQ`'s identical claim, the copy of the enchant sentence that had
**propagated into `X2OfflineQuest.cpp`** and was keeping two title sub-quests
permanently unreachable, and `EGS_ADD_ON_STAT_REQ`'s ignore reason. **Add the
propagation case to the §0.1 habit: a wrong refusal sentence may have been copied
somewhere that makes a decision.**

---

## 1. Shared preamble — paste this into every phase conversation

Each phase below is meant to open a fresh conversation. That conversation gets
`CLAUDE.md` automatically and this file is in the repo, so it does not need
pasting — one line starts a phase:

> Read §0, §0.1, §0.2, §0.3, §1 and the Phase N section of
> `OFFLINE_MODE_PHASE9_PLAN.md`, then do Phase N. Don't read the other phase
> sections — the file is long.

Substitute the phase number. Naming the sections matters: §0 is the bucket
triage, §0.1 is the stale-refusal check, §0.2 is what phases 19-22 turned up,
**§0.3 is what phases 23-26 turned up and is the one that matters most for
anything that loads a packed table**, §1 is this preamble, and skipping them is
how a phase gets debugged with the wrong technique.

Two useful variants:

- **To plan before building:** append *"Investigate and confirm the diagnosis
  first — don't edit anything until you've shown me what you found."* Worth doing
  for the phases marked HYPOTHESIS (14, 17, 18, 27).
- **To carry a finding forward:** phases 23-26 are expected to share root causes,
  so start the follow-up with *"Phase 23 found <the gate>; check whether Phase 24
  is the same gate before implementing anything."* Phases 19-22 were predicted to
  share one and did not — see §0.2 — so treat the prediction as a hint, not a
  premise.

The five standing facts each conversation needs, and which §1 exists to supply:

1. **Branch is `mods/offline-mod-2`.** All work is client-only unless the phase
   says otherwise; nothing here should touch `KncWX2Server/Common/`.
2. **Every edit goes behind `SERV_IRUHADEV_OFFLINE` — the one flag already
   defined in `KTDXLIB/Always.h`. MUST NOT mint a new `SERV_IRUHADEV_OFFLINE_*`
   flag per phase.** Every file under `X2Lib/Offline/` is already wrapped
   top-to-bottom in `#ifdef SERV_IRUHADEV_OFFLINE`, so a phase-specific
   sub-flag nested inside it is always redundant — it can only ever be defined
   or undefined in lockstep with the flag that already encloses it, so it
   toggles nothing on its own and exists only to be forgotten. Phases 9-12
   shipped four of these (`SERV_IRUHADEV_OFFLINE_INVENTORY_EXPAND`,
   `_PET_FEED`, `_INVEN_SORT`, `_ITEM_RESOLVE`) before this rule was written;
   they have been migrated away and must not reappear. Every phase section
   below that still names a distinct flag is stale — treat every "**Flag:**"
   line in this document as `SERV_IRUHADEV_OFFLINE`, regardless of what it
   says, and do not add a new `#define` to `Always.h` for it. Use the house
   comment block only when a phase adds genuinely new client behavior that
   needs its own author/date/description entry; that entry documents the
   change, it does not gate it — the gate is always `SERV_IRUHADEV_OFFLINE`.
   The one exception is a short-lived `SERV_IRUHADEV_*_DEBUG` diagnostic flag
   (per `CLAUDE.md`'s "when a gameplay change cannot be verified by reading
   code" rule) — those are throwaway logging scaffolding meant to be deleted
   after one play-test, not a feature gate, so they are not covered by this
   rule and do not need to be `SERV_IRUHADEV_OFFLINE`-prefixed.
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
| **19** | 2  | Elixir cannot be used | ~~B~~ **C** | **DONE 2026-09-05** | no |
| **20** | 11 | Stamina potion not working | ~~B~~ **C** | **DONE 2026-09-05** | no |
| **21** | 9  | "Camilla's secret manual" unusable | ~~B~~ **C** | **DONE 2026-09-05** | no |
| **22** | 12 | Skill notebook not working | ~~B~~ **C** | **DONE 2026-09-05** | no |
| **23** | 4  | Cannot enhance equipment | A + B | **DONE 2026-09-05** | no — `EnchantTable.lua` is in the tree, pack it |
| **24** | 3  | Cannot socket equipment | A + B | **DONE 2026-09-05** | no — `SocketItemTable.lua` is in the tree, pack it |
| **25** | 5  | Cannot use magic amulet | A | **DONE 2026-09-05** | no — the table is inside `EnchantTable.lua` |
| **26** | 10 | Cannot add equipment attributes | A | **DONE 2026-09-05** | no — `AttribEnchantTable.lua` + `AttribAttachTable.lua` |
| **27** | 17 | Regular drop ("Aqua") never drops | C | HYPOTHESIS | no |
| **28** | *(not in `ISSUES.md` — found 2026-09-05, after phase 14)* | Summoned pet doesn't survive relog / character switch | — | CONFIRMED | no |

**~~Phases 19-22 share one root cause and probably one flag.~~ They did not.**
All four are done (2026-09-05) and they were four unrelated defects in four
different packets. Keeping the prediction here because being wrong about it cost
nothing — the phases were numbered separately "so that if they turn out to be
four different gates you have four slots", which is exactly what happened — but
the reasoning behind it is worth not repeating:

| Phase | The packet it actually was | The defect |
|---|---|---|
| 19 | `EGS_USE_ITEM_IN_INVENTORY_REQ` | ACK sent, no `_NOT`, so the buff never applied |
| 20 | `EGS_OPEN_RANDOM_ITEM_REQ` | ACK field passed through raw; it is an absolute, so the potion *emptied* the bar |
| 21 | `EGS_USE_ITEM_IN_INVENTORY_REQ` | ACK sent, no `EGS_UNSEAL_SKILL_NOT`, and the load path re-sealed it |
| 22 | `EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ` | claim-time item claimed into the bag instead of being applied |

**And none of the four was bucket B.** §0 put all four there on the strength of
"the event name is absent from the `C->S` census". That test has a blind spot:
absence of a *use* packet does not mean absence of a packet. Two of the four rode
on a packet nobody thought to look for (the cube open, the cash claim), and the
other two sent the expected packet and got a successful ACK back — bucket C — in
a *different session* from the one the census covered. See §0.2.

**Phases 23-26 are one feature** — the item workshop — split four ways because
each needs a different table from the user. **Run phase 23 first**; it establishes
whether the tables can be sourced at all. If the answer is no, phases 24-26
become "improve the refusal so the player knows why", which is a real and much
smaller job, and they should be merged into one conversation at that point.

**Phase 23 ran, and the answer was yes** (2026-09-05). The table was already in
the tree — `KncWX2Server/ServerResource/US/EnchantTable.lua` — and needed no
live-DB query at all. **All four are now done** (23-26, 2026-09-05), and not one
of them needed a live-DB query or anything the user did not already have.

The thing worth carrying forward is how badly the "blocked on the user" column
read the situation. Four phases were marked **yes — blocked**; the real answer
was one `ls KncWX2Server/ServerResource/US/`:

| phase | what it "needed" | where it actually was |
|---|---|---|
| 23 | enchant tables | `EnchantTable.lua`, in the tree |
| 24 | socket option tables | `SocketItemTable.lua`, in the tree |
| 25 | attach tables | **already loaded** — inside `EnchantTable.lua` |
| 26 | attrib tables | `AttribEnchantTable.lua` + `AttribAttachTable.lua`, in the tree; and half the feature was already client-side |

And in every one of the four, a meaningful part of what the refusal claimed was
server-only turned out to have a client copy that had been there all along —
`ENCHANT_STAT_SCALE` for 23, the cost and slot-count maths for 24, the whole
require table and every enum for 26. **The audit that pays is "what does the
client already have", done before "what must the user supply".**

Three of the four still need a file packed to *run* (23 is packed and verified;
24 and 26 are not). That is a different thing from being blocked on data that
does not exist, and the degrade paths say which file and what to do about it.

---

# Phase 9 — Expansion card cannot be bought (`ISSUES.md` #14)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

No new packets - still bucket C exactly as filed, but the plan's two candidates
both missed the actual defect, and settling it needed no DB query because
reading the two `SortCategory` implementations side by side made the answer
obvious without a rebuild:

- **Neither candidate 1 nor candidate 2 was it. The ACK shape was already
  correct, and `SortCategory()` did move things - just according to the wrong
  algorithm.** The pre-existing offline implementation compacted the category
  toward slot 0 in whatever order the items already sat in. That is a real
  move, not a no-op report, but items land in the first free slot as they are
  acquired, so a freshly-played character's bag has no gaps to close - the
  compaction had nothing to do on every press, which is indistinguishable from
  "does nothing" without knowing what the button is supposed to do in the first
  place.

- **The phase-5 header comment cited the wrong server function, and that is
  what pointed this phase at compaction instead of a real sort.** It said
  `KInventory::SortInventory` "has its own move loop commented out and only
  reports the category back" - true, but `SortInventory` (`Inventory.cpp:5549`,
  dated 2009) is dead code; nothing calls it. The packet handler
  (`GSUserInventory.cpp:8019`) calls a *different*, later `KInventory::SortCategory`
  (`Inventory.cpp:17757`), which dispatches by category to `SortEquipCategory`
  (`:17799`), `SortNormalCategory` (`:18022`) or `SortConsumptionCategory`
  (`:17982`) - three real multi-key sorts (item ID, grade, use level, equip
  position, set ID, class-usability, in category-dependent precedence), not a
  stub. Reading the 2009 comment instead of the function the handler actually
  calls is exactly the "reads consistently, still wrong" trap `CLAUDE.md`
  warns about for `DataBase/` and `ScriptData/` - the same shape, but this
  time inside the tree's own C++ rather than the DB snapshot.

- **Ported the real three-way sort rather than approximating one client-visible
  order.** `SortEquipCategory`'s six chained stable `multimap` passes collapse
  to one composite comparator (most significant first): usable-by-your-class
  before anyone-usable before everything else grouped by unit type, then set
  ID descending, equip position ascending, use level descending, grade
  ascending, item ID descending. `SortNormalCategory` reduces the same way to
  item ID descending with grade/level as tiebreaks that only matter between
  stacks sharing an ID. `SortConsumptionCategory` is item ID ascending, no
  tiebreak chain at all. All three are now `std::stable_sort` comparators in
  an anonymous namespace in `X2OfflineInventory.cpp`, cited by `Inventory.cpp`
  line number, gated with the rest of the new code under
  `SERV_IRUHADEV_OFFLINE` (originally shipped behind a dedicated
  `SERV_IRUHADEV_OFFLINE_INVEN_SORT` flag, migrated to the shared flag — see
  §1 rule 2).

- **Decision: reused `CX2OfflineInventory::IsAbleToEquip` (itself
  `CX2Unit::CanEquipAsParts`) for the class-usability tier instead of porting
  `KInventory::CompareUnitClass`'s switch by hand.** The header already
  documents this exact tradeoff for other equip checks in this file - hand-
  porting the class-compatibility table would duplicate a table `X2Lib` has no
  other accessor for, and `CanEquipAsParts` is the client's own equivalent
  check, already used for the live unit. Not a byte-for-byte match (its
  `UC_NONE` branch differs from `CompareUnitClass`'s), but that only affects
  items with `USE_CONDITION` `UC_NONE`, an edge case with no observed instance
  in this build's item data.

- **No `Common/` change, no server rebuild.** The wire packet
  (`KEGS_SORT_CATEGORY_ITEM_ACK`, `std::vector<UidType> m_vecUpdatedInventorySlot`
  in slot order) was already right; only the offline server's internal
  ordering logic changed, entirely inside `X2Lib/Offline`.

- **Build**: `X2Lib_2010.vcxproj` then `X2_2010.vcxproj`, both `US_SERVICE`,
  0 errors. Deployed to `X2_offline.exe`; confirmed by size/mtime.

- **Not yet done**: the exit test is a real play-test (an inventory category
  with mixed item types, press sort, confirm the visible reorder, relog,
  confirm it stuck) that needs a human at the client - not run as part of this
  phase.

---

# Phase 12 — Cannot dismantle equipment (`ISSUES.md` #6)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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
3. If it is server data: **implement the real feature now**, on the assumption
   the file will be (or already is) packed - loader, yield logic, and a
   degrade-to-refusal path if the file turns out to be missing, all in the same
   pass, same shape as the Lua-packing rule in `CLAUDE.md`. Name the exact file
   when you report the work done, not before starting. Do not stop and wait for
   packing confirmation - see the correction in "What actually happened" below.

### Trap
The `NOT DRIVEN OFFLINE` census line at 23:19:42 flags two title sub-quests
(`clearType=13`, `clearType=14`) as depending on enhancement and socketing. Do
not widen this phase to chase them — that is phases 23-24.

### Exit test
Dismantle an item: either it yields materials and the item is gone, or a dialog
says dismantling is unavailable offline and the item is untouched. Silence is the
failure.

### What actually happened

Branch 3. `CXSLResolveItemManager` (`KncWX2Server/Common/X2Data/
XSLResolveItemManager.cpp`) draws every dismantle result from
`m_mapResolveData` / `m_vecBrokenPieceResolve`, both populated only by
`AddResolveData_LUA` / `AddResolveDataBrokenPiece_LUA` calls in
`KncWX2Server/ServerResource/<region>/ResolveTable.lua`. That file has no
client-side counterpart - unlike the enchant/socket tables, it was never even a
candidate for packing, since nothing about it is named as a client asset
anywhere. So there is no yield to compute, and this followed the enchant/socket
precedent exactly: a real `Handler_EGS_RESOLVE_ITEM_REQ` that answers with
`ERR_RESOLVE_ITEM_04` ("분해를 할 수 없습니다.", the client's own compiled
`NetError_def_US.h` string - not an invented sentence) instead of leaving the
packet on `X2OfflineIgnore.cpp`'s list. Added behind `SERV_IRUHADEV_OFFLINE_
ITEM_RESOLVE` in `Handlers_Inventory.cpp`, right after the two existing
enchant/socket refusals, plus the matching dispatch case and header
declaration; the ignore-list entry was left in place per the instructions above
(the dispatch's switch handles the id before `Reason()` is ever consulted, so
the entry is now dead but honest documentation of history).

Verified end to end: `offline_packets.log` shows `EGS_RESOLVE_ITEM_REQ ...
HANDLED` followed by `EGS_RESOLVE_ITEM_ACK` (previously `--- IGNORED ---` with
no ACK at all), `offline_server.log` logs the refusal line, and in play the
client showed an "Unable to dismantle" dialog with the item left in the bag.

**Correction, same session:** the refusal above was written and shipped
without ever asking whether `ResolveTable.lua` could be packed - this plan's
own step 3 said a refusal was "a legitimate outcome for this phase" and that
framing was followed instead of `CLAUDE.md`'s server-Lua rule, which the user
caught. `CLAUDE.md`'s rule now says explicitly that the ask comes first even
when a phase plan frames refusal as acceptable; step 3 above is corrected to
match. The ask for this phase specifically: **pack
`KncWX2Server/ServerResource/US/ResolveTable.lua`** (XOR-encrypt, add to
`data036.kom`) and real materials become implementable - the yield algorithm
in `CXSLResolveItemManager::GetResultItem` is fully read and ready to port into
a new `X2OfflineResolveTable` module (same shape as `X2OfflineDropTable` /
`X2OfflineStatTable`) once the table is loadable. Until then the refusal
committed above stands, but it is a "not yet asked to be enabled" state, not a
"can't be done" one.

### Second pass — real materials, once the Lua was packed

The user packed `ResolveTable.lua` the same session and, per the corrected
rule above, the real feature was implemented immediately rather than asked
about and parked. No new packets: still `EGS_RESOLVE_ITEM_REQ` /
`EGS_RESOLVE_ITEM_ACK`, same ids as the refusal above - only the handler body
changed. Everything below is what the plan's one-line "port the algorithm"
undersold:

- **Two source functions had to be read together, not one.** The yield
  algorithm is not all in `CXSLResolveItemManager::GetResultItem` - the sell
  price it needs as an input is computed by the *caller*,
  `KInventory::ResolveItem` (`Inventory.cpp:11799-12029`), which is itself
  called from `GSUserInventory.cpp:2846-2926`. Reading only the manager class
  (as the phase 12 diagnosis above did) would have left "what is `iSellPrice`"
  unanswered. It turned out to already be solved: `KInventory::GetSellPrice`
  (`SERV_SELL_ED_ITEM_PRICE_FIX` arm) is the exact formula
  `Handler_EGS_SELL_ED_ITEM_REQ` (phase 5) already ports for shop selling, so
  the new handler duplicates that same switch rather than inventing a second
  one - "1/5 of shop price, prorated by remaining endurance."

- **Two Lua globals nothing in the client defines - not named in the plan,
  found by running the file.** `ResolveTable.lua` subscripts
  `ITEM_TYPE["IT_WEAPON"]` and `ITEM_GRADE["IG_UNIQUE"]` (etc.) exactly the
  way `RandomItemTable.lua` and `PetData.lua` needed `UNIT_CLASS[...]` /
  `PET_UNIT_ID[...]` in phase 7b. Extended the existing
  `X2OfflineLuaEnum`/`X2OfflineLuaEnumSeed.h` machinery (`Publish()`) rather
  than building a second one, transcribing both enums from the client's own
  `X2Lib/X2Item_Preprocessing.inl` and diffing against the server's
  independent copy (`KncWX2Server/Common/X2Data/XSLItem.h:16-42`): identical,
  value for value, no exceptions - unlike `UNIT_CLASS`'s partial diff in
  phase 7b, there was nothing to reconcile here.

- **Fashion (avatar/costume) items are a second, unrelated data dependency -
  scoped out rather than chased.** `KInventory::ResolveItem` branches on
  `pItemTemplet->GetFashion()` before ever calling the resolve manager: a
  fashion item is priced through `CXSLCashItemManager::GetCashItemPriceFromScript`
  (`Inventory.cpp:11939`) and pays out a fixed "unknown attribute stone,"
  nothing from `ResolveTable.lua` at all. That is a different server-only
  table this phase does not have. Since almost every real
  `CX2Item::IT_ACCESSORY` carries `GetFashion() == true`, this is also where a
  non-dismantleable accessory actually lands - refused with
  `ERR_RESOLVE_ITEM_04` and a log line naming the cash-item table by name,
  rather than silently forcing it through the weapon/armor path or widening
  this phase to also port `CXSLCashItemManager`.

- **The jackpot flag is genuinely per-session state, not the manager's - new
  `KOfflineSession` field the plan never named**, the same shape phase 10's
  "tracks the summoned pet" surprise took. `bJackpot` does not come from
  `CXSLResolveItemManager` at all; it is decided by comparing
  `KGSUser::m_kTimer[TM_RESOLVE_JACKPOT].elapsed()` against
  `m_dResolveJackpotTime` (`GSUserInventory.cpp:2894-2944`), both per-connected-
  user members with no offline equivalent before this phase. Added
  `KOfflineSession::m_tNextJackpotAt` (one absolute deadline instead of an
  elapsed/threshold pair) behind `SERV_IRUHADEV_OFFLINE` (originally shipped
  behind a dedicated `SERV_IRUHADEV_OFFLINE_ITEM_RESOLVE` flag, migrated to the
  shared flag — see §1 rule 2), defaulting to 0 - which is guaranteed `<=` now,
  so the first dismantle after any launch is always a jackpot attempt,
  matching the live server's own default-`0.0` member.

- **A real bug in the studio's own code, transcribed rather than fixed.**
  `CXSLResolveItemManager::ResolveResult_EnchantStone` doubles the quantity
  when it inserts a *new* map entry under jackpot, but adds the plain
  (non-doubled) `m_iResultCount` when a later draw in the same call merges
  into an *existing* entry - both branches of that second ternary read the
  same `kResolveData.m_iResultCount`. Ported byte for byte with a comment
  pointing at the asymmetry rather than "correcting" it, per `CLAUDE.md`'s
  transcribe-don't-tidy rule; it only bites a second-or-later successful draw
  of the same enchant stone inside one dismantle, rare enough at
  `iRandomCnt <= 4` that live players plausibly never noticed either.

- **Space is checked per material type, not as one atomic reservation -
  a known, narrow gap from `KInventory::IsEnoughSpaceExist`.** The real check
  reserves room for every result item at once before touching anything; the
  port calls `CX2OfflineInventory::HasRoomFor` once per distinct material
  independently. The two disagree only when two *different* new material
  types both need the last free slot in the same category at once - narrow
  enough with a handful of material kinds per dismantle that it was not worth
  a bigger, transactional check.

- **A new offline module needs two file-list edits that are easy to forget
  and fail silently at compile time, not at all.** `X2OfflineResolveTable.h`/
  `.cpp` had to be added to `X2Lib_2010.vcxproj`'s `ClInclude`/`ClCompile`
  lists by hand - a `.cpp` not listed there simply never compiles, with no
  error until link time (`LNK2019` unresolved externals), which is a
  confusing place to first learn a file was never in the build.

- **Not yet done at the time of this writeup**: the exit test is a real
  play-test (dismantle an item, read the logs, confirm materials landed and
  the original item is gone) that needs a human at the client. Both projects
  built clean under `US_SERVICE` and the exe was redeployed to
  `X2_offline.exe`, confirmed by size/mtime; the first pass's refusal was
  already play-tested and confirmed working, so this second pass is confirmed
  only by the build, pending that play-test.

---

# Phase 13 — Cobo Express: "You cannot enter the village" (`ISSUES.md` #16)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

**The plan's diagnosis was wrong about which feature "Cobo Express" is**, and
that cost most of the phase. §0's log evidence (two `EGS_SQUARE_LIST_REQ`
`UNHANDLED` lines, 400ms apart) is real and was fixed, but it is the *trade
market* the client calls `ST_TRADE` internally - not what the user meant by
"Cobo Express". The user's own description ("bottom menu, press B, a list of
villages/rest areas, pick one, OK, 'you cannot enter the village'") is the
**warp-by-button** feature - `CX2StateMenu::CreateWarpDest` /
`Handler_EGS_WARP_BY_BUTTON_REQ` - and the server's own item enum names it
outright: `CXSLItem::SI_USE_COBO_EXPRESS_TICKET`
(`KncWX2Server/GameServer/GSUserFunction.cpp:6656`). That packet was never
`UNHANDLED` - it was already answered by a **deliberate refusal already sitting
in `Handlers_Social.cpp`**, written before this phase, on the theory that
"which warp index leads to which map, and what it costs, is server data (the
warp table)". This phase's own "Related, same area, do NOT fold in" note saw
that refusal in the log and explicitly said to leave it alone - which was the
mistake to correct, not a boundary to respect.

- **The square fix (`Handlers_Square.cpp`) is real and stays in**, exactly as
  planned: `EGS_SQUARE_LIST_REQ` (offers one trade square, population 0, page
  1/1), plus `EGS_JOIN_SQUARE_REQ`, `EGS_LEAVE_SQUARE_REQ` and
  `EGS_SQUARE_UNIT_SYNC_DATA_REQ` - none of which the plan named, all three
  being exactly the "second round of `UNHANDLED`" §2's phase index predicted
  once the list request stopped timing out. This is a genuine, separate fix; it
  is just not the one the user was asking about.

- **The "warp table is server data" refusal was the same shape of mistake §0.1
  warns about for phase 10's `PetData.lua`**, just never caught: the thing
  said to be missing was checked by reading a comment, not by checking whether
  the file existed. `CXSLMapData::CheckCOBOExpressTicketMapID` and
  `ComputeCOBOExpressTicketCost` (`KncWX2Server/Common/X2Data/XSLMapData.cpp:
  606-613`) are not a data table at all - they are two plain Lua **functions**,
  and the file that defines them, `KncWX2Server/ServerResource/US/MapData.lua`,
  was already sitting in this tree, ask-the-user-free. New file
  `X2OfflineMapData.{h,cpp}` loads it through the same archive/XOR path every
  other offline data table uses (`X2OfflineStatTable` was the template) and
  calls the two functions directly with `lua_tinker::call<int>` rather than
  reimplementing either - the second one especially is not something to
  reinvent, it is a distance-based ED curve with two tuned constants.

- **`MapData.lua` needed two globals nothing else had published**:
  `VILLAGE_MAP_ID[...]` (every `VMI_*` subscript the file indexes by name) and
  an all-caps `TRUE` (`DEFAULT = TRUE` - separate from the lowercase `True`
  already published for `RandomItemTable.lua`). `VILLAGE_MAP_ID` is the one
  enum table here with no client/server diff to do:
  `KncWX2Server/Common/Enum/Enum.h` is a header **shared** between client and
  server builds, not two independently maintained copies like
  `PET_UNIT_ID`/`UNIT_CLASS`, so there is nothing to drift and nothing to
  verify beyond transcribing it. Both added to
  `X2OfflineLuaEnum`/`X2OfflineLuaEnumSeed.h`.

- **The file also calls `MapData:AddMapData{...}` (~20 times) and
  `MapData:AddLocalMapInfo(...)` (~100 times) before the two functions Cobo
  Express needs are defined further down the same chunk.** Either one being
  unbound raises a Lua error that stops the chunk right there, and neither
  function would ever be defined at all - not merely under-populated. Cobo
  Express needs neither call's data (both build a level/dungeon-clear gate for
  entering a village by ID), so both are bound as no-op stubs: enough to let
  the chunk finish, nothing else.

- **No `CheckEnterTheVillage()` equivalent was built**, deliberately, matching
  the precedent `Handlers_Field.cpp`'s `EGS_STATE_CHANGE_FIELD_REQ` already
  set: `CreateWarpDest` only ever lists a village the player's own progress has
  already unlocked, so the client is trusted rather than this build
  re-deriving a level/dungeon-clear gate it has no data for. Same trust
  extended to `m_bFreeWarp` (a "Cobo Express VIP" ticket being active) rather
  than re-checking `m_trWarpVipEndDate`.

- **A real bug caught by the user's own play-test, not by review**: the first
  cut of `CX2OfflineMapData::IsLoaded()` only read a flag - it never called
  `EnsureLoaded()`, and `Handler_EGS_WARP_BY_BUTTON_REQ` checked `IsLoaded()`
  *before* ever calling `CheckCOBOExpressTicketMapID`, which is the only other
  place that triggers the load. Net effect: the file was never even attempted,
  packed or not, and the refusal read "MapData.lua not loaded" regardless.
  Fixed by having `IsLoaded()` call `EnsureLoaded()` itself before reporting
  status. Confirmed fixed by the user packing `MapData.lua` into `data036.kom`
  and warping successfully on the rebuilt exe.

- **Build**: `X2Lib_2010.vcxproj` then `X2_2010.vcxproj`, both `US_SERVICE`,
  0 errors, twice (once for the square/warp implementation, once for the
  `IsLoaded` fix). Deployed to `X2_offline.exe`; confirmed by size/mtime.

- **Files**: `Handlers_Square.cpp` (new), `X2OfflineMapData.{h,cpp}` (new),
  `Handlers_Social.cpp` (`Handler_EGS_WARP_BY_BUTTON_REQ` rewritten),
  `X2OfflineLuaEnum.{h,cpp}`, `X2OfflineLuaEnumSeed.h`, `X2OfflineServer.{h,cpp}`
  (dispatch + declarations for the square handlers), `X2Lib_2010.vcxproj`.

---

# Phase 14 — Pet summon/unsummon "Failed to create the pet" (`ISSUES.md` #1)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

None of the three ranked candidates was it, and the real bug was two missing
relays of the same shape - not a wrong field, not a missing enum, not a
mismapped string.

- **Candidates 2 and 3 both ruled out by reading code, not by the play-test the
  plan proposed.** Pet 99 is `PUI_PANDA_STICK_BLUE_F` (`X2PetManager.h:66`), a
  real enum entry - candidate 2's missing-mesh theory doesn't hold, and the
  summon-ACK path never even consults the pet templet before showing anything.
  Candidate 3 doesn't hold either: `NetError_def_US.h`'s English string table
  (`szErrorStrF`) gives every `ERR_PET_*` code its own distinct, correct text
  (`ERR_PET_00` = "Failed to create the pet.", `ERR_PET_06` = "You do not have
  a summoned pet.", ...), and `IsValidPacket`'s default case
  (`X2Main.cpp:7136`) already prints `GetErrStrF(enumID)` generically and
  correctly for any code that reaches it - there is no shared/generic string
  bug to fix. This made the temporary-debug-flag plan (candidate 2's
  discriminator) unnecessary; the ranked list was resolved by reading, not by
  the proposed play-test.

- **The dialog's actual cause was `Handler_EGS_SUMMON_PET_REQ` never
  special-casing `m_iSummonPetUID == 0`.** Exactly phase 10's "left alone on
  purpose" paragraph, now paid off: the unsummon button
  (`PCUM_SUMMON_CANCEL`, `X2UIPetInfo.cpp`'s `PLUCM_INFO_SUMMON_CANCEL`, and
  `X2UIInventory.cpp:2724` all call `Handler_EGS_SUMMON_PET_REQ( 0 )`) fell into
  the same `vecPet` UID search real pets use, found nothing (no real pet has UID
  0), and kept the handler's default `ERR_PET_00` - "Failed to create the pet"
  for what was only ever the unsummon click. Fixed the same way live's
  `UserPetManager.cpp:944` reads it: UID 0 means "release", answered from
  `kSes.m_nSummonedPetUID` (the field phase 10 added) - `NET_OK` if something
  was out, `ERR_PET_06` if not.

- **Fixing the dialog was not enough - "press summon, nothing happens" was a
  second, separate bug**, found only after the first fix was play-tested and
  reported back. The summon/unsummon ACK - like the `COMMANDS_FOR_PETS` ACK -
  only carries a result code; it never spawns or removes anything. On the
  client, `CreateGamePet()` / `RemovePet()` (pet appearing or disappearing) and
  `PlayEmotion()` → `StateChange()` (pet animation, and the thing that resets
  `CX2Pet::OnFrameMove`'s stuck-idle timer) live **only** inside the `_NOT`
  handlers (`Handler_EGS_SUMMON_PET_NOT`, `Handler_EGS_PET_ACTION_NOT`), never
  inside the `_ACK` handlers. Live's GameServer always relays that `_NOT` packet
  back to the acting player too, even solo: `GSUserFunction.cpp:8803`'s
  `SendPetAction` and `GSUserPet.cpp:363`'s `DBE_SUMMON_PET_ACK` both have a
  `GetFieldUID() <= 0` branch that `SendPacket`s the `_NOT` to the sender, not
  just to other players in the room - the comment already in this file
  ("the server only relays the command to the other players in the room, and
  there are none") was the exact wrong assumption, missing that the room always
  includes yourself. So this was the same missing-relay-to-self shape twice
  over:
  - `Handler_EGS_COMMANDS_FOR_PETS_REQ` now also sends `EGS_PET_ACTION_NOT` to
    the session after the ACK, using the request's own `m_cActionType` - this is
    what actually explains the 12,876x retry storm (`PlayEmotion` never ran, so
    the idle timer never reset, so `CX2Pet::OnFrameMove` asked again every
    frame).
  - `Handler_EGS_SUMMON_PET_REQ` now also sends `EGS_SUMMON_PET_NOT` after a
    successful ACK (both the unsummon branch, empty `m_vecPetInfo`, and the
    normal-summon branch, `m_vecPetInfo` holding the same `KPetInfo` the ACK
    carries) - never on the `ERR_PET_06`/no-match refusal branches, matching
    live's early-return-before-relay shape.

- **The packet log's payload-size column was not enough to tell success from
  failure while narrowing this down**, and `Handler_EGS_SUMMON_PET_REQ` had zero
  `CX2OfflineLog::Server` lines to begin with - unlike its sibling `CREATE_PET`
  and `FEED_PETS` handlers, which log every outcome. Added one line per branch
  (unsummon success, unsummon-refused, summon-refused) so a repeat of this
  symptom is diagnosable from `offline_server.log` alone next time, without
  needing to add a throwaway debug flag first.

- **Deploy-loop hazard, not code**: redeploying `X2_offline.exe` while the game
  built from the previous copy is still running fails with "the process cannot
  access the file" - the running exe holds its own file locked. The fix is
  closing the game first, not force-copying over it.

- One flag throughout, per §1 rule 2 - no new `SERV_IRUHADEV_OFFLINE_*`
  sub-flag was added for either fix.

---

# Phase 15 — Title image missing in field/dungeon (`ISSUES.md` #13)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Nothing in the plan's three-step "What to do" survived contact, and the fix is a
single assignment. The useful part of the section was the **Trap** and step 1's
advice to trace back from the client's consumer rather than forward from the
packet — that is what found it.

#### What the plan got wrong

**The Diagnosis was stale in the exact way §0.1 warns about.** It says "nothing
anywhere in `X2Lib/Offline/` sets a title id on a unit-info struct", and cites a
grep of `Handlers_Unit.cpp`. The line it was looking for is in
`X2OfflineServer.cpp`: phase 6 already set `KUnitInfo::m_iTitleID` from the save
in `MakeUnitInfoFromRow`, with a comment saying why — *"This travels on the unit
rather than in the title packet, so it has to be here or a title survives a relog
in the list and vanishes off the character."* `git log -L` dates that line to
commit `517c1cc` (phase 6, 2026-09-04), a day **before** this document was
written. A grep scoped to one file is not a grep of `X2Lib/Offline/`.

**All three of step 3's paths were wrong, and the one that mattered was not among
them.** Step 3 said to populate `m_iTitleID` on "field entry, character load, and
the P2P unit sync":

- *Character load* — already correct since phase 6, as above.
- *Field entry* — not a path at all. The offline server sends no `KFieldUserInfo`
  anywhere, and the client's only consumer of one for **my own** unit is
  `CX2TFieldGame::JoinFieldUnit`, which ignores the packet and builds the village
  unit from `g_pData->GetMyUser()->GetSelectUnit()`
  ([X2TFieldGame.cpp:891-899](X2Lib/X2TFieldGame.cpp#L891)). The only caller of
  `SetKFieldUserInfo` in the whole client is the GM-invisibility toggle
  ([X2TFieldGame.cpp:5917](X2Lib/X2TFieldGame.cpp#L5917)).
- *P2P unit sync* — carries no title. The emblem is built from the `CX2Unit`, in
  `CX2GUUser::_CommonSetComponent` and `CX2SquareUnit::UpdateEquippedEmblem`;
  nothing about it comes off the wire.

**Step 2 asked a question that was already answered.** The equipped title *is*
persisted: `unit.title_id`, written by `SaveEquippedTitle` from
`Handler_EGS_EQUIP_TITLE_REQ` and read back at
[X2OfflineDB.cpp:432](X2Lib/Offline/X2OfflineDB.cpp#L432). Confirmed against the
live save rather than by reading the schema — `reyaa` (uid 12) has
`title_id = 10` in `els_db.sql` today, and `CX2OfflineTitle::CheckEquip`
validates against the real title table before writing, so that 10 is a title the
character genuinely owns.

**The Trap was real but bit somewhere else than advertised.** Of the five title
members, `KRoomUserInfo` has exactly one live: with `SERV_TITLE_DATA_SIZE` on
(`ServerDefine.h:747`) its `short m_sTitleID` is commented out outright
([CommonPacket.h:2200](KncWX2Server/Common/CommonPacket.h#L2200)), so there is
nothing to pick wrong. Both members exist on `KUnitInfo` — and phase 6 had
already picked correctly there.

#### The one real defect

`MakeRoomUserInfo` left `KRoomUserInfo::m_iTitleID` at zero, and the damage is
wider than the packet it rides on:

- `CX2Unit::UnitData::SetKRoomUserInfo` assigns `m_iTitleId = data.m_iTitleID`
  unconditionally ([X2Unit.cpp:3455](X2Lib/X2Unit.cpp#L3455)).
- `CX2Room::SlotData::Set_KRoomSlotInfoOfMine` runs it against **my own**
  `CX2Unit` — the very object the village put the title on
  ([X2Room.cpp:2170](X2Lib/X2Room.cpp#L2170)), fetched by
  `GetMyUser()->GetUnitByUID()`.

So entering any room did not merely omit the title inside the dungeon; it
*erased* it from the character for the rest of the session. That is why the
symptom reads "field **and** dungeon" rather than one or the other: on a clean
login the village emblem is fine, the first dungeon takes it away, and it never
returns until the next `EGS_SELECT_UNIT_1_NOT`. One line fixes both halves.

#### Packets involved, none of which the plan named

The plan named no packet. The fix is in one helper, but that helper feeds
`MakeRoomSlots`, which is called from **nine** handlers — every way a room can be
entered, started, re-synced or ended:

`EGS_CREATE_TUTORIAL_ROOM_REQ`, `EGS_CREATE_ROOM_REQ`,
`EGS_QUICK_START_DUNGEON_GAME_REQ`, `EGS_JOIN_BATTLE_FIELD_REQ`,
`EGS_STATE_CHANGE_GAME_START_REQ`, `EGS_GAME_LOADING_REQ`, `EGS_END_GAME_REQ`,
`EGS_STATE_CHANGE_RESULT_REQ`, `EGS_RESULT_SUCCESS_REQ`.

Each fills a `KRoomSlotInfo` vector on its own ACK/NOT, so all nine were sending
title 0 and any one of them was enough to wipe the character's title. Fixing the
shared builder fixes all nine; there was no per-packet work.

Read and **ruled out** rather than changed: `EGS_SELECT_UNIT_1_NOT` (already
right), `EGS_EQUIP_TITLE_REQ`/`_ACK` (already right, and persisting),
`EGS_CHANGE_EQUIPPED_ITEM_IN_ROOM_NOT` / `_IN_FIELD_NOT`, `EGS_ENTER_SQUARE_ACK`,
`EGS_TOGGLE_INVISIBLE_NOT`.

Two of those deserve their reasons recorded, because both *can* write a title
onto a unit and neither does so wrongly:

- `KEGS_CHANGE_EQUIPPED_ITEM_IN_{ROOM,FIELD}_NOT` carries `m_iEquippedTitleID`
  and the client writes it straight onto the unit — but only when
  `m_vecInventorySlotInfo` is empty ([X2Room.cpp:1455](X2Lib/X2Room.cpp#L1455),
  [X2TFieldGame.cpp:2598](X2Lib/X2TFieldGame.cpp#L2598)), which is the packet's
  own "this is a title change, not a gear change" discriminator. Ordinary equip
  broadcasts fill that vector and so cannot clobber a title.
- `EGS_ENTER_SQUARE_ACK`'s `m_vecUserInfo` is empty offline, so
  `KSquareUserInfo::m_iTitleID` is never sent.

#### Decisions

- **No new flag, and no studio file touched.** Per §1 rule 2 the change is
  covered by `SERV_IRUHADEV_OFFLINE`, which already wraps every file under
  `X2Lib/Offline/`; nothing was added to `Always.h`. The client-side render
  machinery (`CX2GUUser::_CommonSetComponent`, `CX2SquareUnit::Init` →
  `UpdateEquippedEmblem`, `NotifyShowObjectChanged`) was read end to end to
  confirm it needs nothing, and left alone. The entire diff is one assignment
  plus one log field.
- **A permanent `title=` field on the `ROOM room ...` log line, not a throwaway
  `SERV_IRUHADEV_*_DEBUG` flag.** A slot that carried the wrong title and a
  client that declined to draw the right one look identical on screen, and this
  is the cheapest thing that separates them. It is one field on a line that
  already exists and is useful for any future room-slot bug, so it is not the
  short-lived scaffolding `CLAUDE.md`'s debug-flag rule is about.
- **Logged in `Handler_EGS_CREATE_ROOM_REQ`, not in `MakeRoomUserInfo`.** The
  builder runs up to nine times per dungeon run; the room-open handler runs once.
  Same information, no noise.
- **No `touch X2Lib/stdafx.cpp`.** The change is `.cpp`-only — no header inside
  the PCH was edited, so the stale-PCH hazard does not apply. It would have, had
  the fix needed a new field on a shared struct.

#### Standing note: the logs in the game directory are not the `ISSUES.md` logs

`CX2OfflineLog::Open` opens both files `"wb"`
([X2OfflineLog.cpp:30-31](X2Lib/Offline/X2OfflineLog.cpp#L30)) — **every launch
truncates them.** The 31,419-line `offline_packets.log` §0 quotes is long gone; at
the time of this phase the file held 164 lines from an unrelated short session
later the same morning. So §0's one-line bucket check only works against a log
from a run that actually reproduced the issue — on any other log it returns zero
hits for everything, which reads exactly like bucket B. Reproduce first, then
grep; and copy a log worth keeping out of the game directory before relaunching.

#### Status

Built, deployed to `X2_offline.exe` (verified by size and mtime, not by eye), and
**not yet play-tested** — the exit test needs a human. `reyaa` (uid 12) already
carries `title_id = 10`, so it is testable immediately. If the emblem still does
not draw, `grep "ROOM     room" offline_server.log` now says whether the server
sent the right id, which is what separates a remaining server bug from a client
rendering one.

---

# Phase 16 — PvP rank not drawn in the character list (`ISSUES.md` #19)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### Status, 2026-09-05 — what this phase actually required

**No packet was involved.** Phase 16 doesn't fit the §0 bucket table at all —
there's no A/B/C/D to classify, because the character-select screen that draws
the emblem runs entirely off a `KUnitInfo` the offline server already sends in
the (correctly `HANDLED`) `EGS_MY_UNIT_AND_INVENTORY_INFO_LIST_ACK`. The bug is
in how the client *reads* a field of that struct, not in whether any packet
reached the offline server. Anyone tempted to `grep` the packet log for this one
will find nothing to grep for.

**The Evidence section above is for a KUnitInfo that doesn't exist in this
build**, and following it wastes the first hour. `SERV_PVP_NEW_SYSTEM` and
`SERV_2012_PVP_SEASON2` are both unconditionally defined
(`KncWX2Server/Common/ServerDefine.h:1188` and `:2182`;
`KTDXLIB/Always.h:1135`'s unguarded `#define PVP_SEASON2` is the client-side
name for the same feature), so `KUnitInfo`'s PvP field is `m_cRank` — the
`m_iPVPEmblem` / `m_cPVPEmblem` / `m_iVSPoint` names the Evidence block cites
are compiled out entirely on both sides. **This was found by attempted compile,
not by reading the `#ifdef`s**: setting `m_iPVPEmblem` in
`X2OfflineServer.cpp` failed with `error C2039: 'm_iPVPEmblem': is not a member
of 'KUnitInfo'`, which is what sent this phase looking for what *is* a member —
the [[preprocess-to-resolve-projectx2-flags]] memory's advice, just via the
compiler's own verdict rather than an actual `cl /P` run (`cl.exe` invoked
outside an `msbuild`-prepared environment exits silently with no INCLUDE/LIB
set up; re-triggering one already-changed file through `msbuild` and reading
its error list worked instead, and is cheaper to set up).

**The real defect is client-side and has nothing to do with what the offline
server sends.** `X2StateBeginning.cpp`'s character-select draw calls
`GetPVPEmblemData( pUnit->GetPVPEmblem() )`, but under `PVP_SEASON2`
`GetPVPEmblemData` is keyed by `PVP_RANK` (`PVPRANK_NONE=0` .. `PVPRANK_RANK_SSS=9`)
while `GetPVPEmblem()` returns a `PVP_EMBLEM` *rating* bucket (`PE_RANK_E=0,
PE_RANK_D=251, PE_RANK_C=551, ...`). For `m_iRating` in `[0,251)` — i.e. every
character that has never played a ranked match — that bucket is `PE_RANK_E`,
numerically 0, which collides with the unrelated enum's `PVPRANK_NONE` and
matches no key `PVPEmblem_Season2.lua` registers (decrypted via the
[[client-lua-xor-key]] method: its lowest registered key is
`PVPRANK_RANK_ARRANGE = 1`, "Arranging"/provisional rank). `GetPVPEmblemData`
returns NULL, the texture is never set, and the picture control's un-textured
state is the black box. This reads as a genuine bug in the shipped 2014 client,
not an offline-only one — nothing about the offline server's behavior is
implicated at all, so **every** character, real server or offline, whose
rating lands in `[0,251)` should show the same black box live.

**Decision taken that the plan did not anticipate:** fixed it anyway, gated
behind `SERV_IRUHADEV_OFFLINE` rather than left unguarded, even though the root
cause isn't offline-specific. Rule 2 in §1 only permits the one shared flag for
this whole 9-27 batch, this phase's scope is the offline character list, and
there's no phase elsewhere in this document that owns "fix general client PvP
display bugs" — so the fix rides the flag this batch already has rather than
going in bare. The `#else` branch keeps the original (buggy) call intact,
consistent with the "keep the original code reachable" rule in `CLAUDE.md`.

**Two client-side changes, one bug:**
- `X2StateBeginning.cpp`: calls `pUnit->GetPvpRank()` (reads `m_cRank` directly,
  the accessor that's actually type-correct for `GetPVPEmblemData`) instead of
  `GetPVPEmblem()`.
- `X2OfflineServer::MakeDefaultUnitInfo`: sets
  `kOut.m_cRank = CX2PVPEmblem::PVPRANK_RANK_ARRANGE` — `Init()` alone leaves it
  at `PVPRANK_NONE` (0), which is *also* an unregistered key, so fixing only the
  accessor without also fixing the default would have left the same black box.
  Both changes were necessary; neither alone was sufficient.

**Process note for the next phase that touches `X2StateBeginning.cpp` or any
other CP949/ISO-8859 file:** the Edit tool re-encoded this file from ISO-8859 to
UTF-8 on the first attempt at this exact change — a ~10-line intended diff came
out as 549 changed lines, caught by `file` and `git diff --stat` per
`CLAUDE.md`'s own warning, not by anything looking wrong in the editor. Restored
via `git checkout --` (confirmed with the user first, since it's destructive)
and redone as a raw byte-level Python patch (`open(path, "rb")`, locate the
exact byte span, splice, `open(path, "wb")`) that left the rest of the file's
bytes untouched. This is the second time this exact failure mode has hit this
project (see [[ide-destroys-cp949-files]]) — treat the verify step as mandatory
on every edit to a non-ASCII file in this tree, not just a precaution for ones
that "look risky".

Built (`X2Lib` then `X2.exe`, 0 errors both), deployed to `X2_offline.exe`
(verified by size and mtime programmatically, not by eye), and **not yet
play-tested** — the exit test needs a human.

---

# Phase 17 — Result screen shows no reward (`ISSUES.md` #18)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

The plan's step 3 — instrument every early return and settle it in one play-test —
was never needed, because the item turned out to be identifiable from data
already on disk, and once it was named the gate fell out of the code in one read.

#### First: the item is `78894`, `Giant Potion (Elixir)`

The session that produced `ISSUES.md` ran 23:19–00:06 on 2026-09-04/05.
`db_backup/els_db.sql` in the game directory is a snapshot of the save from
**20:03 that same evening**, three hours before it. Diffing that snapshot against
today's `els_db.sql` shows what unit 12 was carrying when the defects were
written:

```
(4, 4) OLD (78894, 30) Giant Potion (Elixir)   NEW (109999, 1) The First Job Change Promotion Cube
(4, 5) OLD (78894, 30) Giant Potion (Elixir)   NEW (110850, 1) The Second Job Change Promotion Cube
(4, 6) OLD (78894, 30) Giant Potion (Elixir)   NEW (135184, 100) Sage's Magic Stone
(4, 7) OLD (78894,  4) Giant Potion (Elixir)   NEW (221600, 1) ?
(4, 8) OLD (77200, 96) Stamina Potion          NEW (130079, 2) Weapon Enhancement Stone Lv.3
```

Category 4 is `CX2Inventory::ST_SPECIAL`. Four stacks of `78894` totalling 94 of
the 100 the offline cash shop grants (`X2OfflineCashSeed.h:75`) — so six had
already been consumed — and, for free, **phase 20's item as well**:
`77200 Stamina Potion`, 96 of 100. Both are gone from the save today, which is
why reading only the current `els_db.sql` finds neither.

`78894` is `ELIXIR_GIANT_POTION` ([X2Define.h:1458](X2Lib/X2Define.h#L1458)), one
of the eight `SERV_NEW_DEFENCE_DUNGEON` elixirs. It is **not** `215680 Mana
Elixir`, the quick-bar item that also survives in the save and that a name-guess
lands on first.

#### How the names were read — this is reusable, and it is new

`ScriptData/ItemTemplet.xlsx` is useless here: none of `78894`, `77200`,
`215680`, `500040` or `500720` exists in it, exactly as `CLAUDE.md`'s *stale
snapshot* rule warns. The client's own item table was read instead.

The game directory holds an unpacked `data036/` beside the `.kom` files, and
every `.lua` in it is XOR-encrypted with a **repeating 12-byte key**:

```
02 AA F8 C6 DC AB 47 26 EF BB 00 98
```

Recovered as known-plaintext against the Lua 5.1 header (`1B 4C 75 61 51 00 01 04
04 04 08 00`); the period was confirmed independently by autocorrelation over
200 KB of `Item.lua` — peak at displacement 12, with 24/36/288/300 as harmonics.
XOR the file with that key and out comes ordinary `luac` 5.1 bytecode
(`@Item.lua`, `@ItemTrans.lua`, …). `ItemTrans.lua`'s constant pool is a flat
`number, name, description` run, so a 20-line scan for a `TNUMBER` followed by a
`TSTRING` yields **43,577 item id → English name pairs** — the live US names, not
the KR spreadsheet's. That is also why `221600` prints as `?` above: it is absent
from `ItemTrans.lua` too.

It settles the buff question directly as well. Dumping the strings around the
`78894` constant in `Item.lua` gives:

```
'Giant Potion (Elixir)', 'HQ_Shop_Item_78894.dds', 'BFI_BUFF_ELIXIR_GIANT_POTION'
```

— the client's own row for the item carries a `BUFF_FACTOR` entry. So
`pItemTemplet->GetNumBuffFactorPtr()` is non-zero for it, and
`CX2Game::ApplyBuffFactorToGUUser` can apply the real effect with no server data
at all.

#### The gate, in two halves

**Half one is stock client behaviour, not an offline defect.** In
`CX2UIInventory::OnRClickedItem` the eight `SERV_NEW_DEFENCE_DUNGEON` elixirs and
the four `BELSSING_OF_*` blessings share one `case`
([X2UIInventory.cpp:6723-6771](X2Lib/X2UIInventory.cpp#L6723)):

```c
switch ( g_pMain->GetNowStateID() )
{
case CX2Main::XS_DUNGEON_GAME:
case CX2Main::XS_BATTLE_FIELD:
    Handler_EGS_USE_ITEM_IN_INVENTORY_REQ( m_DefencedungeonPotion, 1, m_DefencedungeonPotionID );
    break;
default:
    KTDGUIOKMsgBox( ... GET_STRING( STR_ID_16529 ) ... );   // no packet, ever
    return true;
}
```

Right-clicked in a village or a field, the elixir puts **no byte on the wire** —
exactly the §0 bucket-B signature the plan measured, and it would look identical
against the live server. Nothing to fix.

#### Half two is the real defect

The plan's evidence could not see it, because it happened in an *earlier*
session: those six missing elixirs. Inside a dungeon the packet **is** sent, and
`CX2OfflineServer::Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` consumed the item,
replied `NET_OK`, logged `ITEM used item ... from the bag` — and applied nothing.
The client's ACK handler
([X2UIInventory.cpp:8908](X2Lib/X2UIInventory.cpp#L8908)) only refreshes the bag,
the ED and the sort order; it never touches the unit. So the elixir was eaten and
did nothing, six times over.

This is **phase 4's correction 6 for the fifth time**: the `_NOT` is what applies
an item, the ACK only bookkeeps. `Handlers_Inventory.cpp` already carried that
comment — on the quick-slot handler, twenty lines below the bag handler that
needed it too.

#### The fix

One block in `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ`: after the consume succeeds
and the ACK is away, if the session is in `S_ROOM` and the item templet has a
special ability or a buff factor, send `EGS_USE_QUICK_SLOT_NOT` with the item id
and the unit uid. `CX2UIQuickSlot::Handler_EGS_USE_QUICK_SLOT_NOT`
([X2UIQuickSlot.cpp:1003-1077](X2Lib/X2UIQuickSlot.cpp#L1003)) is the client's
**only** caller of `UseItemSpecialAbility` and `ApplyBuffFactorToGUUser`, and it
works off the item id alone — it does not care that the item came from a quick
slot. Both guards matter: its `default:` branch is an
`ASSERT( !L"Can not use this item in this State" )`, and an item with neither an
ability nor a buff factor would only add a spurious "you used X" chat line.

**Deliberately not done: the live server's own mechanism.**
`KGSUser::ActivateItemBuff`
([GSUserFunction.cpp:15978](KncWX2Server/GameServer/GSUserFunction.cpp#L15978))
maps the item to a `CXSLBuffManager::BTI_*` id, activates a server-owned buff
with a wall-clock duration, and pushes the whole world-buff list back as
`EGS_UPDATE_BUFF_INFO_IN_ROOM_NOT`. Reproducing that needs a **buff factor id**,
and the client cannot derive one from a buff templet id — `CX2BuffTemplet` does
not store its factor list, and `CX2BuffTempletManager::GetBuffFactorPtr` is
indexed, not keyed. It would also need a persisted buff table. The item's own
`BUFF_FACTOR` reaches the same factor by the path the quick-slot potions already
use offline. **The behavioural difference to know about:** the buff lasts the run
rather than a wall-clock duration across rooms, and it draws no buff icon in the
village.

#### Hand off to phases 20-22

- **Phase 20's item is `77200 Stamina Potion`** — 96 of them in the special tab
  at 20:03, in the diff above. Check its `Item.lua` row for a `BUFF_FACTOR` or a
  special ability before assuming anything. If it has one and is used inside a
  dungeon, **this phase's fix already covers it** and phase 20 may be a
  five-minute confirmation. If it is meant to be used in a village, it is a
  different path: `Handler_EGS_USE_QUICK_SLOT_NOT` asserts outside a room, and
  `CX2GageData::UseSpecialAbilityInVillage` (`X2UIQuickSlot.cpp:975`) is the
  village equivalent.
- **The gate to read first for any "cannot use item X" defect** is
  `CX2UIInventory::OnRClickedItem`'s giant item-id `switch`
  (X2UIInventory.cpp:6214-6900). Most items fall through its `default:` and send
  the packet; the ones that do not are gated on game state, on some other UI
  being open, or on a confirm dialog — and none of those gates ever reaches the
  offline server.
- **Name the item before theorising.** The `data036/` XOR key above turns "the
  elixir" into `78894` in about a minute, and the pre-session
  `db_backup/els_db.sql` says what the character was actually holding when the
  defect was written. Both were available to every earlier phase and neither was
  used.

**How this hand-off actually fared:** the item ids were right and saved phase 20
its whole diagnosis. The *prediction* was wrong — the Stamina Potion is a cube on
`EGS_OPEN_RANDOM_ITEM_REQ`, so this phase's fix does not touch it and phase 20
was not a five-minute confirmation. Neither phase 21 nor 22 shared this gate
either. The second and third bullets, which are technique rather than prediction,
both paid off. See §0.2.

#### Exit test status

**Verified in play, 2026-09-05.** The test has to be run *inside a dungeon or a
battlefield* — that is the only place the stock client will send the packet at
all. `offline_server.log`, on two different elixirs:

```
[12:28:29.429] ITEM     used item 78894 from the bag
[12:28:29.429] ITEM     item 78894 has 0 ability / 1 buff factor(s) - sent USE_QUICK_SLOT_NOT so the effect applies
[12:29:11.196] ITEM     used item 78896 from the bag
[12:29:11.196] ITEM     item 78896 has 0 ability / 1 buff factor(s) - sent USE_QUICK_SLOT_NOT so the effect applies
```

`0 ability / 1 buff factor` is the confirmation that mattered: it is the client's
own `Item.lua` row for the item reporting exactly one `BFI_BUFF_*` entry and no
special ability, which is what `ApplyBuffFactorToGUUser` needs and what the
static read of the bytecode predicted.

---

# Phase 20 — Stamina potion not working (`ISSUES.md` #11)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Phase 19's hand-off named the item (`77200 Stamina Potion`, 96 of them in the
special tab at 20:03) and guessed it might be covered by phase 19's fix. It is
not — it is a different packet entirely, and the defect is worse than "does
nothing": **the potion was emptying the stamina bar.**

#### The potion is a cube, not a bag item

`77200` is `SPIRIT_POTION_ITEM_ID` ([X2Define.h:932](X2Lib/X2Define.h#L932)) and
it has a row in `RandomItemTable.lua`, so `CX2UIInventory::OnRClickedItem` falls
past the whole `GetCanUseInventory()` block into the random-item path and sends
`EGS_OPEN_RANDOM_ITEM_REQ`. It is also in `NO_CUBE_OPEN_ITEM_ID[]`, which only
picks the plain confirm dialog over the cube-opening animation. So this was never
`EGS_USE_ITEM_IN_INVENTORY_REQ` and phase 19's `_NOT` does not touch it.

#### `m_iRestoreSpirit` is not the amount to restore

That is the whole defect, and the field name is the trap. On the live server:

```c
iRestoreSpirit = m_kUserSpiritManager.RestoreSpirit( iRestoreSpirit );
```
[GSUserInventory.cpp:2099](KncWX2Server/GameServer/GSUserInventory.cpp#L2099) —
and `KUserSpiritManager::RestoreSpirit` adds, clamps to the max, and **returns the
new absolute spirit** ([UserSpiritManager.cpp:40](KncWX2Server/GameServer/UserSpiritManager.cpp#L40)).
The client then assigns it straight onto the unit:

```c
int getSpirit = ack.m_iRestoreSpirit - unit->m_iSpirit;
getSpirit = (int)( getSpirit / (float)unit->m_iSpiritMax * 100.0f );   // shown as "+N%"
unit->m_iSpirit = ack.m_iRestoreSpirit;                                // absolute
```
[X2UIInventory.cpp:9690-9697](X2Lib/X2UIInventory.cpp#L9690).

The offline handler passed the raw table value through:

```c
kAck.m_iRestoreSpirit = pData->m_iRestoreSpirit;
```

with a comment saying it was "carried through untouched … what a stamina cube
exists to hand over". `RandomItemTable.lua`'s row for `77200` is
`m_iRestoreSpirit = 1440` (also
[KncWX2Server/ServerResource/US/RandomItemTable.lua:229-236](KncWX2Server/ServerResource/US/RandomItemTable.lua#L229)),
and offline pins spirit to `SHRT_MAX` in `CX2OfflineServer::FillSpirit`. So
drinking a Stamina Potion set the gauge to **1440/32767 = 4.4%** and popped a
message box reading **"stamina restored by -95%"**. Four of them were drunk before
the `ISSUES.md` session, which is why the count had gone 100 → 96.

#### Why the plan's bucket-B reading was wrong

§0 put this in bucket B ("the client never sent the packet") on the grounds that
no `EGS_*STAMINA*` or `EGS_*VITAL*` event id exists. Neither does — the system is
spelled **SPIRIT** in this codebase (`EGS_RESTORE_SPIRIT_NOT`,
`EGS_DECREASE_SPIRIT_NOT`, `m_iSpirit`, `KSpiritTable`), and the potion rides on
`EGS_OPEN_RANDOM_ITEM_REQ` regardless. A grep for the English word the UI shows
missed a subsystem the offline server had already half-implemented.

#### The fix: model it, then refuse the no-op

The offline server pins spirit full on purpose — `FillSpirit` returns `SHRT_MAX`
for both current and max, with a comment explaining that `SHRT_MAX` is the
largest value `dbo.GSpirit`'s `smallint` column can hold and therefore clears
every dungeon's `m_RequireSpirit` entry check. Nothing offline ever decrements
it, so there is genuinely nothing for a potion to restore.

So the handler now does the arithmetic the server does — `min( max, spirit +
restore )` — **before** the cube is consumed, and refuses with
`ERR_RANDOM_ITEM_05` ("The cube cannot be opened") plus a log line when the
answer is a no-op. The potion stays in the bag. Written as a real clamp rather
than an unconditional refusal so that if spirit is ever modelled properly the
potion starts working with no further change.

This is the plan's second exit-test branch, taken deliberately: "*or using it is
refused with a message and the potion is not consumed*". Modelling stamina for
real would need `MAX_SPIRIT` out of `SpiritTable.lua`, which is **not** in
`KncWX2Server/ServerResource/US/` — and it would make offline play strictly worse
by gating dungeon rewards behind a daily counter nobody can refill.

#### Also found, not fixed

`m_iRessurectionCount` on the same ACK has the identical absolute-vs-delta shape
(`SetResurrectionStoneNum( ack.m_iRessurectionCount )`,
[X2UIInventory.cpp:9656](X2Lib/X2UIInventory.cpp#L9656)) and is still passed
through raw. That is harmless only while the character holds no resurrection
stones, which offline it always does — `EGS_SELECT_UNIT_1_NOT` sends
`m_iNumResurrectionStone = 0`. A comment now says so at the call site.

#### Exit test status

**Verified in play, 2026-09-05.** The Stamina Potion is cash product 56 (100 for
the seeded price). Right-clicked twice, refused twice, stack intact:

```
[13:02:06.848] CUBE     refused - item 77200 restores 1440 stamina but stamina is already 32767/32767 offline, so it would be spent for nothing (the potion is left in the bag)
[13:02:09.385] CUBE     refused - item 77200 restores 1440 stamina but stamina is already 32767/32767 offline, so it would be spent for nothing (the potion is left in the bag)
```

---

# Phase 21 — "Camilla's secret manual" cannot be used (`ISSUES.md` #9)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

The plan's guess — "a skill-unlock book … very likely exactly that case, and the
current behaviour is a deliberate safety refusal" — was right about *what the
item is* and wrong about *why nothing happened*. The item never reached the
`GetCanUseInventory()` refusal, because the mapping it needed turned out to be
client data that was sitting in `data036/` all along.

#### The item, and where its skill list lives

`Camilla's Secret Manual` is four items, `270970`–`270973`
(Basic / Intermediate / Advanced / Expert), all four sold by the offline cash
shop (`X2OfflineCashSeed.h:116-119`). `ItemTrans.lua`'s description settles what
they do with no ambiguity:

> Camilla's Secret Manual that contains information on a Basic skill.
> Use this item to learn your character's **sealed** Basic skill.

They are `AddSealSkillInfo` rows in **`SkillData.lua`** — a client script, packed
in `data036`, already parsed into `CX2SkillTree::m_mapSealSkillItemTemplet` at
startup. Decrypted (see phase 19 for the key), `270970`'s row reads:

```
m_iItemID = 270970, m_eUnitClassType = UCT_FIRST_CLASS,
m_SkillID = { 1209, 1214, 1223, 2006, 2407, 2414, 3208, 3214, 3225, 4005,
              4215, 4220, 5209, 5011, 5018, 6005, 6012, 6222, 7211, 7215 }
```

— twenty skills, one per class-and-tier, and `270972`/`270973` carry the
`UCT_SECOND_CLASS` set. So there was never any server data to ask for: the
item → class → skill mapping ships with the client.

#### What the client already does, and what it needs back

`CX2UIInventory::OnRClickedItem` recognises the manual at
[X2UIInventory.cpp:6161](X2Lib/X2UIInventory.cpp#L6161), resolves the one skill
of the twenty that is in *this* character's tree with
`CX2SkillTree::GetUnsealSkillItemInfo`, refuses with `STR_ID_3856` and **no
packet** if none is (that is the bucket-B half, and it is stock behaviour), and
otherwise pops a confirm box and sends `EGS_USE_ITEM_IN_INVENTORY_REQ`.

Past that point it is phase 19's shape again: **the ACK unlocks nothing.** The
live server replies OK and then, out of `DBE_UNSEAL_SKILL_ACK`, pushes a separate
`EGS_UNSEAL_SKILL_NOT { m_iSkillID }`
([GSUserGameCommon.cpp:6716-6730](KncWX2Server/GameServer/GSUserGameCommon.cpp#L6716)).
`CX2SkillTree::Handler_EGS_UNSEAL_SKILL_NOT` is the only thing in the client that
calls `AddSkillUnsealed`, redraws the tree and opens the "skill unsealed" dialog.
Offline sent no such packet, so the manual was consumed and the skill stayed
sealed.

#### And it had to survive a relog

`CX2UserSkillTree::SetUnsealedSkill` **replaces** the whole set from
`EGS_SELECT_UNIT_1_NOT::m_vecSkillUnsealed`, which the offline server was
clearing unconditionally — with a comment reading *"Nothing offline seals or
unseals anything, and an empty list reads as 'no skill has been unsealed', which
is correct rather than a stub."* True when it was written; it would have re-sealed
the skill on the next login the moment the unseal worked. This is the §0.1
pattern for the third time in this batch.

#### The fix

- **Schema v10**, `unit_skill_unsealed( unit_uid, skill_id )` — `dbo.GUnsealSkill`
  on live. Additive rung, so existing saves upgrade.
- `CX2OfflineSkill` grows the unsealed set, loads it in `Load()`, and gets
  `SkillForUnsealItem()` — a thin wrapper over the client's own
  `IsUnsealSkillItemID` / `GetUnsealSkillItemInfo`, so the resolution is the same
  call, on the same data, as the client made a moment earlier.
- `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` refuses **before consuming** when the
  item unseals nothing for this class or the skill is already unsealed
  (`ERR_USE_ITEM_IN_INVENTORY_00`, the code the live server uses for the class
  mismatch at [GSUserInventory.cpp:5596](KncWX2Server/GameServer/GSUserInventory.cpp#L5596)),
  and after consuming sends `EGS_UNSEAL_SKILL_NOT`. That order is the plan's
  Trap, honoured: the manual is never spent on a grant that cannot land.
- `EGS_SELECT_UNIT_1_NOT` now fills `m_vecSkillUnsealed` from the save.

#### Exit test status

**Verified in play, 2026-09-05.** All four manuals used on a Rena, each
resolving to a different skill, and the unlocked one then learned and slotted:

```
[12:59:05.289] ITEM     used item 270970 from the bag
[12:59:05.289] SKILL    item 270970 unsealed skill 3214
[12:59:07.408] SKILL    item 270971 unsealed skill 3013
[12:59:08.973] SKILL    item 270972 unsealed skill 3222
[13:00:39.193] SKILL    item 270973 unsealed skill 3409
[12:59:26.634] SKILL    skill 3013: level 0 -> 5
```

Persistence confirmed on the next launch —
`SKILL loaded for unitUID=12: 18 skill row(s), 4 unsealed, …` — and in the save
itself, `unit_skill_unsealed` holding `(12, 3214) (12, 3013) (12, 3222)
(12, 3409)`.

Note the tier rule: `270970`/`270971` are `UCT_FIRST_CLASS` and
`270972`/`270973` are `UCT_SECOND_CLASS`, and a manual with nothing for the
current class is refused by the *client*, before the server sees it.

---

# Phase 22 — Skill notebook not working (`ISSUES.md` #12)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

The plan's hypothesis was right in substance and wrong about the packet. It said
the client "likely believes it has **zero pages** and disables the UI", and named
`EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT` as the thing nothing offline originates.
The zero was real; it was arriving on a different packet, and it was a literal in
our own code:

```c
kNot.m_cSkillNoteMaxPageNum         = 0;
kNot.m_mapSkillNote.clear();
```
`Handlers_Unit.cpp`, inside `EGS_SELECT_UNIT_1_NOT`. Both fields ride on the
character-load packet (`KEGS_SELECT_UNIT_ACK` / `_1_NOT`,
[CommonPacket.h:8178](KncWX2Server/Common/CommonPacket.h#L8178)), and the `_NOT`
the plan named is only the *later* push for when the count changes mid-session.

With the count at 0, `CX2UIInventory::OnRClickedItem`'s `IT_SKILL_MEMO` branch
hits `GetMaxSkillNoteSlot() <= 0`, shows `STR_ID_4988` and returns without
sending — bucket B, exactly as described.

#### 0 is the right default, and the note is what changes it

The page count is not a constant to be looked up; it is *earned*. Two items grant
it and both call the same function:

- `CXSLItem::SI_SKILL_NOTE_ITEM` = **99600**, "Skill Notebook"
  ([GSUserInventory.cpp:4610](KncWX2Server/GameServer/GSUserInventory.cpp#L4610))
- `CXSLItem::CI_CASH_SKILL_NOTE_ITEM` = **221600**, the cash version
  ([GSUserCashShop.cpp:1585](KncWX2Server/GameServer/GSUserCashShop.cpp#L1585))

Both do `GetExpandSkillNotePage( GetLevel(), n )` then
`UpdateSkillNoteMaxPageNum( n )`. And `GetExpandSkillNotePage` is a plain C++
table in the repo, not server Lua —
[UserSkillTree.cpp:907-940](KncWX2Server/GameServer/UserSkillTree.cpp#L907):

| level | pages |
|---|---|
| below 20 | none (returns false) |
| 20-29 | 1 |
| 30-39 | 2 |
| 40-49 | 3 |
| 50-59 | 4 |
| 60-69 | 5 |

So the plan's "find that default in the client's own constants or in
`KncWX2Server/`; do not pick a number" is satisfied by transcribing that switch.
`221600`'s own item description agrees: *"Level Requirement: 20 or higher"*.

**`221600` is the one that matters here**: it is cash product 65 in
`X2OfflineCashSeed.h` and `99600` is not in the seed at all, so the cash note is
the only route to a page offline — and it is the item `reyaa` was already
carrying in special slot 7 when `ISSUES.md` was written. (It shows as `???` in
phase 19's inventory dump because `ItemTrans.lua` has no name row for it.)

#### The fix

- **Schema v10** (shared with phase 21): `unit.skill_note_page` for the count and
  `unit_skill_note( unit_uid, page, memo_id )` for the memos — two separate
  things, the way the server reads them back with two separate procedures,
  `gup_get_notecnt` and `gup_get_note`
  ([GSGameDBThread.cpp:1822-1845](KncWX2Server/GameServer/GSGameDBThread.cpp#L1822)).
- `CX2OfflineSkill` loads both in `Load()` and gains
  `ExpandSkillNotePageForLevel()` (the table above, verbatim),
  `SetSkillNoteMaxPage()` and `RegisterSkillNoteMemo()`.
- `Handler_EGS_USE_ITEM_IN_INVENTORY_REQ` accepts either note id: refuses before
  consuming with `ERR_SKILL_NOTE_07` below level 20 and `ERR_SKILL_NOTE_06` when
  it would not grow what the character owns, and otherwise consumes and sends
  `EGS_EXPAND_SKILL_NOTE_PAGE_NUM_NOT`. That is the packet that calls
  `SetSkillNoteMaxPage` **and** `HideSkillNote( false )`, which is what makes the
  skill-note button appear in the inventory.
- `EGS_SELECT_UNIT_1_NOT` fills both fields from the save, so a page survives a
  relog.
- `Handler_EGS_REG_SKILL_NOTE_MEMO_REQ` is implemented properly. Its stub said
  *"the memo IDs come from a server table"* — they do not. `KGSUser` reads the
  memo ID straight off the item being spent:
  `const int iMemoID = m_kInventory.GetItemID( m_iItemUID )`
  ([GSUserGameCommon.cpp:7260](KncWX2Server/GameServer/GSUserGameCommon.cpp#L7260)).
  There was never a table to be missing — a fourth §0.1 case. The handler now
  makes the same three checks in the same order (item exists, page exists, memo
  not already registered), consumes the memo only after they pass, and returns
  `m_vecSkillNote` so `SetEqipSkillMemo` applies the memo's effect.

#### A claim made here was wrong: the shop *does* sell memo cards

The first pass said "A preset saves" could not be tested because the offline cash
shop seeds no `IT_SKILL_MEMO` card. **That was wrong**, and the play-test found it
within a minute — three memos were registered on the first try.

The search that produced the claim was the mistake: it grepped the 2,360 seeded
product names for "Note", "Memo" or "Manual". **Memo cards are named after the
skill they teach**, not after what they are — `603002 Phoenix's Affinity`,
`603004 Easy Catch`, `603010 Concentrated Air Technique`. There are fifteen of
them in `X2OfflineCashSeed.h`, products 184-198, ids `603001`-`603015`, and they
were sitting in the seed the whole time.

The general lesson is the same one phase 19 wrote down and this phase then
ignored: **resolve ids, do not pattern-match names.** A name search answers
"is there an item called X", which is a different question from "is there an item
that does X".

#### Exit test status

**Failed in play** — see the second pass below. The play-test produced one line
about `221600` and it was `ITEM discarded`, not `ITEM used`: the client never sent
a use packet for it at all, so the bag branch this pass added could not fire.

#### Second pass — it is a claim-time item, not a bag item

The first pass got the page table and the persistence right and the **packet
wrong**, and the play-test said so immediately. `offline_server.log`:

```
[13:00:11.450] ITEM     discarded 1 x item 221600 (itemUID=169)
```

A discard, and nothing else. No `ITEM used item 221600 from the bag`, no refusal —
so the client never sent `EGS_USE_ITEM_IN_INVENTORY_REQ` for it at all, and the
bag branch that pass added could not fire.

**221600 is never a bag item on the live server.** The two note ids are handled in
two different places, and only one of them is the use-item switch:

| id | server handler | when |
|---|---|---|
| `99600` `SI_SKILL_NOTE_ITEM` | [GSUserInventory.cpp:4610](KncWX2Server/GameServer/GSUserInventory.cpp#L4610) | used from the bag |
| `221600` `CI_CASH_SKILL_NOTE_ITEM` | [GSUserCashShop.cpp:1585](KncWX2Server/GameServer/GSUserCashShop.cpp#L1585) | **claimed out of the cash deposit** |

Both call `GetExpandSkillNotePage` then `UpdateSkillNoteMaxPageNum`; the cash one
does it inside `EGS_GET_PURCHASED_CASH_ITEM_REQ`, so the note is spent at the
moment it is picked out of the deposit window and no inventory item is ever
created. The offline claim handler had no branch for it, so it fell to the
generic "insert into the bag" tail and produced an item that sits there and
cannot be used — which is precisely `ISSUES.md` #12.

And 221600 is the only route offline: `99600` is not in `X2OfflineCashSeed.h`.

#### The corrected fix

`Handler_EGS_BILL_GET_PURCHASED_CASH_ITEM_REQ` now has a **claimed, not carried**
branch for `CASH_SKILL_NOTE_ITEM_ID`, the third of that shape in the same handler
after the class-change ticket and the inventory-expansion card, and written to
match them: both refusals (level below 20, or no more pages than the character
already has) happen *before* `DeleteCashOrder`, so a refused note stays claimable
in the deposit rather than vanishing.

`99600` is deliberately left on the bag path — that one really is a bag item on
live, and the branch the first pass added is correct for it.

#### What the first pass got right and keeps

Everything except the packet: schema v10's `unit.skill_note_page` and
`unit_skill_note`, `ExpandSkillNotePageForLevel` transcribed from
[UserSkillTree.cpp:907](KncWX2Server/GameServer/UserSkillTree.cpp#L907), the
`EGS_SELECT_UNIT_1_NOT` fill so a page survives a relog, and the real
`EGS_REG_SKILL_NOTE_MEMO_REQ` handler. The v10 migration is already proven on the
live save — `pragma user_version` reads 10 and phase 21's four unsealed skills
are sitting in `unit_skill_unsealed`.

#### Exit test status, second pass

**Verified in play, 2026-09-05 — both halves.** The note claimed out of the
deposit window at level 50 gave 4 pages, and three memo cards were then
registered into them:

```
[13:08:50.622] CASH     claimed line 50: item 221600 gave the skill note 4 page(s)
[13:09:19.735] SKILL    memo 603002 registered on page 0 (1 memo(s) now)
[13:09:28.089] SKILL    memo 603010 registered on page 1 (2 memo(s) now)
[13:09:32.504] SKILL    memo 603004 registered on page 2 (3 memo(s) now)
```

All of it in the save afterwards: `unit.skill_note_page = 4` for unit 12, and
`unit_skill_note` holding `(12, 0, 603002) (12, 1, 603010) (12, 2, 603004)`.

Buy the note from the cash shop and **claim it out of the deposit window** rather
than looking for it in the bag. A `221600` already sitting in a bag from an
earlier claim is stranded — the client has no path for it there. Discard it and
buy a fresh one.

---

# Phase 23 — Cannot enhance equipment (`ISSUES.md` #4)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Built and deployed 2026-09-05. **The data can be sourced, so this is the "if the
data arrives" branch, not the "improve the refusal" one — phases 24-26 are not
folded in.** The blocking question above answered itself on the first `ls`:
`KncWX2Server/ServerResource/US/EnchantTable.lua` exists, 442 lines, and it holds
`SetEnchantProbability` for every level 1-20. No live-DB query was needed.

#### The refusal was half wrong, and that half is the finding

`Handler_EGS_ENCHANT_ITEM_REQ`'s comment claimed the per-level success rates
**and the stat multipliers** were server data "with no client copy". The stat
multipliers have a client copy and always did:

| `EnchantTable.lua` | client | agree? |
|---|---|---|
| `SetEnchantRate( 0..20 )` | `ENCHANT_STAT_SCALE[]` ([X2Define.h:212](X2Lib/X2Define.h#L212)) | **identical, all 21 values** |
| `AddEnchantStoneInfo` (130077-130094) | `NEW_{WEAPON,DEFENCE}_ENCHANT_STONE_ITEM_ID[]` ([X2Define.h:325](X2Lib/X2Define.h#L325)) | identical, and `CX2UIShop::GetEnchantStoneLevel` is the same banding |
| `SetWeaponEnchantStone( 109950 )` etc. | `NORMAL_*_ENCHANT_STONE_ITEM_ID` ([X2Define.h:318](X2Lib/X2Define.h#L318)) | identical |
| `SetEnchantProbability` | **nothing** | — |

So exactly one of the three things the refusal named has no client copy. That
also makes the packed file self-checking: the loader compares its
`SetEnchantRate` rows against `ENCHANT_STAT_SCALE` and logs
`ENCHANT  21 stat rate(s) match the client's own ENCHANT_STAT_SCALE - right
file.`, which is a free proof that the archive holds the US table and not some
other region's.

**This is §0.1's seventh-and-eighth stale refusal.** The same sentence had also
propagated into `X2OfflineQuest.cpp`'s `IsClearTypeUndriven`, where it justified
leaving `SQT_ITEM_ENCHANT` in the "nothing can ever drive this" list. Both are
now corrected, and socketing is stated on its own rather than as half of a pair.

#### The bucket-B half was never a bug

`EGS_ENCHANT_ITEM_REQ` is genuinely absent from the census, and the reason is
the client working correctly. `CX2UIShop::EnchantItem`
([X2UIShop.cpp:2729](X2Lib/X2UIShop.cpp#L2729)) refuses to open the dialog at all
unless the character holds a stone **in the item's own level band**, and the save
says exactly why that fired:

```
unit 12 (reyaa, level 50) holds  130077 x7, 130078 x3, 130079 x2   (weapon, bands 0-20 / 21-30 / 31-40)
                                 130086 x8, 130087 x3, 130088 x1   (armour, same three bands)
   ...and none of 130080-130085 / 130089-130094, which is what level-40+ gear needs.
```

Nothing to fix and nothing to remove: with the right stone the dialog opens and
the packet goes. **This is the §0.2 lesson landing the other way round** — the
last four phases found bucket-B calls hiding a real defect, and this one is a
bucket-B call that is simply true and benign.

#### What was built

- **`X2Lib/Offline/X2OfflineEnchantTable.{h,cpp}`** — `CXSLEnchantItemManager`'s
  client-side twin, loaded exactly the way `X2OfflineResolveTable` loads
  `ResolveTable.lua`. **Every** Lua method the file names is bound, including
  those inside its own block comments and those whose `SERV_` flag is TW/HK-only:
  an unbound method is a Lua error that abandons the rest of the chunk, and the
  probability rows are at the very end of the file. Rows this phase does not read
  are parsed, counted and stored anyway — `AddAttachItemInfo` is the enchant-attach
  ticket table **phase 25 needs** (it is in this same file), `AddRestoreItemInfo`
  is the repair scrolls, `Add{Drop,Cube}RandomEnchantInfo` belongs to phase 27.
- **`Handler_EGS_ENCHANT_ITEM_REQ`** — `KInventory::EnchantItem`
  ([Inventory.cpp:12098](KncWX2Server/GameServer/Inventory.cpp#L12098)) plus its
  `KGSUser` wrapper and the ACK that `DBE_ENCHANT_ITEM_ACK` sends
  ([GSUserInventory.cpp:3421](KncWX2Server/GameServer/GSUserInventory.cpp#L3421)),
  gate for gate and in the server's order: bank, sealed, templet, `CanEnchant`,
  broken, fluor-stone level cap, +20 cap, ED cost, stone selection, stone in bag,
  not worn, roll, cheat box, fluor stone, apply, consume, charge. Compiled for
  this build's flag set: `SERV_ENCHANT_PLUS_ITEM` and `SERV_DESTROY_GUARD_ITEM`
  are TW/HK-only so those two arms do not exist, while
  `SERV_BLESSED_RURIEL_ENCHANT_STONE_EVENT` **is** on for US and rewrites the
  legacy-stone branch (rare first, then Ruriel's `152000121`/`152000122`, then
  plain) — the client's own stone count adds the same two IDs under the same flag.
- **`CX2OfflineInventory::SetEnchantLevel`** — one absolute setter rather than
  `Increase`/`DecreaseEnchantLevel`, because the caller has already turned the
  five results into a level. A **negative** level is valid and is how a broken
  item is marked; the setter refuses only outside ±20, which is what the `char`
  on the wire can carry.
- **`CX2OfflineInventory::AddEquippedStat` now applies the enchant multiplier**,
  through the client's own `ENCHANT_STAT_SCALE` and `CX2Item::GetEnchantStat`'s
  own gate (non-fashion weapon/armour, indexed by `abs(level)` under
  `ITEM_RECOVERY_TEST`). This is not cosmetic: `m_kGameStat` is what
  `CX2GUUser::InitStat` prefers, so without it a +7 weapon would have been worth
  nothing to the character's HP in a dungeon. Its header comment used to say
  "nothing offline can enchant … so every item's enchant level is zero".
- **`CX2OfflineQuest::OnEnchantItem`** — `KUserQuestManager::Handler_OnEnchantItem`
  ([UserQuestManager.cpp:2877](KncWX2Server/GameServer/UserQuestManager.cpp#L2877)),
  called on every result **except** no-change, exactly as the real server does
  (a no-change attempt returns before the DB round trip that triggers it). This is
  what unblocks the two title sub-quests the phase-8 census flagged as
  `clearType=13 ** NOT DRIVEN OFFLINE`. One rename to watch: the server's
  `m_ClearCondition.m_iItemID` is the client's `m_iCollectionItemID` — the same
  Lua key parsed into a differently-named field
  ([X2QuestManager.cpp:2292](X2Lib/X2QuestManager.cpp#L2292)) — and the level
  match is `!=`, i.e. **exact**, so a step asking for +5 is not satisfied by +6.

#### Exit test status

**VERIFIED IN PLAY, 2026-09-05.** The user packed `EnchantTable.lua` and
enhancement works with real rolls, real ED and a real failure:

```
[16:15:10.113] ENCHANT  loaded: 29 probability row(s), 20 stone row(s), 59 fluor row(s), 21 stat-rate row(s), 140 other row(s)
[16:15:10.113] ENCHANT  stones: weapon 109950 / rare 109960, armor 109965 / rare 109970; fluor usable below +10 (+11 during an event)
[16:15:10.113] ENCHANT  21 stat rate(s) match the client's own ENCHANT_STAT_SCALE - right file.
[16:15:10.113] ITEM     enhanced item 113532: +0 -> +1 (success), stone 130077, 2040 ED, 184180 ED left
                              ... +1 -> +2, +2 -> +3, +3 -> +4 ...
[16:15:20.367] ITEM     enhanced item 113532: +4 -> +3 (down one), stone 130077, 2040 ED, 176020 ED left
[16:15:22.415] ITEM     enhanced item 113532: +3 -> +4 (success), stone 130077, 2040 ED, 173980 ED left
```

The down-one at +4 is the table working: `SetEnchantProbability( 4 )` is
`Up1 = 60, NoChange = 40`, and +5 onward is where `Down1` first appears - the
run above went 100/100/80/60 and then met the 30% at +5's row. The stat-rate
cross-check line is the packed file confirming it is the US table.

The text below is what the degrade path looked like before it was packed, kept
because it is what the next unpacked table will look like:

```
ENCHANT  ERROR 'EnchantTable.lua' not found in any .kom or on disk - enhancement is OFF.
ENCHANT  XOR-encrypt KncWX2Server/ServerResource/US/EnchantTable.lua and pack it into data036.kom.
ITEM     refused an enhancement - EnchantTable.lua is not loaded. ...
```

`data036/` in the game directory currently holds 182 scripts and
`EnchantTable.lua` is not among them. Once it is, the loader logs
`ENCHANT  loaded: N probability row(s), ...` and the stat-rate cross-check line,
and the attempt itself logs
`ITEM     enhanced item <id>: +N -> +M (success|no change|down one|reset to +0|BROKEN), stone <id>, <ED> ED, <ED> ED left`.

To test it, use a **bag** item (worn gear is refused with `ERR_ENCHANT_ITEM_08`,
which is correct) whose use level falls in a band the character has stones for —
on `reyaa` that means level-40-or-below gear, not the equipped set.

---

# Phase 24 — Cannot socket equipment (`ISSUES.md` #3)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Built and deployed 2026-09-05, together with 25 and 26. **The trap above was
right and the diagnosis was right**: socketing really is a different table and a
different roll from enhancement. It is also in the tree —
`KncWX2Server/ServerResource/US/SocketItemTable.lua`, 3,812 lines, the largest of
the four this batch needed.

#### What has a client copy and what does not

Same audit as phase 23, different answer in one place:

| piece | where it lives |
|---|---|
| socket **cost** | client. `CX2UISocketItem::CalculateNewSocketCost` ([X2UISocketItem.cpp:829](X2Lib/X2UISocketItem.cpp#L829)) is `CalcInsertSocketCost`, same 0.01 and the same four grade factors |
| how many **slots** an item has | client. `CX2SocketItem::SOCKET_COUNT` ([X2SocketItem.h:169](X2Lib/X2SocketItem.h#L169)) is what the socket window draws |
| what an option **does** | client. `OptionItemData.lua` + `SocketOptionForm.lua`, loaded at start-up ([X2StateStartUp.cpp:189](X2Lib/X2StateStartUp.cpp#L189)) |
| which **option** a stone rolls | **server only** — `SocketItemTable.lua` |

So this phase touches none of the option maths. It rolls an option ID and puts
it in the item's socket list; the client reads it back through its own tables.

#### The magic-stone table was extracted, not typed

`CXSLSocketItem::GetSocketDataType` is a ~450-line switch and
`IsMagicStoneItemID` a second one over the same stones. Both were run through a
small preprocessor with this build's flag set applied and flattened into one
58-row table in `X2OfflineSocketTable.cpp`. It self-checks three ways:

1. The 58 stones it maps are **exactly** the 58 `IsMagicStoneItemID` accepts.
2. Every one of the 117 distinct `SOCKET_DATA_TYPE` names the packed
   `SocketItemTable.lua` subscripts resolves in the client's enum.
3. The client enum's own tail pins `SDT_QUEST_CLEAR_COUNT = 120`, and counting
   the members ahead of it lands on exactly 120 — so the seeded values cannot
   be off by one without that pin disagreeing.

**One real asymmetry, and it is the server's to win.** The client's socket UI
offers 16 Luriel stones (`85003840..`, `152000699..`) that are not in the table,
because `SERV_LURIEL_MAGIC_STONE` is not defined **anywhere** in
`KncWX2Server/Common` — the live server would have refused them too. They are
refused with a log line naming the stone rather than silently rolling the wrong
pool.

#### Two deliberate departures from the server

- **The GM socket cheat is not honoured.** `m_bCheat` lets the request carry
  chosen option IDs straight into the item, gated on `UAL_GM`. There is no
  operator offline and the auth level is whatever the offline login handed out,
  so honouring it would let any modified client write arbitrary socket options.
  Refused with `ERR_VERIFY_12` and a log line.
- **Cost and stones are checked in a first pass over the whole request.** One
  request can fill several slots; the real server prices them all, then checks
  the wallet, then applies. That shape is kept so a request that cannot be paid
  for changes nothing at all.

`SQT_ITEM_SOCKET` is now driven too — `CX2OfflineQuest::OnSocketItem`, counting
the slots one request filled and clamping to what the step asks for, which is
what the real hook does with its `UCHAR` counter.

#### Exit test status

**VERIFIED IN PLAY, 2026-09-05.** The user packed `SocketItemTable.lua` and
socketed a weapon repeatedly:

```
[16:57:50.486] SOCKET   loaded: 216 random row(s) over 114 pool(s), 3586 group row(s) over 182 group(s), 58 magic stone(s) known
[16:57:50.487] ITEM     socket slot 1 of item 117040 <- option 40110 (stone 135184)
[16:57:50.487] ITEM     socket slot 2 of item 117040 <- option 40708 (stone 135184)
[16:57:50.487] ITEM     socketed item 117040: 2 slot(s) for 1424 ED, 33176 ED left
[16:57:53.644] ITEM     socket slot 2 of item 117040 <- option 40710 (stone 135184)
[16:57:53.644] ITEM     socketed item 117040: 1 slot(s) for 6414 ED, 26762 ED left
                              ... 40710, 41010, 40320, 40936, 40712 ...
```

Three things are visible in those six lines and all three are the port working:
**one request filling two slots** (the packet is plural — see §0.3), **different
option IDs out of the same stone** (the group lottery is rolling, not returning a
constant), and **the empty-versus-occupied price rule** — two empty slots cost
1424 together, then every re-socket of the now-occupied slot 2 costs a flat 6414,
which is `CalcInsertSocketCost`'s ×3 replace multiplier scaled by the item's
assigned socket count.

---

# Phase 25 — Cannot use magic amulet (`ISSUES.md` #5)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Built and deployed 2026-09-05. **Step 1's hunch was right, and better than
right: it is deterministic AND its table was already loaded.** No new file, no
new loader, no ask.

The "magic amulet" is an *enchant-attach ticket* — `강화권` / `강화의 부적`, an
item that simply sets a piece of gear to a fixed enhancement level. Its table is
`AddAttachItemInfo` **inside `EnchantTable.lua`**, which phase 23 has been
parsing since the day it was packed, precisely because unbound methods abort a
Lua chunk. Phase 23's note that "phase 25's table is in the file phase 23 already
loads" turned out to be exactly the shape of the work: two accessors on
`CX2OfflineEnchantTable`, and a handler.

`KInventory::EnchantAttachItem` ([Inventory.cpp:16066](KncWX2Server/GameServer/Inventory.cpp#L16066))
is a straight gate list with no roll anywhere in it: ticket is a ticket, target
is enchantable, the ticket's item-type restriction (`IT_NONE` = any, `IT_DEFENCE`
= armour only), `CheckAttachItemEnableLevel`'s use-level band, not broken, not
worn, and **the target's current level must be strictly below what the ticket
grants** — an amulet only ever raises. Then it sets the level and eats the
ticket. `SERV_ENCHANT_ATTACH_MODIFY` is on in this build, which is why the ticket
is decremented if it stacks and deleted whole if it does not.

**One thing deliberately not done.** The amulet reaches an enhancement level, so
it looks like it should feed phase 23's `SQT_ITEM_ENCHANT` quest hook. It does
not, because the real server does not: `DBE_ENCHANT_ATTACH_ITEM_ACK` has no
`Handler_OnEnchantItem` call, only the enhancement path does. A quest step that
asks the player to *enhance* an item is not satisfied by buying the level.

#### Exit test status

**The hang is gone and the data is loaded; the effect itself is the one thing in
this batch with no log line of its own yet.** The ignore rule is removed and
every path replies, and `EnchantTable.lua` — which carries this phase's whole
table — has been packed and verified since phase 23, so there is nothing left to
pack. Look for:

```
ITEM     amulet <ticket id> set item <item id> to +N (was +M)
```

or, when the ticket is not usable on that piece, one of the refusal lines naming
why (`is already +N, ticket grants +M` / `is not an enchant-attach ticket` /
`ticket <id> has item type N`). It needs an enhancement ticket in the bag —
`강화권` / `강화의 부적`, IDs `130147`-`130152`, `132495`-`132500`,
`85002810`-`85002860` and the Ruriel `60007250`-`60007300` block among others.

---

# Phase 26 — Cannot add equipment attributes (`ISSUES.md` #10)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

### What actually happened

Built and deployed 2026-09-05. Three ignored event IDs, and the trap fired
exactly as written.

#### `EGS_ADD_ON_STAT_REQ` is not part of this feature and never was

The name collides and nothing else does. It is an **in-match relay**: the
GameServer forwards it to the room server
([GSUserRoomCommon.cpp:4122](KncWX2Server/GameServer/GSUserRoomCommon.cpp#L4122))
and the answer comes back as `EGS_ADD_ON_STAT_NOT`, which the client applies with
`CX2GUUser::SetAddOnStat` ([X2Game.cpp:8660](X2Lib/X2Game.cpp#L8660)) — a
temporary stat buff on a unit inside a dungeon. It stays ignored, because there
is no room server offline and the P2P host applies its own buffs. **Its ignore
reason was wrong and is now fixed**, which is the §0.1 pattern again: the rule
was right, the sentence next to it was not.

#### Most of this feature was already client-side

The server loads three files for attributes; the client loads two of them, and
one is shared:

| file | server | client |
|---|---|---|
| `AttribEnchantRequire.lua` | yes | **yes** — shard count + ED cost, via `CX2EnchantItem::AddEnchantRequire_LUA` |
| `AttribEnchantItem.lua` | no | yes — the damage effects |
| `AttribEnchantTable.lua` | yes | **no** — the random rolls |
| `AttribAttachTable.lua` | yes | **no** — the amulets |

So is the logic: `CX2EnchantItem::CanEnchantAttribute` is
`CXSLAttribEnchantItem::IsPossibleToPush` (the fire/water/nature and
wind/light/dark exclusions), `GetItemID` is `GetRequireItemID`, and `ATI_*`,
`ESI_SLOT_*`, `ACT_*` and `ENCHANT_TYPE` are all in
[X2EnchantItem.h](X2Lib/X2EnchantItem.h) with the server's values. The two cost
functions are **called on the client's object** rather than re-ported —
`GetAttribEnchantRequireMagicStoneCount` and `GetAttribEnchantRequireED` are the
same table the real server reads.

`CX2OfflineAttribTable` is therefore small: three random lotteries (single, dual
keyed by the existing attribute, triple keyed by the existing pair), the amulet
map, and the identify rows, which are parsed and stored but have no accessor
because `EGS_IDENTIFY_ITEM_REQ` is a different packet and still ignored.

#### Two loaders now borrow a Lua global and give it back

`AttribEnchantTable.lua` and `AttribAttachTable.lua` are written against
`g_pCX2EnchantItem`, and `SocketItemTable.lua` against `g_pCX2SocketItem` — both
names the **client's own** managers publish for themselves, and neither client
manager binds the methods those files call. So both loaders point the global at
themselves for the duration of the `DoMemory` calls and restore
`g_pData->GetEnchantItem()` / `GetSocketItem()` immediately afterwards, logging a
warning if they cannot. The client republishes those globals only from
`OpenScriptFile`, which runs once at start-up, long before either loader.

This is worth knowing before adding a fifth loader: `EnchantTable.lua`'s
`EnchantItemManager` and `ResolveTable.lua`'s `g_pResolveItemManager` are names
the client does **not** use, so those two could simply take them. These two
could not.

#### The rules that are easy to get wrong, and were transcribed rather than guessed

- **`GetAttribCountType` counts non-empty slots, not leading ones.** An item with
  slot 0 empty and slot 1 filled is `ACT_SINGLE`, not `ACT_NONE` — which is what
  decides both the price and which lottery runs.
- **`ET_RANDOM` is resolved *inside* the legality check**, before anything is
  charged, and a roll that finds no row comes back `ET_NONE`. Left alone that
  would silently turn "add an attribute" into "clear the slot", so it is refused
  explicitly.
- **For a third attribute the triple lottery IS the legality table** —
  `KLottery::IsExistCase`, not a rule — so even a non-random request needs the
  packed file at that point.
- **Removing an attribute is a real, paid operation**, not a no-op, and reports a
  short wallet with its own error code so the dialog can say which action failed.
- **The amulet overwrites all three slots**, including with `ET_NONE`, because
  the server's loop walks `ESI_SLOT_1..MAX` unconditionally.

#### Exit test status

**VERIFIED IN PLAY, 2026-09-05**, reported by the user after packing both files.
The El shard half is confirmed working; `offline_server.log` is rewritten per
session and the run that exercised it has since been overwritten, so the line
quoted below is the loader from the run *before* `AttribAttachTable.lua` was
packed — which is why it says zero amulets:

```
[16:52:06.100] ATTRIB   ERROR 'AttribAttachTable.lua' failed to run, encrypted or plaintext.
[16:52:06.100] ATTRIB   loaded: 6 single, 24 dual, 42 triple, 0 amulet(s), 12 identify row(s) - AttribAttachTable.lua missing, amulets will refuse
```

**That first line is a lie, and chasing it is what found §0.3's third finding.**
The file was not corrupt, it was simply not packed; `NULL == kInfo` could never
be true, so the not-found branch never fired. After the fix the same situation
reads `ATTRIB   ERROR '...' not found in any .kom or on disk.` followed by the
XOR-and-pack instruction.

Worth knowing: a *specific* attribute (not random, not a third one) needs no
packed file at all — everything that path touches is client-side — so it works
on the El shards and ED alone. Random attributes and third attributes need
`AttribEnchantTable.lua`; amulets need `AttribAttachTable.lua`. The healthy load
line is `ATTRIB   loaded: 6 single, 24 dual, 42 triple, 15 amulet(s), 12 identify
row(s)`.

---

# Phase 27 — Regular drops such as "Aqua" never drop (`ISSUES.md` #17)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

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

# Phase 28 — Summoned pet doesn't survive relog / character switch (not in `ISSUES.md`)

**Flag:** `SERV_IRUHADEV_OFFLINE` (shared flag — do not create a per-phase flag; see §1 rule 2)

Found 2026-09-05 while playtesting phase 14's fix, and diagnosed in that same
conversation before being split off here — the fix is bigger than a packet-logic
bug (it needs a save-schema change), so it gets its own phase rather than
riding on phase 14's commit.

### Symptom
Summon a pet, then log out and back in (or switch characters and back): the
pet is gone and has to be summoned again by hand. On live, a summoned pet is
still summoned next login — nothing has to be redone.

### Evidence
This is a design-comparison finding, not a log grep — read live's own source
rather than trying to catch it in `offline_packets.log`:

- `KncWX2Server/GameServer/GSGameDBThread.cpp:2256-2359` (`DBE_SELECT_UNIT_REQ`
  handler) calls `exec dbo.gup_get_pet_list`, which returns a `bIsSummoned` bit
  per pet row; whichever row has it set becomes `kAck.m_iSummonedPetUID`
  (reset to 0 first, only set from the DB — not derived from anything else).
- `GSGameDBThread.cpp:15588-15672` (`DBE_SUMMON_PET_REQ`) calls
  `exec dbo.gup_update_pet_call <petUID>, 0` then `..., 1` on every
  summon/unsummon — the flag is written to the DB immediately, not just at
  logout.
- `UserPetManager.cpp:251-267` (`KUserPetManager::Init`, run from
  `GSUserFunction.cpp:5189` right after character select): if
  `iSummonedPetUID > 0`, it fetches that pet and calls
  `spSummonedPet->Summon(...)` immediately, before the player has done
  anything - the summoned pet is restored as part of character select, not
  something the player re-triggers.

Offline has none of this: `KOfflineSession::m_nSummonedPetUID`
(`X2OfflineServer.h`) is in-memory session state, reset to 0 on every new
session, and `KOfflinePetRow` (`X2OfflineDB.h:231`) has no "is this pet
summoned" column to restore from even if the session survived.

### Diagnosis — CONFIRMED
Live persists "currently summoned pet" per character and restores it
transparently on every character select. Offline does not persist it at all.
This is a real functional gap, not a misreading of live behavior.

### What to do
1. **Add a persisted flag to the save.** `KOfflinePetRow` gets a
   `bool m_bSummoned` (or equivalent), `X2OfflineDB.h`'s `SCHEMA_VERSION` goes
   from 10 to 11, with a migration step — `X2OfflineDB` migrates forward only,
   and `els_db.sql` is real character data (`reyaa`, `wewswe`, `caswe` as of
   this writing), so treat the migration with the same care `CLAUDE.md`'s
   save-file caution describes. Don't guess a default; a migrated-in column
   should read as "not summoned" (`false`/0) for every existing pet row, which
   is exactly first-login behavior on live for a character that never had one
   summoned.
2. **Persist on every summon/unsummon**, not just `kSes.m_nSummonedPetUID`.
   `Handler_EGS_SUMMON_PET_REQ` (`X2Lib/Offline/Handlers_Social.cpp`, the
   function phase 14 already touched twice) needs to `SavePet()` the flag
   change on both the pet being summoned (`true`) and whatever was previously
   summoned, if any (`false`) - mirroring `dbo.gup_update_pet_call`'s pair of
   calls.
3. **Restore on character select, not on every field/map transition.** This is
   the open question phase 14's conversation didn't settle: the hook needs to
   fire once per login (matching live's `DBE_SELECT_UNIT_ACK` timing), not on
   every dungeon/village transition within an already-running session, or the
   pet would visibly re-spawn every time the player walks through a door.
   `Handler_EGS_SELECT_UNIT_REQ` (`X2Lib/Offline/Handlers_Unit.cpp:393`, "the
   last step of character select") is the candidate for reading the persisted
   flag back into `kSes.m_nSummonedPetUID`; `Handler_EGS_FIELD_LOADING_COMPLETE_REQ`
   (`X2Lib/Offline/Handlers_Field.cpp:109`) is the candidate for actually
   sending the `EGS_SUMMON_PET_NOT` relay (phase 14's mechanism) once the field
   has finished loading, since the client needs to be in a state that can
   render the pet before that packet arrives. Confirm which of these actually
   fires exactly once per login before wiring the restore into it - neither was
   read closely enough in the phase-14 conversation to be sure.

### Trap
Do not restore and send the `EGS_SUMMON_PET_NOT` relay from anywhere that also
runs on ordinary field-to-field travel within a session (e.g. a generic
"entered a field" hook shared with dungeon entry) - the client already keeps
its own pet state for the lifetime of the running process, and re-sending the
spawn packet there would make the pet flicker or re-play its spawn animation
on every map change instead of only once per login.

### Exit test
Summon a pet, log out (or switch to another character and back), log back in
as the same character: the pet is already out, without touching the summon
button.

### What actually happened

**The plan's own open question resolved in its favor, with one correction.**
`Handler_EGS_SELECT_UNIT_REQ` does fire exactly once per login - confirmed by
reading it, not assumed. `Handler_EGS_FIELD_LOADING_COMPLETE_REQ` turned out
**not** to be "every field transition" the way the Trap section worried: a
dungeon/battlefield uses a different completion packet entirely
(`EGS_BATTLE_FIELD_NPC_LOAD_COMPLETE_REQ`, `Handlers_Room.cpp`), so this
handler only runs on village entry. That narrows the risk but doesn't remove
it - a session can walk through several villages - so the fix still needed a
one-shot gate rather than leaning on "this only fires once." That gate is
`KOfflineSession::m_bPetRestorePending`: set once by `RestoreSummonedPet` (at
select-unit), consumed once by `SendPendingPetRestore` (at the next village
load), regardless of how many times the handler around it fires afterward.

**Why the spawn can't happen at character select is a client fact, not a
guess.** `X2PetManager.cpp:2415`'s `CreateGamePet` bails out unless
`g_pX2Game != NULL`, and that doesn't exist yet on the character-select
screen - confirmed by reading the client function before relying on the
plan's own framing of it.

**A field the plan never named needed fixing too.** `KEGS_SELECT_UNIT_3_NOT.
m_iSummonedPetUID` (`Handlers_Unit.cpp`, `PushSelectUnitNotifications`) had
been hardcoded to 0 since phase 7 with a comment claiming "none owned and
none summoned, which is true rather than a stub." True when written, wrong
the moment a pet could survive a relog - the same stale-refusal shape §0.1
tracks, just on a bookkeeping field instead of a request handler. It now
reads `kSes.m_nSummonedPetUID` once `RestoreSummonedPet` has set it. This
field only feeds `CX2Unit::SetSummonPetUid` (which pet `EGS_FEED_PETS_REQ`
resolves against) - it does not spawn anything, so getting it wrong would
have been silent rather than visibly broken, which is exactly why it survived
three phases unnoticed.

**Decisions the plan left open:**
- Schema v11 adds `unit_pet.summoned` (`KOfflinePetRow::m_bSummoned`),
  defaulting to 0 for every existing row on migration - the same "never
  summoned" state a character on live starts in, and the only honest answer
  for a row this migration has no history for.
- `Handler_EGS_SUMMON_PET_REQ` persists the flag on **both** branches, not
  just summon: the unsummon branch (`m_iSummonPetUID == 0`) now clears the
  previously-summoned row's flag, and the summon branch clears whichever pet
  was out before *and* sets the new one, so switching directly between two
  pets without unsummoning first still leaves exactly one row marked
  summoned.
- If the persisted flag names a pet that no longer exists by the time
  `SendPendingPetRestore` runs (released, or the save hand-edited), it logs a
  `WARNING` and clears `kSes.m_nSummonedPetUID` rather than silently doing
  nothing on every later village load for the rest of the session.

**Second bug, found only by play-testing the first fix, not by reading
code.** The pet reappeared in the 3D world correctly after a relog, but
reopening the pet menu jumped straight to the detail view (correct - the
client already thinks a pet is out) with no render, no name, and the button
still reading "Summon" instead of "Unsummon" (wrong). Every one of those
reads `CX2Unit::GetPetInfo()` (`X2UIPetInfo.cpp:1154`, `:1560`, `:1667`, ...),
which is populated by `SetFullPetInfo` - and `Handler_EGS_SUMMON_PET_NOT`
only calls `SetFullPetInfo` for someone *else's* pet
(`X2PetManager.cpp:2426`, `if( false == bMyPet )`); for your own pet it
assumes the just-received `EGS_SUMMON_PET_ACK` already did it, because on a
real summon it did. The restore path only ever sent the `_NOT`, so that
assumption was false the one time it mattered: `GetSummonPetUid()` (set by
`EGS_SELECT_UNIT_3_NOT`, giving the "jump to detail view" behavior) and
`GetPetInfo()` (never set) disagreed.

**Fix:** `SendPendingPetRestore` now sends a `KEGS_SUMMON_PET_ACK` before the
`_NOT`, reproducing the same two-packet order a live summon uses. Confirmed
safe to send unsolicited: `UIServerEventProc`
(`X2PetManager.cpp:401`) dispatches `EGS_SUMMON_PET_ACK` unconditionally on
receipt, and `Handler_EGS_SUMMON_PET_REQ` (the function that sends the REQ,
`X2PetManager.cpp:2226`) never registers an `AddServerPacket` wait for it -
there is no pending-request check to fail.

First fix (visible spawn on relog) was play-tested and confirmed working.
Second fix (pet menu render/name/button) was built (`X2Lib` then `X2.exe`,
0 errors both) and deployed to `X2_offline.exe`, verified by size and mtime
programmatically - **not yet re-played** at the time of writing, only
reasoned from the same client code the first bug was found in.

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
