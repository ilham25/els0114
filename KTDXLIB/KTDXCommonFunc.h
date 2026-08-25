#pragma once

#include < KncUtil.h >
#pragma comment(lib,"Psapi.lib")
#include "psapi.h"

inline void MakeUpperCase(char* str) 
{
	KTDXPROFILE();
	for( char* i = str; (*i) != 0; i++ ) 
		*i = (char)toupper(*i);
}

inline void MakeUpperCase(std::string &str) 
{
	KTDXPROFILE();
	for( std::string::iterator i = str.begin(); i != str.end(); i++ ) 
		*i = (char)toupper(*i);
}

inline void MakeUpperCase(WCHAR* str) 
{
	KTDXPROFILE();
	for( WCHAR* i = str; (*i) != 0; i++ ) 
		*i = (WCHAR)towupper(*i);
}

inline void MakeUpperCase(std::wstring &str) 
{
	KTDXPROFILE();
	for( std::wstring::iterator i = str.begin(); i != str.end(); i++ ) 
		*i = (WCHAR)towupper(*i);
}

inline void MakeUpperCase(WCHAR* dest, size_t size, const WCHAR* str) 
{
	if ( dest == NULL || size == 0 )
		return;
	//KTDXPROFILE();
	WCHAR lower;
	size_t  count = 0;
	while( count + 1 < size && ( lower = (*str) ) != 0 )
	{
		*dest = (WCHAR)towupper(lower);
		dest++;
		str++;
	}//while
	*dest = 0;
}

inline void ConvertCharToWCHAR( wstring& dest, const string& src )
{
	dest.clear();
	if( src.empty() )
		return;
	
	int iSrcLength = src.length();
	if ( iSrcLength <= 0 )
	{
		dest = L"";
		return;
	}//if

	int iDestSize = ::MultiByteToWideChar( CP_ACP, 0, src.c_str(), iSrcLength, NULL, 0 );
	if ( iDestSize <= 0 )
	{
		dest = L"";
		return;
	}//if

	WCHAR* wcharBuf = (WCHAR*) _alloca( iDestSize * sizeof( WCHAR ) );
	ASSERT( wcharBuf != NULL );
	int iNewSize = ::MultiByteToWideChar( CP_ACP, 0, src.c_str(), iSrcLength, wcharBuf, iDestSize );
	ASSERT( iNewSize == iDestSize );
	dest.assign( wcharBuf, iNewSize );
}



inline void ConvertWCHARToChar( string& dest, const wstring& src )
{
	KTDXPROFILE();

	dest.clear();
	if( src.empty() )
		return; 

	int iSrcLength = src.length();
	if ( iSrcLength <= 0 )
	{
		dest = "";
		return;
	}//if

	int iDestSize = ::WideCharToMultiByte( CP_ACP, 0, src.c_str(), iSrcLength, NULL, 0, NULL, NULL );
	if ( iDestSize <= 0 )
	{
		dest = "";
		return;
	}//if

	char* pszConv = (char*) _alloca( iDestSize * sizeof(char) );
	ASSERT( pszConv != NULL );
	int iNewSize = ::WideCharToMultiByte( CP_ACP, 0, src.c_str(), iSrcLength, pszConv, iDestSize, NULL, NULL );
	ASSERT( iNewSize == iDestSize );
	dest.assign( pszConv, iNewSize );
}




inline void ConvertCharToWCHAR( wstring& dest, const char* pSrc, const int iLength = -1  )
{
	KTDXPROFILE();

//{{ robobeg : 2008-01-08
	//if ( pSrc == NULL )
	//	return;

	//int iSize = strlen( pSrc );
	//if ( iSize <= 0 )
	//	return;

	//WCHAR* wcharBuf = (WCHAR*) alloca( (iSize + 1) * sizeof( WCHAR ) );
	//ZeroMemory( wcharBuf, (iSize + 1) * sizeof( WCHAR ) );

	//MultiByteToWideChar( CP_ACP, 0, pSrc, -1, wcharBuf, iSize );
	//dest = wcharBuf;

    dest.clear();
    if ( pSrc == NULL )
        return;

	int iSrcLength = iLength;
	if( -1 == iLength )
	{
		iSrcLength = strlen( pSrc );
		if ( iSrcLength <= 0 )
		{
			dest = L"";
			return;
		}//if
	}

    int iDestSize = ::MultiByteToWideChar( CP_ACP, 0, pSrc, iSrcLength, NULL, 0 );
    if ( iDestSize <= 0 )
    {
        dest = L"";
        return;
    }//if

	WCHAR* wcharBuf = (WCHAR*) _alloca( iDestSize * sizeof( WCHAR ) );
    ASSERT( wcharBuf != NULL );
    int iNewSize = ::MultiByteToWideChar( CP_ACP, 0, pSrc, iSrcLength, wcharBuf, iDestSize );
    ASSERT( iNewSize == iDestSize );
    dest.assign( wcharBuf, iNewSize );
//}} robobeg : 2008-01-08
}


