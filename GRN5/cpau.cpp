#include "nowarnings.h"
#include <windows.h>
#include <stdio.h>
#include <userenv.h>

//#pragma hdrstop

LPWSTR AnsiToUnicode( LPCSTR lpAnsiStr );
LPWSTR GetProcDirectory(LPSTR Full);

static BOOL
getAndAllocateLogonSid(
	HANDLE hToken,
	PSID *pLogonSid
)
{
	PTOKEN_GROUPS	ptgGroups = NULL;
	DWORD			cbBuffer  = 0;  	/* allocation size */
	DWORD			dwSidLength;		/* required size to hold Sid */
	UINT			i;					/* Sid index counter */
	BOOL			bSuccess  = FALSE;	/* assume this function will fail */

	*pLogonSid = NULL; // invalidate pointer

	/*
	** Get neccessary memory allocation
	*/
	GetTokenInformation(hToken, TokenGroups, ptgGroups, cbBuffer, &cbBuffer);

	if (cbBuffer)
		ptgGroups = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuffer);

	/*
	**	Get Sids for all groups the user belongs to
	*/
	bSuccess = GetTokenInformation(
					hToken,
					TokenGroups,
					ptgGroups,
					cbBuffer,
					&cbBuffer
				);
	if (bSuccess == FALSE)
		goto finish3;

	/*
	** Get the logon Sid by looping through the Sids in the token
	*/
	for(i = 0 ; i < ptgGroups->GroupCount ; i++)
	{
		if (ptgGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID)
		{
			/*
			** insure we are dealing with a valid Sid
			*/
			bSuccess = IsValidSid(ptgGroups->Groups[i].Sid);
			if (bSuccess == FALSE)
				goto finish3;

			/*
			** get required allocation size to copy the Sid
			*/
			dwSidLength=GetLengthSid(ptgGroups->Groups[i].Sid);

			/*
			** allocate storage for the Logon Sid
			*/
			if(
				(*pLogonSid = (PSID)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, dwSidLength))
				==
				NULL
			)
			{
				bSuccess = FALSE;
				goto finish3;
			}

			/*
			** copy the Logon Sid to the storage we just allocated
			*/
			bSuccess = CopySid(
							dwSidLength,
							*pLogonSid,
							ptgGroups->Groups[i].Sid
						);

			break;
		}
	}


