#pragma comment (lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>

#define WM_SOCKET (WM_USER + 1)
#define WIN_NAME "Async"
#define BUF_SIZE 4096
#define PORT 9001
#define MAXCONNECT 100

HWND createWorkerWindow();
SOCKET createListenSocket(HWND hWnd);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void onAccept(SOCKET sock, HWND hWnd);
void onRead(SOCKET sock, HWND hWnd);
void onClose(SOCKET sock, HWND hWnd);

int main(int argc, char* argv[])
{
	HWND hWnd = createWorkerWindow();
	
	if (hWnd == NULL)
		return -1;

	printf_s("create window\n");

	SOCKET listenSocket = createListenSocket(hWnd);
	if (listenSocket == INVALID_SOCKET)
		return -1;

	printf_s("create listenSocket\n");

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	closesocket(listenSocket);
	WSACleanup();

	return 0;
}

HWND createWorkerWindow()
{
	WNDCLASS wndClass;
	HWND hWnd;

	wndClass.lpfnWndProc = (WNDPROC) WndProc;
	wndClass.hInstance = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = WIN_NAME;

	RegisterClass(&wndClass);
	hWnd = CreateWindowEx(
		0,
		WIN_NAME,
		"Server",
		0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	if (hWnd == NULL)
		return NULL;

	return hWnd;

}

SOCKET createListenSocket(HWND hWnd)
{
	WSADATA wsaData;
	SOCKET hListenSocket;
	int errorId;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		return INVALID_SOCKET;

	hListenSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hListenSocket == INVALID_SOCKET)
		return INVALID_SOCKET;

	SOCKADDR_IN listenAddr;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_port = htons(PORT);

	errorId = bind(hListenSocket, (sockaddr*)&listenAddr, sizeof(listenAddr));
	if (errorId == SOCKET_ERROR)
	{
		closesocket(hListenSocket);
		return INVALID_SOCKET;
	}

	errorId = listen(hListenSocket, MAXCONNECT);
	if (errorId == SOCKET_ERROR)
	{
		closesocket(hListenSocket);
		return INVALID_SOCKET;
	}

	errorId = WSAAsyncSelect(hListenSocket, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (errorId != NO_ERROR)
	{
		closesocket(hListenSocket);
		return INVALID_SOCKET;
	}

	return hListenSocket;
	
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SOCKET)
	{
		SOCKET sock = wParam;

		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			onAccept(sock, hwnd);
			break;
		case FD_READ:
			onRead(sock, hwnd);
			break;
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void onAccept(SOCKET sock, HWND hWnd)
{
	sockaddr_in addr = { 0, };
	int addrSize = sizeof(addr);

	SOCKET clntSock = accept(sock, (sockaddr*)&addr, &addrSize);
	if (clntSock == INVALID_SOCKET)
		return;

	int errorId;
	errorId = WSAAsyncSelect(clntSock, hWnd, WM_SOCKET, FD_READ | FD_READ | FD_CLOSE);
	if (errorId != NO_ERROR)
	{
		closesocket(clntSock);
		return;
	}

}

void onRead(SOCKET sock, HWND hWnd)
{
	char buf[BUF_SIZE] = { 0, };
	int errorId;

	int recvSize = recv(sock, buf, BUF_SIZE, 0);
	if (recvSize == SOCKET_ERROR)
	{
		errorId = WSAGetLastError();
		if (errorId != WSAEWOULDBLOCK)
		{
			closesocket(sock);
			return;
		}
	}

	int sendSize = send(sock, buf, BUF_SIZE, 0);
	if (sendSize == SOCKET_ERROR)
	{
		errorId = WSAGetLastError();
		if (errorId != WSAEWOULDBLOCK)
		{
			closesocket(sock);
			return;
		}
	}
}

void onClose(SOCKET sock, HWND hWnd)
{
	closesocket(sock);
}
