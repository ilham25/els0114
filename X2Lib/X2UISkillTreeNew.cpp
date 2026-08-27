#include "StdAfx.h"

#ifdef UPGRADE_SKILL_SYSTEM_2013

#include ".\x2uiskilltreeNew.h"

const int	MAGIC_SKILL_DESC_WIDTH					= 370;		/// ��ų ���� ��Ʈ�� ���ٴ� �ִ� ���� ����
const int	MAX_ONE_CLASS_SKILL_ROW_NUM				= 5;		/// �ϳ��� ������ ��ų Ʈ�� �ִ� �� ����
const int	SKILL_USE_IMAGE_WIDTH_SIZE				= 70;		/// ��ų ���� ���� �̹��� ���� ũ��
const float SKILL_MAIN_DESC_OFFSET_Y				= 65.f;		/// ���� ������ ������ ���� ��ġ

//////////////////////////////////////////////////////////////////////////
/// CX2SkillTreeSlot : ��ų Ʈ���� ������ ����
//////////////////////////////////////////////////////////////////////////
CX2SkillTreeSlotData::CX2SkillTreeSlotData()
{
	m_eSlotDataType = SDT_SKILL_TREE;
	m_eSkillID = CX2SkillTree::SI_NONE;
}
/*virtual*/ CX2SkillTreeSlotData::~CX2SkillTreeSlotData()
{


}
/*virtual*/ void CX2SkillTreeSlotData::UpdateUI( CKTDGUISlot* pSlot )
{   
	if ( NULL == g_pData ||
		 NULL == g_pData->GetMyUser() ||
		 NULL == g_pData->GetMyUser()->GetSelectUnit() ||
		 NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() ||
		 NULL == g_pData->GetSkillTree() ||
		 NULL == g_pData->GetUIManager() ||
		 NULL == g_pData->GetUIManager()->GetUISkillTree() )
	{
		return;
	}

	CX2SkillTree*		pSkillTree			= g_pData->GetSkillTree();

	CX2UserSkillTree&	refUserSkillTree	= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;


	/// ���� ��ųâ���� ��ų ������ �ø��� �ִ°�
	const bool			bLearnMode			= g_pData->GetUIManager()->GetUISkillTree()->GetNowLearnSkill();

	/// ���� ��ųâ���� ��ų ������ �ʱ�ȭ �ϰ� �ִ°�
	const bool			bInitSkill			= g_pData->GetUIManager()->GetUISkillTree()->GetNowInitSkill();


	/// ���� ���� �������� ���� ���� --------------------------------------------------------------------------------------------------------------------------------
	int iSkillLevel = g_pData->GetUIManager()->GetUISkillTree()->GetSkillLevelInSkillWimdow( m_eSkillID );

	/// ��ų ���ø� ��ü�� ------------------------------------------------------------------------------------------------------------------------------------------
		/// ��ų ���ø� ( Ư�� ������ ������ �ִ� ��ų ���ø� )
	const CX2SkillTree::SkillTemplet*		pSkillTemplet		= g_pData->GetSkillTree()->GetSkillTemplet( m_eSkillID );

		/// ��ų Ʈ�� ���ø�
	int										iUnitClass			= static_cast<int>( g_pData->GetMyUser()->GetSelectUnit()->GetClass() );
	const CX2SkillTree::SkillTreeTemplet*	pSkillTreeTemplet	= g_pData->GetSkillTree()->GetSkillTreeTemplet( iUnitClass, m_eSkillID );

		/// ��ų Ʈ�� ���̾�α�
	CKTDGUIDialogType						pDLGUISkillTree		= g_pData->GetUIManager()->GetUISkillTree()->GetDLGUISkillTree();

	if( NULL == pSkillTree || NULL == pSkillTemplet || NULL == pSkillTreeTemplet || NULL == pDLGUISkillTree )
		return;

	/// ��ų ������ ���� ��Ʈ�� ���� ��ü ---------------------------------------------------------------------------------------------------------------------------
		/// ��ų ������ ���� ��Ʈ�ѵ��� �����ϴ� ��ü
	const map<CX2SkillTree::SKILL_ID, CX2UISkillTreeNew::SkillSlotUI>& refmapSkillSlotUISet = g_pData->GetUIManager()->GetUISkillTree()->GetMapSkillSlotUISet();

		/// ��ų ���̵� ���� ��ų ������ ��Ʈ�� ��ü �˻�
	map<CX2SkillTree::SKILL_ID, CX2UISkillTreeNew::SkillSlotUI>::const_iterator it = refmapSkillSlotUISet.find(m_eSkillID);

	if(it == refmapSkillSlotUISet.end())
	{
		return;
	}

	/// ��ų ������ ��Ʈ�� ��ü ����
	const CX2UISkillTreeNew::SkillSlotUI& refSkillSlotUI = it->second;

	/// ���� ��ų ����Ʈ ��ȯ ---------------------------------------------------------------------------------------------------------------------------------------
		/// ���� ��ų ����Ʈ ( ��ų ���� �� ��ȭ �����̸� ��ų ����Ʈ ��뷮�� �������� ������ ��ȯ )
	const int iNoUseSP		= true == bLearnMode ? g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iSPoint - pSkillTree->GetUsedSPoint() 
												 : g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iSPoint;
		/// ���� ĳ�� ��ų ����Ʈ ( ��ų ���� �� ��ȭ �����̸� ĳ�� ��ų ����Ʈ ��뷮�� �������� ������ ��ȯ )
	const int iNoUseCP		= true == bLearnMode ? g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iCSPoint - pSkillTree->GetUsedCSPoint()
												 : g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iCSPoint;


	/// ���� ���� ���� ----------------------------------------------------------------------------------------------------------------------------------------------
	const int iUnitLevel	= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_Level;	/// ���� ���� ����

	/// ���� ��Ʈ�� ��ȿ �˻� ---------------------------------------------------------------------------------------------------------------------------------------
	CKTDGUIStatic*	pStaticSkillLock	= refSkillSlotUI.m_pStaticSkillLock;			/// ��� ��ų
	CKTDGUIStatic*	pStaticRequireSP	= refSkillSlotUI.m_pStaticStringRequireSP;		/// ��ų �䱸 SP
	CKTDGUIButton*	pButtonSkillLearn	= refSkillSlotUI.m_pButtonSkillLearn;			/// ��ų ���� ��ư
	CKTDGUIButton*	pButtonSkillDesc	= refSkillSlotUI.m_pButtonSkillDesc;			/// ��ų ������ ��ư
	CKTDGUIButton*	pButtonSkillInit	= refSkillSlotUI.m_pButtonSkillInit;			/// ��ų �ʱ�ȭ ��ư
	CKTDGUIStatic*	pPictureSkillMaster	= refSkillSlotUI.m_pStaticSkillMaster;			/// ������ ��ų ó�� UI
	CKTDGUIStatic*	pStaticSkillNoLearn = refSkillSlotUI.m_pStaticSkillNoLearn;			/// ��ų ���� �Ұ� ó�� UI
	CKTDGUIStatic*	pStaticSkillBG		= refSkillSlotUI.m_pStaticSkillBG;				/// ��ų ���� ���

	if ( NULL == pStaticSkillLock ||
		 NULL == pStaticRequireSP ||
		 NULL == pButtonSkillLearn ||
		 NULL == pButtonSkillDesc ||
		 NULL == pButtonSkillInit ||
		 NULL == pPictureSkillMaster || 
		 NULL == pStaticSkillNoLearn ||
		 NULL == pStaticSkillBG )
		return;

#ifdef ADDED_RELATIONSHIP_SYSTEM
	// �ο� ��ų�� ���� ���� ó��
	if ( pSkillTemplet->m_eType == CX2SkillTree::ST_RELATIONSHIP_SKILL )
	{
		int iRelationshipLevel = 0; // ���� �ο� �ܰ�, 0 = �ַ�, 1 = Ŀ��, 2 = ��ȥ

		/// �ʱ�ȭ �� ��ų ���� ǥ�� -----------------------------------------------------------------------------------
		if ( NULL != g_pData->GetRelationshipManager() &&
			NULL != g_pData->GetRelationshipManager()->GetMyRelationshipInfo() )
		{
			switch ( g_pData->GetRelationshipManager()->GetMyRelationshipInfo()->m_eRelationStateType )
			{
			case SEnum::RT_SOLO :
				{
					iRelationshipLevel = 0;
					// �������� �ο� ��ų ���� ����
					if( NULL != g_pData && NULL != g_pData->GetSkillTree() )
					{
						g_pData->GetSkillTree()->UnequipRelationSkill();
					}
				}
				break;
			case SEnum::RT_COUPLE :
				iRelationshipLevel = 1;
				break;

			case SEnum::RT_MARRIED :
				iRelationshipLevel = 2;
				break;
			}

		}

		// �ؽ�ó ���� -------------------------------------------------------------------------------------------------
		pSlot->SetItemTex( pSkillTemplet->m_wstrIconName.c_str(), pSkillTemplet->m_wstrIconPieceName.c_str() );

		/// ��ȥ ��ų ���� ---------------------------------------------------------------------------------------------
		refUserSkillTree.SetSkillLevelAndCSP( m_eSkillID, iRelationshipLevel, 0 );

		// if(true == refUserSkillTree.DoIHaveThisSkill(m_eSkillID)) // �� ��ų�� ���� �ִٸ�..
		// ��� ĳ���ʹ� �ο� ��ų�� ���� �־����
		{
			// Ŀ�� �̻� �ܰ��� ���� "Lv. 1" Static �� Drag �� ���´�.
			// RButtonUp ���� ��ų �ִ� �κ��� �ش� �޽������� �ٽ� �ڵ�
			// �׳� ���¿����� Drag �Ұ�
			if( iRelationshipLevel > 0 )		
				pSlot->SetDragable( true );
			else
				pSlot->SetDragable( false );
		}

		/// ��ų ������ ������Ʈ ---------------------------------------------------------------------------------------
		/// ȹ�� ����, �� ������ �Ⱥ�����	
		/// ��� �Ұ�, �� ��Ȳ�� ���� ������
		ShowSlotPicture( false, pSlot, STSAPT_CANLEARN ); 

		if ( iRelationshipLevel > 0 )
		{
			ShowSlotPicture( false, pSlot, STSAPT_DISABLE );
		}
		else
		{
			ShowSlotPicture( true, pSlot, STSAPT_DISABLE );
		}

		//if( -1 != refUserSkillTree.GetSkillEquippedSlotIndex( m_eSkillID, false ) || 
		//	-1 != refUserSkillTree.GetSkillEquippedSlotIndex( m_eSkillID, true ) )
		//{
		//	// ��ų�� ���� ���̸�
		//	ShowSlotPicture( true, pSlot, STSAPT_EQUIPPED );
		//}
		//else
		//{
		//	ShowSlotPicture( false, pSlot, STSAPT_EQUIPPED );
		//}


		/// �䱸 ���� �� ������ ���� ������Ʈ --------------------------------------------------------------------------
		if ( NULL != pStaticRequireSP->GetString( 0 ) && 
			 NULL != pPictureSkillMaster->GetPicture(0) )
		{
			if ( 2 > iRelationshipLevel )	/// ��ȥ �����̸�, ���� ���� ǥ��
			{
				pStaticRequireSP->SetShow( true );
				pPictureSkillMaster->GetPicture(0)->SetShow( false );

				/// �䱸 SP ��� �ʿ��� ���� ���� ǥ�� -------------------------------------------------------------------------
				wstring wstrRelationState = L"";

				switch ( iRelationshipLevel ) 
				{
				case 0:
					wstrRelationState = GET_STRING ( STR_ID_24461 );	/// Ŀ��
					break;
				default:
					wstrRelationState = GET_STRING ( STR_ID_24462 );	/// ��ȥ
					break;
				}

				WCHAR buf[256] = {0,};

				/// ������ ���� ���� �̴� ǥ��
				StringCchPrintf( buf, 255, L"#CFF8000%s#CX", wstrRelationState.c_str() );
				pStaticRequireSP->GetString( 0 )->msg = buf;
			}
			else		/// ��ȥ �̻��̸�, ������ ��ų ó��
			{
				pStaticRequireSP->SetShow( false );
				pPictureSkillMaster->GetPicture(0)->SetShow( true );	/// ������ ���� ����
			}
			
		}

		/// ��ư ������Ʈ ----------------------------------------------------------------------------------------------
		pButtonSkillLearn->SetEnableClick( false );		/// ��ų ����Ʈ�� ��� �� �ִ� ��ų�� �ƴϹǷ�, ������ ��Ȱ��
		pButtonSkillLearn->SetEnable( false );
		pButtonSkillLearn->SetShow( 2 > iRelationshipLevel ? true : false );		/// ��ȥ ���̶��, ��ư ����

		/// �ڹ��� ������Ʈ --------------------------------------------------------------------------------------------
		if ( iRelationshipLevel > 0 )
			pStaticSkillLock->SetShow( false );

		else
			pStaticSkillLock->SetShow( true );
	}

	else
#endif // ADDED_RELATIONSHIP_SYSTEM
	{
		/// ��ų ���� �˻� ---------------------------------------------------------------------------------------------
		bool bIsLockSkill			= false;	/// ���ν�ų ��� ����
		bool bMasterLevel			= false;	/// ������ ��ų ���� ����
		bool bCanLearnLevel			= false;	/// ��ų�� ��� �� �ִ� ���� ����
		bool bHaveLearnSP			= false;	/// ��ų�� ��� �� �ִ� SP ���� ����
		bool bLearnPrecedingSkill	= true;		/// ���� ��ų ���� ���� ( ���� ��ų�� ���ٸ�, true )

		const int iMaxLearnLevel = pSkillTreeTemplet->m_iMaxSkillLevel;	/// �ִ�� ���� �� �ִ� ��ų ����

			/// ���ν�ų�̸�, Ǯ���� �ʾ��� ��
		if( true == pSkillTemplet->m_bBornSealed && false == refUserSkillTree.IsSkillUnsealed( m_eSkillID ) )
		{
			bIsLockSkill = true;		/// ���� ��ų ���

			/// ���� ��ų �������� ������ ������ ��			pDLGUISkillTree->ChangeSequence( refSkillSlotUI.m_pStaticSkillLock, true );
		}

		refSkillSlotUI.m_pStaticSkillLock->SetShow( bIsLockSkill );

			/// ������ ������ ��ų���� �˻�
		if ( iMaxLearnLevel <= iSkillLevel )		/// ��ų�� ���̻� ���� �� ���ٸ�, ������ ���� UI ó��
			bMasterLevel = true;

			/// ���� ������ ��ų�� ������ �� �ִ��� �˻�
		if ( pSkillTemplet->GetSkillRequireLevelValue( iSkillLevel + 1 ) <= iUnitLevel )
			bCanLearnLevel = true;

			/// ���� ��ų ���� ���� �˻�
		if ( 0 < pSkillTreeTemplet->m_iPrecedingSkill )
		{
			/// ���� ��ų�� ����
			int iPrecedingSkillLevel = g_pData->GetUIManager()->GetUISkillTree()->GetSkillLevelInSkillWimdow
				( static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iPrecedingSkill ) );

			if ( 0 >= iPrecedingSkillLevel )
				bLearnPrecedingSkill = false;	/// ���� ��ų�� 0 �����̸�, �̽��� ó��
		}


		/// ���� ������Ʈ ----------------------------------------------------------------------------------------------
		pSlot->SetItemTex( pSkillTemplet->m_wstrIconName.c_str(), pSkillTemplet->m_wstrIconPieceName.c_str() );

		if(true == refUserSkillTree.DoIHaveThisSkill(m_eSkillID))
		{
			switch(pSkillTemplet->m_eType)
			{
			case CX2SkillTree::ST_ACTIVE:
			case CX2SkillTree::ST_BUFF:
			case CX2SkillTree::ST_SPECIAL_ACTIVE:
				{
					/// �����ε� ó���� ��ų�̰ų� ��ų ������ �ø��� ���̶��, �巡�� �� �� ����.
					if ( true == refSkillSlotUI.m_bSelectSkillBlind || true == bLearnMode || true == bIsLockSkill )
						pSlot->SetDragable(false);
					else
						pSlot->SetDragable(true);
				} break;
			default:
				{
					pSlot->SetDragable(false);
				} break;
			}
		}
		else
		{
			ShowSlotPicture( false, pSlot, STSAPT_LEVEL );
			pSlot->SetDragable(false);
		}

			/// ��ų ����  ǥ��
		if ( 0 < iSkillLevel )
			ShowSlotPicture( true, pSlot, STSAPT_LEVEL, iSkillLevel );


			
		/// ���� ���� ������Ʈ -----------------------------------------------------------------------------------------
		if( false == refUserSkillTree.DoIHaveThisSkill(m_eSkillID) || true == bIsLockSkill )		/// ����� ���� ��ų�̰ų� ��� ��ų�� ������ ��Ӱ� ó��
			ShowSlotPicture( true, pSlot, STSAPT_DISABLE );
		else
			ShowSlotPicture( false, pSlot, STSAPT_DISABLE );

		// ��ų ������ ǥ��
		/*if( -1 != refUserSkillTree.GetSkillEquippedSlotIndex( m_eSkillID, false ) || 
			-1 != refUserSkillTree.GetSkillEquippedSlotIndex( m_eSkillID, true ) )
			ShowSlotPicture( true, pSlot, STSAPT_EQUIPPED );
		else
			ShowSlotPicture( false, pSlot, STSAPT_EQUIPPED );*/

		/// ��ų ���� ���� ���� �ȳ� �ؽ�ó ǥ��
		if ( 0 >= iSkillLevel && false == refSkillSlotUI.m_bSkillLineBlind )			/// ��� �� �ִ� ��ų�� ��
		{
			const int	iIndexInTier			= pSkillTreeTemplet->m_iIndexInTier;	/// ��ų ���� ���� �ε���
			bool		bIsSelectAnotherSkill	= false;								/// �ݴ��� ��ų ���� ���� ����

			/// 2�� ���� ��ų�� ��, �ݴ��� ��ų ���̵� �����ϰų� ���� ������ ��ų���� �˻�
			if ( CX2UISkillTreeNew::STST_SINGLE_SKILL != iIndexInTier )
			{
				const CX2SkillTree::SKILL_ID eAnotherSkillID = g_pData->GetUIManager()->GetUISkillTree()->GetAnotherSelectSkillID( m_eSkillID );

				/// �ݴ��� ��ų ���̵� ���� ������ ��ų���� �˻�
				if ( CX2SkillTree::SI_NONE != eAnotherSkillID )
				{
					const CX2SkillTree::SkillTemplet* pSkillTemplet	= g_pData->GetSkillTree()->GetSkillTemplet( eAnotherSkillID );

					/// ���� ��ų�� �ƴϰų�, ������ ���ν�ų�̸�, ���� ������ �ݴ��� ��ų�� ����
					if ( NULL != pSkillTemplet && 
							false == pSkillTemplet->m_bBornSealed || 
							( true == pSkillTemplet->m_bBornSealed && true == refUserSkillTree.IsSkillUnsealed( eAnotherSkillID ) ) )
					{
						bIsSelectAnotherSkill = true;
					}
				}
			}


			if ( CX2UISkillTreeNew::STST_SINGLE_SKILL != iIndexInTier &&	/// 2������ ��ų�̸�,
			 	 false == refSkillSlotUI.m_bSelectSkillBlind &&				/// ������ ���� ���� ��ų�� �ƴϸ�,
				 false == bIsLockSkill &&									/// ��� ��ų�� �ƴϸ�,
				 true  == bIsSelectAnotherSkill )							/// �ݴ��� ��ų�� ���� ������ ��ų�� ��
			{
				if ( CX2UISkillTreeNew::STST_SELECT_SKILL_LEFT == iIndexInTier )	/// ���� ��ų�̸�, ���� ���� ǥ��
				{
//{{ Iruha : 2026-08-27 // SKILLTREE_NO_LOCK removes the choice restriction, so the "AbleChoice" indicator no longer applies
#ifdef SERV_IRUHADEV_SKILLTREE_NO_LOCK
					ShowSlotPicture( false, pSlot, STSAPT_SELECT );
#else
					ShowSlotPicture( true, pSlot, STSAPT_SELECT );

					pDLGUISkillTree->ChangeSequence( pSlot, true );		/// 2������ ���� ���� �ֻ����� ����
#endif SERV_IRUHADEV_SKILLTREE_NO_LOCK
//}}
				}
				else	/// ������ ��ų�̸�, ���� ���� ���� ( ���ʿ��� ����� ���̴�. )
					ShowSlotPicture( false, pSlot, STSAPT_SELECT );

				ShowSlotPicture( false, pSlot, STSAPT_CANLEARN );		/// ȹ�� ���� ����
			}
			else		/// �ϳ��� ��ų�� ���� ���� ����
			{
				ShowSlotPicture( false, pSlot, STSAPT_SELECT );			/// 2������ ���� ���� ����

				if ( false == bIsLockSkill )
					ShowSlotPicture( true, pSlot, STSAPT_CANLEARN );	/// ȹ�� ���� ����
			}
		}
		else			/// ��� �� ���� ��ų�� ��
		{
			ShowSlotPicture( false, pSlot, STSAPT_SELECT );				/// 2������ ���� ���� ����
			ShowSlotPicture( false, pSlot, STSAPT_CANLEARN );			/// ȹ�� ���� ����
		}



		/// ���� ǥ�ÿ� ��Ʈ�� ���� ------------------------------------------------------------------------------------
		if ( NULL != pStaticRequireSP->GetString( 0 ) )
		{
			
			/// ������ ������ �ƴ� �� �� ����
			if ( false == bMasterLevel && false == bInitSkill )
			{
				/// �䱸 SP ������Ʈ -------------------------------------------------------------------------------------------
				int iOutputREquireSPValue = 0 == iSkillLevel ? pSkillTemplet->m_iRequireLearnSkillPoint : 
																pSkillTemplet->m_iRequireUpgradeSkillPoint;		/// ��ų ��ȭ�� �䱸�Ǵ� SP ��

				wstring wstrOutputMsg = L"";	/// ��½�ų ��Ʈ��

				/// ��ų�� ���µ� �䱸�Ǵ� ����Ʈ�� �������� ��, �Ķ� ��( �⺻ )���� ǥ��
				if ( iOutputREquireSPValue <= iNoUseSP + iNoUseCP )
				{
					wstrOutputMsg	+= GET_REPLACED_STRING( ( STR_ID_25073, "i", iOutputREquireSPValue ) );
					bHaveLearnSP	=  true;
				}
				/// ��ų�� ���µ� �䱸�Ǵ� ����Ʈ�� �������� ���� ��, ���� ������ ǥ��
				else
				{
					wstrOutputMsg	=  L"#CFF8000";
					wstrOutputMsg	+= GET_REPLACED_STRING( ( STR_ID_25073, "i", iOutputREquireSPValue ) );
					wstrOutputMsg	+=  L"#CX";
				}

				/// ���
				pStaticRequireSP->GetString( 0 )->msg = wstrOutputMsg;

				pStaticRequireSP->SetShow( true );
			}
			else
			{
				pStaticRequireSP->SetShow( false );
			}
		}
			


		/// ������ ��ų UI ���� ----------------------------------------------------------------------------------------
		if ( NULL != pPictureSkillMaster->GetPicture(0) )
		{
			if ( true == bInitSkill )
				pPictureSkillMaster->GetPicture(0)->SetShow( false );		 /// ��ų �ʱ�ȭ �����, ǥ�� ����
			else
				pPictureSkillMaster->GetPicture(0)->SetShow( bMasterLevel ); /// ������ �����̸� Ȱ��
		}

			/// ���� ��ư�� �ʱ�ȭ �����̸� ������ ǥ��, �� �ܿ� ������ ������ �ƴ� ���� ǥ��
		pButtonSkillLearn->SetShow( true == bInitSkill ? false : !bMasterLevel );

			/// �ʱ�ȭ ��ư�� �ʱ�ȭ�� ������ ǥ��
		pButtonSkillInit->SetShow( bInitSkill );

			/// �ʱ�ȭ ��ư ���۽�, SP ǥ�� ���� ����
		if ( NULL != pStaticSkillBG->GetPicture( 1 ) )
			pStaticSkillBG->GetPicture( 1 )->SetShow( !bInitSkill );

			/// �ʱ�ȭ �����̸�, ������ ������ ���� �̹��� ��ǥ��, �� �ܿ� ������ ������ ���� ǥ��
		if ( NULL != pPictureSkillMaster->GetPicture( 0 ) )
			pPictureSkillMaster->GetPicture( 0 )->SetShow( true == bInitSkill ? false : bMasterLevel );



		/// ��ư ó�� --------------------------------------------------------------------------------------------------
		if ( true == bInitSkill )		/// ��ų �ʱ�ȭ ������ ��
		{
			pButtonSkillDesc->SetEnable( false );	/// ������ ��ư ����

			const bool bIsDefaultSkill = g_pData->GetSkillTree()->isDefaultSkill( m_eSkillID );		/// �⺻ ��ų ����

			/// �޽��� �� �ؽ�ó ����
			if ( ( false == bIsDefaultSkill && 0 < iSkillLevel ) || 
				 ( true == bIsDefaultSkill && 1 < iSkillLevel ) )		/// �⺻ ��ų�� �ƴϸ鼭 0���� ���� ũ�ų�, �⺻ ��ų�̸鼭 1���� ���� Ŭ ��
			{
				pButtonSkillInit->SetEnableClick( true );
				pButtonSkillInit->SetEnable( true );
			}
			else						/// ����� �ʾ�����, ��ư ������ ��Ȱ��
			{
				pButtonSkillInit->SetEnableClick( false );
				pButtonSkillInit->SetEnable( false );
			}
		}
		else
		{
			if ( false == bMasterLevel )
			{
				/// ��� �� �ִ� ������ ��, ��ų ���� ��ư Ȱ��
				if ( true  == bCanLearnLevel &&							/// ��� �� �ִ� ����
					 true  == bHaveLearnSP &&							/// ����� SP ����
					 false == bIsLockSkill &&							/// ����� ��ų
					 true  == bLearnPrecedingSkill &&					/// ���� ��ų ���� ����
					 false == refSkillSlotUI.m_bSelectSkillBlind &&		/// 2�������� ���ù��� ��ų
					 false == refSkillSlotUI.m_bSkillLineBlind )		/// ��ų ������ �����ִ� ��ų
				{
					pButtonSkillLearn->SetEnable( true );
					pButtonSkillLearn->SetEnableClick( true );
					pButtonSkillDesc->SetEnable( false );	/// ������ ��ư ����

					if ( 0 >= iSkillLevel )		/// ����
						pButtonSkillLearn->SetCustomMsgMouseUp( CX2UISkillTreeNew::STUCM_BUTTON_SKILL_LEVEL_PLUS );
					else						/// ��ȭ
						pButtonSkillLearn->SetCustomMsgMouseUp( CX2UISkillTreeNew::STUCM_BUTTON_SKILL_LEVEL_UPGRADE );
				}
				else	/// ������ �����ϰų� SP�� ���ڶ� ��, ��ų ���� ��ư ��Ȱ��
				{
					pButtonSkillLearn->SetEnable( false );
					pButtonSkillLearn->SetEnableClick( false );

					/// ��ų ���� �������� ���Ͽ� ������ ���� ������, ���� ǥ��
					if ( false == refSkillSlotUI.m_bSkillLineBlind )
					{
						pButtonSkillDesc->SetEnable( true );	/// ������ ��ư Ȱ��

						/// ��Ȱ���� ���ǿ� ���� ���� ����
						wstring wstrNotLearnSkillDesc = L"";

						if ( false == bCanLearnLevel )	/// ���� ����
							wstrNotLearnSkillDesc += GET_REPLACED_STRING( ( STR_ID_25111, "i", pSkillTemplet->GetSkillRequireLevelValue( iSkillLevel + 1 ) ) );

						if ( false == bHaveLearnSP )	/// ��ų ����Ʈ�� ����
						{
							if ( wstrNotLearnSkillDesc != L"" )		/// ��ĭ�� �ƴϸ�, ���� ����
								wstrNotLearnSkillDesc += L"\n";

							wstrNotLearnSkillDesc += GET_STRING( STR_ID_25107 );
						}

						pButtonSkillDesc->SetGuideDesc( wstrNotLearnSkillDesc.c_str() );
					}
					else		/// ��ų ���� �������� ���Ͽ� ����������, ���� ��ǥ��
						pButtonSkillDesc->SetEnable( false );	/// ������ ��ư ��Ȱ��
				}
			}
			else	/// ������ ���� �����̸�, Ŭ�� ����
			{
				pButtonSkillLearn->SetEnableClick( false );
			}
		}



		/// 2������ ��ų ���� �Ҵ� UI ���� -----------------------------------------------------------------------------
		if ( NULL != pStaticSkillNoLearn->GetPicture( 0 ) )
		{
			pStaticSkillNoLearn->GetPicture( 0 )->SetShow( refSkillSlotUI.m_bSelectSkillBlind );

			if ( true == refSkillSlotUI.m_bSelectSkillBlind )
				pDLGUISkillTree->ChangeSequence( pStaticSkillNoLearn, true );
					
		}
	}
}

void CX2SkillTreeSlotData::ShowSlotPicture(bool bShow, CKTDGUISlot* pSlot, SKILL_TREE_SLOT_ADD_PICTURE_TYPE eType, int val /*= -1*/ )
{
	if(pSlot == NULL)
		return;

	CKTDGUIControl::CPictureData* pPicture = pSlot->GetPicture((int)eType);
	if(pPicture == NULL )
	{
		if(bShow)
		{
			pPicture = new CKTDGUIControl::CPictureData();
			CKTDGUIControl::UIPointData* pPoint = new CKTDGUIControl::UIPointData(pSlot->GetNowPoint());
			pPicture->SetPoint( pPoint );

			switch(eType)
			{
			case STSAPT_DISABLE:
				{
					pPicture->SetTex( L"HQ_ImpossibleEqip.tga" );			
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,0.6f );
				} break;
			case STSAPT_SEALED:
				{
					pPicture->SetTex( L"DLG_UI_Common_Texture02.TGA", L"lock" );
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,1 );
				} break;
			case STSAPT_LEVEL:
				{
					// ���� �ؽ���. ���ҽ� ������...
					wstring wstrLevelTextureName = g_pData->GetSkillTree()->GetSkillLevelTextureName( val );
					wstring wstrLevelTextureKey = g_pData->GetSkillTree()->GetSkillLevelTextureKey( val );
					if( false == pPicture->SetTex( wstrLevelTextureName.c_str(), wstrLevelTextureKey.c_str() ) )
					{
						pPicture->SetTex( L"NoAlphaImage.dds" );
					}
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,1 );

					/// ���� �ؽ�ó ��ġ ����
					D3DXVECTOR2 vecPos					= pPicture->GetPos();
					pPicture->SetPos( D3DXVECTOR2( vecPos.x, vecPos.y + 33.f ) );

					D3DXVECTOR2 vecPicturePos			= pPicture->pPoint->leftTopPoint;
					pPicture->pPoint->leftBottomPoint	= D3DXVECTOR2( vecPicturePos.x, vecPicturePos.y + 14.f );
					pPicture->pPoint->rightTopPoint		= D3DXVECTOR2( vecPicturePos.x + 25.f, vecPicturePos.y );
					pPicture->pPoint->rightBottomPoint	= D3DXVECTOR2( vecPicturePos.x + 25.f, vecPicturePos.y + 14.f );
				} break;
			case STSAPT_EQUIPPED:
				{
					pPicture->SetTex( L"DLG_Common_New_Texture26.tga", L"Skill_Mount" );
					pPicture->SetFlicker( 2.0f, 1.4f, 0.2f );

				} break;
			case STSAPT_CANLEARN:
				{
					pPicture->SetTex( L"DLG_Common_New_Texture26.tga", L"acquire_Skill" );
					pPicture->SetFlicker( 2.0f, 1.4f, 0.2f );

				} break;
			case STSAPT_SELECT:
				{
					pPicture->SetTex( L"DLG_UI_Common_Texture_NewSkill_01.TGA", L"AbleChoice" );
					pPicture->SetFlicker( 2.0f, 1.4f, 0.2f );

					D3DXVECTOR2 vecPos = pPicture->GetPos();

					pPicture->SetPos( D3DXVECTOR2( vecPos.x + 20.f, vecPos.y ) );
					pPicture->SetSizeX( 90.f );
				} break;
			default:
				{

				} break;
			}

			pSlot->AddPicture( (int)eType, pPicture );
			pPicture->SetShow(bShow);
		}		
	}
	else
	{
		if(bShow)
		{
			switch(eType)
			{
			case STSAPT_LEVEL:
				{
					wstring wstrLevelTextureName = g_pData->GetSkillTree()->GetSkillLevelTextureName( val );
					wstring wstrLevelTextureKey = g_pData->GetSkillTree()->GetSkillLevelTextureKey( val );
					if( false == pPicture->SetTex( wstrLevelTextureName.c_str(), wstrLevelTextureKey.c_str() ) )
					{
						pPicture->SetTex( L"NoAlphaImage.dds" );
					}
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,1 );

				} break;
			default:
				break;
			}

		}

		pPicture->SetShow(bShow);
	}

}

