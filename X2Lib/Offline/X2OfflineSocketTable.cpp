#include "stdafx.h"
#include "X2OfflineSocketTable.h"

#ifdef SERV_IRUHADEV_OFFLINE

#include "X2OfflineLog.h"
#include "X2OfflineLuaEnum.h"
#include "X2OfflineInventory.h"
#include "X2OfflineEnchantTable.h"

CX2OfflineSocketTable* CX2OfflineSocketTable::ms_pInstance = NULL;

namespace
{
	const wchar_t* const SCRIPT_SOCKET_TABLE = L"SocketItemTable.lua";

	/// CXSLSocketItem::GetSocketDataType (XSLSocketItem.cpp:211), flattened.
	///
	/// Every case in that switch has the same shape - one socket data type if
	/// the target is a weapon, another for anything else - so it is a table
	/// here rather than a hundred-case switch. Extracted mechanically with this
	/// build's flag set applied (SERV_NEW_DUAL_MAGIC_STONE, SERV_2012_PVP_SEASON2,
	/// SERV_FIXED_EFFECTS_SOCKET_ITEM, SERV_FISHING_HOLE_MAGIC_STONE,
	/// SERV_MISS_ELIOS_STONE_ITEM and SERV_BC_EVENT on; SERV_LURIEL_MAGIC_STONE
	/// is not defined anywhere in KncWX2Server/Common, so its stones are out).
	/// The values are the CLIENT's CX2SocketItem::SOCKET_DATA_TYPE, which is
	/// the server's enum name for name and value for value.
	///
	/// Two cross-checks passed on the way in. These 58 stones are exactly the
	/// set CXSLSocketItem::IsMagicStoneItemID accepts, extracted the same way -
	/// so nothing here can be socketed that the live server would have refused,
	/// and nothing it accepted is missing. And every SOCKET_DATA_TYPE name the
	/// packed SocketItemTable.lua subscripts resolves in the client enum.
	///
	/// The last six rows look wrong and are not: the fixed-effect stones give a
	/// weapon the 5%/10% option and armour the 2.5%/5% one, and two different
	/// event stones both map to the Sage pool. Transcribed, not tidied.
	struct KStoneRow
	{
		int	m_iMagicStoneItemID;
		int	m_iWeaponType;
		int	m_iDefenceType;
	};

