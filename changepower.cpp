#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Aclapi.h> 
//#pragma comment (lib,"Advapi32.lib")

//Dynamic memory allocation

#define myheapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define myheapfree(x) (HeapFree(GetProcessHeap(), 0, x))

typedef BOOL(WINAPI *SetSecurityDescriptorControlFnPtr)(
	IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
	IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
	IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet);

typedef BOOL(WINAPI *AddAccessAllowedAceExFnPtr)(
	PACL pAcl,
	DWORD dwAceRevision,
	DWORD AceFlags,
	DWORD AccessMask,
	PSID pSid
	);

BOOL SystemAccessOnly(TCHAR *lpszFileName, TCHAR *lpszAccountName, DWORD dwAccessMask)
{
	SID_NAME_USE  snuType;

	// LookupAccountName's variable
	TCHAR *    szDomain = NULL;
	DWORD     cbDomain = 0;
	LPVOID     pUserSID = NULL;
	DWORD     cbUserSID = 0;

	// SD's variable
	PSECURITY_DESCRIPTOR pFileSD = NULL;
	DWORD     cbFileSD = 0;

	// a new SDm,include new and old ACL
	SECURITY_DESCRIPTOR newSD;

	// ACL's variable
	PACL      pACL = NULL;
	BOOL      fDaclPresent;
	BOOL      fDaclDefaulted;
	ACL_SIZE_INFORMATION AclInfo;

	// a new ACL
	PACL      pNewACL = NULL;
	DWORD     cbNewACL = 0;

	BOOL      fResult;
	BOOL      fAPISuccess;
	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	SetSecurityDescriptorControlFnPtr _SetSecurityDescriptorControl = NULL;
	AddAccessAllowedAceExFnPtr _AddAccessAllowedAceEx = NULL;

	__try {
		// get SID
		fAPISuccess = LookupAccountName(NULL, lpszAccountName,pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		if (fAPISuccess)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			printf("LookupAccountName() failed. Error %d\n",GetLastError());
			__leave;
		}

		pUserSID = myheapalloc(cbUserSID);
		if (!pUserSID)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;

		}

		szDomain = (TCHAR *)myheapalloc(cbDomain * sizeof(TCHAR));

		if (!szDomain)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;
		}

		fAPISuccess = LookupAccountName(NULL, lpszAccountName, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		if (!fAPISuccess) {
			printf("LookupAccountName() failed. Error %d\n",GetLastError());
			__leave;
		}

		// get file's SD

		fAPISuccess = GetFileSecurity(lpszFileName,
			secInfo, pFileSD, 0, &cbFileSD);

		if (fAPISuccess)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			printf("GetFileSecurity() failed. Error %d\n",GetLastError());
			__leave;
		}

		pFileSD = myheapalloc(cbFileSD);
		if (!pFileSD)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;
		}

		fAPISuccess = GetFileSecurity(lpszFileName, secInfo, pFileSD, cbFileSD, &cbFileSD);

		if (!fAPISuccess)
		{
			printf("GetFileSecurity() failed. Error %d\n",GetLastError());
			__leave;
		}

		// initialize a new SD
		if (!InitializeSecurityDescriptor(&newSD, SECURITY_DESCRIPTOR_REVISION))
		{
			printf("InitializeSecurityDescriptor() failed.""Error %d\n", GetLastError());
			__leave;
		}

		// get present DACL from SD

		if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL, &fDaclDefaulted))
		{
			printf("GetSecurityDescriptorDacl() failed. Error %d\n",GetLastError());
			__leave;
		}

		// get ACL info
		AclInfo.AceCount = 0; // Assume NULL DACL.
		AclInfo.AclBytesFree = 0;
		AclInfo.AclBytesInUse = sizeof(ACL);

		if (pACL == NULL)
			fDaclPresent = FALSE;

		if (fDaclPresent)
		{
			if (!GetAclInformation(pACL, &AclInfo,sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
			{
				printf("GetAclInformation() failed. Error %d\n",GetLastError());
				__leave;
			}

		}

		// create a new ACL
		cbNewACL = AclInfo.AclBytesInUse;
		pNewACL = (PACL)myheapalloc(cbNewACL);

		if (!pNewACL)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;
		}

		// initialize new ACL
		if (!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2))
		{
			_tprintf(TEXT("InitializeAcl() failed. Error %d\n"),
				GetLastError());
			__leave;
		}

		// add the new ACE
		_AddAccessAllowedAceEx = (AddAccessAllowedAceExFnPtr)GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")), "AddAccessAllowedAceEx");

		if (_AddAccessAllowedAceEx)
		{
			if (!_AddAccessAllowedAceEx(pNewACL, ACL_REVISION2, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE, dwAccessMask, pUserSID))
			{
				printf("AddAccessAllowedAceEx() failed. Error %d\n",GetLastError());
				__leave;
			}
		}
		else
		{
			if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, dwAccessMask, pUserSID))
			{
				printf("AddAccessAllowedAce() failed. Error %d\n",GetLastError());
				__leave;
			}
		}

		

		//  add the new ACL to SD.
		if (!SetSecurityDescriptorDacl(&newSD, TRUE, pNewACL, FALSE))
		{
			printf("SetSecurityDescriptorDacl() failed. Error %d\n",GetLastError());
			__leave;
		}

		// set SD control
		_SetSecurityDescriptorControl = (SetSecurityDescriptorControlFnPtr)GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")), "SetSecurityDescriptorControl");
		if (_SetSecurityDescriptorControl)
		{
			SECURITY_DESCRIPTOR_CONTROL controlBitsOfInterest;
			SECURITY_DESCRIPTOR_CONTROL controlBitsToSet;

			controlBitsOfInterest = SE_DACL_AUTO_INHERIT_REQ& SE_DACL_AUTO_INHERITED & SE_DACL_PROTECTED&SE_DACL_PRESENT;
			controlBitsToSet = controlBitsOfInterest;

			if (controlBitsOfInterest)
			{
				if (!_SetSecurityDescriptorControl(&newSD, controlBitsOfInterest, controlBitsToSet))
				{
					printf("SetSecurityDescriptorControl() failed.""Error %d\n", GetLastError());
					__leave;
				}
			}
		}

		//add SD to file
		if (!SetFileSecurity(lpszFileName, secInfo, &newSD))
		{
			printf("SetFileSecurity() failed. Error %d\n",GetLastError());
			__leave;
		}
		fResult = TRUE;
	}
	__finally {

		if (pUserSID) myheapfree(pUserSID);
		if (szDomain) myheapfree(szDomain);
		if (pFileSD) myheapfree(pFileSD);
		if (pNewACL) myheapfree(pNewACL);
	}
	return fResult;
}