inline void ConvertWCHARToChar( string& dest, const WCHAR* pSrc, const int iLength = -1 )
{
	KTDXPROFILE();

//{{ robobeg : 2008-01-08
	//if ( pSrc == NULL )
	//	return;

	//int iSize = wcslen( pSrc ) * sizeof( WCHAR );
	//if ( iSize <= 0 )
	//	return;

	//char* charBuf = (char*) alloca( iSize + 1 );
	//ZeroMemory( charBuf, iSize + 1 );

	//WideCharToMultiByte( CP_ACP, 0, pSrc, -1, charBuf, iSize, NULL, NULL );
	//dest = charBuf;

    dest.clear();
    if ( pSrc == NULL )
        return;

	int iSrcLength = iLength;
	if( -1 == iSrcLength )
	{
		iSrcLength = wcslen( pSrc );
		if ( iSrcLength <= 0 )
		{
			dest = "";
			return;
		}//if
	}

    int iDestSize = ::WideCharToMultiByte( CP_ACP, 0, pSrc, iSrcLength, NULL, 0, NULL, NULL );
    if ( iDestSize <= 0 )
    {
        dest = "";
        return;
    }//if

    char* pszConv = (char*) _alloca( iDestSize * sizeof(char) );
    ASSERT( pszConv != NULL );
    int iNewSize = ::WideCharToMultiByte( CP_ACP, 0, pSrc, iSrcLength, pszConv, iDestSize, NULL, NULL );
    ASSERT( iNewSize == iDestSize );
    dest.assign( pszConv, iNewSize );
//}} robobeg : 2008-01-08
}

inline void ConvertUtf8ToWCHAR( wstring& dest, const char* pSrc, const int iLength = -1 )
{
    KTDXPROFILE();

    dest.clear();
    if( !pSrc )
    {
        return;
    }

    int iSrcLength = iLength;
    if( -1 == iSrcLength )
    {
        iSrcLength = strlen( pSrc );
        if ( iSrcLength <= 0 )
        {
            dest = L"";
            return;
        }//if
    }

    int iDestSize = ::MultiByteToWideChar( CP_UTF8, 0, pSrc, iSrcLength, NULL, 0 );
    if ( iDestSize <= 0 )
    {
        dest = L"";
        return;
    }//if

    WCHAR* wcharBuf = (WCHAR*) _alloca( iDestSize * sizeof( WCHAR ) );
    ASSERT( wcharBuf != NULL );
    int iNewSize = ::MultiByteToWideChar( CP_UTF8, 0, pSrc, iSrcLength, wcharBuf, iDestSize );
    ASSERT( iNewSize == iDestSize );
    dest.assign( wcharBuf, iNewSize );
}

inline float GetDistance( const float& x1, const float& y1, const float& z1, const float& x2, const float& y2, const float& z2 )
{
	KTDXPROFILE();
	return sqrtf( powf( x2 - x1, 2.0f ) + powf( y2 - y1, 2.0f ) + powf( z2 - z1, 2.0f ) );
}

inline float GetDistance( const D3DXVECTOR3& pos1, const D3DXVECTOR3& pos2 )
{
	D3DXVECTOR3 temp = pos2 - pos1;
	return D3DXVec3Length( &temp );
}

inline float GetDistance_LUA( const D3DXVECTOR3& pos1, const D3DXVECTOR3& pos2 )
{
	D3DXVECTOR3 temp = pos2 - pos1;
	return D3DXVec3Length( &temp );
}

inline float GetDistance2( const float& x1, const float& y1, const float& x2, const float& y2 )
{
	KTDXPROFILE();
	return sqrtf( powf( x2 - x1, 2.0f ) + powf( y2 - y1, 2.0f ) );
}

inline float GetDistance2( const D3DXVECTOR3& pos1, const D3DXVECTOR3& pos2 )
{
	D3DXVECTOR2 temp = pos2 - pos1;
	return D3DXVec2Length( &temp );
}

inline float GetDistance2_LUA( const D3DXVECTOR3& pos1, const D3DXVECTOR3& pos2 )
{
	D3DXVECTOR2 temp = pos2 - pos1;
	return D3DXVec2Length( &temp );
}

inline float GetDistance3Sq( const D3DXVECTOR3& vPos1_, const D3DXVECTOR3& vPos2_ )
{
	D3DXVECTOR3 vResult = vPos2_ - vPos1_;

	return D3DXVec3LengthSq( &vResult );
}

inline bool OUTOF_RANGE( const float& x, const float& rangeMin, const float& rangeMax ) 
{
	if( x < rangeMin || x > rangeMax )
		return true;
	else
		return false;
}

inline CHAR* StrHeapCopy( CHAR *sName )
{
//{{ robobeg : 2008-01-05
	//DWORD dwLen = (DWORD) strlen( sName ) + 1;
	//CHAR * sNewName = new CHAR[ dwLen ];
	//if( sNewName )
	//	StringCchCopyA( sNewName, dwLen, sName );
	//return sNewName;
    ASSERT( sName != NULL );
	DWORD dwLen = (DWORD) strlen( sName ) + 1;
	CHAR * sNewName = new CHAR[ dwLen ];
    ASSERT( sNewName != NULL );
    StringCchCopyA( sNewName, dwLen, sName );
	return sNewName;
//}} robobeg : 2008-01-05
}

inline std::string RemoveQuotes(std::string &str) 
{
	for (std::string::iterator i = str.begin(); i != str.end(); i++) 
	{
		if (*i == '\"') 
		{
			i = str.erase(i); 
			if (i == str.end()) 
				break;
		}
	}
	return(str);
}

