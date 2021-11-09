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
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

struct DataHeader
{
    short dataLength;
    short cmd;

};


//DataPackage
struct  Login : public DataHeader
{
    Login()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;

    }
    char UserName[32];
    char PassWord[32];
    char name[32];

};

struct LoginResult : public DataHeader
{
    LoginResult()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct Logout : public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGOUT;

    }
    char UserName[32];
};

struct LogoutResult : public DataHeader
{
    LogoutResult()
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }

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
            Login login ;
            strcpy(login.UserName, "lyd");
            strcpy(login.PassWord, "lydmima");
            //5 �������������������
            send(_sock, (const char*)&login, sizeof(login), 0);
            //���շ�������������
            LoginResult loginRet = {};
            recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
            printf("LoginResult: %d", loginRet.result);
        }
        else if (0 == strcmp(cmdBuf, "logout")) {
            Logout logout;
            strcpy(logout.UserName, "lyd");
            //��������˷�����������
            send(_sock, (const char*)&logout, sizeof(logout), 0);
            LogoutResult logoutRet = {};
            recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
            printf("LogoutResult : %d\n", logoutRet.result);
         
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