finish3:
	/*
	** free allocated resources
	*/
	if (bSuccess == FALSE)
	{
		if(*pLogonSid != NULL)
		{
			HeapFree(GetProcessHeap(), 0, *pLogonSid);
			*pLogonSid = NULL;
		}
	}

	if (ptgGroups != NULL)
		HeapFree(GetProcessHeap(), 0, ptgGroups);

	return bSuccess;
}
 
 
static BOOL
setSidOnAcl(
	PSID pSid,
	PACL pAclSource,
	PACL *pAclDestination,
	DWORD AccessMask,
	BOOL bAddSid,
	BOOL bFreeOldAcl
)
{
	ACL_SIZE_INFORMATION	AclInfo;
	DWORD					dwNewAclSize;
	LPVOID					pAce;
	DWORD					AceCounter;
	BOOL					bSuccess=FALSE;

	/*
	** If we were given a NULL Acl, just provide a NULL Acl
	*/
	if (pAclSource == NULL)
	{
		*pAclDestination = NULL;
		return TRUE;
	}

	if (!IsValidSid(pSid)) return FALSE;

	/*
	**	Get ACL's parameters
	*/
	if (
		!GetAclInformation(
			pAclSource,
			&AclInfo,
			sizeof(ACL_SIZE_INFORMATION),
			AclSizeInformation
		)
	)
		return FALSE;

	/*
	**	Compute size for new ACL, based on
	**	addition or subtraction of ACE
	*/
	if (bAddSid)
	{
		dwNewAclSize = AclInfo.AclBytesInUse  +
							sizeof(ACCESS_ALLOWED_ACE)  +
							GetLengthSid(pSid)          -
							sizeof(DWORD)               ;
	}
	else
	{
		dwNewAclSize = AclInfo.AclBytesInUse  -
							sizeof(ACCESS_ALLOWED_ACE)  -
							GetLengthSid(pSid)          +
							sizeof(DWORD)               ;
	}

	*pAclDestination = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewAclSize);
	if(*pAclDestination == NULL)
		return FALSE;

	/*
	** initialize new Acl
	*/
	bSuccess = InitializeAcl(*pAclDestination, dwNewAclSize, ACL_REVISION);
	if (bSuccess == FALSE)
		goto finish5;

	/*
	** copy existing ACEs to new ACL
	*/
	for(AceCounter = 0 ; AceCounter < AclInfo.AceCount ; AceCounter++)
	{
		/*
		** fetch existing ace
		*/
		bSuccess = GetAce(pAclSource, AceCounter, &pAce);
		if (bSuccess == FALSE)
			goto finish5;

		/*
		** check to see if we are removing the ACE
		*/
		if (!bAddSid)
		{
			/*
			** we only care about ACCESS_ALLOWED ACEs
			*/
			if ((((PACE_HEADER)pAce)->AceType) == ACCESS_ALLOWED_ACE_TYPE)
			{
				PSID pTempSid=(PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;
				/*
				** if the Sid matches, skip adding this Sid
				*/
				if (EqualSid(pSid, pTempSid)) continue;
			}
		}

		/*
		** append ACE to ACL
		*/
		bSuccess = AddAce(
						*pAclDestination,
						ACL_REVISION,
						0,  // maintain Ace order
						pAce,
						((PACE_HEADER)pAce)->AceSize
					);
		if (bSuccess == FALSE)
			goto finish5;

	}

	/*
	** If appropriate, add ACE representing pSid
	*/
	if (bAddSid)
		bSuccess = AddAccessAllowedAce(
						*pAclDestination,
						ACL_REVISION,
						AccessMask,
						pSid
					);

finish5:
	/*
	** free memory if an error occurred
	*/
	if (!bSuccess)
	{
		if(*pAclDestination != NULL)
			HeapFree(GetProcessHeap(), 0, *pAclDestination);
	}
	else if (bFreeOldAcl)
		HeapFree(GetProcessHeap(), 0, pAclSource);

	return bSuccess;
}

