#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

int main() {
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);

    //���windows scoket����
    WSACleanup();
    return 0;


}

