// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>
#include <atlbase.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383


void RedirectIOToConsole()  //printf the result.

{
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
		coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT|_O_WTEXT);
	fp = _fdopen(hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	return;
}


void QueryServices(HKEY hKey, TCHAR *data_set)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 
	TCHAR    keyname[MAX_KEY_LENGTH];
	TCHAR    szDate[MAX_VALUE_NAME];
	TCHAR    szDate2[MAX_VALUE_NAME];
	TCHAR    szDate3[MAX_VALUE_NAME];
	CHAR    data_setbuf[MAX_KEY_LENGTH];
	TCHAR   databuf[MAX_KEY_LENGTH];
	DWORD    MaxValueDateLen;

	DWORD i, retCode, type;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	if (cSubKeys)
	{
		printf("\nNumber of subkeys: %d\n", cSubKeys);
		int a = 0;
		for (i = 0; i < cSubKeys; i++)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime); //GET achkeyname
			if (retCode == ERROR_SUCCESS)
			{

				sprintf(data_setbuf, "%s\\%s", data_set, achKey); //get subkey's path

				memcpy(databuf, data_setbuf, strlen(data_setbuf) + 2);

				HKEY hNextKey;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, databuf, 0, KEY_READ, &hNextKey) == ERROR_SUCCESS) //get subkey's value
				{

					RegQueryValueEx(hNextKey, TEXT("DisplayName"), NULL, &type, (LPBYTE)szDate2, &MaxValueDateLen);
					MaxValueDateLen = MAX_VALUE_NAME;  //because  &MaxValueDateLen is _IN_OUT

					RegQueryValueEx(hNextKey, TEXT("ImagePath"), NULL, &type, (LPBYTE)szDate3, &MaxValueDateLen);
					MaxValueDateLen = MAX_VALUE_NAME;

					RegQueryValueEx(hNextKey, TEXT("Start"), NULL, &type, (LPBYTE)szDate, &MaxValueDateLen);
					MaxValueDateLen = MAX_VALUE_NAME;


					if (*szDate == 2)  //in the key "Start",2 mean autostart
					{
						printf("achkey:(%d) %s\n", i + 1, achKey);
						printf("DisplayName:%s\n", szDate2);
						printf("ImagePath:%s\n", szDate3);
						a++;
					}
					RegCloseKey(hNextKey);
				}
				else
				{
					printf("Queryservices ERRROR!!");
					return;
				}
			}
			else
				return;
		}
		printf("Automatic Start Services Total:[%d]\n", a);
	}
	return;
}



void QueryKey(HKEY hKey, TCHAR *data_set)
{
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys = 0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 
	TCHAR    keyname[MAX_KEY_LENGTH];
	TCHAR    szDate[MAX_VALUE_NAME];
	TCHAR    szDate2[MAX_VALUE_NAME];
	TCHAR    szDate3[MAX_VALUE_NAME];
	CHAR    data_setbuf[MAX_KEY_LENGTH];
	TCHAR   databuf[MAX_KEY_LENGTH];
	DWORD    MaxValueDateLen;

	DWORD i, retCode, type;

	TCHAR  achValue[MAX_VALUE_NAME];
	DWORD cchValue = MAX_VALUE_NAME;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	// Enumerate the subkeys, until RegEnumKeyEx fails.
	if (retCode != ERROR_SUCCESS)
	{
		printf("Input correct path");
		return;
	}

	if (cSubKeys)
	{
		printf("\n[Number of subkeys:] %d\n", cSubKeys);

		for (i = 0; i < cSubKeys; i++)
		{

			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
			if (retCode == ERROR_SUCCESS)
			{
				sprintf(data_setbuf, "%s\\%s", data_set, achKey);

				printf("achkey:<%d> %s\n", i + 1, data_setbuf);
				memcpy(databuf, data_setbuf, strlen(data_setbuf) + 2);

				HKEY hNextKey;
				if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, databuf, 0, KEY_READ, &hNextKey) == ERROR_SUCCESS)
				{

					QueryKey(hNextKey, databuf);
					RegCloseKey(hNextKey);
				}
				else
				{
					printf("Querykey ERRROR!!");
					return;
				}
			}
			else
				return;
		}
	}
	printf("This key haven't subkey!!\n");
	return;
}