	const KStoneRow STONE_TABLE[] =
	{
		{ 109945    ,   1,   3 },	///< MSI_MAGIC_STONE -> SDT_WEAPON_NORMAL / SDT_DEFENCE_NORMAL
		{ 109955    ,   2,   4 },	///< MSI_RARE_MAGIC_STONE -> SDT_WEAPON_RARE / SDT_DEFENCE_RARE
		{ 132110    ,   8,   9 },	///< MSI_HARMONY_MAGIC_STONE -> SDT_WEAPON_HARMONY / SDT_DEFENCE_HARMONY
		{ 132111    ,  10,  11 },	///< MSI_MYSTERY_MAGIC_STONE -> SDT_WEAPON_MYSTERY / SDT_DEFENCE_MYSTERY
		{ 132112    ,  12,  13 },	///< MSI_SHINE_MAGIC_STONE -> SDT_WEAPON_SHINE / SDT_DEFENCE_SHINE
		{ 132113    ,  14,  15 },	///< MSI_BATTLE_MAGIC_STONE -> SDT_WEAPON_BATTLE / SDT_DEFENCE_BATTLE
		{ 132114    ,  16,  17 },	///< MSI_INCREASE_MAGIC_STONE -> SDT_WEAPON_INCREASE / SDT_DEFENCE_INCREASE
		{ 132115    ,  18,  19 },	///< MSI_HIT_MAGIC_STONE -> SDT_WEAPON_HIT / SDT_DEFENCE_HIT
		{ 132193    ,  20,  21 },	///< MSI_ATK_PHYSIC_MAGIC_STONE -> SDT_WEAPON_ATK_PHYSIC / SDT_DEFENCE_ATK_PHYSIC
		{ 132194    ,  22,  23 },	///< MSI_ATK_MAGIC_MAGIC_STONE -> SDT_WEAPON_ATK_MAGIC / SDT_DEFENCE_ATK_MAGIC
		{ 132195    ,  24,  25 },	///< MSI_DEF_PHYSIC_MAGIC_STONE -> SDT_WEAPON_DEF_PHYSIC / SDT_DEFENCE_DEF_PHYSIC
		{ 132196    ,  26,  27 },	///< MSI_DEF_MAGIC_MAGIC_STONE -> SDT_WEAPON_DEF_MAGIC / SDT_DEFENCE_DEF_MAGIC
		{ 132197    ,  28,  29 },	///< MSI_HYPER_TIME_MAGIC_STONE -> SDT_WEAPON_HYPER_TIME / SDT_DEFENCE_HYPER_TIME
		{ 132198    ,  30,  31 },	///< MSI_INCREASE_HP_MAGIC_STONE -> SDT_WEAPON_INCREASE_HP / SDT_DEFENCE_INCREASE_HP
		{ 132199    ,  32,  33 },	///< MSI_HYPER_CHARGE_MAGIC_STONE -> SDT_WEAPON_HYPER_CHARGE / SDT_DEFENCE_HYPER_CHARGE
		{ 132200    ,  34,  35 },	///< MSI_SPEED_MAGIC_STONE -> SDT_WEAPON_SPEED / SDT_DEFENCE_SPEED
		{ 132201    ,  36,  37 },	///< MSI_CRITICAL_MAGIC_STONE -> SDT_WEAPON_CRITICAL / SDT_DEFENCE_CRITICAL
		{ 132202    ,  38,  39 },	///< MSI_MOVE_SPEED_MAGIC_STONE -> SDT_WEAPON_MOVE_SPEED / SDT_DEFENCE_MOVE_SPEED
		{ 132203    ,  40,  41 },	///< MSI_STRIKE_MAGIC_STONE -> SDT_WEAPON_STRIKE / SDT_DEFENCE_STRIKE
		{ 132204    ,  42,  43 },	///< MSI_AVOID_MAGIC_STONE -> SDT_WEAPON_AVOID / SDT_DEFENCE_AVOID
		{ 132205    ,  44,  45 },	///< MSI_JUMP_SPEED_MAGIC_STONE -> SDT_WEAPON_JUMP_SPEED / SDT_DEFENCE_JUMP_SPEED
		{ 72510     ,  46,  47 },	///< MSI_INCREASE_DAMEGE_STONE -> SDT_WEAPON_ADD_DAMAGE / SDT_DEFENCE_ADD_DAMAGE
		{ 72520     ,  48,  49 },	///< MSI_DECREASE_DAMEGE_STONE -> SDT_WEAPON_ADD_DAMAGE_REDUCE / SDT_DEFENCE_ADD_DAMAGE_REDUCE
		{ 111701    ,  50,  51 },	///< MSI_DESTROY_MAGIC_STONE -> SDT_WEAPON_DESTROY / SDT_DEFENCE_DESTROY
		{ 111702    ,  52,  53 },	///< MSI_EXPLOSION_MAGIC_STONE -> SDT_WEAPON_EXPLOSION / SDT_DEFENCE_EXPLOSION
		{ 111703    ,  54,  55 },	///< MSI_BURN_MAGIC_STONE -> SDT_WEAPON_BURN / SDT_DEFENCE_BURN
		{ 111704    ,  56,  57 },	///< MSI_QUICK_MAGIC_STONE -> SDT_WEAPON_QUICK / SDT_DEFENCE_QUICK
		{ 111705    ,  58,  59 },	///< MSI_FURY_MAGIC_STONE -> SDT_WEAPON_FURY / SDT_DEFENCE_FURY
		{ 135184    ,  60,  61 },	///< MSI_WISE_MAN_STONE_ITEM -> SDT_WEAPON_SAGE / SDT_DEFENCE_SAGE
		{ 98004     ,  72,  73 },	///< MSI_MOVE_SPEED_PVP -> SDT_WEAPON_MOVE_SPEED_PVP / SDT_DEFENCE_MOVE_SPEED_PVP
		{ 98005     ,  78,  79 },	///< MSI_JUMP_SPEED_PVP -> SDT_WEAPON_JUMP_SPEED_PVP / SDT_DEFENCE_JUMP_SPEED_PVP
		{ 98006     ,  64,  65 },	///< MSI_INCREASE_HP_PVP -> SDT_WEAPON_INCREASE_HP_PVP / SDT_DEFENCE_INCREASE_HP_PVP
		{ 98007     ,  74,  75 },	///< MSI_STRIKE_PVP -> SDT_WEAPON_STRIKE_PVP / SDT_DEFENCE_STRIKE_PVP
		{ 98008     ,  76,  77 },	///< MSI_AVOID_PVP -> SDT_WEAPON_AVOID_PVP / SDT_DEFENCE_AVOID_PVP
		{ 98009     ,  66,  67 },	///< MSI_HYPER_CHARGE_PVP -> SDT_WEAPON_HYPER_CHARGE_PVP / SDT_DEFENCE_HYPER_CHARGE_PVP
		{ 98010     ,  62,  63 },	///< MSI_HYPER_TIME_PVP -> SDT_WEAPON_HYPER_TIME_PVP / SDT_DEFENCE_HYPER_TIME_PVP
		{ 98011     ,  70,  71 },	///< MSI_CRITICAL_PVP -> SDT_WEAPON_CRITICAL_PVP / SDT_DEFENCE_CRITICAL_PVP
		{ 98012     ,  68,  69 },	///< MSI_SPEED_PVP -> SDT_WEAPON_SPEED_PVP / SDT_DEFENCE_SPEED_PVP
		{ 98013     ,  80,  81 },	///< MSI_ADD_DAMAGE_PVP -> SDT_WEAPON_ADD_DAMAGE_PVP / SDT_DEFENCE_ADD_DAMAGE_PVP
		{ 98014     ,  82,  83 },	///< MSI_ADD_DAMAGE_REDUCE_PVP -> SDT_WEAPON_ADD_DAMAGE_REDUCE_PVP / SDT_DEFENCE_ADD_DAMAGE_REDUCE_PVP
		{ 98015     ,  86,  87 },	///< MSI_EXPLOSION_PVP -> SDT_WEAPON_EXPLOSION_PVP / SDT_DEFENCE_EXPLOSION_PVP
		{ 98016     ,  84,  85 },	///< MSI_DESTROY_PVP -> SDT_WEAPON_DESTROY_PVP / SDT_DEFENCE_DESTROY_PVP
		{ 98017     ,  90,  91 },	///< MSI_QUICK_PVP -> SDT_WEAPON_QUICK_PVP / SDT_DEFENCE_QUICK_PVP
		{ 98018     ,  88,  89 },	///< MSI_BURN_PVP -> SDT_WEAPON_BURN_PVP / SDT_DEFENCE_BURN_PVP
		{ 98019     ,  92,  93 },	///< MSI_FURY_PVP -> SDT_WEAPON_FURY_PVP / SDT_DEFENCE_FURY_PVP
		{ 98028     ,  97,  96 },	///< MSI_SPEED_MAGIC_STONE_FIXED_EFFECT -> SDT_WEAPON_SPEED_5P_FIXED / SDT_DEFFENCE_SPEED_2P5_FIXED
		{ 98029     , 101, 100 },	///< MSI_CRITICAL_MAGIC_STONE_FIXED_EFFECT -> SDT_WEAPON_CRITICAL_5P_FIXED / SDT_DEFFENCE_CRITICAL_2P5_FIXED
		{ 98032     , 105, 104 },	///< MSI_AVOID_MAGIC_STONE_FIXED_EFFECT -> SDT_WEAPON_AVOID_5P_FIXED / SDT_DEFFECNE_AVOID_2P5_FIXED
		{ 98030     , 109, 108 },	///< MSI_INCREASE_DAMEGE_STONE_FIXED_EFFECT -> SDT_WEAPON_ADD_DAMAGE_5P_FIXED / SDT_DEFFENCE_ADD_DAMAGE_2P5_FIXED
		{ 98031     , 117, 116 },	///< MSI_HYPER_CHARGE_MAGIC_STONE_FIXED_EFFECT -> SDT_WEAPON_HYPER_CHARGE_10P_FIXED / SDT_DEFFENCE_HYPER_CHARGE_5P_FIXED
		{ 98033     , 113, 112 },	///< MSI_DECREASE_DAMEGE_STONE_FIXED_EFFECT -> SDT_WEAPON_DAMAGE_REDUCE_5P_FIXED / SDT_DEFFECNE_DAMAGE_REDUCE_2P5_FIXED
		{ 60004121  ,  50,  51 },	///< MSI_EVENT_MAGIC_STONE_1 -> SDT_WEAPON_DESTROY / SDT_DEFENCE_DESTROY
		{ 60004122  ,  52,  53 },	///< MSI_EVENT_MAGIC_STONE_2 -> SDT_WEAPON_EXPLOSION / SDT_DEFENCE_EXPLOSION
		{ 60004123  ,  54,  55 },	///< MSI_EVENT_MAGIC_STONE_3 -> SDT_WEAPON_BURN / SDT_DEFENCE_BURN
		{ 60004124  ,  56,  57 },	///< MSI_EVENT_MAGIC_STONE_4 -> SDT_WEAPON_QUICK / SDT_DEFENCE_QUICK
		{ 60004125  ,  58,  59 },	///< MSI_EVENT_MAGIC_STONE_5 -> SDT_WEAPON_FURY / SDT_DEFENCE_FURY
		{ 67003750  ,  60,  61 },	///< MSI_MISS_ELIOS_STONE_ITEM -> SDT_WEAPON_SAGE / SDT_DEFENCE_SAGE
		{ 60005473  ,  60,  61 },	///< MSI_VC_EVENT_STONE_ITEM -> SDT_WEAPON_SAGE / SDT_DEFENCE_SAGE
	};
}

