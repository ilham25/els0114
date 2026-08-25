//#define DUNGEON_SELECT_SKILLTREE_LOCK          //김창한 / 13-01-11 / 던전선택화면이 떳을때 스킬트리 단축키 'K' 입력을 막음 : 멘티스<0348513>
//#define DUNGEON_SELECT_ALL_CLOSE			   //김창한 / 13-01-16 / 던전시작버튼을 선택하면 열려있는 다이얼로그들을 닫는다 : 멘티스<0348513>의 새로운 문제
                                               //                    인벤토리,캐릭터상태창,스킬트리창,커맨드스킬창,퀘스트창,펫리스트창,커뮤니티창,옵션창

//SERV_ENCHANT_ITEM_DROP_EVENT				  ServerDefine.h에 등록되어 있음 / 김창한 / 13-01-15 / 드랍되는 장비에 강화 수치 붙여서 출력
 
//#define MANUFACTURE_MINUS_CHECK_ELD				//김창한 / 13-01-17 / 0348822: 코보 특제 선물상자 - [엘의나무열매] 제조 시, 수량 입력 기능 오류 문제
												//코보 특제 선물상자를 만드는 과정에서 수량창이 뜨고 마이너스버튼을 누르면 최대제작가능수량이 ED를 체크하지 않고 재료갯수만 체크해서 문제가 발생
                                                //ED를 넘은 수량이 수량창에 자동으로 출력되고 확인을 누르면 ED가 부족하다고 뜬다.그리고 그 뒤에 제작하기 버튼을 누르면 수량에 관계없이 제작이 불가능하게 됨
                                                //최대제작가능수량을 체크하는 GetMaxMakeNum()함수에서 소유하고 있는 ED도 체크하도록 수정


//#define GET_STRING_NULL_CHECK					//김창한 / 13-01-21 / crash : x2!CKTDGUIStatic::GetString+0xa [d:\hudson\jobs\es_kr_service\workspace\source\ktdxlib\ktdguistatic.cpp @ 336]
												//GetString()함수 후 NULL 체크를 하도록 수정


//#define GET_ITEM_NULL_CHECK					//김창한 / 13-01-21/ crash : x2!CX2Eqip::GetItem+0x19 [d:\hudson\jobs\es_kr_service\workspace\source\x2lib\x2eqip.cpp @ 1833]
												//원인을 찾지 못함

//#define SWEEP_ROLLING_END_NULL_CHECK			//김창한 / 13-01-21/ crash : x2!CX2GUEve::ENSI_SA_EPR_SWEEP_ROLLING_END_Start+0x64 [d:\hudson\jobs\es_kr_service\workspace\source\x2lib\x2gueve.cpp @ 23927]
												//GetMainEffect()와 GetDamageData()함수 후 NULL 체크를 하도록 수정


//#define UNIT_VIEWER_UI_ONFRAMEMOVE				//김창한 / 13-01-21/ crash : x2!CX2UnitViewerUI::OnFrameMove+0x300 [d:\hudson\jobs\es_kr_service\workspace\source\x2lib\x2unitviewerui.cpp @ 247]
												//GetInventory()함수 후 NULL체크를 하도록 수정

//#define GET_MAINEFFECT_NULL_CHECK				//김창한 / 13-01-21/ crash : x2!CKTDGXMeshPlayer::CXMeshInstance::SetRotateLocalDegree+0x8 [d:\hudson\jobs\es_kr_service\workspace\source\ktdxlib\ktdgxmeshplayer.h @ 960]
												//GetMainEffect()함수 후 NULL체크를 하도록 수정


//#define PET_CHANGESATIETY_NULL_CHECK			//김창한 / 13-01-21/ crash : x2!CX2PetManager::ChangeSatiety+0x112 [d:\hudson\jobs\es_kr_service\workspace\source\x2lib\x2petmanager.cpp @ 2310]
												//GetPetInfo()함수 후 NULL체크를 하도록 수정


//#define PVP_EVE_CLOCKING_EFFECT					//김창한 / 13-01-24/ 0348885: 대전맵에서 이브가 클로킹 스킬 시전 시, 무기강화 이팩트가 노출되는 문제