inline D3DXVECTOR3 GetDecomposeScale( D3DXMATRIX* pMat )
{
	KTDXPROFILE();

	ASSERT( NULL != pMat );
	
	D3DXVECTOR3 scale;

	// set scaling
	float fBeforeSqrtf = pMat->m[0][0] * pMat->m[0][0] +
		pMat->m[0][1] * pMat->m[0][1] +
		pMat->m[0][2] * pMat->m[0][2];

	scale.x = ( 0 < fBeforeSqrtf ? sqrtf( fBeforeSqrtf ) : 0.0f );

	fBeforeSqrtf = pMat->m[1][0] * pMat->m[1][0] +
		pMat->m[1][1] * pMat->m[1][1] +
		pMat->m[1][2] * pMat->m[1][2];

	scale.y = ( 0 < fBeforeSqrtf ? sqrtf( fBeforeSqrtf ) : 0.0f );

	fBeforeSqrtf = pMat->m[2][0] * pMat->m[2][0] +
		pMat->m[2][1] * pMat->m[2][1] +
		pMat->m[2][2] * pMat->m[2][2];

	scale.z = ( 0 < fBeforeSqrtf ? sqrtf( fBeforeSqrtf ) : 0.0f );

	return scale;
}

inline D3DXVECTOR3 GetDecomposeAngle( D3DXMATRIX* pMat )
{
	KTDXPROFILE();

	ASSERT( NULL != pMat );

	D3DXVECTOR3 angle;

	float   fSinX = -pMat->_32;//GetElement(3,2);

	angle.x = asinf( fSinX );

	float   fSinY, fSinZ, fCosY, fCosZ;

	if ( 1 - fabs( fSinX ) > 1.0e-5f ) // Gimbal lock?
	{
		fCosY  = pMat->_33;//(3,3);
		fSinY  = pMat->_31;//(3,1);
		angle.y  = atan2f( fSinY, fCosY );

		fCosZ  = pMat->_22;//(2,2);
		fSinZ  = pMat->_12;//(1,2);
		angle.z = atan2f( fSinZ, fCosZ );
	}
	else // Gimbal lock has occurred
	{
		angle.y  = 0; // Yaw is undefined; just fix it
		fCosZ  = pMat->_11;//(1,1);
		fSinZ  = pMat->_13/*(1,3)*/ * fSinX;
		angle.z = atan2f( fSinZ, fCosZ );
	}//if.. else..

	return angle;
}

#ifdef DAMAGE_EFFECT_BUG_FIX
inline D3DXVECTOR3 GetDecomposeAngle( D3DXMATRIX* pMat, D3DXVECTOR3 vecScale_ )
{
	KTDXPROFILE();

	ASSERT( NULL != pMat );

	D3DXVECTOR3 angle;
	D3DXMATRIX matScale;

	if ( vecScale_.x <= 0.f )
		vecScale_.x = 1.f;
	if ( vecScale_.y <= 0.f )
		vecScale_.y = 1.f;
	if ( vecScale_.z <= 0.f )
		vecScale_.z = 1.f;

	D3DXMatrixIdentity( &matScale );
	D3DXMatrixScaling( &matScale, 1.f / vecScale_.x, 1.f / vecScale_.y, 1.f / vecScale_.z );
	D3DXMatrixMultiply( &matScale, pMat, &matScale );

	float   fSinX = -matScale._32;//GetElement(3,2);

	angle.x = asinf( fSinX );

	float   fSinY, fSinZ, fCosY, fCosZ;

	if ( 1 - fabs( fSinX ) > 1.0e-5f ) // Gimbal lock?
	{
		fCosY  = matScale._33;//(3,3);
		fSinY  = matScale._31;//(3,1);
		angle.y  = atan2f( fSinY, fCosY );

		fCosZ  = matScale._22;//(2,2);
		fSinZ  = matScale._12;//(1,2);
		angle.z = atan2f( fSinZ, fCosZ );
	}
	else // Gimbal lock has occurred
	{
		angle.y  = 0; // Yaw is undefined; just fix it
		fCosZ  = matScale._11;//(1,1);
		fSinZ  = matScale._13/*(1,3)*/ * fSinX;
		angle.z = atan2f( fSinZ, fCosZ );
	}//if.. else..

	return angle;
}
#endif

inline D3DXVECTOR3 GetDirVecToDegree( D3DXVECTOR3 dirVec )
{
	KTDXPROFILE();

	D3DXVECTOR3 outRadian;
	D3DXVECTOR3 outDegree;
	float temp;
	D3DXMATRIX matRotationOrth;
	D3DXMATRIX matRotation;


	D3DXVECTOR3 vZero(0, 0, 0), vUp(0, 1, 0);
	

	D3DXVec3Normalize( &dirVec, &dirVec );
	D3DXMatrixLookAtLH( &matRotation, &vZero, &dirVec, &vUp );
	D3DXMatrixInverse( &matRotationOrth, NULL, &matRotation );

	temp = matRotationOrth._11;
	matRotationOrth._11 = matRotationOrth._13;
	matRotationOrth._13 = temp;

	temp = matRotationOrth._21;
	matRotationOrth._21 = matRotationOrth._23;
	matRotationOrth._23 = temp;

	temp = matRotationOrth._31;
	matRotationOrth._31 = matRotationOrth._33;
	matRotationOrth._33 = temp;

	if( matRotationOrth._12 == 0 && matRotationOrth._22 == 0 )
	{
		outRadian.z	= 0.0f;
		outRadian.y	= atan2f( -matRotationOrth._31, matRotationOrth._33 );
		outRadian.x	= atan2f( -matRotationOrth._21, matRotationOrth._11 );

		outDegree.z	= 0.0f;
		outDegree.y	= D3DXToDegree(outRadian.y);
		outDegree.x	= D3DXToDegree(outRadian.x);
	}
	else
	{
		outRadian.z	= asinf( matRotationOrth._32 );
		outRadian.y	= atan2f( -matRotationOrth._31, matRotationOrth._33 );
		outRadian.x	= atan2f( -matRotationOrth._12, matRotationOrth._22 );

		outDegree.z	= D3DXToDegree(outRadian.z);
		outDegree.y	= D3DXToDegree(outRadian.y);
		outDegree.x	= 0.0f;//D3DXToDegree(outRadian.x);
		//outDegree.x	= 0.0f;
	}

	return outDegree;
}