//////////////////////////////////////////////////////////////////////////

CX2OfflineSocketTable::CX2OfflineSocketTable()
: m_bLoadAttempted( false )
, m_bLoaded( false )
, m_iRandomRows( 0 )
, m_iGroupRows( 0 )
{
}

CX2OfflineSocketTable::~CX2OfflineSocketTable()
{
}

CX2OfflineSocketTable* CX2OfflineSocketTable::Instance()
{
	if( NULL == ms_pInstance )
		ms_pInstance = new CX2OfflineSocketTable;

	return ms_pInstance;
}

void CX2OfflineSocketTable::Release()
{
	if( NULL == ms_pInstance )
		return;

	delete ms_pInstance;
	ms_pInstance = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading

void CX2OfflineSocketTable::EnsureLoaded()
{
	if( true == m_bLoadAttempted )
		return;

	m_bLoadAttempted = true;

	if( NULL == g_pKTDXApp ||
		NULL == g_pKTDXApp->GetLuaBinder() ||
		NULL == g_pKTDXApp->GetDeviceManager() ||
		NULL == g_pKTDXApp->GetDeviceManager()->GetMassFileManager() )
	{
		CX2OfflineLog::Server( L"SOCKET   ERROR engine not ready - socketing will refuse." );
		return;
	}

	lua_State* pLuaState = g_pKTDXApp->GetLuaBinder()->GetLuaState();
	if( NULL == pLuaState )
	{
		CX2OfflineLog::Server( L"SOCKET   ERROR no lua state - socketing will refuse." );
		return;
	}

	// SocketItemTable.lua subscripts SOCKET_DATA_TYPE["SDT_WEAPON_NORMAL"] and
	// 116 siblings.
	if( false == CX2OfflineLuaEnum::Publish() )
	{
		CX2OfflineLog::Server( L"SOCKET   ERROR could not publish SOCKET_DATA_TYPE -"
			L" socketing will refuse." );
		return;
	}

	lua_tinker::class_add< CX2OfflineSocketTable >( pLuaState, "CX2OfflineSocketTable" );
	lua_tinker::class_def< CX2OfflineSocketTable >( pLuaState, "AddSocketRandomData",
		&CX2OfflineSocketTable::AddSocketRandomData_LUA );
	lua_tinker::class_def< CX2OfflineSocketTable >( pLuaState, "AddSocketGroup",
		&CX2OfflineSocketTable::AddSocketGroup_LUA );

	// Borrow g_pCX2SocketItem for the load and give it straight back - see the
	// header. The client's own object owns this name for the rest of the
	// process, and it publishes it only from OpenScriptFile at start-up.
	lua_tinker::decl( pLuaState, "g_pCX2SocketItem", this );

	KGCMassFileManager::CMassFile::MASSFILE_MEMBERFILEINFO_POINTER kInfo;
	kInfo = g_pKTDXApp->GetDeviceManager()->GetMassFileManager()->LoadDataFile( SCRIPT_SOCKET_TABLE );

	bool bRan = false;

	// NOT `NULL == kInfo`. MASSFILE_MEMBERFILEINFO_POINTER is a STRUCT BY
	// VALUE with an `operator const MASSFILE_MEMBERFILEINFO*() const` that
	// returns `this` (KGCMassFileManager.h), so comparing it against NULL
	// takes the address of a local and is never true. A missing file then
	// falls through to the DoMemory calls and is reported as "failed to run"
	// instead of "not packed" - which is the one diagnosis this branch
	// exists to give. The client's own loader tests the payload
	// (X2ItemManager.cpp:231); so does this.
	if( NULL == kInfo->pRealData || kInfo->size <= 0 )
	{
		CX2OfflineLog::Server(
			L"SOCKET   ERROR '%s' not found in any .kom or on disk - socketing is OFF.",
			SCRIPT_SOCKET_TABLE );
		CX2OfflineLog::Server(
			L"SOCKET   XOR-encrypt KncWX2Server/ServerResource/US/%s and pack it into data036.kom.",
			SCRIPT_SOCKET_TABLE );
	}
	else
	{
		bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemory( kInfo->pRealData, kInfo->size ) );

		if( false == bRan || m_iGroupRows <= 0 )
		{
			bRan = ( E_FAIL != g_pKTDXApp->GetLuaBinder()->DoMemoryNotEncript(
						kInfo->pRealData, kInfo->size ) );

			if( true == bRan && m_iGroupRows > 0 )
			{
				CX2OfflineLog::Server(
					L"SOCKET   NOTE '%s' is NOT encrypted - loaded as plaintext. Fine for"
					L" testing; encrypt it to match every other packed script.",
					SCRIPT_SOCKET_TABLE );
			}
		}
	}

	// Give the global back before any early return below.
	if( NULL != g_pData && NULL != g_pData->GetSocketItem() )
	{
		lua_tinker::decl( pLuaState, "g_pCX2SocketItem", g_pData->GetSocketItem() );
	}
	else
	{
		CX2OfflineLog::Server( L"SOCKET   WARNING could not restore g_pCX2SocketItem -"
			L" g_pData->GetSocketItem() was NULL." );
	}

	if( false == bRan || m_iRandomRows <= 0 || m_iGroupRows <= 0 )
	{
		CX2OfflineLog::Server( L"SOCKET   ERROR '%s' produced %d random row(s) and %d group"
			L" row(s) - socketing is OFF.", SCRIPT_SOCKET_TABLE, m_iRandomRows, m_iGroupRows );
		return;
	}

	m_bLoaded = true;

	CX2OfflineLog::Server( L"SOCKET   loaded: %d random row(s) over %u pool(s),"
		L" %d group row(s) over %u group(s), %u magic stone(s) known",
		m_iRandomRows, (unsigned int)m_mapSocketRandom.size(),
		m_iGroupRows, (unsigned int)m_mapSocketGroup.size(),
		(unsigned int)( sizeof( STONE_TABLE ) / sizeof( STONE_TABLE[0] ) ) );
}

