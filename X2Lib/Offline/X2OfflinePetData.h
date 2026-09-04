#pragma once

//////////////////////////////////////////////////////////////////////////
// Author: Iruha
// Date: 2026-09-04
// Description: Offline mode - which egg hatches into which pet, and which
//              summon stone into which mount (phase 7b).
//
//              Phase 7 refused both and said why: the client knows only WHICH
//              items are eggs and stones (PET_ITEM_ID and RIDING_PET_ITEM_ID in
//              X2Define.h, which is how CX2UIInventory decides to offer "use"
//              at all), never what any of them turns into. That mapping is two
//              server files, and this is the class that reads them:
//
//                PetData.lua       g_pPetManager:AddPetCreateItemInfo(
//                                      itemID, PET_UNIT_ID[...], periodDays )
//                                  plus the new-pet starting stats in
//                                  SetNewPetInfo, and five random-pet items.
//
//                RidingPetData.lua g_pRidingPetManager:AddRidingPetCreateItemInfo(
//                                      itemID, RIDING_PET_UNIT_ID[...], periodDays )
//
//              Both ship XOR-encrypted inside data036.kom like every other
//              client script, and load exactly the way X2OfflineDropTable loads
//              DropTable.lua. A loose copy in the game directory resolves too
//              and should be deleted once packed - it masks a failed repack.
//
//              periodDays is -1 for a permanent pet and a day count otherwise;
//              4 of the 120 pet rows and 20 of the 38 mount rows are timed.
//
//              THE SHIPPED .lua FILES ARE NEWER THAN THIS SOURCE TREE. They
//              index enum names this build does not have:
//
//                RidingPetData.lua  RPUI_ICE_HAMELING, RPUI_DARK_HAMELING,
//                                   RPUI_RED_HAMELING, RPUI_OBERON_CUSTOM_650,
//                                   RPUI_NASOD_MOBI_INT, RPUI_KOUCIKA_INT,
//                                   RPUI_ANCIENT_PPORU_INT
//                                   -> 7 of the 11 mounts it names; this build's
//                                      CX2RidingPetManager knows exactly four.
//                PetData.lua        PUI_SONOKONG, PUI_SONOKONG_F and five more
//                                   -> 7 of the 105 pets it names.
//
//              Those subscripts come back nil, arrive here as 0, and the row is
//              refused with a log line naming the item. That is deliberate.
//              Inventing an ID would hand the player a mount the client cannot
//              render and burn the ticket doing it. Three of the seven mounts
//              the cash shop sells (items 550040/550041/550042, the Hamelings)
//              land in that bucket, and the client's own RIDING_PET_ITEM_ID
//              array offers "use" on them regardless - so this refusal is
//              reachable in normal play, not a theoretical edge.
//
//              Everything the created pet is then stored in already existed:
//              unit_pet / unit_riding_pet and LoadPets / InsertPet /
//              InsertRidingPet, written in phase 7 and already used by every
//              other pet packet.
//////////////////////////////////////////////////////////////////////////

#ifdef SERV_IRUHADEV_OFFLINE

class CX2OfflinePetData
{
public:
	/// What a pet looks like the moment it hatches - PetData.lua's
	/// SetNewPetInfo( evolutionStep, satiety, intimacy, extroversion, emotion ),
	/// which is one call at the top of the file.
	struct KNewPetInfo
	{
		int	m_iEvolutionStep;
		int	m_iSatiety;
		int	m_iIntimacy;
		int	m_iExtroversion;
		int	m_iEmotion;

		KNewPetInfo()
			: m_iEvolutionStep( 0 )
			, m_iSatiety( 0 )
			, m_iIntimacy( 0 )
			, m_iExtroversion( 0 )
			, m_iEmotion( 0 )
		{
		}
	};

public:
	static CX2OfflinePetData*	Instance();
	static void					Release();