inline bool Pick2DRect( const POINT& ptMouse,
					   const D3DXVECTOR2& vLeftTop, const D3DXVECTOR2& vRightTop,
					   const D3DXVECTOR2& vLeftBottom,	const D3DXVECTOR2& vRightBottom )
{
	KTDXPROFILE();

	//삼각형 점포함 공식으로 피킹을 한다.
	// 1. V[AB] X V[AT]
	// 2. V[BC] X V[BT]
	// 3. V[CD] X V[CT]
	// 4. V[DA] X V[CT]
	// 1, 2, 3, 4 의 부호가 같으면 점이 포함한다.
	D3DXVECTOR2	vT;
	D3DXVECTOR2	vAB, vBC, vCD, vDA;
	D3DXVECTOR2	vAT, vBT, vCT, vDT;

	vT.x = (float)ptMouse.x;
	vT.y = (float)ptMouse.y;

	vAB	= vRightTop		- vLeftTop;
	vBC	= vRightBottom	- vRightTop;
	vCD	= vLeftBottom	- vRightBottom;
	vDA	= vLeftTop		- vLeftBottom;

	vAT	= vT - vLeftTop;
	vBT = vT - vRightTop;
	vCT = vT - vRightBottom;
	vDT	= vT - vLeftBottom;

	if( (vAB.x*vAT.y - vAB.y*vAT.x) > 0.0f &&
		(vBC.x*vBT.y - vBC.y*vBT.x) > 0.0f &&
		(vCD.x*vCT.y - vCD.y*vCT.x) > 0.0f &&
		(vDA.x*vDT.y - vDA.y*vDT.x) > 0.0f)
	{
		return true;
	}
	else if( (vAB.x*vAT.y - vAB.y*vAT.x) < 0.0f &&
		(vBC.x*vBT.y - vBC.y*vBT.x) < 0.0f &&
		(vCD.x*vCT.y - vCD.y*vCT.x) < 0.0f &&
		(vDA.x*vDT.y - vDA.y*vDT.x) < 0.0f)
	{
		return true;
	}

	return false;
}




static inline USHORT floatToHalfI(UINT i)
{
	register int s =  (i >> 16) & 0x00008000;
	register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	register int m =   i        & 0x007fffff;

	if (e <= 0)
	{
		if (e < -10)
		{
			return 0;
		}
		m = (m | 0x00800000) >> (1 - e);

		return s | (m >> 13);
	}
	else if (e == 0xff - (127 - 15))
	{
		if (m == 0) // Inf
		{
			return s | 0x7c00;
		} 
		else    // NAN
		{
			m >>= 13;
			return s | 0x7c00 | m | (m == 0);
		}
	}
	else
	{
		if (e > 30) // Overflow
		{
			return s | 0x7c00;
		}

		return s | (e << 10) | (m >> 13);
	}
}
static inline USHORT floatToHalf(float i)
{
	return floatToHalfI(*reinterpret_cast<UINT*>(&i));
}