//////////////////////////////////////////////////////////////////////////
// reads

bool CX2OfflineSocketTable::IsLoaded()
{
	EnsureLoaded();
	return m_bLoaded;
}

/*static*/ int CX2OfflineSocketTable::SocketDataType( int iMagicStoneItemID, int iItemType )
{
	for( int i = 0; i < (int)( sizeof( STONE_TABLE ) / sizeof( STONE_TABLE[0] ) ); ++i )
	{
		if( STONE_TABLE[i].m_iMagicStoneItemID != iMagicStoneItemID )
			continue;

		// The real switch tests IT_WEAPON and takes the defence branch for
		// everything else, accessories included - not an if/else over two
		// item types.
		return ( CX2Item::IT_WEAPON == iItemType ) ? STONE_TABLE[i].m_iWeaponType
												   : STONE_TABLE[i].m_iDefenceType;
	}

	return (int)CX2SocketItem::SDT_NONE;
}

bool CX2OfflineSocketTable::IsMagicStoneItemID( int iItemID )
{
	for( int i = 0; i < (int)( sizeof( STONE_TABLE ) / sizeof( STONE_TABLE[0] ) ); ++i )
	{
		if( STONE_TABLE[i].m_iMagicStoneItemID == iItemID )
			return true;
	}

	return false;
}

