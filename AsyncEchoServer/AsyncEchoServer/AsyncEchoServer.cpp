// AsyncEchoServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

#define WM_SOCKET (WM_USER + 1)

#define WIN_NAME "AsyncServer"
#define BUF_SIZE 4096
#define PORT 9001
#define MAXCONNECT 100

HWND createWokerWindow();
void printError(char* funcName, int errorType);
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int _tmain(int argc, _TCHAR* argv[])
{
	HWND workerWindow = createWokerWindow();
	
	if (NULL == workerWindow)
		return -1;
	MSG wMSG;
	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN serAdr;


	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		printError("WSAStartup()", WSAGetLastError());
		return 0;
	}

	if (INVALID_SOCKET == (hServSock = socket(PF_INET, SOCK_STREAM, 0)))
	{
		printError("socket()", WSAGetLastError());
		return 0;
	}

	if (0 != WSAAsyncSelect(hServSock, workerWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE))
	{
		printError("WSAAsyncSelect()", WSAGetLastError());
		return 0;
	}
	
	serAdr.sin_family = AF_INET;
	serAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	serAdr.sin_port = htons(PORT);

	if (SOCKET_ERROR == (bind(hServSock, (SOCKADDR*)&serAdr, sizeof(serAdr))))
	{
		printError("bind()", WSAGetLastError());
		return 0;
	}

	if (SOCKET_ERROR == (listen(hServSock, MAXCONNECT)))
	{
		printError("listen()", WSAGetLastError());
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
	wndClass.lpfnWndProc = (WNDPROC) WndProc;
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

	window = CreateWindowEx(
		0,
		WIN_NAME, 
		"", 
		0,
		CW_USEDEFAULT, CW_USEDEFAULT, 
		CW_USEDEFAULT, CW_USEDEFAULT,
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
	printf_s("In %s, ErrorType : %d\n", funcName, errorType);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SOCKET clntSock;
	SOCKADDR_IN socketAddr;
	int sockAddrSize;
	//getpeername(wParam, (SOCKADDR*)&socketAddr, &sockAddrSize);
	//char* ip = inet_ntoa(socketAddr.sin_addr);
	//int port = ntohs(socketAddr.sin_port);
	char buffer[BUF_SIZE] = { 0, };
	int recvSize, sendSize;
	int errorId;

	if (uMsg == WM_SOCKET)
	{
		errorId = WSAGETSELECTERROR(lParam);
		if (errorId != WSAEWOULDBLOCK)
		{
			printError("windowProc()", GetLastError());
		}
		else
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				clntSock = accept(wParam, NULL, NULL);
				WSAAsyncSelect(clntSock, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
				break;
			case FD_READ:
		
				recvSize = recv(wParam, buffer, BUF_SIZE, 0);
				sendSize = send(wParam, buffer, BUF_SIZE, 0);
				
				break;
			case  FD_CLOSE:
				//printf_s("Socket DisConnected = ip: %s, port : %d", ip, port);
				closesocket(wParam);
				break;
			}
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


