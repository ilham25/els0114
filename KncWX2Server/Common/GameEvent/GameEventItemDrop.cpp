#include ".\gameeventitemdrop.h"

KGameEventItemDrop::KGameEventItemDrop(void) :
//{{ 2010. 07. 09  최육사	드롭률 이벤트 확장
m_iDropCount( 1 ),
m_bWithPlayPcBang( false )
//{{ 2012. 03. 28	박세훈	드롭률 설정 이벤트 개편 Merge 작업 ( 2012.03.20 lygan_조성욱 )
#ifdef DROPEVENT_RENEWAL
, m_iDungeongID( 0 )
, m_cUnitType(0)
, m_cUnitClass(0)
, m_iStartLevel(0)
, m_iEndLevel(0)
#endif //DROPEVENT_RENEWAL
//}}
{
}

KGameEventItemDrop::~KGameEventItemDrop(void)
{
}