/*static*/ int CX2OfflineSocketTable::GetSocketCount( int iItemGrade, int iItemType )
{
	const bool bIsWeapon = ( CX2Item::IT_WEAPON == iItemType );

	switch( iItemGrade )
	{
	case CX2Item::IG_LOW:
		return bIsWeapon ? (int)CX2SocketItem::SC_WEAPON_LOW    : (int)CX2SocketItem::SC_DEFENCE_LOW;

	case CX2Item::IG_NORMAL:
		return bIsWeapon ? (int)CX2SocketItem::SC_WEAPON_NORMAL : (int)CX2SocketItem::SC_DEFENCE_NORMAL;

	case CX2Item::IG_RARE:
		return bIsWeapon ? (int)CX2SocketItem::SC_WEAPON_RARE   : (int)CX2SocketItem::SC_DEFENCE_RARE;

	case CX2Item::IG_ELITE:
		return bIsWeapon ? (int)CX2SocketItem::SC_WEAPON_ELITE  : (int)CX2SocketItem::SC_DEFENCE_ELITE;

	case CX2Item::IG_UNIQUE:
		return bIsWeapon ? (int)CX2SocketItem::SC_WEAPON_UNIQUE : (int)CX2SocketItem::SC_DEFENCE_UNIQUE;
	}

	return 0;
}