//////////////////////////////////////////////////////////////////////////
/// CX2EquippedSkillSlot : ���� ������� ��ų ����
//////////////////////////////////////////////////////////////////////////
CX2EquippedSkillSlotData::CX2EquippedSkillSlotData()
{
	m_eSlotDataType = SDT_SKILL_EQUIP;
	m_bDataUpdated	= true;
	m_eSkillID		= CX2SkillTree::SI_NONE;
	m_iIndex = -1;
	m_bSlotb = false;
	m_bDataUpdated = true;
}
CX2EquippedSkillSlotData::CX2EquippedSkillSlotData(int index, bool bSlotb )
{
	m_eSlotDataType = SDT_SKILL_EQUIP;
	m_bDataUpdated	= true;
	m_eSkillID		= CX2SkillTree::SI_NONE;
	m_iIndex		= index;
	m_bSlotb		= bSlotb;
	m_bDataUpdated = true;

}
/*virtual*/ CX2EquippedSkillSlotData::~CX2EquippedSkillSlotData()
{

}
/*virtual*/ void CX2EquippedSkillSlotData::UpdateUI( CKTDGUISlot* pSlot )
{
	// 1. ���� ���� �� �ִ� ��ų ���� : ��ư�� Ȱ��ȭ/��Ȱ��ȭ. bool ���� ����
	// 2. 1�����̶� ���� ��ų ���� : ��ų ������ �巡�� Ȱ��ȭ / ��Ȱ��ȭ(�нú�� �巡�� �� �ǰ�), �ؽ��� ����. ��Ʈ�� ���� (����/�ִ뷹��)


	CX2UserSkillTree& refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;


	if(m_bSlotb == true && false == refUserSkillTree.GetEnabledSkillSlotB() )
	{
		if(m_eSkillID != CX2SkillTree::SI_NONE)
		{
			m_eSkillID = CX2SkillTree::SI_NONE;
			pSlot->SetItemTex( L"DLG_Common_New_Texture03.TGA", L"invisible" );
			pSlot->SetDragable(false);
			ShowSlotPicture(false, pSlot, SESAPT_LEVEL );
		}
		if(true == pSlot->GetEnable())
		{			
			ShowSlotPicture(true, pSlot, SESAPT_DISABLE );
			pSlot->SetEnable(false);
		}		
		return;
	}
	else
	{
		ShowSlotPicture(false, pSlot, SESAPT_DISABLE );
		pSlot->SetEnable(true);
	}	


	const CX2UserSkillTree::SkillSlotData* pSkillData = refUserSkillTree.GetSkillSlot( m_iIndex, m_bSlotb);
	m_eSkillID = pSkillData->m_eID;

	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( m_eSkillID );
	if( pSkillTemplet != NULL)
	{
		pSlot->SetItemTex( pSkillTemplet->m_wstrIconName.c_str(), pSkillTemplet->m_wstrIconPieceName.c_str() );		
		pSlot->SetDragable(true);

		int iSkillLevel = refUserSkillTree.GetSkillLevel( m_eSkillID, true );

		if(iSkillLevel <= 0 && m_eSkillID != CX2SkillTree::SI_NONE)
		{
			ASSERT( !"Skill Level 0 Equipped!" );

			/// ��ų ������ 0�� ��ų�� �������̶��, ��������
			CX2EquippedSkillSlotData* pSlotData = (CX2EquippedSkillSlotData*)pSlot->GetSlotData();

			g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), CX2SkillTree::SI_NONE );

			return;
		}

		if(pSkillTemplet->m_bShowSkillLevel)
		{
			int ilevelUpNum = 0;

			/// �ο� ��ų�� �ƴҶ��� �߰� ���� ���� ȿ�� ����
			if ( pSkillTemplet->m_eType != CX2SkillTree::ST_RELATIONSHIP_SKILL )
				ilevelUpNum = refUserSkillTree.GetIncreaseSkillLevel( m_eSkillID );

			ShowSlotPicture(true, pSlot, SESAPT_LEVEL, min(iSkillLevel + ilevelUpNum, MAX_LIMITED_SKILL_LEVEL), ( ilevelUpNum != 0 ));
		}
		else
			ShowSlotPicture(false, pSlot, SESAPT_LEVEL);

#ifdef FIX_SPECTRO_EL_CRYSTAL_ACTIVE_BUG	/// �б����� ������ ���� ��, CKTDGUISlot �� OnFrameMove Ÿ�鼭 ������ �ʱ�ȭ �ǹ����� ���� ����
		if( NULL != g_pX2Game && CX2SkillTree::SI_A_EEL_SPECTRO_EL_CRYSTAL == pSkillTemplet->m_eID )	/// �б����� ��ų�� ��, ������ ���Ž�������
		{
			g_pX2Game->UpdateEveElectraSkillSlotIcon();
		}
#endif FIX_SPECTRO_EL_CRYSTAL_ACTIVE_BUG
	}
	else
	{
		if ( CX2SkillTree::SI_NONE != m_eSkillID )		/// ��ų ���ø��� ������, ��ų ���̵� 0�� �ƴ� ��
		{
			/// �̻��� ��ų�� �����ϰ� �־��!!
			ASSERT( !L"Equipped Unknown SKill!!!" );

			/// ��ų ���ø��� ���� ���̵��� ��ų�� �������̶��, ��������
			CX2EquippedSkillSlotData* pSlotData = (CX2EquippedSkillSlotData*)pSlot->GetSlotData();

			g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), CX2SkillTree::SI_NONE );

			return;
		}
		else
		{
			pSlot->SetItemTex( L"DLG_Common_New_Texture03.TGA", L"invisible" );
			ShowSlotPicture(false, pSlot, SESAPT_LEVEL);
			pSlot->SetDragable(false);
		}
	}
}

void CX2EquippedSkillSlotData::ShowSlotPicture(bool bShow, CKTDGUISlot* pSlot, SKILL_EQUIPPED_SLOT_ADD_PICTURE_TYPE eType, int val /*= -1*/ , bool bIsSkillLevelUpByItem/* = false*/)
{
	if(pSlot == NULL)
		return;

	CKTDGUIControl::CPictureData* pPicture = pSlot->GetPicture((int)eType);

	if(pPicture == NULL )
	{
		if(bShow)
		{
			pPicture = new CKTDGUIControl::CPictureData();
			CKTDGUIControl::UIPointData* pPoint = new CKTDGUIControl::UIPointData(pSlot->GetNowPoint());
			pPicture->SetPoint( pPoint );

			switch(eType)
			{
			case SESAPT_DISABLE:
				{
					pPicture->SetTex( L"HQ_ImpossibleEqip.tga" );			
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,0.3f );
				} break;

			case SESAPT_LEVEL:
				{
					wstring wstrLevelTextureName = g_pData->GetSkillTree()->GetSkillLevelTextureName( val );
					wstring wstrLevelTextureKey = g_pData->GetSkillTree()->GetSkillLevelTextureKey( val );
					if( false == pPicture->SetTex( wstrLevelTextureName.c_str(), wstrLevelTextureKey.c_str() ) )
					{
						pPicture->SetTex( L"NoAlphaImage.dds" );
					}
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,1 );

					/// ���� �ؽ�ó ��ġ ����
					D3DXVECTOR2 vecPos					= pPicture->GetPos();
					pPicture->SetPos( D3DXVECTOR2( vecPos.x, vecPos.y + 30.f ) );

					D3DXVECTOR2 vecPicturePos			= pPicture->pPoint->leftTopPoint;
					pPicture->pPoint->leftBottomPoint	= D3DXVECTOR2( vecPicturePos.x, vecPicturePos.y + 14.f );
					pPicture->pPoint->rightTopPoint		= D3DXVECTOR2( vecPicturePos.x + 25.f, vecPicturePos.y );
					pPicture->pPoint->rightBottomPoint	= D3DXVECTOR2( vecPicturePos.x + 25.f, vecPicturePos.y + 14.f );

					if( true == bIsSkillLevelUpByItem )
					{
						pPicture->SetColor( D3DXCOLOR(0.2f, 1.0f, 0.2f, 1.f));
					}
					else
					{
						pPicture->SetColor( D3DXCOLOR(1, 1, 1, 1));
					}

				} break;
			default:
				{

				} break;
			}

			pSlot->AddPicture( (int)eType, pPicture );
			pPicture->SetShow(bShow);
		}
	}
	else
	{
		if(bShow)
		{
			switch(eType)
			{
			case SESAPT_LEVEL:
				{
					wstring wstrLevelTextureName = g_pData->GetSkillTree()->GetSkillLevelTextureName( val );
					wstring wstrLevelTextureKey = g_pData->GetSkillTree()->GetSkillLevelTextureKey( val );
					if( false == pPicture->SetTex( wstrLevelTextureName.c_str(), wstrLevelTextureKey.c_str() ) )
					{
						pPicture->SetTex( L"NoAlphaImage.dds" );
					}
					pPicture->pPoint->color = D3DXCOLOR( 1,1,1,1 );


					if( true == bIsSkillLevelUpByItem )
					{
						pPicture->SetColor( D3DXCOLOR(0.2f, 1.0f, 0.2f, 1.f));
					}
					else
					{
						pPicture->SetColor( D3DXCOLOR(1, 1, 1, 1));
					}
				} break;
			default:
				break;
			}

		}
		pPicture->SetShow(bShow);
	}

}

int CX2EquippedSkillSlotData::GetSlotID()		// 0~8�� ���Ǵ� ID�� �����ش�
{
	if(m_bSlotb)
	{
		return m_iIndex + EQUIPPED_SKILL_SLOT_COUNT;
	}
	else
	{
		return m_iIndex;
	}
}


//////////////////////////////////////////////////////////////////////////

CX2UISkillTreeNew::CX2UISkillTreeNew( CKTDXStage* pNowStage ) :
m_bShow(false),
m_bSkillRemoveMode(false),
m_bClassInfoVaild(false),
//m_MovedPosition(0,0),
//m_DLGPosition(0,0),
m_DLGSize(0,0),
m_pDLGUISkillTree(NULL),
m_pDLGSkillToolTip(NULL),
m_pDLGSkillUnsealInform(NULL),
m_pDLGSkillUnsealed( NULL ),
m_pUserSkillTree(NULL),
m_SumDelta(0),
m_iPrevSP(-1),
m_fNaviMoveRange(0.f),
m_pTalkBoxManager(NULL)
#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
, m_pDLGFirstGuide(NULL)
#endif


#ifdef REFORM_UI_SKILLSLOT
, m_bIsExpandSkillSlot( false )
, m_pDLGGameSkillSlot (NULL)
, m_bShowEquipSkillGuide (false)
#endif //REFORM_UI_SKILLSLOT
, m_bSkillSlotSwap( false )
#ifdef SKILL_SLOT_UI_TYPE_B 
, m_bIsSkillUITypeA( false )
#endif //SKILL_SLOT_UI_TYPE_B
, m_iMaxSkillRowNum( 0 )
, m_bNowLearnSkill( false )
, m_bNowInitSkill(false)
, m_bReplacedEsCounter( false )
{
	m_pTalkBoxManager = CX2TalkBoxManager::CreateTalkBoxManger();
	g_pKTDXApp->GetDGManager()->AddObjectChain( m_pTalkBoxManager );
	m_pTalkBoxManager->SetOverUI( true );
	m_bUsingCSP = false;	// ����ϴ� ���� ���µ� �ѵ�.....
#ifdef REFORM_UI_SKILLSLOT
	m_pDLGGameSkillSlot	= new CKTDGUIDialog( pNowStage, L"DLG_GAME_SKILL_SLOT_NEW.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGGameSkillSlot );
	m_pDLGGameSkillSlot->SetShow(false);
	UpdateSkillSlotString();
#endif //REFORM_UI_SKILLSLOT
	m_mapSkillPosition.clear();
}


CX2UISkillTreeNew::~CX2UISkillTreeNew(void)
{
	SAFE_DELETE_DIALOG( m_pDLGUISkillTree );	
	SAFE_DELETE_DIALOG( m_pDLGSkillToolTip );
	SAFE_DELETE_DIALOG( m_pDLGSkillUnsealInform );

	SAFE_DELETE_DIALOG( m_pDLGSkillUnsealed );

#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
	SAFE_DELETE_DIALOG( m_pDLGFirstGuide );
#endif
#ifdef ELSWORD_WAY_OF_SWORD
	SAFE_DELETE_DIALOG( m_pDLGElswordWSSDesc );
#endif ELSWORD_WAY_OF_SWORD

#ifdef REFORM_UI_SKILLSLOT
	SAFE_DELETE_DIALOG( m_pDLGGameSkillSlot );
#endif //REFORM_UI_SKILLSLOT

	SAFE_DELETE_KTDGOBJECT( m_pTalkBoxManager );


	m_mapSkillSlotUISet.clear();

}

void CX2UISkillTreeNew::SetLayer( X2_DIALOG_LAYER layer )
{
	if(m_pDLGUISkillTree != NULL) 
	{
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeLayer(m_pDLGUISkillTree, layer);
		g_pKTDXApp->GetDGManager()->GetDialogManager()->ChangeSequence( m_pDLGUISkillTree, false );
	}
}

void CX2UISkillTreeNew::SetPosition( D3DXVECTOR2 vec )
{
	// ���� ���̾�α�
	m_pDLGUISkillTree->SetPos(vec);
	//m_MovedPosition = vec;
	
}

HRESULT CX2UISkillTreeNew::OnFrameMove( double fTime, float fElapsedTime )
{	
	return S_OK;
}

bool CX2UISkillTreeNew::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// �տ� ��� ���̾�αװ� ������ �޽����� ó������ �ʴ´�
	if( true == g_pKTDXApp->GetDGManager()->GetDialogManager()->CheckFrontModalDlg( m_pDLGUISkillTree ) )
		return false;

	bool bFlag = false;
	switch( uMsg )
	{
		// ���콺 �޽����鿡 ����
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		//case WM_MOUSEWHEEL:
		{
#ifdef REFORM_UI_SKILLSLOT
			//��ų �巡�׽ÿ� �˸� �����ֵ��� ����
			for(map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator it =	m_mapSkillSlotUISet.begin(); it != m_mapSkillSlotUISet.end(); ++it)
			{
				SkillSlotUI& refSkillSlotUI = it->second;
				if ( NULL != refSkillSlotUI.m_pSlot && NULL != refSkillSlotUI.m_pSlot->GetSlotData() )
				{
					if( refSkillSlotUI.m_pSlot->GetState() == CKTDGUISlot::SCS_ONDRAG && uMsg != WM_LBUTTONUP) //�巡�� ��
					{
						ShowSkillDesc(false);
						refSkillSlotUI.m_pSlot->SetGuideDescLT(false);
						m_bShowEquipSkillGuide = true;
						break;
					}	 
					m_bShowEquipSkillGuide = false;
				}
			}
#endif //REFORM_UI_SKILLSLOT
			if ( false == m_pDLGUISkillTree->GetIsMouseOver())
			{
				return false;
			}
			bFlag = true;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			if ( false == m_pDLGUISkillTree->GetIsMouseOver())
			{
				return false;
			}
			g_pData->GetUIManager()->SetUILayerIndex(CX2UIManager::UI_MENU_SKILL, true);
			bFlag = true;		
		}
		break;
	default:
		break;
	}

	switch( uMsg )
	{
	case WM_MOUSEWHEEL:
		{ 
			if( m_pDLGUISkillTree->GetIsMouseOver() ) 
				OnMouseWheel(hWnd, uMsg, wParam,lParam);
		}break;
	}

	return bFlag;
}

bool CX2UISkillTreeNew::OnMouseWheel(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CKTDGUIContolList* pControlList = (CKTDGUIContolList*) m_pDLGUISkillTree->GetControl( L"SkillTreeControlList" );
	if( pControlList == NULL )
		return false;
	CKTDGUIScrollBar* pScroolBarY = pControlList->GetYScroolBar();
	if(pScroolBarY == NULL)
		return false;

	short zDelta = (short)HIWORD(wParam);
	m_SumDelta += zDelta;

#if 0
	if(abs(m_SumDelta) < WHEEL_DELTA)
		return false;

	pScroolBarY->Scroll(-m_SumDelta / WHEEL_DELTA);
	m_SumDelta %= WHEEL_DELTA;
#else
	while (abs(m_SumDelta) >= WHEEL_DELTA)
	{
		if(m_SumDelta>0)
		{
			pScroolBarY->Scroll(-1);
			m_SumDelta -= WHEEL_DELTA;
		}
		else
		{	
			pScroolBarY->Scroll(1);
			m_SumDelta += WHEEL_DELTA;
		}	

	}
#endif

	UpdateAtMoveScrollBar();
	return true;
}

bool CX2UISkillTreeNew::UICustomEventProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(wParam)
	{
	case STUCM_UNSEAL_SKILL_CANCLE:
		{
			if( m_pDLGSkillUnsealInform != NULL )
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGSkillUnsealInform, NULL, false );
				m_pDLGSkillUnsealInform = NULL;
			}			
		} break;

	case STUCM_UNSEAL_SKILL_BUY:
		{
			//** ��ų ���� ��ư�� ������ �� ���⿡ �۾��� �ּ���.

		} break;

	case STUCM_CSP_GUIDE_MOUSEIN:
		{	
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2.f;
			pos.y += pControl->GetHeight();
						
			if( NULL != g_pData->GetSkillTree() &&
				NULL != g_pData->GetMyUser() &&
				NULL != g_pData->GetMyUser()->GetSelectUnit() && 
				NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
			{
				CX2UserSkillTree& refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;
				int			iSPoint			= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iSPoint;	/// ���� ��ų ����Ʈ
				int			iCSPoint		= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_iCSPoint;	/// ���� ĳ�� ��ų ����Ʈ
				const int	iUnitLevel		= g_pData->GetSelectUnitLevel();	/// ���� ����
				const int	iInitSkillPoint	= g_pData->GetSkillTree()->GetCalcInitSkillPoint( iUnitLevel );		/// ������ Ư�� ������ ���� �� �ִ� ��ų ����Ʈ��
				int			iUsedSPoint		= 0;	/// ����� ��ų ����Ʈ
				int			iUsedCSPoint	= 0;	/// ����� ĳ�� ��ų ����Ʈ

				refUserSkillTree.CalcUsedSPointAndCSPoint( iUsedSPoint,iUsedCSPoint );	/// ����� ��ų ����Ʈ ����
				const int iMaxCSPoint = iSPoint + iCSPoint + iUsedSPoint + iUsedCSPoint - iInitSkillPoint;	/// �׳�ý��� ���� �ö� �� SP ����

				WCHAR wszText[32] = L"";
				StringCchPrintfW( wszText, ARRAY_SIZE(wszText), L"(%dSP)\n", iMaxCSPoint );
				
				wstring wstr = L"";
				wstr += GET_STRING(STR_ID_2689);
				wstr += wszText;
				wstr += GetExpirationDateDesc( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_wstrCSPointEndDate, g_pData->GetServerCurrentTime() );

				ShowUIDesc(true, wstr, pos, CX2TalkBoxManagerImp::TBT_FROM_UP_LEFT, D3DXCOLOR(1,1,1,1), D3DXCOLOR(0.97f, 0.23f, 0.06f, 1) );			
			}
			
			return true;
		} break;

	case STUCM_CSP_GUIDE_MOUSEOUT:
		{
			ShowUIDesc(false);
			return true;
		} break;

#ifndef REFORM_UI_SKILLSLOT
	case STUCM_SKILLTREESLOTB_GUIDE_MOUSEIN:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2.f;
			
			if(g_pData->GetMyUser() != NULL &&
				g_pData->GetMyUser()->GetSelectUnit() != NULL && 
				g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() != NULL )
			{
				CX2UserSkillTree& refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;
				wstring wstr = L"";
				wstr += GET_STRING(STR_ID_2690);
				wstr += GetExpirationDateDesc( refUserSkillTree.GetSkillSlotBEndDateString(), g_pData->GetServerCurrentTime() );
				ShowUIDesc(true, wstr, pos, CX2TalkBoxManagerImp::TBT_FROM_DOWN_LEFT, D3DXCOLOR(1,1,1,1), D3DXCOLOR(0.97f, 0.23f, 0.06f, 1) );			
			}
						 
			return true;
		} break;

	case STUCM_SKILLTREESLOTB_GUIDE_MOUSEOUT:
		{
			ShowUIDesc(false);
			return true;
		} break;
#endif //REFORM_UI_SKILLSLOT

#ifdef GUIDE_SKILL_SLOTB
	case STUCM_SKILLTREESLOTB_BUY_GUIDE_MOUSEIN:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2.f; 
			ShowUIDesc( true, GET_STRING(STR_ID_16097), pos, CX2TalkBoxManagerImp::TBT_FROM_DOWN_LEFT ); 

			return true;
		} break;

	case STUCM_SKILLTREESLOTB_BUY_GUIDE_MOUSEOUT:
		{
			ShowUIDesc(false);
			return true;
		} break; 
#endif GUIDE_SKILL_SLOTB

#ifdef GUIDE_SKILL_LEVEL_MINUS
	case STUCM_BUTTON_SKILL_LEVEL_MINUS_GUIDE_MOUSEOVER:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2;
			pos.y += pControl->GetHeight() / 2.f;
			ShowUIDesc( true, GET_STRING(STR_ID_16099), pos );
			return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_MINUS_GUIDE_MOUSEOUT:
		{
			ShowUIDesc(false);
			return true;
		} break; 
#endif GUIDE_SKILL_LEVEL_MINUS

#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
	case STUCM_FIRST_GUIDE_CLOSE:
		{
			if(m_pDLGFirstGuide != NULL)
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGFirstGuide, NULL, false );
			}
			m_pDLGFirstGuide = NULL;
			return true;
		} break;
#endif OPEN_SKILL_GUIDE_FIRST_TIME

#ifdef ELSWORD_WAY_OF_SWORD
	case STUCM_INGAME_ELSWORD_WSS_MOUSE_OVER:
		{
			if( CX2Unit::UT_ELSWORD == g_pData->GetMyUser()->GetSelectUnit()->GetType()
#ifdef NEW_CHARACTER_EL
				|| CX2Unit::UT_ELESIS == g_pData->GetMyUser()->GetSelectUnit()->GetType()
#endif //NEW_CHARACTER_EL
				)
			{
				CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
				D3DXVECTOR2 vControlPos = pControl->GetPos();
				DrawElswordWSSDesc( vControlPos );
			}
			return true;
		} break;
	case STUCM_INGAME_ELSWORD_WSS_MOUSE_OUT:
		{
			if( m_pDLGElswordWSSDesc != NULL )
				m_pDLGElswordWSSDesc->SetShow(false);
			return true;
		} break;
#endif ELSWORD_WAY_OF_SWORD

	case STUCM_SKILL_RESET_MODE_CHANGE_MOUSEOVER:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2.f;
			ShowUIDesc( true, GET_STRING(STR_ID_2667), pos, CX2TalkBoxManagerImp::TBT_FROM_DOWN_LEFT);

			return true;
		} break;

	case STUCM_SKILL_RESET_MODE_CHANGE_MOUSEOUT:
		{
			ShowUIDesc( false );
			return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_PLUS_MOUSEOVER:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2;
			pos.y += pControl->GetHeight() / 2.f;
			ShowUIDesc( true, GET_STRING(STR_ID_2663), pos );

			return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_PLUS_MOUSEOUT:
		{
			ShowUIDesc( false );
			return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_MINUS_MOUSEOVER:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2;
			pos.y += pControl->GetHeight() / 2.f;
			ShowUIDesc( true, GET_STRING(STR_ID_2664), pos );

				return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_MINUS_MOUSEOUT:
		{
			ShowUIDesc( false );
			return true;
		} break;

	case STUCM_BUTTON_SKILL_MASTER_MOUSEOVER:
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			D3DXVECTOR2 pos = pControl->GetPos() + pControl->GetDialog()->GetPos();
			pos.x += pControl->GetWidth() / 2;
			pos.y += pControl->GetHeight() / 2.f;
			ShowUIDesc( true, GET_STRING(STR_ID_2665), pos );

				return true;
		} break;

	case STUCM_BUTTON_SKILL_MASTER_MOUSEOUT:
		{
			ShowUIDesc( false );
			return true;
		} break;

	case STUCM_CONTROLLIST_SCROLL:
		{
			UpdateAtMoveScrollBar();	/// ��ũ�� �ٰ� ������ �� ����Ǿ�� �� ������ ó���ϴ� �Լ�

			UpdateBlind();			/// ��ų ��Ȳ�� ���� ó���ؾ� �ϴ� ��ų ���� ����
		
			return true;
		} break;
	case STUCM_SKILLTREESLOT_MOUSEIN:
		{
			CKTDGUISlot* pSlot = (CKTDGUISlot*) lParam;
			CX2SkillTreeSlotData* pSlotData = (CX2SkillTreeSlotData*) pSlot->GetSlotData();
			if(pSlotData == NULL || pSlotData->m_eSkillID == CX2SkillTree::SI_NONE )
			{
				ShowSkillDesc(false);
				return true;
			}

			D3DXVECTOR2 pos = pSlot->GetPos() + pSlot->GetOffsetPos() + pSlot->GetDialog()->GetPos();
			D3DXVECTOR2 size = pSlot->GetSize();

			ShowSkillDesc(true, pSlotData->m_eSkillID, pos, size, false );		
			return true;

		} break;

	case STUCM_EQUIPSLOT_MOUSEIN:
		{
			CKTDGUISlot* pSlot = (CKTDGUISlot*) lParam;
			CX2SkillTreeSlotData* pSlotData = (CX2SkillTreeSlotData*) pSlot->GetSlotData();
			if(pSlotData == NULL || pSlotData->m_eSkillID == CX2SkillTree::SI_NONE )
			{
				D3DXVECTOR2 pos = pSlot->GetPos() + pSlot->GetDialog()->GetPos();
				pos.x = pos.x + pSlot->GetSize().x * 0.5f;
				ShowUIDesc( true, GET_STRING(STR_ID_2666), pos, CX2TalkBoxManagerImp::TBT_FROM_DOWN_LEFT );

				ShowSkillDesc(false);
				return true;
			}

			D3DXVECTOR2 pos = pSlot->GetPos() + pSlot->GetOffsetPos() + pSlot->GetDialog()->GetPos();
			D3DXVECTOR2 size = pSlot->GetSize();
			ShowSkillDesc(true, pSlotData->m_eSkillID, pos, size, true );			
			return true;

		} break;

	case STUCM_SKILLTREESLOT_MOUSEOUT:
	case STUCM_EQUIPSLOT_MOUSEOUT:
		{			
			ShowUIDesc( false );
			ShowSkillDesc(false);
			return true;

		} break;

	case STUCM_SKILLTREESLOT_RMOUSEUP:
		{
		//{{ kimhc // 2010.3.26 // ���� ��ų ���� ����
		#ifdef	MODIFY_INFINITE_SKILL_BUG
			if ( g_pMain->GetNowState() != NULL)
			{
				CX2State* pX2State = static_cast<CX2State*>( g_pMain->GetNowState() );
				if ( pX2State->CheckIsReady() == true )
				{
					pX2State->ShowOkAndCancelMsgForUnReadyBySkill();
					return true;
				}
			}
		#endif	MODIFY_INFINITE_SKILL_BUG
			//}} kimhc // 2010.3.26 // ���� ��ų ���� ����			

			return ProcessSkillTreeSlotRMouseUp( lParam );

		} break;

	case STUCM_BUTTON_EXIT:
		{
			if ( NULL != g_pData && NULL != g_pData->GetSkillTree() )
			{
				/// ��ų �ʱ�ȭ ���°� �ƴϰ�, ������ �ø����� �ϴ� ��ų�� ���� �� ���� �˾� ����
				if ( false == m_bNowInitSkill && false == g_pData->GetSkillTree()->GetMapSkillInfo().empty() )
				{
					g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(-999,-999), GET_STRING( STR_ID_25109 ), 
						STUCM_EXIT_WITH_SKILL_LEVEL_PLUS, g_pMain->GetNowState(), STUCM_EXIT );

					m_bNowLearnSkill = false;
				}
				/// ���ٸ�, �ٷ� �ݱ�
				else
				{
					m_bNowInitSkill = false;
					m_bNowLearnSkill = false;
					SetShow(false);
				}
			}

			return true;
		} break;

	case STUCM_EXIT:
		{
			CKTDGUIControl* pControl = (CKTDGUIControl*)lParam;

			if ( NULL != pControl )
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );

			 SetShow(false);

			 return true;
		} break;

	case STUCM_OPEN_COMBO_TREE:
		{
			//g_pData->GetUIManager()->ToggleUI( CX2UIManager::UI_MENU_SKILL );
			SetShow(false);
			g_pData->GetUIManager()->ToggleUI( CX2UIManager::UI_MENU_COMBO_TREE );
			return true;
		} break;

	case STUCM_EQUIPSLOT_DROPPED:
		{
			if ( NULL != g_pX2Game )
			{
				if ( g_pX2Game->GetGameType() == CX2Game::GT_PVP ||
					g_pX2Game->CheckAndWarningBusyStateNow() )
					return true;
			}

			//{{ kimhc // 2010.3.26 // ���� ��ų ���� ����
#ifdef	MODIFY_INFINITE_SKILL_BUG
			if ( g_pMain->GetNowState() != NULL)
			{
				CX2State* pX2State = static_cast<CX2State*>( g_pMain->GetNowState() );
				if ( pX2State->CheckIsReady() == true )
				{
					pX2State->ShowOkAndCancelMsgForUnReadyBySkill();
					return true;
				}
#ifdef REFORM_UI_SKILLSLOT
				UpdateSkillEquipedSlotUI();
#endif //REFORM_UI_SKILLSLOT
			}
#endif	MODIFY_INFINITE_SKILL_BUG
			//}} kimhc // 2010.3.26 // ���� ��ų ���� ����

			return ProcessEquipSlotDropped( lParam );

		} break;

	case STUCM_BUTTON_SKILL_LEVEL_PLUS:					/// ��ų ���� ��ư�� ������ ��
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			CX2SkillTree::SKILL_ID eSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDummyInt(0) );

			if ( NULL != g_pData && NULL != g_pData->GetSkillTree() )
			{
				const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );	/// ��ų ���ø�

				if ( NULL != pSkillTemplet )
				{
					wstring wstrSkillName	=  L"#CFF0000";
					wstrSkillName			+= pSkillTemplet->m_wstrName;
					wstrSkillName			+= L"#CX";

					/// �޽��� �ڽ� ��Ʈ��
					const wstring wstrMsgString	= GET_REPLACED_STRING( ( STR_ID_25106, "L", wstrSkillName ) );

					CKTDGUIDialogType m_pDLGMsgBox = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(-999,-999), wstrMsgString.c_str(), 
																		STUCM_CHANGE_SKILL_LEVEL_PLUS, g_pMain->GetNowState() );

					m_pDLGMsgBox->AddDummyInt( static_cast<int>( eSkillID ) );
				}
			}

			return true;
		} break;

	case STUCM_CHANGE_SKILL_LEVEL_PLUS:					/// ��ų ���� ó��
		{
			CKTDGUIControl* pControl = (CKTDGUIControl*)lParam;

			if ( NULL != pControl && NULL != pControl->GetDialog() )
			{
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );

				if ( false == m_bNowLearnSkill )
				{
					g_pData->GetSkillTree()->SetUsedSPoint( 0 );
					g_pData->GetSkillTree()->SetUsedCSPoint( 0 );
				}

				m_bNowLearnSkill = true;
				CX2SkillTree::SKILL_ID eCheckSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDialog()->GetDummyInt( 0 ) );

				if ( CX2SkillTree::SI_NONE != eCheckSkillID )
					SetLearnSkillInfo( eCheckSkillID );
			}

			return true;
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_UPGRADE:				/// ��ų ��ȭ ��ư�� ������ ��
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			CX2SkillTree::SKILL_ID eSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDummyInt(0) );

			if ( false == m_bNowLearnSkill )
			{
				g_pData->GetSkillTree()->SetUsedSPoint( 0 );
				g_pData->GetSkillTree()->SetUsedCSPoint( 0 );

				m_bNowLearnSkill = true;
			}

			if ( CX2SkillTree::SI_NONE != eSkillID )
				SetLearnSkillInfo( eSkillID );
		} break;

	case STUCM_BUTTON_SKILL_LEVEL_PLUS_RESULT:			/// �����ϱ� ��ư ������ ��
		{

			if ( NULL != g_pData && NULL != g_pData->GetSkillTree() )
			{
				g_pData->GetSkillTree()->Handler_EGS_GET_SKILL_REQ();

				g_pData->GetSkillTree()->SetUsedSPoint( 0 );
				g_pData->GetSkillTree()->SetUsedCSPoint( 0 );
				g_pData->GetSkillTree()->ClearMapSkillInfo();
			}

			m_bNowLearnSkill = false;

			return true;
		}  break;

	case STUCM_BUTTON_SKILL_LEVEL_PLUS_CANCEL:			/// ��� ��ư�� ������ ��
		{
			if ( NULL != g_pData && NULL != g_pData->GetSkillTree() )
			{
				g_pData->GetSkillTree()->SetUsedSPoint( 0 );
				g_pData->GetSkillTree()->SetUsedCSPoint( 0 );
				g_pData->GetSkillTree()->ClearMapSkillInfo();
			}

			m_bNowLearnSkill = false;

			UpdateUI( false, true, true );

			return true;
		} break;

	case STUCM_EXIT_WITH_SKILL_LEVEL_PLUS:				/// �����鼭 ��ų ���� ���� ó��
		{
			CKTDGUIControl* pControl = (CKTDGUIControl*)lParam;

			if ( NULL != pControl )
				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );

			if ( NULL != g_pData && NULL != g_pData->GetSkillTree() )
			{
				g_pData->GetSkillTree()->Handler_EGS_GET_SKILL_REQ();

				g_pData->GetSkillTree()->SetUsedSPoint( 0 );
				g_pData->GetSkillTree()->SetUsedCSPoint( 0 );
				g_pData->GetSkillTree()->ClearMapSkillInfo();
			}
			m_bNowLearnSkill = false;

			SetShow(false);
		} break;

	
	case STUCM_EQUIPSLOT_RMOUSEUP:
		{
	//{{ kimhc // 2010.3.26 // ���� ��ų ���� ����
	#ifdef	MODIFY_INFINITE_SKILL_BUG
			if ( g_pMain->GetNowState() != NULL)
			{
				CX2State* pX2State = static_cast<CX2State*>( g_pMain->GetNowState() );
				if ( pX2State->CheckIsReady() == true )
				{
					pX2State->ShowOkAndCancelMsgForUnReadyBySkill();
					return true;
				}
			}
	#endif	MODIFY_INFINITE_SKILL_BUG
			//}} kimhc // 2010.3.26 // ���� ��ų ���� ����

			// �ٻ� ���� üũ
			if ( NULL != g_pX2Game && g_pX2Game->CheckAndWarningBusyStateNow() )
				return true;

#ifdef REFORM_UI_SKILLSLOT // ��ųâ �������� �� �� ������ ��ų ���� �� �� �ֵ��� ����
			if ( false == m_bShow ) 
			{
				return true;
			}
#endif //REFORM_UI_SKILLSLOT

			return ProcessEquipSlotRMouseUp( lParam );
		} break;

	case STUCM_SKILL_RESET_MODE_CHANGE:
		{
			m_bSkillRemoveMode = !m_bSkillRemoveMode;
			UpdateUI(false, false, true);
 
			return true;
		} break;

	case STUCM_BUTTON_ENABLE_INIT_SKILL:		/// ��ų Ʈ�� �� �ʱ�ȭ ��ư
		{
			m_bNowLearnSkill	= false;					/// ��ų ���� �������̾��ٸ�, ĵ��
			g_pData->GetSkillTree()->ClearMapSkillInfo();	/// �ӽ÷� ������ ��ų ���� �ʱ�ȭ

			if ( false == m_bNowInitSkill )	/// �ʱ�ȭ ��ư Ȱ�� ��ų ��, Ȱ�� ���� �˻�
			{
				if ( NULL != g_pData && NULL != g_pData->GetMyUser()  )
				{
					CX2Unit* pMyUnit = g_pData->GetMyUser()->GetSelectUnit();

					if( NULL != pMyUnit && NULL != pMyUnit->GetInventory() )
					{
						/// �ʱ�ȭ �������� �����ϰ� �ִ°�
						if( NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_ITEM_ID ) ||
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_EVENT_ITEM_ID ) ||
#ifdef SERV_EVENT_RESET_A_SKILL_ITEM
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_ITEM ) ||
#endif SERV_EVENT_RESET_A_SKILL_ITEM
#ifdef SERV_QUEST_SKILL_RESET
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_QUEST_ITEM_ID ) ||
#endif SERV_QUEST_SKILL_RESET
#ifdef SERV_2ND_CLASS_SKILL_RESET
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_2ND_CLASS_CHANGE_EVENT_ITEM_ID ) ||
#endif SERV_2ND_CLASS_SKILL_RESET
#ifdef SERV_EVENT_RURIEL_RESET_SKILL_ITEM
							NULL != pMyUnit->GetInventory()->GetItemByTID( RURIEL_RESET_A_SKILL_ITEM ) ||
							NULL != pMyUnit->GetInventory()->GetItemByTID( RURIEL_RESET_A_SKILL_EVENT_ITEM ) ||