BOOL GetKeyPath(LPSTR KeyPath, HKEY *index, TCHAR *data_set)
{

	char *indexbuf[MAX_KEY_LENGTH];
	int b = 0;
	if (KeyPath == NULL)
	{
		printf("input correct path!!");
		return FALSE;
	}


	char *p;
	char *buf[MAX_VALUE_NAME];
	const char *split = "\\";
	p = strtok(KeyPath, split);  //split HKLM\\Windows\\..\\.. like HKLM windows ..
	int a = 0;
	while (p != NULL)
	{

		buf[a] = p;
		a++;
		p = strtok(NULL, split);
	}
	*indexbuf = buf[0]; //get hkey

	char databuf[MAX_VALUE_NAME] = "";

	for (b = 1; b <= a - 1; b++)  // get path like windows\\..\\.. without hkey
	{

		strcat(databuf, buf[b]);
		if (b + 1 <= a - 1)
		{
			strcat(databuf, "\\");
		}
	}

	if (databuf != NULL)
	{
		memcpy(data_set, databuf, strlen(databuf) + 2);

	}
	else
	{
		printf("ERROR:cannot get the path!");
		return FALSE;
	}

	if (strcmp(*indexbuf, "HKEY_CLASSES_ROOT") == 0)
	{
		*index = HKEY_CLASSES_ROOT;

		return TRUE;
	}
	else if (strcmp(*indexbuf, "HKEY_CURRENT_USER") == 0)
	{
		*index = HKEY_CURRENT_USER;

		return TRUE;
	}
	else if (strcmp(*indexbuf, "HKEY_LOCAL_MACHINE") == 0)
	{
		*index = HKEY_LOCAL_MACHINE;
	
		return TRUE;
	}
	else if (strcmp(*indexbuf, "HKEY_USERS") == 0)
	{
		*index = HKEY_USERS;
	
		return TRUE;
	}
	else if (strcmp(*indexbuf, "HKEY_CURRENT_CONFIG") == 0)
	{
		*index = HKEY_CURRENT_CONFIG;
		
		return TRUE;
	}
	else
		return FALSE;

	return TRUE;
}


DWORD GetKeyType(LPSTR keytype)
{
	DWORD typebuf;
	if (strcmp(keytype, "REG_BINARY") == 0)
	{
		typebuf = REG_BINARY;

	}
	else if (strcmp(keytype, "REG_DWORD") == 0)
	{
		typebuf = REG_DWORD;

	}
	else if (strcmp(keytype, "REG_DWORD_LITTLE_ENDIAN") == 0)
	{
		typebuf = REG_DWORD_LITTLE_ENDIAN;

	}
	else if (strcmp(keytype, "REG_DWORD_BIG_ENDIAN") == 0)
	{
		typebuf = REG_DWORD_BIG_ENDIAN;

	}
	else if (strcmp(keytype, "REG_EXPAND_SZ") == 0)
	{
		typebuf = REG_EXPAND_SZ;

	}
	else if (strcmp(keytype, "REG_LINK") == 0)
	{
		typebuf = REG_LINK;

	}
	else if (strcmp(keytype, "REG_MULTI_SZ") == 0)
	{
		typebuf = REG_MULTI_SZ;

	}
	else if (strcmp(keytype, "REG_NONE") == 0)
	{
		typebuf = REG_NONE;

	}
	else if (strcmp(keytype, "REG_RESOURCE_LIST") == 0)
	{
		typebuf = REG_RESOURCE_LIST;

	}
	else if (strcmp(keytype, "REG_SZ") == 0)
	{
		typebuf = REG_SZ;

	}
	else
	{
		printf("Input correct type!!");
		return -1;
	}

	return typebuf;

}


