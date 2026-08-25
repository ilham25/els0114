# Local modifications

The `import-2014` commit is **not** stock 2014 code. It is stock code plus the
four changes below, which were already applied to the working copy before git
existed here. No pristine copy exists on disk, so the import could not be split
into a vanilla baseline plus mods.

This file is the substitute for that baseline. Keep it current.

## Applied in `import-2014`

| Flag | Defined in | Client files | Server files |
|---|---|---|---|
| `SERV_IRUHADEV_SKILLTREE_NO_LOCK` | `KTDXLIB/Always.h:2414`, `KncWX2Server/Common/ServerDefine.h:4210` | `X2Lib/X2UISkillTreeNew.cpp:5788` | `KncWX2Server/GameServer/UserSkillTree.cpp:1489` |
| `SERV_IRUHADEV_SKILL_SLOT_B_FREE` | `KTDXLIB/Always.h:2421`, `KncWX2Server/Common/ServerDefine.h:4217` | `X2Lib/X2UISkillTree.h:228`, `X2Lib/X2UISkillTreeNew.h:269`, `X2Lib/X2UserSkillTree.h:307`, `X2Lib/X2UserSkillTree.cpp:2078` | `KncWX2Server/GameServer/UserSkillTree.cpp` (lines 10, 766, 978) |
| `SERV_IRUHADEV_BUFF_DURATION_TEXT` | `KTDXLIB/Always.h:2428` | `X2Lib/X2GageUI.{h,cpp}`, `X2Lib/X2BuffTemplet.{h,cpp}`, `X2Lib/X2BuffFinalizerTemplet.h`, `X2Lib/X2GameUnit.cpp`, `X2Lib/X2PremiumBuffManager.cpp` | -- |
| `STATIC_AUTO_LOGIN` | `KTDXLIB/OnlyGlobal/Always_US.h:358` | `X2Lib/X2Main.cpp:1255`, `X2Lib/X2Main.cpp:1285` | -- |

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

## Reverting to stock

Comment out the five `#define`s listed in the *Defined in* column and rebuild.
No git operation is needed -- the flag system is the revert switch. That is the
main reason a reconstructed vanilla baseline was not worth building.

## Rebuild requirements

`SERV_IRUHADEV_SKILLTREE_NO_LOCK` and `SERV_IRUHADEV_SKILL_SLOT_B_FREE` both
touch `KncWX2Server/Common/ServerDefine.h`, which compiles into **both** sides.
Toggling either one requires rebuilding the VS2010 client *and* the five VS2003
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