bool CX2OfflineSocketTable::GetResultItem( int iMagicStoneItemID, int iItemType, int iUseLevel,
										   bool bFashion, int iEnchantLevel,
										   OUT int& iResultSocketOption )
{
	EnsureLoaded();

	// 1. Which pool does this stone draw from?
	const int eSocketDataType = SocketDataType( iMagicStoneItemID, iItemType );

	if( (int)CX2SocketItem::SDT_NONE == eSocketDataType )
	{
		CX2OfflineLog::Server( L"SOCKET   item %d is not a magic stone this build accepts",
			iMagicStoneItemID );
		return false;
	}

	// 2/3. Roll a group out of that pool.
	std::map< int, KProbRow >::const_iterator mit = m_mapSocketRandom.find( eSocketDataType );
	if( m_mapSocketRandom.end() == mit )
	{
		CX2OfflineLog::Server( L"SOCKET   no random pool for socket data type %d", eSocketDataType );
		return false;
	}

	const int iGroupID = LotteryDecideMulti( mit->second );
	if( iGroupID < 0 )
	{
		CX2OfflineLog::Server( L"SOCKET   group roll fell outside pool %d", eSocketDataType );
		return false;
	}

	std::map< int, std::vector< KSocketBand > >::const_iterator mitSG =
		m_mapSocketGroup.find( iGroupID );

	if( m_mapSocketGroup.end() == mitSG )
	{
		CX2OfflineLog::Server( L"SOCKET   no group %d in SocketItemTable.lua", iGroupID );
		return false;
	}

	// 4. Which enchant band. A fashion item always draws from the best band,
	// and a non-fashion item whose use level is 0 is PvP gear and draws from
	// +8 - SERV_2012_PVP_SEASON2_1, on in this build.
	int iBandLevel = iEnchantLevel;

	if( true == bFashion )
		iBandLevel = (int)CX2OfflineEnchantTable::MAX_ENCHANT_LEVEL;
	else if( 0 == iUseLevel )
		iBandLevel = 8;

	// 5. The first band containing the level wins, in registration order.
	for( size_t i = 0; i < mitSG->second.size(); ++i )
	{
		const KSocketBand& kBand = mitSG->second[i];

		if( iBandLevel < kBand.m_iBeginEnchantLv || iBandLevel > kBand.m_iEndEnchantLv )
			continue;

		const int iOption = LotteryDecideMulti( kBand.m_vecOption );
		if( iOption < 0 )
			break;

		iResultSocketOption = iOption;
		return true;
	}

	CX2OfflineLog::Server( L"SOCKET   group %d has no option for enchant level %d"
		L" (stone %d, pool %d)", iGroupID, iBandLevel, iMagicStoneItemID, eSocketDataType );

	return false;
}