#endif SERV_EVENT_RURIEL_RESET_SKILL_ITEM
#ifdef SERV_EVENT_CASH_SKILL_POINT_ITEM_JP
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_EVENT_ITEM_ID_BY_JP ) ||
#endif //SERV_EVENT_CASH_SKILL_POINT_ITEM_JP
							NULL != pMyUnit->GetInventory()->GetItemByTID( RESET_A_SKILL_ITEM_NOT_TRADE_ID ) )
						{
							m_bNowInitSkill = true;		/// ���� ������, �ʱ�ȭ Ȱ��
						}
						else
						{
							/// �� ���� �� �̶��, �ʱ�ȭ ������ ȹ�� �ȳ�â ǥ��
							g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_25175 ), g_pMain->GetNowState() );
						}
					}
				}
			}
			else	/// �ʱ�ȭ ��ư ��Ȱ�� �� ���� ���� �˻� �ʿ���� ��Ȱ�� ��Ű��
			{
				m_bNowInitSkill = false;
			}

			UpdateUI( false, true, true );

			return true;
		} break;

	case STUCM_BUTTON_INIT_SKILL:		/// ��ų ���� �ʱ�ȭ ��ư
		{
			CKTDGUIButton* pControl = (CKTDGUIButton*)lParam;
			CX2SkillTree::SKILL_ID eSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDummyInt(0) );

			if ( NULL != g_pData && 
				 NULL != g_pData->GetSkillTree() &&
				 NULL != g_pData->GetMyUser() &&
				 NULL != g_pData->GetMyUser()->GetSelectUnit() &&
				 NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
			{
				/// ���� Ŭ���� Ÿ��
				CX2Unit::UNIT_CLASS						eUnitClass			= static_cast<CX2Unit::UNIT_CLASS>( g_pData->GetMyUser()->GetSelectUnit()->GetClass() );
				/// ��ų Ʈ�� ���ø�
				const CX2SkillTree::SkillTreeTemplet*	pSkillTreeTemplet	= g_pData->GetSkillTree()->GetSkillTreeTemplet( eUnitClass, eSkillID );
				/// ��ų ���ø�
				const CX2SkillTree::SkillTemplet*		pSkillTemplet		= g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );	/// ��ų ���ø�
				/// ���� ��ų ����
				CX2UserSkillTree&						refUserSkillTree	= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;

				if ( NULL != pSkillTreeTemplet && NULL != pSkillTemplet )
				{
					const int				iSkillLevel				= refUserSkillTree.GetSkillLevel( eSkillID );						/// ��ų ����
					const int				iUnitLevel				= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_Level;	/// ���� ����
					CX2SkillTree::SKILL_ID	eAnotherSkillID			= GetAnotherSelectSkillID( eSkillID );	/// �ݴ��� ��ų ���̵�
					bool					bEnoughLevel			= false;								/// �䱸 ���� ���� ���� ����
					
					/// ���� ��ų�� ���� ��ų�� �� ��ų�� ���� �ִ��� �˻�
					const bool bHaveFollowingSkill = 0 < refUserSkillTree.GetSkillLevel( static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iFollowingSkill ) ) ? true : false ;

					/// �⺻ ��ų �̰ų�, ���� ��ų�� ���� ��ų�� �� ��ų�� ���� �ִٸ�
					if ( true == g_pData->GetSkillTree()->isDefaultSkill( eSkillID ) || true == bHaveFollowingSkill )
					{
						/// ��ų ���� 1�� �ʱ�ȭ ��Ų�ٴ� �˾� �ȳ�
						CKTDGUIDialogType m_pDLGMsgBox = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(-999,-999), 
							GET_REPLACED_STRING( ( STR_ID_25295, "L", pSkillTemplet->m_wstrName ) ), STUCM_INIT_SKILL_ONE_POINT, g_pMain->GetNowState() );

						if ( NULL != m_pDLGMsgBox )
							m_pDLGMsgBox->AddDummyInt( static_cast<int>( eSkillID ) );
					}
					else
					{
						/// ��ų ���� 0���� �ʱ�ȭ ��Ų�ٴ� �˾� �ȳ�
						CKTDGUIDialogType m_pDLGMsgBox = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2(-999,-999), 
							GET_REPLACED_STRING( ( STR_ID_25174, "L", pSkillTemplet->m_wstrName ) ), STUCM_INIT_SKILL, g_pMain->GetNowState() );

						if ( NULL != m_pDLGMsgBox )
							m_pDLGMsgBox->AddDummyInt( static_cast<int>( eSkillID ) );
					}
				}
			}

			return true;
		} break;

	case STUCM_INIT_SKILL:				/// ��ų ���� �ʱ�ȭ ó�� ( 0���� )
		{
			CKTDGUIControl* pControl = (CKTDGUIControl*)lParam;

			if ( NULL != pControl && NULL != pControl->GetDialog() )
			{
				CX2SkillTree::SKILL_ID eCheckSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDialog()->GetDummyInt( 0 ) );

				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );

				if ( NULL != g_pData && NULL != g_pData->GetSkillTree() && CX2SkillTree::SI_NONE != eCheckSkillID  )
				{
					m_bNowInitSkill = false;
					g_pData->GetSkillTree()->Handler_EGS_RESET_SKILL_REQ( eCheckSkillID );
				}
			}

			return true;
		} break;

	case STUCM_INIT_SKILL_ONE_POINT:	/// ��ų ���� �ʱ�ȭ ó�� ( 1�� )
		{
			CKTDGUIControl* pControl = (CKTDGUIControl*)lParam;

			if ( NULL != pControl && NULL != pControl->GetDialog() )
			{
				CX2SkillTree::SKILL_ID eCheckSkillID = static_cast<CX2SkillTree::SKILL_ID>( pControl->GetDialog()->GetDummyInt( 0 ) );

				g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, pControl->GetDialog(), NULL, false );

				if ( NULL != g_pData && NULL != g_pData->GetSkillTree() && CX2SkillTree::SI_NONE != eCheckSkillID  )
				{
					m_bNowInitSkill = false;
					g_pData->GetSkillTree()->Handler_EGS_RESET_SKILL_REQ( eCheckSkillID );
				}
			}

			return true;
		} break;

	default:
		break;
	}
	return false;
}

void CX2UISkillTreeNew::SetShow(bool val)
{
	m_bShow = val;

	if(val)	// ���� �� ó���ؾ� �� �κ�
	{
#ifdef FIX_INTRUSION_SKILL_BUG
		//���� ���۽� ��ųâ �� �� ������ ����
		if( CX2Main::XS_PVP_ROOM == g_pMain->GetNowStateID())
		{
			CX2StatePVPRoom* pState = (CX2StatePVPRoom*) g_pMain->GetNowState();
			if( NULL != pState )
			{
				if( true == pState->GetDisableSkillUIOpen() )
				{
					m_bShow = false;
					return;
				}
			}
		}
#endif //FIX_INTRUSION_SKILL_BUG
#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
		if( g_pData != NULL && g_pData->GetSelectUnitLevel() <= 10 )
		{
			const string strFileName = "readme.txt";
			FILE* file = NULL;
			file = fopen( strFileName.c_str(), "r" );		
			if( NULL == file )
			{
				file = fopen( strFileName.c_str(), "w" );
				fclose(file);

				if( m_pDLGFirstGuide == NULL )
				{
					m_pDLGFirstGuide = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_Help_Skill.lua" );
					g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGFirstGuide );
					m_pDLGFirstGuide->SetDisableUnderWindow(true);					
				}		
				m_pDLGFirstGuide->SetShow(true);
			}
			else
			{
				fclose(file);
			}		
		}
#endif OPEN_SKILL_GUIDE_FIRST_TIME

		m_bSkillRemoveMode = false;
		UpdateUI(true, true, true);

		g_pData->GetUIManager()->UIOpened( CX2UIManager::UI_MENU_SKILL );

#ifdef REFORM_UI_SKILLSLOT
		if( CX2Main::XS_PVP_ROOM == g_pMain->GetNowStateID() )
			g_pData->GetUIManager()->SetShowSkillSlot(true);
		
#endif //REFORM_UI_SKILLSLOT

#ifdef SKILL_SLOT_UI_TYPE_B
		SetSkillUIType(true);
#endif //SKILL_SLOT_UI_TYPE_B
	}
	else	// ���� �� ó���ؾ� �� �κ�
	{
#ifdef OPEN_SKILL_GUIDE_FIRST_TIME
		if(m_pDLGFirstGuide != NULL)
			m_pDLGFirstGuide->SetShow(false);
#endif
		ShowSkillDesc(false);
		ShowUIDesc(false);
		SAFE_DELETE_DIALOG(m_pDLGSkillUnsealInform);

#ifdef REFORM_UI_SKILLSLOT
		m_bShowEquipSkillGuide = false;
#endif //REFORM_UI_SKILLSLOT		
		g_pData->GetUIManager()->UIClosed(CX2UIManager::UI_MENU_SKILL);

#ifdef REFORM_UI_SKILLSLOT
		if( CX2Main::XS_PVP_ROOM == g_pMain->GetNowStateID() )
			g_pData->GetUIManager()->SetShowSkillSlot(false);
#endif //REFORM_UI_SKILLSLOT

#ifdef SKILL_SLOT_UI_TYPE_B //�ɼ����� ���� �ϰԵǸ� �ɼ��ʿ��� ���������� ����
		if( NULL != g_pMain && NULL != g_pMain->GetGameOption() )
		{
			SetSkillUIType(g_pMain->GetGameOption()->GetIsSkillUITypeA());
		}
#endif //SKILL_SLOT_UI_TYPE_B

		m_bNowLearnSkill	= false;	/// ��ų ���� ���� ����
		m_bNowInitSkill		= false;	/// ��ų �ʱ�ȭ ���� ����

		if (	NULL != g_pData &&
			NULL != g_pData->GetSkillTree() )
		{
			g_pData->GetSkillTree()->ClearMapSkillInfo();
			g_pData->GetSkillTree()->SetUsedSPoint( 0 );
			g_pData->GetSkillTree()->SetUsedCSPoint( 0 );
		}
	}
	if( NULL != m_pDLGUISkillTree )
		m_pDLGUISkillTree->SetShowEnable(m_bShow, m_bShow);
}

// ĳ���Ͱ� ���õ��� �� / �������� ���� �� �ش�.
void CX2UISkillTreeNew::InitSkillTreeUI()
{
	SAFE_DELETE_DIALOG(m_pDLGUISkillTree);
	
	m_bSkillSlotSwap = false;

	m_mapSkillSlotUISet.clear();
	m_pDLGUISkillTree = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Skill_Tree_New.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGUISkillTree );
	m_pDLGUISkillTree->SetDisableUnderWindow(true);
	m_pDLGUISkillTree->SetShow(false);


	D3DXVECTOR3 tmp;
	tmp = m_pDLGUISkillTree->GetDummyPos(1);
 	m_DLGSize.x = tmp.x;
 	m_DLGSize.y = tmp.y;

	

	if( g_pData->GetMyUser() != NULL &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL && 
		g_pData->GetMyUser()->GetSelectUnit()->GetUnitTemplet() != NULL &&
		g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() != NULL )
	{
		m_pUserSkillTree = &(g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree);

		const CX2Unit::UnitTemplet* pUnitTemplet = g_pData->GetMyUser()->GetSelectUnit()->GetUnitTemplet();

		CreateSkillTree( pUnitTemplet->m_UnitClass );
		InitEquippedSkillSlotData();

	}	
}

void CX2UISkillTreeNew::CreateSkillTree( CX2Unit::UNIT_CLASS eUnitClass )
{

	if( NULL == m_pDLGUISkillTree ||
		NULL == g_pData ||
		NULL == g_pData->GetSkillTree() ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		return;
	}

	CKTDGUIContolList*	pControlList	= static_cast<CKTDGUIContolList*>( m_pDLGUISkillTree->GetControl( L"SkillTreeControlList" ) );	/// ��ũ�� ��ü
	if ( NULL == pControlList )
		return;

	// Unit Class�� �̿��ؼ� �ش� Map�� ��Ե� �޾ƿ´�. �Դٰ� ġ��.
	const CX2SkillTree::SkillTreeTempletMap& refSkillTreeTempletMap = g_pData->GetSkillTree()->GetSkillTreeTempletMap( (int)eUnitClass );

	for( CX2SkillTree::SkillTreeTempletMap::const_iterator mapit = refSkillTreeTempletMap.begin(); mapit != refSkillTreeTempletMap.end(); ++mapit )
	{
		CX2SkillTree::SKILL_ID eSkillID = mapit->first;
		const CX2SkillTree::SkillTreeTemplet& refSkillTreeTemplet = mapit->second;


		/// ���� ���� SP ��ȯ
		CX2Unit::UnitData* pUnitData = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();

		int iNoUseSP = 0;
		int iNoUseCP = 0;

		if ( NULL != pUnitData )
		{
			iNoUseSP = pUnitData->m_iSPoint;		/// ���� ��ų ����Ʈ
			iNoUseCP = pUnitData->m_iCSPoint;	/// ���� ĳ�� ��ų ����Ʈ ( �׳�ý��� �ູ )
		}

		/// ���� ��ų ������ ���� ��ȯ
		CX2UserSkillTree&	refUserSkillTree	= pUnitData->m_UserSkillTree;
		const int			iSkillLevel			= refUserSkillTree.GetSkillLevel( eSkillID );

		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
		if(pSkillTemplet == NULL)
		{
#ifndef _SERVICE_
			WCHAR buf[256] = {0,};
			//wsprintf(buf, L"Skill ID : %d\nExist in SkillTreeTemplet but NULL SkillTemplet", (int)eSkillID);
			StringCchPrintf(buf, 256, L"Skill ID : %d\nExist in SkillTreeTemplet but NULL SkillTemplet", (int)eSkillID);
			MessageBox( g_pKTDXApp->GetHWND(), buf, L"Error", MB_OK );
#endif
			continue;
		}

		int row = refSkillTreeTemplet.m_iTier;
		int col = refSkillTreeTemplet.m_iIndexInTier;
		
		/// ��� ���� ���� ��ų ���̵� ����
		std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator it = m_mapSkillPosition.find( SkillPositionKey( col, row ) );
		if( it == m_mapSkillPosition.end() )
		{
			m_mapSkillPosition.insert( make_pair( SkillPositionKey( col, row ), eSkillID ) );
		}

		SkillSlotUI UISet;

		CreateSlotUIPreset( UISet, static_cast<int>( eSkillID ), col );		/// �� ���Ժ� UI ����

		/// ��ũ�� ����Ʈ�� ���� ���� UI ��Ʈ�� �߰�
		pControlList->InsertItem( UISet.m_pSlot,					row, col );
		pControlList->InsertItem( UISet.m_pStaticBG,				row, col );
		pControlList->InsertItem( UISet.m_pStaticSkillBG,			row, col );
		pControlList->InsertItem( UISet.m_pStaticSkillLock,			row, col );
		pControlList->InsertItem( UISet.m_pButtonSkillInit,			row, col );
		pControlList->InsertItem( UISet.m_pButtonSkillDesc,			row, col );
		pControlList->InsertItem( UISet.m_pButtonSkillLearn,		row, col );
		pControlList->InsertItem( UISet.m_pStaticSkillMaster,		row, col );
		pControlList->InsertItem( UISet.m_pStaticSkillNoLearn,		row, col );
		pControlList->InsertItem( UISet.m_pStaticStringRequireSP,	row, col );

		/// ������ Map�� �߰�
		m_mapSkillSlotUISet.insert(std::make_pair(eSkillID, UISet));

		/// ���� Data�� ����� ���� ����				
		CX2SkillTreeSlotData* pSkillTreeSlotData = new CX2SkillTreeSlotData;
		pSkillTreeSlotData->m_eSkillID = eSkillID;

		UISet.m_pSlot->SetID( static_cast<int>( eSkillID ) );
		UISet.m_pSlot->SetSlotData( pSkillTreeSlotData );

		pSkillTreeSlotData->m_eSkillID = eSkillID;
		pSkillTreeSlotData->m_bDataUpdated = true;

		/// ��ų Ʈ�� �ʻ�ȭ ����� ���� �ִ� ��ų �迭 ���� ����
		if ( row > m_iMaxSkillRowNum )
			m_iMaxSkillRowNum = row;
	}

	SetBlindSelectSkill();						/// 2������ ��ų �� ���õ��� ���� ��ų�� ���� �����ε� ����

	CreateSkillTreePortrait( pControlList );	/// ��ų Ʈ�� �ʻ�ȭ ����

	SetSelectSkillSlotPosition();				/// 2������ ��ų ��ġ ����

	SetDivisionLine( pControlList );			/// ��ų ���м� ����

	/// ��ų ���� â�� ���� ���� ���̾�� ����
	CKTDGUIStatic* pStaticSkill = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStaticSkill" ) );

	if ( NULL != pStaticSkill )
		m_pDLGUISkillTree->ChangeSequence( pStaticSkill, false );

	/// ��ũ�� ����Ʈ ���� ���� ����
	pControlList->SetIndex(0, 0);
}

CX2EquippedSkillSlotData* CX2UISkillTreeNew::GetEquippedSlotData( int index, bool bSlotB /*= false*/ )
{
	CX2EquippedSkillSlotData* pSlotData = NULL;

#ifdef REFORM_UI_SKILLSLOT
	if(false == bSlotB)	// ���� A
	{
		switch(index)
		{
		case 0:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA1" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 1:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA2" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 2:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA3" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 3:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA4" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		default:
			break;
		}
	}
	else	// ���� B
	{
		switch(index)
		{
		case 0:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB1" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 1:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB2" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 2:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB3" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 3:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB4" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		default:
			break;
		}
	}
#else
	if(false == bSlotB)	// ���� A
	{
		switch(index)
		{
		case 0:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA1" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 1:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA2" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 2:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA3" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 3:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA4" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		default:
			break;
		}
	}
	else	// ���� B
	{
		switch(index)
		{
		case 0:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB1" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 1:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB2" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 2:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB3" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		case 3:
			{
				CKTDGUISlot* pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB4" );
				if(pSlot != NULL)
					pSlotData = (CX2EquippedSkillSlotData*) pSlot->GetSlotData();
			} break;
		default:
			break;
		}
	}
#endif //REFORM_UI_SKILLSLOT

	return pSlotData;
}
#ifdef REFORM_UI_SKILLSLOT
CKTDGUISlot* CX2UISkillTreeNew::GetEquippedSlot( int index , bool bSlotB /* = false */ )
{
	if (m_pDLGUISkillTree == NULL)
		return NULL;

	CKTDGUISlot* pSlot = NULL ;

	//�ε������� 4�̻����� ���� ������ B�������� �Ǵ�
	if( index >= 4 )
	{
		index -= 4;
		bSlotB = true;
	}
		
	if(false == bSlotB)	// ���� A
	{
		switch(index)
		{
		case 0:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA1" );
			} break;
		case 1:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA2" );
			} break;
		case 2:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA3" );
			} break;
		case 3:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA4" );
			} break;
		default:
			break;
		}
	}
	else	// ���� B
	{
		switch(index)
		{
		case 0:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB1" );
			} break;
		case 1:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB2" );
			} break;
		case 2:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB3" );
			} break;
		case 3:
			{
				pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB4" );
			} break;
		default:
			break;
		}
	}

	return pSlot;
}
#endif //REFORM_UI_SKILLSLOT
void CX2UISkillTreeNew::InitEquippedSkillSlotData()
{

	if(m_pDLGUISkillTree == NULL)
		return;

	for(int i=0; i<EQUIPPED_SKILL_SLOT_COUNT*2; ++i)
	{
		bool bSlotB = (i > 3) ? true : false;
		int iSlotIndex = (i > 3) ? i-4 : i;
		CX2EquippedSkillSlotData* pData = GetEquippedSlotData(iSlotIndex, bSlotB);
		SAFE_DELETE(pData);
	}
#ifdef REFORM_UI_SKILLSLOT
	CKTDGUISlot* pSlot = NULL;
	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA1" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(0, false));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA2" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(1, false));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA3" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(2, false));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotA4" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(3, false));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB1" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(0, true));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB2" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(1, true));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB3" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(2, true));

	pSlot = (CKTDGUISlot*) m_pDLGGameSkillSlot->GetControl( L"SlotB4" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(3, true));
#else
	CKTDGUISlot* pSlot = NULL;
	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA1" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(0, false));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA2" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(1, false));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA3" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(2, false));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotA4" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(3, false));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB1" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(0, true));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB2" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(1, true));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB3" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(2, true));

	pSlot = (CKTDGUISlot*) m_pDLGUISkillTree->GetControl( L"SlotB4" );
	if(pSlot != NULL)
		pSlot->SetSlotData(new CX2EquippedSkillSlotData(3, true));
#endif //REFORM_UI_SKILLSLOT
}

void CX2UISkillTreeNew::UpdateSkillEquipedSlotUI()
{
	if(m_pUserSkillTree != NULL)
	{
		for(int i=0; i<EQUIPPED_SKILL_SLOT_COUNT*2; ++i)
		{			
			bool bSlotB = (i > 3) ? true : false;
			int iSlotIndex = (i > 3) ? i-4 : i;
			CX2EquippedSkillSlotData* pSlotData = GetEquippedSlotData(iSlotIndex, bSlotB);
			if(pSlotData != NULL)
			{
				pSlotData->m_bDataUpdated = true;				
			}
		}

#ifdef REFORM_UI_SKILLSLOT
		if(m_pDLGGameSkillSlot != NULL)
		{
			switch(m_pUserSkillTree->GetSkillSlotBExpirationState())
			{
			case CX2UserSkillTree::SSBES_NOT_EXPIRED: //���� Ȯ�� ����
			case CX2UserSkillTree::SSBES_PERMANENT: 
				{
					m_bIsExpandSkillSlot = true;
					CKTDGUIButton* pButtonBuyGuide = (CKTDGUIButton*) m_pDLGGameSkillSlot->GetControl( L"Button_Buy_Guide_Slot" );
					if( NULL != pButtonBuyGuide)
						pButtonBuyGuide->SetShowEnable(false, false);

					CKTDGUIStatic* pStaticLock = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"LOCK" );
					if( NULL != pStaticLock)
						pStaticLock->SetShow(false);

					CKTDGUIStatic* pStaticSkillHotKey = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"SkillHotKeyEx" );
					if( NULL != pStaticSkillHotKey)
						pStaticSkillHotKey->SetColor(D3DXCOLOR(1.f,1.f,1.f,1.f));
				}
				break;
			case CX2UserSkillTree::SSBES_EXPIRED: //���� ��Ȯ�� ����
			default:
				{
					m_bIsExpandSkillSlot =false;
					CKTDGUIButton* pButtonBuyGuide = (CKTDGUIButton*) m_pDLGGameSkillSlot->GetControl( L"Button_Buy_Guide_Slot" );
					if( NULL != pButtonBuyGuide)
						pButtonBuyGuide->SetShowEnable(true, true);

					CKTDGUIStatic* pStaticLock = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"LOCK" );
					if( NULL != pStaticLock)
						pStaticLock->SetShow(true);

					CKTDGUIStatic* pStaticSkillHotKey = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"SkillHotKeyEx" );
					if( NULL != pStaticSkillHotKey)
						pStaticSkillHotKey->SetColor(D3DXCOLOR(1.f,1.f,1.f,0.5f));
				}
				break;
			}
#else
		if(m_pDLGUISkillTree != NULL)
		{
			CKTDGUIButton* pButton = (CKTDGUIButton*) m_pDLGUISkillTree->GetControl( L"Button_Notice_Slot" );
			if(pButton != NULL)
			{
				switch(m_pUserSkillTree->GetSkillSlotBExpirationState())
				{
				case CX2UserSkillTree::SSBES_NOT_EXPIRED: //���� Ȯ�� ����
					{
						pButton->SetShowEnable(true, true);
					} break;
				case CX2UserSkillTree::SSBES_PERMANENT: //���� ��Ȯ�� ����
				case CX2UserSkillTree::SSBES_EXPIRED:
				default:
					{
						pButton->SetShowEnable(false, false);
					} break;
				}
			}
#endif //REFORM_UI_SKILLSLOT
#ifdef GUIDE_SKILL_SLOTB 
			pButton = (CKTDGUIButton*) m_pDLGUISkillTree->GetControl( L"Button_Buy_Guide_Slot" );
			if(pButton != NULL)
			{
				switch(m_pUserSkillTree->GetSkillSlotBExpirationState())
				{
				case CX2UserSkillTree::SSBES_EXPIRED:
					{
						pButton->SetShowEnable(true, true);
					} break;
				case CX2UserSkillTree::SSBES_PERMANENT:
				case CX2UserSkillTree::SSBES_NOT_EXPIRED:
				default:
					{
						pButton->SetShowEnable(false, false);
					} break;
				}
			}
#endif GUIDE_SKILL_SLOTB
		}