static inline UINT halfToFloatI(USHORT y)
{
	register int s = (y >> 15) & 0x00000001;
	register int e = (y >> 10) & 0x0000001f;
	register int m =  y        & 0x000003ff;

	if (e == 0)
	{
		if (m == 0) // Plus or minus zero
		{
			return s << 31;
		}
		else // Denormalized number -- renormalize it
		{
			while (!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}
			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31)
	{
		if (m == 0) // Inf
		{
			return (s << 31) | 0x7f800000;
		}
		else // NaN
		{
			return (s << 31) | 0x7f800000 | (m << 13);
		}
	}
	e = e + (127 - 15);
	m = m << 13;
	return (s << 31) | (e << 23) | m;
}

static inline float halfToFloat(USHORT y)
{
	UINT r = halfToFloatI(y);
	return *reinterpret_cast<float*>(&r);
}


inline float RandomFloat( float fMinBound = 0.f, float fMaxBound = 1.f )
{
	if( fMinBound == fMaxBound )
		return fMinBound;

	return (float)(rand()%10001) / 10000.f * (fMaxBound-fMinBound ) + fMinBound;
}

inline int RandomInt( int iMinBound = 0, int iMaxBound = 100 )
{
	if( iMinBound == iMaxBound )
		return iMinBound;

	return rand()%(iMaxBound-iMinBound) + iMinBound;
}

inline char* XORDecrypt( const char* pEncryptedBuffer, int iBufferSize )
{
	int	aDecryptionKey[3] = { XOR_KEY0, XOR_KEY1, XOR_KEY2 };

	char pBuf[5];

	std::string strCryptBuffer;

	if( pEncryptedBuffer == NULL )
		return NULL;

	int iKeyIndex = 0;
	int iKeySize = 3;
	int iRemainSize = iBufferSize;
	int iCryptedSize = 0;

	while( iRemainSize > 0 )
	{
		memset( pBuf, 0, 5 );

		int iBufTemp;
//{{ robobeg : 2012-11-23
		if ( iRemainSize >= 4 )
			memcpy( &iBufTemp, pEncryptedBuffer + iCryptedSize, 4 );
		else
			memcpy( &iBufTemp, pEncryptedBuffer + iCryptedSize, iRemainSize );
//}} robobeg : 2012-11-23

		// 복호화
		int iEncryptData = iBufTemp ^ aDecryptionKey[iKeyIndex];

		memcpy( pBuf, &iEncryptData, 4 );

		if( iRemainSize < 4 )
		{
			strCryptBuffer.append( pBuf, iRemainSize );
			break;
		}
		else
		{
			strCryptBuffer.append( pBuf, 4 );
		}		

		// 남은 버퍼 길이 감소
		iRemainSize -= 4;

		// 복호화한 길이 증가
		iCryptedSize += 4;

		// 4바이트씩 번갈아 가면서 복호화키 적용
		++iKeyIndex;

		if( iKeyIndex >= iKeySize )
			iKeyIndex = 0;

	}

	// 힙에 버퍼 생성 후 복호화된 데이터 담기
	char* pDecryptedBuffer = new char[strCryptBuffer.size()+1];
	memset( pDecryptedBuffer, 0, strCryptBuffer.size()+1 );
	memcpy( pDecryptedBuffer, strCryptBuffer.c_str(), strCryptBuffer.size() );	

	return pDecryptedBuffer;
}

inline char* XOREncrypt( const char* pEncryptedBuffer, int iBufferSize )
{
	return XORDecrypt( pEncryptedBuffer, iBufferSize );
}

inline bool FileEncrypt( std::string strFileName )
{
	std::vector<int> vecEncryptionKey;
	vecEncryptionKey.push_back( XOR_KEY0 );
	vecEncryptionKey.push_back( XOR_KEY1 );
	vecEncryptionKey.push_back( XOR_KEY2 );

	char pBuf[5];
	std::string strEncryptedFileName;
	strEncryptedFileName = strFileName;
	strEncryptedFileName += ".encrypt.lua";

	std::ofstream ofs;
	std::ifstream ifs;
	std::string strCryptBuffer;
	ifs.open( strFileName.c_str(), std::ios::binary );
	ofs.open( strEncryptedFileName.c_str(), std::ios::binary );

	if( ifs.is_open() == false || ofs.is_open() == false )	
		return false;

	// 파일 크기 구하기
	struct stat fileSize;
	stat( strFileName.c_str(), &fileSize );

	int iKeyIndex = 0;
	int iKeySize = vecEncryptionKey.size();	
	int iRemainSize = fileSize.st_size;

	while( !ifs.eof() )
	{
		memset( pBuf, 0, 5 );

		// 파일버퍼 읽기
		ifs.read( pBuf, 4 );

		int iBufTemp;
		memcpy( &iBufTemp, pBuf, 4 );

		// 암호화
		int iEncryptData = iBufTemp ^ vecEncryptionKey[iKeyIndex];

		memcpy( pBuf, &iEncryptData, 4 );

		if( iRemainSize < 4 )
		{
			strCryptBuffer.append( pBuf, iRemainSize );
			break;
		}
		else
		{
			strCryptBuffer.append( pBuf, 4 );
		}

		// 남은 파일 크기 감소
		iRemainSize -= 4;

		// 4바이트씩 번갈아 가면서 암호화키 적용
		++iKeyIndex;

		if( iKeyIndex >= iKeySize )
			iKeyIndex = 0;
	}

	ofs.write( strCryptBuffer.c_str(), strCryptBuffer.size() );

	ifs.close();
	ofs.close();

	_chmod( strFileName.c_str(), _S_IREAD | _S_IWRITE ); // 파일 삭제를 위해서

	if( CopyFileA( strEncryptedFileName.c_str(), strFileName.c_str(), false ) == 0 )
		return false;

	if( DeleteFileA( strEncryptedFileName.c_str() ) == 0 )
		return false;

	return true;	

}





inline bool CheckIsVistaComputer()
{
	OSVERSIONINFO sInfo;
	sInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if ( ::GetVersionEx(&sInfo) )
	{
		if ( sInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && sInfo.dwMajorVersion >= 6 && sInfo.dwMinorVersion >= 0 )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}






// 문자열을 입력으로 받아서 숫자를 tokenize해서 하나씩 리턴해준다, 포인터는 다음 token 위치로 옮겨준다
inline int TokenizeInt( WCHAR** ppStart, WCHAR* pEnd )
{
#define IN_INT_CHARSET( c ) ( (c) == L'-' || ( (c) >= L'0' && (c) <= L'9' ) )


	if( NULL == ppStart )
		return 0;

	if( NULL == *ppStart )
		return 0;


	if( *ppStart == pEnd )
		return 0;

	WCHAR* pStop = pEnd;
	int iNumber = (int) wcstol( *ppStart, &pStop, 10 );
	*ppStart = pStop;
	
	// skip leading non-number character 
	while( !IN_INT_CHARSET( **ppStart ) )
	{
		if( **ppStart == L'\0' || *ppStart == pEnd )
			break;

		++ppStart;
	}

	return iNumber;
}




// 어떤 프로세스의 메모리 사용량을 얻어오는 함수
inline DWORD GetUsingMemorySize(std::wstring strProcessName, SIZE_T& currMemSize, SIZE_T& peakMemSize )
{
	DWORD pid[1024];
	DWORD cb;
	EnumProcesses(pid,sizeof(DWORD)*1024,&cb);
	HMODULE hMo;	
	for(int i = 0 ; i < 1024 ; ++i)
	{
		WCHAR szFileName[1024];
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ,FALSE,pid[i]); 
		if(!hProcess)
			continue;
		BOOL bb=EnumProcessModules(hProcess,&hMo,sizeof(hMo),&cb);
		if(!bb)
		{
			DWORD error=GetLastError();
			LPVOID lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
				|FORMAT_MESSAGE_FROM_SYSTEM
				|FORMAT_MESSAGE_IGNORE_INSERTS
				, NULL,error,0,(LPWSTR)&lpMsgBuf,0,NULL);
			std::wstring strLastErrMsg=static_cast<LPCWSTR>(lpMsgBuf);
			LocalFree(lpMsgBuf);
			CloseHandle(hProcess);
			continue;
		}
		GetModuleFileNameEx(hProcess,hMo,szFileName,1024);
		std::wstring strProcName(szFileName);
		if(strProcName.find(strProcessName) != -1 )
		{
			PROCESS_MEMORY_COUNTERS mem;
			GetProcessMemoryInfo(hProcess,&mem,sizeof(PROCESS_MEMORY_COUNTERS));
			CloseHandle(hProcess);
			currMemSize = mem.WorkingSetSize;
			peakMemSize = mem.PeakWorkingSetSize;
			return (DWORD)mem.WorkingSetSize;
		}
		CloseHandle(hProcess);
	}
	return NULL;
}

