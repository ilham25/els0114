#pragma once


#ifdef NEW_SKILL_TREE
/** @class	CX2UserSkillTree
	@brief	������ ��ų ���� ����, B ���� ���� Ȯ��, ���� �÷��� �� ��ų ���� ���� ������ ���� ��
	@date	2011/01/06(�ּ� �߰�)
*/
class CX2UserSkillTree
{
public:
	/// ��ų ���� �ε���
	enum SKILL_SLOT_INDEX
	{
		SKILL_SLOT_A1 = 0,		/// A���� 1, Key = A
		SKILL_SLOT_A2,			/// A���� 2, Key = S
		SKILL_SLOT_A3,			/// A���� 3, Key = D
		SKILL_SLOT_A4,			/// A���� 4, Key = C

		SKILL_SLOT_B1,			/// B���� 1, Key = A
		SKILL_SLOT_B2,			/// B���� 2, Key = S
		SKILL_SLOT_B3,			/// B���� 3, Key = D
		SKILL_SLOT_B4,			/// B���� 4, Key = C

		MAX_SKILL_SLOT,			/// �ִ� ����
	};

	/// ���� B ���� ���� Ȯ��
	enum SKILL_SLOT_B_EXPIRATION_STATE
	{
		SSBES_EXPIRED,		/// ���� B ��� ����
		SSBES_NOT_EXPIRED,	/// ���� B ��� ��� ����
		SSBES_PERMANENT,	/// ���� B ���� ���
	};

	/** @struct	UserSkillData
		@brief	���� ��ų ����
	*/
	struct UserSkillData
	{
	#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
		KProtectedType<int> m_iSkillLevel;					/// ��ų ����
		KProtectedType<int> m_iSkillCSPoint;				/// ��ų ����Ʈ
		KProtectedType<int> m_iIncreaseSkillLevelByBuff;	/// ������ ���� ������ ��ų ����
		KProtectedType<int> m_iIncreaseSkillLevelBySocket;	/// ������ ���� ������ ��ų ����

		UserSkillData()				/// ������
			: m_iSkillLevel( 0 )
			, m_iSkillCSPoint( 0 )
			, m_iIncreaseSkillLevelByBuff(0 )
			, m_iIncreaseSkillLevelBySocket(0 )
		{
		}

		/// ������
		UserSkillData( int iLevel, int iSkillCSPoint )
		{
			m_iSkillLevel					= iLevel;
			m_iSkillCSPoint					= iSkillCSPoint;
			m_iIncreaseSkillLevelByBuff		= 0;
			m_iIncreaseSkillLevelBySocket	= 0;
		}

		/// ��ų ���� ������ ���� ��ȯ �Լ� ( ���� + ���� )
		int  GetIncreaseSkillLevel() const { return m_iIncreaseSkillLevelBySocket + m_iIncreaseSkillLevelByBuff; }
		
		/// ��ų ���� ���� ��ȯ �Լ� ( ��ų ����Ʈ + ���� + ���� )
		int  GetTotalSkillLevel() const { return m_iSkillLevel + m_iIncreaseSkillLevelBySocket + m_iIncreaseSkillLevelByBuff; }

		void SetIncreaseSkillLevelByBuff( int val ) { m_iIncreaseSkillLevelByBuff = val; }
		void SetIncreaseSkillLevelBySocket( int val ) { m_iIncreaseSkillLevelBySocket = val; }
	#else // UPGRADE_SKILL_SYSTEM_2013
		int m_iSkillLevel;			/// ��ų ����
		int m_iSkillCSPoint;		/// ��ų ����Ʈ

		UserSkillData()				/// ������
			: m_iSkillLevel( 0 )
			, m_iSkillCSPoint( 0 )
		{
		}

		/// ������
		UserSkillData( int iLevel, int iSkillCSPoint )
		{
			m_iSkillLevel = iLevel;
			m_iSkillCSPoint = iSkillCSPoint;
		}
	#endif // UPGRADE_SKILL_SYSTEM_2013
	};