#ifdef ELSWORD_WAY_OF_SWORD
		if( NULL == g_pX2Game && NULL != m_pDLGGameSkillSlot )
		{
			CKTDGUIStatic* pStatic_ElSkillVigor = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_EL_SKILL_B_B" );
			CKTDGUIStatic* pStatic_ElSkillDest = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_EL_SKILL_R_B" );
			if( NULL != pStatic_ElSkillVigor && NULL != pStatic_ElSkillDest)
			{
				pStatic_ElSkillVigor->SetShow(false);
				pStatic_ElSkillDest->SetShow(false);
			}

			pStatic_ElSkillVigor = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_EL_SKILL_B" );
			pStatic_ElSkillDest = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_EL_SKILL_R" );
			if( NULL != pStatic_ElSkillVigor && NULL != pStatic_ElSkillDest)
			{
				pStatic_ElSkillVigor->SetShow(false);
				pStatic_ElSkillDest->SetShow(false);
			}
		}
#endif //ELSWORD_WAY_OF_SWORD

	}
}

void CX2UISkillTreeNew::UpdateUI( bool bEquipped, bool bSP /*= true*/, bool bSkillTree /*= false*/ )
{
	if(bEquipped)
	{
		UpdateSkillEquipedSlotUI();
	}

	if(bSP)
	{
		UpdateSPInfo();
	}

	if(bSkillTree)
	{
		UpdateSkillTree();
	}

	

}

void CX2UISkillTreeNew::UpdateSkillTree()
{
	// ControlList�� Map�� ���ʷ� Iteration�� ���鼭 (������ ������!)
	// �����̶� ��ư�� �����ϸ� �ش� ������ �����͸� �о�ͼ�
	// �׳� ����Ÿ �ٲ���ٶ�� �����ָ� �ǳ�? -_-
	// 1. ���� ���� �� �ִ� ��ų ���� : ��ư�� Ȱ��ȭ/��Ȱ��ȭ. bool ���� ����
	// 2. 1�����̶� ���� ��ų ���� : ��ų ������ �巡�� Ȱ��ȭ / ��Ȱ��ȭ(�нú�� �巡�� �� �ǰ�), �ؽ��� ����. ��Ʈ�� ���� (����/�ִ뷹��)

	SetBlindSelectSkill();		/// 2������ ��ų �� ���õ��� ���� ��ų�� ���� �����ε� ����

	/// ��� ��ų ���� ����
	for(map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator it =	m_mapSkillSlotUISet.begin(); it != m_mapSkillSlotUISet.end(); ++it)
	{
		SkillSlotUI& refSkillSlotUI = it->second;
		if ( refSkillSlotUI.m_pSlot != NULL )
		{
			if(refSkillSlotUI.m_pSlot->GetSlotData() != NULL)
			{
				refSkillSlotUI.m_pSlot->GetSlotData()->m_bDataUpdated = true;		
			}		 
		}
	}

	/// ���� ��ųâ���� ��ų ������ �ø��� �ִ� ������ ���� ��ư Ȱ��
	CKTDGUIButton* pButtonDecide = static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( L"Decide" ) );
	CKTDGUIButton* pButtonCancel = static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( L"Cancel" ) );

	if( NULL != pButtonDecide && NULL != pButtonCancel )
	{
		pButtonDecide->SetEnable( m_bNowLearnSkill );
		pButtonCancel->SetEnable( m_bNowLearnSkill );
	}

	UpdateBlind();
}

void CX2UISkillTreeNew::UpdateSkillTree( CX2SkillTree::SKILL_ID eSkillID )
{
	map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator it =	m_mapSkillSlotUISet.find(eSkillID);
	if( it != m_mapSkillSlotUISet.end() )
	{
		SkillSlotUI& refSkillSlotUI = it->second;
		if ( refSkillSlotUI.m_pSlot != NULL )
		{
			refSkillSlotUI.m_pSlot->GetSlotData()->m_bDataUpdated = true;			
		}
		const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet = g_pData->GetSkillTree()->GetSkillTreeTemplet((int)g_pData->GetMyUser()->GetSelectUnit()->GetClass(), eSkillID);
		if(pSkillTreeTemplet != NULL)
		{
#ifdef UPGRADE_SKILL_SYSTEM_2013 // ����ȯ - ��ų �ý��� ����
			CX2SkillTree::SKILL_ID eFollowingSkillID = static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iFollowingSkill );
			map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator itfol =	m_mapSkillSlotUISet.find(eFollowingSkillID);
			if( itfol != m_mapSkillSlotUISet.end())
			{
				SkillSlotUI& refFollowingSkillSlotUI = itfol->second;
				if ( refFollowingSkillSlotUI.m_pSlot != NULL )
				{
					refFollowingSkillSlotUI.m_pSlot->GetSlotData()->m_bDataUpdated = true;			
				}

			}
#else // UPGRADE_SKILL_SYSTEM_2013
			for(vector<int>::const_iterator it = pSkillTreeTemplet->m_vecFollowingSkill.begin(); it != pSkillTreeTemplet->m_vecFollowingSkill.end(); ++it)
			{
				CX2SkillTree::SKILL_ID eFollowingSkillID = (CX2SkillTree::SKILL_ID) *it;
				map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator itfol =	m_mapSkillSlotUISet.find(eFollowingSkillID);
				if( itfol != m_mapSkillSlotUISet.end())
				{
					SkillSlotUI& refFollowingSkillSlotUI = itfol->second;
					if ( refFollowingSkillSlotUI.m_pSlot != NULL )
					{
						refFollowingSkillSlotUI.m_pSlot->GetSlotData()->m_bDataUpdated = true;			
					}

				}

			}
#endif // UPGRADE_SKILL_SYSTEM_2013
		}

	}
	UpdateBlind();

}

void CX2UISkillTreeNew::UpdateBlind()
{
	CKTDGUIContolList*	pControlList = static_cast<CKTDGUIContolList*>( m_pDLGUISkillTree->GetControl( L"SkillTreeControlList" ) );
	CKTDGUIStatic*		pStaticBlind = (CKTDGUIStatic*) m_pDLGUISkillTree->GetControl( L"g_pStaticBlind" );
	
	if( NULL != pStaticBlind &&
		NULL != pControlList && 
		NULL != g_pData &&
		NULL != g_pData->GetMyUser() &&
		NULL != g_pData->GetMyUser()->GetSelectUnit() &&
		NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
#if defined( _IN_HOUSE_ ) || defined( _IN_HOUSE_SERVICE_READY_QA_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
		/// ������ ��� - ��ų ��� �����ؾ� �� ��, �����ε� ó�� ����

		int iLearnSkillNum	= 0;		/// ���� ��� ��ų ��
		int	iiLimitSkillnum	= 0;		/// �����ε� ���� ���� ��ų ��

		/// ������ ��޿��� �����ε� ó�� ����
		if( true == g_pMain->IsMyAuthLevelHigherThan( CX2User::XUAL_DEV ) )
		{
			const int iClassLevel = g_pData->GetMyUser()->GetSelectUnit()->GetClassLevel();	/// ���� ���

			switch ( iClassLevel )
			{
			case 0:		/// �⺻ ������ �� ��ų 10�� ������, ��� ����
				iiLimitSkillnum = 10;
				break;
			case 1:		/// 1�� ������ �� ��ų 20�� ������, ��� ����
				iiLimitSkillnum = 20;
				break;
			default:	/// 2�� ������ �� ��ų 30�� ������, ��� ����
				iiLimitSkillnum = 30;
			}

			/// ���� ��� ��ų�� ��ȯ
			iLearnSkillNum = static_cast<int>( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetMapSkillAcquired().size() );
		}
#endif // _SERVICE_
		int							iTopTier				= pControlList->GetIndexY();											/// ���� ��ųâ�� ��ũ�� �� ��ġ �ε���
		int							iUnitLevel				= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_Level;	/// ���� ����
		CX2Unit::UNIT_CLASS_TYPE	eUnitClassType			= static_cast<CX2Unit::UNIT_CLASS_TYPE>( g_pData->GetMyUser()->GetSelectUnit()->GetClassLevel() );	/// ���� Ŭ���� Ÿ��
		const int					iNotLearnTierIndex		= GetNotLearnTierIndex();											/// ����� ���� �ֻ��� ��ų ���� ��ȯ
		const int					iEnoughLevelTierIndex	= GetNotEnoughLevelTierIndex( iUnitLevel );							/// ��� �� ���� ������ �ֻ��� ��ų ���� ��ȯ


		for( int i = 0; i < pControlList->GetViewSizeY(); ++i )
		{
			if ( NULL == pStaticBlind->GetPicture( i ) ||
				 NULL == pStaticBlind->GetPicture( 5+ i ) ||
				 NULL == pStaticBlind->GetString( i ) )
				continue;

			const float fOffSet = static_cast<float>( 92.f * i );

			/// ����ǥ �ؽ�ó ��ġ ����
			const float fPictureXPos = pStaticBlind->GetPicture( 5 + i )->GetPos().x;
			pStaticBlind->GetPicture( 5 + i )->SetPos( D3DXVECTOR2( fPictureXPos, 120.f + fOffSet ) );

			/// �ȳ� ��Ʈ�� ��ġ ����
			pStaticBlind->GetString( i )->pos.y = 121.f + fOffSet;


			const int	iNowSkillLineLevel	= iTopTier + i;		/// �ش� ��ų ���� ����
			bool		bButtonBlind		= false;			/// �ش� ��ų ������ ��ų ���� ��ư ���� ����

			bool		bEnoughLevel		= false;			/// �䱸 ���� ���� ���� ����
			bool		bLearnBeforeSkill	= false;			/// ���� ��ų ���� ����

			/// ���� ��ų�� �⺻ ��ų�̰ų�, ���� ���� ������ �ش� ��ų ���� ���� �䱸 ������ ������ ��
			if ( 0 == GetRequireSkillLineLevel( iNowSkillLineLevel ) || 
				 iUnitLevel >= GetRequireSkillLineLevel( iNowSkillLineLevel ) )
			{
				bEnoughLevel = true;
			}

			/// ������ �����ϸ�, ���� ���� ǥ��
			if ( false == bEnoughLevel )
			{
				/// ����ǥ �ؽ�ó Ȱ��
				pStaticBlind->GetPicture( 5 + i )->SetShow( true );

				pStaticBlind->GetString( i )->msg = GET_REPLACED_STRING( ( STR_ID_25172, "i", GetRequireSkillLineLevel( iNowSkillLineLevel ) ) );

				bButtonBlind = true;		/// ��ư �����ε�
			}

			/// �����ε� ������ �ƴ϶��, �����ε� ����
			if ( false == bButtonBlind )
			{
				pStaticBlind->GetPicture( 5 + i )->SetShow( false );

				pStaticBlind->GetString( i )->msg = L"";
			}

			/// �����ε� ���ο� ���� �����ε� �ؽ�ó ǥ�� ����
			pStaticBlind->GetPicture(i)->SetShow( bButtonBlind );

#if defined( _IN_HOUSE_ ) || defined( _IN_HOUSE_SERVICE_READY_QA_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
			/// ������ ��� - ��ų ��� �����ؾ� �� ��, �����ε� ó�� ����
			if( true == g_pMain->IsMyAuthLevelHigherThan( CX2User::XUAL_DEV ) )
			{
				if ( iiLimitSkillnum <= iLearnSkillNum )
				{
					pStaticBlind->GetPicture(i)->SetShow( false );	/// �����ε� ��Ȱ��
					pStaticBlind->GetPicture( 5 + i )->SetShow( false );
					pStaticBlind->GetString( i )->msg = L"";

					bButtonBlind = false;
				}
			}
#endif // _SERVICE_

			/// �ش� ������ ��ų�� �˻��Ͽ�, ��ư Ȱ�� ���� ����
			for ( int index = 0; index < 3; ++index )
			{
				/// ��ġ�� �ش��ϴ� ��ų ���̵� ��ȯ
				std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator it = m_mapSkillPosition.find( SkillPositionKey( index, iNowSkillLineLevel ) );

				if ( it != m_mapSkillPosition.end() )
				{
					/// ��ų ���̵� �ش��ϴ� UI Set ��ȯ
					map<CX2SkillTree::SKILL_ID, CX2UISkillTreeNew::SkillSlotUI>::iterator itUISet = m_mapSkillSlotUISet.find( it->second );

					if ( itUISet != m_mapSkillSlotUISet.end() )		/// ��ư Ȱ�� ���� ����
						itUISet->second.SetSkillLineBlind( bButtonBlind );
				}
			}
		}

		m_pDLGUISkillTree->ChangeSequence( pStaticBlind, true );
	}
}

void CX2UISkillTreeNew::UpdateSPInfo()
{
	/// ��ų ����Ʈ ���� ��Ʈ�� ����
	CKTDGUIStatic* pNowSPString = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStaticSPBox" ) );

	if( NULL != pNowSPString && 
		NULL != pNowSPString->GetString(0) &&
		NULL != g_pData &&
		NULL != g_pData->GetMyUser() &&
		NULL != g_pData->GetMyUser()->GetSelectUnit() )
	{
		CX2Unit::UnitData*	pUnitData	= g_pData->GetMyUser()->GetSelectUnit()->GetUnitData();
		CKTDGUIButton*		pButton		= (CKTDGUIButton*) m_pDLGUISkillTree->GetControl( L"Button_Notice_Notice_SP" );

		if ( NULL != pUnitData && NULL != pButton )
		{
			const int iHaveSPAndCSP	= pUnitData->m_iSPoint + pUnitData->m_iCSPoint;		/// ���� ��ų ����Ʈ + ���� ĳ�� ��ų ����Ʈ

			WCHAR	buf[255]	= {0,};
			/// �� ��ų ����Ʈ ���� ����

			/// ĳ�� ��ų ����Ʈ( �׳�ý��� �ູ )�� ���������� �˻�
			CTime cTime;
			KncUtil::ConvertStringToCTime( g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_wstrCSPointEndDate, cTime );
			CTime tCurrentTime = g_pData->GetServerCurrentTime();

			bool bUsingCSP = tCurrentTime >= cTime ? false : true;

			wstring wstrUsedSkillString = L"";

			if (true == bUsingCSP )
			{
				StringCchPrintf(buf, 256, L"#CEC008C%d#CX", iHaveSPAndCSP );	/// ĳ�� ��ų ����Ʈ�� ���� �� �̶��, ���������� ǥ��
				pButton->SetShowEnable(true, true);								/// �׳�ý� ����� �˾� ǥ��
			}
			else
			{
				StringCchPrintf(buf, 256, L"%d", iHaveSPAndCSP );				/// ĳ�� ��ų ����Ʈ�� ����O���� ������, �⺻ ������ ǥ�� ( ����� )
				pButton->SetShowEnable(false, false);							/// �׳�ý� ����� �˾� ����
			}

			wstrUsedSkillString = buf;


			if ( true == m_bNowLearnSkill )		/// ��ų ���� �ø��� �� �̶��
			{
				if ( NULL != g_pData->GetSkillTree() )
				{
					const int iUsedSPoint	= g_pData->GetSkillTree()->GetUsedSPoint();			/// ����� ��ų ����Ʈ
					const int iUsedCSPoint	= g_pData->GetSkillTree()->GetUsedCSPoint();		/// ����� ĳ�� ��ų ����Ʈ
					const int iUsedSPAndCSP	= iHaveSPAndCSP - ( iUsedSPoint + iUsedCSPoint );	/// ���� ��ų ����Ʈ

					StringCchPrintf( buf, 256, L"#CFF0000%d#CX / %s", iUsedSPAndCSP, wstrUsedSkillString.c_str() );		/// ���� ����Ʈ ( ������ ) / �� ��ų ����Ʈ

					wstrUsedSkillString = buf;
				}
			}

			pNowSPString->GetString(0)->msg = wstrUsedSkillString;
		}
	}
}

void CX2UISkillTreeNew::CreateSlotUIPreset(SkillSlotUI& UISet, int eSkillID, int iCol )
{
	if( m_pDLGUISkillTree == NULL )
		return;

	/// ���� ���� --------------------------------------------------------------------------------------------------------------------
 	m_pDLGUISkillTree->OpenScriptFile( L"DLG_UI_Skill_Icon_New.lua" );

	UISet.m_pSlot = static_cast<CKTDGUISlot*>( m_pDLGUISkillTree->GetControl( L"g_pSlot_SkillTree" ) );		/// ���� ����

	SKILL_TREE_SLOT_TYPE eSkillTreeSlotType = static_cast<SKILL_TREE_SLOT_TYPE>( iCol );

	WCHAR buf[255] = {0,};
	
	if ( NULL != UISet.m_pSlot )
	{
		StringCchPrintf( buf, 255, L"g_pSlot_SkillTree_%d", eSkillID );
		UISet.m_pSlot->SetName(buf);

		if ( STST_SELECT_SKILL_LEFT == iCol )
		{
			CKTDGUIControl::UIPointData* pPoint		= new CKTDGUIControl::UIPointData();
			pPoint->fChangeTime						= 0.0f;
			pPoint->addSize							= D3DXVECTOR2( 46.f, 46.f );
			pPoint->color							= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
			pPoint->leftTopPoint					= D3DXVECTOR2( 119.f, 16.f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime					= 0.0f;
			pOverPoint->color						= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
			pOverPoint->leftTopPoint				= D3DXVECTOR2( 112.f, 9.f );

			UISet.m_pSlot->SetPoint( pPoint );
			UISet.m_pSlot->SetMouseOverPoint( pOverPoint );
		}
		if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pPoint		= new CKTDGUIControl::UIPointData();
			pPoint->fChangeTime						= 0.0f;
			pPoint->addSize							= D3DXVECTOR2( 46.f, 46.f );
			pPoint->color							= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
			pPoint->leftTopPoint					= D3DXVECTOR2( 89.f, 16.f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime					= 0.0f;
			pOverPoint->color						= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
			pOverPoint->leftTopPoint				= D3DXVECTOR2( 82.f, 9.f );

			UISet.m_pSlot->SetPoint( pPoint );
			UISet.m_pSlot->SetMouseOverPoint( pOverPoint );
		}
	}

	/// ��� ���� --------------------------------------------------------------------------------------------------------------------
	UISet.m_pStaticBG = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_BackGround" ) );

	if( UISet.m_pStaticBG != NULL && 
		UISet.m_pStaticBG->GetPicture( 0 ) != NULL && 
		UISet.m_pStaticBG->GetPicture( 1 ) != NULL &&
		UISet.m_pStaticBG->GetPicture( 2 ) != NULL)
	{
		StringCchPrintf( buf, 256, L"g_pStatic_BackGround_%d", eSkillID );
		UISet.m_pStaticBG->SetName( buf );

		if ( STST_SELECT_SKILL_LEFT == iCol )			/// 2������ ���� ��ų
		{
			UISet.m_pStaticBG->GetPicture( 0 )->SetShow( false );
			UISet.m_pStaticBG->GetPicture( 1 )->SetPos( D3DXVECTOR2( 97.f, 8.f ) );
			UISet.m_pStaticBG->GetPicture( 2 )->SetShow( true );
		}
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų
		{
			UISet.m_pStaticBG->GetPicture( 0 )->SetShow( false );
			UISet.m_pStaticBG->GetPicture( 1 )->SetShow( false );
			UISet.m_pStaticBG->GetPicture( 2 )->SetShow( false );
		}
		else
		{
			UISet.m_pStaticBG->GetPicture( 1 )->SetShow( false );
			UISet.m_pStaticBG->GetPicture( 2 )->SetShow( false );
		}

		/// ��� ������ �ǵڷ� ������
		m_pDLGUISkillTree->ChangeSequence( UISet.m_pStaticBG, false );
	}

	/// ��ų ���� ���� ---------------------------------------------------------------------------------------------------------------
	UISet.m_pStaticSkillBG	= static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_SkillBackGround" ) );

	if ( NULL != UISet.m_pStaticSkillBG && NULL != UISet.m_pStaticSkillBG->GetPicture(0) && NULL != UISet.m_pStaticSkillBG->GetPicture(1) )
	{
		StringCchPrintf( buf, 255, L"g_pStaticSkillBackGround_%d", eSkillID );
		UISet.m_pStaticSkillBG->SetName( buf );

		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( static_cast<CX2SkillTree::SKILL_ID>( eSkillID ) );
		if(pSkillTemplet != NULL)
		{
			wstring wstrSkillBGKeyName = L"";	/// ��ų ���� �ؽ�ó Ű��

			switch(pSkillTemplet->m_eType)		/// ��ų Ÿ�Կ� ���� �ؽ��� ����
			{
			case CX2SkillTree::ST_ACTIVE:
		#ifdef ADDED_RELATIONSHIP_SYSTEM
			case CX2SkillTree::ST_RELATIONSHIP_SKILL:
		#endif // ADDED_RELATIONSHIP_SYSTEM
				{
					wstrSkillBGKeyName = L"Skill_BG_Active";
				} break;
			case CX2SkillTree::ST_SPECIAL_ACTIVE:
				{
					wstrSkillBGKeyName = L"Skill_BG_Active_Special";
				} break;
			case CX2SkillTree::ST_BUFF:
				{
					wstrSkillBGKeyName = L"Skill_BG_Active_Buff";
				} break;
			default:
				{
					wstrSkillBGKeyName = L"Skill_BG_Passive";
				} break;
			}

			UISet.m_pStaticSkillBG->GetPicture( 0 )->SetTex( L"DLG_UI_Common_Texture10.TGA", wstrSkillBGKeyName.c_str() );	/// ����� �ؽ��� ����
		}

		if ( STST_SELECT_SKILL_LEFT == iCol )			/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillBG->GetPicture( 0 )->SetPos( D3DXVECTOR2( 116.f, 13.f ) );
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillBG->GetPicture( 0 )->SetPos( D3DXVECTOR2( 86.f, 13.f ) );

		/// �䱸 SP ���� ��ġ ����
		D3DXVECTOR2 vecSkillBGPos = UISet.m_pStaticSkillBG->GetPicture( 0 )->GetPos();
		UISet.m_pStaticSkillBG->GetPicture( 1 )->SetPos( D3DXVECTOR2( vecSkillBGPos.x + 21.f, vecSkillBGPos.y + 52.f ) );
	}

	/// ��ų ���� ��ư ���� ----------------------------------------------------------------------------------------------------------
	UISet.m_pButtonSkillLearn =	static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( L"g_pButton_SkillLearn" ) );

	if( UISet.m_pButtonSkillLearn != NULL )
	{
		StringCchPrintf( buf, 256, L"ButtonSkillLearn_%d", eSkillID );
		UISet.m_pButtonSkillLearn->SetName( buf );
		UISet.m_pButtonSkillLearn->AddDummyInt( static_cast<int>( eSkillID ) );			/// ��ư�� ��ų ���̵� ����


		if ( STST_SELECT_SKILL_LEFT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 118.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 116.f, 63.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 117.f, 64.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 118.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			UISet.m_pButtonSkillLearn->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillLearn->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillLearn->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillLearn->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillLearn->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 87.f, 63.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 88.f, 64.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			UISet.m_pButtonSkillLearn->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillLearn->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillLearn->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillLearn->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillLearn->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}
	}

	/// ��ų ������ ��ư ���� --------------------------------------------------------------------------------------------------------
	UISet.m_pButtonSkillDesc =	static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( L"g_pButton_SkillDesc" ) );

	if( UISet.m_pButtonSkillDesc != NULL )
	{
		StringCchPrintf( buf, 256, L"ButtonSkillDesc_%d", eSkillID );
		UISet.m_pButtonSkillDesc->SetName( buf );


		if ( STST_SELECT_SKILL_LEFT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 118.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 116.f, 63.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 117.f, 64.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 118.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			UISet.m_pButtonSkillDesc->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillDesc->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillDesc->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillDesc->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillDesc->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 87.f, 63.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 88.f, 64.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

			UISet.m_pButtonSkillDesc->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillDesc->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillDesc->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillDesc->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillDesc->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}
		else
		{
			CKTDGUIControl::UIPointData* pNormalPoint = UISet.m_pButtonSkillDesc->GetNormalPoint();

			if ( NULL != pNormalPoint )
			{
				UISet.m_pButtonSkillDesc->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
			}
		}

		UISet.m_pButtonSkillDesc->SetGuideDesc(L"");
	}

	/// ��ų �ʱ�ȭ ��ư ���� --------------------------------------------------------------------------------------------------------
	UISet.m_pButtonSkillInit =	static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( L"g_pButton_SkillInit" ) );

	if( UISet.m_pButtonSkillInit != NULL )
	{
		StringCchPrintf( buf, 256, L"ButtonSkillInit_%d", eSkillID );
		UISet.m_pButtonSkillInit->SetName( buf );
		UISet.m_pButtonSkillInit->AddDummyInt( static_cast<int>( eSkillID ) );			/// ��ư�� ��ų ���̵� ����


		if ( STST_SELECT_SKILL_LEFT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 119.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 119.f, 65.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 120.f, 66.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 119.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			UISet.m_pButtonSkillInit->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillInit->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillInit->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillInit->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillInit->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
		{
			CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
			pNormalPoint->fChangeTime	= 0.0f;
			pNormalPoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
			pOverPoint->fChangeTime		= 0.0f;
			pOverPoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
			pDownPoint->fChangeTime		= 0.0f;
			pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
			pDownPoint->leftTopPoint	= D3DXVECTOR2( 90.f, 66.f );
			pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
			pDisablePoint->fChangeTime	= 0.0f;
			pDisablePoint->leftTopPoint	= D3DXVECTOR2( 89.f, 65.f );
			pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

			UISet.m_pButtonSkillInit->SetNormalPoint( pNormalPoint );
			UISet.m_pButtonSkillInit->SetOverPoint( pOverPoint );
			UISet.m_pButtonSkillInit->SetDownPoint( pDownPoint );
			UISet.m_pButtonSkillInit->SetDisablePoint( pDisablePoint );
			UISet.m_pButtonSkillInit->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
		}

		UISet.m_pButtonSkillInit->SetShow( false );
	}

	/// ���� ��Ʈ�� ���� -------------------------------------------------------------------------------------------------------------
	UISet.m_pStaticStringRequireSP = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_StringInfo" ) );

	if( UISet.m_pStaticStringRequireSP != NULL )
	{
		StringCchPrintf( buf, 256, L"g_pStaticStringInfo_%d", eSkillID );
		UISet.m_pStaticStringRequireSP->SetName( buf );

		if ( STST_SELECT_SKILL_LEFT == iCol )			/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticStringRequireSP->GetString( 0 )->pos = D3DXVECTOR2( 151.f, 66.f );
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticStringRequireSP->GetString( 0 )->pos = D3DXVECTOR2( 121.f, 66.f );
	}

	/// ������ ��ų �̹��� ���� ------------------------------------------------------------------------------------------------------
	UISet.m_pStaticSkillMaster = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_SkillMaster" ) );

	if( UISet.m_pStaticSkillMaster != NULL )
	{
		StringCchPrintf( buf, 256, L"g_pStaticSkillMaster_%d", eSkillID );
		UISet.m_pStaticSkillMaster->SetName( buf );

		if ( STST_SELECT_SKILL_LEFT == iCol )			/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillMaster->GetPicture(0)->SetPos( D3DXVECTOR2( 119.f, 65.f ) );
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillMaster->GetPicture(0)->SetPos( D3DXVECTOR2( 89.f, 65.f ) );

		UISet.m_pStaticSkillMaster->GetPicture( 0 )->SetShow( false );
	}

	/// ��ų ���� �Ұ� ���� ----------------------------------------------------------------------------------------------------------
	UISet.m_pStaticSkillNoLearn = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_SkillNoLearn" ) );

	if( UISet.m_pStaticSkillNoLearn != NULL )
	{
		StringCchPrintf( buf, 256, L"g_pStatic_SkillNoLearn_%d", eSkillID );
		UISet.m_pStaticSkillNoLearn->SetName( buf );
		UISet.m_pStaticSkillNoLearn->GetPicture(0)->SetShow( false );

		if ( STST_SELECT_SKILL_LEFT == iCol )			/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillNoLearn->GetPicture(0)->SetPos( D3DXVECTOR2( 114.f, 12.f ) );
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillNoLearn->GetPicture(0)->SetPos( D3DXVECTOR2( 84.f, 12.f ) );
	}

	/// �ڹ��� ���� ----------------------------------------------------------------------------------------------------------
	UISet.m_pStaticSkillLock = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStaticSkill_Lock" ) );

	if( UISet.m_pStaticSkillLock != NULL )
	{
		StringCchPrintf( buf, 256, L"g_pStaticSkill_Lock_%d", eSkillID );
		UISet.m_pStaticSkillLock->SetName( buf );
		UISet.m_pStaticSkillLock->SetShow( false );

		if ( STST_SELECT_SKILL_RIGHT == iCol )			/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillLock->GetPicture(0)->SetPos( D3DXVECTOR2( 97.f, 19.f ) );
		else if ( STST_SELECT_SKILL_RIGHT == iCol )		/// 2������ ���� ��ų�̸�, ��ġ �̵�
			UISet.m_pStaticSkillLock->GetPicture(0)->SetPos( D3DXVECTOR2( 197.f, 19.f ) );
	}

	return;
}

// CX2UISkillTreeNew::SkillSlotUI& CX2UISkillTreeNew::GetSkillSlotUI( CX2SkillTree::SKILL_ID eSkillID )
// {
// 	map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator it = m_mapSkillSlotUISet.find(eSkillID);
// 	if(it != m_mapSkillSlotUISet.end())
// 	{
// 		return it->second;
// 	}
// 
// 	ASSERT( !"Wrong Skill ID" );
// 	return m_mapSkillSlotUISet.begin()->second;
// 
// }

// CX2UISkillTreeNew::ArrowUI& CX2UISkillTreeNew::GetArrowUI( CX2SkillTree::SKILL_ID eSkillID )
// {
// 	map<CX2SkillTree::SKILL_ID, ArrowUI>::iterator it = m_mapArrowUISet.find(eSkillID);
// 	if(it != m_mapArrowUISet.end())
// 	{
// 		return it->second;
// 	}
// 
// 	ASSERT( !"Wrong Skill ID" );
// 	return m_mapArrowUISet.begin()->second;
// }


// ��� ��ų�� ��Ŭ���� �ϸ� ��Ÿ���� ���̾�α׸� ���� ���� �Լ�.
// ���� ��ų �������� ��ü�� �̱����̹Ƿ� ��ݸ� �۾� �д�.
// ���̾�α� �ݴ� �������� ����� �׽��ϴ�. ���� ��ư�� ������ ���� �ƹ��͵� �� �ϰ� �Ǿ� ������ ���� �۾��� �в����� 
// UICustomMsgProc���� case STUCM_UNSEAL_SKILL_BUY �׸� �� �κ��� �۾��� �ּ���.
void CX2UISkillTreeNew::OpenSkillUnsealInformDLG( const CX2SkillTree::SkillTemplet* pSkillTemplet, D3DXVECTOR2 SlotCenterPos )
{	
	SAFE_DELETE_DIALOG( m_pDLGSkillUnsealInform );
		
	m_pDLGSkillUnsealInform = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_Skill_Purchase_Window.lua" );
	g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGSkillUnsealInform );
	m_pDLGSkillUnsealInform->SetDisableUnderWindow(true);

	// ���̾�αװ� ������ �� ��ġ(���� �߽ɿ��������� ��밪) : ��ũ��Ʈ�� �־� ��. DLG_Skill_Purchase_Window.lua �ֻ�� ����.
	D3DXVECTOR2 OffsetPos((float)m_pDLGSkillUnsealInform->GetDummyInt(0), (float)m_pDLGSkillUnsealInform->GetDummyInt(1));	
	m_pDLGSkillUnsealInform->SetPos( SlotCenterPos + OffsetPos );

	CKTDGUIStatic* pStaticRequireItem = (CKTDGUIStatic*) m_pDLGSkillUnsealInform->GetControl( L"StaticRequireItemName" );
	CKTDGUIStatic* pStaticRequireCash = (CKTDGUIStatic*) m_pDLGSkillUnsealInform->GetControl( L"StaticPrice" );
	

