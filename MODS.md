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
| `STATIC_AUTO_LOGIN` | import-2014 | `KTDXLIB/OnlyGlobal/Always_US.h:358` | `X2Lib/X2Main.cpp:1255`, `X2Lib/X2Main.cpp:1285` | -- |
| `SERV_IRUHADEV_QUICK_SLOT_FULL_FREE` | 2026-08-27 | `KTDXLIB/Always.h:2435`, `KncWX2Server/Common/ServerDefine.h:4224` | `X2Lib/X2Unit.h:982-988`, `X2Lib/X2Unit.cpp:104-111`, `X2Lib/X2UIQuickSlot.cpp:1797-1822`, `X2Lib/X2CashShop.cpp:8306-8326` | `KncWX2Server/GameServer/Inventory.cpp:500-534`, `KncWX2Server/GameServer/Inventory.cpp:146-152` |
| `SERV_IRUHADEV_OFFLINE` | 2026-08-31 | `KTDXLIB/Always.h:2446` **and** `X2ServerProtocol/X2ServerProtocol_2010.vcxproj:1444` (`US_SERVICE` `PreprocessorDefinitions`) | `X2Lib/Offline/` (the whole directory: 44 sources plus `start_offline.bat`), plus seams in `X2ServerProtocol/Socket/Session.cpp` (5 blocks), `X2ServerProtocol/OfflineHook.h`, `X2Lib/X2Data.cpp:2136`, `X2Lib/X2StateServerSelect.cpp:6982`, `X2Lib/X2DungeonSubStage.cpp:1452`, `X2Lib/X2QuestManager.{h,cpp}`, `X2Lib/X2TitleManager.h:339` | -- |
| `SERV_IRUHADEV_NO_PATCHER_TOKEN` | 2026-09-04 | `KTDXLIB/Always.h:2525` | `X2/X2.cpp:805` | -- |

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
- **`STATIC_AUTO_LOGIN`** -- skips the login screen and signs in with a fixed
  account, overriding whatever the publisher switch decided.
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

The remaining four -- `SERV_IRUHADEV_BUFF_DURATION_TEXT`,
`STATIC_AUTO_LOGIN`, `SERV_IRUHADEV_OFFLINE` and
`SERV_IRUHADEV_NO_PATCHER_TOKEN` -- are client-only. None of them touches
anything under `KncWX2Server/Common/`, so the servers never need rebuilding
for any of them. Two caveats:

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

`STATIC_AUTO_LOGIN` breaks the project rule in CLAUDE.md two ways:

1. It is not `SERV_IRUHADEV_`-prefixed.
2. It lives in `KTDXLIB/OnlyGlobal/Always_US.h` instead of `KTDXLIB/Always.h`.
   That file is only pulled in for `CLIENT_COUNTRY_US`, so the change is
   silently US-only and will not survive a look at any other region config.

It also stores a plaintext account password in a header that compiles into the
shipped executable. Acceptable for a local-only repo; worth moving to an
untracked local header if this ever gets pushed anywhere.
