#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <tchar.h>
#include <string.h>
#include <winternl.h>
#pragma comment(lib,"Psapi.lib")




BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath)
{	TCHAR szDriveStr[500];	TCHAR szDrive[3];	TCHAR szDevName[100];	int cchDevName;	int i;
	if (!pszDosPath || !pszNtPath)
	{
		printf("input pszDosPath or pszNtPath!\n");
		return FALSE;	}
	if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))  //get c:\ or d:\ ...
	{
		for (i = 0; szDriveStr[i]; i += 4)  //each one drive need 4 byte to save
		{
			if (!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))
				continue;
			/*QueryDosDevice need c: instead of c:\ or c:\\ */
			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';
			if (!QueryDosDevice(szDrive, szDevName, 100)) //change the szDrive(c:) to the DosDrive
			{				printf("QueryDosDevice fail!\n");				return FALSE;			}
			cchDevName = lstrlen(szDevName);
			if (_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)
			{
				lstrcpy(pszNtPath, szDrive);
				lstrcat(pszNtPath, pszDosPath + cchDevName);
				return TRUE;
			}
		}	}
	else
	{		lstrcpy(pszNtPath, pszDosPath);
		return FALSE;	}
}



BOOL GetProcessFullPath(HANDLE hProcess)
{	TCHAR		szImagePath[MAX_PATH];
	TCHAR		pszFullPath[MAX_PATH];
	if (!pszFullPath)
		return FALSE;
	pszFullPath[0] = '\0';
	
	if (hProcess == NULL)
		return FALSE;	
	if (!GetProcessImageFileName(hProcess, szImagePath, MAX_PATH))  //get the dos path
	{
		DWORD errorinfo = GetLastError();
		printf("filenameERROR = %d\n", errorinfo);
		CloseHandle(hProcess);
		return FALSE;
	}
	if (!DosPathToNtPath(szImagePath, pszFullPath))
	{
		printf("DosPath to NtPath fail\n");
		CloseHandle(hProcess);
		return FALSE;
	}
	printf(("[PATH]:%S \n"), pszFullPath);
	return TRUE;
}




BOOL Is64bitSystem()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		return TRUE;
	}
	else
	{		return FALSE;
	}
}





BOOL Is64bitProc(HANDLE hProcess)
{
	if (hProcess == NULL)
		return FALSE;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	BOOL blswow64 = FALSE;

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")), "IsWow64Process");
	if (Is64bitSystem() == TRUE)
	{
		if (fnIsWow64Process != NULL)  //iswow64process only in 64bit system
		{
			if (fnIsWow64Process(hProcess, &blswow64))  //if blswow64 is TRUE , the process is 32bit.
			{
				if (blswow64 == TRUE)
				{
					//printf("run 32bit in 64bit system\n");
					return FALSE;
				}
				else
				{
					//printf("run 64bit in 64bit system\n");
					return TRUE;
				}
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		//printf("32bit system\n");
		return FALSE;
	}
}





BOOL UserNameForProc(HANDLE hProcess)
{
	HANDLE hToken;
	if (hProcess==NULL)
		return FALSE;
	BOOL bRet = OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
	if (FALSE == bRet)
	{
		printf("OpenProcessToken is failed\n");
	}
	TCHAR tkUser[MAX_PATH];
	DWORD dwRetLen;
	bRet = GetTokenInformation(hToken, TokenUser, NULL, 0, &dwRetLen);  //get the len to dwRetlen
	PTOKEN_USER pToken = new TOKEN_USER[dwRetLen];
	bRet = GetTokenInformation(hToken, TokenUser, pToken, dwRetLen, &dwRetLen);
	TCHAR szUserName[MAX_PATH];
	DWORD dwUserNameSize;
	TCHAR szDomainName[MAX_PATH];
	DWORD dwDomainNameSize;
	SID_NAME_USE  sidname;
	if (!LookupAccountSid(NULL, pToken->User.Sid,
		szUserName, &dwUserNameSize,
		szDomainName, &dwDomainNameSize,
		&sidname))	{		printf("LookUpAccountSid fail!\n");		return FALSE;	}	else
	{
		printf("[USERNAME]:%S\n", szUserName);
		printf("[DOMAINNAME]:%S\n", szDomainName);
	}	CloseHandle(hToken);	return TRUE;
}





BOOL  EnableDebugPrivilege()
{
	HANDLE hToken;
	BOOL fOk = FALSE;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //Get Token
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))//Get Luid
			printf("Can't lookup privilege value.\n");

		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;//change the privilege to SE_PRIVILEGE_ENABLED
		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))//Adjust Token
			printf("Can't adjust privilege value.\n");
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOk;
}