//#if 0		//** ��ų �������� ����� �� �� ���⿡ �߰��۾� �� �ּ���. 
////{{AFX
//	if( NULL != pStaticRequireCash->GetString(0) )
//	{
//		//** ���⿡ �ش� ��ų�� ���������� ������ �� �ּ���
//		int iRequireCash = pSkillTemplet->����;
//		WCHAR buf[256] = {0,};
//		//wsprintf( buf, L"(%d%s)", iRequireCash, GET_STRING(STR_ID_34) );
//		StringCchPrintf( buf, 256, L"(%d%s)", iRequireCash, GET_STRING(STR_ID_34) );
//		pStaticRequireCash->GetString(0)->msg = buf;
//	}
//	else
//	{
//		SAFE_DELETE_DIALOG(m_pDLGSkillUnsealInform);
//		ASSERT( "Script Error." );
//		return;
//	}
//
//	if( NULL != pStaticRequireItem->GetString(0) )
//	{
//
//		//** ���⿡ �ش� ��ų�� ���������� �̸��� �� �ּ���
//		WCHAR buf[256] = {0,};
//		wsprintf( buf, L"%s", pSkillTemplet->�ʿ��Ѿ������̸� );
//		StringCchPrintf( buf, 256, L"%s", pSkillTemplet->�ʿ��Ѿ������̸� );
//		pStaticRequireCash->GetString(0)->msg = buf;
//	}
//	else
//	{
//		SAFE_DELETE_DIALOG(m_pDLGSkillUnsealInform);
//		ASSERT( "Script Error." );
//		return;
//	}
//
//	m_pDLGSkillUnsealInform->SetShowEnable(true, true);
////}}AFX
//#endif

		
}

// SlotManager�� ItemDesc �� �ڵ带 �ܾ����
void CX2UISkillTreeNew::ShowSkillDesc( bool bShow, CX2SkillTree::SKILL_ID eSkillID /*= CX2SkillTree::SI_NONE*/, D3DXVECTOR2 pos /*= D3DXVECTOR2(0,0)*/, 
								    D3DXVECTOR2 size /*= D3DXVECTOR2(0,0)*/, bool bEquipped /*= false*/ )
{
	if(m_pDLGSkillToolTip == NULL )
	{
		/// ���� ���̾�α� ����
		m_pDLGSkillToolTip = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_UI_Skill_Desc.lua", 0.07f, XDL_POP_UP );

		if ( NULL != m_pDLGSkillToolTip )
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGSkillToolTip );
		else
			return;
	}

	// �������..

	if(bShow)
	{	
		CKTDGUIStatic* pItemDesc = static_cast<CKTDGUIStatic*>( m_pDLGSkillToolTip->GetControl(L"SkillDesc") );

		if ( NULL == g_pData ||
			 NULL == g_pData->GetSkillTree() ||
			 NULL == g_pData->GetTitleManager() ||
			 NULL == g_pData->GetMyUser() ||
			 NULL == g_pData->GetMyUser()->GetSelectUnit() ||
			 NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() ||
			 NULL == g_pKTDXApp ||
			 NULL == g_pKTDXApp->GetDGManager() ||
			 NULL == g_pKTDXApp->GetDGManager()->GetDialogManager() ||
			 NULL == pItemDesc )
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}

		CKTDGUIControl::UIStringData* pTitleStringData		= pItemDesc->GetString( 0 );	/// Ÿ��Ʋ ���� ��Ʈ�� ������ ( ��ų �̸� �� ���� )
		CKTDGUIControl::UIStringData* pMainStringData		= pItemDesc->GetString( 1 );	/// ���� ��ų ���� ���� ��Ʈ�� ������ ( ��ų ���� �� ȿ�� )
		CKTDGUIControl::UIStringData* pSubStringData		= pItemDesc->GetString( 2 );	/// ���� ��ų ���� ��Ʈ�� ������ ( ��ų Ÿ��, �Ҹ� MP, ���� �ð� )

		CKTDGUIControl::CPictureData* pMainPictureData		= pItemDesc->GetPicture( 0 );	/// ���� ��ų ���� ���� ��� �̹��� ������
		CKTDGUIControl::CPictureData* pIconPictureData		= pItemDesc->GetPicture( 1 );	/// ��ų ������ �̹��� ������
		CKTDGUIControl::CPictureData* pUseSkillPictureData	= pItemDesc->GetPicture( 2 );	/// ��ų ���� ���� �̹��� ������

		if ( NULL == pTitleStringData ||
			 NULL == pMainStringData ||
			 NULL == pSubStringData ||
			 NULL == pMainPictureData ||
			 NULL == pIconPictureData ||
			 NULL == pUseSkillPictureData )
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}

		
		/// ��ų ���� ���� -------------------------------------------------------------------------------------
		CX2UserSkillTree& refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;
		
		int iSkillLevel = refUserSkillTree.GetSkillLevel( eSkillID , true );		/// ��ų ���� ( ������ �����Ǿ� �ִ� ���� )

		/// ���� ��ųâ���� ��ų ������ �ø��� ������, ���� ��ų�� ������ �ƴ� ��
		if ( true == m_bNowLearnSkill && false == bEquipped )
		{
			const std::map<int, KGetSkillInfo> mapSkillInfo = g_pData->GetSkillTree()->GetMapSkillInfo();

			std::map<int, KGetSkillInfo>::const_iterator it = mapSkillInfo.find( static_cast<int>( eSkillID ) );

			if ( it != mapSkillInfo.end() )
				iSkillLevel = it->second.m_iSkillLevel;
		}

		/// ��ų ���ø� ��ȯ -----------------------------------------------------------------------------------
#ifdef UPGRADE_SKILL_SYSTEM_2013 //JHKang
		m_bReplacedEsCounter = false;
		if( bEquipped == true && eSkillID == CX2SkillTree::SI_A_ES_COUNTER )
		{
			int iSkillLevel = refUserSkillTree.GetSkillLevel( CX2SkillTree::SI_P_EIS_COMPACT_COUNTER );
			if( iSkillLevel > 0 )
			{
				eSkillID = CX2SkillTree::SI_P_EIS_COMPACT_COUNTER;
				m_bReplacedEsCounter = true;
			}
		}

		if ( true == m_bReplacedEsCounter )
			iSkillLevel = refUserSkillTree.GetSkillLevel( CX2SkillTree::SI_P_EIS_COMPACT_COUNTER );
#endif //UPGRADE_SKILL_SYSTEM_2013

		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );

		if ( NULL == pSkillTemplet )	/// ��ų ���ø��� ������, ������ ���� ������
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}
		
		/// ��ų ���� ���� ���� �� �������� ������ ���� --------------------------------------------------------
		/// ȿ���� ���� ������ ��ų ��ġ
		int iIncreasekillLevel = 0;

		/// �нú� ��ų�� �ƴҶ��� �߰� ��ų ���� ����
		switch( pSkillTemplet->m_eType )
		{
		case CX2SkillTree::ST_BUFF:		
		case CX2SkillTree::ST_ACTIVE:
		case CX2SkillTree::ST_SPECIAL_ACTIVE:
			{
				iIncreasekillLevel = g_pData->GetMyUser()->GetSelectUnit()->GetIncreaseSkillLevelByBuff() + 
									 g_pData->GetMyUser()->GetSelectUnit()->GetIncreaseSkillLevelBySocket( eSkillID );
			} break;
		}

		/// ����ġ���� ���ؼ� �Ѱ�ġ�� �Ѵ´ٸ�, �Ѱ�ġ�� �°� ����
		if ( iSkillLevel + iIncreasekillLevel > MAX_LIMITED_SKILL_LEVEL )
			iIncreasekillLevel = MAX_LIMITED_SKILL_LEVEL - iSkillLevel;

		
		/// ��ų Ʈ�� ���ø� ��ȯ ------------------------------------------------------------------------------
		int										iUnitClass			= static_cast<int>( g_pData->GetMyUser()->GetSelectUnit()->GetClass() );
		const CX2SkillTree::SkillTreeTemplet*	pSkillTreeTemplet	= g_pData->GetSkillTree()->GetSkillTreeTemplet( iUnitClass, pSkillTemplet->m_eID );


		if ( NULL == pSkillTreeTemplet )	/// ��ų Ʈ�� ���ø��� ������, ������ ���� ������
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}


		/// ��ų ������ ���� -----------------------------------------------------------------------------------
		pIconPictureData->SetTex( pSkillTemplet->m_wstrIconName.c_str(), pSkillTemplet->m_wstrIconPieceName.c_str() );
		//pIconPictureData->SetSize( D3DXVECTOR2( 30.f, 30.f ) );


		/// ��ų ���� ���� �̹��� ���� -------------------------------------------------------------------------
		if ( L"" != pSkillTemplet->m_wstrImageName && L"" != pSkillTemplet->m_wstrImagePieceName )
		{
			pUseSkillPictureData->SetShow( true );		/// ��ų �̹����� �����Ǿ� ���� ���� ǥ��
			pUseSkillPictureData->SetTex( pSkillTemplet->m_wstrImageName.c_str(), pSkillTemplet->m_wstrImagePieceName.c_str() );
		}
		else
		{
			pUseSkillPictureData->SetShow( false );
		}

		
		/// Ÿ��Ʋ ���� ���� -----------------------------------------------------------------------------------
		wstring SkillSkillNameAndLevelDesc = L""; 

		GetSkillTitleDesc( SkillSkillNameAndLevelDesc, pSkillTemplet, pSkillTreeTemplet, iSkillLevel, iIncreasekillLevel );		/// ��ų �̸� �� ���� ��Ʈ�� ����

		if( SkillSkillNameAndLevelDesc.empty() == true )		/// ��Ʈ���� ������, ������ ���� ������
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}

		/// ��Ʈ�� ����
		pTitleStringData->msg	= SkillSkillNameAndLevelDesc;

		/// ��ġ ���� ( ��ų ������ ������ ���� )
		pTitleStringData->pos.x	= pMainStringData->pos.x + 45.f;


		/// ���� ���� ���� -------------------------------------------------------------------------------------
		wstring SkillSubDesc = L"";

		const int iSubDeskSkillLevel = 0 < iSkillLevel ? iSkillLevel + iIncreasekillLevel : 1;

		GetSkillSubDesc( pSkillTemplet, iSubDeskSkillLevel, SkillSubDesc );		/// ���� ���� ��Ʈ�� ����

		if( SkillSubDesc.empty() == true )			/// ��Ʈ���� ������, ������ ���� ������
		{
			m_pDLGSkillToolTip->SetShow(false);
			return;
		}

		/// ��Ʈ�� ����
		pSubStringData->msg		= SkillSubDesc;

		
		wstring wstrMainSkillDesc = L"";
		/// ���� ��ų ���� ���� --------------------------------------------------------------------------------
		if ( 0 < pSkillTreeTemplet->m_iPrecedingSkill )		/// ���� ��ų�� ���� ��, ���� ��ų 
			GetPrecedingSkillDesc( wstrMainSkillDesc, static_cast<CX2SkillTree::SKILL_ID>( pSkillTreeTemplet->m_iPrecedingSkill ) );


		/// ���� ��ų ������ ���� �ȳ� ���� ���� ---------------------------------------------------------------
		if( true == pSkillTemplet->m_bBornSealed && false == refUserSkillTree.IsSkillUnsealed( eSkillID ) )
			GetSkillUnSealedDesc( wstrMainSkillDesc, eSkillID );

		
		/// ���� ���� ���� -------------------------------------------------------------------------------------
		wstrMainSkillDesc += pSkillTemplet->m_wstrMainDesc;					/// ����Ǵ� ��ų ���� ��Ʈ�� �߰� ( ���� ������ ��ų ȿ�� ���̿� ���� �ֱ� ����, ���� �и� )

		wstrMainSkillDesc += L"\n";

		/// �� ����
		wstrMainSkillDesc = g_pMain->GetStrByLineBreakColor
			( wstrMainSkillDesc.c_str(), static_cast<int>( MAGIC_SKILL_DESC_WIDTH ), SLOT_MANAGER_FONT_INDEX );

		
		/// ���� ũ�� ���� -------------------------------------------------------------------------------------
		CKTDGFontManager::CUKFont* pItemDescFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( SLOT_MANAGER_FONT_INDEX );	/// XUF_DODUM_13_SEMIBOLD

		/// ���� ���� ��Ʈ�� ����
		int iMainDescHeight = static_cast<int>( pItemDescFont->GetHeight( wstrMainSkillDesc.c_str() ) );

		/// ���ڰ� ���� ó��
		int iLineNum = 0;
		int iOffSetSize = wstrMainSkillDesc.find( L"\n", 0 );
		while( iOffSetSize != -1 )
		{
			iOffSetSize++;
			iLineNum += 2;
			iOffSetSize = wstrMainSkillDesc.find( L"\n", iOffSetSize );
		}
		iMainDescHeight += iLineNum;

		/// ���� ���� Y ��ġ
		int iMainDescYPos = static_cast<int>( pTitleStringData->pos.y + SKILL_MAIN_DESC_OFFSET_Y );

		/// ������ ���� ������ ���� ����
		int			itemDescWidth		= static_cast<int>( MAGIC_SKILL_DESC_WIDTH ) + ITEM_DESC_ADD_SIZE_X;
		/// ������ ���� ������ ����
		int			itemDescHeight		= static_cast<int>( iMainDescHeight / g_pKTDXApp->GetResolutionScaleY() );


		/// ���� �� ���Ҽ� �߰� ( ���� ������ ��ų ȿ�� ���� )
		CKTDGUIStatic* pDevideLine = static_cast<CKTDGUIStatic*>( m_pDLGSkillToolTip->GetControl(L"g_pStatic_HorizonLine") );
		if ( NULL != pDevideLine && 
			 NULL != pDevideLine->GetPicture( 0 ) && 
			 NULL != pDevideLine->GetPicture( 0 )->pPoint && 
			 NULL != pDevideLine->GetPicture( 1 ) &&
			 NULL != pDevideLine->GetPicture( 1 )->pPoint )
		{
			pDevideLine->GetPicture( 0 )->SetSizeX( itemDescWidth - 20.f );
			pDevideLine->GetPicture( 1 )->SetSizeX( itemDescWidth - 20.f );
			pDevideLine->GetPicture( 1 )->SetPos( D3DXVECTOR2( 10.f, static_cast<float>( iMainDescYPos + itemDescHeight ) ) );
		}


		/// ��ų ȿ�� ���� ��ȯ ( ���� ���������� ���̸� ���� �� �ش� ��ġ�� �� ����, ���� ������ ��Ʈ�� �����Ͽ� ���� �ϼ� )
		GetSkillEffectiveDesc( wstrMainSkillDesc, iSkillLevel, iIncreasekillLevel, pSkillTemplet );

		/// �� ����
		wstrMainSkillDesc = g_pMain->GetStrByLineBreakColor
			( wstrMainSkillDesc.c_str(), static_cast<int>( MAGIC_SKILL_DESC_WIDTH ), SLOT_MANAGER_FONT_INDEX );

		pMainStringData->msg = wstrMainSkillDesc;		/// ���� ���� ���� ��Ʈ�� ����


		/// ���� ���� ��Ʈ�� ���� ( ���� ���� + ��ų ȿ�� )
		iMainDescHeight = static_cast<int>( pItemDescFont->GetHeight( pMainStringData->msg.c_str() ) + SKILL_MAIN_DESC_OFFSET_Y );

		/// ���ڰ� ���� ó��
		iLineNum = 0;
		iOffSetSize = wstrMainSkillDesc.find( L"\n", 0 );
		while( iOffSetSize != -1 )
		{
			iOffSetSize++;
			iLineNum += 2;
			iOffSetSize = wstrMainSkillDesc.find( L"\n", iOffSetSize );
		}
		iMainDescHeight += iLineNum;


		/// ���� ���� �� ���� ( ���� ���� + ��ų ȿ�� )
		itemDescHeight	= static_cast<int>( iMainDescHeight / g_pKTDXApp->GetResolutionScaleY() ) + ITEM_DESC_ADD_SIZE_Y;

		pTitleStringData->fontIndex	= SLOT_MANAGER_FONT_INDEX;	/// Ÿ��Ʋ ���� ��Ʈ ũ�� ����
		pMainStringData->fontIndex	= SLOT_MANAGER_FONT_INDEX;	/// ���� ���� ��Ʈ ũ�� ����
		pSubStringData->fontIndex	= SLOT_MANAGER_FONT_INDEX;	/// ���� ���� ��Ʈ ũ�� ����


		/// ��ų ���� ���� ��ġ ���� ---------------------------------------------------------------------------
		int itemDescPosX = 0;
		int itemDescPosY = 0;

		int iDescWidthWithPicture = true == pUseSkillPictureData->GetShow() ?
			static_cast<int>( itemDescWidth + ( 10.f * g_pKTDXApp->GetResolutionScaleX() ) + pUseSkillPictureData->GetSize().x ) : itemDescWidth;

		if ( pos.x + ( size.x / 2 ) - ( iDescWidthWithPicture / 2 ) >= 0 ) /// ���ʿ� ������ ����� ���
		{
			if ( pos.x+ ( size.x / 2 ) - ( iDescWidthWithPicture / 2 ) + iDescWidthWithPicture > 1024 )
			{
				itemDescPosX = static_cast<int>( 1024 - iDescWidthWithPicture );
			}
			else
			{
				itemDescPosX = static_cast<int>( pos.x + ( size.x / 2 ) - ( iDescWidthWithPicture / 2 ) );
			}
		}
		else
		{
			itemDescPosX = 0;
		}

		/// ����Ű�� �ִ� ��ų ���� �������� ������ ǥ���� �� �ִ°��
		if ( static_cast<int>( pos.y ) - itemDescHeight >= 0 )
		{
			/// ����Ű�� �ִ� ��ų ���� ��ġ + ���� ������
			itemDescPosY =  static_cast<int>( pos.y ) - itemDescHeight;
		}
		else	/// ����Ű�� �ִ� ��ų ���� �Ʒ��� ǥ��
		{
			itemDescPosY = static_cast<int>( pos.y + size.y );		/// ��ų ������ Bottom ��ġ

			int iBottomOverSize = 768 - (itemDescPosY + itemDescHeight );	/// ȭ���� �Ѿ�� ũ�⸦ ����Ͽ�, ��ų ������ Bottom ��ġ�� ���� ���� �ش�.

			if ( iBottomOverSize < 0 )
				itemDescPosY += iBottomOverSize;
		}

		// ���� ���κ��� ȭ�� ������ ������ �ʵ��� ����
		if( itemDescPosY < 0 )
			itemDescPosY = 0;
		
		/// ���� ��ġ ����
		m_pDLGSkillToolTip->SetPos( D3DXVECTOR2( static_cast<float>( itemDescPosX ), static_cast<float>( itemDescPosY ) ) );
		
		/// ���� ��� ����
		pMainPictureData->SetSize( D3DXVECTOR2( static_cast<float>( itemDescWidth ), static_cast<float>( itemDescHeight ) ) );
		pMainPictureData->pPoint->color = D3DXCOLOR( 1,1,1,0.7f );

		bool bCheckEquipping = false;


		/// ���� ���� ������ y��ǥ�� Ÿ��Ʋ ������ ��ġ�� ���� ���� 
		/// ( Ÿ��Ʋ ������ ���� ��ܿ� �����Ƿ�, �������� �����Ͽ� ���� + Ÿ��Ʋ ��� ��Ʈ�� ���� + Ÿ��Ʋ ��Ʈ���� ���� ���� Ÿ��Ʋ ������ ���� )
		pMainStringData->pos.y = static_cast<float>( iMainDescYPos );

		/// Ÿ��Ʋ ��Ʈ�� ��ġ ���� ( ��ų ������ ������ ���� )
		pTitleStringData->pos.x	= pMainStringData->pos.x + 45.f;

		/// ���� ���� ��ġ�� ���� ���� ���� ���̿� ���� ���� ( ���� ũ���� ���� �������� '�ؽ�Ʈ ������ ����'�� ���� )
		pSubStringData->pos.x	= static_cast<float>( pMainStringData->pos.x + MAGIC_SKILL_DESC_WIDTH - 15.f );
		/// ��ų ���� ���� �̹��� ��ġ ���� ( ���� ������ �� + Offset )
		pUseSkillPictureData->SetPos( D3DXVECTOR2( static_cast<float>(itemDescWidth ) + ( 10.f * g_pKTDXApp->GetResolutionScaleX() ), 0.f ) );


		m_pDLGSkillToolTip->SetShow( true );
	}
	else
	{
		m_pDLGSkillToolTip->SetShow( false );
	}
	
	
}

/** @function	: GetSkillDesc
	@brief		: ��ų ȿ���� ǥ���ϴ� ���� ���� ����
	@param		: ��Ʈ�� ������, ��ų ����, �߰� ��ų ����, ��ų ���ø�
*/
void CX2UISkillTreeNew::GetSkillEffectiveDesc( OUT wstring& wstrDesc, IN int iSkillLevel, int iIncreasekillLevel, IN const CX2SkillTree::SkillTemplet* pSkillTemplet ) const
{
	if ( NULL == pSkillTemplet ||
		 NULL == g_pData ||
		 NULL == g_pData->GetSkillTree() )
		return;

	wstrDesc += L"\n";

	if ( 0 < iSkillLevel )		/// ��ų ������ 0���� Ŭ ����, ���� ���� ��ų ȿ�� ���
	{
		wstrDesc += L"\n";

		wstrDesc += GET_STRING( STR_ID_25071 );	/// <���� ����>

		wstrDesc += L"\n";

		wstrDesc += pSkillTemplet->GetSkillEffectiveDescValue( iSkillLevel + iIncreasekillLevel );			/// ���� ������ ���� ��ų ȿ�� ���� ����
	}


	if ( MAX_LIMITED_SKILL_LEVEL > iSkillLevel )		/// ���� ��ų ������ �ִ� ��ų ���� �Ѱ躸�� ���� ��, ���� ���� ǥ��
	{
		if ( NULL != g_pData &&
			 NULL != g_pData->GetMyUser() && 
			 NULL != g_pData->GetMyUser()->GetSelectUnit() &&
			 NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
		{
			const int iUnitLevel = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_Level;	/// ������ ����
			const int iNextLevel = 0 == iSkillLevel ? 1 : iSkillLevel + iIncreasekillLevel + 1;		/// ������ 0�̸�, 1���� ǥ��

			/// �䱸 ���� ǥ�� ��Ʈ�� ����
			wstring wstrRequireLevel = L"";

			/// �䱸 ���� ��ȯ ( ���� ��ų ������ �ø��µ� �ʿ��� �����̹Ƿ�, �߰��Ǵ� ������ �����Ѵ�. )
			const int iRequireLevel = pSkillTemplet->GetSkillRequireLevelValue( iSkillLevel + 1 );

			if ( iUnitLevel >= iRequireLevel )
				wstrRequireLevel = GET_REPLACED_STRING( ( STR_ID_25173, "i", iRequireLevel ) );		/// ĳ���� ���� @1 �̻�
			else	/// ���� ���� ������ �����ϸ�, ������ ó��
			{
				wstrRequireLevel =  L"#CFF0000";
				wstrRequireLevel += GET_REPLACED_STRING( ( STR_ID_25173, "i", iRequireLevel ) );	/// ĳ���� ���� @1 �̻�
				wstrRequireLevel += L"#C878787";
			}

			if ( 0 < iSkillLevel )
				wstrDesc += L"\n";

			wstrDesc += L"\n#C878787";			/// ��Ʈ�� ȸ�� ó�� ( R:135, G: 135, B: 135 )

			wstrDesc += GET_REPLACED_STRING( ( STR_ID_25072, "L", wstrRequireLevel ) );				/// <���� ���� : @1>

			wstrDesc += L"\n";

			/// ���� ������ ���� ��ų ȿ�� ���� ���� ( �߰��Ǵ� ���� ȿ���� �ִٸ�, �ش� ������ �����Ͽ� ��ȯ )
			wstrDesc += pSkillTemplet->GetSkillEffectiveDescValue( iNextLevel );

			wstrDesc += L"#CX";

			const int iRequireLearnSkillPoint	= pSkillTemplet->m_iRequireLearnSkillPoint;		/// ����� �ʿ��� SP��
			const int iRequireUpgradeSkillPoint = pSkillTemplet->m_iRequireUpgradeSkillPoint;	/// ��ȭ�� �ʿ��� SP��

			/// ���� ������ ���� ��ȯ
			const int iLearnLevel = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetSkillLevel( pSkillTemplet->m_eID );

			/// ���� ����� ��ȭ�� �ʿ��� SP���� �ٸ���, ���� ����� ���� ��ų�� ��, �ȳ� ���� ���
			if ( iRequireLearnSkillPoint != iRequireUpgradeSkillPoint && 0 >= iLearnLevel )
			{
				wstrDesc += L"\n\n";

				wstrDesc += GET_REPLACED_STRING( ( STR_ID_25398, "ii", iRequireLearnSkillPoint, iRequireUpgradeSkillPoint ) );
			}
		}
	}

#if defined( _IN_HOUSE_ ) || defined( _IN_HOUSE_SERVICE_READY_QA_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
	/// ������ ��� - ��ų ���̵� ���
	if( true == g_pMain->IsMyAuthLevelHigherThan( CX2User::XUAL_DEV ) )
	{
		WCHAR buf[256] = {0,};
		StringCchPrintf( buf, 255, L"\n\n#C00FF00Skill ID : %d#CX", static_cast<int>( pSkillTemplet->m_eID ) );
		
		wstrDesc += buf;
	}
#endif // _SERVICE_
}

void CX2UISkillTreeNew::ShowUIDesc( bool bShow, wstring wstr /*= L""*/, D3DXVECTOR2 pos /*= D3DXVECTOR2(0,0)*/, CX2TalkBoxManagerImp::TALK_BOX_TYPE TBT /*= CX2TalkBoxManagerImp::TBT_FROM_UP_RIGHT*/, D3DXCOLOR coTextColor /*= D3DXCOLOR(0,0,0,1)*/, D3DXCOLOR coBackColor /*= D3DXCOLOR(1, 0.96f, 0.6f, 1 )*/)
{
	m_pTalkBoxManager->Clear();

	if(bShow)
	{
		CX2TalkBoxManagerImp::TalkBox talkBox;

		talkBox.m_vPos				= pos;
		talkBox.m_GameUnitType		= CX2GameUnit::GUT_USER;
		talkBox.m_fRemainTime		= 2.0f;

		talkBox.m_wstrTalkContent	= wstr.c_str();
		talkBox.m_bTraceUnit		= false;
		talkBox.m_TalkBoxType		= TBT;
		talkBox.m_coTextColor		= coTextColor;
		talkBox.m_BackTexColor		= coBackColor;
	
		m_pTalkBoxManager->Push( talkBox );

	}
	
}


// kimhc // KTDXCommonFunc�� �ű� // 2009-06-16
//wstring CX2UISkillTreeNew::GetExpirationDateDesc( const wstring& wstrExpirationDate )
//{
//	wstring tempDesc = L"";
//
//	
//	tempDesc = GET_STRING( STR_ID_309 );
//	//tempDesc = L"���� �ð� : ";
//
//	CTime cTime;
//	KncUtil::ConvertStringToCTime( wstrExpirationDate, cTime );
//
//	wstringstream wstrstm;
//
//	//wstrstm << cTime.GetYear() << L"�� " << cTime.GetMonth() << L"�� " << cTime.GetDay() << L"�� " << cTime.GetHour() << L"�� " << cTime.GetMinute() << L"��"; 
//	//tempDesc += wstrstm.str().c_str();
//	//tempDesc += L"\n";
//
//	wstrstm.str( L"" );
//	wstrstm << L"\n";
//
//	//time_t t0 = time(0);
//	__time64_t t0 = g_pData->GetServerCurrentTime();
//
//	double fDiffTime = difftime( KncUtil::TimeToInt( cTime ), (time_t)t0 );
//
//	int remainDay =  (int)(fDiffTime / ( 60 * 60 * 24 ) );
//	if ( remainDay > 0 )
//		wstrstm << remainDay << GET_STRING( STR_ID_14 ) << L" ";
//
//	int remainHour = ((time_t)fDiffTime % ( 60 * 60 * 24 )) / (60 * 60);
//	if ( remainHour < 0 )
//		remainHour = 0;
//
//	wstrstm << remainHour << GET_STRING( STR_ID_310 ) << L" ";
//
//	int remainMinute = ((time_t)fDiffTime % ( 60 * 60 )) / (60 );
//
//	if ( remainDay <= 0 )
//	{
//		if ( remainMinute <= 0 )
//			remainMinute = 0;
//		wstrstm << remainMinute << GET_STRING( STR_ID_311 ) << L" ";
//	}
//
//	tempDesc += wstrstm.str().c_str();
//
//
//	return tempDesc;
//
//}







void CX2UISkillTreeNew::OpenDLGSkillUnsealed( bool bOpen )
{
	if( false == bOpen )
	{
		if( m_pDLGSkillUnsealed != NULL )
		{
			m_pDLGSkillUnsealed->SetShowEnable( bOpen, bOpen );
		}
	}
	else
	{
		if( m_pDLGSkillUnsealed == NULL )
		{
			m_pDLGSkillUnsealed = new CKTDGUIDialog( g_pMain->GetNowState(), L"DLG_Skill_Acquisition.lua" );
			g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGSkillUnsealed );
		}

		m_pDLGSkillUnsealed->SetShowEnable( bOpen, bOpen );
	}
}



void CX2UISkillTreeNew::UpdateDLGSkillUnsealed( CX2SkillTree::SKILL_ID eSkillID )
{
	if( m_pDLGSkillUnsealed == NULL )
		return;


	const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( eSkillID );
	if( NULL == pSkillTemplet )
		return;
	

	CKTDGUIStatic* pStaticSkillSlot = (CKTDGUIStatic*) m_pDLGSkillUnsealed->GetControl( L"g_pStaticSkill_Acquisition_Slot" );
	if( NULL != pStaticSkillSlot &&
		NULL != pStaticSkillSlot->GetPicture(1) )
	{
		std::string textureName = "";
		std::string texturePieceName = "";

		ConvertWCHARToChar( textureName, pSkillTemplet->m_wstrIconName.c_str() );
		ConvertWCHARToChar( texturePieceName, pSkillTemplet->m_wstrIconPieceName.c_str() );

		pStaticSkillSlot->GetPicture(1)->SetTex_LUA( textureName.c_str(), texturePieceName.c_str() );
	}


	CKTDGUIStatic* pStaticSkillName = (CKTDGUIStatic*) m_pDLGSkillUnsealed->GetControl( L"Static_Skill_Name" );
	if( NULL != pStaticSkillName )
	{
		pStaticSkillName->GetString(0)->msg = pSkillTemplet->m_wstrName;
	}

	/// ��ų Ʈ�� ������Ʈ
	g_pData->GetUIManager()->GetUISkillTree()->UpdateUI( false, false, true );
}