	/** @class	SkillSlotData
		@brief	��ų ���� ����
	*/
	struct SkillSlotData
	{
#ifdef VERIFY_STAT_BY_BUFF
		KProtectedType<CX2SkillTree::SKILL_ID>	m_eID;				/// ��ų ���̵�
		KProtectedType<int>		m_iSkillLevel;		/// ��ų ����
		KProtectedType<int>		m_iSkillLevelPlus;	/// �����۵ ���� ��ų ���� ���� ȿ��
		KProtectedType<float>	m_fCoolTimeLeft;	/// ��Ÿ��
		KProtectedType<float>	m_fMPConsumption;	/// ���� �Ҹ�
//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		KProtectedType<USHORT>	m_usCBConsumption;	/// ĳ���� ��뷮
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		KProtectedType<USHORT>	m_usFPConsumption;	/// ��� ��뷮
		KProtectedType<USHORT>	m_usFPGainCount;	/// ��� ȹ�淮
#endif

#else	// VERIFY_STAT_BY_BUFF

		CX2SkillTree::SKILL_ID	m_eID;				/// ��ų ���̵�
		int						m_iSkillLevel;		/// ��ų ����
		int						m_iSkillLevelPlus;	/// �����۵ ���� ��ų ���� ���� ȿ��
		float					m_fCoolTimeLeft;	/// ��Ÿ��
		float					m_fMPConsumption;	/// ���� �Ҹ�
		//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		USHORT					m_usCBConsumption;	/// ĳ���� ��뷮
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
		USHORT					m_usFPConsumption;	/// ��� ��뷮
		USHORT					m_usFPGainCount;	/// ��� ȹ�淮
#endif

#endif // VERIFY_STAT_BY_BUFF


		CX2SkillTree::SKILL_SLOT_ENCHANT m_eSlotEnchant;	/// �Ӽ� ����

		SkillSlotData() :				/// ������
		m_eID( CX2SkillTree::SI_NONE ),
		m_iSkillLevel( 0 ),
		m_iSkillLevelPlus( 0 ),
		m_fCoolTimeLeft( 0.f ),
		m_fMPConsumption(0.f),
		//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
		m_usCBConsumption(0),	/// ĳ���� ��뷮
#endif	NEW_CHARACTER_CHUNG
		//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG
#ifdef ARA_CHARACTER_BASE
		m_usFPConsumption(0),	/// ��� ��뷮
		m_usFPGainCount(0),	/// ��� ȹ�淮
#endif
		m_eSlotEnchant( CX2SkillTree::SSE_NONE )
		{
		}

		void Init()				/// �ʱ�ȭ �Լ�
		{
			m_eID = CX2SkillTree::SI_NONE;
			m_fCoolTimeLeft = 0.f;
			m_iSkillLevel = 0;
			//m_iSkillLevelPlus = 0;	// pvpgame���� EGS_PLAY_START_NOT�� ������ unitdata�� ����鼭 Init()�� ȣ���ϱ� ������ ������ ���� m_iSkillLevelPlus�� m_fMPConsumption�� ���ư������� ���� ������
			//m_fMPConsumption = 0.f;	// pvpgame���� EGS_PLAY_START_NOT�� ������ unitdata�� ����鼭 Init()�� ȣ���ϱ� ������ ������ ���� m_iSkillLevelPlus�� m_fMPConsumption�� ���ư������� ���� ������
		}

		int GetActualSkillLevel()		/// ��ų ���� ���(�����ۿ� ���� ���� ���� ����)
		{
#ifdef ADDED_RELATIONSHIP_SYSTEM
			if( CX2SkillTree::SI_ETC_WS_COMMON_LOVE == m_eID )
				return m_iSkillLevel;
			else
#endif //ADDED_RELATIONSHIP_SYSTEM
				return m_iSkillLevel + m_iSkillLevelPlus;
		}

#ifdef VERIFY_STAT_BY_BUFF
		bool Verify()
		{
			if ( m_eID.Verify() == false
				|| m_iSkillLevel.Verify() == false
				|| m_iSkillLevelPlus.Verify() == false
				|| m_fCoolTimeLeft.Verify() == false
				|| m_fMPConsumption.Verify() == false
				|| m_usCBConsumption.Verify() == false
				|| m_usFPConsumption.Verify() == false
				|| m_usFPGainCount.Verify() == false )
				return false;
			else
				return true;
		}
#endif // VERIFY_STAT_BY_BUFF

	};
	
public:		/// public function
	CX2UserSkillTree(void);
	virtual ~CX2UserSkillTree(void);
	