// NtQueryInformationProcess for pure 32 and 64-bit processes !!Designate this proc!!
typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
	IN HANDLE ProcessHandle,
	ULONG ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

typedef NTSTATUS(NTAPI *_NtReadVirtualMemory)(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress,
	OUT PVOID Buffer,
	IN SIZE_T Size,
	OUT PSIZE_T NumberOfBytesRead);

// NtQueryInformationProcess for 32-bit process on WOW64  !!Designate this proc!!
typedef NTSTATUS(NTAPI *_NtWow64ReadVirtualMemory64)(
	IN HANDLE ProcessHandle,
	IN PVOID64 BaseAddress,
	OUT PVOID Buffer,
	IN ULONG64 Size,
	OUT PULONG64 NumberOfBytesRead);


// PROCESS_BASIC_INFORMATION for 32-bit process on WOW64
// The definition is quite funky, as we just lazily doubled sizes to match offsets...
typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
	PVOID Reserved1[2];
	PVOID64 PebBaseAddress;
	PVOID Reserved2[4];
	ULONG_PTR UniqueProcessId[2];
	PVOID Reserved3[2];
} PROCESS_BASIC_INFORMATION_WOW64;



typedef struct _UNICODE_STRING_WOW64 {
	USHORT Length;
	USHORT MaximumLength;
	PVOID64 Buffer;
} UNICODE_STRING_WOW64;


BOOL GetCommandLine(HANDLE hProcess)
{
	DWORD err = 0;

	// use WinDbg "dt ntdll!_PEB" command and search for ProcessParameters offset to find the truth out
	DWORD ProcessParametersOffset = Is64bitProc(hProcess) == TRUE ? 0x20 : 0x10;  //0x20 = 32byte 0x10 = 16byte
	DWORD CommandLineOffset = Is64bitProc(hProcess) == TRUE ? 0x70 : 0x40;  //0x70 = 112byte  0x40 =64byte 

	// read basic info to get ProcessParameters address, we only need the beginning of PEB
	DWORD pebSize = ProcessParametersOffset + 8;
	PBYTE peb = (PBYTE)malloc(pebSize);
	ZeroMemory(peb, pebSize);

	// read basic info to get CommandLine address, we only need the beginning of ProcessParameters
	DWORD ppSize = CommandLineOffset + 16;
	PBYTE pp = (PBYTE)malloc(ppSize);
	ZeroMemory(pp, ppSize);

	PWSTR cmdLine;

	if (Is64bitProc(hProcess)==TRUE)
	{
		printf("64bit process\n");

		// we're running as a 32-bit process in a 64-bit OS  !!Designate this proc!!
		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information from 64-bit world
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
		err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
		if (err != 0)
		{
			printf("NtWow64QueryInformationProcess64 failed\n");
			return FALSE;
		}

		// read PEB from 64-bit address space
		_NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
		err = read(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 PEB failed\n");
			return FALSE;
		}

		// read ProcessParameters from 64-bit address space
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
		err = read(hProcess, parameters, pp, ppSize, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 Parameters failed\n");
			return FALSE;
		}

		// read CommandLine
		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(pp + CommandLineOffset);
		cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
		err = read(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 Parameters failed\n");
			return FALSE;
		}
	}
	else
	{
		printf("32bit process\n");

		// we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS  !!Designate this proc!!
		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
		if (err != 0)
		{
			printf("NtQueryInformationProcess failed\n");
			return FALSE;
		}

		// read PEB
		if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL))
		{
			printf("ReadProcessMemory PEB failed\n");
			return FALSE;
		}

		// read ProcessParameters
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
		if (!ReadProcessMemory(hProcess, parameters, pp, ppSize, NULL))
		{
			printf("ReadProcessMemory Parameters failed\n");
			return FALSE;
		}

		// read CommandLine
		UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(pp + CommandLineOffset);
		cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
		if (!ReadProcessMemory(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL))
		{
			printf("ReadProcessMemory Parameters failed\n");
			return FALSE;
		}
	}
	printf("[CommandLine]%S\n\n", cmdLine);
	return TRUE;
}