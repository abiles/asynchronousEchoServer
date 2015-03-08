// AsyncEchoServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"



#define WM_SOCKET (WM_USER + 1)


HWND createWokerWindow();
void printError(char* funcName, int errorType);

int _tmain(int argc, _TCHAR* argv[])
{
	HWND workerWindow = createWokerWindow();
	
	if (NULL == workerWindow)
		return 0;
	MSG wMSG;
	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN serAdr;
	int port = 9001;
	int numOfBackLog = 100;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printError("WSAStartup()", GetLastError());
		return 0;
	}


	if (INVALID_SOCKET == (hServSock = socket(PF_INET, SOCK_STREAM, 0)));
	{
		printError("socket()", GetLastError());
		return 0;
	}

	if (0 != WSAAsyncSelect(hServSock, workerWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE))
	{
		printError("WSAAsyncSelect()", GetLastError());
		return 0;
	}
	
	serAdr.sin_family = AF_INET;
	serAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	serAdr.sin_port = htons(port);

	if (SOCKET_ERROR == (bind(hServSock, (SOCKADDR*)&serAdr, sizeof(serAdr))))
	{
		printError("bind()", GetLastError());
		return 0;
	}

	if (SOCKET_ERROR == (listen(hServSock, numOfBackLog)))
	{
		printError("listen()", GetLastError());
		return 0;
	}

	while (GetMessage(&wMSG, workerWindow, 0, 0))
	{
		TranslateMessage(&wMSG);
		DispatchMessage(&wMSG);
	}

	closesocket(hServSock);
	WSACleanup();

	return 0;
}

HWND createWokerWindow()
{
	WNDCLASS wndClass;
	HWND window;
	CHAR* windowName = "AsyncSelectServer";

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	//wndClass.lpfnWndProc = (WNDPROC)windowProc();
	wndClass.cbClsExtra = 0; 
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = NULL;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszClassName = (LPCWSTR)windowName;
	wndClass.lpszMenuName = NULL;

	if (false == RegisterClass(&wndClass))
	{
		printError("createWorkerWindow()", GetLastError());
		return 0;
	}
	else
	{
		printf_s("wndclass is registered\n");
	}

	window = CreateWindow( (LPCWSTR)windowName, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	if (NULL == window)
	{
		printError("createWindow()", GetLastError());
		return NULL;
	}
	else
	{
		printf_s("create window\n");
	}

	return window;

}

void printError(char* funcName, int errorType)
{
	printf_s("In %s, GetLastError : %d\n", funcName, errorType);
}