// kimhc // CX2UISkillTree에 있던것을 옮겨와 수정함 // 2009-06-16
inline wstring GetExpirationDateDesc( const wstring& wstrExpirationDate, __time64_t currentTime, bool bShowStrID309 = true )
{
	wstring tempDesc = L"";


	if ( bShowStrID309 == true )
		tempDesc = GET_STRING( STR_ID_309 ); //tempDesc = L"만료 시간 : ";

	CTime cTime;
	KncUtil::ConvertStringToCTime( const_cast< std::wstring & >( wstrExpirationDate ), cTime );

	wstringstream wstrstm;

	//wstrstm << cTime.GetYear() << L"년 " << cTime.GetMonth() << L"월 " << cTime.GetDay() << L"일 " << cTime.GetHour() << L"시 " << cTime.GetMinute() << L"분"; 
	//tempDesc += wstrstm.str().c_str();
	//tempDesc += L"\n";

	if ( bShowStrID309 == true )
	{
		wstrstm.str( L"" );
		wstrstm << L"\n";
	}

	//time_t t0 = time(0);
	__time64_t t0 = currentTime;//g_pData->GetServerCurrentTime();

	double fDiffTime = difftime( KncUtil::TimeToInt( cTime ), (time_t)t0 );

	int remainDay =  (int)(fDiffTime / ( 60 * 60 * 24 ) );
	if ( remainDay > 0 )
		wstrstm << remainDay << GET_STRING( STR_ID_14 ) << L" ";

	int remainHour = (int)((time_t)fDiffTime % ( 60 * 60 * 24 )) / (60 * 60);
	if ( remainHour < 0 )
		remainHour = 0;

	wstrstm << remainHour << GET_STRING( STR_ID_310 ) << L" ";

	int remainMinute = (int)((time_t)fDiffTime % ( 60 * 60 )) / (60 );

	if ( remainDay <= 0 )
	{
		if ( remainMinute <= 0 )
			remainMinute = 0;
		wstrstm << remainMinute << GET_STRING( STR_ID_311 ) << L" ";
	}

	tempDesc += wstrstm.str().c_str();


	return tempDesc;

}

inline bool ConvertFileAnsiToUTF8( const string& srcFileName, const string& destFileName )
{
	FILE* srcFile = NULL;
	FILE* destFile = NULL;

	UCHAR szBuf[1024] = "";
	size_t sizeRead = 0;
	size_t sizeWrote = 0;


	string tmpDestFileName = "temp_";
	tmpDestFileName += destFileName;

#ifdef	CONVERSION_VS
	fopen_s( &srcFile, srcFileName.c_str(), "r" );
#else	CONVERSION_VS
	srcFile = fopen( srcFileName.c_str(), "r" );
#endif	CONVERSION_VS
	
	if( NULL == srcFile )
		return false;

	sizeRead = fread( szBuf, sizeof(char), 3, srcFile );
    if( sizeRead >= 3 )
    {
	    if( szBuf[0] == 0xEF && szBuf[1] == 0xBB && szBuf[2] == 0xBF )
	    {
		    fclose( srcFile );
		    return true;
	    }
    }

	fseek( srcFile, 0, SEEK_SET );


#ifdef	CONVERSION_VS
	fopen_s( &destFile, tmpDestFileName.c_str(), "w" );
#else	CONVERSION_VS
	destFile = fopen( tmpDestFileName.c_str(), "w" );
#endif	CONVERSION_VS
	
	if( NULL == destFile )
	{
		fclose( srcFile );
		return false;
	}


	fputc( 0xEF, destFile );
	fputc( 0xBB, destFile );
	fputc( 0xBF, destFile );

	//WCHAR wszHeader[MAX_PATH] = L"-- lua header. UTF-8 인코딩 인식을 위해 이 줄은 지우지 마세요.\r\n\r\n";
	//::MultiByteToWideChar(CP_UTF8,0,szHeader,-1,wszHeader,MAX_PATH);
	//fwrite( wszHeader, sizeof(WCHAR), wcslen(wszHeader), destFile );

	while( false == feof( srcFile ) )
	{
		sizeRead = fread( szBuf, sizeof(char), ARRAY_SIZE(szBuf), srcFile );
		if( sizeRead <= 0 )
			break;

		sizeWrote = fwrite( szBuf, sizeof(char), sizeRead, destFile );
		ASSERT( sizeRead == sizeWrote );
	}

	fclose( srcFile );
	fclose( destFile );


	if( 0 == srcFileName.compare( destFileName ) )
	{
		remove( srcFileName.c_str() );
	}
	rename( tmpDestFileName.c_str(), destFileName.c_str() );

	return true;
}

