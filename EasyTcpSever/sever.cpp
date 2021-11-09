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

    //-��Scoket API��������TCP�����
    //1.����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    sockaddr_in _sin = {};  //={}������ʼ��һ������ṹ��ʵ��
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
 
    if(SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("���󣬰�����˿�ʧ��....\n");
    
    }
    else {
    
        printf("������˿ڳɹ�...\n");
    }
    
    if(SOCKET_ERROR ==listen(_sock, 5))
    {
        printf("���󣬼�������˿�ʧ��....\n");

    }
    else {

        printf("��������˿ڳɹ�...\n");
    }
    //���տͻ��˷��ص�socket
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(sockaddr_in);
    SOCKET _cSock = INVALID_SOCKET;
    //������ʾ��Ч��socket��ַ��

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("���󣬽��ܵ���Ч�ͻ���SOCKET..\n");
    }
    printf("�¿ͻ��˼���:socket = %d��IP = %s\n", (int)_cSock,inet_ntoa(clientAddr.sin_addr));

    char _recvBuf[128] = {};
    while (true)
    {
        //5 ���տͻ��˵���������
        int nLen = recv(_cSock, _recvBuf, 128, 0);
        if (nLen <= 0) {
        
            printf("�ͻ������˳����������");
            break;
        }
        printf("�յ�����:%s \n", _recvBuf);
         //6 ��������
        if (0 == strcmp(_recvBuf, "getName")) 
        {
            char msgBuf[] = "xiao Qiang.";
            send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
        
        }
        else if(0 == strcmp(_recvBuf,"getAge")) {
            char msgBuf[] = "80.";
            send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
        
        }
        else
        {
            char msgBuf[] = "???.";
            send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
        
        }
        //6 send ��ͻ��˷���һ������

        

    }

    closesocket(_sock);

    //���windows scoket����
    WSACleanup();
    printf("������Ѿ��˳����������");
    getchar();
    return 0;

    
}


