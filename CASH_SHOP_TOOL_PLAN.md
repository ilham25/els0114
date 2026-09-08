# X2CashShopTool — a cash shop catalog editor for offline mode

## Context

Offline mode's cash shop is driven by one SQLite table, `cash_product` in
`els_db.sql`. It was populated once, by the schema v9 migration, from
`X2Lib/Offline/X2OfflineCashSeed.h` — a transcription of `dbo.EB_Product` out of
the original ES_BILLING database. **Nothing in the client ever writes that
table**; `CX2OfflineCashShop` reads it on first use and never again
([X2OfflineCashShop.h:98](X2Lib/Offline/X2OfflineCashShop.h#L98)), and the seed
rung is `INSERT OR IGNORE` inside `if( iFrom < 9 )`
([X2OfflineDB.cpp:905-948](X2Lib/Offline/X2OfflineDB.cpp#L905)), so on a save
already at v11 it can never run again.

That leaves the catalog frozen in a shape nobody chose:

- **2,360 products, every single one priced at 1.** The billing DB's real prices
  were not in the source table, so `SEED_CASH_PRICE` is a flat 1
  ([X2OfflineDB.h:432](X2Lib/Offline/X2OfflineDB.h#L432)). The wallet is a
  cosmetic 999,999 that is compared but never deducted.
- **No way to add an item.** 48,754 items exist in the client's own `Item.lua`
  (measured in phase 1; the "~43,000" this plan estimated before running it was
  low); 2,360 of them are purchasable, and the other ~46,000 are unreachable.
- **No way to remove or recategorise one**, or to fix a product that landed in a
  tab that makes no sense.
- **388 of the 2,360 rows never reach the shop at all.** A real logged run,
  recorded at `OFFLINE_MODE_PHASE9_PLAN.md:3848-3854`:

  ```
  [23:22:53.590] CASH  catalog: 1972 product(s) from 2360 cash_product row(s);
                       388 dropped for having no item templet
  ```

  16% of the catalog is invisible, and the only trace is that one log line. The
  tool holds the same item universe the client does, so it can flag those rows
  *before* they are saved — the single most useful validation it can offer, and
  it costs one join.

The header of `X2OfflineCashShop.h` says outright that the catalog "can be
edited there" — in the save file. This tool is the thing that makes that
sentence true, instead of a hand-written `UPDATE` in a SQL prompt against a
schema with no names in it.

**Outcome**: a standalone Windows tool, run out of the game directory, that
shows the catalog as the game shows it — the game's own tabs, the game's own
item icons, the game's own item names — and lets you add, edit, delete and
bulk-reprice products, writing straight to `cash_product`.

---

## What was verified before planning, and why it matters

Every one of these was confirmed first-hand against the real game directory
(`F:\elsword stuff\elsword_2014\els_2014\237311\22191271\data`), not inferred.
They are the load-bearing facts; a phase that contradicts one of them is wrong.

### 1. The catalog table, and that it is safe to write

```sql
CREATE TABLE cash_product (
  product_no INTEGER PRIMARY KEY,      -- NOT autoincrement: the tool allocates max+1
  item_id    INTEGER NOT NULL,
  category   INTEGER NOT NULL,         -- billing category 11..63, NOT the CSSC_* enum
  quantity   INTEGER NOT NULL DEFAULT 1,
  price      INTEGER NOT NULL DEFAULT 1,
  is_event   INTEGER NOT NULL DEFAULT 0 );
CREATE INDEX ix_cash_product_item ON cash_product( item_id );
```

Live state read out of `els_db.sql.bak`: 2,360 rows, `product_no` 1..2360,
`price` = 1 for all 2,360, `quantity` ∈ {1, 10, 20, 100}, `is_event` set on 60,
`PRAGMA user_version` = 11.

**Two hard limits.** `MakeProductInfo` narrows both `category` and `quantity`
to `char` on the wire ([X2OfflineCashShop.cpp:166](X2Lib/Offline/X2OfflineCashShop.cpp#L166)),
so the tool must reject anything over 127 in either field. `price` is a plain
`int` and is unconstrained.

**One silent failure to guard.** A row whose `item_id` has no client templet is
dropped at load with only a log line
([X2OfflineCashShop.cpp:85-89](X2Lib/Offline/X2OfflineCashShop.cpp#L85)). Since
the tool picks items *from* the client's own table, it cannot produce one — but
it should still validate on insert rather than trust that.

### 2. `category` is the billing number space, not the client enum

This is the single easiest thing to get wrong, and the offline code's own header
says a previous attempt got it wrong
([X2OfflineCashShop.h:23-31](X2Lib/Offline/X2OfflineCashShop.h#L23)).

`CX2CashShop::GetItemByCategory` matches `m_cCategoryNo` against the **second**
element of each pair in `CashShopCategory.lua`
([X2CashShop.cpp:6222](X2Lib/X2CashShop.cpp#L6222)), whose structure is:

```lua
CASH_SHOP_REAL_CATEGORY_ID = {
  [CASH_SHOP_CATEGORY.CSC_FASHION] = {
      REAL_ID = <n>,
      CASH_SHOP_REAL_SUB_CATEGORY_ID = {
          { CASH_SHOP_SUB_CATEGORY.CSSC_FASHION_WEAPON, <billing category no> },
          ...
      },
  },
  ...
}
```

parsed at [X2CashShop.cpp:3120-3189](X2Lib/X2CashShop.cpp#L3120). So the tab
list and the legal `category` values are **data, read at runtime**, not
constants to be typed into the tool. The tool builds its tabs from this file for
the same reason the client does.

Verified present and decodable: `CashShopCategory.lua` is in `data036.kom`, and
its constant pool names `CSC_FASHION / CSC_ACCESSORY / CSC_CONSUME /
CSC_INSTALL / CSC_PET / CSC_EVENT` and their `CSSC_*` sub-entries.

### 3. The full `.kom` → item table chain, proven end to end

Run in Python against the real archives, so none of this is guesswork:

| Step | Detail |
|---|---|
| Archive header | `char[50]` version string, `UINT iTotalFileNo` at offset 52, `UINT bCompressed` at 56 — `sizeof(MASSFILE_HEADER)` is 60 |
| V.0.3 directory | then `DWORD filetime, adler32, headersize`, then a **plaintext XML manifest** at offset 72: `<File Name=".." Size=".." CompressedSize=".." />` |
| Member offsets | cumulative `CompressedSize`, starting at `72 + headersize` |
| Decompress | one whole zlib stream per member (`uncompress()`) |
| Decrypt | XOR with a rotating 3×DWORD key, 4 bytes at a time, key index cycling 0→1→2→0 |
| Key | `XOR_KEY0 0xc6f8aa02`, `XOR_KEY1 0x2647abdc`, `XOR_KEY2 0x9800bbef` ([KTDX.h:388](KTDXLIB/KTDX.h#L388)) |
| Result | `1B 4C 75 61 51` — **Lua 5.1 precompiled bytecode** |

**The last row is the decisive one.** `Item.lua` is 34,211,379 bytes of *luac
bytecode*, not source. There is no regex, no text scrape, no shortcut: the tool
must embed a Lua 5.1 interpreter and execute it, which is also exactly what
`CLAUDE.md` means by "load it the shipped way".

**And there is no binary fast path either.** `X2OPTIMIZE_ITEM_TEMPLET_PREPROCESSING`
is defined ([Always.h:2352](KTDXLIB/Always.h#L2352)), so the client first tries
a preprocessed `Item.kim` and only falls back to the two-pass Lua load
([X2ItemManager.cpp:230-281](X2Lib/X2ItemManager.cpp#L230)). A scan of every
member name across all 145 archives finds **no `.kim` file at all** — so this
install always takes the Lua path, and so must the tool. Don't go looking for
`Item.kim`.

Everything needed is in one archive, `data036.kom`:

| File | Decompressed | Role |
|---|---|---|
| `Enum.lua` | 473,104 | defines `ITEM_TYPE`, `CASH_SHOP_CATEGORY`, `CASH_SHOP_SUB_CATEGORY` … as **tables**. Must run first or every later chunk errors on its first index |
| `Item.lua` | 34,211,379 | 48,754 `g_pItemManager:AddItemTemplet{ … }` calls (counted in phase 1; this table estimated "~43k" beforehand and was low) |
| `ItemTrans.lua` | 6,601,013 | localized name overlay, `AddItemTempletTrans{ … }` |
| `CashShopCategory.lua` | 3,193 | the tab → billing-category table above |

The fields the tool needs off each item: `m_ItemID`, `m_Name`, `m_ShopImage`,
`m_ItemType`, `m_ItemGrade`, `m_bFashion`, `m_EqipPosition`
([X2ItemManager.cpp:403-486](X2Lib/X2ItemManager.cpp#L403)).

The call shape is a **method** call — `g_pItemManager:AddItemTemplet{ … }` — so
the receiver is argument 1 and the field table is argument 2
([X2Main.cpp:3319](X2Lib/X2Main.cpp#L3319)). `lua_tinker` is not needed: its
value is binding C++ *classes* with overload resolution, and this is one function
reading one table. Build the receiver as a plain table of C functions:

```c
lua_newtable( L );                                  /* stand-in g_pItemManager */
lua_pushcfunction( L, l_AddItemTemplet );
lua_setfield( L, -2, "AddItemTemplet" );
lua_pushcfunction( L, l_AddItemTempletTrans );
lua_setfield( L, -2, "AddItemTempletTrans" );
lua_setglobal( L, "g_pItemManager" );
/* arg 1 = the stand-in (ignore); arg 2 = the item table */
```

**Give that table a catch-all `__index` metamethod returning a shared no-op
closure.** `Item.lua` is 34 MB of studio data and may call methods nobody has
enumerated; without the catch-all, one unknown name raises `attempt to call a
nil value` and the entire chunk is lost. Logging each name the metamethod sees
enumerates the full method list in one run instead of one run per missing stub.

**Order is not optional**: `Enum.lua` → `Item.lua` → `ItemTrans.lua` →
`CashShopCategory.lua`, all in the *same* `lua_State`. `Item.lua`'s
`m_ItemType = IT_WEAPON` are plain global reads; if `Enum.lua` has not run they
are `nil`, `lua_tonumber` yields **0**, and the result is a silently wrong index
rather than an error. Assert a couple of known enum globals are non-nil straight
after `Enum.lua` and fail loudly. `CashShopCategory.lua` needs no stub at all —
it only assigns a table, so run it and walk the global.

> **Corrected by phase 1 — `Enum.lua` publishes TABLES, not flat globals.**
> `IT_WEAPON` and friends come back `nil`; the scripts write
> `m_ItemType = ITEM_TYPE.IT_WEAPON`. The ordering requirement above is real and
> unchanged, but the assertion must probe `ITEM_TYPE` / `ITEM_GRADE` /
> `USE_CONDITION` / `EQIP_POSITION`, not the bare names. See phase 1's
> corrections; the tool now also *reads* its enum constants out of those tables
> rather than transcribing them.

**Expected cost**, to be replaced with measurements in phase 1: inflate is
sub-second; the in-place XOR is ~20 ms; `luaL_loadbuffer` on 34 MB of bytecode is
the expensive step at roughly **2–6 s and 60–120 MB resident**, because `lundump`
materializes every `Proto`, `TString`, boxed `TValue` and the debug `lineinfo`;
running the chunk adds 1–3 s of transient garbage. **Peak process 200–350 MB**,
which is survivable in Win32 but is why `LargeAddressAware` is set in phase 0
rather than after phase 1 surprises someone. Two easy wins: free the 34 MB
plaintext immediately after `luaL_loadbuffer` (it copies), and `lua_close` the
moment extraction finishes, before any window exists.

### 4. Icons are whole files, not an atlas

`CX2SlotItem::CreateItemUI` sets the icon from `pItemTemplet->GetShopImage()`
with the one-argument `SetTex`, falling back to `HQ_Shop_Ui_Noimage.dds`
([X2SlotItem.cpp:255-265](X2Lib/X2SlotItem.cpp#L255)). `m_ShopImage` is a
verbatim filename (convention `HQ_Shop_Item_<id>.dds`), resolved through the
same flat archive index — so no UV rects, no `.TET` sidecar, no atlas maths.

Sampled from the real archives: **64×64 DXT1 or DXT5 DDS, 2,176 / 4,224 bytes**,
and **not XOR-encrypted** — the bytes out of the archive are a plain DDS. They
are spread across many archives (`data016`, `data020`, `data067` alone holds
16,931 files, `data130` …), so the index must cover all 145, exactly as
`X2Main.cpp:1332-1354` mounts them.

A DXT1/DXT5 decoder is ~150 lines of well-known block code. **No Direct3D device
is needed anywhere in this tool.**

Scale, for sizing the index: a scan across all 145 archives sees at least 88,723
member names, of which 5,277 follow the `HQ_Shop_Item_<id>` convention (many more
icons use other names, e.g. `HQ_SHOP_AISHA_CASH_UPBODY10.dds` — another reason to
read `m_ShopImage` rather than derive it). A flat name → (archive, member) map at
that size is trivially small; the expensive part is the Lua run, not the index.

### 5. What can be reused, and one trap

| Reuse | Where |
|---|---|
| XOR, standalone and MFC-free | `X2KomFileViewer/FileCrypt.h` — a templated `FileDecrypt`/`BufferEncrypt` over a key vector, including only `windows.h`, `<fstream>`, `<memory>`, `<io.h>` |
| The keys | `KTDXLIB/KTDX.h:388`, and independently in `X2KomFileViewer/Config.lua:12-25` as decimals |
| Lua 5.1 + `lua_tinker` | `luaLib/luaLib_2010.vcxproj`; prebuilt `KTDXLIB/luaLib.lib`, rebuilt 2026-09-04 |
| zlib | prebuilt `KTDXLIB/zlib.lib`, header `KTDXLIB/zlib.h`; full source at `X2ChinaLauncher/Lib/zlib-1.2.5/` |
| SQLite | `Libs/ExternalLib/sqlite3/sqlite3.c` (3.53.4 amalgamation, no `.lib`) |
| Reference reader | `KTDXLIB/KGCMassFileManager.cpp:1355-1461` — read it for the format, don't compile it |
| Project template | `X2MassFileTool/X2MassFileTool_2010.vcxproj` — v100, Unicode, no engine linkage |

> **Trap: every standalone `.kom` reader in this tree is stale.** Both
> `X2MassFileTool/MassFile.cpp:43-70` and `X2KomFileViewer/MassFile.cpp:59`
> compute the version digits and then unconditionally read a **fixed-size binary
> directory** — they handle archive versions 1 and 2 only. Every shipped archive
> is `KOG GC TEAM MASSFILE V.0.3.` with an XML directory, so either copy will
> silently misparse all 145 of them rather than failing loudly. The only reader
> with a `case 3:` is the engine's
> ([KGCMassFileManager.cpp:1355](KTDXLIB/KGCMassFileManager.cpp#L1355)) — and
> that file `#include "StdAfx.h"`, which resolves to `KTDXLIB/StdAfx.h` and pulls
> in `afxwin.h` **and** `KTDX.h`, i.e. MFC plus the entire engine hub. It is also
> 3,546 lines of threading, buffer pooling and background-loading machinery a
> tool does not want.
>
> **So the tool gets a fresh ~250-line reader** written from the format spec
> verified above, with the engine's `case 3:` as the reference. This is the one
> place the plan deliberately writes new code instead of reusing shipped code,
> and the reason is that the reusable copies are wrong and the correct copy is
> unreachable.

The XML manifest is flat and machine-generated (`<File Name=".." Size=".."
CompressedSize=".." Checksum=".." FileTime=".." Algorithm=".." />`), so a small
attribute scanner reads it without taking a dependency on libxml at all. If a
real parser is ever wanted, `libxml/libxml_2010.vcxproj` builds under VS2010 and
is already `/MD`.

**The runtime library already agrees.** `/clr` requires `/MD`, and both
`luaLib_2010.vcxproj` and `libxml_2010.vcxproj` are `MultiThreadedDLL` /
`MultiThreadedDebugDLL` throughout — so their prebuilt libs
(`KTDXLIB/luaLib.lib`, `KTDXLIB/libxml_mt.lib`, both rebuilt 2026-09-04 by the
VS2010 toolchain) link into a `/clr` exe without the `/MT` vs `/MD` duplicate
symbol fight. The `_mt` in `libxml_mt.lib` is a misleading filename, not a `/MT`
build.

`KTDXLIB/zlib.lib` is the one stale dependency — dated 2014, so built by an
older toolchain. If it does not link under VC10, full zlib 1.2.5 source is in
the tree at `X2ChinaLauncher/Lib/zlib-1.2.5/`, and only a handful of files are
needed for `uncompress()` (`adler32.c`, `crc32.c`, `inflate.c`, `inftrees.c`,
`inffast.c`, `zutil.c`, `uncompr.c`). Compiling those into the tool removes the
dependency outright.

The `sqlite3.c` compile settings from `X2Lib_2010.vcxproj:3133` must be copied
verbatim — `CompileAs=CompileAsC`, `PrecompiledHeader=NotUsing`,
`WarningLevel=TurnOffAllWarnings`, and
`INFINITY=HUGE_VAL;SQLITE_THREADSAFE=1;SQLITE_OMIT_LOAD_EXTENSION;SQLITE_DEFAULT_FOREIGN_KEYS=1;_CRT_SECURE_NO_WARNINGS`.
`INFINITY=HUGE_VAL` is not optional: VC10's `math.h` has no `INFINITY` and
SQLite 3.53 will not compile without it.

### 6. The `/clr` trap, and why the tool is two projects

**This is the finding that shapes the build.** `Microsoft.CppBuild.targets:733-738`
(at `C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\`) reads:

```xml
<ClCompile Condition="'@(ClCompile)' != '' and '$(CLRSupport)' != 'false' and '$(CLRSupport)' != ''">
  <AdditionalUsingDirectories>$(TargetFrameworkDirectory);%(ClCompile.AdditionalUsingDirectories)</AdditionalUsingDirectories>
  <AdditionalOptions Condition="'$(TargetFrameworkVersion)' == 'v4.0'">/clr:nostdlib %(ClCompile.AdditionalOptions)</AdditionalOptions>
</ClCompile>
```

With `CLRSupport=true` and `TargetFrameworkVersion=v4.0`, MSBuild stamps
`/clr:nostdlib` onto **every** `ClCompile` item in the project, at the item-group
level, with **no `CompileAsManaged` guard**. That omission is not stylistic: the
very next `ClCompile` block, at `:744`, *does* guard on
`'%(ClCompile.CompileAsManaged)' != 'false'`.

And `CompileAsManaged=false` cannot cancel it — that enum value emits the empty
string (`Microsoft.Cpp\v4.0\1033\cl.xml:32`), so it only suppresses the `/clr`
in its own slot, never a `/clr:nostdlib` already sitting in `AdditionalOptions`.

Two consequences for a single-project build:

- **`sqlite3.c` fails loudly**: `cl : Command line error D8016: '/clr' and '/TC'
  command-line options are incompatible`. The X2Lib override block does not save
  it.
- **The native C++ fails silently.** The kom reader, the XOR loop and the Lua
  bridge compile to IL with no warning at all — which is the far worse outcome,
  because Lua signals errors by C++ `throw` (see below) and unwinding those
  through managed frames is exactly what must not happen.

**The split is the fix**: with `CLRSupport` absent on the native static lib, the
condition at `:733` excludes it entirely and the injection never fires. A
per-file `/clr-` in `AdditionalOptions` would also work — `/clr:nostdlib` is
*prepended*, so a later `/clr-` wins — but that is a hack applied to a 9.5 MB C
file and every native `.cpp`, resting on last-wins ordering holding for the
D8016 check. Take the split.

**Phase 0 proves the containment** by putting this at the top of every native
translation unit, `sqlite3.c` included:

```c
#ifdef _MANAGED
#error "This TU compiled managed. /clr leaked in - see CppBuild.targets:737"
#endif
```

A clean build then *is* the evidence, not an assumption.

### 7. Link-time facts, all checked against the actual libs

| Fact | Consequence |
|---|---|
| `luaLib.lib` exports **C++-mangled** symbols — `?lua_pcall@@YAHPAUlua_State@@HHH@Z`, `?luaL_loadbuffer@@YAHPAUlua_State@@PBDI1@Z`; there is no `_lua_pcall` | Include `lua.h` / `lauxlib.h` / `lualib.h` **directly**. Never `lua.hpp`, never your own `extern "C"` — either gives `LNK2019` on every Lua call. `luaLib_2010.vcxproj` sets `CompileAs=CompileAsCpp` for the whole project, which is why. |
| Same cause: `luaconf.h:619-622` takes the C++ branch, so `LUAI_THROW` is `throw(c)` and `LUAI_TRY` is `try/catch(...)` | Lua raises errors as **C++ exceptions**. Keep every frame between `lua_pcall` and the `AddItemTemplet` callback native. The split makes this structural rather than a matter of discipline. |
| `luaLib.lib` is release-STL (`/DEFAULTLIB:msvcprt`) | **Do not create a `_DEBUG` configuration.** VS2010's `_ITERATOR_DEBUG_LEVEL` `detect_mismatch` pragma gives `LNK2038: value '0' doesn't match value '2'`. Ship two configs differing only in `Optimization`, both `NDEBUG`, both `MultiThreadedDLL`. Comment the vcxproj so nobody "fixes" it. |
| `KTDXLIB/zlibstat.lib` carries `/DEFAULTLIB:LIBC` (single-threaded static CRT) | Use **`zlib.lib`**, never `zlibstat.lib` — the latter gives an `LNK2005` storm against `/MD`. |
| `zlib.lib`'s only CRT references are `__imp__` DLL imports; no `_iob`, no `__security_cookie`, no `_except_handler3` | The 2014 build should link clean under VC10 despite its age. Phase 0 confirms. |
| `libxml`'s `XMLPUBFUN` expands to nothing, but `XMLPUBVAR` becomes `__declspec(dllimport)` — and `xmlFree`/`xmlMalloc` are *variables* | Define **`LIBXML_STATIC`** in the tool, or get `unresolved external __imp__xmlFree`. |
| A WinForms `int main(array<String^>^)` under `SubSystem=Windows` | Set **`<EntryPointSymbol>main</EntryPointSymbol>`** or get `LNK2019: unresolved external symbol _WinMain@16`. |
| `VC
edist\` has no `msvcm100.dll`, and `VC\lib\msvcmrt.lib` is present | VS2010 statically links the managed-C++ support runtime, so deployment is just `msvcr100.dll` + `msvcp100.dll` + .NET 4.0. |

Also set `WholeProgramOptimization=false` (`/GL` is incompatible with `/clr`),
`TreatWChar_tAsBuiltInType=true`, `PrecompiledHeader=NotUsing` project-wide (a
`/clr` PCH cannot be shared with native TUs), `LargeAddressAware=true` (see the
memory figures below), and be explicit about `<PlatformToolset>v100</PlatformToolset>`
rather than relying on the default `X2MassFileTool_2010.vcxproj` omits.

Use `..\` relative include paths rather than `$(SolutionDir)` — it avoids the
trailing-slash footgun `CLAUDE.md` documents for the client build, and lets the
project build with no extra properties.

---

## Decisions taken

| Decision | Choice | Why |
|---|---|---|
| UI | **C++/CLI WinForms**, `/clr`, .NET 4.0 | The picker needs a virtualized list over ~43k rows; WinForms `ListView` VirtualMode gives that, plus tabs, image lists and a search box, for a fraction of the owner-draw code MFC needs. Managed form code calls the native readers directly — no P/Invoke. |
| Project shape | **Two projects**: a native `X2CashShopCore` static lib and a thin `/clr` WinForms exe | Not a hedge — it sidesteps a real defect in the VS2010 targets. See *The `/clr` trap* below. |
| Which DB | **The live `els_db.sql` in the game dir** | It is the only catalog the client reads. |
| Safety | Backup into the existing `db_backup/` before the first write of a session; `PRAGMA wal_checkpoint(TRUNCATE)` on close; refuse to open if the client holds the file | `els_db.sql` is the only copy of the character, and it migrates forward only. |
| Extra scope | **Wallet** (`settings.cash_start`) and **bulk price/qty editing** with CSV round-trip | Every one of the 2,360 rows is priced at 1; per-row dialogs are not a realistic way to fix that. |
| Out of scope | `cash_order` (deposit) and `wish_list` editing | Not asked for; both are per-character state the game manages. |
| Solution | A **new** `X2CashShopTool.sln`, not added to `X2Project_2010.sln` | `X2Project_2010.sln`'s `US_SERVICE` sweeps in ~15 dead projects and does not even build the client exe; a tool has no business in it. |
| Feature flag | **None** | `CLAUDE.md`'s `SERV_IRUHADEV_` rule governs edits to `X2/`, `X2Lib/`, `KTDXLIB/`. This adds new files under a new directory and edits none of them. |
| Human labels *(phase 6)* | **`General.ess` through the client's own `enum` → `STR_ID` switches**, the studio's control names where there is no string, and a curated table for the rest — each answer carrying its **origin**, and the number never replaced | An enum name is the script's name for a thing, not the thing's name. There is a real localized string table in the game directory and the client's own switches say which id goes with which enum value, so most labels can be the game's own words rather than the tool's. The ones that cannot are marked as the tool's, because a label that looks sourced and is not is the failure the live-DB section of this plan exists to prevent. See *Phase 6*. |

**Nothing in `X2Lib/`, `KTDXLIB/`, `X2/` or `KncWX2Server/Common/` is modified**,
so neither the client nor the servers need rebuilding for any of this.

---

## Design

```
X2CashShopTool/
  X2CashShopTool_2010.sln              two projects, two configs, nothing else

  Core/X2CashShopCore_2010.vcxproj     StaticLibrary. CLRSupport ABSENT (this is
                                       what contains the /clr:nostdlib injection).
                                       /MD, Unicode, UseOfMfc=false, v100.
    KomArchive.{h,cpp}                 V.0.3 only, ~200 lines, fresh
    LuaXor.{h,cpp}                     ~20 lines, in-place DWORD XOR
    ItemIndex.{h,cpp}                  Lua state + stubs + extraction
    DdsDecode.{h,cpp}                  DXT1/3/5 + uncompressed -> BGRA32 (phase 2:
                                       the plan said DXT1/DXT5 only; the shipped set
                                       has DXT3 and two uncompressed shapes as well)
    IconStore.{h,cpp}                  the .dds locator over all 145 archives, the
                                       client's Noimage fallback, and the bounded LRU
    CashDb.{h,cpp}                     sqlite3 over els_db.sql
    IndexCache.{h,cpp}                 the SQLite item index, next to the tool exe
    Labels.{h,cpp}                     phase 6: General.ess + the client's own
                                       enum -> STR_ID switches + the curated
                                       table, every answer carrying its origin.
                                       Read every run, never cached - see
                                       phase 6's label rule
    ../../Libs/ExternalLib/sqlite3/sqlite3.c   (X2Lib_2010.vcxproj:3133-3138 verbatim)

  Ui/X2CashShopTool_2010.vcxproj       Application. CLRSupport=true,
                                       TargetFrameworkVersion=v4.0, /MD, v100.
    Main.cpp                           [STAThread] int main(array<String^>^)
    UiBridge.h                         NativeBridge + IconProvider (phase 5: moved
                                       out of MainForm.h, which includes the picker,
                                       which needs both - see phase 5's corrections)
    MainForm.h                         tabs, product grid, wallet, bulk actions,
                                       and phase 6's details pane + the
                                       View -> Technical details toggle
    ItemPickerForm.h                   the picker + search + filters (phase 5: an
                                       owner-drawn Panel, not a VirtualMode
                                       ListView - see phase 5's decisions)
    IconWallForm.h                     phase 2's icon wall, reached with --wall
    EditProductForm.h                  category / price / quantity / event, in
                                       either edit or insert mode
```

**Every frame between `lua_pcall` and the `AddItemTemplet` callback lives in
`Core`, and that is the point of the split** — not just build hygiene. Lua
raises errors as C++ exceptions here, and unwinding those through managed frames
is a documented hazard.

### The item index is a cache, and it is the design — not an optimization

Loading `Item.lua` costs seconds and hundreds of megabytes (below). Paying that
every time the tool opens to change one price is not acceptable, so extraction
happens once and lands in a **SQLite file next to the tool exe** — not in the
game directory, where a stray `.db` beside `els_db.sql` is the sort of thing
someone deletes at 2am.

SQLite rather than a binary blob because `sqlite3` is already linked and the
picker wants `WHERE name LIKE ?` over 43k rows anyway.

```sql
CREATE TABLE index_meta( key TEXT PRIMARY KEY, value TEXT );
  -- kom_path, kom_size, kom_mtime, extractor_version

CREATE TABLE item( item_id INTEGER PRIMARY KEY, name TEXT, shop_image TEXT,
                   item_type INTEGER, item_grade INTEGER,
                   is_fashion INTEGER, equip_position INTEGER );
CREATE INDEX ix_item_name ON item( name );

CREATE TABLE cash_category( tab_idx INTEGER, real_id INTEGER, sub_ordinal INTEGER,
                            cssc_enum INTEGER, billing_category_no INTEGER );

-- phase 5: Enum.lua's own value -> name maps, so the picker's filters can say
-- IT_ARMOR rather than 2; and the items the shop filters out AFTER the catalog
-- packet has carried them, which is the second silent-drop rule and is not in
-- offline_server.log at all. Empty vs. unknown is index_meta.package_data_ran.
CREATE TABLE enum_name( table_name TEXT, value INTEGER, name TEXT,
                        PRIMARY KEY( table_name, value ) );
CREATE TABLE hidden_package_item( item_id INTEGER PRIMARY KEY );

-- icons are a LOCATOR table; never the bytes, never decoded bitmaps
-- (phase 2: these two are versioned and stamped SEPARATELY from the item
--  half, via icon_locator_version + icon_kom, because the catalog depends
--  on data036.kom alone and the locator on all 145 archives)
CREATE TABLE icon( name TEXT PRIMARY KEY, kom TEXT, offset INTEGER,
                   comp_size INTEGER, real_size INTEGER );
CREATE TABLE icon_kom( kom TEXT PRIMARY KEY, size INTEGER, mtime INTEGER );
```

Invalidate when **any** of `kom_path`, `kom_size`, `kom_mtime` or
`extractor_version` differs. `extractor_version` is a hardcoded integer, bumped
whenever the *set of captured fields* changes — that is the one people forget,
and forgetting it yields an index that looks valid and is quietly missing a
column. Use mtime+size rather than a content hash: hashing hundreds of megabytes
costs seconds and defeats the purpose.

Build the icon locator from **manifests only** — for each of the 145 archives
read the 60-byte header, the 12 bytes and the XML, then stop. No payload reads.
Decode one DDS on demand as the grid scrolls, and announce a rebuild in the
status bar (`item index rebuilt from data036.kom: 48,754 items, 0.46 s` — the
real phase 1 numbers) for the same reason the offline server logs its decisions.

### The flow, mapped onto the data

1. **Tabs** are `CASH_SHOP_REAL_CATEGORY_ID` from `CashShopCategory.lua`: one
   top-level tab per `CSC_*`, a sub-tab selector per `CSSC_*` pair. Selecting a
   sub-tab filters `cash_product` on `category = pair[2]`. An "All" pseudo-tab
   shows every row, including any whose `category` matches no pair — those are
   invisible in the game and the tool should say so.
2. **The grid** is one tile per product: the 64×64 icon decoded from
   `m_ShopImage`, the item name from `ItemTrans.lua`, the price, the quantity,
   an event badge, and Edit / Delete. Tiles come from `cash_product` joined in
   memory against the item catalog.
3. **Insert** opens the picker over the whole ~43k-item catalog — only visible
   rows are ever painted, so it neither lags nor grows without bound. The search
   box filters on name and on item ID. *(Phase 5: an owner-drawn `Panel` rather
   than the VirtualMode `ListView` written here, because a virtual `ListView`
   cannot draw an icon without a pre-populated `ImageList` — see phase 5's
   decisions. The bound on what is materialised is unchanged, and measured.)*
4. **The insert form** takes category (pre-filled from the current tab), price,
   quantity and the event flag, allocates `product_no = max+1`, validates the
   ≤127 limits, and inserts.

### Things the implementation must not get wrong

- **`MASSFILE_MEMBERFILEINFO_POINTER` is a struct by value** with an
  `operator const MASSFILE_MEMBERFILEINFO*() const { return this; }`, so
  `Info == NULL` is never true. Test `Info->pRealData == NULL || Info->size <= 0`
  — the trap is documented at
  [X2OfflineStatTable.cpp:104-111](X2Lib/Offline/X2OfflineStatTable.cpp#L104).
- **Archive lookups are uppercased.** `LoadDataFile` calls `MakeUpperCase` before
  the map lookup ([KGCMassFileManager.cpp:653](KTDXLIB/KGCMassFileManager.cpp#L653)).
- **First archive to claim a name wins**, so build the index in `data001` →
  `data145` order.
- **A stub must exist for every method the scripts call**, or the chunk errors on
  its first line — the `ReserveMemory_LUA` lesson from
  [X2OfflineStatTable.cpp:176-182](X2Lib/Offline/X2OfflineStatTable.cpp#L176).
- **The client caches the catalog for the life of the process**
  (`EnsureLoaded`), so an edit is visible only after restarting the game. Say so
  in the UI rather than letting it look like a failed write.
- **Back up the whole WAL set, not just the main file.** `db_backup/` already
  exists in the game directory and already uses this convention —
  `els_db.sql.bak-pre-<label>` alongside `els_db.sql-wal.bak-pre-<label>` and
  `-shm`. Copying `els_db.sql` alone yields a *stale* snapshot: it once read
  schema v6 while the live DB was at v7. Follow the existing naming, with the
  label `cashtool-<timestamp>`.
- **Valid range for `quantity` and `category` is 1..127, enforced not clamped.**
  The upper bound is the signed-char wire field; the lower is
  `X2OfflineCashShop.cpp:94`, which silently rewrites a non-positive quantity to
  1. A tool that clamps rather than refuses just moves the surprise.
- **Duplicate `item_id` across different `product_no` is deliberate** — eighteen
  items are sold as two products each, and the catalog index is keyed on product
  number precisely so they survive
  ([X2OfflineCashShop.cpp:100-103](X2Lib/Offline/X2OfflineCashShop.cpp#L100)).
  Do not dedupe and do not "helpfully" warn about it.
- **Refuse to open a save whose `PRAGMA user_version` is not 11.** `X2OfflineDB`
  migrates forward only, and a tool writing into a schema it does not know is how
  a save gets corrupted. Fail loud.
- **Do not reuse the studio's `XORDecrypt`.**
  [KTDXCommonFunc.h:650-713](KTDXLIB/KTDXCommonFunc.h#L650) accumulates the
  plaintext into a `std::string` four bytes at a time, then copies it out — for
  34 MB that is ~8.5 million `append` calls and a transient peak around 140 MB.
  `X2KomFileViewer/FileCrypt.h:108-158` has the same shape. Cite them as the
  spec and write the ~15-line in-place version: `p[i] ^= k[i % 3]` over `n/4`
  dwords plus the 1–3 byte tail. It is symmetric, so the same function encrypts.
- **Do not touch the studio's `.lua`.** The tool is read-only against every
  `.kom`; it never repacks anything.

---

## Running a phase in a fresh conversation

This file lives at the trunk root, next to `AI_PARTY_PLAN.md` and
`OFFLINE_MODE_PLAN.md`, and it is the only thing a fresh conversation needs to
read to pick up a phase.

`CLAUDE.md` and the memory index load automatically, so a phase prompt only has
to name the phase and forbid re-exploration:

```
Read CASH_SHOP_TOOL_PLAN.md and do Phase 1.

The plan's verified-facts section has the file:line refs and the archive format
already - trust it and verify by reading those specific lines, don't re-explore
the codebase from scratch. Build per the plan's Verification section when you're
done, then stop so I can run it.
```

Three things to hold to across all phases:

1. **Phase 0 is a gate.** Do not start phase 1 before phase 0 has actually built
   and run. Whether the `/clr` exe links cleanly against `luaLib.lib`,
   `libxml_mt.lib` and the 2014-vintage `zlib.lib`, and whether the two-project
   split really does contain the `/clr:nostdlib` injection, is not knowable by
   reading. The split is already the design (section 6) — what phase 0 settles is
   whether it holds in practice, and if it does not, the fallback is a fully
   native tool with an MFC or Win32 front end.
2. **Write the outcome back into the file at the end of every phase**, in the
   house shape: an `### Exit test — PASSED/PARTIAL (date)` block, a *Corrections
   to this plan, found by doing it* section, and *Decisions made while
   implementing phase N*. The next conversation reads that and nothing else of
   the previous one.
3. **Only running it closes a phase.** There is no test suite. A phase ends when
   its exit test has been run against the real game directory and the result
   recorded — not when it compiles.
4. **Deploy to the game directory after every build, every phase — not only
   when that phase's exit test happens to touch archives or `els_db.sql`.**
   Build, then `cp X2CashShopTool/Ui/Release/X2CashShopTool.exe "$DATA/"`,
   then run it from `$DATA`, confirming the copy landed by re-listing the
   directory rather than trusting the `cp` exit code — the same discipline
   `CLAUDE.md`'s "Deploying the offline client" section prescribes for
   `X2_offline.exe`. Phase 0's own probe needs no game data at all and still
   gets deployed and re-run from there, because the tool's whole reason to
   exist is running out of that directory (see *Outcome*), and every phase
   from here on depends on it being there.

## Phases

Each phase is one conversation, ends with an explicit exit test, and writes its
outcome back into `CASH_SHOP_TOOL_PLAN.md` — the same convention as
`AI_PARTY_PLAN.md` and `OFFLINE_MODE_PLAN.md`. Ordered so the two genuine
unknowns (does `/clr` link against this tree, and does 34 MB of bytecode load in
a usable time) are settled before any UI exists.

### Phase 0 — Build-system probe: prove the link, read nothing real

Both projects, no UI, no game data. `X2CashShopCore` is `sqlite3.c` plus a
20-line `Probe.cpp`; the `/clr` exe links it plus `luaLib.lib`, `libxml_mt.lib`
and `zlib.lib`, and prints `sqlite3_libversion()`, `LUA_RELEASE` after
`luaL_newstate()`, `zlibVersion()`, `LIBXML_DOTTED_VERSION`, and a `String^`
round-trip through `msclr::interop::marshal_as` to prove the CLR is live.

Every native translation unit, `sqlite3.c` included, carries the `#ifdef _MANAGED`
/ `#error` canary from section 6.

**Exit test**: the build completes with **zero LNK2038, LNK2005 or LNK4098**, and
the exe prints all four versions. A clean build is itself the proof that the
split contained the `/clr:nostdlib` injection — that is what the canary is for.

**This is a gate.** Record in this file whether the 2014-vintage `zlib.lib`
linked, and if `/clr` cannot be made to work at all, record that and the chosen
fallback before phase 1 starts.

#### Exit test — PASSED (2026-09-06)

Built both configs of `X2CashShopTool/Ui/X2CashShopTool_2010.vcxproj`
(`Release|Win32` and `Debug|Win32`, MSBuild pulling `Core`'s
`X2CashShopCore_2010.vcxproj` in as a project reference). Both produced
`X2CashShopTool.exe` with **0 Warning(s), 0 Error(s)** — no LNK2038, LNK2005 or
LNK4098 anywhere in the verbose (`/v:normal`) log. Running the exe (both
configs) prints:

```
sqlite3 : 3.53.4
lua     : Lua 5.1.4 (state created: true)
zlib    : 1.2.3
libxml  : 2.7.2
CLR round-trip : X2CashShopTool phase 0 probe
```

exit code 0 (the round-trip string compares equal after the managed ->
native `std::string` -> managed hop). **The 2014-vintage `zlib.lib` linked
clean** with no `/MT` vs `/MD` fight, exactly as the plan's link-time table
predicted. The two-project split does contain the `/clr:nostdlib` injection:
`Probe.cpp` and `sqlite3.c` (native, in `Core`, `CLRSupport` absent) compiled
with plain `/TC`/`/TP`, and only `Main.cpp` (in `Ui`) got `/clr:nostdlib` —
confirmed both by the canary staying silent in `Core` and by reading the
actual `cl.exe`/`link.exe` command lines out of `/v:normal`.

Deployed per the standing rule (see point 4 above): copied to
`F:\...\237311\22191271\data\X2CashShopTool.exe`, landing confirmed by
listing the directory (not just the `cp` exit code), and re-run with that
directory as the working directory — same output, exit code 0. Phase 0 needed
none of that to pass its own exit test, and got it anyway.

**One thing worth recording precisely**: `link.exe`'s own printed command
line does *not* list `X2CashShopCore.lib` among its visible arguments — only
`Main.obj` and the system libs appear there. That looks alarming on a first
read, and was chased down: MSBuild's `<ProjectReference>` handling appends
the referenced static lib as a tracked link input outside the argument list
this build shows, and `Release\link.command.1.tlog` (UTF-16, undocumented by
the visible log) confirms `X2CashShopCore.lib` *is* actually there, ahead of
`Main.obj`. Trust the tlog over the printed command line if this comes up
again; the exe's own output (correct native version strings) is the more
direct proof that linking against `Core` actually worked.

#### Corrections to this plan, found by doing it

- **`Probe.h`'s canary placement, not its existence, was the trap.** The
  first draft put `#ifdef _MANAGED / #error` in `Probe.h` itself, reasoning
  it should cover "every native TU in `Core`". But `Probe.h` is legitimately
  included from `Main.cpp` too — that shared declaration is the entire point
  of the split — and `Main.cpp` is *supposed* to compile managed. Putting the
  canary in the header makes it fire on the one file it must never fire on.
  The canary belongs only in the `.cpp` files that actually touch
  Lua/sqlite3/zlib/libxml (`Probe.cpp`, and `sqlite3.c` itself, which now
  carries its own copy prepended by hand — see below); a shared header
  declaring the call boundary carries no canary at all.
- **`msclr::interop::marshal_as<std::string, String^>` needs a second header.**
  `<msclr/marshal.h>` alone gives `C4996` ("this conversion is not supported")
  for the `std::string` specialization; `<msclr/marshal_cppstd.h>` is required
  in addition. Worth knowing before phase 5/6, which will want the same
  conversion for search-box text and CSV fields.
- **`Libs/ExternalLib/sqlite3/sqlite3.c` is untracked by git** (matched by a
  `.gitignore` rule), not merely large. The canary was still added to it
  (prepended by hand, ASCII, no encoding risk — confirmed with `file` before
  and after) because it is the same physical file `X2Lib_2010.vcxproj`
  compiles, and the guard is inert for that build (`_MANAGED` is never
  defined there). Anyone diffing this repo against a pristine checkout of the
  external lib will see the 6-line addition at the top of that one file.
- **VS2010's actual install path here is `D:\Program Files\VS\Microsoft
  Visual Studio 10.0\`, not the default `C:\Program Files (x86)\...`.** Not a
  plan defect — MSBuild resolves it from
  `HKLM\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\SxS\VC7` regardless of
  where it sits on disk — but worth recording since a plain filesystem search
  for `cl.exe` under `C:\Program Files (x86)` finds nothing on this machine.

#### Decisions made while implementing phase 0

- **Two configs, named `Release` and `Debug`, both `NDEBUG` /
  `MultiThreadedDLL`**, differing only in `Optimization`
  (`MaxSpeed`/`Disabled`) — exactly the plan's prescription. Both vcxproj
  files carry a comment block warning against ever giving either config the
  normal debug CRT. Both configs were built and run in this phase precisely
  to prove the `_ITERATOR_DEBUG_LEVEL` trap really is avoided, not just
  documented.
- **`Ui`'s subsystem is `Console`, not `Windows`, for phase 0.** There is no
  WinForms UI yet, so `int main(array<String^>^)` under the default
  `mainCRTStartup` entry point needs no `EntryPointSymbol` override. That
  trap (section 7: `SubSystem=Windows` needs `EntryPointSymbol=main`) is
  real but not yet exercised — it becomes live in phase 4 when the actual
  WinForms `MainForm` replaces `Main.cpp`'s console probe, and both vcxproj
  files carry a comment flagging that in advance.
- **`TargetName` set explicitly on both projects** (`X2CashShopCore`,
  `X2CashShopTool`) rather than left to the vcxproj-file-name default
  (`X2CashShopCore_2010`, `X2CashShopTool_2010`) — the latter produced a
  `MSB8012` warning and an exe name that didn't match the plan's
  Verification section.
- No fallback to a native-only (MFC/Win32) front end was needed — `/clr`
  works cleanly against this tree, so phase 1 onward proceeds on the
  `Core`/`Ui` split as designed.

### Phase 1 — The archive index and the item catalog

`KomIndex` over all 145 archives; `LuaHost` running `Enum.lua` → `Item.lua` →
`ItemTrans.lua` → `CashShopCategory.lua` with stubs for every method they call;
`ItemCatalog` holding id, name, icon name, type, grade, fashion flag, equip slot;
`IndexCache` so the second launch is instant.

**Exit test**: dumps a count near 43,000 items, prints ten known ids with their
names and `m_ShopImage`, prints the parsed tab table with its billing category
numbers, and reports first-run vs cached load times.

#### Exit test — PASSED (2026-09-06)

Built both configs (`Release|Win32` and `Debug|Win32`), **0 Warning(s), 0
Error(s)** on a full rebuild of each, no `LNK2038` / `LNK2005` / `LNK4098`.
Deployed to `F:\...\237311\22191271\data\X2CashShopTool.exe` — landing
confirmed by re-listing the directory — and run with that directory as the
working directory. Exit code 0.

```
archives : mounted 145 of 145 (88723 names, 0 shadowed by an earlier archive)
index    : C:\Users\Iruha\AppData\Local\X2CashShopTool\ItemIndex.db

  Enum.lua                   473104 bytes  luac  load 0.00s  run 0.00s
    enum tables : ITEM_TYPE=table  ITEM_GRADE=table  USE_CONDITION=table
                  EQIP_POSITION=table  CASH_SHOP_CATEGORY=table  CASH_SHOP_SUB_CATEGORY=table
    enum values : UC_NONE=0  UC_ANYONE=1  IG_NORMAL=4  EP_QUICK_SLOT=1
  Item.lua                 34211379 bytes  luac  load 0.16s  run 0.20s
  ItemTrans.lua             6601013 bytes  luac  load 0.04s  run 0.03s
  CashShopCategory.lua         3193 bytes  luac  load 0.00s  run 0.00s

items    : 48754 captured, 0 duplicate id(s) dropped
rejected : 0 row(s) ... 48609 with a non-empty m_ShopImage
           0 with m_ItemType 0
trans    : 48754 name(s) overlaid from ItemTrans.lua, 0 orphan id(s)
stubbed  : none - the scripts called nothing the stand-ins do not implement

extract  : 0.46 s in total (0.01 Enum, 0.35 Item, 0.07 ItemTrans, 0.00 CashShopCategory)
cache    : 48754 item(s) + 31 category row(s) written in 176 ms
cached load : 48754 item(s), 31 category row(s), 79 ms
```

All four exit-test items are covered: the count (48,754 — see the corrections
below, it is **not** near 43,000 and the plan's estimate was low), ten sampled
items with names and `m_ShopImage`, the full tab table with its billing
category numbers, and both timings. **First run 0.46 s + 176 ms to write the
cache; every run after that 79 ms**, and `--rebuild` forces the slow path so
both numbers are reachable on demand.

The parsed tab table, which phase 4 builds its tabs from — 7 tabs, 31
sub-category pairs. The right-hand column is what `cash_product.category`
holds:

| tab | REAL_ID | billing category numbers |
|---|---|---|
| 1 | 10 | 11, 12, 13, 14, 15, 16, 17 |
| 2 | 20 | 21, 22, 23, 24, 25, 26, 27 |
| 3 | 30 | 31, 32, 33, 34 |
| 4 | 40 | 41, 42, 43 |
| 5 | 60 | 61, 62, 63 |
| 6 | 50 | 51, 52, 53, 54, 55, 56 |
| 7 | 1112 | 1113 |

Note tab 5 carries `REAL_ID` 60 and tab 6 carries 50 — the tab order and the
`REAL_ID` order genuinely disagree, and that is the studio's data, not a
parsing error. Tab 7's 1112/1113 is the odd one out in every respect.

**Two numbers the plan asked phase 1 to measure, both far better than
predicted**: extraction is **0.46 s**, not the estimated 2–6 s plus 1–3 s, and
peak working set is **114.7 MB**, not the estimated 200–350 MB. So
`LargeAddressAware` is not load-bearing after all — it stays set, because it
costs nothing and phase 2 adds decoded bitmaps.

#### Corrections to this plan, found by doing it

- **`Enum.lua` publishes TABLES, not flat globals — section 3 is wrong about
  this.** The plan says "`Item.lua`'s `m_ItemType = IT_WEAPON` are plain global
  reads" and tells phase 1 to "assert a couple of known enum globals are
  non-nil straight after `Enum.lua`". Probing both shapes on the real file:
  `ITEM_TYPE`, `ITEM_GRADE`, `USE_CONDITION`, `EQIP_POSITION`,
  `CASH_SHOP_CATEGORY` and `CASH_SHOP_SUB_CATEGORY` all resolve as **tables**,
  while bare `IT_WEAPON`, `IG_NORMAL` and `EP_QUICK_SLOT` all come back
  **nil**. The scripts write `m_ItemType = ITEM_TYPE.IT_WEAPON`. An assertion
  written the way the plan describes would have failed on a perfectly healthy
  load. The check now probes the tables, and the ordering requirement the
  assertion exists to protect is unchanged and still real.
- **This turned out to be a gift**: because the enum values are reachable as
  table members, the tool *reads* `USE_CONDITION.UC_NONE`, `ITEM_GRADE.IG_NORMAL`
  and `EQIP_POSITION.EP_QUICK_SLOT` out of `Enum.lua` at runtime instead of
  transcribing them. `IG_NORMAL` came back 4, which independently matches the
  C++ enum in `KncWX2Server/Common/X2Data/XSLItem.h:34-42` — two genuinely
  independent sources agreeing, which is the standard `CLAUDE.md` sets.
- **There are 48,754 items, not ~43,000.** The plan's "~43k" appears in the
  Context, in section 3 and in this phase's exit test; it was an estimate and
  it was low. 34,211,379 bytes / 48,754 items = 702 bytes per item, which is
  the right order for these table constructors. Corroboration that the number
  is real rather than double-counting: zero duplicate ids, and `ItemTrans.lua`
  overlaid **exactly** 48,754 names with **zero** orphans — a second,
  separately exported file whose id set matches the first one one-for-one.
- **The client rejects items on more than a missing id, and its defaults are
  not zero.** `CX2ItemManager::KProxy::AddItemTemplet_LUA`
  ([X2ItemManager_Preprocessing.cpp:13-35](X2Lib/X2ItemManager_Preprocessing.cpp#L13))
  refuses an item whose `m_UseCondition` is `UC_NONE`, and `LUA_GET_VALUE_RETURN`
  refuses one missing `m_ItemID`, `m_Name` or `m_ItemType`. Separately,
  `m_ItemGrade` defaults to `IG_NORMAL` (4) and `m_EqipPosition` to
  `EP_QUICK_SLOT` (1), **not** to 0 ([X2ItemManager.cpp:430](X2Lib/X2ItemManager.cpp#L430),
  [:486](X2Lib/X2ItemManager.cpp#L486)). The first draft of the extractor got
  the defaults wrong and every grade-less item read back as `IG_NONE`. The
  catalog now mirrors the client's rule exactly — which matters because
  phase 3 joins this catalog against `cash_product` to reproduce the client's
  own dropped-row count, and a catalog holding items the client refuses would
  make that join disagree with the game in the flattering direction.
  **On this data all four rejection counters are 0**, so the accepted set is
  every `AddItemTemplet` call — but that is now a measured fact printed on
  every rebuild rather than an assumption.
- **The manifest's `Size` attribute is not authoritative.** 121 of the 88,723
  members state `CompressedSize == Size` while still holding a real zlib
  stream (they all begin `78 9C`), so their stated `Size` is smaller than what
  they actually inflate to. A reader that sized its output buffer from `Size`
  and called `uncompress()` would get `Z_BUF_ERROR` on those 121 and, if it
  treated that as "absent", drop them silently. `KomArchive` inflates with a
  **growing** buffer and uses `Size` only as the initial hint. None of the four
  scripts phase 1 needs is in that set, but phase 2's icons are drawn from the
  same 88,723.
- **Do not copy the engine's `case 3:` offset expression literally.** Section 5
  points at [KGCMassFileManager.cpp:1355](KTDXLIB/KGCMassFileManager.cpp#L1355)
  as the reference, and it is the right reference for the *structure* — three
  DWORDs, then the XML manifest, then payloads at a running sum of
  `CompressedSize`. But its initial offset,
  `72 + headersize - (sizeof(MASSFILE_HEADER) + sizeof(MASSFILE_MEMBERFILEHEADER) * iTotalFileNo)`
  ([:1395](KTDXLIB/KGCMassFileManager.cpp#L1395)), does not reproduce the real
  layout: `data036.kom` states `iTotalFileNo` 187, which is genuinely its
  member count, giving **9706** where the first payload actually begins at
  `72 + headersize` = **23230**. Proven, not inferred — the bytes at 23230
  inflate and XOR-decrypt to `1B 4C 75 61 51`, the Lua 5.1 bytecode header of
  `AccountQuest.lua`, the manifest's first entry. The plan's own stated rule
  ("cumulative `CompressedSize`, starting at `72 + headersize`") is the correct
  one and is what got implemented.
- **Lua strings in these scripts are UTF-8**, so nothing is converted anywhere.
  `KLuaManager`'s `wstring` getters decode with `CP_UTF8`
  ([luaLib/KLuaManager.h:803](luaLib/KLuaManager.h#L803)), not `CP_ACP`. Item
  names therefore go from the Lua state into SQLite `TEXT` and out to the
  console unchanged; the console just needs
  `Console::OutputEncoding = Encoding::UTF8`. Worth knowing for phase 7's CSV.
- **The cache location the plan gives contradicts the plan's own standing
  rule.** "A SQLite file next to the tool exe — *not* in the game directory,
  where a stray `.db` beside `els_db.sql` is the sort of thing someone deletes
  at 2am" and "deploy to the game directory after every build, every phase"
  are the same directory. The first run duly wrote `X2CashShopIndex.db` next to
  `els_db.sql`. The stated *reason* is the load-bearing half, so the cache now
  lives at **`%LOCALAPPDATA%\X2CashShopTool\ItemIndex.db`**, and the tool prints
  the full path on every run so it is never hidden. The stray file the first
  run created was deleted; `els_db.sql` and its WAL set were never opened.

#### Decisions made while implementing phase 1

- **`lua_tinker` is not used, as the plan prescribed**, and the stand-in shape
  in section 3 worked exactly as written: a plain table of C closures with a
  catch-all `__index` returning one shared no-op. The catch-all recorded
  **zero** unknown method names across all four scripts, so the shipped data
  calls nothing but `AddItemTemplet` and `AddItemTempletTrans`. It stays in
  anyway — it costs one metatable and it is what makes that a measured result
  instead of a hope.
- **Both call shapes are registered**: `AddItemTemplet` / `AddItemTempletTrans`
  as methods on the `g_pItemManager` stand-in *and* as bare globals, with the
  callbacks taking the first table argument they find in slots 1–2. The two
  shapes are indistinguishable from outside 34 MB of bytecode, and this way
  the question never had to be answered.
- **`g_pCX2SetItemManager` gets a stand-in too** (catch-all only, no methods) —
  the client binds the same proxy object under both names
  ([X2ItemManager.cpp:276-277](X2Lib/X2ItemManager.cpp#L276)), and set-item
  data is not wanted here.
- **No catch-all on `_G`.** It was considered and rejected: a global `__index`
  would mask exactly the "an enum is missing" failure the `Enum.lua` check
  exists to catch, and would turn `if SomeGlobal then` into an always-true
  test. A genuinely missing global now surfaces as a loud `lua_pcall` error.
- **`ItemExtractorVersion()` is 2, not 1.** It was bumped the moment the
  acceptance rule changed, because a version-1 cache was already sitting on
  disk and would otherwise have been served as valid. The comment on that
  function now says to bump it for a change to the captured fields **or** the
  acceptance rule — the plan only said fields.
- **The icon locator tables (`icon`, `icon_kom`) are not created yet.** The
  plan sketches the whole schema in one block, but creating them empty in
  phase 1 would only make them look meaningful. Phase 2 adds them and bumps
  the extractor version, which is the invalidation path that already exists.
- **The index cache is `journal_mode=MEMORY` / `synchronous=OFF`.** It is a
  pure cache, rebuildable in half a second, and the 48,754 inserts run inside
  one transaction with a prepared statement. `els_db.sql` in phase 3 gets the
  opposite treatment.
- **`libxml` stays linked but unused.** The manifest is read by a ~40-line
  attribute scanner, as section 5 suggests. Keeping the lib in the link line
  costs nothing (the linker drops what is unreferenced), keeps phase 0's proof
  that it links intact, and leaves the documented fallback available.
- **The `_MANAGED` canary is in all four new `.cpp` files** and in none of the
  four new headers — the phase 0 lesson. `KomArchive.h`, `LuaXor.h`,
  `ItemIndex.h` and `IndexCache.h` are all included from the `/clr` `Main.cpp`
  and deliberately pull in no lua / sqlite3 / zlib / libxml header; `sqlite3*`
  is reached through a forward declaration. The rebuild log confirms the split
  still holds: the five `Core` files compiled `/TP` with no `/clr`, `sqlite3.c`
  `/TC`, and only `Main.cpp` got `/clr:nostdlib`.

### Phase 2 — Icons

`DdsDecode` for DXT1 and DXT5 → 32-bit BGRA; resolve `m_ShopImage` through the
archive index; `HQ_Shop_Ui_Noimage.dds` fallback; a bounded LRU cache so browsing
does not accumulate 43,000 bitmaps.

**Exit test**: a scratch form showing a wall of ~200 decoded icons, correct
colours and alpha, with a count of how many items resolved to a real file versus
the fallback.

#### Exit test — PASSED, bar the user's own look at the wall (2026-09-06)

Built both configs (`Release|Win32` and `Debug|Win32`) on a full rebuild,
**0 Warning(s), 0 Error(s)**, no `LNK2038` / `LNK2005` / `LNK4098`. The
`/clr` split still holds: the eight `Core` translation units compiled with
no `/clr` on the command line at all and only `Main.cpp` got
`/clr:nostdlib` — read out of the `-v:normal` log, not assumed. Deployed to
`F:\...\237311\22191271\data\X2CashShopTool.exe`, landing confirmed by
listing the directory programmatically, and run with that directory as the
working directory.

First run, `--rebuild --no-window --decode-all`:

```
archives : mounted 145 of 145 (88723 names, 0 shadowed by an earlier archive)
           mount took 0.15 s
icons    : 28680 .dds locator entr(ies) from 88723 member(s) across 145 archive(s), 0.05 s (manifests only)
           28680 locator row(s) + 145 archive stamp(s) written in 81 ms
           fallback HQ_SHOP_UI_NOIMAGE.DDS: present

--- icon resolution across the whole catalog ---
  48754 item(s) total
  48419 resolve to a real file  (15098 distinct image(s) - many items share one icon)
  145 fall back: m_ShopImage is empty
  190 fall back: m_ShopImage names a file no archive holds
  335 would show HQ_Shop_Ui_Noimage.dds in the client, and will here

--- the wall: 240 tile(s), 4208 Get() call(s), 0.45 s ---
  DXT1           decoded 199
  DXT3           decoded 3
  DXT5           decoded 10
  uncompressed   decoded 12
  fallback (no m_ShopImage)      7
  fallback (no such file)        7
  fallback (undecodable)         2
  2 shown through GDI+ (a real image under a .dds name, not a DDS)
  LRU: 1391 hit(s), 2665 miss(es), 1639 eviction(s), 1024 entr(ies) holding 16384 KB of 16384 KB

--- decoding every distinct image the catalog names ---
  15265 distinct name(s), 2.44 s
  DXT1           14521
  DXT3           141
  DXT5           190
  uncompressed   245
  fallback (no such file)        167
  fallback (undecodable)         1
  the ones that are there but did not decode as a DDS:
    HQ_Shop_Common_Elite_AC_FACE2_30035.dds - fallback (undecodable): magic is 89 50 4E 47, not 'DDS '

peak working set : 116.0 MB
```

Second run, no switches — the path every later phase actually takes:

```
archives : not mounted - both halves of the index are current
catalog  : 48754 item(s), 31 category row(s) loaded from the cache in 86 ms
icons    : 28680 locator row(s) loaded from the cache in 48 ms
peak working set : 62.0 MB
```

**The archives are not opened at all on a cached run** — that is what
persisting the locator bought, and it is why the peak working set halves
from 116 MB to 62 MB.

**The decoder was verified by arithmetic, not by eye.** The exit test asks
for "correct colours and alpha", and a wall of thumbnails cannot actually
settle that, so `--dump <name> [outfile]` was added: it writes one decoded
surface out as raw BGRA, which anything can read without an image library.
A second, independently written implementation of the same block formats
(in Python, straight from the format description — `scratchpad/crosscheck_dds.py`)
then decodes the same member out of the same archive and compares
byte-for-byte. Seven files, covering every shape in the shipped set:

| file | shape | result |
|---|---|---|
| `HQ_Shop_Item_100000.dds` | 64×64 DXT1, opaque | identical |
| `HQ_SHOP_ARA_SET_ED_WEAPON140.DDS` | 64×64 DXT1, **1-bit alpha / 3-colour mode** (alpha ∈ {0,255}) | identical |
| `HQ_SHOP_ARME_CASH_FOOT120.DDS` | 64×64 DXT3 | identical |
| `HQ_Shop_Ui_Noimage.dds` | 64×64 DXT5, **graduated alpha** (0, 21, 243, 247, 255) | identical |
| `HQ_SHOP_ARA_INT_CASH_ONEPIECE110.DDS` | 64×64 uncompressed 24-bit | identical |
| `HQ_SHOP_ARME_ED_FOOT100.DDS` | 64×64 uncompressed 32-bit | identical |
| `HQ_SHOP_COMMON_AC_UPBODY_129630.DDS` | **55×55** uncompressed — partial block column and row | identical |

All 16,384 bytes (12,100 for the 55×55) match in every case, so the 565
expansion, both DXT1 colour modes, the DXT3 nibble alpha, the DXT5 3-bit
alpha palette in both of its modes, the mask-driven uncompressed unpack and
the edge clipping are all confirmed against a second transcription.

**What is left for the user is one look at the wall**, which is the half no
measurement replaces: whether the pictures are the right pictures. The
window opened and stayed up with no exception (verified by launching it and
watching the process live for 7 s before closing it), the tiles are drawn
over a checkerboard so a wrongly-decoded alpha shows as a hard square, the
fallback tiles carry a gold border, and each tile's tooltip names the item,
its `m_Name`, its `m_ShopImage`, what was actually decoded and the outcome.
The tiles are laid out in **labelled groups by outcome**, so the three DXT3
and ten DXT5 tiles are together rather than scattered through 200 DXT1 ones.

#### Corrections to this plan, found by doing it

- **Section 4 is wrong about the formats: it is not just DXT1 and DXT5.**
  Measured over all 15,098 distinct `m_ShopImage` files the catalog names —
  and confirmed twice, once by the tool and once by an independent Python
  pass over the same archives:

  | format | files |
  |---|---|
  | DXT1 | 14,521 |
  | DXT5 | 190 |
  | uncompressed 24-bit (`DDPF_RGB`) | 168 |
  | DXT3 | 141 |
  | uncompressed 32-bit (`DDPF_RGB \| DDPF_ALPHAPIXELS`) | 77 |
  | PNG, under a `.dds` name | 1 |

  A DXT1/DXT5-only decoder would have shown the fallback for 386 icons and
  had nothing to say about why. `DdsDecode` therefore handles DXT1, DXT3,
  DXT5 and mask-driven uncompressed 16/24/32-bit. **DXT3's and DXT5's colour
  block always uses the four-colour interpolation** regardless of how `c0`
  and `c1` compare — only DXT1 has the `c0 <= c1` three-colour mode with a
  transparent fourth entry. Getting that wrong is invisible on opaque icons
  and produces dark blocky edges on exactly the ones with real alpha.
- **The plan's "64×64" is right for the shop images but not for `.dds` in
  general, and ten shop images are 55×55.** 55 is 13.75 blocks, so the last
  block column and row are partial. The decoder clips rather than assuming a
  multiple of four; the 55×55 case is one of the seven files cross-checked
  above precisely because it is the one that would have overrun the surface.
- **One shipped shop image is a PNG stored under a `.dds` name** —
  `HQ_Shop_Common_Elite_AC_Face2_30035.dds`, 10,070 bytes, magic
  `89 50 4E 47`. It is a perfectly good picture, so showing the fallback for
  it would be a lie about it: `EDdsError` keeps `DdsError_NotDds` apart from
  `DdsError_Corrupt`, `CIconStore::ReadRaw` hands the undecoded bytes up, and
  the Ui puts them through GDI+. Reported as its own line so it is never
  mistaken for a decode bug. (This is why the wall shows two "undecodable"
  tiles for one distinct file: two items share that icon.)
- **The plan's icon-count estimate was low, in the useful direction.**
  Section 4 says "5,277 follow the `HQ_Shop_Item_<id>` convention" out of
  88,723 member names, and warns that many more icons use other names. The
  measured figure: **28,680 `.dds` members** across the 145 archives, of
  which 22,720 begin `HQ_SHOP_`, and the catalog's own `m_ShopImage` values
  resolve to **15,098 distinct** files. The locator stores all 28,680 —
  16 MB of SQLite, written in 81 ms — because phase 4 and 5 will want to
  resolve any icon name, not only the ones some item happens to reference
  today.
- **Phase 1's decision to "bump the extractor version" for phase 2 was the
  wrong instrument, and doing it would have cost a needless rebuild.** The
  item catalog depends on `data036.kom` alone; the icon locator depends on
  all 145 archives. Sharing one version number means every change to either
  half discards the other. So `IconLocatorVersion()` is separate (now 1),
  `ItemExtractorVersion()` stays at **2**, `icon_kom` stamps each archive's
  size and mtime, and `CreateSchema` adds the two tables with
  `CREATE TABLE IF NOT EXISTS` — a phase 1 cache gains them on its next open
  and keeps its 48,754 items. This also required `Store()` to stop doing
  `DELETE FROM index_meta`: it now deletes only the four keys it owns, or it
  would silently drop the icon locator's stamp every time the catalog was
  rebuilt.
- **`m_ShopImage` names a file no archive holds for 190 items, and is empty
  for 145.** So **335 of 48,754** items would show
  `HQ_Shop_Ui_Noimage.dds` in the client and do so here too — the client's
  rule verbatim (`IsValidFile( GetShopImage() )` else
  `L"HQ_Shop_Ui_Noimage.dds"`, [X2SlotItem.cpp:255-265](X2Lib/X2SlotItem.cpp#L255),
  re-read and confirmed). The 190 are 167 distinct missing names; `data036`
  and friends simply do not carry them. Phase 3's dropped-row report is a
  different number from a different join and these two must not be conflated.
- **The manifest step of the VS2010 build fails on this machine, reproducibly,
  and it is not a code problem.** After `X2CashShopTool.exe` links cleanly,
  MSBuild runs `mt.exe` to embed the manifest, which reopens the just-written
  exe **for write**; Defender's real-time scan of the fresh binary holds it
  and `mt.exe` fails with `general error c101008d: ... being used by another
  process`. Two consecutive Release builds failed that way while Debug
  succeeded. The fix in the project is `<GenerateManifest>false</GenerateManifest>`
  on the Ui project, with the reasoning in a comment: VC10 dropped the
  side-by-side CRT binding VC8/VC9 used, so `msvcr100.dll` is found by plain
  DLL search, and the execution level would have been the default
  `asInvoker` anyway. The manifest-less exe was then built and run to
  confirm. Same family as `CLAUDE.md`'s Defender note; expect it on the
  other tool projects too.
- **A scripted patch silently truncated a wide backslash literal.** Editing
  `IndexCache.cpp` through a bash heredoc turned `L'\\'` into `L'\'` — an
  unterminated char literal — in two places, while `L"\\"` two lines below
  survived intact. Exactly the hazard the `bash-heredoc-eats-double-backslashes`
  note warns about, and it does not fire uniformly, so "the other one came
  out right" is no evidence. The repair was to delete the construct rather
  than re-escape it: `JoinPath( dir, leaf )` now lives in `KomArchive.cpp`
  (there were four hand-rolled copies of that concatenation by this point)
  and uses a named `SEPARATOR = (wchar_t) 92` so no wide backslash literal
  has to survive a shell round-trip at all.

#### Decisions made while implementing phase 2

- **`CIconStore::Get` returns a BORROWED pointer, valid only until the next
  call to `Get`.** The alternative was copying 16 KB per lookup into a
  caller-owned buffer, which is pure waste when the Ui's next act is always
  to turn the pixels into a `Bitmap`. The contract is stated in capitals in
  `IconStore.h` and honoured in exactly one place in the Ui (`ToBitmap`),
  which is called immediately after each `Get`. It is also the right shape
  for phase 5's virtualized grid: paint the row, move on.
- **The fallback is pinned outside the LRU.** It is the single most-drawn
  image in the tool, so evicting it is the one eviction guaranteed to be
  wrong. It is decoded once, lazily, on the first `Get`.
- **The LRU budget is 16 MB, about a thousand 64×64 icons, and it is
  measured rather than asserted.** Hits, misses, evictions, entries and
  bytes held are printed on every run. `--decode-all` deliberately thrashes
  it (15,061 misses, 15,060 evictions, 37 hits over 15,265 distinct names)
  which is what proves the eviction path runs at all; the wall's own numbers
  (1,391 hits, 2,665 misses) are the realistic case.
- **`CIconStore` never XOR-decrypts.** The `.lua` members carry the XOR and
  the `.dds` members do not — the bytes straight out of `uncompress()` begin
  `DDS `. Re-verified across all 15,098 shop images in this phase, not taken
  on trust from section 4.
- **The icon locator holds every `.dds` member, not only shop images**, and
  is built from manifests alone — the 60-byte header, the three DWORDs and
  the XML, then stop, exactly as the plan prescribes. No payload is read
  during indexing; the whole sweep is 0.05 s once the archives are mounted.
- **`Adopt()` re-resolves archive file names against the directory given
  now**, rather than trusting the `icon_dir` the locator was built in. So a
  copied install with byte-identical archives reuses the cache correctly,
  and a changed one fails the size/mtime check per archive. `icon_dir` is
  stored for information only.
- **`AreIconsCurrent` also counts how many `data###.kom` are present**, not
  just whether the stamped ones still match. A 146th archive appearing, or
  one of the 145 having been absent at build time and present now, changes
  what the locator should contain and no stamp comparison would notice.
- **The wall is one owner-drawn `Panel`, not 240 `PictureBox`es.** 240
  child controls is 240 window handles and a visibly slow resize, and phase
  5's picker needs the same paint-on-demand discipline over 48,754 rows
  anyway. Fonts and pens are created once in the constructor rather than per
  `OnPaint`.
- **`SubSystem` stays `Console` even though there is now a window.** A
  console-subsystem `/clr` exe opens a `Form` perfectly well, and keeping
  stdout means the census and the wall come out of one run. The
  `EntryPointSymbol=main` trap from section 7 is therefore still not
  exercised; it goes live in phase 4, and both vcxproj files still say so.
- **`Control::Layout`, `Rectangle`, `Size` and `Point` all had to be worked
  around in the WinForms header**, and the errors are worth knowing before
  phase 4 writes a lot more of this: `Rectangle` collides with `wingdi.h`'s
  `Rectangle()` function (windows.h arrives via `msclr/marshal.h`), `Size`
  and `Point` collide with `Control`'s own inherited *properties* inside a
  `Control`-derived ref class — `Size( 900, 700 )` parses as a call to the
  property and gives "term does not evaluate to a function taking 2
  arguments" — and a private method named `Layout()` hides `Control`'s
  `Layout` **event**. All three are fixed by spelling out
  `System::Drawing::` and by renaming to `EnsureLayout()`.
- **Every managed timing is now printed in the invariant culture.** This
  machine's locale writes `0,45`, so the `{0:F2}` figures came out with
  commas while Core's own `printf` figures came out with dots — one report,
  two number formats. Every number this tool prints is meant to be compared
  against another run.
- **`CToolStopwatch` moved out of `ItemIndex.cpp`'s anonymous namespace into
  `KomArchive.h`**, and `ItemIndex.cpp` keeps its local `CStopwatch` name as
  a typedef. One clock, and the icon locator can report its own timings from
  it.
- **`--dump` is staying.** It was written for this phase's cross-check, but
  a switch that turns "does the decoder work" into a byte comparison is
  worth more than a switch that opens a window, and phase 4 will want it the
  first time an icon looks wrong.
- **The two DXT3/uncompressed-32 alpha paths could not be exercised on this
  data.** All 141 DXT3 and all 77 uncompressed-32 shop images are fully
  opaque, so their alpha handling is confirmed only against the second
  implementation's *agreement on opaque output*, not against a graduated
  case. DXT1's 1-bit mode and DXT5's 3-bit palette both had real cases and
  both matched. Recorded rather than glossed: if a future icon set ever
  shows blocky alpha on a DXT3 tile, this is the untested corner.

### Phase 3 — The database layer

`CashDb`: read `cash_product`, `settings.cash_start`, `PRAGMA user_version`;
insert / update / delete with the ≤127 validation and `product_no = max+1`;
timestamped backup into `db_backup/` before the first write; lock detection;
`wal_checkpoint(TRUNCATE)` on close.

Also here: the `user_version == 11` refusal, and the **dropped-row report** —
join `cash_product.item_id` against the phase-1 item index and list every row the
client will silently discard. On this save that is a known 388 of 2,360, so the
report has a number to be checked against on its first run.

**Exit test**: a round-trip against a *copy* of the save — insert, edit, delete,
reopen, confirm — then the same against the live file with the backup verified
on disk. Nothing in this phase runs against the live save until the copy passes.
The dropped-row report must say **388**; then launch the game and confirm
`offline_server.log` still reports `1972 product(s) from 2360` — the tool's count
and the client's must agree, and that is the check that proves the join is right.

#### Exit test — PASSED (2026-09-06)

Built both configs (`Release|Win32` and `Debug|Win32`) on a full rebuild,
**0 Warning(s), 0 Error(s)**, no `LNK2038` / `LNK2005` / `LNK4098`. The
`/clr` split still holds: `CashDb.cpp` compiled in the same `/TP` batch as
the other eight `Core` translation units with no `/clr` on the command line
at all, and only `Main.cpp` got `/clr:nostdlib` — read out of the
`-v:normal` log, not assumed. Deployed to
`F:\...\237311\22191271\data\X2CashShopTool.exe`, landing confirmed by
listing the directory programmatically, and run with that directory as the
working directory.

`--db` against the live save:

```
--- els_db.sql ---
save     : F:\...\237311\22191271\data\els_db.sql
open     : ok (read-only), PRAGMA user_version = 11
wallet   : settings.cash_start = 999999
products : 2360 row(s), next free product_no 2361

--- the dropped-row report ---
  2360 cash_product row(s)
  1972 the client will show
  388 dropped for having no item templet
  ...
  0 categor(ies) reaching no tab, holding 0 row(s)
  0 row(s) with category or quantity outside 1..127
  18 item(s) sold as more than one product - deliberate, not a defect
  price ranges 1..1
```

**The report says 388, which is what the exit test asked for**, and 1,972
kept — the client's own logged line, verbatim
(`OFFLINE_MODE_PHASE9_PLAN.md:3848-3854`). The **18** duplicated items are a
second, unasked-for agreement: `X2OfflineCashShop.cpp:100-103` says
"eighteen items are sold as two products each", and the join found exactly
eighteen without being told the number.

`--db-test --live` — the round-trip, first against a copy of the save and
then, only after that passed, against the live one:

```
--- round-trip 1 of 2: a COPY of the save ---
  copy     : C:\Users\Iruha\AppData\Local\X2CashShopTool\selftest\els_db.sql
  open     : ok (read-write), PRAGMA user_version = 11
  baseline : 2360 product(s), next free product_no 2361, wallet 999999
  refusals : the limits are enforced, not clamped -
    refused  quantity 0 / quantity 128 / category 0 / category 128
    refused  an item id no templet resolves
    refused  price -1
  insert   : product_no 2361 allocated (max+1 was 2361), item 1 "Elsword's basic hair.",
             category 11, qty 7, price 4242, event
  backup   : cashtool-20260906-172418
             ...\db_backup\els_db.sql.bak-pre-cashtool-20260906-172418      (290816 byte(s))
             ...\db_backup\els_db.sql-wal.bak-pre-cashtool-20260906-172418  (0 byte(s))
             ...\db_backup\els_db.sql-shm.bak-pre-cashtool-20260906-172418  (32768 byte(s))
  reopen   : product 2361 reads back item 1, category 11, qty 3, price 777, event 0 - matches the edit
  wallet   : settings.cash_start 999999 -> 1000000 -> 999999, restored
  delete   : product 2361 gone; 2360 row(s) left, identical row for row to the catalog this started with
  backup   : re-read at the end - 2360 row(s), the catalog as it was before the first write
  PASSED   : the copy round-tripped.

--- round-trip 2 of 2: the LIVE save ---
  ... the same sequence, against els_db.sql itself ...
  PASSED   : the live save round-tripped and ends as it started.
```

Verified afterwards from outside the tool, with Python against the real
file rather than by reading the tool's own claim: `user_version` 11, 2,360
rows, `min(price) = max(price) = 1`, no `product_no` 2361, `cash_start`
999999, `els_db.sql-wal` back to **0 bytes** (so the close-time
`wal_checkpoint(TRUNCATE)` ran), and the backup trio on disk with the main
file passing `PRAGMA integrity_check` and holding the character rows.

**Both refusal paths were exercised against real files, not just written.**
A `--db-path <file>` switch was added for exactly that (see the decisions
below):

| refusal | how it was provoked | result |
|---|---|---|
| schema is not 11 | a scratch copy of the save with `PRAGMA user_version = 12` | `REFUSED - unexpected PRAGMA user_version: ... is 12, this tool only writes version 11 saves`, exit 12; the same file back at 11 opened fine, so the refusal is the version and not the file |
| the client holds the save | a second process holding the file open while the tool ran | `REFUSED - another process holds the save: ... close the game (X2_offline.exe) first`, exit 12 |

Phases 1 and 2 re-run clean after the surgery on `Main.cpp`: cached load
81 ms / 52 ms, the same 48,754 items and 28,680 locator rows, the icon wall
and `--dump` both unchanged.

**The game was then launched, and `offline_server.log` reported the same
two numbers** — `CASH  catalog: 1972 product(s) from 2360 cash_product
row(s); 388 dropped` — confirmed by the user after the live round-trip had
run against the save. That is the check the phase actually turns on: the
tool's join and the client's own load agree on both figures, from two
independent code paths over the same table, so the catalog the tool reports
is the catalog the game sees.

#### Corrections to this plan, found by doing it

- **A backup silently overwrote another backup, and only an end-of-run
  re-read caught it.** The first live round-trip's
  `els_db.sql.bak-pre-cashtool-...` held **2,361** rows — one more than the
  catalog it claimed to predate. Three things combined: `Open()` resets the
  "backup taken" flag (correctly — a fresh connection that writes must be
  able to guarantee a snapshot exists), the round-trip closes and reopens
  three times, and `MakeBackupLabel` is second-granular while
  `BackupSaveSet` copied with `bFailIfExists = FALSE`. So the second
  connection's backup landed on the first one's name inside the same second
  and replaced a pre-edit snapshot with a mid-edit one. **A backup that a
  later, worse backup can overwrite is worse than no backup, because it
  still looks like one.** Three changes came out of it: `BackupSaveSet` now
  uniques its label against what is already in `db_backup/` (`-2`, `-3` …)
  and copies `bFailIfExists`; `CCashDb::AdoptBackup` carries one run's
  backup across a `Close()`/`Open()` pair, so a run leaves one backup rather
  than three; and the round-trip **reopens the backup at the very end and
  counts its rows**, because the size printed at the moment it was written
  was correct and the content was wrong forty milliseconds later. The plan's
  "back up the whole WAL set" caution was right and was followed from the
  first draft; this is a failure it does not cover.
- **Reading a WAL-mode backup creates `-wal` and `-shm` beside it**, so the
  backup re-check above dropped `els_db.sql.bak-pre-<label>-wal` into
  `db_backup/` — one character away from the real
  `els_db.sql-wal.bak-pre-<label>` and holding something entirely different.
  `db_backup/` is the safety net and has to stay legible under stress, so
  the check now removes the two sidecars it created, and only when the
  `-wal` is empty, which is the only shape a read can leave behind.
- **`db_backup/` still holds one mislabelled file from before that fix**:
  `els_db.sql.bak-pre-cashtool-20260906-171443` and its `-wal`/`-shm`
  partners carry 2,361 rows — the mid-test state, not a "pre" anything.
  They are safe to delete, and were left in place rather than removed on the
  tool's own initiative. The two later trios (`-172118`, `-172418`) are
  correct pre-edit snapshots at 2,360 rows.
- **The plan's "refuse to open if the client holds the file" covers reads
  too, and that is deliberate.** It reads like a write-path rule and an
  argument for letting `--db` through while the game runs is easy to make.
  It was implemented as written — refuse either way — because the report's
  whole subject is a shop the running client has *already cached*
  (`EnsureLoaded` reads `cash_product` once per process), so an answer
  produced while the game is up describes a catalog the game is no longer
  reading. Refusing says that; allowing it would not.

#### Decisions made while implementing phase 3

- **`--db-path <file>` was added, and it is not scope creep.** Without it
  the `user_version != 11` and "another process holds it" refusals could be
  written but never run, because the tool otherwise only ever opens
  `./els_db.sql` and the live save is neither of those things. It also
  serves phase 4 onward (open a backup, open a copy). The archives still
  come from the working directory; only the save moves.
- **The report opens read-only, the round-trip read-write.** `--db` cannot
  write even by accident, because the connection is `SQLITE_OPEN_READONLY`
  and not because the code path happens not to call a write.
  `SQLITE_OPEN_CREATE` is absent from both: a mistyped directory must
  produce an error, never an empty new save beside the real one.
- **`EnsureBackup` checkpoints before it copies.** The convention in
  `db_backup/` copies all three files either way; checkpointing first makes
  the copied main file complete on its own and the copied `-wal` empty, so
  the three are consistent with each other rather than a stale main file
  plus a live WAL. The existing `-wal.bak-pre-*` files at 4 MB are what the
  other shape looks like, and the v6/v7 mismatch the plan warns about is
  what it costs.
- **`journal_mode` is never touched; `synchronous` is set to `FULL` on a
  read-write open.** The first is a persistent property of the file that the
  client expects to find as WAL. The second is the exact opposite of what
  the phase 1 index cache gets (`MEMORY` / `OFF`), for the reason the plan
  gives: that one is a pure cache rebuildable in half a second, this one is
  the only copy of the character.
- **Every write is one `BEGIN IMMEDIATE` … `COMMIT`**, `IMMEDIATE` rather
  than `DEFERRED` so a lock conflict fails before the work rather than at
  the commit — the same shape as `CX2OfflineServer`'s one-packet-one-
  transaction rule, for the same reason.
- **`UPDATE` and `DELETE` treat "matched no rows" as an error.** SQL calls
  that success, while the caller believes a product changed.
  `sqlite3_changes()` is checked and the transaction rolled back.
- **Validation lives in `CCashDb::Validate` and is `const`**, so the Ui can
  refuse a value where it is typed without opening a transaction — and,
  importantly, without taking the backup. The round-trip asserts exactly
  that: six bad rows are refused and `BackupTaken()` must still be false
  afterwards.
- **The item-exists check is optional and says so.** `SetKnownItems` hands
  the phase 1 catalog over; without it `Validate` skips that one rule rather
  than pretending to have checked it. With it, an insert naming an item the
  client cannot resolve is refused with the reason, so the row the client
  would silently drop can no longer be created in the first place.
- **`price` is rejected below 0 and unconstrained above.** The plan says
  `price` is "a plain `int` and is unconstrained", which is true of the wire
  format; negative is still refused, because the value is compared against
  the wallet and nothing sensible comes of that.
- **The report already counts what phase 4 will want**: rows per billing
  category with the tab each belongs to, categories matching no
  `CashShopCategory.lua` pair (the "All" tab's orphan report — **0** on this
  save; every category present is 11..16 / 21..27 / 31..34 / 41 / 51 /
  61..63, all legal), rows outside the 1..127 limits (**0**), and the
  duplicate-item count. None of it costs a second pass.
- **The round-trip ends where it started by construction.** It inserts one
  product, edits it, deletes it, and moves the wallet up and back — then
  compares all 2,360 rows field by field against the baseline it read at the
  start. That property is what makes running it against the live save
  defensible at all, and it is checked rather than argued.

### Phase 4 — The main window

Tabs and sub-tabs from the parsed category table; the product grid with icon,
name, price, quantity, event badge; Edit and Delete; the wallet box; the "All"
tab and its report of orphaned categories.

**Exit test**: every tab matches what the game shows for the same category, and
one product edited in the tool appears changed in the game after a restart.
Entering 128 or 0 in a quantity or category cell is refused, not clamped.

#### Exit test — BUILT, DEPLOYED AND RUNNING; the two halves that need the game are the user's (2026-09-06)

Built both configs (`Release|Win32` and `Debug|Win32`) on a full rebuild,
**0 Warning(s), 0 Error(s)**, no `LNK2038` / `LNK2005` / `LNK4098`. The
`/clr` split still holds: the nine `Core` translation units compiled with no
`/clr` on the command line at all and only `Main.cpp` got `/clr:nostdlib` —
read out of the `-v:normal` log, not assumed. Deployed to
`F:/.../237311/22191271/data/X2CashShopTool.exe`, landing confirmed by
reading the directory programmatically (present, 1,326,080 bytes, fresh
mtime), and run with that directory as the working directory.

The index cache migrated on its own, which is the half of this phase that
could have gone wrong silently:

```
items    : rebuilding: built by extractor version 2, this build is 3
  CashShopCategory.lua         3193 bytes  luac  load 0.00s  run 0.00s
    tab names   : 31 of 31 row(s) carry a CSC_* name, 31 a CSSC_* one (from Enum.lua, not transcribed)
extract  : 0.47 s in total   cache : 48754 item(s) + 31 category row(s) written in 228 ms
```

and the second run took the cached path with the archives never opened:

```
archives : not mounted - both halves of the index are current
catalog  : 48754 item(s), 31 category row(s) loaded from the cache in 89 ms
icons    : 28680 locator row(s) loaded from the cache in 48 ms
open     : ok (read-write), PRAGMA user_version = 11
2360 product(s) loaded, 7 tab(s) from CashShopCategory.lua, 31 sub-categor(ies).   388 of them the game will not show.
peak working set : 47.6 MB
```

That last line is the window reporting what it built, and it is the only
thing that would have said so if the tab lists had come back empty — a
window that opens with nothing in its lists throws no exception. The load
announces itself for the same reason every write does.

The parsed tab table, now with the names the window labels its tabs by —
read back out of the cache rather than out of the tool's own report:

| tab | REAL_ID | CSC_* | billing category numbers |
|---|---|---|---|
| 1 | 10 | `CSC_FASHION` | 11..17 |
| 2 | 20 | `CSC_ACCESSORY` | 21..27 |
| 3 | 30 | `CSC_CONSUME` | 31..34 |
| 4 | 40 | `CSC_INSTALL` | 41..43 |
| 5 | 60 | `CSC_PET` | 61..63 |
| 6 | 50 | `CSC_EVENT` | 51..56 |
| 7 | 1112 | `CSC_AUTO_PAYMENT` | 1113 |

`--db`'s report is unchanged by any of this — still **2,360 rows, 1,972 the
client will show, 388 dropped, 0 orphaned, 0 out of range, 18 duplicated
items, price 1..1** — so the tab names were added without disturbing the
join phase 3 proved against the client's own log line.

The window opens, stays up and closes cleanly (launched, watched live for
8 s, closed through `CloseMainWindow`, exit 0, nothing on stderr). **The
save was verified untouched afterwards, from outside the tool**: `els_db.sql`
reads `user_version` 11, 2,360 rows, `min(price) = max(price) = 1`,
`cash_start` 999999, `PRAGMA integrity_check` ok, the WAL truncated away,
and `db_backup/` still holding only phase 3's nine files — the backup is
taken by the first *write*, so opening the editor and closing it leaves it
alone.

**What is left is the half no measurement replaces**, and it is the half the
exit test is actually about:

1. **every tab matches what the game shows for the same category** — open a
   tab in the tool and the same tab in the game side by side. The tool
   labels its tabs with the script's own `CSC_*` / `CSSC_*` names, so the
   comparison is by name and category number rather than by ordinal;
2. **one product edited in the tool appears changed in the game after a
   restart** — edit a price, close the tool, launch `start_offline.bat`,
   open the shop. The window says in as many words that a restart is
   required, because `EnsureLoaded` reads `cash_product` once per process;
3. **entering 128 or 0 in a quantity or category cell is refused, not
   clamped** — this one needs no game: open Edit on any product and type
   either.

#### Corrections to this plan, found by doing it

- **The plan gave phase 4 no way to check a tab against the game except by
  ordinal, which is the one thing the exit test asks it to do.** Phase 1
  parsed `CashShopCategory.lua` into `(tab_idx, real_id, sub_ordinal,
  cssc_enum, billing_category_no)` and stopped there, so the window could
  only have offered "tab 5" against a game tab with a picture and a
  localized caption on it. `SCashCategoryRow` now also carries the `CSC_*`
  and `CSSC_*` names, **reversed out of `Enum.lua`'s own
  `CASH_SHOP_CATEGORY` and `CASH_SHOP_SUB_CATEGORY` tables** rather than
  transcribed — the same discipline phase 1 adopted for `UC_NONE` and
  `IG_NORMAL`, and it resolved **31 of 31** rows on the first run. These are
  the script's names and not the captions the game paints; that distinction
  is stated on the struct.
- **A version bump alone would NOT have added the two columns, and the
  failure would have been a broken tool rather than a stale one.**
  `CIndexCache::CreateSchema` builds every table with
  `CREATE TABLE IF NOT EXISTS`, which does nothing to a table that already
  exists, and `Store()` clears *rows*, not columns. So bumping
  `ItemExtractorVersion()` to 3 would have forced a re-extract into a
  five-column `cash_category` while every `SELECT` named seven. The fix is a
  `HasColumn` probe over `PRAGMA table_info` and a `DROP TABLE` + recreate
  when the column is absent — safe precisely because the version bump
  re-extracts anyway. **Phases 5 and 6 will hit this the moment either adds
  a column**; the schema text now exists once, as `CASH_CATEGORY_SCHEMA`, so
  the create and the recreate cannot drift apart.
- **Tab 7 can never legally hold a product, and now there is a reason for
  it.** Phase 1 recorded `1112/1113` as "the odd one out in every respect"
  without saying why. It is `CSC_AUTO_PAYMENT` / `CSSC_AUTO_PAYMENT_BASIC` —
  a subscription tab, not a tab of goods — and its billing category, 1113,
  is **above `CASH_FIELD_MAX`**. Narrowed to the wire's signed char it
  arrives as 89, which matches no category in the table, so a row put there
  would be invisible in the game rather than merely misfiled. The tool shows
  the tab, because it is in the data, and the edit dialog refuses 1113,
  because `Validate` refuses anything over 127 — the two are consistent, and
  the reason is worth having written down before someone "fixes" one of
  them.
- **The heredoc backslash hazard fired twice in one session, and one of the
  two was silent.** The known variant (`bash-heredoc-eats-double-backslashes`)
  turned an escaped CR-LF in a format string into two real newlines — noisy,
  caught immediately. The new variant ate the line continuations off a
  multi-line `#define`, collapsing `CASH_CATEGORY_SCHEMA` onto a single
  250-character line that **compiled perfectly**. A scripted patch that
  writes valid code is the one nobody re-reads. Both were caught by reading
  the bytes back after every scripted edit; the repairs were made with the
  Edit tool, which puts no shell between the text and the file.

#### Decisions made while implementing phase 4

- **`SubSystem` stays `Console`, and `EntryPointSymbol=main` is therefore
  still not exercised.** Section 7 lists it as a link-time fact and both
  vcxproj files predict that phase 4 makes it live. It does not, and the
  reason is the verification loop: there is no test suite, every phase of
  this tool is judged by reading what it printed, and a Windows-subsystem
  exe prints nothing back to the terminal that launched it without
  `AttachConsole` gymnastics. Keeping the console also gives the editor an
  **audit log** — every write is echoed to stdout as well as to the status
  bar, which is worth having for a tool whose only job is writing into the
  only copy of the character. The trap is real and the comments describing
  it stay; it is simply not this phase's trap.
- **The editor is now the DEFAULT action, and phase 2's icon wall moved to
  `--wall`.** `--no-window`, `--decode-all`, `--dump`, `--db`, `--db-test`
  and `--db-path` all behave exactly as before, so phase 2's and phase 3's
  exit tests are still runnable verbatim from the same exe.
- **Every numeric field is a plain `TextBox`, and the category field is an
  EDITABLE `ComboBox` — never `NumericUpDown`, never a `DropDownList`.**
  This is the phase's exit test expressed as a control choice:
  `NumericUpDown` silently clamps 128 to its Maximum, and a closed list
  makes 128 impossible to type — either one turns "refused, not clamped"
  into a claim that cannot be tested. The list is the convenience; the free
  text is the contract. There is a comment saying so at the top of
  `EditProductForm.h`, because this is exactly the kind of thing a later
  tidy-up "improves".
- **The dialog calls `CCashDb::Validate` — the same `const` method the write
  path calls — and shows the reason it gives.** A value refused where it is
  typed is refused for the database layer's reason, not for a second opinion
  written in the Ui that can drift from it. This is what phase 3's decision
  to make `Validate` public and `const` was for.
- **The item a product sells is shown and not editable.** Changing it needs
  the picker, which is phase 5; a free-text item id box would have been a
  way to create exactly the row the client silently drops, which is the
  failure the tool exists to expose.
- **The "All" pseudo-tab's sub-list IS the report the plan asked for**:
  everything / orphaned - no tab shows it / dropped - no item templet / out
  of range - not 1..127, each with its count. On this save that reads
  2,360 / 0 / 388 / 0. **The 388 rows that until now existed only as a
  number in one log line are individually reachable**, listed by product,
  item and category, painted with a dark band and labelled in the row
  itself.
- **The grid is one owner-drawn `Panel` painting only the visible band**,
  the discipline phase 2 settled on and the one phase 5's picker needs over
  48,754 rows. Two things a `Panel` does not give for free and had to be
  written: `IsInputKey` must be overridden or the arrow keys never reach
  `OnKeyDown` at all, and `ControlStyles::Selectable` plus a `Focus()` on
  mouse-down is what makes it keyboard-driven in the first place.
- **`IconProvider` is a SECOND, thinner cache above `CIconStore`'s own.**
  The native store caches decoded BGRA in a 16 MB LRU; this one caches the
  managed `Bitmap` made from it, name-keyed, FIFO, capped at 1,500. Without
  it every repaint would rebuild a `Bitmap` per visible row and churn GDI+
  handles on every scroll. FIFO rather than LRU because the access pattern
  is a scrolling list. **Eviction disposes**, which is why the modal edit
  dialog is handed a `gcnew Bitmap( kIcon )` copy: the grid repaints behind
  a modal dialog, and a repaint that evicts would dispose the bitmap the
  dialog is still showing.
- **The backup is taken by the first write, not at startup.** `EnsureBackup`
  is idempotent and public precisely so the Ui *could* take it up front, but
  doing so would drop three files into `db_backup/` every time the tool was
  opened merely to look at something. The status bar names the backup the
  moment one exists. Verified by opening and closing the editor and finding
  `db_backup/` unchanged.
- **`String^ == String^` is a REFERENCE comparison in C++/CLI**, unlike C#.
  A first draft laid the header out by walking `Controls` and comparing
  `kChild->Text` against a literal, which would have silently matched
  nothing and left the wallet label sitting under its text box. Same class
  of bug as indexing `Controls[2]` to find a button, which the same draft
  also did; both are now held as members.

### Phase 5 — Insert, with the virtualized picker

`InsertItemForm`: VirtualMode `ListView` over the whole catalog, search by name
and id, optional filters by item type and equip slot; then the field form.

**Exit test**: the picker opens instantly and scrolls the full ~43k rows without
stutter or growth in memory; an item that was never purchasable is inserted and
is then buyable in the game.

#### Exit test — BUILT, DEPLOYED, MEASURED; the in-game half is the user's (2026-09-07)

Both configs on a full rebuild, **0 Warning(s), 0 Error(s)**. The `/clr` split
still holds in both: exactly one `/clr:nostdlib` on each command line, on
`Main.cpp`, read out of the `-v:normal` log. Deployed to
`F:/.../237311/22191271/data/X2CashShopTool.exe`, confirmed by reading the
directory programmatically and by SHA-256 against the build output — 1,378,304
bytes, identical, and still the only `X2CashShop*` name in the directory.

The first half of the exit test is two measurements and one probe, all from
`--picker-test`, a new switch that builds the picker headless and touches
`els_db.sql` not at all:

```
build    : 48,754 item(s) marshalled in 81 ms, 17,708 KB managed
filters  : 9 item type(s) and 20 equip slot(s) actually occur

  a filter pass is over ALL 48,754 rows; no index, no debounce:
    (empty)           48,754 row(s)   1 ms
    hat                  122 row(s)   4 ms
    aisha                947 row(s)   4 ms
    aisha hat             11 row(s)   4 ms
    1316                  99 row(s)   4 ms
    131641                 1 row(s)   4 ms
    zzzznothing            0 row(s)   3 ms

  extent   : 2,145,176-pixel canvas for 48,754 row(s) at 44 px
             scrolled to the end: y = 2,144,688, last row visible = 48,754   -> the last row is reachable

  scroll   : 48,754 icon fetch(es) - every row in the list - in 3.48 s
             71.3 us per row; a screenful is about 20 rows
  managed  : 17,493 KB before, 17,575 KB after - delta 82 KB
  native   : 0 hit(s), 17943 miss(es), 16917 eviction(s); 1024 entr(ies), 16384 of 16384 KB
  peak ws  : 109.5 MB
```

So: **4 ms a keystroke** over the whole catalog, **71 µs a row** to draw, which
is 1.4 ms for a twenty-row screenful, and **82 KB** left behind by fetching
every icon in the catalog — the two bounded caches hold at their caps
(1,024 entries / 16,384 KB on the native LRU) rather than growing. "Without
stutter or growth in memory" is those three numbers; whether it *feels* smooth
is the half only a person answers.

Phase 3's round-trip was re-run because `Core` changed under it, and still
**PASSED**: all six refusals refused (quantity 0 and 128, category 0 and 128, an
unresolvable item id, a negative price), `product_no 2361` allocated as max+1,
the edit read back, the wallet restored, the row deleted and the catalog
identical row-for-row to what it started as.

The editor was launched, watched for 9 s, closed through `CloseMainWindow`
(exit 0, nothing on stderr, the clean-exit `peak working set : 47.8 MB` line
printed). **Peak working set is unchanged from phase 4's 47.6 MB**, which is
the lazy build working: the picker's 18 MB is paid on the first Add and not by
a session that opened the tool to change a price. The save was verified
untouched from outside the tool afterwards — `user_version` 11, 2,360 rows,
`min(price) = max(price) = 1`, `product_no` 1..2360, `cash_start` 999999,
`integrity_check` ok, the WAL truncated away, and no new `cashtool-20260907-*`
files in `db_backup/`.

**What is left is the in-game half**, which no measurement replaces: *an item
that was never purchasable is inserted and is then buyable in the game.* Open
the tool, pick a tab, press Add or Insert, search the picker for something that
was never for sale (the picker marks the 2,360 items that already are), set a
price, close the tool, launch `start_offline.bat`, and buy it. Phase 5 removed
one obstacle to that test being conclusive: see the `IsShowPackageItem`
correction below.

#### Corrections to this plan, found by doing it

- **There is a SECOND silent-drop rule in the shop, and the plan did not know
  about it.** The plan's headline finding is the 388 rows dropped for having no
  item templet. But `CX2ItemManager::AddCashItem` sets each `CashItem`'s
  `m_bShow` from `IsShowPackageItem( itemID )`
  ([X2ItemManager.cpp:1868](X2Lib/X2ItemManager.cpp#L1868)), and
  `GetAllCashItemList` **erases every entry whose `m_bShow` is false**
  ([:2866-2880](X2Lib/X2ItemManager.cpp#L2866)) — so an item in
  `m_setShowPackageItem` is filtered out of the shop *after* the catalog packet
  has already carried it. That set is the items `PackageItemData.lua` declares
  as package contents with `bShowItem` false
  ([:3288-3292](X2Lib/X2ItemManager.cpp#L3288)). This rule is invisible in
  `offline_server.log` — the emulator's "1972 product(s) … 388 dropped" line is
  computed before it applies — so a product inserted for such an item would
  simply never appear, with nothing anywhere saying why. That is exactly the
  failure mode phase 5's exit test would have hit blind. The tool now runs
  `PackageItemData.lua`, carries the set, and says so in the picker, in the
  edit dialog, in the grid row and as a filter in the "All" tab.

  **On this install the set is empty**, and that is a measured answer rather
  than an assumption: 7,472 `AddPackageItemData` rows, not one with `bShowItem`
  false. `IsShowPackageItem` returns true for anything not in the set, so every
  one of the 48,754 items is insertable and will show. The machinery stays
  because the rule is real and a later `.kom` could populate it; `--picker-test`
  and the console both report the count, and `package_data_ran` in the cache
  keeps "the set is empty" distinct from "nobody could tell".

- **`0` from a new extractor is not the same as `0` from a failed one, and the
  first draft could not tell them apart.** `PackageItemData.lua` failing has to
  be non-fatal — what it contributes is an advisory, while the other four
  scripts produce the catalog itself — but "no hidden items" and "the script
  did not run" then print identically. `SExtractResult::bPackageDataRan` and the
  `package_data_ran` meta key exist for that, and every place the count is shown
  says `unknown` instead of `0` when the flag is false. This mattered
  immediately: the script failed on the first run (below), and the run that
  followed the fix reported the same `0` for an entirely different reason.

- **The stand-in receiver had to be bound under every name
  `OpenScriptFile` declares, and the plan's list of two was wrong.**
  `PackageItemData.lua` died on its seventh line with `attempt to index global
  'g_pCashItemManager' (a nil value)`. `CX2ItemManager::OpenScriptFile`
  ([X2ItemManager.cpp:165-172](X2Lib/X2ItemManager.cpp#L165)) calls
  `lua_tinker::decl` **four times with the same `this`** —
  `g_pItemManager`, `g_pManufactureItemManager`, `g_pCashItemManager`,
  `g_pCX2SetItemManager`, plus `g_pCX2CubePackageManager` under
  `PACKAGE_IN_QUBE_PREVIEW` — so they are one object under five names, and the
  tool now installs one stand-in table under all five. Phase 1 got away with two
  because `Item.lua` and `ItemTrans.lua` happen to use `g_pItemManager`. The
  list is read out of `OpenScriptFile`, not guessed at.

- **`AddPackageItemData` is the one stub that takes positional arguments**, not
  a field table: `( iPackageItemID, iItemID, usProductPieces, bShowItem )`,
  bound at [X2Main.cpp:3348](X2Lib/X2Main.cpp#L3348). Since 7,472 rows with not
  one `bShowItem` false is the sort of answer that is either the truth or an
  off-by-one in the argument indices, it was settled with a diagnostic rather
  than by re-reading them — `X2CASHTOOL_PACKAGE_ARG_DEBUG` printed
  `top=5 base=1 : [1]table [2]number=200890 [3]number=200950 [4]number=0
  [5]boolean=true` six times over. The switch stays, undefined, with that output
  quoted next to it, because the surprising number is the one someone will want
  to re-check.

- **A scroll-extent probe on an unshown form measures nothing, and says so
  confidently.** 48,754 rows at 44 px is a 2,145,176-pixel virtual canvas, which
  is the one number in the picker big enough to be worth doubting — a saturating
  scroll range would leave the last thousands of items unreachable while
  everything above them looked perfect. The first probe reported
  `*** THE END OF THE LIST CANNOT BE REACHED ***`, and that was the measurement
  failing: a `Panel` with `AutoScroll` has no scrollbars until it is laid out on
  a **visible** form, so assigning `AutoScrollPosition` before that is simply
  dropped. Shown off-screen at `(-32000, -32000)`, it reaches `y = 2,144,688`
  and row 48,754. A diagnostic that fails loudly for its own reasons is worse
  than none, because its output looks exactly like the defect it was written to
  find.

- **`NativeBridge` and `IconProvider` had to leave `MainForm.h`.** Phase 4 wrote
  both inside it; phase 5's picker needs both, and `MainForm.h` includes the
  picker — a cycle. They moved verbatim into a new `Ui/UiBridge.h` that both
  include. Bodies unchanged; only the file is new.

#### Decisions made while implementing phase 5

- **The picker's list is an owner-drawn `Panel`, NOT the VirtualMode `ListView`
  the plan's decision table specifies.** Two reasons, and the second is the
  deciding one. Phase 4's own note already calls the panel "the one phase 5's
  picker needs over 48,754 rows". And the icons settle it: a virtual `ListView`
  can only draw an icon from a pre-populated `ImageList`, so 48,754 rows means
  either decoding the whole catalog up front — 780 MB of 64×64 BGRA — or writing
  an owner-draw path on top of virtual mode anyway. The panel gets icons on
  demand through the same `IconProvider` the product grid uses and touches only
  the ~20 rows on screen. The two `Panel` traps phase 4 documented both applied
  again unchanged: `IsInputKey` must be overridden or the arrow keys never reach
  `OnKeyDown`, and `ControlStyles::Selectable` plus a `Focus()` on mouse-down is
  what makes it keyboard-driven.
- **The catalog is marshalled once per run, on the first Add, and kept.**
  48,754 items × (name, shop image, two enum labels, a lower-cased name) is 81 ms
  and ~18 MB, which is worth paying once and not worth paying at startup: most
  sessions open this tool to change a price and never open the picker. Measured
  both ways — peak working set is 47.8 MB for a session that does not open it,
  against phase 4's 47.6 MB.
- **`NameLower` is precomputed per item and compared with `StringComparison::Ordinal`.**
  Culture-aware comparison over 48,754 rows on every keystroke is roughly two
  orders of magnitude slower and would have been the entire cost of the pass.
  Space-separated terms are ANDed over the name; an all-digits query
  additionally matches an item id **prefix**, computed arithmetically
  (`for( iId = ItemID; iId > 0; iId /= 10 )`) rather than with a `ToString` per
  row — 48,754 string allocations per keystroke to answer "does 1316 prefix
  this id" is the sort of thing that makes a list feel slow for no reason.
- **No debounce timer, and the filter cost is printed in the footer.** A 4 ms
  pass does not need debouncing, and a window that reports what its own filter
  cost will say so if that ever stops being true, instead of just feeling
  sluggish.
- **The two filter dropdowns are built from the values that actually OCCUR, not
  from the enum tables** — 9 item types and 20 equip slots out of the 13 and 40
  `Enum.lua` names — each with its name and a count. A filter that offers a
  value no item has is a filter that returns an empty list and teaches nothing.
  The labels come from a new `enum_name` cache table holding
  `ITEM_TYPE` / `ITEM_GRADE` / `EQIP_POSITION` / `USE_CONDITION` reversed out of
  `Enum.lua` (63 values), the same discipline phase 1 took for `UC_NONE` and
  phase 4 for the tab names. A value the script has no name for is shown as
  `<n>  (no Enum.lua name)`, because that is information rather than something to
  paper over.
- **`ItemExtractorVersion()` went to 4, and phase 4's `HasColumn` probe was
  applied to both new tables** even though neither can pre-exist in any cache
  written so far. Phase 4's correction predicted phases 5 and 6 would hit that
  trap; the probes are no-ops today and are there so the version bump that adds
  a column to `enum_name` later is not the third occurrence of the same defect.
  Each table's `CREATE` is spelled once, as a macro, for the same reason
  `CASH_CATEGORY_SCHEMA` is.
- **One dialog with two modes, not two dialogs.** `EditProductForm` gained
  `bIsInsert`, which changes the captions and — load-bearingly — passes
  `bIsInsert` to `CCashDb::Validate`, since an insert has no `product_no` yet
  and the edit path refuses a non-positive one. A second copy of the four
  fields would have been a second place for the 1..127 rule and its explanation
  to drift. It also gained an `sExtraNote` line, amber, which today carries only
  the hidden-package warning.
- **The picked item is still not editable in the field dialog.** Add chooses an
  item in the picker and Edit cannot change one; a free-text item id box remains
  the one thing that could produce the row the client silently drops.
- **`AcceptButton` is deliberately NOT set on the picker.** Enter belongs to the
  list, where it activates the highlighted row; a form-wide accept button would
  steal it while the search box has focus and commit whatever happened to be
  selected.
- **The picker marks items that are already sold and offers to hide them, and
  does not refuse them.** Eighteen items are deliberately sold as two products
  each ([X2OfflineCashShop.cpp:100-103](X2Lib/Offline/X2OfflineCashShop.cpp#L100)),
  so a duplicate is legal; the badge is information, the checkbox is
  convenience, and neither is a rule.
- **The new product's category defaults to the sub-tab being viewed**, and to 0
  on the "All" tab — where `Validate` then refuses it, which is correct, because
  there is no sensible guess and a silently-chosen category is how a product
  lands in a tab that makes no sense.
- **`--picker-test` opens no window and reads no save.** It exists because there
  is no test suite and the exit test's first half is numbers; it drives the same
  `ApplyFilter` the keystrokes drive, through `MeasureFilter`, rather than a
  parallel copy of the loop — a measurement of a parallel implementation is a
  measurement of nothing.
- **`iKept` still means "what the catalog packet will carry".** The
  package-hidden count is reported *beside* it and never folded into it, because
  `iKept` is the number `offline_server.log` prints and the number phase 3
  checked the join against. Folding a further client-side filter into it would
  have broken the one cross-check the report has.

### Phase 6 — Plain language, and a layout that reads

Phases 4 and 5 built a window that is correct and hard to use. It is laid out
like the data model it edits: the left-hand lists are headed
`tab  (CashShopCategory.lua)` and `sub-category  ->  cash_product.category`, a
tab is called `CSC_FASHION`, a sub-tab `CSSC_FASHION_WEAPON -> 11`, the picker's
biggest filter offers `IT_DEFENCE` for the 34,083 items it covers, the event
checkbox is captioned `is_event`, and the 96-pixel header spends three lines of
goldenrod prose citing `X2OfflineCashShop.h:98`. Every one of those is the
*script's* name or the *column's* name rather than the thing's name, and the one
label that is neither is wrong: the price column reads `1  ED`, and the shop
prints prices in **K-Ching**.

None of that is decoration. The tool's stated outcome is to show the catalog
"as the game shows it — the game's own tabs, the game's own item icons, the
game's own item names", and two thirds of that is done: the icons are the
game's and the names come from `ItemTrans.lua`, so the *primary* text on every
row is already the localized name a player sees. This phase is about everything
around it.

**The whole phase turns on one question — where does a human label honestly come
from?** — so that is settled first, by measurement, before any control moves.

#### Where the labels come from: four tiers, and which enum falls in which

There is a real localized string table in the game directory, and phases 1–5
never touched it. `Core/ItemIndex.h:58-63` says the `CSC_*` names are "the
SCRIPT's names, not the captions the game paints - those are localized strings
this tool does not read". Half of that comment is now wrong and the other half
is more interesting than it looks: **some** of those captions are strings, and
the cash-shop tab captions are not strings at all. The comment gets corrected in
the same commit as this phase.

**Tier 1 — `General.ess`, the game's own localized string table.** Verified
first-hand against the real game directory, not inferred:

| Fact | Detail |
|---|---|
| Where | A **loose file** in the game directory, `General.ess` ([KTDXApp.cpp:162](KTDXLIB/KTDXApp.cpp#L162)), opened with `_wfopen` ([KTDXStringTable.cpp:201](KTDXLIB/KTDXStringTable.cpp#L201)) — **not** through the mass file manager. No `.kom`, no zlib, no Lua, no XOR key. On this install it is `general.ess`, 3,043,944 bytes |
| Format | UTF-16LE; skip the 2-byte BOM ([:211](KTDXLIB/KTDXStringTable.cpp#L211)), read a line, strip the trailing `\r\n` ([:232-236](KTDXLIB/KTDXStringTable.cpp#L232)), **XOR every `wchar_t` with 16** ([:238-243](KTDXLIB/KTDXStringTable.cpp#L238)), split on the first tab, then `\\n` → CRLF ([:274](KTDXLIB/KTDXStringTable.cpp#L274)) |
| Index check | The left half of each line is the row index and **must equal the running line count** ([:250-254](KTDXLIB/KTDXStringTable.cpp#L250)). The client `ASSERT`s it; the tool checks it and refuses the file on a mismatch rather than shifting every label by one |
| Size | 30,056 rows on this install, indices 0..30055 dense |
| Not a thing | `InitializeStringTable` **ignores its second argument entirely** ([:21-24](KTDXLIB/KTDXStringTable.cpp#L21)) — `Script.ess` is never loaded by anything, and there is none in the game directory. `GET_SCRIPT_STRING` indexes an empty vector. Don't go looking for one |

A ~40-line reader, and the cheapest new code in this tool by a wide margin.

The string *ids* carry no semantic names — `StringID_def1.h` spells them
`STR_ID_263` with the Korean original in a trailing comment — so an id is only
meaningful through a mapping, and **the mappings are transcribed out of client
code in this repo, not guessed**:

| Enum | The client's mapping | Strings, read back out of `general.ess` |
|---|---|---|
| `EQIP_POSITION` | the `switch` at [X2ItemSlotManager.cpp:1199-1265](X2Lib/X2ItemSlotManager.cpp#L1199) | `STR_ID_263..277` — `Hair`, `Top Piece`, `Accessory (Top Piece)`, `Bottom Piece`, `Accessory (Bottom Piece)`, `Gloves`, `Shoes`, `Weapon`, `Face Accessory (Top/Middle/Bottom)`, `Accessory (Ring/Necklace/Arm/Weapon)` |
| `EP_ONEPIECE_FASHION` | the same switch, under `SERV_NEW_ONE_PIECE_AVATAR_SLOT` ([:1260-1265](X2Lib/X2ItemSlotManager.cpp#L1260)) | `STR_ID_28362` = `Suit`. **The flag is defined for US** ([ServerDefine_US.h:300](KncWX2Server/Common/OnlyGlobal/ServerDefine/ServerDefine_US.h#L300)), so this branch is live in this build |
| `ITEM_GRADE` | the `switch` at [X2ItemSlotManager.cpp:1089-1110](X2Lib/X2ItemSlotManager.cpp#L1089) | `STR_ID_257..261` = `[Unique] [Elite] [Rare] [Normal] [Old]` — **the strings carry square brackets**; strip them for a filter caption and keep them where the game would show them |
| the costume flag | [X2ItemSlotManager.cpp:1270-1274](X2Lib/X2ItemSlotManager.cpp#L1270) | `STR_ID_251` = **`Costume`**. That is the game's word for `m_bFashion`; the tool currently says "fashion" |
| the currency | [X2CashShop.cpp:9194](X2Lib/X2CashShop.cpp#L9194) — `wstrstm << m_iSalePrice << GET_STRING( STR_ID_34 )` | `STR_ID_34` = **`K-Ching`**. This is the fix for the `ED` in the price column |

**Tier 2 — the studio's own words, out of a shipped dialog script.** The cash
shop's tab captions are **pictures, not text**, so tier 1 cannot reach them.
`DLG_Cash_Shop_Subpage_Fashion.lua` (in `data034.kom`, 8,560 bytes decompressed,
luac like everything else) builds its sub-tabs as texture radio buttons —
`SetNormalTex( "DLG_Common_New_Texture54_A.TGA", "wapon_normal" )` — and names
the controls `Tab_Total`, `Tab_Wapon`, `Tab_Hair`, `Tab_Hood1`, `Tab_Hood2`,
`Tab_OnePiece`, `Tab_Glove`, `Tab_Shoes`, each tagged with its
`CASH_SHOP_SUB_CATEGORY.CSSC_*` through `AddDummyInt`. So the caption is a TGA
region, and the nearest thing to a name is the control name.

Those control names are worth having and are not worth cleaning up: one of them
is a typo in the original (`Tab_Wapon`), and `Hood1`/`Hood2` are the studio's
own words for two tabs whose `CSSC_*` names say something else. **Transcribe,
don't tidy** — the same rule the live-DB section states for a table with an
unfinished-looking row, and for the same reason: a tool that silently corrects
studio data disagrees with the game for a reason nobody can later reconstruct.

**Tier 3 — this tool's own wording, in one table, marked as such.** What is
left after tiers 1 and 2 is real and has to be written by hand:

- **every `ITEM_TYPE` value.** The only `ITEM_TYPE` → `STR_ID` mapping in the
  client is two cases wide and sits inside `#ifdef SHOW_ITEM_TYPE_AT_TOOLTIP`
  ([X2ItemSlotManager.cpp:2224-2241](X2Lib/X2ItemSlotManager.cpp#L2224)), which
  is **defined nowhere in this tree**. Compiled out is not the same as wrong,
  though: the pairs it names — `IT_WEAPON` → `STR_ID_270` `Weapon`,
  `IT_DEFENCE` → `STR_ID_17818` `Armor` — are the studio's own, and they cover
  the two biggest buckets in the catalog. Take those two from there and label
  their origin honestly; the other seven are this tool's wording.
- **the four `EQIP_POSITION` values the switch does not cover** (below).
- **`CASH_SHOP_CATEGORY` and `CASH_SHOP_SUB_CATEGORY`**, beyond what tier 2
  gives.

**Tier 4 — the number and the script name, always still there.** Never removed,
only demoted. See the label rule below.

#### Coverage, measured rather than assumed

Counted against the live `ItemIndex.db` (`%LOCALAPPDATA%\X2CashShopTool\`) and
the decoded `general.ess`, so the phase starts knowing exactly how much of the
window tier 1 can actually carry:

| Enum | Values occurring | Resolve from `General.ess` | Fall to tier 3 |
|---|---|---|---|
| `ITEM_TYPE` | 9 of 13 | **0** live; 2 from the compiled-out block (`IT_WEAPON` 6,929 items, `IT_DEFENCE` 34,083) | 7 — `IT_ACCESSORY` 1,442, `IT_SPECIAL` 4,520, `IT_QUEST` 966, `IT_QICK_SLOT` 438, `IT_MATERIAL` 234, `IT_SKILL_MEMO` 105, `IT_OUTLAY` 37 |
| `EQIP_POSITION` | 20 of 40 | **16** | 4 — `EP_NONE` (5,862 items), `EP_QUICK_SLOT` (438), `EP_DEFENCE_FACE` (16), `EP_RAVEN_LEFT_ARM` (7) |
| `ITEM_GRADE` | 6 | **5** (`STR_ID_257..261`) | 1 — `IG_NONE` (5 items) |

Two things fall straight out of that table. `EP_NONE` is the second-largest
equip-slot bucket in the catalog and **is not a slot at all** — its honest label
is "not equipped" and that is unambiguously the tool's wording, not the game's.
And `IT_ACCESSORY` has no client mapping even though `STR_ID_246` is
`Accessories`; pairing the two would be a guess that looks like a citation,
which is precisely the failure the live-DB section of this plan was written
about. It goes in as tier 3 with the tool's wording, and if the pairing is
wanted it is wanted as an explicit decision, not as an inference nobody recorded.

#### The label rule

**One place, one function, and every label carries where it came from.** A new
native `Core/Labels.{h,cpp}` — native, so no `/clr` reaches it — holding the
`.ess` reader, the transcribed switches, and the curated table, and answering
every request with the text *and* an origin:

| Origin | Means |
|---|---|
| `ESS` | the game's own localized string, reached through the client's own mapping |
| `CODE` | a mapping that exists in this repo but that this build compiles out (the two item types) |
| `SCRIPT` | the studio's own control name out of a shipped dialog `.lua` |
| `TOOL` | this tool's wording |

And five rules on top of it:

1. **Never present `TOOL` wording as the game's.** `--labels` dumps the whole
   set — every value of every enum, its label, and its origin — so the tool's
   own inventions are auditable in one read rather than scattered through a UI.
   The details pane shows the origin for the selected row.
2. **Never replace a number with a label where the number is the contract.**
   `category`, `product_no`, `item_id`, `quantity` and `price` are what gets
   written to the save; a label sits *beside* them and never instead of them.
   The edit dialog's category box stays an **editable** `ComboBox` carrying the
   number, for exactly the reason phase 4 recorded: a closed list makes 128
   impossible to type, and typing 128 and watching it be refused is an exit
   test.
3. **Degrade visibly.** If `General.ess` is missing, or fails the index check,
   the tool falls back to the enum names, says which file is missing and what
   that costs, once, in the status bar and on the console — the same shape as
   the offline mode's degrade rule. A window that quietly shows `IT_DEFENCE`
   because a file was absent is indistinguishable from today's window.
4. **No cache column, and no `ItemExtractorVersion()` bump.** The `.ess` is a
   3 MB loose file parsed in tens of milliseconds; it is read every run and
   never cached. That is deliberate — it is also what keeps this phase clear of
   the `HasColumn` trap phase 4 documented and predicted for the phases after
   it. That trap belongs to phase 7, which does add columns.
5. **`product`, `category` and `quantity` keep their names.** They are the
   billing number space's own words and the save's own column names, and
   renaming them in the UI would put a third vocabulary between the user and
   both. The labels this phase adds are for the *values*, not the fields.

#### The layout

| Today | What it becomes |
|---|---|
| Two stacked monospace `ListBox`es in a 300 px left panel, headed `tab  (CashShopCategory.lua)` and `sub-category  ->  cash_product.category` | Human tab names with counts; the headings name the thing, not the file it was parsed from |
| The "All" pseudo-tab mixes the four diagnostic buckets (`orphaned`, `dropped`, `out of range`, `hidden`) into the same list as the real tabs | The buckets move into their own **Problems** group, so repricing a product never navigates through them — and so their count is visible without selecting anything |
| 96 px header carrying three lines of goldenrod prose citing `X2OfflineCashShop.h:98` | One line — *restart the game to see changes* — with the citation in the details/About pane. The rule is the important part; the file:line is for whoever doubts it |
| Row line 2: `product 12   item 131641   category 11  CSC_FASHION / CSSC_FASHION_WEAPON` | `Costume · Shoes · Rare`, with the numbers in the details pane and the technical view |
| Price drawn as `{0}  ED` | `K-Ching`, per `STR_ID_34` |
| The orange `EVENT` badge is the loudest thing on a row | Demoted to a small mark. `m_bEvent` appears **nowhere in `X2Lib/` or `KTDXLIB/` outside `Offline/`** — the emulator sets it ([X2OfflineCashShop.cpp:172](X2Lib/Offline/X2OfflineCashShop.cpp#L172)) and no client code reads it, so today the catalog's loudest visual marks the one field with no in-game effect. The edit dialog says that in words instead of `is_event` |
| Button captions carry the key hints: `Edit  (Enter)`, `Delete  (Del)`, `Add product  (Ins)` | Clean captions, hints in tooltips and the status line. The keys keep working — `IsInputKey`, `Selectable` and the `Focus()` on mouse-down are all load-bearing and phase 4 and 5 both had to discover them |
| Fixed-pixel `Bounds` everywhere plus a hand-rolled `LayoutHeader()` re-running on `Resize` | Containers that survive both the 900×560 minimum and a maximized window without overlap or clipping |
| `Consolas 8.5` as the primary font for both lists | Segoe UI for prose; Consolas only in the number columns, where digits have to align |
| Nothing shows the selected row in full | A **details pane** — the single place every demoted technical fact lives: `product_no`, `item_id`, `m_ShopImage`, the billing category and its `CSC_*`/`CSSC_*` names, the label origins, and whichever of the two silent-drop rules applies |

#### The technical view stays, behind a toggle

The dense presentation is not clutter to be deleted — it is the tool's whole
diagnostic value, and it is what made the 388 invisible rows individually
reachable for the first time. So a **View → Technical details** toggle brings
today's presentation back verbatim: enum names in the lists, numbers on the
rows, the full citation in the header. Default **off**; `--technical` starts
with it on; the setting persists in a small `X2CashShopTool.ini` beside the
index cache in `%LOCALAPPDATA%\X2CashShopTool\` — *not* in `ItemIndex.db`,
which is a cache that gets thrown away and rebuilt.

**The console output and every switch are unchanged.** `--db`, `--db-test`,
`--picker-test`, `--dump`, `--decode-all`, `--wall`, `--no-window` and
`--db-path` all behave exactly as before, so the exit tests of phases 0–5 stay
runnable verbatim from the same exe. The console is also the audit log for
every write, which matters more than the window's wording for a tool whose only
job is writing into the only copy of the character.

#### Things this phase must not lose

Checked one by one at the end, with the technical view **off**, because a
prettier window that has quietly dropped one of these is a worse tool than the
one it replaced:

- each of the 388 dropped rows individually reachable, and marked on the row;
- the orphaned-category, out-of-range and package-hidden buckets, with counts;
- the exact `product_no` / `item_id` / `category` / `quantity` / `price` of the
  selected row, somewhere, without switching the toggle on;
- the 1..127 refusal and the reason `CCashDb::Validate` gives for it — refused,
  never clamped, and refused for the database layer's reason rather than a
  second opinion written in the Ui;
- the restart-to-see-changes rule;
- the picker's filter cost in its footer, and the `already sold` /
  `package-hidden` marks;
- the backup-on-first-write status line naming the backup it took.

#### Out of scope, and why

**Painting the game's real tab captions.** Both halves are located, so a later
phase need not re-explore: `DLG_Common_New_Texture54_A.TGA` (1,048,620 bytes)
and `DLG_Common_New_Texture54_A.TET` (8,345 bytes), both in `data054.kom`.
Doing it needs a TGA decoder plus a `.TET` named-region parser — a
named-region atlas, which is exactly the thing section 4 of this plan
established the *item icons* do **not** need — to render a decoration. It would
make the tool's tabs literally the game's tabs, which is appealing and is not
what makes the window less technical. Note it, don't build it.

**Renaming `product` / `category` / `quantity`**, per rule 5 above.

**Exit test**, in four parts, three measurable and one only a person can answer:

1. **`--labels` resolves what the coverage table above says it should.** Every
   `ITEM_TYPE`, `EQIP_POSITION` and `ITEM_GRADE` value that occurs in the
   catalog, plus every `CSSC_*` in the tab table, printed with its label and
   its origin; 16 of 20 equip slots and 5 of 6 grades come back `ESS`, the two
   item types come back `CODE`, and every `TOOL` label is listed together so the
   whole set of this tool's own wording can be read in one place. Then delete
   or rename `general.ess` and confirm the tool says which file is missing and
   falls back to enum names instead of showing them silently.
2. **No enum name and no source citation is visible anywhere in the default
   view** — tabs, sub-tabs, rows, both filter dropdowns, the edit dialog, the
   header, the status bar. Prices read `K-Ching`. `m_bFashion` reads `Costume`.
   Turning the technical view on brings all of it back, and turning it off
   again leaves every diagnostic in the *must not lose* list still reachable.
3. **Resize to the 900×560 minimum and to maximized**: nothing overlaps,
   nothing is clipped, the grid and both lists still scroll to their last row.
   Phase 5's own scroll-extent probe is the precedent for not trusting this by
   eye — and for the trap that a probe on an unshown form measures nothing.
4. **The half no measurement replaces**: a person who has not read this plan
   opens the tool and reprices one product, and adds one, without asking what a
   word means. That is the phase's actual exit test. The user runs it.

`--db`'s report must be unchanged by all of it — still 2,360 rows, 1,972 the
client will show, 388 dropped, 0 orphaned, 0 out of range, 18 duplicated items
— and the save verified untouched from outside the tool afterwards, the same
way phases 4 and 5 ended.

#### Exit test — the three measurable parts PASSED; the two only a person can answer are the user's (2026-09-08)

Both configs on a full rebuild, **0 Warning(s), 0 Error(s)**. The `/clr` split
still holds in both: exactly one `/clr:nostdlib` on one of the four `CL.exe`
command lines, on `Main.cpp`, read out of the `-v:normal` log — and the new
`Core/Labels.cpp` compiled on a native one, which its own
`#ifdef _MANAGED / #error` guard makes a fact rather than a reading. Deployed to
`F:/.../237311/22191271/data/X2CashShopTool.exe`, confirmed by reading the
directory programmatically and by SHA-256 against the build output: 1,479,680
bytes, identical, and still the only `X2CashShop*` name there.

**Part 1 — `--labels` resolves exactly what the coverage table said it would.**
Every figure in *Coverage, measured rather than assumed* came back unchanged
when the tool computed it for itself:

```
strings  : 30,056 row(s) from ...\General.ess in 20 ms

--- the two standalone strings ---
  the currency the shop prints after every price : K-Ching   [ESS]   STR_ID_34
  what the client itself calls m_bFashion         : Costume   [ESS]   STR_ID_251

ITEM_TYPE       0 of 9 value(s) occurring in the catalog are the game's own text; 2 from a mapping this build compiles out; 7 this tool's
EQIP_POSITION  16 of 20 ...                                                    ; 0                                        ; 4
ITEM_GRADE      5 of 6  ...                                                    ; 0                                        ; 1

56 of the 79 entries in the label table are this tool's own wording.
```

The grades print their citation *and* the untouched row —
`STR_ID_259   the file holds "[Rare]"` — so the bracket-stripping is visible
rather than silent. All 31 sub-tabs are listed with their billing category, and
the eight the shipped dialog script names carry a second line saying so
(`the studio calls this control Tab_Wapon, out of DLG_Cash_Shop_Subpage_Fashion.lua`).

Then `general.ess` was renamed and the tool re-run. It names the file, says what
its absence costs, and **every origin flips to TOOL**:

```
strings  : *** NOT READ *** ...\General.ess
           General.ess could not be read from the game directory, so every label below is
           this tool's own wording rather than the game's - item types, equip slots, grades,
           the Costume flag and the K-Ching currency all fall back. The catalog itself is
           unaffected. Reason: the file is not there, or could not be opened for reading

*** General.ess WAS NOT READ, so EVERY LABEL BELOW IS THIS TOOL'S OWN WORDING. ***
  1     IT_WEAPON     Weapon      TOOL      6,929
  2     IT_DEFENCE    Defence     TOOL     34,083
```

`IT_DEFENCE` degrading from `Armor` to `Defence` is the fallback working: the
label is derived from the script's own name, marked TOOL, and no longer claims
`STR_ID_17818`. The file was restored and the run repeated to confirm the ESS
column came back.

**Part 3 — the layout, measured rather than eyeballed.** A new `--layout-test`
builds the real window over a *copy* of the save, shows it off-screen, and walks
the whole control tree at both sizes and in both views, complaining about any
two visible siblings whose bounds intersect, any child outside its parent's
client area, and any control squeezed to zero:

```
  900x560, technical off:    no overlapping, clipped or collapsed control anywhere in the tree
                             grid: 2,374 row(s), 170,928-pixel canvas, scrolled to the end
                             reaches 170,928   -> the last row is reachable
  1920x1080, technical off:  (same)
  900x560, technical on:     (same)
  1920x1080, technical on:   (same)

settings : ...\X2CashShopTool.ini
           wrote on -> read on; wrote off -> read off   -> the toggle survives a restart

0 complaint(s) in total across both sizes and both views.
The live save was not opened: this ran against the copy above.
```

**That probe earned its keep on its first run** — see the corrections below.

**`--db`'s report is unchanged by all of it**: 388 dropped, 0 categories reaching
no tab, 0 rows outside 1..127, 18 items sold as more than one product, prices
1..1. Phase 3's round-trip was re-run because `Core` changed under it and still
**PASSED**: all six refusals refused with the reason `CCashDb::Validate` gives,
`product_no` allocated as max+1, the edit read back, the wallet restored, the row
deleted, the catalog identical row for row. `--picker-test` still measures 4–5 ms
a keystroke over all 48,754 rows, 82 KB left behind by fetching every icon in the
catalog, and the last row reachable on a 2,145,176-pixel canvas.

The editor was launched, watched for 9 s, closed through `CloseMainWindow`
(exit 0, nothing on stderr, the clean-exit `peak working set : 54.1 MB` line
printed). **The save was verified untouched from outside the tool afterwards** —
`user_version` 11, 2,374 rows, `product_no` 1..2374, `min(price) = max(price) = 1`,
`cash_start` 999999, `integrity_check` ok, the WAL truncated to 0 bytes, and no
new `cashtool-20260908-2226*` files in `db_backup/`: the backup is still taken by
the first *write*, so opening the editor to look at something leaves it alone.

**What is left is the two halves no measurement replaces**, and the second is the
phase's actual exit test:

1. **Part 2 by eye** — open it and confirm no enum name and no source citation is
   visible anywhere in the default view: tabs, sub-tabs, rows, both picker
   filters, the edit dialog, the header, the status bar. Prices should read
   `K-Ching` and a costume piece should read `Costume`. Then `View → Technical
   details` (or Ctrl+T) and confirm all of it comes back, and that turning it off
   again leaves every item on the *must not lose* list reachable. The tool's own
   `--labels` and `--layout-test` cover the machinery; whether a *word* is the
   right word is not a thing a probe can answer.
2. **Part 4** — a person who has not read this plan opens the tool and reprices
   one product, and adds one, without asking what a word means.

#### Corrections to this plan, found by doing it

- **The layout could not be claimed by eye, and the probe caught a real clip on
  its first run.** The plan says so in advance — "Phase 5's own scroll-extent
  probe is the precedent for not trusting this by eye" — and it was right. The
  first draft of the button row was a `TableLayoutPanel` with four absolute
  columns (90 + 90 + 120 + 90 plus margins = 402 px) sitting in a grid column
  that is **360 px wide at the 900×560 minimum**. `--layout-test` reported it
  exactly:

  ```
  CLIPPED  ... / Button(Reload)  {X=310,Y=8,Width=82,Height=28}  outside  {X=0,Y=0,Width=360,Height=44}
  CLIPPED  ... / Label(showing 2,374 of 2,374)  {X=403,Y=8,Width=1,Height=28}  outside  ...
  ```

  Absolute columns in a container narrower than their sum **clip in silence** —
  the last button and the row count were simply gone, at exactly the size the
  exit test names. The fix is a wrapping, auto-sizing `FlowLayoutPanel`, which
  cannot clip because it takes a second line instead, plus moving the row count
  out of the button row into the status bar where it reads better anyway. The
  moral is the plan's own: replacing hand-rolled `Bounds` with containers removed
  *one* class of layout bug and introduced a second, and only a measurement
  distinguished "it looks fine on my 1920×1080" from "it is broken at the
  documented minimum".

- **A fallback label must drop its citation, and the first version did not.**
  Deleting `general.ess` produced this:

  ```
  the currency ... : K-Ching   [TOOL]   STR_ID_34
  1  IT_WEAPON  Weapon  TOOL  6,929
       STR_ID_270   the file holds ""
  ```

  A label marked TOOL that still cites `STR_ID_270`, and a line claiming the
  file holds an empty string it never opened. That is *precisely* the "looks
  sourced and is not" failure this phase's rule 1 exists to prevent, produced by
  the phase's own degrade path. All three fallback paths now clear `iStringID`
  and `strEssRaw`, and the printer omits the citation when there is none. **The
  degrade test is not optional and it is not only about the missing file** — it
  is the only run in which the origin machinery is exercised in its unhappy
  state, and the unhappy state was the one that lied.

- **The plan's tier-2 transcription had one wrong name.** It lists the first
  sub-tab control as `Tab_Total`; the constant pool of
  `DLG_Cash_Shop_Subpage_Fashion.lua` shows the variable is
  `g_pRadioSub_Tab_Total` but the name the script actually *sets* is
  **`Totalitem`**. The other seven are as the plan has them, `Tab_Wapon` typo
  included. Read out of the bytecode's string constants in layout order, which
  also yielded the tag each control carries — so the eight `(control, CSSC_*)`
  pairs are transcribed rather than paired by position:
  `Totalitem/CSSC_ALL`, `Tab_Wapon/CSSC_FASHION_WEAPON`,
  `Tab_Hood1/CSSC_FASHION_BODY`, `Tab_Hood2/CSSC_FASHION_LEG`,
  `Tab_Hair/CSSC_FASHION_HAIR`, `Tab_Glove/CSSC_FASHION_HAND`,
  `Tab_Shoes/CSSC_FASHION_FOOT`, `Tab_OnePiece/CSSC_FASHION_ONE_PIECE`.

- **`CASH_FIELD_MIN` / `CASH_FIELD_MAX` are an UNNAMED NATIVE enum, and those do
  not box for `String::Format`.** `String::Format( "...{0}..{1}...",
  CASH_FIELD_MIN, CASH_FIELD_MAX )` fails with C2665 and an argument list
  printed as `'(const char [162], , )'` — two blanks where the values should be.
  Phase 5 never hit it because it only ever *compared* against them. An `(int)`
  cast at the use site fixes it. This belongs beside phase 4's
  `String^ == String^` note as a C++/CLI trap the native side of this tool keeps
  handing to the managed side.

- **`Path::GetTempPath` is unusable in this TU**, because `windows.h` has
  already `#define`d `GetTempPath` to `GetTempPathW` — so the call becomes a
  member lookup for `GetTempPathW` on `System::IO::Path` and fails with C2039
  plus a baffling "function does not take 0 arguments". The layout probe's copy
  goes beside the index cache instead, which is where phase 3's round-trip
  already puts its own.

- **`--db`'s report is no longer "still 2,360 rows".** The live save now holds
  **2,374** products and 1,986 the client will show: the user has added fourteen
  since phase 5, which is the tool doing its job. The numbers the check is
  actually about are the structural ones — 388 dropped, 0 orphaned, 0 out of
  range, 18 duplicated items, prices 1..1 — and those are unchanged. A phase that
  checks a row *count* against a plan written weeks earlier is checking the wrong
  thing.

- **A public method dropped into a private section reads as C3767, not as an
  access error.** `SetTechnical` was inserted next to `DrawRow`, which is
  private, and the compiler said
  `'ItemListPanel::SetTechnical': candidate function(s) not accessible` from the
  caller — which sounds like an assembly-visibility problem and is not. Worth
  knowing before spending time on `public ref class`.

- **The heredoc backslash hazard fired a third time**, in the now-familiar way:
  a `bash` heredoc collapsed `\\r\\n` in a Python patch script to `\r\n`, which
  Python then read as a real CR LF, so the pattern did not match the C++ source's
  literal `\r\n`. It failed loudly this time (an assertion on the pattern) rather
  than silently. The fix that finally sticks: **write the patch script to a file
  with the Write tool and run it by path**, never through a heredoc. Every file
  touched by a scripted edit was re-read afterwards and checked for stray CR
  bytes and NULs.

#### Decisions made while implementing phase 6

- **The shop's sub-tab labels are this tool's wording, NOT the equip-slot
  strings — even though the pairing would have worked.** `CSSC_FASHION_BODY`
  does line up with `EP_DEFENCE_BODY`, whose shipped string is "Top Piece", and
  using it would have made seven sub-tabs read as ESS instead of TOOL. It is not
  done, because the only thing in this repo that asserts that correspondence is
  **our own offline emulator** (`X2Lib/Offline/X2OfflineCashShop.cpp:320-332`),
  so citing it as the game's word would be the tool quoting itself and calling
  the result a citation. This is the same judgement the plan already made for
  `IT_ACCESSORY` and `STR_ID_246`, applied to a case where the temptation was
  much stronger. The label text is the same either way; what differs is whether
  the origin column tells the truth.
- **The studio's control names ride in the details pane and in `--labels`, never
  as the label.** `Tab_Hood1` is no more a human name for a thing than
  `CSSC_FASHION_BODY` is, so promoting tier 2 to the primary label would have
  satisfied the letter of the phase and none of its point. The consequence is
  that **origin `SCRIPT` never appears in the origin column** — it appears as a
  line underneath a TOOL label — and `--labels` says so where it explains the
  four tags.
- **The note attached to a label is per-row, not per-enum.** The first version
  gave all 32 sub-tabs the note "with the studio's own control name beside it",
  which is true of eight of them. Two constants now, and the twenty-four without
  a control name say so. A note that overclaims in a file whose whole job is not
  overclaiming.
- **Numbers stayed everywhere they are the contract, in both views.** The
  category number is beside every sub-tab label (`Costume weapon  ->  11   (392)`),
  the item id stays on every picker row, and the details pane always shows
  `product_no / item_id / category / quantity / price / is_event` in Consolas
  whether the toggle is on or off. The category box in the edit dialog is still
  an **editable** `ComboBox` with the number as the leading token, for the reason
  phase 4 recorded: a closed list makes 128 impossible to type, and typing 128
  and watching it be refused is an exit test.
- **The plain second line uses the equip SLOT for equippable items and the item
  TYPE for the rest.** `EP_NONE` is the second-largest bucket in the catalog at
  5,862 items and its honest label is "not equipped", which is true and is noise
  on a row; those rows show what the item *is* instead. Same rule in the picker.
- **Grades are stripped of their brackets for a caption and the raw form is
  kept.** `SLabel::strEssRaw` holds `[Rare]` and `strText` holds `Rare`, and
  `--labels` prints both, so the one place this tool edits a shipped string says
  so out loud.
- **The details pane is two read-only multiline `TextBox`es, not `Label`s.** A
  `TextBox` scrolls where a `Label` clips — which is what exit test 3 asks about
  — and a `product_no` you can select is a `product_no` you can paste into a
  `sqlite3` query.
- **The event mark is demoted and the checkbox now says why.** `m_bEvent`
  appears nowhere in `X2Lib/` or `KTDXLIB/` outside `Offline/`: the emulator sets
  it (`X2OfflineCashShop.cpp:172`) and no client code reads it. Phase 5's orange
  badge gave the one field with no in-game effect top billing on every row. It is
  now small grey text, and the dialog's caption reads "mark this as an event
  product  (nothing in the game reads it)".
- **`X2CashShopTool.ini` lives in `IndexCache.cpp`, next to `DefaultCachePath()`.**
  That file already answers "where does this tool keep its state", and the ini is
  one more answer to the same question; putting it in `Labels.cpp` would have
  muddled a file whose subject is where *words* come from. It is `%LOCALAPPDATA%\
  X2CashShopTool\X2CashShopTool.ini` and deliberately **not** `ItemIndex.db`,
  which is a cache that gets thrown away whenever an archive changes.
- **No `ItemExtractorVersion()` bump and no new cache column**, per the phase's
  own rule 4 — so phase 4's `HasColumn` trap is still waiting for phase 7, which
  is the phase that actually adds columns. `General.ess` is re-read every run
  (20 ms for 30,056 rows) and never cached.
- **Every switch from phases 0–5 is untouched**, so their exit tests are still
  runnable verbatim from this exe. Three are new: `--labels`, `--technical` and
  `--layout-test`.
- **The picker's one-off cost went up and is accepted, with the number.**
  Marshalling 48,754 items went from 81 ms / 17.7 MB to **162 ms / 23.9 MB**,
  because every row now carries both spellings of its second line so the
  technical toggle is a repaint rather than a rebuild. Building those two strings
  lazily would recover most of the 6 MB, and it is not done: the cost is paid
  once, only on the first Add, and it is reported in the picker's own footer. The
  numbers the exit test is about are unchanged — 4–5 ms a keystroke, 82 KB left
  behind by a full icon sweep. Editor peak working set went from 47.8 MB to
  **54.1 MB**, most of it the 30,056-row string table.
- **`--layout-test` opens the editor over a COPY of the save**, the same
  discipline phase 3's round-trip took, because the probe has to build a real
  `MainForm` to measure one and a real `MainForm` opens its save read-write. It
  also drives `WriteSettingBool`/`ReadSettingBool` against the real ini and puts
  back whatever value it found, because "the setting persists" is a claim and the
  menu handler is one line of code away from it.
- **`ItemIndex.h`'s comment about the tab names was corrected in the same
  commit**, as the phase asked. Both halves of it were wrong: there *is* a
  localized string table and this tool reads it now, and the cash-shop tab
  captions are the one thing in the window that is not in it.

### Phase 7 — Bulk editing and CSV

Multi-select within a tab; set price or quantity across the selection; CSV
export and import of the whole catalog with a diff preview before applying.

**Exit test**: export, edit prices in a spreadsheet, re-import, and confirm the
new prices in the game — the thing that makes the flat price of 1 fixable.

### Phase 8 — Deploy and document

Ship the exe into the game directory next to `X2_offline.exe`, and write
`CASH_SHOP_TOOL.md` — what it is, how to run it, the billing category number
space, the ≤127 limits on `category` and `quantity`, and the
restart-to-see-changes rule.

Add one line to `CLAUDE.md`'s *Tooling* section listing `X2CashShopTool`
alongside the other content tools. **Do not add a `MODS.md` entry**: that file
tracks client modifications behind `SERV_IRUHADEV_` flags and their rebuild
requirements, and this tool changes no client code and requires no rebuild of
anything.

---

## Verification

There is no test suite in this repo, so verification is running the tool and
then running the game. **Deploying the exe into the game directory is not an
optional last step for phases that happen to need archives or `els_db.sql` —
it happens after every build, every phase, including phase 0**, whose probe
touches no game data at all. See point 4 under *Running a phase in a fresh
conversation* above.

```sh
TRUNK="f:/elsword stuff/.../source/EU_CN_US/Trunk"
DATA="F:/elsword stuff/elsword_2014/els_2014/237311/22191271/data"

# building the Ui project pulls Core in as a project reference
msbuild X2CashShopTool/Ui/X2CashShopTool_2010.vcxproj \
        -p:Configuration=Release -p:Platform=Win32

ls -la X2CashShopTool/Ui/Release/X2CashShopTool.exe
cp X2CashShopTool/Ui/Release/X2CashShopTool.exe "$DATA/"
cd "$DATA" && ./X2CashShopTool.exe
```

**No `SolutionDir` is needed**, deliberately: every include path is written
`..\..\` relative, so the project builds standalone. That is the one thing
this tool does differently from the client build, and it is on purpose — see
`CLAUDE.md` on the trailing-slash footgun.

Deployment needs only `msvcr100.dll`, `msvcp100.dll` and .NET 4.0; VS2010
statically links the managed-C++ support runtime, so there is no `msvcm100.dll`
to find.

The tool must be run **with the game directory as its working directory**, for
the same reason the client is: the archives are found relative to `.`, and
`els_db.sql` lives there.

Checking a change actually landed:

```sh
cd "$DATA"
python -c "import sqlite3; c=sqlite3.connect('els_db.sql'); \
  print(list(c.execute('select category, count(*), min(price), max(price) \
  from cash_product group by category order by category')))"
```

Then launch `start_offline.bat`, open the cash shop, and confirm the tab
contents and prices. Because `CX2OfflineCashShop::EnsureLoaded` reads the table
once per process, **the game must be restarted** for an edit to show — a change
that appears not to have worked is almost always this.

Watch `offline_server.log` for the catalog line, which reports both the row
count and how many rows were dropped for naming an item the client cannot
resolve:

```sh
grep "^CASH" offline_server.log
```
