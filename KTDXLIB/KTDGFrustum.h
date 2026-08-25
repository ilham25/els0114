#pragma once

class CKTDGFrustum  
{
	public:
		CKTDGFrustum( LPDIRECT3DDEVICE9 pd3dDevice );
		virtual ~CKTDGFrustum();
		
		bool Construct( float ZDistance = 0.0f );
		__forceinline bool CheckPoint( float XPos, float YPos, float ZPos );
		__forceinline bool CheckCube( float XCenter, float YCenter, float ZCenter, float Size );
		__forceinline bool CheckRectangle( float XCenter, float YCenter, float ZCenter, float XSize, float YSize, float ZSize );
		__forceinline bool CheckSphere( float XCenter, float YCenter, float ZCenter, float Radius )
		{
			D3DXVECTOR3 vCenter(XCenter, YCenter, ZCenter);

			// Make sure radius is in frustum
			for( int i = 0; i < 6; i++ ) 
			{
				if( D3DXPlaneDotCoord( &m_Planes[i], &vCenter ) < -Radius)
					return false;
			}
            return true;
        }
        __forceinline bool CheckSphere( const D3DXVECTOR3& Center, float Radius )
        {
            return CheckSphere( Center.x, Center.y, Center.z, Radius );
		}

		__forceinline bool CheckSphere( float XCenter, float YCenter, float ZCenter, float Radius, float& fNeedDistance )
		{
			D3DXVECTOR3 vCenter(XCenter, YCenter, ZCenter);

			// Make sure radius is in frustum
			for( int i = 0; i < 6; i++ ) 
			{
				float fDistance = D3DXPlaneDotCoord(&m_Planes[i], &vCenter );
				if( fDistance < -Radius )
				{
					fNeedDistance = abs( fDistance ) - Radius;
					return false;
				}
			}

			fNeedDistance = 0;
			return true;
		}
		__forceinline bool CheckSphere( const D3DXVECTOR3& Center, float Radius, float& fNeedDistance )
		{
			return CheckSphere( Center.x, Center.y, Center.z, Radius, fNeedDistance );
		}
		
	private:
	    D3DXPLANE			m_Planes[6];

};