//{{ kimhc // 2010.3.26 // ���� ��ų ���� ����
#ifdef MODIFY_INFINITE_SKILL_BUG
bool CX2UISkillTreeNew::ProcessSkillTreeSlotRMouseUp( LPARAM lParam )
{
	CX2State* pNowState = static_cast<CX2State*>( g_pMain->GetNowState() );
	if ( NULL != pNowState && false == pNowState->GetEnableShortCutKey() )	/// ����Ű�� ��� ���ϴ� ���̸�
		return true;	/// �巡�׾� ����� ���ϰ� ����

	CKTDGUISlot* pSlot = (CKTDGUISlot*) lParam;
	CX2SkillTreeSlotData* pSlotData = (CX2SkillTreeSlotData*)pSlot->GetSlotData();
	if( pSlotData != NULL)
	{
		/// ���� ������ ��ų ������ ������ 0�̸�, ��ų ���� ���� ���� ����
		if ( NULL != g_pData && 
			 NULL != g_pData->GetMyUser() &&
			 NULL != g_pData->GetMyUser()->GetSelectUnit() &&
			 NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
		{
			const int iSkillLevel = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetSkillLevel( pSlotData->m_eSkillID );

			if ( 0 >= iSkillLevel )
			{
				g_pMain->KTDGUIOKMsgBox( D3DXVECTOR2(250,300), GET_STRING( STR_ID_25347 ), g_pMain->GetNowState() );

				return false;
			}
		}

		const CX2SkillTree::SkillTemplet* pSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( pSlotData->m_eSkillID );
		if(pSkillTemplet != NULL)
		{		
			if( true == pSkillTemplet->m_bBornSealed && false == m_pUserSkillTree->IsSkillUnsealed(pSlotData->m_eSkillID) )
			{
				D3DXVECTOR2 CenterPos = pSlot->GetPos() + pSlot->GetDialog()->GetPos() + pSlot->GetSize() * 0.5f;
				OpenSkillUnsealInformDLG( pSkillTemplet, CenterPos );
			}
			else if( m_pUserSkillTree->DoIHaveThisSkill( pSlotData->m_eSkillID ) )
			{

				switch(pSkillTemplet->m_eType)
				{
				case CX2SkillTree::ST_ACTIVE:
				case CX2SkillTree::ST_BUFF:
				case CX2SkillTree::ST_SPECIAL_ACTIVE:
#ifdef ADDED_RELATIONSHIP_SYSTEM
				case CX2SkillTree::ST_RELATIONSHIP_SKILL:
#endif // ADDED_RELATIONSHIP_SYSTEM
					{
						int iAlreadyEquippedSlotID = m_pUserSkillTree->GetSkillEquippedSlotIndex(pSlotData->m_eSkillID, false );
						int iAlreadyEquippedSlotBID = m_pUserSkillTree->GetSkillEquippedSlotIndex(pSlotData->m_eSkillID, true );
						// ���� ��ų�� �����Ǿ� ���� ������
						if( -1 == iAlreadyEquippedSlotID && -1 == iAlreadyEquippedSlotBID )					
						{
							/// A ������ ����ִ��� Ȯ��
							iAlreadyEquippedSlotID = m_pUserSkillTree->GetSkillEquippedSlotIndex(CX2SkillTree::SI_NONE, false );
							if( -1 != iAlreadyEquippedSlotID )
							{
								g_pKTDXApp->GetDeviceManager()->PlaySound( L"Skill_Equip.ogg", false, false );
								g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( iAlreadyEquippedSlotID, pSlotData->m_eSkillID );
								return true;
							}
							else
							{
								// B  �� ������ ã�Ƽ� �����Ѵ�
								if( true == m_bIsExpandSkillSlot )
								{
									iAlreadyEquippedSlotID = m_pUserSkillTree->GetSkillEquippedSlotIndex(CX2SkillTree::SI_NONE, true );
									if( -1 != iAlreadyEquippedSlotID )
									{
										g_pKTDXApp->GetDeviceManager()->PlaySound( L"Skill_Equip.ogg", false, false );
										g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( iAlreadyEquippedSlotID+EQUIPPED_SKILL_SLOT_COUNT, pSlotData->m_eSkillID );
										return true;
									}
								}
							}
						}
					} break;
				default:
					break;
				}
			}
		}
	}
	return true;
}

bool CX2UISkillTreeNew::ProcessEquipSlotDropped( LPARAM lParam )
{
	CKTDGUISlot* pSlot = (CKTDGUISlot*) lParam;
	CX2EquippedSkillSlotData* pSlotData = (CX2EquippedSkillSlotData*)pSlot->GetSlotData();

	CX2State* pNowState = static_cast<CX2State*>( g_pMain->GetNowState() );
	if ( NULL != pNowState && false == pNowState->GetEnableShortCutKey() )	/// ����Ű�� ��� ���ϴ� ���̸�
		return false;	/// �巡�׾� ����� ���ϰ� ����

	CKTDGUISlot* pDroppedSlot = pSlot->GetDroppedSlot();
	CKTDGUISlotData* pDroppedSlotData = NULL;
	if( pDroppedSlot != NULL )
	{
		pDroppedSlotData = pDroppedSlot->GetSlotData();
		
		if ( NULL != pDroppedSlotData )
		{
			switch(pDroppedSlotData->m_eSlotDataType)
			{
			case CKTDGUISlotData::SDT_SKILL_TREE:
				{
					g_pKTDXApp->GetDeviceManager()->PlaySound( L"Skill_Equip.ogg", false, false );
					CX2SkillTreeSlotData* pDropData = (CX2SkillTreeSlotData*) pDroppedSlotData;
#ifndef SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
					int iAlreadyEquippedSlotID = m_pUserSkillTree->GetSkillEquippedSlotIndex(pDropData->m_eSkillID, false );
					int iAlreadyEquippedSlotBID = m_pUserSkillTree->GetSkillEquippedSlotIndex(pDropData->m_eSkillID, true );

					// ���� ��ų�� ��ų���Կ� ���� �Ǿ� ������ ����
					if( -1 != iAlreadyEquippedSlotID )					
					{
						g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( iAlreadyEquippedSlotID, CX2SkillTree::SI_NONE );
					}
					if( -1 != iAlreadyEquippedSlotBID)
					{
						g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( iAlreadyEquippedSlotBID+EQUIPPED_SKILL_SLOT_COUNT, CX2SkillTree::SI_NONE );
					}
#endif // SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE

					if ( NULL != pSlotData )
						g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), pDropData->m_eSkillID );

					return true;
				} break;
			case CKTDGUISlotData::SDT_SKILL_EQUIP:
				{
					g_pKTDXApp->GetDeviceManager()->PlaySound( L"Skill_Equip.ogg", false, false );
#ifdef SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
					CX2EquippedSkillSlotData* pDropData = (CX2EquippedSkillSlotData*) pDroppedSlotData;
					if ( NULL != pSlotData  )
						g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), pDropData->m_eSkillID );
#else SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE
					// Skill Swap
					CX2EquippedSkillSlotData* pDropData = (CX2EquippedSkillSlotData*) pDroppedSlotData;
					CX2SkillTree::SKILL_ID SkillIDA = pSlotData->m_eSkillID;
					CX2SkillTree::SKILL_ID SkillIDB = pDropData->m_eSkillID;
					g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), SkillIDB );
					g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pDropData->GetSlotID(), SkillIDA );
#endif // SERV_SKILL_SLOT_CHANGE_PACKET_INTEGRATE

					return true;
				} break;
			default:
				break;
			}
		}
	} 
	return false;
}

bool CX2UISkillTreeNew::ProcessEquipSlotRMouseUp( LPARAM lParam )
{
	CX2State* pNowState = static_cast<CX2State*>( g_pMain->GetNowState() );
	if ( NULL != pNowState && false == pNowState->GetEnableShortCutKey() )	/// ����Ű�� ��� ���ϴ� ���̸�
		return true;	/// �巡�׾� ����� ���ϰ� ����

	CKTDGUISlot* pSlot = (CKTDGUISlot*) lParam;
	CX2EquippedSkillSlotData* pSlotData = (CX2EquippedSkillSlotData*)pSlot->GetSlotData();
	if(pSlotData->m_eSkillID != CX2SkillTree::SI_NONE)
	{
		g_pKTDXApp->GetDeviceManager()->PlaySound( L"Skill_UnEquip.ogg", false, false );
	}			

	g_pData->GetSkillTree()->Handler_EGS_CHANGE_SKILL_SLOT_REQ( pSlotData->GetSlotID(), CX2SkillTree::SI_NONE );
	return true;
}

// bool CX2UISkillTreeNew::CheckIsReady()
// {
// 	bool bIsReady = false;
// 
// 	switch ( g_pMain->GetNowStateID() )
// 	{
// 	case CX2Main::XS_PVP_ROOM:
// 		{
// 			CX2PVPRoom* pPvpRoom = g_pData->GetPVPRoom();
// 
// 			if ( pPvpRoom != NULL &&
// 				pPvpRoom->GetMySlot() != NULL && 
// 				pPvpRoom->GetMySlot()->m_bReady == true )
// 				bIsReady = true;
// 		} break;
// 
// 	case CX2Main::XS_VILLAGE_MAP:
// 		{
// 			CX2PartyManager::PartyMemberData* pPartyMemberData = 
// 				g_pData->GetPartyManager()->GetMyPartyMemberData();
// 
// 			if ( pPartyMemberData != NULL &&
// 				pPartyMemberData->m_bGameReady == true )
// 				bIsReady = true;
// 		} break;
// 
// 	default:
// 		break;
// 	}
// 
// 	return bIsReady;
// }
// 
// void CX2UISkillTreeNew::ShowOkAndCancelMsgForUnReady( LPARAM lParam, SKILL_TREE_UI_CUSTOM_MSG eSTUCM )
// {
// 	m_pDLGUnReady = g_pMain->KTDGUIOkAndCancelMsgBox( D3DXVECTOR2( -999, -999 ), GET_STRING( STR_ID_4958 ),
// 		STUCM_UNREADY_OK, g_pMain->GetNowState(), STUCM_UNREADY_CANCEL );
// 	
// 	m_pDLGUnReady->ClearDummyInt();
// 	m_pDLGUnReady->AddDummyInt( static_cast<int>(lParam) );	// ù��° ���̿� ���� Ŀ���� MSG�� lParam ��( ���⼭�� ���� )
// 	m_pDLGUnReady->AddDummyInt( static_cast<int>(eSTUCM) );	// �ι�° ���� int�� ������ �����ߴ� Ŀ���� MSG�� �����ΰ�
// }
// 
// void CX2UISkillTreeNew::ProcessChangeEquipSlotAfterUnReady()
// {
// 	if ( m_pDLGUnReady == NULL )
// 		return;
// 
// 	LPARAM lParam = m_pDLGUnReady->GetDummyInt( 0 );
// 
// 	switch ( m_pDLGUnReady->GetDummyInt( 1 ) )
// 	{
// 	case STUCM_SKILLTREESLOT_RMOUSEUP:
// 		ProcessSkillTreeSlotRMouseUp( lParam );
// 		break;
// 
// 	case STUCM_EQUIPSLOT_DROPPED:
// 		ProcessEquipSlotDropped( lParam );
// 		break;
// 
// 	case STUCM_EQUIPSLOT_RMOUSEUP:
// 		ProcessEquipSlotRMouseUp( lParam );
// 		break;
// 
// 	default:
// 		break;
// 
// 	}
// 
// 	g_pKTDXApp->SendGameDlgMessage( XGM_DELETE_DIALOG, m_pDLGUnReady, NULL, false );
// }
#endif MODIFY_INFINITE_SKILL_BUG
//}} kimhc // 2010.3.26 // ���� ��ų ���� ����

#ifdef ELSWORD_WAY_OF_SWORD
void CX2UISkillTreeNew::DrawElswordWSSDesc( D3DXVECTOR2 vDrawPos_ )
{
	if( CX2Unit::UT_ELSWORD != g_pData->GetMyUser()->GetSelectUnit()->GetType() )
	{
		return;
	}

	if( m_pDLGElswordWSSDesc == NULL)
	{
		m_pDLGElswordWSSDesc = new CKTDGUIDialog( g_pMain->GetNowState(), L"", 0.07f, XDL_POP_UP );
		m_pDLGElswordWSSDesc->SetFront(true);
		m_pDLGElswordWSSDesc->SetCloseOnFocusOut(true);

		g_pKTDXApp->GetDGManager()->GetDialogManager()->AddDlg( m_pDLGElswordWSSDesc );
		m_pDLGElswordWSSDesc->SetFront( true );
		m_pDLGElswordWSSDesc->SetModal( true );

		CKTDGUIStatic* pStatic = new CKTDGUIStatic();
		pStatic->SetName( L"ItemDesc" );
		m_pDLGElswordWSSDesc->AddControl( pStatic );
		CKTDGUIControl::CPictureData* pPicture = new CKTDGUIControl::CPictureData();
		pStatic->AddPicture( pPicture );
		pPicture->SetEdge( false, 2, D3DXCOLOR(1,1,1,1) );
		pPicture->SetTex( L"FadeInOut.dds" );

		CKTDGUIControl::UIStringData* pString = new CKTDGUIControl::UIStringData();
		pStatic->AddString( pString );
		pString->fontIndex = SLOT_MANAGER_FONT_INDEX;
		pString->color = D3DXCOLOR( 1,1,1,1 );
		pString->outlineColor = D3DXCOLOR( 1,1,1,1 );
		pString->msg = L"";
		pString->fontStyle = CKTDGFontManager::FS_NONE;
		pString->sortFlag = DT_LEFT;
		pString->pos = D3DXVECTOR2( 20, 20 );
	}

	int itemDescWidth = 300;
	int itemDescHeight = 200;

	int itemDescPosX = (int)vDrawPos_.x;
	int itemDescPosY = (int)vDrawPos_.y;
	
	if( g_pX2Game != NULL && g_pX2Game->GetMyUnit() != NULL )
	{
		wstringstream wstrm;
		
		switch( g_pX2Game->GetMyUnit()->GetWayOfSwordState() )
		{
			case -2: // CX2GUElsword_SwordMan::WSS_DESTRUCTION
				{
					wstrm << L"#CFFFFFF" << L"\n" << GET_STRING(STR_ID_14553) << L"\n" <<  L"#CX" <<
						L"\n" << L"#CFF4444" << L"\n" << GET_STRING(STR_ID_14554) << L"#CX" << L"\n\n";
					m_pDLGElswordWSSDesc->SetShow( true );
				} break;
			case -1: // CX2GUElsword_SwordMan::WSS_TOWARD_DESTRUCTION
				{
					wstrm << L"#CFFFFFF" << L"\n" << GET_STRING(STR_ID_14553) << L"\n" <<  L"#CX" <<
						L"\n" << L"#CFF4444" << L"\n" << GET_STRING(STR_ID_14554) << L"#CX" << L"\n\n";
					m_pDLGElswordWSSDesc->SetShow( true );
				} break;
			case 0: // CX2GUElsword_SwordMan::WSS_CENTER
				{
					wstrm << L"#CFFFFFF" << L"\n" << GET_STRING(STR_ID_14555) << L"\n" <<  
						L"\n" << L"#C999999" << L"\n" << GET_STRING(STR_ID_14556) << L"#CX" << L"\n\n";
					m_pDLGElswordWSSDesc->SetShow( true );
				} break;
			case 1: // CX2GUElsword_SwordMan::WSS_TOWARD_VIGOR
				{
					wstrm << L"#CFFFFFF" << L"\n" << GET_STRING(STR_ID_14557) <<  L"#CX" << 
						L"\n" << L"#C4488FF" << L"\n" << GET_STRING(STR_ID_14558) << L"#CX" << L"\n\n";
					m_pDLGElswordWSSDesc->SetShow( true );
				} break;
			case 2: // CX2GUElsword_SwordMan::WSS_VIGOR
				{
					wstrm << L"#CFFFFFF" << L"\n" << GET_STRING(STR_ID_14557) <<  L"#CX" << 
						L"\n" << L"#C4488FF" << L"\n" << GET_STRING(STR_ID_14558) << L"#CX" << L"\n\n";
					m_pDLGElswordWSSDesc->SetShow( true );
				} break;
			default:
				m_pDLGElswordWSSDesc->SetShow( false );
		}
		wstring wstrDesc = wstrm.str();
		CKTDGUIStatic* pItemDesc = (CKTDGUIStatic*)m_pDLGElswordWSSDesc->GetControl(L"ItemDesc");

		const CKTDGFontManager::CUKFont* pFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( pItemDesc->GetString(0)->fontIndex );

		const int MAGIC_ELSWORD_DESC_WIDTH = 330;
		wstrDesc = g_pMain->GetStrByLineBreakColor(wstrDesc.c_str(), (int)((float)MAGIC_ELSWORD_DESC_WIDTH*g_pKTDXApp->GetResolutionScaleX()), SLOT_MANAGER_FONT_INDEX);

		CKTDGFontManager::CUKFont* pItemDescFont = g_pKTDXApp->GetDGManager()->GetDialogManager()->GetUKFont( SLOT_MANAGER_FONT_INDEX );
		int itemDescWidth = (int)( pItemDescFont->GetWidth( wstrDesc.c_str() ) / g_pKTDXApp->GetResolutionScaleX() ) + ITEM_DESC_ADD_SIZE_X;
		int itemDescHeight = (int)( pItemDescFont->GetHeight( wstrDesc.c_str() ) / g_pKTDXApp->GetResolutionScaleY() ) + ITEM_DESC_ADD_SIZE_Y;


		m_pDLGElswordWSSDesc->SetPos( D3DXVECTOR2( (float)itemDescPosX, (float)itemDescPosY ) );
		m_pDLGElswordWSSDesc->GetStatic_LUA( "ItemDesc" )->GetPicture(0)->SetSize( D3DXVECTOR2( (float)itemDescWidth, (float)itemDescHeight) );
		m_pDLGElswordWSSDesc->GetStatic_LUA( "ItemDesc" )->GetPicture(0)->pPoint->color = D3DXCOLOR( 1,1,1,0.7f );		
		m_pDLGElswordWSSDesc->GetStatic_LUA( "ItemDesc" )->GetString(0)->msg = wstrDesc.c_str();
	}
	
	return;
}


#endif ELSWORD_WAY_OF_SWORD

//x2game�� �����Ǿ� ���� ���� �� ����ϴ� ��Ÿ�Ӱ��� �Լ�.
void CX2UISkillTreeNew::UpdateSkillCoolTime( float fElapsedTime )
{
	if( g_pData != NULL &&
		g_pData->GetMyUser() != NULL &&
		g_pData->GetMyUser()->GetSelectUnit() != NULL &&
		g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() != NULL)
	{
		CX2UserSkillTree& refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;
		
		if ( m_pDLGGameSkillSlot != NULL )
		{
			CKTDGUIStatic* pStaticCoolTime = (CKTDGUIStatic*)m_pDLGGameSkillSlot->GetControl( L"Static_Skill_Cool_Time" );
			CKTDGUIStatic* pStatic_Skill_Cool_Time_Fade = (CKTDGUIStatic*)m_pDLGGameSkillSlot->GetControl( L"Static_Skill_Cool_Time_Fade" );

			CKTDGUIStatic* pStaticSlot_A = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_SKILL_SLOT_A" );
			CKTDGUIStatic* pStaticSlot_B = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"Static_SKILL_SLOT_B" );
			for ( int i = 0; i < EQUIPPED_SKILL_SLOT_COUNT; i++ )
			{
				pStaticSlot_A->GetPicture(i)->SetColor( D3DXCOLOR(1.f,1.f,1.f,0.3f));
				pStaticSlot_B->GetPicture(i)->SetColor( D3DXCOLOR(1.f,1.f,1.f,0.3f));
			}
			if ( pStaticCoolTime != NULL && pStatic_Skill_Cool_Time_Fade != NULL )
			{  //B���Ե� ���� �ϱ� ���ؼ� EQUIPPED_SKILL_SLOT_COUNT*2
				for ( int i = 0; i < EQUIPPED_SKILL_SLOT_COUNT*2; i++ )
				{				
					bool bSlotB = (i > 3) ? true : false;
					int iSlotIndex = (i > 3) ? i-4 : i;

					//���� B�� Ȱ��ȭ ���� ������ ���� A�� Ȯ���ϵ��� break;
					if( false == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetEnabledSkillSlotB() 
						&& bSlotB == true)
					{
						break;
					}

					if( true == bSlotB )
					{
						pStatic_Skill_Cool_Time_Fade = (CKTDGUIStatic*)m_pDLGGameSkillSlot->GetControl( L"Static_Skill_Cool_Time_Fade_B" );
						if( NULL == pStatic_Skill_Cool_Time_Fade )
							break;

						pStaticCoolTime = (CKTDGUIStatic*)m_pDLGGameSkillSlot->GetControl( L"Static_Skill_Cool_Time_B" );
						if( NULL == pStaticCoolTime )
							break;
					}


					const CX2UserSkillTree::SkillSlotData* pSlotData = refUserSkillTree.GetSkillSlot(iSlotIndex, bSlotB);
					refUserSkillTree.SetSkillCoolTimeLeft(CX2SkillTree::SI_NONE,0.f);
					if(pSlotData != NULL)
					{			
						if( NULL != pStaticSlot_A && NULL != pStaticSlot_B )
						{
							if( CX2SkillTree::SI_NONE != pSlotData->m_eID )
							{// 0~3 : B����, 4~8 : A����				
								if( true == bSlotB )
								{
									pStaticSlot_B->GetPicture(iSlotIndex)->SetColor( D3DXCOLOR(1.f,1.f,1.f,1.f));
								}
								else
								{
									pStaticSlot_A->GetPicture(iSlotIndex)->SetColor( D3DXCOLOR(1.f,1.f,1.f,1.f));
								}
							}
						}

						if(pSlotData->m_fCoolTimeLeft > 0.0f)
						{
							if (pStatic_Skill_Cool_Time_Fade->GetPicture(iSlotIndex) != NULL )
								pStatic_Skill_Cool_Time_Fade->GetPicture(iSlotIndex)->bShow = true;

							wstringstream wstrstm;
							wstrstm << (int)( ceil(pSlotData->m_fCoolTimeLeft) );
							pStaticCoolTime->SetString( iSlotIndex, wstrstm.str().c_str() );
						}
						else
						{
							if ( pStatic_Skill_Cool_Time_Fade->GetPicture(iSlotIndex) != NULL )
							{
								pStatic_Skill_Cool_Time_Fade->GetPicture(iSlotIndex)->bShow = false;
							}

							pStaticCoolTime->SetString( iSlotIndex, L"" );
						}						
					}			
				}
			}
		}
	}
}

/** @function : SwapSkillSlot
	@brief  : ���� ��ų���� Ÿ�Կ� ���� ���� �Լ� ȣ��
*/
void CX2UISkillTreeNew::SwapSkillSlot(bool bVal, bool bForce/* = false*/)
{

#ifdef SKILL_SLOT_UI_TYPE_B
	if( true == m_bIsSkillUITypeA )
	{
		SwapSkillSlotTypeA( bVal, bForce );
	}
	else
	{
		SwapSkillSlotTypeB( bVal, bForce );
	}
#else
	if( m_bSkillSlotSwap == bVal && false == bForce )
		return;

	m_bSkillSlotSwap = bVal;
	if( true == m_bSkillSlotSwap ) 
	{
		D3DXVECTOR2 vOffsetPos(0,0);
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i )
		{
			vOffsetPos.y = ( i >= 4 ) ? 51.f : -51.f;
			if( NULL != GetEquippedSlot(i) )
			{
				GetEquippedSlot(i)->SetOffsetPos( vOffsetPos );
			}
		}	
		vOffsetPos.y = 51.f;
		SetOffsetPosSkillSLotStatic(vOffsetPos);
	}
	else
	{
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i )
		{
			if( NULL != GetEquippedSlot(i) )
			{
				GetEquippedSlot(i)->SetOffsetPos(D3DXVECTOR2(0,0));
			}
		}

		SetOffsetPosSkillSLotStatic(D3DXVECTOR2(0,0));
	}
#endif //SKILL_SLOT_UI_TYPE_B
}

/** @function : SetOffsetPosSkillSLotStatic
	@brief : ��ų���Կ� ���� ����ƽ�� ������ ����	
*/
void CX2UISkillTreeNew::SetOffsetPosSkillSLotStatic(const D3DXVECTOR2& vPos_, const D3DXVECTOR2& vAddPos_/* = D3DXVECTOR2(0,0)*/)
{
	if( NULL == m_pDLGGameSkillSlot )
		return;

	SetOffsetStatic( L"Static_Skill_Cool_Time_Fade", -vPos_ -vAddPos_);
	SetOffsetStatic( L"Static_Skill_Cool_Time_Fade_B", vPos_ );
	SetOffsetStatic( L"Static_Skill_Cool_Time", -vPos_ -vAddPos_);
	SetOffsetStatic( L"Static_Skill_Cool_Time_B", vPos_ );
	SetOffsetStatic( L"Static_SKILL_SLOT_A", -vPos_ -vAddPos_ );
	SetOffsetStatic( L"Static_SKILL_SLOT_B", vPos_ );
	SetOffsetStatic( L"Static_EL_SKILL_B", -vPos_ -vAddPos_ );
	SetOffsetStatic( L"Static_EL_SKILL_B_B", vPos_ );
	SetOffsetStatic( L"Static_EL_SKILL_R", -vPos_ -vAddPos_ );
	SetOffsetStatic( L"Static_EL_SKILL_R_B", vPos_ );
}

/** @function : SetOffsetStatic
	@brief : ��Ʈ�� �̸����� ���� ����ƽ ��ü�� �����°� ����
			SetOffsetPosSkillSLotStatic���� ���Ǵ� �Լ�( �ڵ��ߺ� ���� )
	@param : ��Ʈ���̸� (strControl_), �����°�(vPos)
*/
void CX2UISkillTreeNew::SetOffsetStatic( const WCHAR* strControl_, const D3DXVECTOR2& vPos_)
{
	CKTDGUIStatic* pStatic = static_cast<CKTDGUIStatic*>(m_pDLGGameSkillSlot->GetControl(strControl_));
	if( NULL != pStatic )
	{
		pStatic->SetOffsetPos(vPos_);
	}
}

#ifdef REFORM_UI_KEYPAD
void CX2UISkillTreeNew::UpdateSkillSlotString()
{
	if ( NULL == m_pDLGGameSkillSlot )
		return;

	if ( NULL != g_pMain->GetKeyPad() )
	{
		CKTDGUIStatic* pStaticSkillHotKey = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"SkillHotKeyEx" );
		if( NULL != pStaticSkillHotKey )
		{
	#ifdef SKILL_SLOT_UI_TYPE_B
			if( false == m_bIsSkillUITypeA )
			{
				pStaticSkillHotKey->GetString(0)->msg = L"";
				pStaticSkillHotKey->GetString(1)->msg = L"";
				pStaticSkillHotKey->GetString(2)->msg = L"";
				pStaticSkillHotKey->GetString(3)->msg = L"";
			}
			else
	#endif //SKILL_SLOT_UI_TYPE_B
			{
				pStaticSkillHotKey->GetString(0)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL_EX1 );
				pStaticSkillHotKey->GetString(1)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL_EX2 );
				pStaticSkillHotKey->GetString(2)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL_EX3 );
				pStaticSkillHotKey->GetString(3)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL_EX4 );
			}
		}

		pStaticSkillHotKey = (CKTDGUIStatic*) m_pDLGGameSkillSlot->GetControl( L"SkillHotKey" );
		{
			pStaticSkillHotKey->GetString(0)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL1 );
			pStaticSkillHotKey->GetString(1)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL2 );
			pStaticSkillHotKey->GetString(2)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL3 );
			pStaticSkillHotKey->GetString(3)->msg = g_pMain->GetKeyPad()->GetKeyString( GAMEACTION_SKILL4 );
		}
	}
}
#endif

#ifdef SKILL_SLOT_UI_TYPE_B
void CX2UISkillTreeNew::SetSkillUIType( bool bIsTypeA_ )
{
	if( NULL == m_pDLGGameSkillSlot)
		return;

	m_bIsSkillUITypeA = bIsTypeA_;


	D3DXVECTOR2 vSlotPos(0,0);
	if( false == m_bIsSkillUITypeA )
	{
		vSlotPos.x = -297.f;
#ifdef INT_WIDE_BAR
		vSlotPos.y = -599.f;
#else // INT_WIDE_BAR
		vSlotPos.y = -619.f;
#endif // INT_WIDE_BAR
	}
	//���� B Ȱ��ȭ ����
	bool bEnableSlotB = false;
	if( NULL != g_pData && NULL != g_pData->GetMyUser() && NULL != g_pData->GetMyUser()->GetSelectUnit() &&
		NULL != g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		if( true == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree.GetEnabledSkillSlotB() )
			bEnableSlotB = true;
	}

	//���� ������ A�������� B�������� �˷��ִ� UI
	CKTDGUIStatic* pStatic = static_cast<CKTDGUIStatic*>(m_pDLGGameSkillSlot->GetControl( L"g_pStatic_Slot_Guide" ));
	if( NULL != pStatic )
	{
		if( false == m_bIsSkillUITypeA && //��ų���� BŸ���̰�
			true == bEnableSlotB ) //���� B�� Ȱ��ȭ �Ǿ� �ִٸ�
			pStatic->SetShow(true); 
		else
			pStatic->SetShow(false);
	}

	
	

 	m_pDLGGameSkillSlot->SetPos( vSlotPos );

	SetOffsetPosEtcSkillSlotUI();// B���� ���/���Ű��̵� ��ġ ����
	UpdateSkillSlotString();// B���� ����Ű ��Ʈ�� ����
	SwapSkillSlot( m_bSkillSlotSwap, true );
}

void CX2UISkillTreeNew::SetOffsetPosEtcSkillSlotUI()
{
	if( NULL == m_pDLGGameSkillSlot )
		return;

	D3DXVECTOR2 vOffsetPos(0,0);
	if( false == m_bIsSkillUITypeA )
		vOffsetPos.y = -200.f;

	CKTDGUIStatic* pStaticLock = static_cast<CKTDGUIStatic*>(m_pDLGGameSkillSlot->GetControl(L"LOCK"));
	if( NULL != pStaticLock )
		pStaticLock->SetOffsetPos(vOffsetPos);

	CKTDGUIButton* pButtonBuyGuide = static_cast<CKTDGUIButton*>(m_pDLGGameSkillSlot->GetControl(L"Button_Buy_Guide_Slot"));
		pButtonBuyGuide->SetOffsetPos(vOffsetPos);

}

/** @function : SwapSkillSlotTypeA
	@brief  : ���/�ϴ� ���� ���� ��ġ ����
*/
void CX2UISkillTreeNew::SwapSkillSlotTypeA(bool bVal, bool bForce/* = false*/)
{
	if( m_bSkillSlotSwap == bVal && false == bForce )
		return;

	m_bSkillSlotSwap = bVal;
	if( true == m_bSkillSlotSwap ) 
	{
		D3DXVECTOR2 vOffsetPos(0,0);
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i )
		{
			vOffsetPos.y = ( i >= 4 ) ? 51.f : -51.f;
			if( NULL != GetEquippedSlot(i) )
			{
				GetEquippedSlot(i)->SetOffsetPos( vOffsetPos );
			}
		}	
		vOffsetPos.y = 51.f;
		SetOffsetPosSkillSLotStatic(vOffsetPos);
	}
	else
	{
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i )
		{
			if( NULL != GetEquippedSlot(i) )
			{
				GetEquippedSlot(i)->SetOffsetPos(D3DXVECTOR2(0,0));
			}
		}

		SetOffsetPosSkillSLotStatic(D3DXVECTOR2(0,0));
	}
}