static BOOL
setWinstaDesktopSecurity(
	HWINSTA hWinsta,
	HDESK hDesktop,
	PSID pLogonSid,
	BOOL bGrant,
	HANDLE hToken
)
{
	SECURITY_INFORMATION	si = DACL_SECURITY_INFORMATION;
	PSECURITY_DESCRIPTOR	sdDesktop = NULL;
	PSECURITY_DESCRIPTOR	sdWinsta = NULL;
	SECURITY_DESCRIPTOR		sdNewDesktop;
	SECURITY_DESCRIPTOR		sdNewWinsta;
	DWORD					sdDesktopLength	= 0;	/* allocation size */
	DWORD					sdWinstaLength	= 0;	/* allocation size */
	PACL					pDesktopDacl;		/* previous Dacl on Desktop */
	PACL					pWinstaDacl;        /* previous Dacl on Winsta */
	PACL					pNewDesktopDacl	= NULL;	/* new Dacl for Desktop */
	PACL					pNewWinstaDacl	= NULL;	/* new Dacl for Winsta */
	BOOL					bDesktopDaclPresent;
	BOOL					bWinstaDaclPresent;
	BOOL					bDaclDefaultDesktop;
	BOOL					bDaclDefaultWinsta;
	BOOL					bSuccess		= FALSE;
	PSID					pUserSid = NULL;

	/*
	** Obtain security descriptor for Desktop
	*/
	DWORD GLE=GetUserObjectSecurity(
		hDesktop,
		&si,
		sdDesktop,
		sdDesktopLength,
		&sdDesktopLength
	);
	GLE=GetLastError();
	if (sdDesktopLength)
		sdDesktop = (PSECURITY_DESCRIPTOR)HeapAlloc(
						GetProcessHeap(), HEAP_ZERO_MEMORY, sdDesktopLength);

	bSuccess = GetUserObjectSecurity(
		hDesktop,
		&si,
		sdDesktop,
		sdDesktopLength,
		&sdDesktopLength
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain security descriptor for Window station
	*/
	GetUserObjectSecurity(
		hWinsta,
		&si,
		sdWinsta,
		sdWinstaLength,
		&sdWinstaLength
	);

	if (sdWinstaLength)
		sdWinsta = (PSECURITY_DESCRIPTOR)HeapAlloc(
							GetProcessHeap(), HEAP_ZERO_MEMORY, sdWinstaLength);

	bSuccess = GetUserObjectSecurity(
		hWinsta,
		&si,
		sdWinsta,
		sdWinstaLength,
		&sdWinstaLength
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain DACL from security descriptor for desktop
	*/
	bSuccess = GetSecurityDescriptorDacl(
					sdDesktop,
					&bDesktopDaclPresent,
					&pDesktopDacl,
					&bDaclDefaultDesktop
				);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain DACL from security descriptor for Window station
	*/
	bSuccess = GetSecurityDescriptorDacl(
					sdWinsta,
					&bWinstaDaclPresent,
					&pWinstaDacl,
					&bDaclDefaultWinsta
				);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Create new DACL with Logon and User Sid for Desktop
	*/
	
	if(bDesktopDaclPresent) {
		bSuccess = setSidOnAcl(
			pLogonSid,
			pDesktopDacl,
			&pNewDesktopDacl,
			GENERIC_READ | GENERIC_WRITE | READ_CONTROL
			| DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW
			| DESKTOP_CREATEMENU | DESKTOP_SWITCHDESKTOP
			| DESKTOP_ENUMERATE,
			bGrant,
			FALSE
		);

		if (bSuccess == FALSE)
			goto finish4;
	}
	
	/*
	** Create new DACL with Logon and User Sid for Window station
	*/
	if(bWinstaDaclPresent)
	{
		bSuccess = setSidOnAcl(
						pLogonSid,
						pWinstaDacl,
						&pNewWinstaDacl,
						GENERIC_READ | GENERIC_WRITE | READ_CONTROL
						| WINSTA_ACCESSGLOBALATOMS
						| WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES
						| WINSTA_ACCESSCLIPBOARD | WINSTA_ENUMERATE
						| WINSTA_EXITWINDOWS,
						bGrant,
						FALSE
					);

		if (bSuccess == FALSE)
			goto finish4;
	}
 
	/*
	** Initialize the target security descriptor for Desktop
	*/
	if (bDesktopDaclPresent)
	{
		bSuccess = InitializeSecurityDescriptor(
						&sdNewDesktop,
						SECURITY_DESCRIPTOR_REVISION
					);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Initialize the target security descriptor for Window station
	*/
	if(bWinstaDaclPresent)
	{
		bSuccess = InitializeSecurityDescriptor(
						&sdNewWinsta,
						SECURITY_DESCRIPTOR_REVISION
					);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply new ACL to the Desktop security descriptor
	*/
	if(bDesktopDaclPresent)
	{
		bSuccess = SetSecurityDescriptorDacl(
						&sdNewDesktop,
						TRUE,
						pNewDesktopDacl,
						bDaclDefaultDesktop
					);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply new ACL to the Window station security descriptor
	*/
	if(bWinstaDaclPresent)
	{
		bSuccess = SetSecurityDescriptorDacl(
						&sdNewWinsta,
						TRUE,
						pNewWinstaDacl,
						bDaclDefaultWinsta
					);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply security descriptors with new DACLs to Desktop and Window station
	*/
	if (bDesktopDaclPresent)
	{
		bSuccess = SetUserObjectSecurity(
									hDesktop,
									&si,
									&sdNewDesktop
					);

		if (bSuccess == FALSE)
			goto finish4;
	}

	if(bWinstaDaclPresent)
		bSuccess = SetUserObjectSecurity(
									hWinsta,
									&si,
									&sdNewWinsta
					);

	if (bSuccess == FALSE)
		goto finish4;
finish4:
	if (sdDesktop != NULL)
		HeapFree(GetProcessHeap(), 0, sdDesktop);

	if (sdWinsta != NULL)
		HeapFree(GetProcessHeap(), 0, sdWinsta);

	if (pNewDesktopDacl != NULL)
		HeapFree(GetProcessHeap(), 0, pNewDesktopDacl);

	if (pNewWinstaDacl != NULL)
		HeapFree(GetProcessHeap(), 0, pNewWinstaDacl);

	return bSuccess;
}

static BOOL
allowDesktopAccess(HANDLE hToken)
{
	HWINSTA	hWinsta = NULL;
	HDESK	hDesktop;
	HDESK   hD2=NULL;
	PSID	pLogonSid = NULL;
	BOOL	ok = FALSE;

	if (!getAndAllocateLogonSid(hToken, &pLogonSid))
		return FALSE;

	hWinsta=GetProcessWindowStation();

	hDesktop=NULL;	
	DWORD RRR=0x01FF|WRITE_DAC|READ_CONTROL|WRITE_OWNER|DELETE;
	hDesktop=OpenDesktop("Default",DF_ALLOWOTHERACCOUNTHOOK,TRUE,RRR);
	
	//hDesktop=GetThreadDesktop(GetCurrentThreadId());

	if (hDesktop==NULL)
		return FALSE;
		
	ok=1;
	ok = SetHandleInformation(hDesktop, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	if (!ok)
		return FALSE;

	ok = SetHandleInformation(hWinsta, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	if (!ok)
		return FALSE;

	ok = setWinstaDesktopSecurity(hWinsta, hDesktop, pLogonSid, TRUE, hToken);

	CloseDesktop(hDesktop);

if(pLogonSid != NULL)
		HeapFree(GetProcessHeap(), 0, pLogonSid);

	return ok;
}

int CPAU(HANDLE hTokenCall, char *Desktop, char *user, char *domain, char *password, char *cmdline, STARTUPINFOW* psi, PROCESS_INFORMATION* ppi)
{
	int rz;
	HANDLE	hToken=NULL;
	HANDLE	hNewToken=NULL;;
	BOOL ok = FALSE;
	LPTSTR	lpCmd = NULL;
	char Buff[128];
	char FullDeskName[192];
	DWORD DWN;
	LPWSTR PWCmd=NULL;
	LPWSTR PWProcDir;
	
	PWCmd=AnsiToUnicode(cmdline);

	if(Desktop==NULL){//если имя десктопа не указано, то берём десктоп, принимающий ввод
		HDESK hopd=OpenInputDesktop(DF_ALLOWOTHERACCOUNTHOOK,1,DESKTOP_READOBJECTS);
		GetUserObjectInformation(hopd,UOI_NAME,Buff,128,&DWN);
		HWINSTA hwst=GetProcessWindowStation();
		GetUserObjectInformation(hwst,UOI_NAME,FullDeskName,192,&DWN);
		strcat(FullDeskName,"\\");
		strcat(FullDeskName,Buff);
		CloseDesktop(hopd);
		Desktop=FullDeskName;
	}

	//ZeroMemory(psi,sizeof(STARTUPINFOW));

	psi->cb = sizeof(STARTUPINFOW);
	//psi->wShowWindow = SW_SHOWNORMAL;
	psi->dwFlags=STARTF_FORCEOFFFEEDBACK|STARTF_USESHOWWINDOW;
	psi->lpDesktop =AnsiToUnicode(Desktop);

	
	PWProcDir=GetProcDirectory(cmdline);

	if(_stricmp(Desktop,"winsta0\\Winlogon")==0){
			user=(char*)"SYSTEM";//если имя десктопа Winlogon, то запускаем только под SYSTEM
			goto SYSRUN;
		}


if(hTokenCall!=NULL){
	DuplicateTokenEx(hTokenCall, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
           TokenPrimary, &hNewToken);
			goto ADA;
}
	
if(user!=NULL && strcmp(user,"SYSTEM")==0){
		OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hNewToken);
goto ENV;
}
	ok = LogonUser(
				user,
				domain,
				password,
				LOGON32_LOGON_INTERACTIVE,
				LOGON32_PROVIDER_DEFAULT,
				&hToken
		);
	if (ok == FALSE)
		goto finish2;


	DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
           TokenPrimary, &hNewToken);

ADA:
	
	ok = allowDesktopAccess(hNewToken);

	if (ok == FALSE)
		goto finish2;


ENV:
	
LPVOID lpEnvironment;
lpEnvironment=0;
CreateEnvironmentBlock(&lpEnvironment, hNewToken, false);

	ok = CreateProcessAsUserW(
					hNewToken,           //from hToken,
					NULL,
					PWCmd,               //from cmdline,
					NULL,
					NULL,
					TRUE,
					CREATE_UNICODE_ENVIRONMENT|CREATE_NEW_CONSOLE,
					lpEnvironment,
					NULL,//PWProcDir, временно убрана, т.к. GetProcDirectory работает некорректно, в ProcCmdFunc.cpp есть SetProcDirectory
					psi,
					ppi);


rz=VirtualFree(lpEnvironment,0,MEM_RELEASE);//т.к. DestroyEnvironmentBlock почему то не работает

close_hTh:


finish2:
	
	if (lpCmd != NULL)
		LocalFree(lpCmd);

	if (hToken != NULL)
		CloseHandle(hToken);
	
	if (hNewToken != NULL)
		CloseHandle(hNewToken);
	
	if (PWProcDir!=NULL)
		free(PWProcDir);

	if (PWCmd!=NULL)
		free(PWCmd);

	if (psi->lpDesktop!=NULL)
		free(psi->lpDesktop);

	return ok;


SYSRUN:

ok = CreateProcessW(NULL,PWCmd,NULL,NULL,0,0,NULL,PWProcDir,psi,ppi);

	goto close_hTh;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
LPWSTR AnsiToUnicode( LPCSTR lpAnsiStr )
{
    LPWSTR lpUnicodeStr =NULL;
    int iLen;
    // вычисляем длину будущей Unicode строки
    if( ( iLen =MultiByteToWideChar( CP_ACP, 0, lpAnsiStr, -1, NULL, 0 ) ) != 0 )
        // выделяем память под будущую Unicode строку
        if( ( lpUnicodeStr =(LPWSTR)malloc( iLen*sizeof(WCHAR) ) ) !=NULL )
            // кодируем существующую ANSI строку в Unicode
            if( MultiByteToWideChar( CP_ACP, 0, lpAnsiStr, -1, lpUnicodeStr, iLen ) == 0 )
            {
                // ОШИБКА
                free(lpUnicodeStr);
                lpUnicodeStr =NULL;
            }
 
    return lpUnicodeStr;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
LPWSTR GetProcDirectory(LPSTR Full){

int i, len;

char temp[384];

strcpy(temp,Full);
len=strlen(temp);
i=len-1;
	while(i>0){
		if(temp[i]==0x5C){
			break;
		}
	i--;	
	}
temp[i]=0;
	if(strlen(temp)==0){
	return 0;
	}
	
	return AnsiToUnicode(temp);
}