	/// Egg -> pet. false when the item is not an egg at all, or is one whose
	/// pet this build has no enum for; the caller must leave the item alone
	/// either way. iPeriodDays comes back -1 for a permanent pet.
	///
	/// Rolls the random-pet table (AddRandomPetCreateItemInfo) when the item is
	/// one of the five that hatch into one of several pets, which is what
	/// CXSLPetManager::GetPetIDByItemID falls through to.
	bool	GetPetByItem( int iItemID, OUT int& iPetID, OUT int& iPeriodDays );

	/// Summon stone -> mount. Same contract as GetPetByItem.
	bool	GetRidingPetByItem( int iItemID, OUT int& iRidingPetID, OUT int& iPeriodDays );

	const KNewPetInfo&	GetNewPetInfo();

	/// True once at least one of the two files has produced rows. Used only to
	/// word the refusal - which file to name.
	bool	HasPetRows();
	bool	HasRidingPetRows();

	//////////////////////////////////////////////////////////////////////////
	// Bound into the Lua state as g_pPetManager:* and g_pRidingPetManager:*.
	// Public because lua_tinker needs to take their addresses.
	//
	// EVERY function the two files reach has to be bound, including the ones
	// this class has no use for: an unbound call aborts the whole chunk at that
	// line, so a missing AddPetCashFeedItemInfo would cost us the 60 pet rows
	// that come after it. The ones that are recorded but never read say so.

	void	SetNewPetInfo_LUA( int iEvolutionStep, int iSatiety, int iIntimacy,
							   int iExtroversion, int iEmotion );
	void	AddPetCreateItemInfo_LUA( int iItemID, int iPetID, int iPeriodDays );
	void	AddRandomPetCreateItemInfo_LUA( int iItemID, int iPetID, float fRate );

	/// Feeding, satiety decay and pet transformation. Recorded but unused: the
	/// offline feed handler already works off the item the client hands it, and
	/// SERV_TRANSFORM_PET is off in this build.
	void	AddPetCashFeedItemInfo_LUA( int iItemID, int iIncreaseSatiety );
	void	AddSpecialFeedItemID_LUA( int iItemID );
	void	AddPetSatietyDecreaseFactor_LUA( int iEvolutionStep, double fFactor );
	void	AddTransformPetItemInfo_LUA( int iTransformID, int iPetCreateItemID,
										 int iPetID, float fRate );
	void	IncreaseSpecialFeedIntimacyRate_LUA( float fRate );
	void	DecreaseSpecialFeedIntimacyRate_LUA( float fRate );

	void	AddRidingPetCreateItemInfo_LUA( int iItemID, int iRidingPetID, int iPeriodDays );
	void	SetEnableStamina_LUA( float fStamina );

private:
	CX2OfflinePetData();
	~CX2OfflinePetData();

	void	EnsureLoaded();
	bool	RunScript( const wchar_t* szName, int& iRowCounter );

private:
	struct KCreateItem
	{
		int	m_iUnitID;		///< PET_UNIT_ID or RIDING_PET_UNIT_ID
		int	m_iPeriodDays;	///< -1 permanent

		KCreateItem() : m_iUnitID( 0 ), m_iPeriodDays( -1 ) {}
	};

	/// One outcome of a random egg. Probabilities are percentages and are drawn
	/// the way KLottery::Decision draws: one roll in [0,100), first case whose
	/// running total reaches it wins, and nothing at all is a legal result.
	struct KRandomCase
	{
		int		m_iPetID;
		float	m_fRate;

		KRandomCase() : m_iPetID( 0 ), m_fRate( 0.0f ) {}
	};

	static CX2OfflinePetData*	ms_pInstance;

	std::map< int, KCreateItem >					m_mapPetItem;
	std::map< int, KCreateItem >					m_mapRidingPetItem;
	std::map< int, std::vector< KRandomCase > >		m_mapRandomPetItem;

	KNewPetInfo	m_kNewPet;

	bool	m_bLoadAttempted;
	int		m_iPetRows;
	int		m_iRidingPetRows;

	/// Rows the .lua named but this build has no enum for. Counted separately
	/// so the load line can say "7 skipped" out loud rather than leaving a
	/// silent shortfall between the file and the map.
	int		m_iPetSkipped;
	int		m_iRidingPetSkipped;
};

#endif SERV_IRUHADEV_OFFLINE
