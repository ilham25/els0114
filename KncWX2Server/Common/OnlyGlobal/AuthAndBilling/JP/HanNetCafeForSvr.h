/*
 * COPYRIGHT(C) 2010 NHN Japan Corp. ALL RIGHTS RESERVED.
 */
#ifndef HanNetCafeForSvr_h
#define HanNetCafeForSvr_h


#ifdef HANNETCAFEFORSVR_EXPORTS
#define HANNETCAFEFORSVR_API __declspec(dllexport)
#else
#define HANNETCAFEFORSVR_API __declspec(dllimport)
#endif



// defined error code
#define		HAN_NETCAFE_OK							0
#define		HAN_NETCAFE_OK_NOT_NETCAFE_IP			-1
#define		HAN_NETCAFE_E_NOT_INITIALIZED			-2
#define		HAN_NETCAFE_E_ALREADY_INITIALIZED		-3
#define		HAN_NETCAFE_E_INSUFFICIENT_BUFFER		-4

#define		HAN_NETCAFE_E_LOAD_CONFIG				-10
#define		HAN_NETCAFE_E_INITIALZATION				-11
#define		HAN_NETCAFE_E_RETRIEVE_INFORM			-12
#define		HAN_NETCAFE_E_NOT_REGISTERED_USER		-13

#define		HAN_NETCAFE_E_INTERNET_OPEN				-20
#define		HAN_NETCAFE_E_INTERNET_CONNECT			-21
#define		HAN_NETCAFE_E_INTERNET_OPEN_REQUEST		-22
#define		HAN_NETCAFE_E_INTERNET_SEND_REQUEST		-23
#define		HAN_NETCAFE_E_INTERNET_RECEIVE_DATA		-24

#define		HAN_NETCAFE_E_UNKNOWN					-99





// interface
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_Init();
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_Cleanup();
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_User_Login(/*IN*/ const char* szUserID, /*IN*/ const char* szIP);
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_User_Logout(/*IN*/ const char* szUserID);
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_Get_NetCafe_Code(/*IN*/ const char* szUserID, /*OUT*/ char* szOutBuffer, /*IN*/ int nSize);
HANNETCAFEFORSVR_API int __stdcall HanNetCafe_Get_Product_Code(/*IN*/ const char* szUserID, /*OUT*/ char* szOutBuffer, /*IN*/ int nSize);


#endif	// HanNetCafeForSvr_h