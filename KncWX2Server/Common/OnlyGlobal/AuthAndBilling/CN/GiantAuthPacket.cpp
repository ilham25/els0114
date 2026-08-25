#include "GiantAuthPacket.h"

#ifdef SERV_COUNTRY_CN

KGiantAuthPacket::KGiantAuthPacket()
: KGiantCommonPacket()
{
}

KGiantAuthPacket::~KGiantAuthPacket()
{
}

bool KGiantAuthPacket::Read( KEPUBLISHER_AUTHENTICATION_REQ& kPacket )
{
	int iCP = 0;
	_JIF( Read4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Read2Byte( kPacket.m_usZone, iCP ), return false );
	_JIF( Read2Byte( kPacket.m_usGame, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrServiceAccountID, 48, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrServicePassword, 33, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrIP, 16, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrMACAddress, 13, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrPublisherOTP, 9, iCP ), return false );
	LIF( iCP + ms_iHeaderSize == m_iPacketLength );

	return true;
}

bool KGiantAuthPacket::Write( const KEPUBLISHER_AUTHENTICATION_REQ& kPacket )
{
	m_byteCommand = GAP_ACT_LOGIN;
	m_byteParaCommand = GA_PCT_LOGIN_REQ;

	int iCP = 0;
	_JIF( Write4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Write2Byte( kPacket.m_usZone, iCP ), return false );
	_JIF( Write2Byte( kPacket.m_usGame, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrServiceAccountID, 48, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrServicePassword, 33, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrIP, 16, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrMACAddress, 13, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrPublisherOTP, 9, iCP ), return false );
	m_iPacketLength = iCP + ms_iHeaderSize;

	return true;
}

bool KGiantAuthPacket::Read( KEGIANT_AUTH_LOGIN_SUCCESS& kPacket )
{
	int iCP = 0;
	_JIF( Read4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiUID, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrName, 48, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiType, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrEMail, 48, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiState, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiBindType, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrSuperpasswd, 16, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrMobileAccount, 48, iCP ), return false );
	LIF( iCP + ms_iHeaderSize == m_iPacketLength );

	return true;
}

bool KGiantAuthPacket::Write( const KEGIANT_AUTH_LOGIN_SUCCESS& kPacket )
{
	return true;
}

bool KGiantAuthPacket::Read( KEGIANT_AUTH_LOGIN_FAIL& kPacket )
{
	int iCP = 0;
	_JIF( Read4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_iReturnCode, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrName, 48, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiKey, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrCoordinate, 7, iCP ), return false );
	LIF( kPacket.m_iReturnCode < 0 );
	LIF( iCP + ms_iHeaderSize == m_iPacketLength );

	return true;
}

bool KGiantAuthPacket::Write( const KEGIANT_AUTH_LOGIN_FAIL& kPacket )
{
	m_byteCommand = GAP_ACT_LOGIN;
	m_byteParaCommand = GA_PCT_LOGIN_FAIL;

	int iCP = 0;
	_JIF( Write4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Write4Byte( kPacket.m_iReturnCode, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrName, 48, iCP ), return false );
	_JIF( Write4Byte( kPacket.m_uiKey, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrCoordinate, 7, iCP ), return false );
	LIF( kPacket.m_iReturnCode < 0 );
	LIF( iCP + ms_iHeaderSize == m_iPacketLength );

	return true;
}

bool KGiantAuthPacket::Read( KEPUBLISHER_SECURITY_AUTH_REQ& kPacket )
{
	int iCP = 0;
	_JIF( Read4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Read2Byte( kPacket.m_usZone, iCP ), return false );
	_JIF( Read2Byte( kPacket.m_usGame, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrUserID, 48, iCP ), return false );
	_JIF( ReadString( kPacket.m_wstrMatrix, 7, iCP ), return false );
	_JIF( Read4Byte( kPacket.m_uiMatrixKey, iCP ), return false );
	LIF( iCP + ms_iHeaderSize == m_iPacketLength );

	return true;
}

bool KGiantAuthPacket::Write( const KEPUBLISHER_SECURITY_AUTH_REQ& kPacket )
{
	m_byteCommand = GAP_ACT_LOGIN;
	m_byteParaCommand = GA_PCT_LOGIN_MTCARD;

	int iCP = 0;
	_JIF( Write4Byte( kPacket.m_uiRequestID, iCP ), return false );
	_JIF( Write2Byte( kPacket.m_usZone, iCP ), return false );
	_JIF( Write2Byte( kPacket.m_usGame, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrUserID, 48, iCP ), return false );
	_JIF( WriteString( kPacket.m_wstrMatrix, 7, iCP ), return false );
	_JIF( Write4Byte( kPacket.m_uiMatrixKey, iCP ), return false );
	m_iPacketLength = iCP + ms_iHeaderSize;

	return true;
}

#endif // SERV_COUNTRY_CN