// 실행파일과 다른위치에 있는 파일을 컨버팅할경우.
inline bool ConvertFullPathFileAnsiToUTF8( const string& srcFullPath, const string& destFullPath )
{
	FILE* srcFile = NULL;
	FILE* destFile = NULL;

	UCHAR szBuf[1024] = "";
	size_t sizeRead = 0;
	size_t sizeWrote = 0;

	bool bIsidentical = false;
	if( 0 == srcFullPath.compare(destFullPath) )
		bIsidentical = true;

	// file path와 name을 분리
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

#ifdef	CONVERSION_VS
	_splitpath_s( destFullPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT );
#else	CONVERSION_VS
	_splitpath( destFullPath.c_str(), drive, dir, fname, ext);
#endif	CONVERSION_VS
	
	string tmpDestFileName;
	tmpDestFileName = drive;
	tmpDestFileName += dir;

	if( true == bIsidentical )
		tmpDestFileName += "temp_";

	tmpDestFileName += fname;
	tmpDestFileName += ext;

#ifdef	CONVERSION_VS
	fopen_s( &srcFile, srcFullPath.c_str(), "r" );
#else	CONVERSION_VS
	srcFile = fopen( srcFullPath.c_str(), "r" );
#endif	CONVERSION_VS
	
	if( NULL == srcFile )
		return false;

	sizeRead = fread( szBuf, sizeof(char), 3, srcFile );
	if( sizeRead >= 3 )
	{
		if( szBuf[0] == 0xEF && szBuf[1] == 0xBB && szBuf[2] == 0xBF )
		{
			fclose( srcFile );
			return true;
		}
	}

	fseek( srcFile, 0, SEEK_SET );

#ifdef	CONVERSION_VS
	fopen_s( &destFile, tmpDestFileName.c_str(), "w" );
#else	CONVERSION_VS
	destFile = fopen( tmpDestFileName.c_str(), "w" );
#endif	CONVERSION_VS
		
	if( NULL == destFile )
	{
		fclose( srcFile );
		return false;
	}


	fputc( 0xEF, destFile );
	fputc( 0xBB, destFile );
	fputc( 0xBF, destFile );

	//WCHAR wszHeader[MAX_PATH] = L"-- lua header. UTF-8 인코딩 인식을 위해 이 줄은 지우지 마세요.\r\n\r\n";
	//::MultiByteToWideChar(CP_UTF8,0,szHeader,-1,wszHeader,MAX_PATH);
	//fwrite( wszHeader, sizeof(WCHAR), wcslen(wszHeader), destFile );

	while( false == feof( srcFile ) )
	{
		sizeRead = fread( szBuf, sizeof(char), ARRAY_SIZE(szBuf), srcFile );
		if( sizeRead <= 0 )
			break;

		sizeWrote = fwrite( szBuf, sizeof(char), sizeRead, destFile );
		ASSERT( sizeRead == sizeWrote );
	}

	fclose( srcFile );
	fclose( destFile );


	if( true == bIsidentical )
	{
		remove( srcFullPath.c_str() );
		//DeleteFileA( srcFullPath.c_str() );
		rename( tmpDestFileName.c_str(), destFullPath.c_str() );
	}

	return true;
}






inline bool DecomposePathAndFileName( WCHAR* wszPath, WCHAR* wszFileName, const WCHAR* wszFullPath )
{
	if( NULL == wszFullPath )
		return false;

	WCHAR drive[_MAX_DRIVE] = L"";
	WCHAR dir[_MAX_DIR] = L"";
	WCHAR fname[_MAX_FNAME] = L"";
	WCHAR ext[_MAX_EXT] = L"";

#ifdef	CONVERSION_VS
	_wsplitpath_s( wszFullPath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT );

	if( NULL != wszFileName )
	{
		wcscat_s( wszFileName, _MAX_FNAME, fname );
		wcscat_s( wszFileName, _MAX_FNAME, ext );	
	}
	if( NULL != wszPath )
	{
		wcscat_s( wszPath, _MAX_DIR, drive );
		wcscat_s( wszPath, _MAX_DIR, dir );
	}
#else	CONVERSION_VS
	_wsplitpath( wszFullPath, drive, dir, fname, ext );

	if( NULL != wszFileName )
	{
		wcscat( wszFileName, fname );
		wcscat( wszFileName, ext );	
	}
	if( NULL != wszPath )
	{
		wcscat( wszPath, drive );
		wcscat( wszPath, dir );
	}
#endif	CONVERSION_VS
	
	return true;
}