	void OnFrameMove( double fTime, float fElapsedTime );
	void Reset( bool bResetSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetSkillSlotB );

	void SetEquippedSkill( const int iSlotIndex_, const bool bSlotB_, const KSkillData& equippedSkillData_ );
	void SetEquippedSkill( const KSkillData aEquippedSkill[], const KSkillData aEquippedSkillSlotB[] );
	void SetEquippedSkillLevelPlusBySlotIndex( const int iEquippedSkillLevelPlus_, const int iSlotIndex_, const bool bIsSlotB_ );
	void SetEquippedSkillLevelPlus( const int aEquippedSkillLevelPlus[], const int aEquippedSkillLevelPlusSlotB[] );

	void ConstructSkillTreeNotMyUnit( const std::vector<KSkillData>& vecSkillData, const KSkillData aEquippedSkill[], const KSkillData aEquippedSkillSlotB[] );

	void SetAcquiredSkill( const std::vector<KUserSkillData>& vecSkillList );
	void SetUnsealedSkill( const std::vector<short>& vecUnsealedSkillList );

	int GetSkillLevel( CX2SkillTree::SKILL_ID eSkillID, bool bBaslcLevel = false ) const;
	
	//{{ oasis907 : ����� //// 2009-11-18 ////   userskilltree.GetSkillLevel + skilltree.GetSkillTemplet ���� �Լ�
	const CX2SkillTree::SkillTemplet* GetUserSkillTemplet( CX2SkillTree::SKILL_ID eSkillID) const; 

	bool GetSkillLevelAndCSP( IN CX2SkillTree::SKILL_ID eSkillID, OUT int& iSkillLevel, OUT int& iSkillCSPoint );	// ȹ���� ��ų�� �����ϴ��� üũ, �ִٸ� ��ų������ return, ������ -1 return
	bool SetSkillLevelAndCSP( CX2SkillTree::SKILL_ID eSkillID, int iSkillLevel, int iSkillCSPoint );				// ��ų�� �����ϸ� ��ų������ ������ ������, ������ �߰��ϰ� ��ų������ ������ ������	
	bool IsMaxSkillLevel( CX2SkillTree::SKILL_ID eSkillID );														// ��ų�� �ְ��������� �������

	void SetSkillSlotInfo( int iSkillSlotID, CX2SkillTree::SKILL_ID eSkillID );

//{{ kimhc // 2010.12.14 // 2010-12-23 New Character CHUNG
#ifdef	NEW_CHARACTER_CHUNG
	USHORT GetCBConsumption( const int iSlotIndex_, const bool bSlotB_ ) const;
	void SetCBConsumption( int iSlotIndex_, bool bSlotB_, USHORT usCBConsumption_ );
#ifdef ARA_CHARACTER_BASE
	bool SetSkillSlot( int iSlotIndex_, CX2SkillTree::SKILL_ID eSkillID_, float fMPConsume_, USHORT usCBConsume_,
					   USHORT usFPConsume, bool bSlotB_ );
#else
	bool SetSkillSlot( int iSlotIndex_, CX2SkillTree::SKILL_ID eSkillID_, float fMPConsume_, USHORT usCBConsume_, bool bSlotB_ );
#endif //ARA_CHARACTER_BASE
#else	NEW_CHARACTER_CHUNG
	bool SetSkillSlot( int iSlotIndex, CX2SkillTree::SKILL_ID eSkillID, float MPConsume, bool bSlotB );
#endif	NEW_CHARACTER_CHUNG
//}} kimhc // 2010.12.14 //  2010-12-23 New Character CHUNG

#ifdef ARA_CHARACTER_BASE
	USHORT	GetFPConsumption( const int iSlotIndex_, const bool bSlotB_ ) const;
	void	SetFPConsumption( int iSlotIndex_, bool bSlotB_, USHORT usFPConsumption_ );

