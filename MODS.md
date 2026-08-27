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
| `SERV_IRUHADEV_SKILLTREE_NO_LOCK` | import-2014 | `KTDXLIB/Always.h:2414`, `KncWX2Server/Common/ServerDefine.h:4210` | `X2Lib/X2UISkillTreeNew.cpp:5788` | `KncWX2Server/GameServer/UserSkillTree.cpp:1489` |
| `SERV_IRUHADEV_SKILL_SLOT_B_FREE` | import-2014 | `KTDXLIB/Always.h:2421`, `KncWX2Server/Common/ServerDefine.h:4217` | `X2Lib/X2UISkillTree.h:228`, `X2Lib/X2UISkillTreeNew.h:269`, `X2Lib/X2UserSkillTree.h:307`, `X2Lib/X2UserSkillTree.cpp:2078` | `KncWX2Server/GameServer/UserSkillTree.cpp` (lines 10, 766, 978) |
| `SERV_IRUHADEV_BUFF_DURATION_TEXT` | import-2014 | `KTDXLIB/Always.h:2428` | `X2Lib/X2GageUI.{h,cpp}`, `X2Lib/X2BuffTemplet.{h,cpp}`, `X2Lib/X2BuffFinalizerTemplet.h`, `X2Lib/X2GameUnit.cpp`, `X2Lib/X2PremiumBuffManager.cpp` | -- |
| `STATIC_AUTO_LOGIN` | import-2014 | `KTDXLIB/OnlyGlobal/Always_US.h:358` | `X2Lib/X2Main.cpp:1255`, `X2Lib/X2Main.cpp:1285` | -- |
| `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` | 2026-08-27 | `KTDXLIB/Always.h:2435`, `KncWX2Server/Common/ServerDefine.h:4224` | `X2Lib/X2Unit.h:982-988`, `X2Lib/X2Unit.cpp:104-111`, `X2Lib/X2UIQuickSlot.cpp:1797-1822`, `X2Lib/X2CashShop.cpp:8306-8326` | `KncWX2Server/GameServer/Inventory.cpp:500-534`, `KncWX2Server/GameServer/Inventory.cpp:146-152` |

What each one does:

- **`SERV_IRUHADEV_SKILLTREE_NO_LOCK`** -- both skills in a 2-choice skill tree
  row can be learned; the unchosen one is no longer locked out.
- **`SERV_IRUHADEV_SKILL_SLOT_B_FREE`** -- Skill Slot B is open by default and
  permanent. No medal purchase, and the server keeps the permanent-sentinel end
  date rather than expiring it.
- **`SERV_IRUHADEV_BUFF_DURATION_TEXT`** -- draws a remaining-seconds countdown
  over each buff/debuff icon on the status HUD.
- **`STATIC_AUTO_LOGIN`** -- skips the login screen and signs in with a fixed
  account, overriding whatever the publisher switch decided.
- **`SERV_IRUHADEV_QUICK_SLOT_FULL_FREE`** -- all 6 equipped consumable quick
  slots are open from character creation. No cash ticket, no `LOCK` overlay,
  and the Quick Slot Expansion ticket (item 244560, plus the Ara/Elesis
  variants) is hidden from the cash shop. The server pins inventory category
  11 (`ST_E_QUICK_SLOT`) to exactly 6 slots on load, so a legacy `+3` purchase
  row already sitting in `dbo.GItemInventorySize` no longer stacks on top of
  the new base.

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

`SERV_IRUHADEV_BUFF_DURATION_TEXT` and `STATIC_AUTO_LOGIN` are client-only.

## Known deviations to clean up

`STATIC_AUTO_LOGIN` breaks the project rule in CLAUDE.md two ways:

1. It is not `SERV_IRUHADEV_`-prefixed.
2. It lives in `KTDXLIB/OnlyGlobal/Always_US.h` instead of `KTDXLIB/Always.h`.
   That file is only pulled in for `CLIENT_COUNTRY_US`, so the change is
   silently US-only and will not survive a look at any other region config.

It also stores a plaintext account password in a header that compiles into the
shipped executable. Acceptable for a local-only repo; worth moving to an
untracked local header if this ever gets pushed anywhere.
