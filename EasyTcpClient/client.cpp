#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")


enum CMD 
{
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

struct DataHeader
{
    short dataLength; 
    short cmd;

};


//DataPackage
struct  Login
{
    char UserName[32];
    char PassWord[32];
    char name[32];

};

struct LoginResult
{
    int result;
};

struct Logout {

    char UserName[32];
};

struct LogoutResult 
{

    int result;
};

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
            printf("�յ�exit����������");
           break;
        }
        else if(0 == strcmp(cmdBuf,"login")) {
            Login login = { "lyd","lydmm"};
            DataHeader dh = {sizeof(login),CMD_LOGIN};
            //5 �������������������
            send(_sock, (const char*)& dh,sizeof(dh), 0);
            send(_sock, (const char*)&login, sizeof(login), 0);
            //���շ�������������
            DataHeader retHeader = {};
            LoginResult loginRet = {};
            recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
            recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
            printf("LoginResult: %d", loginRet.result);
        }
        else if (0 == strcmp(cmdBuf, "logout")) {
            Logout logout = { "lyd" };
            DataHeader dh = {sizeof(logout),CMD_LOGOUT };
            //��������˷�����������
            send(_sock, (const char*)&dh, sizeof(dh), 0);
            send(_sock, (const char*)&logout, sizeof(logout), 0);
            DataHeader retHeader = {};
            LogoutResult logoutRet = {};
            recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
            recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
            
         
        }
        else{
        
            printf("��֧�ֵ�������������롣\n");
        }


        
    }
    closesocket(_sock);


    //���windows scoket����
    WSACleanup();
    printf("�ͻ����Ѿ��˳��������");
    getchar();
    return 0;


}
