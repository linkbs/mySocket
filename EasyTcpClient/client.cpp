#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

int main() {
    //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    //����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == _sock) {
    
        printf("���󣬽���SOCKETʧ��....\n");
    }
    else {
       
        printf("����SOCKET�ɹ�....\n");
    }

   

    //���ӷ�����connect

    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        printf("���󣬽���Socketʧ��....\n");
    }
    else {
        printf("��ȷ������Socket�ɹ�.....\n");
    
    }


    char cmdBuf[128] = {};
    while (true)
    {
        //3������������
        char cmdBuf[128] = {};
        scanf("%s", cmdBuf);
        //4��������
        if (0 == strcmp(cmdBuf, "exit"))
        {
            break;
        }
        else {
            //5 �������������������
            send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
        }


        //���շ�������Ϣ recv
        char recvBuf[128] = {};
        int nlen = recv(_sock, recvBuf, 128, 0);
        if (nlen > 0) {

            printf("���յ������ݣ�%s\n", recvBuf);
        }
        //�ر��׽���
    }
    closesocket(_sock);


    //���windows scoket����
    WSACleanup();
    printf("�ͻ����Ѿ��˳��������");
    getchar();
    return 0;


}