	USHORT	GetFPGainCount( const int iSlotIndex_, const bool bSlotB_ ) const;
	void	SetFPGainCount( int iSlotIndex_, bool bSlotB_, USHORT usFPConsumption_ );
#endif
	const CX2UserSkillTree::SkillSlotData* GetSkillSlot( int iSlotIndex, bool bSlotB ) const;
	const CX2UserSkillTree::SkillSlotData* GetSkillSlot( const CX2SkillTree::SKILL_ID eSkillID_ ) const;

	void ChangeMpConsumptionPercent( const CX2SkillTree::SKILL_ID eSkillID_, const float fMultiplier_ );
	void SetMpConsumption( int iSlotIndex_, bool bSlotB_, float fMpConsumption_ );

	// 09.05.30 �¿�
	int GetSkillEquippedSlotIndex( CX2SkillTree::SKILL_ID eSkillID, bool bSlotB );		// ������ -1. ��ĭ�� ã�� ������ ��ųID�� SI_NONE�� �Է��ϸ� �ȴ�.

	void SetSkillCoolTimeLeft( CX2SkillTree::SKILL_ID eSkillID, float fCoolTimeLeft );

//{{ kimhc // 2010.11.2 // �������� - ���� ������
#ifdef	NEW_SKILL_2010_11
	void ResetLeftSkillCoolTimeAll();

#ifdef UPGRADE_SKILL_SYSTEM_2013 /// ����ȯ

#ifdef ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������ / 13-07-04 / ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�
	void ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType, const float fCoolTime = 0.f, bool bForceSet = true, BUFF_CHANGE_TYPE eChangeType = BCT_RELATION_VALUE, const float fCoolTimeRate = 1.0f, const bool bIsRelativeAllSkill = false );

#else // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME
	void ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType, const float fCoolTime = 0.f, bool bForceSet = true, BUFF_CHANGE_TYPE eChangeType = BCT_RELATION_VALUE );

#endif // ADDED_BBT_ALL_EQUIP_SKILL_SET_OR_RESET_COOLTIME					// ������ / 13-07-04 / ��� ���� ���� ��ų�� ��Ÿ�� �ִ�ġ�� �����ǰ� �ϴ� BBT �߰�

#else // UPGRADE_SKILL_SYSTEM_2013
	void ResetLeftSkillCoolTimeBySkillType( const CX2SkillTree::SKILL_TYPE eSkillType );
#endif // UPGRADE_SKILL_SYSTEM_2013

#endif	NEW_SKILL_2010_11
//}} kimhc // 2010.11.2 // �������� - ���� ������

	void InitSkillCoolTimeFromGageManager( const vector<float>& vecSkillCoolTime_ );

	CX2Stat::Stat GetSkillStat() const;

#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-16 //// 
	bool IsGuildSkill( CX2SkillTree::SKILL_ID eSkillID ) const;

	//{{ oasis907 : ����� //// 2009-11-18 ////   userskilltree.GetSkillLevel + skilltree.GetSkillTemplet ���� �Լ� (����)
	const CX2SkillTree::SkillTemplet* GetUserSkillTemplet( CX2SkillTree::SKILL_ID eSkillID, int iGuildMemberGrade) const; 

	//{{ oasis907 : ����� //// 2009-11-12 //// 
	void Reset( bool bResetSkillTree, bool bResetGuildSkillTree, bool bResetEquippedSkill, bool bResetUnsealedSkill, bool bResetSkillSlotB );

	void ConstructGuildSkillTreeNotMyUnit( const std::vector<KSkillData>& vecGuildSkillData);

	//{{ oasis907 : ����� //// 2009-11-12 //// 
	void SetAcquiredGuildSkill( const std::vector<KGuildSkillData>& vecSkillList );

	//{{ oasis907 : ����� //// 2009-11-18 //// 
	int GetSkillLevel( CX2SkillTree::SKILL_ID eSkillID, int iGuildMemberGrade) const;

	//{{ oasis907 : ����� //// 2009-11-12 //// 
	CX2Stat::Stat GetGuildSkillStat(int iGuildMemberGrade);

