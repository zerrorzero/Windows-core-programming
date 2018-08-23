/*typedef struct tagPROCESSENTRY32
{
DWORD dwSize;
DWORD cntUsage;
DWORD th32ProcessID;  //PID
ULONG_PTR th32DefaultHeapID;
DWORD th32ModuleID;
DWORD cntThreads;
DWORD th32ParentProcessID;
LONG pcPriClassBase;
DWORD dwFlags;
TCHAR szExeFile[MAX_PATH];  //PROCESSNAME
} PROCESSENTRY32, *PPROCESSENTRY32;
typedef struct _PEB {
	BYTE                          Reserved1[2];   //2byte
	BYTE                          BeingDebugged;  //1byte
	BYTE                          Reserved2[1];   //1byte
	PVOID                         Reserved3[2];   //2*4=8byte
	PPEB_LDR_DATA                 Ldr;            //4byte
	PRTL_USER_PROCESS_PARAMETERS  ProcessParameters; //the processparameters like command line (we need it)
	BYTE                          Reserved4[104];
	PVOID                         Reserved5[52];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE                          Reserved6[128];
	PVOID                         Reserved7[1];
	ULONG                         SessionId;
} PEB, *PPEB;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE           Reserved1[16]; //16byte
	PVOID          Reserved2[10]; //10*4=40byte
	UNICODE_STRING ImagePathName; //2+2+4byte
	UNICODE_STRING CommandLine;   //2+2+4byte，the last 4byte is *commandline
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;


typedef struct _UNICODE_STRING {
USHORT Length;
USHORT MaximumLength;
PWSTR  Buffer;
} UNICODE_STRING;


// PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes
typedef struct _PROCESS_BASIC_INFORMATION {
PVOID Reserved1;
PVOID PebBaseAddress;
PVOID Reserved2[2];
ULONG_PTR UniqueProcessId;
PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