bool CX2OfflineSocketTable::CalcInsertSocketCost( int iItemID, int iMagicStoneItemID,
												  int iAssignedSocketCount, bool bEmptySlot,
												  OUT int& iCost )
{
	iCost = 0;

	const CX2Item::ItemTemplet* pItemTemplet		= CX2OfflineInventory::Templet( iItemID );
	const CX2Item::ItemTemplet* pStoneTemplet	= CX2OfflineInventory::Templet( iMagicStoneItemID );

	if( NULL == pItemTemplet || NULL == pStoneTemplet )
		return false;

	// The STONE's grade sets the multiplier, not the target's. Identical to the
	// figure the client already printed in the socket window
	// (CX2UISocketItem::CalculateNewSocketCost, X2UISocketItem.cpp:829, with the
	// same four factors and the same 0.01).
	double fItemGradeFactor = 0.0;

	switch( pStoneTemplet->GetItemGrade() )
	{
	case CX2Item::IG_UNIQUE:	fItemGradeFactor = 2.5;	break;
	case CX2Item::IG_ELITE:		fItemGradeFactor = 2.0;	break;
	case CX2Item::IG_RARE:		fItemGradeFactor = 1.5;	break;
	case CX2Item::IG_NORMAL:	fItemGradeFactor = 1.0;	break;

	default:
		CX2OfflineLog::Server( L"SOCKET   magic stone %d has grade %d, which has no cost factor",
			iMagicStoneItemID, (int)pStoneTemplet->GetItemGrade() );
		return false;
	}

	// A fashion item is priced flat rather than off its own price.
	if( true == pItemTemplet->GetFashion() )
	{
		iCost = (int)( (double)1000 * fItemGradeFactor );

		if( false == bEmptySlot )
			iCost *= 3;

		return true;
	}

	// Zero sockets already in counts as one, so the first insert is not free.
	if( 0 == iAssignedSocketCount )
		iAssignedSocketCount = 1;

	iCost = (int)( ( (double)pItemTemplet->GetPrice() * 0.01 )
				   * (double)iAssignedSocketCount * fItemGradeFactor );

	// Replacing an option that is already there costs three times as much.
	if( false == bEmptySlot )
		iCost *= 3;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Bound into Lua as g_pCX2SocketItem:* while SocketItemTable.lua runs

void CX2OfflineSocketTable::AddSocketRandomData_LUA( int iSocketDataType, int iGroupID, float fRate )
{
	if( iSocketDataType <= 0 || iGroupID <= 0 )
		return;

	m_mapSocketRandom[ iSocketDataType ].push_back( std::make_pair( iGroupID, fRate ) );
	++m_iRandomRows;
}

void CX2OfflineSocketTable::AddSocketGroup_LUA( int iGroupID, int iBeginEnchantLv, int iEndEnchantLv,
												int iOptionID, float fRate )
{
	if( iGroupID <= 0 || iOptionID <= 0 )
		return;

	std::vector< KSocketBand >& vecBand = m_mapSocketGroup[ iGroupID ];

	// The rows for one band arrive consecutively, so the band being filled is
	// the last one; a new band is appended only when the range changes. That
	// keeps the registration order GetResultItem's "first band containing the
	// level" walk depends on, which a std::map keyed by the range would lose.
	if( true == vecBand.empty() ||
		vecBand.back().m_iBeginEnchantLv != iBeginEnchantLv ||
		vecBand.back().m_iEndEnchantLv != iEndEnchantLv )
	{
		KSocketBand kBand;
		kBand.m_iBeginEnchantLv	= iBeginEnchantLv;
		kBand.m_iEndEnchantLv	= iEndEnchantLv;
		vecBand.push_back( kBand );
	}

	vecBand.back().m_vecOption.push_back( std::make_pair( iOptionID, fRate ) );
	++m_iGroupRows;
}

//////////////////////////////////////////////////////////////////////////

/*static*/ int CX2OfflineSocketTable::LotteryDecideMulti( const KProbRow& kRow )
{
	if( true == kRow.empty() )
		return -1;							///< KLottery::CASE_BLANK

	const float fRoll = (float)( rand() % 10000 ) * 0.01f;		///< 0.00 .. 99.99

	float fAccumulate = 0.0f;

	for( size_t i = 0; i < kRow.size(); ++i )
	{
		fAccumulate += kRow[i].second;

		if( fRoll <= fAccumulate )
			return kRow[i].first;
	}

	return -1;
}

#endif SERV_IRUHADEV_OFFLINE
