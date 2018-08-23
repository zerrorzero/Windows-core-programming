#include <windows.h>
#include <stdio.h>


#define ID_EDITBOX    1            
#define ID_SAVEBTN    2        
#define ID_CLSBTN    3             


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CreateChildWindow(HWND, HWND *, LPARAM);
int SavaInputContent(TCHAR *);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("savetotext");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(szAppName, TEXT("Save Message"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
	SetTimer(hwnd, 1, 10000, NULL);
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);


	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(hwnd, 1);
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static        HWND hwndChild[3];
	RECT    rect;
	static    TCHAR *szBuffer;               
	static int        iLength;
	static int i;
	static HBRUSH hBrush;
	switch (message)
	{
	case WM_CREATE:
		CreateChildWindow(hwnd, hwndChild, lParam);
		return 0;

	case WM_SIZE:
		GetClientRect(hwnd, &rect);
		MoveWindow(hwndChild[ID_EDITBOX], 0, 0, rect.right, rect.bottom - 50, TRUE);
		MoveWindow(hwndChild[ID_SAVEBTN], rect.right - 200, rect.bottom - 35, 50, 25, TRUE);
		MoveWindow(hwndChild[ID_CLSBTN], rect.right - 120, rect.bottom - 35, 50, 25, TRUE);

		return 0;


	case WM_CTLCOLOREDIT:
		HDC hdc;
		hdc = (HDC)wParam;
		SetBkColor(hdc, RGB(0, 0, 0));
		SetTextColor(hdc, RGB(0, 255, 0));
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		return (LRESULT)hBrush;

	case WM_COMMAND:   //button save and clean
		switch (LOWORD(wParam))
		{

		case ID_SAVEBTN:
			iLength = GetWindowTextLength(hwndChild[ID_EDITBOX]);
			if (iLength != 0)
				szBuffer = (TCHAR *)malloc((iLength) * sizeof(TCHAR));
			else
				return -1;
			GetWindowText(hwndChild[ID_EDITBOX], szBuffer, GetWindowTextLength(hwndChild[ID_EDITBOX]) + 1);
			SavaInputContent(szBuffer);
			SetWindowText(hwndChild[ID_EDITBOX], TEXT(""));
			return 0;

		case ID_CLSBTN:
			SetWindowText(hwndChild[ID_EDITBOX], TEXT(""));
			return 0;

		}
		return 0;
	
	case WM_DRAWITEM:
	case WM_TIMER:  //each 10s show or hide the window,and save message when hide.
		if (IsWindowVisible(hwnd)==0)
		{
			SetWindowText(hwndChild[ID_EDITBOX], TEXT(""));
			ShowWindow(hwnd, SW_SHOW);
			break;
		}
		else
		{
			ShowWindow(hwnd, SW_HIDE);
			iLength = GetWindowTextLength(hwndChild[ID_EDITBOX]);
			if (iLength != 0)
				szBuffer = (TCHAR *)malloc((iLength)* sizeof(TCHAR));
			else
				return -1;
			GetWindowText(hwndChild[ID_EDITBOX], szBuffer, GetWindowTextLength(hwndChild[ID_EDITBOX]) + 1);
			SavaInputContent(szBuffer);

			break;
		}
		
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int CreateChildWindow(HWND hwnd, HWND *hwndChild, LPARAM lParam)
{
	HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

	hwndChild[ID_EDITBOX] = CreateWindow(TEXT("edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		0, 0, 0, 0,
		hwnd, (HMENU)ID_EDITBOX, hInst, NULL);

	hwndChild[ID_SAVEBTN] = CreateWindow(TEXT("button"), TEXT("SAVE"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
		hwnd, (HMENU)ID_SAVEBTN, hInst, NULL);

	hwndChild[ID_CLSBTN] = CreateWindow(TEXT("button"), TEXT("CLEAN"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0,
		hwnd, (HMENU)ID_CLSBTN, hInst, NULL);


	return 0;
}

int SavaInputContent(TCHAR *content)
{
	FILE *fp;
	char *path = "c:\\msg.txt";
	fp = fopen(path, "a");
	if (fp == NULL)
	{
		MessageBox(NULL, TEXT("SAVE FAIL!!"), TEXT("INFO"), MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	fputs(content, fp);
	fprintf(fp, "\n");
	fclose(fp);

	return 0;
}