//SERV_FIRST_SELECT_UNIT_REWARD_SYSTEM		ServerDefine.h에 등록되어 있음 / 김창한 / 13-01-22 /대규모 이벤트 보상 시 최초 선택 캐릭터에게 지급하는 시스템

//#define NEVER_MOVE_GRAVITY_APPLY			//김창한 / 13-02-12 / 버프 BBT_NEVER_MOVE에 y축은 고정이 아니라 중력이 적용되도록 수정

//GetStrByLienBreak
//iUniCharSize = static_cast<int>(static_cast<float>(pFont->GetWidth( tempWchar )) * g_pKTDXApp->GetResolutionScaleX() );


//#define NEXON_QA_CHEAT_REQ					//김창한 / 13-02-18 / 넥슨 퍼블리싱 QA 테스트 명령어 구현 요청 구현

#define GUILD_RADIO_NULL_CHECK				//crash 수정. 길드 탭 UI에서 라디오 버튼 생성한 후 NULL체크하도록 변경

// SERV_HERO_PVP_MANAGE_LIST					//영웅 대전 UI 추가. 아직 작업중

//#define SPT_NONE_ITEM_DELETE						// cocy33	/ 김창한 / 13-03-13 / 되팔기 불가 아이템 버리기 기능.

//#define SERV_LOCAL_RANKING_SYSTEM						//지인시스템

// SERV_APRIL_FOOLS_DAY						//만우절 이벤트 - 스킬 컷신 교체, 벤더스 NPC 설치

// SERV_RECOMMEND_LIST_EVENT					//친구 찾기 이벤트


//#define NASOD_SCOPE_ID_SAVE				//나소드 메가폰 메세지 입력창에서 퀵슬롯 아이템을 클릭한 후 사용하면 크래쉬 나는 문제 수정

//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-03-29
// 패치날짜: 2013-04-02
// 작업내용: 나소드 메가폰 메세지 입력창에서 퀵슬롯 아이템을 클릭한 후 사용하면 크래쉬 나는 문제 수정
//#define FIX_SEND_LIST_AT_LEAVE_GUILD
//////////////////////////////////////////////////////////////////////////

//#define RIDING_PET //임시로 만듬  >>RIDING_SYSTEM로 수정


//////////////////////////////////////////////////////////////////////////
// 이름: 강정훈
// 아이디: JHKang
// 작업시작날짜: 2013-03-26
// 패치날짜: 미정
// 작업내용: 탈 것 시스템
//#define RIDING_SYSTEM
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-05-08
// 패치날짜: 2013-05-09
// 작업내용: 어린이날 이벤트 아이템
//#define CHILDRENS_DAY_EVENT_ITEM
//////////////////////////////////////////////////////////////////////////



//#define NEW_OPTION_SECURITYPAD //옵션창, 보안패드 개편

//#define SERV_RELATIONSHIP_SYSTEM_LAUNCHING_EVENT	//결혼 시스템 런칭 이벤트

//#define EMBLEM_DISPLAY_LIST //중간에 출력되는 엠블렘 순차적으로 출력되게.
//#ifdef EMBLEM_DISPLAY_LIST
//struct EmblemDisplayList 
//{
//	wstring			wstrName;
//	D3DXVECTOR2		vec2Position;
//
//
//};
//#endif //EMBLEM_DISPLAY_LIST


//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-05-31
// 패치날짜: 
// 작업내용: 로컬랭킹 개선 작업
//#define LOCAL_RANKING_UPDATE
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-06-10
// 패치날짜: 
// 작업내용: 탈 것 소환석 사용시 확인 절차 추가하기
//#define RIDING_ITEM_POPUP_CHECK
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-06-10
// 패치날짜: 
// 작업내용: EFFECTSET에서 Trace한 유닛이 죽었을 경우 해당 이펙트셋을 삭제하는 기능
//#define TRACE_UNIT_DIE_DELETE_EFFECTSET
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 이름: 김창한
// 아이디: cocy33
// 작업시작날짜: 2013-07-03
// 패치날짜: 2013-07-04
// 작업내용: 교환 리스트에서 수량이 부족하면 상위로 정렬되지 않도록 수정.
//#define EXCHANGE_ITEM_SORT_FIX
//////////////////////////////////////////////////////////////////////////