inline void CompulsorybyConvertFileAnsiToUTF8A( string strFullPath )
{
	FILE* srcFile = NULL;

#ifdef	CONVERSION_VS
	fopen_s( &srcFile, strFullPath.c_str(), "r" );
#else	CONVERSION_VS
	srcFile = fopen( strFullPath.c_str(), "r" );
#endif	CONVERSION_VS	

	if( NULL == srcFile )
		return;

	UCHAR szBuf[1024] = "";
	size_t sizeRead = 0;

	sizeRead = fread( szBuf, sizeof(char), 3, srcFile );

	fclose( srcFile );

	if( sizeRead >= 3 )
	{
		if( szBuf[0] == 0xEF && szBuf[1] == 0xBB && szBuf[2] == 0xBF )
		{
			return;
		}
		else
		{
			::MessageBoxA( DXUTGetHWND(), "ANSI 파일이라 UTF 로 변환 됩니다.", strFullPath.c_str(), MB_ICONWARNING | MB_OK );

			if( false == ConvertFullPathFileAnsiToUTF8( strFullPath, strFullPath ) )
			{
				::MessageBoxA( DXUTGetHWND(), "UTF 변환 실패.!", strFullPath.c_str(), MB_ICONWARNING | MB_OK ); 
			}
		}
	}
}

inline void CompulsorybyConvertFileAnsiToUTF8W( wstring wstrFullPath )
{
	string strFileName;
	ConvertWCHARToChar( strFileName, wstrFullPath );

	CompulsorybyConvertFileAnsiToUTF8A( strFileName );
}

// kimhc // 1.05 (5%), 0.95 (-5%) 등의 증가율을 0.5, -0.5 등으로 반환 해주는 함수
inline float CalculateIncreasingRate( float fRate )
{
	const float STANDARD_RATE = 1.f;
	const float EPSILON = 0.00005f;
	return fRate - STANDARD_RATE + EPSILON;
}

// JHKang / 강정훈 / 2011.6.26 / Int64 형식의 날짜를 스트링으로 바꿀 경우 사용
inline wstring GetInt64ToDateDesc( __int64 iTime, __time64_t currentTime, bool bShowStrID309 = true )
{
	wstring tempDesc = L"";


	if ( bShowStrID309 == true )
		tempDesc = GET_STRING( STR_ID_309 ); //tempDesc = L"만료 시간 : ";

	wstringstream wstrstm;
	
	if ( bShowStrID309 == true )
	{
		wstrstm.str( L"" );
		wstrstm << L"\n";
	}

	CTime t0(iTime);
	__time64_t t1 = currentTime;

	double fDiffTime = difftime( KncUtil::TimeToInt( t0 ), (time_t)t1 );
	
	int remainDay =  (int)(fDiffTime / ( 60 * 60 * 24 ) );
	if ( remainDay > 0 )
		wstrstm << remainDay << GET_STRING( STR_ID_14 ) << L" ";

	int remainHour = (int)((time_t)fDiffTime % ( 60 * 60 * 24 )) / (60 * 60);
	if ( remainHour < 0 )
		remainHour = 0;

	wstrstm << remainHour << GET_STRING( STR_ID_310 ) << L" ";

	int remainMinute = (int)((time_t)fDiffTime % ( 60 * 60 )) / (60 );

	if ( remainDay <= 0 )
	{
		if ( remainMinute <= 0 )
			remainMinute = 0;
		wstrstm << remainMinute << GET_STRING( STR_ID_311 ) << L" ";
	}

	tempDesc += wstrstm.str().c_str();

	return tempDesc;
}

inline bool IsSamef( float a, float b = 0.f )
{
	if( fabs( a - b ) > FLT_EPSILON )
		return false;
	return true;
}

inline bool DisplayErrorMessage( const WCHAR* wszError_ )
{
#if defined( EFFECT_TOOL ) && defined( NDEBUG )
	return false;
#endif //EFFECT_TOOL

#ifndef	_SERVICE_
	StateLog( wszError_ );

#if defined(_DEBUG )
	ASSERT( NULL );
	MessageBox( NULL, wszError_, wszError_, MB_OK );
#else
	MessageBox( NULL, wszError_, wszError_, MB_OK );
#endif

#endif	_SERVICE_

	return false;
}

#ifdef	_SERViCE_
#define DISPLAY_ERROR( wszError_ ) DisplayErrorMessage( L"Error" );
#else	_SERViCE_
#define DISPLAY_ERROR( wszError_ ) DisplayErrorMessage( wszError_ );
#endif	_SERViCE_

#ifdef CHEAT_WCHART_TO_UTF8
inline void ConvertWCHARToUtf8( std::string& dest, const WCHAR* pSrc, const int iLength = -1 )
{
	KTDXPROFILE();

	dest.clear();
	if ( pSrc == NULL )
		return;

	int iSrcLength = iLength;
	if( -1 == iSrcLength )
	{
		iSrcLength = wcslen( pSrc );
		if ( iSrcLength <= 0 )
		{
			dest = "";
			return;
		}//if
	}

	int iDestSize = ::WideCharToMultiByte( CP_UTF8, 0, pSrc, iSrcLength, NULL, 0, NULL, NULL );
	if ( iDestSize <= 0 )
	{
		dest = "";
		return;
	}//if

	char* pszConv = (char*) _alloca( iDestSize * sizeof(char) );
	ASSERT( pszConv != NULL );
	int iNewSize = ::WideCharToMultiByte( CP_UTF8, 0, pSrc, iSrcLength, pszConv, iDestSize, NULL, NULL );
	ASSERT( iNewSize == iDestSize );
	dest.assign( pszConv, iNewSize );
}
#endif //CHEAT_WCHART_TO_UTF8