BOOL RegAddValue(LPSTR keypath, LPCSTR achkey, LPSTR keytype, LPSTR keyvalue)
{
	HKEY hTestKey;
	HKEY index[MAX_KEY_LENGTH];
	TCHAR data_set[MAX_KEY_LENGTH];
	TCHAR data_buf[MAX_KEY_LENGTH] = {};
	DWORD typebuf;
	DWORD dwDisposition;
	DWORD dwOptions = REG_OPTION_NON_VOLATILE;
	TCHAR buf[MAX_KEY_LENGTH];

	DWORD valuebuf;
	TCHAR valuebuf2[MAX_VALUE_NAME];

	GetKeyPath(keypath, index, data_set);
	typebuf = GetKeyType(keytype);
	memcpy(data_buf, data_set, strlen(data_set));
	


	if (RegCreateKeyEx(index[0], data_buf, 0, NULL, dwOptions, KEY_WRITE, NULL, &hTestKey, &dwDisposition) == ERROR_SUCCESS)
	{
		
		if (typebuf == 4 || typebuf == 5)
		{
			int i, c;
			i = strlen(keyvalue);
			for (c = 0; c < i; c++)
			{
				sprintf(buf, "%c", keyvalue[c]);
				int d = 0;
				d = buf[0];
				if ((48 <= d) && (d <= 57))
				{
					continue;
				}
				else
				{
					printf("ERROR: Invalid syntax. Specify valid numeric value for keyvalue.");
					return FALSE;
				}
			}
			valuebuf = atoi(keyvalue);  //str to int

			if (RegSetValueEx(hTestKey, achkey, 0, typebuf, (CONST BYTE*)&valuebuf, sizeof(DWORD)) == ERROR_SUCCESS) //set num value
			{
				printf("REG ADD SUCCESS!!");
			}
			else
				return FALSE;
		}
		else if (typebuf == 1 || typebuf == 2 || typebuf == 6 || typebuf == 7)
		{
			memcpy(buf, keyvalue, strlen(keyvalue) + 2);

			if (RegSetValueEx(hTestKey, achkey, 0, typebuf, (CONST BYTE*)buf, lstrlen(buf) + 2) == ERROR_SUCCESS) //set str value
			{
				printf("REG ADD SUCCESS!!");
			}
			else
				return FALSE;
		}
		else if (typebuf == -1)
		{
			return FALSE;
		}
		else
		{

			int i, b, c, m;
			i = strlen(keyvalue);
			m = 0;
			for (c = 0; c <i; c++)
			{
				sprintf(buf, "%c", keyvalue[c]);
				int d = 0;
				d = buf[0];
				if ((48 <= d) && (d <= 57))  //only HEX can input
				{
					continue;
				}
				else if ((65 <= d) && (d <= 70))
				{
					continue;
				}
				else if ((97 <= d) && (d <= 102))
				{
					continue;
				}
				else
				{
					printf("ERROR: Invalid syntax. Specify valid hex value for keyvalue.");
					return FALSE;
				}
			}
			memset(buf, 0, sizeof(buf));
			//each two str to one HEX,and put in buf[]={hex,hex,hex}
			for (b = 0; b < i; b = b + 2)
			{

				memset(buf, 0, sizeof(buf));
				if (i % 2 != 0 && b == 0)  //if str' num is odd,add "0" at the head of str
				{
					sprintf(buf, "0%c", keyvalue[b]);
					b = b - 1;
				}
				else
				{
					sprintf(buf, "%c%c", keyvalue[b], keyvalue[b + 1]);
				}
				sscanf(buf, "%x", &valuebuf);
				valuebuf2[m] = valuebuf;
				m++;

			}


			if (RegSetValueEx(hTestKey, achkey, 0, typebuf, (CONST BYTE*)valuebuf2, (i + 1)*sizeof(TCHAR) / 2) == ERROR_SUCCESS) //set binary value
			{
				printf("REG ADD SUCCESS!!");
			}
			else
				return FALSE;

		}


		RegCloseKey(hTestKey);
	}
	else
		return FALSE;

	return TRUE;
}



extern "C" __declspec(dllexport)
void F2(
HWND hwnd,        // handle to owner window   
HINSTANCE hinst,  // instance handle for the DLL   
LPTSTR lpCmdLine, // string the DLL will parse   
int nCmdShow      // show state   
)
{

	USES_CONVERSION;
	int nArgs = 0;
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if (NULL != szArglist && nArgs == 2)  //get the autostart services
	{
		RedirectIOToConsole();
		HKEY hTestKey;
		TCHAR data_set[] = TEXT("SYSTEM\\CurrentControlSet\\services");

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, data_set, 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
		{
			QueryServices(hTestKey, data_set);
		}

		RegCloseKey(hTestKey);


	}
	else if (NULL != szArglist && nArgs == 3)  //get the key's subkey
	{
		RedirectIOToConsole();
		HKEY hTestKey;
		HKEY index[MAX_KEY_LENGTH];
		TCHAR data_set[MAX_KEY_LENGTH];
		TCHAR data_buf[MAX_KEY_LENGTH] = {};

		printf("%s\n", W2A(szArglist[2]));
		GetKeyPath(W2A(szArglist[2]), index, data_set);

		memcpy(data_buf, data_set, strlen(data_set));
		if (RegOpenKeyEx(index[0], data_buf, 0, KEY_READ, &hTestKey) == ERROR_SUCCESS)
		{

			QueryKey(hTestKey, data_buf);
		}
		else
		{
			printf("ERROR PATH!!\n");
		}
		RegCloseKey(hTestKey);

	}
	else if (NULL != szArglist&&nArgs == 6) //reg add key
	{
		RedirectIOToConsole();
		RegAddValue(W2A(szArglist[2]), W2A(szArglist[3]), W2A(szArglist[4]), W2A(szArglist[5]));
	}
	else
	{
		MessageBox(NULL, "error", "error", MB_OK);
		return;
	}
	LocalFree(szArglist);
	system("pause");
	return;
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("PROCESS ATTACH!");
		break;
	case DLL_THREAD_ATTACH:
		printf("THREAD ATTACH!");
		break;
	case DLL_THREAD_DETACH:
		printf("THREAD DETACH!");
		break;
	case DLL_PROCESS_DETACH:
		printf("PROCESS DETACH");
		break;
	}
	return TRUE;
}

