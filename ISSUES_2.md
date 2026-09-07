issue:
- equipment from quest by data is in the inventory, but it dont directly appear on inventory so i need to back to char list and select my char again to make the item appear in inventory
- the dungeon quest that 'any difficulty' have an issue because i can only progress through it with normal dungeon
- when choosing a character and back to character list screen, the choosed character pvp emblem back to a small black box
- result screen alway F rank for all unit
- when opening a "box" my ED get emptied (even though the real data isn't, the state is)
- pet fetch aura status not persist
- i need you to cover all "box" item (the item that need to be opened) functionality because right now i feel like it's half baked. dont make assumption and refer on how the real server behaviour does (i can access the real live database data when you need something)
  -> planned as phases 36-45 in OFFLINE_MODE_PHASE36_PLAN.md.
     "box" is three routes, not one; most boxes go through EGS_USE_ITEM_IN_INVENTORY_REQ, not the cube path.
     Philosopher's Scroll = item 160267 (instant one level up) = phase 36, first.
