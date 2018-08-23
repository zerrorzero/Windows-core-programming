#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <TlHelp32.h>
#include "howtoget.h"


int main()
{
	PROCESSENTRY32 pe64;
	pe64.dwSize = sizeof(pe64);

	EnableDebugPrivilege();

	HANDLE hprocessnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //get the process snapshot
	if (hprocessnap == INVALID_HANDLE_VALUE)
	{
		printf("Get Process Fail!");
		return -1;
	}

	printf("START:\n");

	int i = 0;

	if (!Process32First(hprocessnap,&pe64)) //check the first snapshot
	{
		printf("check process fail!");
		CloseHandle(hprocessnap);
		return -1;
	}

	
	do 
	{
		i++;
		MODULEENTRY32 me64;
		me64.dwSize = sizeof(me64);

		printf("[%d]: %S  %d\n", i, pe64.szExeFile, pe64.th32ProcessID);

		HANDLE hmousnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe64.th32ProcessID);
		if (hmousnap == INVALID_HANDLE_VALUE)
		{
			printf("[MODULE]:NO MODULE\n");
			CloseHandle(hmousnap);
			//continue;
		}
		else
		{
			if (!Module32First(hmousnap, &me64))
			{
				printf("check module fail!");
				CloseHandle(hmousnap);
				break;
			}
			int e = 0;
			do
			{
				e++;
				printf("%d. [MODULE]:%S\n   [MODULEPATH]:%S\n", e, me64.szModule, me64.szExePath);
			} while (Module32Next(hmousnap, &me64));
		}

		CloseHandle(hmousnap);

		HANDLE hProcnow = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, 0, pe64.th32ProcessID);
		if (!hProcnow)
		{
			DWORD errorinfo = GetLastError();
			printf("cannot get HANDLE  %d\n", errorinfo);
			continue;
		}
		else
		{
			GetProcessFullPath(hProcnow);
			UserNameForProc(hProcnow);
			GetCommandLine(hProcnow);
			CloseHandle(hProcnow);

			printf("=======================================================\r\n");
		}

	} while (Process32Next(hprocessnap,&pe64));
	system("pause");
	return 0;
}