#endif GUILD_SKILL		
	int UpdateEquippedSkillLevelFromAcqureidMap( const CX2SkillTree::SKILL_ID eSkillID, const int iSlotIndex_, bool bSlotB = false );

	static void GetSlotIndexAndSlotB( IN const int iSkillSlotId_, OUT int& iSlotIndex_, OUT bool& bSlotB_ );	// ���� ID�� ���� SlotIndex�� SlotB ���� ���θ� �˼��ִ� �Լ�

	CX2UserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE GetSkillSlotBExpirationState() const
	{
//{{ Iruha : 2026-08-25 // Skill Slot B open by default, no medal purchase required
#ifdef SERV_IRUHADEV_SKILL_SLOT_B_FREE
		return SSBES_PERMANENT;
#else
		return m_eSkillSlotBExpirationState;
#endif SERV_IRUHADEV_SKILL_SLOT_B_FREE
//}}
	}
	void SetSkillSlotBExpirationState(CX2UserSkillTree::SKILL_SLOT_B_EXPIRATION_STATE val)
	{
		m_eSkillSlotBExpirationState = val; 
	}
	bool GetEnabledSkillSlotB() const;
	const std::wstring& GetSkillSlotBEndDateString() const { return m_wstrSkillSlotBEndDate; }
	void SetSkillSlotBEndDateString(std::wstring val) { m_wstrSkillSlotBEndDate = val; }

	void CalcUsedSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint );
	void CalcCumulativeUsedSPointOnEachTier( OUT std::vector<int>& vecTierSPoint );

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ
	void ExpireCashSkillPoint( OUT const std::map<int, int> mapHaveSKill_ );
#else // UPGRADE_SKILL_SYSTEM_2013
	void ExpireCashSkillPoint();
#endif // UPGRADE_SKILL_SYSTEM_2013

#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	void CalcUsedGuildSPointAndCSPoint( OUT int& iSPoint, OUT int& iCSPoint );
	void CalcCumulativeUsedGuildSPointOnEachTier( OUT std::vector<int>& vecTierSPoint );
	void ExpireGuildCashSkillPoint(); // oasis907 : ����� [2009.11.27] // ��� ĳ�� ��ų ����Ʈ �Ⱓ ���
#endif GUILD_SKILL

	bool IsSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID );
	void AddSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID );

	bool DoIHaveThisSkill( CX2SkillTree::SKILL_ID eSkillID );
	bool CanIResetThisSkill( CX2SkillTree::SKILL_ID eSkillID );
	bool CanILearnThisSkill( CX2SkillTree::SKILL_ID eSkillID, bool bCheckSP = true );
	bool CanIInitSkillTree();

#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	bool CanIInitGuildSkillTree();
#endif GUILD_SKILL

	int GetUnitClass() const { return m_iUnitClass; }
	void SetUnitClass( int val ) { m_iUnitClass = val; }

#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	int GetGuildClass() const { return m_iGuildClass; }
	void SetGuildClass( int val ) { m_iGuildClass = val; }
#endif GUILD_SKILL

#ifdef SERV_SKILL_NOTE
	void SetEqipSkillMemo( const std::vector<int>& vecSkillMemo) { m_vecSkillNote = vecSkillMemo; }
	bool GetEqipSkillMemo(int iSkillMemo);