/** @function : SwapSkillSlotTypeB
	@brief  : ��� ������ ȭ�鿡�� �Ⱥ��̵��� ���� �����鼭 ��ġ ����
*/
void CX2UISkillTreeNew::SwapSkillSlotTypeB(bool bVal_, bool bForce_/* = false */)
{
	if( m_bSkillSlotSwap == bVal_ && false == bForce_)
		return;

	m_bSkillSlotSwap = bVal_;

	D3DXVECTOR2 vOffsetPos(0,0);	// ���� ��ü�� ���� �⺻ ������ ��
	D3DXVECTOR2 vOffsetAddPos(0,0); // ��� ������ ���� �� ������ ���� �߰� ������ ��

	if( true == m_bSkillSlotSwap ) //���� ��ü ��(B������ �ϴܿ� ������ ����)
	{
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i )//4~7 ��ܽ���
		{
			vOffsetPos.y = ( i >= 4 ) ? 51.f : -200.f;
			if( NULL != GetEquippedSlot(i) )
				GetEquippedSlot(i)->SetOffsetPos( vOffsetPos );
		}	
		vOffsetPos.y = 51.f;
		vOffsetAddPos.y = 200.f;

	}
	else //�⺻ ����( A������ �ϴܿ� ������ ����)
	{
		for( int i = 0 ; i < EQUIPPED_SKILL_SLOT_COUNT * 2; ++i ) //4~7 ��ܽ���
		{
			if( NULL != GetEquippedSlot(i) )
				vOffsetPos.y = ( i >= 4 ) ? -150.f : 0.f;
				GetEquippedSlot(i)->SetOffsetPos( vOffsetPos );
		}

		vOffsetPos.y = -200.f;
		vOffsetAddPos.y = 200.f;
	}
	SetOffsetPosSkillSLotStatic(vOffsetPos, vOffsetAddPos);
}
#endif //SKILL_SLOT_UI_TYPE_B

/** @function	: CreateSkillTreePortrait
	@brief		: ��ų Ʈ�� ���� ������ �ٴ� �ʻ�ȭ ���� �̹��� ���� �Լ�
	@param		: ��Ʈ�� ����Ʈ ������, �ִ� ��ų �迭 ����
*/
void CX2UISkillTreeNew::CreateSkillTreePortrait( CKTDGUIContolList* pControlList )
{
	if( NULL == m_pDLGUISkillTree || 
		NULL == pControlList ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit()  ) 
	{
		return;
	}

	CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();

	if ( NULL == pUnit )
		return;

	WCHAR buf[255] = {0,};

	/// �ʻ�ȭ UI ���� ( �ϳ��� ������ 5��, ���� ���� ����Ͽ� ������ �ʻ�ȭ UI ǥ�� ���� ����  )
	int iPortraitUINum = pUnit->GetClassLevel() + 1;

	/// �ʻ�ȭ ����
	for ( int i = 0; i < iPortraitUINum; ++i )
	{
		m_pDLGUISkillTree->OpenScriptFile( L"DLG_UI_Skill_Tree_Portrait.lua" );

		/// �ʻ�ȭ �ڽ� ------------------------------------------------------------------------------------------------------------------
		CKTDGUIStatic* pStaticPortraitBox = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_PortraitBox" ) );

		if ( NULL != pStaticPortraitBox )
		{
			wstring wstrTectureKey = L"Bg_Portrait_Aisha";	/// �ʻ�ȭ ��� �ؽ�ó Ű��

			/// ���� Ÿ�Կ� ���� �ʻ�ȭ ��� ����
			switch ( pUnit->GetType() )
			{
			case CX2Unit::UT_ELSWORD:
#ifdef NEW_CHARACTER_EL // ������ ���� ����
			case CX2Unit::UT_ELESIS:
#endif //NEW_CHARACTER_EL
				{
					wstrTectureKey = L"Bg_Portrait_Elsword";
				}
				break;
			case CX2Unit::UT_ARME:
				{
					wstrTectureKey = L"Bg_Portrait_Aisha";
				}
				break;
			case CX2Unit::UT_LIRE:
				{
					wstrTectureKey = L"Bg_Portrait_Lena";
				}
				break;
			case CX2Unit::UT_RAVEN:
				{
					wstrTectureKey = L"Bg_Portrait_Raven";
				}
				break;
			case CX2Unit::UT_EVE:
				{
					wstrTectureKey = L"Bg_Portrait_Eve";
				}
				break;
			case CX2Unit::UT_CHUNG:
				{
					wstrTectureKey = L"Bg_Portrait_Chung";
				}
				break;
			case CX2Unit::UT_ARA:
				{
					wstrTectureKey = L"Bg_Portrait_Ara";
				}
				break;
			}

			pStaticPortraitBox->GetPicture( 0 )->SetTex( L"DLG_UI_Common_Texture_NewSkill_01.tga", wstrTectureKey.c_str() );

			StringCchPrintf( buf, 255, L"pStaticPortraitBox_%d", i );
			pStaticPortraitBox->SetName(buf);

			pControlList->InsertItem( pStaticPortraitBox, ( i * MAX_ONE_CLASS_SKILL_ROW_NUM ), 0 );							/// �� ���� 5�������� �ֻ��

			SetUnitClassImageAndName( pStaticPortraitBox, pUnit->GetClass(), static_cast<CX2Unit::UNIT_CLASS_TYPE>( i ) );	/// �ʻ�ȭ �� ������ ��Ʈ�� ����
		}

		/// �ʻ�ȭ ���� �ϴ� ���� --------------------------------------------------------------------------------------------------------
		CKTDGUIStatic* pStaticPortraitBottomStart = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_PortraitBottomStart" ) );

		if ( NULL != pStaticPortraitBottomStart )
		{
			StringCchPrintf( buf, 255, L"g_pStatic_PortraitBottomStart_%d", i );
			pStaticPortraitBottomStart->SetName(buf);

			pControlList->InsertItem( pStaticPortraitBottomStart, ( i * MAX_ONE_CLASS_SKILL_ROW_NUM ) + 1, 0 );		/// �� ���� 5�������� �߰� ���
		}

		/// �ʻ�ȭ ����	�ϴ� �߰� ----------------------------------------------------------------------------------------------------------
		CKTDGUIStatic* pStaticPortraitBottomMiddle = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_PortraitBottomMiddle" ) );

		if ( NULL != pStaticPortraitBottomMiddle )
		{
			StringCchPrintf( buf, 255, L"g_pStatic_PortraitBottomMiddle_%d", i );
			pStaticPortraitBottomMiddle->SetName(buf);

			pControlList->InsertItem( pStaticPortraitBottomMiddle, ( i * MAX_ONE_CLASS_SKILL_ROW_NUM ) + 2, 0 );	/// �� ���� 5�������� �߰� �ϴ�
		}

		/// �ʻ�ȭ ����	�ϴ� �� ----------------------------------------------------------------------------------------------------------
		CKTDGUIStatic* pStaticPortraitBottomEnd = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_PortraitBottomEnd" ) );

		if ( NULL != pStaticPortraitBottomEnd )
		{
			StringCchPrintf( buf, 255, L"g_pStatic_PortraitBottomEnd_%d", i );
			pStaticPortraitBottomEnd->SetName(buf);

			pControlList->InsertItem( pStaticPortraitBottomEnd, ( i * MAX_ONE_CLASS_SKILL_ROW_NUM ) + 3, 0 );		/// �� ���� 5�������� ���ϴ�
		}
	}

	return;
}

/** @function	: SetSelectSkillSlotPosition
	@brief		: 2������ ��ų ��, 1���� ���� �� �߾����� ��ġ �̵���Ű�� �Լ�
	@param		: �ִ� ��ų �迭 ����
*/
void CX2UISkillTreeNew::SetSelectSkillSlotPosition()
{
	for ( int i = 0; i < m_iMaxSkillRowNum; ++i )
	{
		/// 2�� ���� ��ų ��, ���� ��ų�� �����ϴ��� �˻�
		std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator itSecond = m_mapSkillPosition.find( SkillPositionKey( 1, i ) );

		if( itSecond != m_mapSkillPosition.end() )	/// ���� ��ų�� �����Ѵٸ�, ���� ��ų ��ġ �̵� ��ų �ʿ� ����.
			continue;

		/// 2�� ���� ���� ��ų ���̵� ��ȯ
		std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator itFirst = m_mapSkillPosition.find( SkillPositionKey( 0, i ) );

		if ( itFirst != m_mapSkillPosition.end() )
		{
			CX2SkillTree::SKILL_ID eSkillID = itFirst->second;

			WCHAR buf[256] = {0,};

			/// ���� ��ġ ����
			StringCchPrintf( buf, 255, L"g_pSlot_SkillTree_%d", eSkillID );

			CKTDGUISlot* pSlotSkillTree = static_cast<CKTDGUISlot*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pSlotSkillTree )
			{
				CKTDGUIControl::UIPointData* pPoint		= new CKTDGUIControl::UIPointData();
				pPoint->fChangeTime						= 0.0f;
				pPoint->addSize							= D3DXVECTOR2( 46.f, 46.f );
				pPoint->color							= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
				pPoint->leftTopPoint					= D3DXVECTOR2( 162.f, 16.f );

				CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
				pOverPoint->fChangeTime					= 0.0f;
				pOverPoint->color						= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
				pOverPoint->leftTopPoint				= D3DXVECTOR2( 156.f, 9.f );

				pSlotSkillTree->SetPoint( pPoint );
				pSlotSkillTree->SetMouseOverPoint( pOverPoint );
			}

			/// ��� ���� ��� ���� ����
			StringCchPrintf( buf, 256, L"g_pStatic_BackGround_%d", eSkillID );
			
			CKTDGUIStatic* pStaticBackGround = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pStaticBackGround && NULL != pStaticBackGround->GetPicture( 2 ) )
			{
				pStaticBackGround->GetPicture( 2 )->SetShow( false );	/// 2������ ��ų�ε� �Ѱ��� �ִٸ�, ��� ���м� ����
			}


			/// ��ų ���� ��ġ ���� 
			StringCchPrintf( buf, 255, L"g_pStaticSkillBackGround_%d", eSkillID );

			CKTDGUIStatic* pStaticSkillBackGround = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pStaticSkillBackGround && 
				 NULL != pStaticSkillBackGround->GetPicture( 0 ) && 
				 NULL != pStaticSkillBackGround->GetPicture( 1 ) )
			{
				pStaticSkillBackGround->GetPicture( 0 )->SetPos( D3DXVECTOR2( 159.f, 13.f ) );

				/// �䱸 SP ���� ��ġ ����
				D3DXVECTOR2 vecSkillBGPos = pStaticSkillBackGround->GetPicture( 0 )->GetPos();
				pStaticSkillBackGround->GetPicture( 1 )->SetPos( D3DXVECTOR2( vecSkillBGPos.x + 21.f, vecSkillBGPos.y + 52.f ) );
			}


			/// ��ų ���� ��ư ��ġ ���� 
			StringCchPrintf( buf, 255, L"ButtonSkillLearn_%d", eSkillID );

			CKTDGUIButton* pButtonSkillLearn = static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pButtonSkillLearn )
			{
				CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
				pNormalPoint->fChangeTime	= 0.0f;
				pNormalPoint->leftTopPoint	= D3DXVECTOR2( 161.f, 65.f );
				pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
				pOverPoint->fChangeTime		= 0.0f;
				pOverPoint->leftTopPoint	= D3DXVECTOR2( 159.f, 63.f );
				pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
				pDownPoint->fChangeTime		= 0.0f;
				pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
				pDownPoint->leftTopPoint	= D3DXVECTOR2( 160.f, 64.f );
				pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
				pDisablePoint->fChangeTime	= 0.0f;
				pDisablePoint->leftTopPoint	= D3DXVECTOR2( 161.f, 65.f );
				pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				pButtonSkillLearn->SetNormalPoint( pNormalPoint );
				pButtonSkillLearn->SetOverPoint( pOverPoint );
				pButtonSkillLearn->SetDownPoint( pDownPoint );
				pButtonSkillLearn->SetDisablePoint( pDisablePoint );
			}



			/// ��ų ������ ��ư ��ġ ���� 
			StringCchPrintf( buf, 255, L"ButtonSkillDesc_%d", eSkillID );

			CKTDGUIButton* pButtonSkillDesc = static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pButtonSkillDesc )
			{
				CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
				pNormalPoint->fChangeTime	= 0.0f;
				pNormalPoint->leftTopPoint	= D3DXVECTOR2( 161.f, 65.f );
				pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

				CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
				pOverPoint->fChangeTime		= 0.0f;
				pOverPoint->leftTopPoint	= D3DXVECTOR2( 159.f, 63.f );
				pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

				CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
				pDownPoint->fChangeTime		= 0.0f;
				pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
				pDownPoint->leftTopPoint	= D3DXVECTOR2( 160.f, 64.f );
				pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

				CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
				pDisablePoint->fChangeTime	= 0.0f;
				pDisablePoint->leftTopPoint	= D3DXVECTOR2( 161.f, 65.f );
				pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 0.0f );

				pButtonSkillDesc->SetNormalPoint( pNormalPoint );
				pButtonSkillDesc->SetOverPoint( pOverPoint );
				pButtonSkillDesc->SetDownPoint( pDownPoint );
				pButtonSkillDesc->SetDisablePoint( pDisablePoint );
				pButtonSkillDesc->SetGuideDescAbsolutePos( pNormalPoint->leftTopPoint );
			}



			/// ��ų �ʱ�ȭ ��ư ��ġ ���� 
			StringCchPrintf( buf, 255, L"ButtonSkillInit_%d", eSkillID );

			CKTDGUIButton* pButtonSkillInit = static_cast<CKTDGUIButton*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pButtonSkillInit )
			{
				CKTDGUIControl::UIPointData* pNormalPoint = new CKTDGUIControl::UIPointData();
				pNormalPoint->fChangeTime	= 0.0f;
				pNormalPoint->leftTopPoint	= D3DXVECTOR2( 162.f, 65.f );
				pNormalPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pOverPoint = new CKTDGUIControl::UIPointData();
				pOverPoint->fChangeTime		= 0.0f;
				pOverPoint->leftTopPoint	= D3DXVECTOR2( 162.f, 65.f );
				pOverPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pDownPoint = new CKTDGUIControl::UIPointData();
				pDownPoint->fChangeTime		= 0.0f;
				pDownPoint->addSize			= D3DXVECTOR2( -2.f, -2.f );
				pDownPoint->leftTopPoint	= D3DXVECTOR2( 163.f, 66.f );
				pDownPoint->color			= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				CKTDGUIControl::UIPointData* pDisablePoint = new CKTDGUIControl::UIPointData();
				pDisablePoint->fChangeTime	= 0.0f;
				pDisablePoint->leftTopPoint	= D3DXVECTOR2( 162.f, 65.f );
				pDisablePoint->color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

				pButtonSkillInit->SetNormalPoint( pNormalPoint );
				pButtonSkillInit->SetOverPoint( pOverPoint );
				pButtonSkillInit->SetDownPoint( pDownPoint );
				pButtonSkillInit->SetDisablePoint( pDisablePoint );
			}



			/// ���� ��Ʈ�� ��ġ ���� 
			StringCchPrintf( buf, 255, L"g_pStaticStringInfo_%d", eSkillID );

			CKTDGUIStatic* pStaticStringInfo = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pStaticStringInfo && NULL != pStaticStringInfo->GetString( 0 ) )
			{
				pStaticStringInfo->GetString( 0 )->pos = D3DXVECTOR2( 194.f, 66.f );
			}


			/// ������ ��ų �̹��� ��ġ ���� 
			StringCchPrintf( buf, 255, L"g_pStaticSkillMaster_%d", eSkillID );

			CKTDGUIStatic* pStaticSkillMaster = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pStaticSkillMaster && NULL != pStaticSkillMaster->GetPicture( 0 ) )
			{
				pStaticSkillMaster->GetPicture( 0 )->SetPos( D3DXVECTOR2( 162.f, 65.f ) );
			}


			/// �ڹ��� �̹��� ��ġ ���� 
			StringCchPrintf( buf, 255, L"g_pStaticSkill_Lock_%d", eSkillID );

			CKTDGUIStatic* pStaticSkill_Lock = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( buf ) );

			if ( NULL != pStaticSkill_Lock && NULL != pStaticSkill_Lock->GetPicture( 0 ) )
			{
				pStaticSkill_Lock->GetPicture( 0 )->SetPos( D3DXVECTOR2( 170.f, 19.f ) );
			}
		}
	}
}

/** @function	: SetDivisionLine
	@brief		: ������ ��ų ���м� ���� �Լ�
	@param		: ��Ʈ�� ����Ʈ ������
*/
void CX2UISkillTreeNew::SetDivisionLine( CKTDGUIContolList* pControlList )
{
	if( NULL == pControlList ||
		NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() )
	{
		return;
	}

	CX2Unit* pUnit = g_pData->GetMyUser()->GetSelectUnit();

	if ( NULL != pUnit )
	{
		WCHAR buf[255] = {0,};

		/// �ʻ�ȭ UI ���� ( �ϳ��� ������ 5��, ���� ���� ����Ͽ� ������ �ʻ�ȭ UI ǥ�� ���� ����  )
		CX2Unit::UNIT_CLASS_TYPE eUnitClassLevel = static_cast<CX2Unit::UNIT_CLASS_TYPE>( pUnit->GetClassLevel() );


		/// ��ų ���м�( ���� ) ���� ---------------------------------------------------------------------------------------------------------
		CKTDGUIStatic* pStatic_HorizonLine1 = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_HorizonLine1" ) );
		CKTDGUIStatic* pStatic_HorizonLine2 = static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_HorizonLine2" ) );

		if ( NULL != pStatic_HorizonLine1 && NULL != pStatic_HorizonLine2 )
		{
			switch ( eUnitClassLevel )
			{
			case CX2Unit::UCT_FIRST_CLASS:	/// ������ -> 1�� ����
				{
					pStatic_HorizonLine1->SetShow(true);
					pStatic_HorizonLine2->SetShow(false);

					pControlList->InsertItem( pStatic_HorizonLine1, MAX_ONE_CLASS_SKILL_ROW_NUM - 1 , 0 );			/// ������ ��ų ������ �ٿ� ���м� �߰�
				} break;
			case CX2Unit::UCT_SECOND_CLASS: /// ������ -> 1�� ���� -> 2�� ����
				{
					pStatic_HorizonLine1->SetShow(true);
					pStatic_HorizonLine2->SetShow(true);

					pControlList->InsertItem( pStatic_HorizonLine1, MAX_ONE_CLASS_SKILL_ROW_NUM - 1 , 0 );			/// ������ ��ų ������ �ٿ� ���м� �߰�
					pControlList->InsertItem( pStatic_HorizonLine2, ( MAX_ONE_CLASS_SKILL_ROW_NUM * 2 ) - 1, 0 );	/// 1�� ���� ��ų ������ �ٿ� ���м� �߰�
				} break;
			default: /// ������
				{
					pStatic_HorizonLine1->SetShow(false);	/// ���м� ����
					pStatic_HorizonLine2->SetShow(false);
				} break;
			}
		}
	}
}

/** @function	: UpdateAtMoveScrollBar
	@brief		: ��ũ�� �ٰ� ������ �� ����Ǿ�� �� ������ ó���ϴ� �Լ�
*/
void CX2UISkillTreeNew::UpdateAtMoveScrollBar()
{
	if (	NULL == m_pDLGUISkillTree ||
			NULL == g_pData ||
			NULL == g_pData->GetMyUser() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit() )
		{
			return;
		}

	/// ������ ��ų ���м��� ��ų â �Ʒ��� ���� ��, ���������� �����ֱ�
	CKTDGUIContolList*	pControlList			= static_cast<CKTDGUIContolList*>( m_pDLGUISkillTree->GetControl( L"SkillTreeControlList" ) );
	CKTDGUIStatic*		pStatic_HorizonLine1	= static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_HorizonLine1" ) );
	CKTDGUIStatic*		pStatic_HorizonLine2	= static_cast<CKTDGUIStatic*>( m_pDLGUISkillTree->GetControl( L"g_pStatic_HorizonLine2" ) );
	CX2Unit*			pUnit					= g_pData->GetMyUser()->GetSelectUnit();

	if ( NULL != pUnit && NULL != pControlList && NULL != pStatic_HorizonLine1 && NULL != pStatic_HorizonLine2 )
	{
		CX2Unit::UNIT_CLASS_TYPE	eUnitClassType	= static_cast<CX2Unit::UNIT_CLASS_TYPE >( pUnit->GetClassLevel() );	/// ���� ���� ���
		int							iTopTier		= pControlList->GetIndexY();	/// ���� ��ũ�� �ε���

		{
			if ( CX2Unit::UCT_FIRST_CLASS <= eUnitClassType )	/// 1�� ������ �� ( �⺻ ���� -> 1�� ���� ��ų ���м��� ������ �� )
			{
				if ( 0 == iTopTier )		/// ���� ù��° ��ų ���м��� ��ųâ ���� �ؿ� ��ġ�Ǿ� ���� ��, ����
					pStatic_HorizonLine1->SetShow( false );
				else
					pStatic_HorizonLine1->SetShow( true );
			}

			if ( CX2Unit::UCT_SECOND_CLASS <= eUnitClassType )	/// 2�� ������ �� ( 1�� ���� -> 2�� ���� ��ų ���м��� ������ �� )
			{
				if ( 5 == iTopTier )		/// ���� �ι�° ��ų ���м��� ��ųâ ���� �ؿ� ��ġ�Ǿ� ���� ��, ����
					pStatic_HorizonLine2->SetShow( false );
				else
					pStatic_HorizonLine2->SetShow( true );
			}
		}
	}
}
/** @function	: SetUnitClassImageAndName
	@brief		: ��ų Ʈ�� �ʻ�ȭ�� ���� �̹��� �� ������ ���� �Լ�
	@Param		: ������ �ʻ�ȭ ��Ʈ��, ���� ���� Ŭ����
*/
void CX2UISkillTreeNew::SetUnitClassImageAndName(	CKTDGUIStatic* pStatic,
													CX2Unit::UNIT_CLASS eUnitClass /*= CX2Unit::UC_ARME_VOID_PRINCESS*/,
													CX2Unit::UNIT_CLASS_TYPE UnitClassType /*= CX2Unit::UCT_BASIC_CLASS*/ )
{
	if ( NULL == pStatic )
		return;

	wstring wstrFileName	= L"";
	wstring wstrPieceName	= L"";

	switch ( eUnitClass )
	{
		/// ���ҵ�
	case CX2Unit::UC_ELSWORD_SWORDMAN:			/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25010 );
				eUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;
			}
		} break;
	case CX2Unit::UC_ELSWORD_KNIGHT:			/// 1�� ����
	case CX2Unit::UC_ELSWORD_LORD_KNIGHT:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25010 );
					eUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25011 );
					eUnitClass = CX2Unit::UC_ELSWORD_KNIGHT;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25012 );
					eUnitClass = CX2Unit::UC_ELSWORD_LORD_KNIGHT;
				} break;
			}
		} break;
	case CX2Unit::UC_ELSWORD_MAGIC_KNIGHT:		/// 1�� ����
	case CX2Unit::UC_ELSWORD_RUNE_SLAYER:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25010 );
					eUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25013 );
					eUnitClass = CX2Unit::UC_ELSWORD_MAGIC_KNIGHT;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25014 );
					eUnitClass = CX2Unit::UC_ELSWORD_RUNE_SLAYER;
				} break;
			}
		} break;
	case CX2Unit::UC_ELSWORD_SHEATH_KNIGHT:		/// 1�� ����
	case CX2Unit::UC_ELSWORD_INFINITY_SWORD:	/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25010 );
					eUnitClass = CX2Unit::UC_ELSWORD_SWORDMAN;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25015 );
					eUnitClass = CX2Unit::UC_ELSWORD_SHEATH_KNIGHT;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25016 );
					eUnitClass = CX2Unit::UC_ELSWORD_INFINITY_SWORD;
				} break;
			}
		} break;

		/// ���̻�
	case CX2Unit::UC_ARME_VIOLET_MAGE:			/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25017 );
				eUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
			}
		} break;
	case CX2Unit::UC_ARME_HIGH_MAGICIAN:		/// 1�� ����
	case CX2Unit::UC_ARME_ELEMENTAL_MASTER:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25017 );
					eUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25018 );
					eUnitClass = CX2Unit::UC_ARME_HIGH_MAGICIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25019 );
					eUnitClass = CX2Unit::UC_ARME_ELEMENTAL_MASTER;
				} break;
			}
		} break;
	case CX2Unit::UC_ARME_DARK_MAGICIAN:		/// 1�� ����
	case CX2Unit::UC_ARME_VOID_PRINCESS:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25017 );
					eUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25020 );
					eUnitClass = CX2Unit::UC_ARME_DARK_MAGICIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25021 );
					eUnitClass = CX2Unit::UC_ARME_VOID_PRINCESS;
				} break;
			}
		} break;
	case CX2Unit::UC_ARME_BATTLE_MAGICIAN:		/// 1�� ����
	case CX2Unit::UC_ARME_DIMENSION_WITCH:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25017 );
					eUnitClass = CX2Unit::UC_ARME_VIOLET_MAGE;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25022 );
					eUnitClass = CX2Unit::UC_ARME_BATTLE_MAGICIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25023 );
					eUnitClass = CX2Unit::UC_ARME_DIMENSION_WITCH;
				} break;
			}
		} break;

		/// ����
	case CX2Unit::UC_LIRE_ELVEN_RANGER:			/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25024 );
				eUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
			}
		} break;
	case CX2Unit::UC_LIRE_COMBAT_RANGER:		/// 1�� ����
	case CX2Unit::UC_LIRE_WIND_SNEAKER:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25024 );
					eUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25025 );
					eUnitClass = CX2Unit::UC_LIRE_COMBAT_RANGER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25026 );
					eUnitClass = CX2Unit::UC_LIRE_WIND_SNEAKER;
				} break;
			}
		} break;
	case CX2Unit::UC_LIRE_SNIPING_RANGER:		/// 1�� ����
	case CX2Unit::UC_LIRE_GRAND_ARCHER:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25024 );
					eUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25027 );
					eUnitClass = CX2Unit::UC_LIRE_SNIPING_RANGER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25028 );
					eUnitClass = CX2Unit::UC_LIRE_GRAND_ARCHER;
				} break;
			}
		} break;
	case CX2Unit::UC_LIRE_TRAPPING_RANGER:		/// 1�� ����
	case CX2Unit::UC_LIRE_NIGHT_WATCHER:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25024 );
					eUnitClass = CX2Unit::UC_LIRE_ELVEN_RANGER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25029 );
					eUnitClass = CX2Unit::UC_LIRE_TRAPPING_RANGER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25030 );
					eUnitClass = CX2Unit::UC_LIRE_NIGHT_WATCHER;
				} break;
			}
		} break;

		/// ���̺�
	case CX2Unit::UC_RAVEN_FIGHTER:				/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25031 );
				eUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
			}
		} break;
	case CX2Unit::UC_RAVEN_SOUL_TAKER:			/// 1�� ����
	case CX2Unit::UC_RAVEN_BLADE_MASTER:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25031 );
					eUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25032 );
					eUnitClass = CX2Unit::UC_RAVEN_SOUL_TAKER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25033 );
					eUnitClass = CX2Unit::UC_RAVEN_BLADE_MASTER;
				} break;
			}
		} break;
	case CX2Unit::UC_RAVEN_OVER_TAKER:			/// 1�� ����
	case CX2Unit::UC_RAVEN_RECKLESS_FIST:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25031 );
					eUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25034 );
					eUnitClass = CX2Unit::UC_RAVEN_OVER_TAKER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25035 );
					eUnitClass = CX2Unit::UC_RAVEN_RECKLESS_FIST;
				} break;
			}
		} break;
	case CX2Unit::UC_RAVEN_WEAPON_TAKER:		/// 1�� ����
	case CX2Unit::UC_RAVEN_VETERAN_COMMANDER:	/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25031 );
					eUnitClass = CX2Unit::UC_RAVEN_FIGHTER;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25036 );
					eUnitClass = CX2Unit::UC_RAVEN_WEAPON_TAKER;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25037 );
					eUnitClass = CX2Unit::UC_RAVEN_VETERAN_COMMANDER;
				} break;
			}
		} break;

		/// �̺�
	case CX2Unit::UC_EVE_NASOD:					/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS == UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25038 );
				eUnitClass = CX2Unit::UC_EVE_NASOD;
			}
		} break;
	case CX2Unit::UC_EVE_EXOTIC_GEAR:			/// 1�� ����
	case CX2Unit::UC_EVE_CODE_NEMESIS:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25038 );
					eUnitClass = CX2Unit::UC_EVE_NASOD;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25039 );
					eUnitClass = CX2Unit::UC_EVE_EXOTIC_GEAR;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25040 );
					eUnitClass = CX2Unit::UC_EVE_CODE_NEMESIS;
				} break;
			}
		} break;
	case CX2Unit::UC_EVE_ARCHITECTURE:			/// 1�� ����
	case CX2Unit::UC_EVE_CODE_EMPRESS:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25038 );
					eUnitClass = CX2Unit::UC_EVE_NASOD;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25041 );
					eUnitClass = CX2Unit::UC_EVE_ARCHITECTURE;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25042 );
					eUnitClass = CX2Unit::UC_EVE_CODE_EMPRESS;
				} break;
			}
		} break;
	case CX2Unit::UC_EVE_ELECTRA:				/// 1�� ����
	case CX2Unit::UC_EVE_BATTLE_SERAPH:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25038 );
					eUnitClass = CX2Unit::UC_EVE_NASOD;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25043 );
					eUnitClass = CX2Unit::UC_EVE_ELECTRA;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25044 );
					eUnitClass = CX2Unit::UC_EVE_BATTLE_SERAPH;
				} break;
			}
		} break;

		/// û
	case CX2Unit::UC_CHUNG_IRON_CANNON:			/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25045 );
				eUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
			}
		} break;
	case CX2Unit::UC_CHUNG_FURY_GUARDIAN:		/// 1�� ����
	case CX2Unit::UC_CHUNG_IRON_PALADIN:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25045 );
					eUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25046 );
					eUnitClass = CX2Unit::UC_CHUNG_FURY_GUARDIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25047 );
					eUnitClass = CX2Unit::UC_CHUNG_IRON_PALADIN;
				} break;
			}
		} break;
	case CX2Unit::UC_CHUNG_SHOOTING_GUARDIAN:	/// 1�� ����
	case CX2Unit::UC_CHUNG_DEADLY_CHASER:		/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25045 );
					eUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25048 );
					eUnitClass = CX2Unit::UC_CHUNG_SHOOTING_GUARDIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25049 );
					eUnitClass = CX2Unit::UC_CHUNG_DEADLY_CHASER;
				} break;
			}
		} break;
	case CX2Unit::UC_CHUNG_SHELLING_GUARDIAN:	/// 1�� ����
	case CX2Unit::UC_CHUNG_TACTICAL_TROOPER:	/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25045 );
					eUnitClass = CX2Unit::UC_CHUNG_IRON_CANNON;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25050 );
					eUnitClass = CX2Unit::UC_CHUNG_SHELLING_GUARDIAN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25051 );
					eUnitClass = CX2Unit::UC_CHUNG_TACTICAL_TROOPER;
				} break;
			}
		} break;
		/// �ƶ�
	case CX2Unit::UC_ARA_MARTIAL_ARTIST:		/// �⺻ ����
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25052 );
				eUnitClass = CX2Unit::UC_ARA_MARTIAL_ARTIST;
			}
		} break;
	case CX2Unit::UC_ARA_LITTLE_HSIEN:			/// 1�� ����
	case CX2Unit::UC_ARA_SAKRA_DEVANAM:			/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25052 );
					eUnitClass = CX2Unit::UC_ARA_MARTIAL_ARTIST;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25053 );
					eUnitClass = CX2Unit::UC_ARA_LITTLE_HSIEN;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25054 );
					eUnitClass = CX2Unit::UC_ARA_SAKRA_DEVANAM;
				} break;
			}
		} break;
	case CX2Unit::UC_ELESIS_KNIGHT:
		{
			if ( CX2Unit::UCT_BASIC_CLASS ==  UnitClassType )	/// �⺻ ���� ���� ����
			{
				pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25873 );
				eUnitClass = CX2Unit::UC_ELESIS_KNIGHT;
			}
		} break;
	case CX2Unit::UC_ELESIS_SABER_KNIGHT:
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25873 );
					eUnitClass = CX2Unit::UC_ELESIS_KNIGHT;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_26114 );
					eUnitClass = CX2Unit::UC_ELESIS_SABER_KNIGHT;
				} break;
			}
		} break;
	case CX2Unit::UC_ELESIS_PYRO_KNIGHT:
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25873 );
					eUnitClass = CX2Unit::UC_ELESIS_KNIGHT;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_26115 );
					eUnitClass = CX2Unit::UC_ELESIS_PYRO_KNIGHT;
				} break;
			}
		} break;
