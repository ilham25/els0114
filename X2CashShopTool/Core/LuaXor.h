//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-06
// Description: X2CashShopTool phase 1 - in-place XOR for packed .lua
// scripts. See CASH_SHOP_TOOL_PLAN.md section 5.
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <stddef.h>

// Undoes (or applies - it is symmetric) the XOR every packed .lua carries.
// Works in place over the buffer given.
//
// Deliberately not KTDXCommonFunc.h's XORDecrypt nor X2KomFileViewer's
// FileCrypt.h: both accumulate the plaintext into a std::string four bytes
// at a time and then copy it out, which for Item.lua's 34 MB is ~8.5 million
// appends and a transient peak around 140 MB. Those two are the spec this
// was written from; the behaviour, tail included, matches them exactly.
void XorLuaScript( void* pData, size_t uSize );
