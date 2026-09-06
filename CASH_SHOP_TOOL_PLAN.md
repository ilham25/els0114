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
    DdsDecode.{h,cpp}                  DXT1/DXT5 -> BGRA32 (nothing in-tree does this)
    CashDb.{h,cpp}                     sqlite3 over els_db.sql
    IndexCache.{h,cpp}                 the SQLite item index, next to the tool exe
    ../../Libs/ExternalLib/sqlite3/sqlite3.c   (X2Lib_2010.vcxproj:3133-3138 verbatim)

  Ui/X2CashShopTool_2010.vcxproj       Application. CLRSupport=true,
                                       TargetFrameworkVersion=v4.0, /MD, v100.
    Main.cpp                           [STAThread] int main(array<String^>^)
    MainForm.h                         tabs, product grid, wallet, bulk actions
    ItemPickerForm.h                   virtualized picker + search
    EditProductForm.h                  category / price / quantity / event
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

-- icons are a LOCATOR table; never the bytes, never decoded bitmaps
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
3. **Insert** opens the picker over the whole ~43k-item catalog in a VirtualMode
   `ListView` — only visible rows are ever materialised, so it neither lags nor
   grows without bound. The search box filters on name and on item ID.
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
  `Console::OutputEncoding = Encoding::UTF8`. Worth knowing for phase 6's CSV.
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

### Phase 4 — The main window

Tabs and sub-tabs from the parsed category table; the product grid with icon,
name, price, quantity, event badge; Edit and Delete; the wallet box; the "All"
tab and its report of orphaned categories.

**Exit test**: every tab matches what the game shows for the same category, and
one product edited in the tool appears changed in the game after a restart.
Entering 128 or 0 in a quantity or category cell is refused, not clamped.

### Phase 5 — Insert, with the virtualized picker

`InsertItemForm`: VirtualMode `ListView` over the whole catalog, search by name
and id, optional filters by item type and equip slot; then the field form.

**Exit test**: the picker opens instantly and scrolls the full ~43k rows without
stutter or growth in memory; an item that was never purchasable is inserted and
is then buyable in the game.

### Phase 6 — Bulk editing and CSV

Multi-select within a tab; set price or quantity across the selection; CSV
export and import of the whole catalog with a diff preview before applying.

**Exit test**: export, edit prices in a spreadsheet, re-import, and confirm the
new prices in the game — the thing that makes the flat price of 1 fixable.

### Phase 7 — Deploy and document

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