BOOL AddAceToService(TCHAR *lpszServiceName, TCHAR *lpszAccountName, DWORD dwAccessMask)
{
	SE_OBJECT_TYPE ObjectType = SE_SERVICE; // type of object

	DWORD dwRes = 0;
	PACL pOldDACL = NULL, pNewDACL = NULL;
	PSECURITY_DESCRIPTOR pServicesSD = NULL;
	EXPLICIT_ACCESS ea;
	
	SID_NAME_USE  snuType;

	// LookupAccountName's variable
	TCHAR *    szDomain = NULL;
	DWORD     cbDomain = 0;
	LPSTR     pUserSID = NULL;
	DWORD     cbUserSID = 0;
	BOOL      fAPISuccess;
	BOOL      fResult;
	__try
	{

		if (NULL == lpszServiceName)
			return ERROR_INVALID_PARAMETER;

		// get SID
		fAPISuccess = LookupAccountName(NULL, lpszAccountName, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		if (fAPISuccess)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			printf("LookupAccountName() failed. Error %d\n", GetLastError());
			__leave;
		}

		pUserSID = (LPSTR)myheapalloc(cbUserSID);
		if (!pUserSID)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;

		}

		szDomain = (TCHAR *)myheapalloc(cbDomain * sizeof(TCHAR));

		if (!szDomain)
		{
			printf("HeapAlloc() failed. Error %d\n", GetLastError());
			__leave;
		}

		fAPISuccess = LookupAccountName(NULL, lpszAccountName, pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		if (!fAPISuccess) {
			printf("LookupAccountName() failed. Error %d\n", GetLastError());
			__leave;
		}

		// Get a pointer to the existing DACL.
		dwRes = GetNamedSecurityInfo(lpszServiceName, ObjectType,DACL_SECURITY_INFORMATION,NULL, NULL, &pOldDACL, NULL, &pServicesSD);
		
		if (ERROR_SUCCESS != dwRes)
		{
			printf("GetNamedSecurityInfo() failed. Error %u\n", dwRes);
			__leave;
		}

		// Initialize an EXPLICIT_ACCESS structure for the new ACE. 
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = dwAccessMask;
		ea.grfAccessMode = GRANT_ACCESS;
		ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.ptstrName = pUserSID;

		// Create a new ACL that merges the new ACE into the existing DACL
		dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
		if (ERROR_SUCCESS != dwRes)  
		{
			printf("SetEntriesInAcl() failed. Error %u\n", dwRes);
			__leave;
		}

		// Attach the new ACL as the object's DACL.
		dwRes = SetNamedSecurityInfo(lpszServiceName, ObjectType,DACL_SECURITY_INFORMATION,NULL, NULL, pNewDACL, NULL);
		
		if (ERROR_SUCCESS != dwRes) 
		{
			printf("SetNamedSecurityInfo() failed. Error %u\n", dwRes);
			__leave;
		}
		fResult = TRUE;
	}

	__finally
	{

		if (pServicesSD)
			LocalFree((HLOCAL)pServicesSD);
		if (pNewDACL)
			LocalFree((HLOCAL)pNewDACL);
		if (pUserSID)
			myheapfree(pUserSID);
		if (szDomain)
			myheapfree(szDomain);
	}
	return fResult;
}



int main(int argc, TCHAR *argv[])
{
	TCHAR *AccountName = "system";

	if (argc == 2)
	{
		if (!SystemAccessOnly(argv[1], AccountName, GENERIC_ALL))
		{
			printf("AddAccessRights() failed.\n");
			return 1;
		}
		else
		{
			printf("AddAccessRights() succeeded.\n");
			return 0;
		}
	}
	else if (argc==3)
	{
		if (!AddAceToService(argv[1], argv[2], GENERIC_ALL))
		{
			printf("AddAccessRights() failed.\n");
			return 1;
		}
		else
		{
			printf("AddAccessRights() succeeded.\n");
			return 0;
		}
	}
	
	else
	{
		printf("usage: \"%s\" <FileName> \n", argv[0]);
		printf("or usage: \"%s\" <ServiceName> <AccountName>\n", argv[0]);
		return 1;
	}


	
}