#ifdef SERV_ARA_CHANGE_CLASS_SECOND // ����ȯ
	case CX2Unit::UC_ARA_LITTLE_DEVIL:			/// 1�� ����
	case CX2Unit::UC_ARA_YAMA_RAJA:					/// 2�� ����
		{
			switch ( UnitClassType )					/// ���ϴ� ���� ��޿� ���� ���� ����
			{
			case CX2Unit::UCT_BASIC_CLASS:	/// �⺻ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25052 );
					eUnitClass = CX2Unit::UC_ARA_MARTIAL_ARTIST;
				} break;
			case CX2Unit::UCT_FIRST_CLASS:	/// 1�� ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_26771 );
					eUnitClass = CX2Unit::UC_ARA_LITTLE_DEVIL;
				} break;
			case CX2Unit::UCT_SECOND_CLASS:	/// 2ġ ���� ���� ����
				{
					pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_26772 );
					eUnitClass = CX2Unit::UC_ARA_YAMA_RAJA;
				} break;
			}
		} break;
#endif // SERV_ARA_CHANGE_CLASS_SECOND
		/// ���� Ȥ�� �ɸ��� �߰���, �ش� case�� �߰��ؾ� �մϴ�!
		/// ���� : 
		/// �⺻ ����						- �⺻ ������ �߰�
		/// 1�� Ȥ�� 2�� ���� ( ��Ʈ 1 )	- �⺻ ���� ~ 2�� �������� �߰� ( �ش� �������� ���� ������ ������ �޾ƿ� �� �ֵ��� )
		/// 1�� Ȥ�� 2�� ���� ( ��Ʈ 2 )	- �⺻ ���� ~ 2�� �������� �߰�
		/// 1�� Ȥ�� 2�� ���� ( ��Ʈ 3 )	- �⺻ ���� ~ 2�� �������� �߰�
	}

	/// �ʻ�ȭ �̹��� Ű �� ����
	if( false == CX2Data::GetCharacterImageName( wstrFileName, wstrPieceName, eUnitClass, CX2Data::CIT_50by50 ) )
	{
		/// Unit Class ���ڰ��� �߸� �����Ͽ���!
		ASSERT( !"Skill Tree Portrait Unit Class Is Wrong!!!" );

		/// ���̻� ���̵� ���������� ����Ʈ�� ���� ( �� �ǹ̴� ����. )
		wstrFileName	= L"DLG_UI_Common_Texture21_NEW.tga";
		wstrPieceName	= L"V_PRINCESS_SMALL";
	}

	pStatic->GetPicture( 1 )->SetTex( wstrFileName.c_str(),	wstrPieceName.c_str() );	/// �ʻ�ȭ ����

	/// Unit Class Type ���� ó��
	if ( L"None" == pStatic->GetString( 0 )->msg )
	{
		/// Unit Class Type ���ڰ��� �߸� �����Ͽ���!
		ASSERT( !"Skill Tree Portrait Unit Class Type Is Wrong!!!" );

		pStatic->GetString( 0 )->msg = GET_STRING( STR_ID_25021 );
	}

	
}

/** @function	: GetSkillTitleDesc
	@brief		: ��ų �̸�, ��ų ������ ǥ���ϴ� ���� ���� ����
	@param		: ��ų ���̵�
	@return		: ��ų ���ø�, ��Ʈ�� ������
*/
void CX2UISkillTreeNew::GetSkillTitleDesc( OUT wstring& wstrTitleDesc, IN const CX2SkillTree::SkillTemplet* pSkillTemplet, 
										   IN const CX2SkillTree::SkillTreeTemplet* pSkillTreeTemplet, IN int iSkillLevel, IN int iIncreasekillLevel ) const
{
	if( NULL == pSkillTemplet ||
		NULL == pSkillTreeTemplet ||
		NULL == g_pData ||
		NULL == g_pData->GetMyUser() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit() ||
		NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() ||
		NULL == g_pData->GetSocketItem() ||
		NULL == g_pData->GetSkillTree() )
		return;

#ifdef CLIENT_GLOBAL_LINEBREAK
	wstrTitleDesc = CWordLineHandler::GetStrByLineBreakInX2Main( pSkillTemplet->m_wstrName.c_str(), 150, XUF_DODUM_11_NORMAL );
#else //CLIENT_GLOBAL_LINEBREAK
	wstrTitleDesc = pSkillTemplet->m_wstrName;		/// ��ų��
#endif //CLIENT_GLOBAL_LINEBREAK

	wstrTitleDesc += L"\n";

	/// ��ų ����
	wstring wstrLevelTitle = L"";

	/// ���� ������ ������ ��, ���� �ڿ� (������) �߰�
	if ( pSkillTreeTemplet->m_iMaxSkillLevel <= iSkillLevel )
		wstrLevelTitle = GET_STRING(STR_ID_25113);

	WCHAR buf[256] = {0,};

	/// ���� ��ų ������ 0�� �ƴϰ�, �߰� ��ų ������ ������
	if ( 0 < iSkillLevel && 0 < iIncreasekillLevel )
		StringCchPrintf( buf, 256, L"#C0DBE26%s.%d (+%d)#CX #CFF0000%s#CX\n", GET_STRING(STR_ID_489), iSkillLevel, iIncreasekillLevel, wstrLevelTitle.c_str() );
	else
		StringCchPrintf( buf, 256, L"%s.%d #CFF0000%s#CX\n", GET_STRING(STR_ID_489), iSkillLevel, wstrLevelTitle.c_str() );

	wstrTitleDesc += buf;
}

/** @function	: GetSkillSubDesc
	@brief		: ��ų Ÿ��, �Ҹ� MP, ���� �ð��� ǥ���ϴ� ���� ���� ����
	@param		: ��ų ���ø�, �� ��ų ����, ��Ʈ�� ������
*/
void  CX2UISkillTreeNew::GetSkillSubDesc( IN const CX2SkillTree::SkillTemplet* pSkillTemplet, IN const int iTotalSkillLevel, OUT wstring& wstrSubDesc ) const
{
	if( NULL == pSkillTemplet )
		return;

	bool bIsPassive = false;	/// �нú� ��ų ���� ( �нú� ��ų�� ���� �ð� 0�϶�, ��ǥ�� )

	/// ��ų Ÿ��
	WCHAR buf[256] = {0,};

	switch(pSkillTemplet->m_eType)
	{	
	case CX2SkillTree::ST_SPECIAL_ACTIVE:
		{
			StringCchPrintf( buf, 256, L"[%s]", GET_STRING(STR_ID_2672) );		/// ����� ��Ƽ��

		} break;
	case CX2SkillTree::ST_ACTIVE:
#ifdef ADDED_RELATIONSHIP_SYSTEM
	case CX2SkillTree::ST_RELATIONSHIP_SKILL:
#endif // ADDED_RELATIONSHIP_SYSTEM
		{
			StringCchPrintf( buf, 256, L"[%s]", GET_STRING(STR_ID_2673) );		/// ��Ƽ��

		} break;
	case CX2SkillTree::ST_BUFF:
		{
			StringCchPrintf( buf, 256, L"[%s]", GET_STRING(STR_ID_2674) );		/// ����

		} break;
	default:
		{
			if ( true == m_bReplacedEsCounter )
			{
				StringCchPrintf( buf, 256, L"[%s]", GET_STRING(STR_ID_2673) );		/// ��Ƽ��
			}
			else
			{
				StringCchPrintf( buf, 256, L"[%s]", GET_STRING(STR_ID_2675) );		/// �нú�
				bIsPassive = true;		/// �нú� ��ų��
			}
		} break;
	}
	wstrSubDesc = buf;	/// ��ų Ÿ�� ����

	/// �Ҹ� MP
	const int iMPConsumption = static_cast<int>( pSkillTemplet->GetSkillMPConsumptionValue( iTotalSkillLevel ) );	/// �Ҹ� MP ��

	/// �нú� ��ų�̰� �Ҹ� MP�� �ְų�, �нú� ��ų�� �ƴ� �� �Ҹ� MP ǥ��
	if ( ( true == bIsPassive && 0 < iMPConsumption ) || false == bIsPassive )
	{
		StringCchPrintf( buf, 256, L"\n%s%d", GET_STRING(STR_ID_2648), static_cast<int>( pSkillTemplet->GetSkillMPConsumptionValue( iTotalSkillLevel ) ) );
		wstrSubDesc += buf;
	}

	/// ���� �ð�
	const int iSkillCoolTime = static_cast<int>( pSkillTemplet->GetSkillCoolTimeValue( iTotalSkillLevel ) );		/// ���� �ð� ��

	/// �нú� ��ų�̰� ���� �ð��� �ְų�, �нú� ��ų�� �ƴ� �� ���� �ð� ǥ��
	if ( ( true == bIsPassive && 0 < iSkillCoolTime ) || false == bIsPassive )
	{
		wstrSubDesc += L"\n";
		wstrSubDesc += GET_REPLACED_STRING( ( STR_ID_25070, "i", static_cast<int>( pSkillTemplet->GetSkillCoolTimeValue( iTotalSkillLevel ) ) ) );	/// ���� �ð� @1
	}
}

/** @function	: SetLearnSkillInfo
	@brief		: ��ųâ���� ��ų �����ϰų� ��ȭ�� ��, ���氪 �����ϴ� �Լ�
	@param		: ��ų ���̵�
*/
void CX2UISkillTreeNew::SetLearnSkillInfo( CX2SkillTree::SKILL_ID eSkillID )
{
	if (	NULL == g_pData ||
			NULL == g_pData->GetSkillTree() )
	{
		return;
	}

	g_pData->GetSkillTree()->FixOrInsertMapSkillInfoValue( eSkillID );

	/// ��ų ���� ������Ʈ ó��
	UpdateUI( false, true, true );
}

/** @function	: SetBlindSelectSkill
	@brief		: 2������ ��ų �� ���õ��� ���� ��ų�� ���� �����ε� ����
*/
void CX2UISkillTreeNew::SetBlindSelectSkill()
{
	if (	NULL == m_pDLGUISkillTree ||
			NULL == g_pData ||
			NULL == g_pData->GetMyUser() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		return;
	}
	
	for ( int iTier = 0; iTier < m_iMaxSkillRowNum + 1; ++iTier )
	{
		/// 2������ ������ �ִ� ��ų ���̵� �˻�
		std::map<CX2UISkillTreeNew::SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator itRight
			= m_mapSkillPosition.find( CX2UISkillTreeNew::SkillPositionKey( 1, iTier ) );

		if ( itRight != m_mapSkillPosition.end() )	/// 2������ ������ ��ų�� ������ ��
		{
			const CX2UserSkillTree&	refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;	/// ������ ��ų ��ü

			int						iSkillLevelRight = refUserSkillTree.GetSkillLevel( itRight->second );		/// 2������ ������ ��ų ���� ( ������ �����Ǿ� �ִ� ���� )

			if ( true == m_bNowLearnSkill )		/// ���� ��ųâ���� ��ų ������ �ø��� ������, �ش� ��ų�� ������ �ø����� �ִٸ� ��ųâ ������ �ö� ������ ����
			{
				const std::map<int, KGetSkillInfo> mapSkillInfo = g_pData->GetSkillTree()->GetMapSkillInfo();

				std::map<int, KGetSkillInfo>::const_iterator it = mapSkillInfo.find( static_cast<int>( itRight->second ) );

				if ( it != mapSkillInfo.end() )
					iSkillLevelRight = it->second.m_iSkillLevel;
			}

			std::map<CX2UISkillTreeNew::SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator itLeft		/// 2������ ������ ��ų ���̵� �˻�
				= m_mapSkillPosition.find( CX2UISkillTreeNew::SkillPositionKey( 0, iTier ) );

			/// 2������ ��, ���� ���� UI ��ü �˻�
			std::map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator itLeftUISet		= m_mapSkillSlotUISet.find( itLeft->second );
			std::map<CX2SkillTree::SKILL_ID, SkillSlotUI>::iterator itRightUISet	= m_mapSkillSlotUISet.find( itRight->second );

			if ( itLeft == m_mapSkillPosition.end() || itLeftUISet == m_mapSkillSlotUISet.end() || itRightUISet == m_mapSkillSlotUISet.end() )
			{
				ASSERT( !"Skill Tree Slot Information Is Error!!!" );	/// ��ų ���� �� ������ ���� �̻��ϴ�!!!
				return;
			}

#ifdef SERV_IRUHADEV_SKILLTREE_NO_LOCK
			itLeftUISet->second.m_bSelectSkillBlind	= false;	/// 2������ ���� ��ų �����ε� ����
			itRightUISet->second.m_bSelectSkillBlind	= false;	/// 2������ ���� ��ų �����ε� ����

			continue;
#else
#if defined( _IN_HOUSE_ ) || defined( _IN_HOUSE_SERVICE_READY_QA_ ) || defined( _OPEN_TEST_ ) || defined( _OPEN_TEST_2_ )
			/// ������ ��� - ���� ��� ��ų�� �����ִٸ�, �����ε� ����
			if( true == g_pMain->IsMyAuthLevelHigherThan( CX2User::XUAL_DEV ) )
			{
				int iSkillLevelLeft = refUserSkillTree.GetSkillLevel( itLeft->second );		/// 2������ ���� ��ų ���� ( ������ �����Ǿ� �ִ� ���� )

				if ( 0 < iSkillLevelLeft && 0 < iSkillLevelRight )
				{
					itLeftUISet->second.m_bSelectSkillBlind		= false;	/// 2������ ���� ��ų �����ε� ����
					itRightUISet->second.m_bSelectSkillBlind	= false;	/// 2������ ���� ��ų �����ε� ����

					continue;
				}
			}
#endif // _SERVICE_

			if ( 0 < iSkillLevelRight )		/// 2������ ������ ��ų�� ���ڵǾ� �ִٸ�,
			{
				itLeftUISet->second.m_bSelectSkillBlind		= true;		/// 2������ ���� ��ų �����ε� ó��
				itRightUISet->second.m_bSelectSkillBlind	= false;	/// 2������ ���� ��ų �����ε� ����
			}
			else	/// 2������ ������ ��ų�� ���ڵǾ� ���� �ʴٸ�,
			{
				int iSkillLevelLeft = refUserSkillTree.GetSkillLevel( itLeft->second );		/// 2������ ���� ��ų ���� ( ������ �����Ǿ� �ִ� ���� )

				if ( true == m_bNowLearnSkill )		/// ���� ��ųâ���� ��ų ������ �ø��� ������, �ش� ��ų�� ������ �ø����� �ִٸ� ��ųâ ������ �ö� ������ ����
				{
					const std::map<int, KGetSkillInfo> mapSkillInfo = g_pData->GetSkillTree()->GetMapSkillInfo();

					std::map<int, KGetSkillInfo>::const_iterator it = mapSkillInfo.find( static_cast<int>( itLeft->second ) );

					if ( it != mapSkillInfo.end() )
						iSkillLevelLeft = it->second.m_iSkillLevel;
				}

				if( 0 < iSkillLevelLeft )		/// 2������ ������ ��ų�� ���ڵǾ� �ִٸ�,
				{
					itLeftUISet->second.m_bSelectSkillBlind		= false;	/// 2������ ���� ��ų �����ε� ����
					itRightUISet->second.m_bSelectSkillBlind	= true;		/// 2������ ���� ��ų �����ε� ó��
				}
				else							/// 2������ ��ų ��� ���ڵ� �� ���ٸ�,
				{
					itLeftUISet->second.m_bSelectSkillBlind		= false;	/// 2������ ���� ��ų �����ε� ����
					itRightUISet->second.m_bSelectSkillBlind	= false;	/// 2������ ���� ��ų �����ε� ����
				}
			}
#endif SERV_IRUHADEV_SKILLTREE_NO_LOCK
		}
	}
}

/** @function	: GetAnotherSelectSkillID
	@brief		: 2������ ��ų �� �ݴ����� �ִ� ��ų ���̵� ��ȯ
	#param		: �˻��� ��ų ���̵�
	@return		: �ݴ����� �ִ� ��ų ���̵�
*/
const CX2SkillTree::SKILL_ID CX2UISkillTreeNew::GetAnotherSelectSkillID( CX2SkillTree::SKILL_ID eCheckSkillID ) const
{
	if (	NULL == g_pData ||
			NULL == g_pData->GetSkillTree() ||
			NULL == g_pData->GetMyUser() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		return CX2SkillTree::SI_NONE;
	}

	int										iUnitClass			= static_cast<int>( g_pData->GetMyUser()->GetSelectUnit()->GetClass() );		/// ���� Ŭ����
	const CX2SkillTree::SkillTreeTemplet*	pSkillTreeTemplet	= g_pData->GetSkillTree()->GetSkillTreeTemplet( iUnitClass, eCheckSkillID );	/// ��ų Ʈ�� ���ø�

	if ( NULL != pSkillTreeTemplet )
	{
		int iIndexInTier	= pSkillTreeTemplet->m_iIndexInTier;	/// ���� �ε���

		if ( STST_SINGLE_SKILL == iIndexInTier )	/// 2������ ��ų�� �ƴ϶��, ����
			return CX2SkillTree::SI_NONE;

		int iTier			= pSkillTreeTemplet->m_iTier;			/// ���� �ε���

		/// �ݴ����� ��ų ���̵� �˻�
		std::map<CX2UISkillTreeNew::SkillPositionKey, CX2SkillTree::SKILL_ID>::const_iterator it
									= m_mapSkillPosition.find( CX2UISkillTreeNew::SkillPositionKey( 1 == iIndexInTier ? 0 : 1, iTier ) );

		/// �ݴ����� ��ų ���̵� �ִٸ�, �ش� ��ų ���̵� ��ȯ
		if ( it != m_mapSkillPosition.end() )
			return it->second;
	}

	return CX2SkillTree::SI_NONE;
}

/** @function	: GetSkillLevelInSkillWimdow
	@brief		: ��ųâ �� ��ų ���̵� ��ȯ �Լ� ( ���� �������� ��ų�̶�� ������ ����, �������� �ƴ����� ��ųâ���� �ӽ÷� �� ��ų�̶�� ���� ���� )
	#param		: �˻��� ��ų ���̵�
	@return		: ��ų ����
*/
const int CX2UISkillTreeNew::GetSkillLevelInSkillWimdow( const CX2SkillTree::SKILL_ID eSkillID ) const
{
	if (	NULL == g_pData ||
			NULL == g_pData->GetMyUser() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit() ||
			NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
	{
		return 0;
	}

	const CX2UserSkillTree&	refUserSkillTree = g_pData->GetMyUser()->GetSelectUnit()->GetUnitData()->m_UserSkillTree;	/// ������ ��ų ��ü

	int iSkillLevel = refUserSkillTree.GetSkillLevel( eSkillID, true );		/// ��ų ���� ( ������ �����Ǿ� �ִ� ���� )

	if ( true == m_bNowLearnSkill )		/// ���� ��ųâ���� ��ų ������ �ø��� ������, �ش� ��ų�� ������ �ø����� �ִٸ� ��ųâ ������ �ö� ������ ����
	{
		const std::map<int, KGetSkillInfo> mapSkillInfo = g_pData->GetSkillTree()->GetMapSkillInfo();

		std::map<int, KGetSkillInfo>::const_iterator it = mapSkillInfo.find( static_cast<int>( eSkillID ) );

		if ( it != mapSkillInfo.end() )
			iSkillLevel = it->second.m_iSkillLevel;
	}

	return iSkillLevel;
}

/** @function	: GetPrecedingSkillDesc
	@brief		: ���� ��ų�� ���� ���� ����
	#param		: ���� ��Ʈ��, ���� ��ų ���̵�
*/
void CX2UISkillTreeNew::GetPrecedingSkillDesc( OUT wstring& wstrDesc, CX2SkillTree::SKILL_ID ePrecedingSkill ) const
{
	if ( NULL == g_pData ||
		 NULL == g_pData->GetSkillTree() )
	{
		return;
	}

	/// ���� ��ų�� ����
	const int iPrecedingSkillLevel = GetSkillLevelInSkillWimdow( ePrecedingSkill );

	if ( 0 >= iPrecedingSkillLevel )		/// ���� ��ų�� ����� �ʾ��� ����, ���� ��ų �ȳ� �˾� ����
	{
		const CX2SkillTree::SkillTemplet* pPrecedingSkillTemplet = g_pData->GetSkillTree()->GetSkillTemplet( ePrecedingSkill );

		if ( NULL != pPrecedingSkillTemplet )
		{
			WCHAR buf[256] = {0,};
			StringCchPrintf( buf, 256, L"\n#CFF0000%s#CX\n\n", GET_REPLACED_STRING( ( STR_ID_25114, "L", pPrecedingSkillTemplet->m_wstrName ) ) );	/// ���ེų : @1 Lv.1 �ʿ�
			wstrDesc += buf;
		}
	}
}

/** @function	: SetSkillSlotButton
	@brief		: ��ų Ʈ���� ��ų ���� ��ư ���� �Լ�
	#param		: ��ư ��ü, ��ư Ÿ��
*/
void CX2UISkillTreeNew::SetSkillSlotButton( CKTDGUIButton* pSlotButton, SKILL_TREE_SLOT_BUTTON_TYPE eButtonType )
{
	if ( NULL == pSlotButton )
		return;

	switch ( eButtonType )
	{
	case STSBT_LEARN_SKILL:
		{

		} break;
	case STSBT_UPGRADE_SKILL:
		{

		} break;
	case STSBT_CAN_NOT_LEARN_SKILL:
		{

		} break;
	case STSBT_INIT_SKILL:
		{

		} break;
	default:
		{
			ASSERT( !"Button Type is Error" );
		} break;
	}
}

/** @function	: IsLearnBeforeSelectSkill
	@brief		: ���� �ܰ��� 2������ ��ų�� ������� ���θ� ��ȯ
	@param		: ���� ��ų ������ Tier �ε���
	@return		: ���� �ܰ� 2������ ��ų ���� ����
*/
bool CX2UISkillTreeNew::IsLearnBeforeSelectSkill( const int iNowTierIndex )
{
	/// �˻��Ϸ��� Tier �ε����� �⺻ ��ų�̶��, ������ ���� ó��
	if ( 0 == GetRequireSkillLineLevel( iNowTierIndex ) )
		return true;

	if ( NULL == g_pData ||
		 NULL == g_pData->GetMyUser() ||
		 NULL == g_pData->GetMyUser()->GetSelectUnit() || 
		 NULL == g_pData->GetMyUser()->GetSelectUnit()->GetUnitData() )
		 return true;

	const CX2Unit::UNIT_CLASS				eUnitClass			= g_pData->GetMyUser()->GetSelectUnit()->GetClass();

	/// ���� ���� ù��° ��( �⺻ ������ �⺻ ��ų )�̰ų� �ι�° ��( �⺻ ��ų�� �������� �� ��ų ) �̶��, ������ ��ų ���� ó��
	if ( 0 == iNowTierIndex || 1 == iNowTierIndex )
		return true;

	int BeforeSkillLineIndex = iNowTierIndex - 1;	/// ���� ��ų ������ ���� �ε���

	/// ���� ������ ��ų�� �⺻ ��ų�̶��, �⺻ ��ų ������ �ε����� �˻� ( 2�ܰ� ���� ��ų ���� )
	if ( 0 == GetRequireSkillLineLevel( BeforeSkillLineIndex ) )
		--BeforeSkillLineIndex;

	/// ���� �ܰ��� ��ų ���̵� �˻�
	const SkillPositionKey skillPositionKey	= SkillPositionKey( 0, BeforeSkillLineIndex );

	std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::const_iterator mit = m_mapSkillPosition.find( skillPositionKey );

	if ( m_mapSkillPosition.end() != mit )	/// ���� �ܰ��� ��ų ���̵� ���� ��
	{
		CX2SkillTree::SKILL_ID eSkillID = mit->second;

		/// �ش� ��ų�� ������ 1���� ũ��, ���� �Ȱ����� ó��
		if ( 0 < GetSkillLevelInSkillWimdow( eSkillID ) )
			return true;
		/// �ƴϸ�, 2������ �ݴ��� ���̵� �˻�
		else
		{
			const CX2SkillTree::SKILL_ID eAnotherSkillID = GetAnotherSelectSkillID( mit->second );

			/// �ݴ��� ��ų ���̵� ������, �ش� ��ų�� ������ 1���� ũ�� ���� �Ȱ����� ó��
			if ( CX2SkillTree::SI_NONE != eAnotherSkillID && 0 < GetSkillLevelInSkillWimdow( eAnotherSkillID ) )
				return true;
		}
	}

	return false;	/// ���� ������ ��ų�� ����� �ʾҴ�.
}

/** @function	: GetNotLearnTierIndex
	@brief		: ����� ���� �ֻ��� ��ų ���� ��ȯ
	@return		: ��ų ���� Tier �ε���
*/
const int CX2UISkillTreeNew::GetNotLearnTierIndex()
{
	std::map<SkillPositionKey, CX2SkillTree::SKILL_ID>::iterator mit = m_mapSkillPosition.find( SkillPositionKey( 0, 0 ) );
	int iTierIndex = 0;

	/// �� ���������� �Ʒ��� ���ʴ�� �˻��Ͽ�, ����� ��ų ������ ã�Ƴ��� �ش� Tier �ε����� ��ȯ��Ű��
	while ( m_mapSkillPosition.end() != mit )
	{
		const CX2SkillTree::SKILL_ID eLeftSkillID = mit->second;	/// 2������ ���� ��ų ���̵�

		if ( 0 != GetRequireSkillLineLevel( iTierIndex ) && 0 >= GetSkillLevelInSkillWimdow( eLeftSkillID ) )	/// �⺻ ��ų�� �ƴϰ�, ��ų�� ����� �ʾҴٸ�
		{
			const CX2SkillTree::SKILL_ID eRightSkillID = GetAnotherSelectSkillID( eLeftSkillID );				/// 2������ ������ ��ų ���̵� ��ȯ

			if ( CX2SkillTree::SI_NONE == eRightSkillID || 0 >= GetSkillLevelInSkillWimdow( eRightSkillID ) )	/// 2�� ���� ������ ��ų�� ���ų�, ����� �ʾҴ�
				break;		/// �ش� ��ų ������ �ʹ������, Tier �ε��� ��ȯ ����
		}

		++iTierIndex;

		mit = m_mapSkillPosition.find( SkillPositionKey( 0, iTierIndex ) );
	}

	return iTierIndex;
}

/** @function	: GetEnoughLevelSelectSkillTierIndex
	@brief		: ��� �� ���� ������ �ֻ��� ��ų ���� ��ȯ
	@return		: ��ų ���� Tier �ε���
*/
const int CX2UISkillTreeNew::GetNotEnoughLevelTierIndex( const int iLevel )
{
	int iTierIndex = 0;

	if ( NULL != m_pDLGUISkillTree )
	{
		CKTDGUIContolList*	pControlList = static_cast<CKTDGUIContolList*>( m_pDLGUISkillTree->GetControl( L"SkillTreeControlList" ) );

		if ( NULL != pControlList )
		{
			/// �� ���������� �Ʒ��� ���ʴ�� �˻��Ͽ�, ��� �� ���� ������ ��ų ������ ã�Ƴ��� �ش� Tier �ε����� ��ȯ��Ű��
			while ( pControlList->GetMaxIndexY() >= iTierIndex )
			{
				int iRequireSkillLevel = GetRequireSkillLineLevel( iTierIndex );

				if ( 0 != iRequireSkillLevel && iRequireSkillLevel > iLevel )	/// �⺻ ��ų�� �ƴϰ�, ��ų�� ����� �ʾҴٸ�
					break;		/// �ش� ��ų ������ �ʹ������, Tier �ε��� ��ȯ ����

				++iTierIndex;
			}
		}
	}

	return iTierIndex;
}

/** @function	: GetSkillUnSealedDesc
	@brief		: ���� ��ų ������ ���� �ȳ� ���� ����
	@param		: ��Ʈ�� ������, ��ų ���̵�
*/
void CX2UISkillTreeNew::GetSkillUnSealedDesc( OUT wstring& wstrDesc, IN const CX2SkillTree::SKILL_ID eSkillID ) const
{
	if ( NULL == g_pData ||
		 NULL == g_pData->GetSkillTree() ||
		 NULL == g_pData->GetItemManager() )
		 return;

	/// ���� ��ų ���ø� ��ȯ
	const std::map<int, CX2SkillTree::SealSkillItemInfo>& mapSealSkillItemTemplet = g_pData->GetSkillTree()->GetSealSkillItemTemplet();

	int iUnSealedItemID = 0;		/// ���� ��ų ������ ���̵�

	
	/// �ش� ��ų ���̵� ���� ���� ��ų �������� �˻�
	std::map<int, CX2SkillTree::SealSkillItemInfo>::const_iterator mit = mapSealSkillItemTemplet.begin();

	for ( mit; mit != mapSealSkillItemTemplet.end(); ++mit )
	{
		std::vector<CX2SkillTree::SKILL_ID> vecValue = mit->second.m_vecSkillID;

		BOOST_FOREACH( CX2SkillTree::SKILL_ID eValue, vecValue )
		{
			/// �ش� ���� ��ų �������� ã�Ҵٸ�, ������ ���̵� ��ȯ
			if ( eSkillID == eValue )
				iUnSealedItemID = mit->first;
		}
	}

	/// ������ ���̵� ã�Ҵٸ�, ������ ������ ���� �ȳ� ǥ��
	if ( iUnSealedItemID != 0 )
	{
		const CX2Item::ItemTemplet * pItemTemplet = g_pData->GetItemManager()->GetItemTemplet( iUnSealedItemID );
		if ( NULL != pItemTemplet )
		{			
			wstring wstrItemName = pItemTemplet->GetName();

			wstrDesc += L"#CFF0000";
			wstrDesc += GET_REPLACED_STRING( ( STR_ID_25304, "L", wstrItemName ) );;
			wstrDesc += L"#CX\n\n";
		}	
	}

}
#endif // UPGRADE_SKILL_SYSTEM_2013