#endif

	bool IsChangedSkillSlot( const int iSlotIndex_, const bool bSlotB_, const KSkillData& kSkillData_ )
	{
		if ( bSlotB_ )
		{
			// ������ ��ų������ ID�� ���ٸ� ���Ѱ��� �ƴ�
			if ( m_aEquippedSkillSlotB[iSlotIndex_].m_iSkillLevel == kSkillData_.m_cSkillLevel && 
				 static_cast<int>( m_aEquippedSkillSlotB[iSlotIndex_].m_eID ) == kSkillData_.m_iSkillID )
				return false;
			else
				return true;			
		}
		else
		{
			// ������ ��ų������ ID�� ���ٸ� ���Ѱ��� �ƴ�
			if ( m_aEquippedSkill[iSlotIndex_].m_iSkillLevel == kSkillData_.m_cSkillLevel && 
					 static_cast<int>( m_aEquippedSkill[iSlotIndex_].m_eID ) == kSkillData_.m_iSkillID )
				return false;
			else
				return true;
		}		
	}
	void ToggleSkillSlotAB()
	{
		SkillSlotData EquipSkillTemp[4];

		memcpy( EquipSkillTemp, m_aEquippedSkill, sizeof(m_aEquippedSkill) );
		memcpy( m_aEquippedSkill, m_aEquippedSkillSlotB, sizeof(m_aEquippedSkillSlotB) );
		memcpy( m_aEquippedSkillSlotB, EquipSkillTemp, sizeof(EquipSkillTemp) );
	}

#ifdef VERIFY_STAT_BY_BUFF
	bool VerifyEquippedSkillslot()
	{
		for ( int i = 0; i < EQUIPPED_SKILL_SLOT_COUNT; i++ )
		{
			if ( m_aEquippedSkill[i].Verify() == false ||
				m_aEquippedSkillSlotB[i].Verify() == false )
				return false;
		}

		return true;
	}
#endif // VERIFY_STAT_BY_BUFF

#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
	std::map< CX2SkillTree::SKILL_ID, UserSkillData >& GetMapSkillAcquired() { return m_mapSkillAcquired; }	/// ��� ��ų ��� ��ȯ
	void SetDefaultSkill( bool bCanPassIfChecked_ = false );													/// �� ������ �⺻ ��ų ����
	int  GetIncreaseSkillLevel( CX2SkillTree::SKILL_ID eSkillID ) const;	/// ���� ����ġ �� ��ȯ
#endif // UPGRADE_SKILL_SYSTEM_2013

#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
	/// ��ų ���� ID�� ���� �Ǿ��ִ� ��ų ���̵� ��ȯ
	const CX2SkillTree::SKILL_ID GetSkillIDByEquipSlotID( IN const int iStateID_ ) const;
#endif // SERV_ARA_CHANGE_CLASS_SECOND

/// private variable Start ----------
private:
	typedef std::map< CX2SkillTree::SKILL_ID, UserSkillData > SkillDataMap;				// <skill id, UserSkillData>
	SkillDataMap						m_mapSkillAcquired;
#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	SkillDataMap						m_mapGuildSkillAcquired;
#endif GUILD_SKILL
	std::set<CX2SkillTree::SKILL_ID>	m_setUnsealedSkillID;

	SkillSlotData						m_aEquippedSkill[ EQUIPPED_SKILL_SLOT_COUNT ];
	SkillSlotData						m_aEquippedSkillSlotB[ EQUIPPED_SKILL_SLOT_COUNT ];

	SKILL_SLOT_B_EXPIRATION_STATE		m_eSkillSlotBExpirationState;
	std::wstring						m_wstrSkillSlotBEndDate;

	int									m_iUnitClass;
#ifdef GUILD_SKILL
	//{{ oasis907 : ����� //// 2009-11-12 //// 
	int									m_iGuildClass;
#endif GUILD_SKILL

#ifdef SERV_SKILL_NOTE
	std::vector<int> m_vecSkillNote; // CX2SkillTree::SKILL_MEMO_ID
#endif


#ifdef NEXON_QA_CHEAT_REQ
	void AllSkillNoCoolTime();
#endif //NEXON_QA_CHEAT_REQ


/// private variable End ----------
};



#endif NEW_SKILL_TREE