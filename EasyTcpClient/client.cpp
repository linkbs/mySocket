#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <vector>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
enum CMD
{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
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
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct Logout : public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;

    }
    char UserName[32];
};

struct LogoutResult : public DataHeader
{
    LogoutResult()
    {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }

    int result;
};

struct NewUserJoin : public DataHeader
{
    NewUserJoin()
    {
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }

    int sock;
};

vector<SOCKET> g_clients;

int processor(SOCKET _cSock)
{
    char szRecv[4096] = {};
    //5 ���տͻ��˵���������,��������
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {

        printf("��������Ͽ����ӣ��������. \n", _cSock);
        return -1;
    }

    switch (header->cmd)
    {
    case CMD_LOGIN_RESULT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        LoginResult* login = (LoginResult*)szRecv;
        printf("�յ�����������Ϣ��CMD_LOGIN_RESULT ���ݳ���: %d \n", _cSock, header->dataLength);

    }
    break;
    case CMD_LOGOUT_RESULT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        LogoutResult* logout = (LogoutResult*)szRecv;
        printf("�յ�����������Ϣ��CMD_LOGOUT_RESULT ���ݳ���: %d \n", _cSock, header->dataLength);
    }
    break;
    case CMD_NEW_USER_JOIN:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        NewUserJoin* userJoin = (NewUserJoin*)szRecv;
        printf("�յ�����������Ϣ��CMD_NEW_USER_JOIN ���ݳ���: %d \n", _cSock, header->dataLength);

    }
    break;



    }

}


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
        fd_set fdReads;
        FD_ZERO(&fdReads);
        FD_SET(_sock, &fdReads);
        timeval t = { 1,0 };
        int ret = select(_sock,&fdReads,0,0,&t);
        if (ret < 0)
        {
            printf("select�������");
            break;
        }
        if (FD_ISSET(_sock, &fdReads))
        {

            FD_CLR(_sock, &fdReads);

            if (-1 == processor(_sock))
            {
                printf("select�������2\n");
                break;
            }
        }
        printf("����ʱ�䴦������ҵ��..\n");
        Login login;
        strcpy(login.UserName, "lyd");
        strcpy(login.PassWord, "lydmm");
        send(_sock, (const char*)&login, sizeof(Login), 0);
    
        
    }
    closesocket(_sock);


    //���windows scoket����
    WSACleanup();
    printf("�ͻ����Ѿ��˳��������");
    getchar();
    return 0;


}
