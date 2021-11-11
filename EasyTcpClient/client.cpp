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
    //5 接收客户端的请求数据,处理请求
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {

        printf("与服务器断开链接，任务结束. \n", _cSock);
        return -1;
    }

    switch (header->cmd)
    {
    case CMD_LOGIN_RESULT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        LoginResult* login = (LoginResult*)szRecv;
        printf("收到服务器端消息：CMD_LOGIN_RESULT 数据长度: %d \n", _cSock, header->dataLength);

    }
    break;
    case CMD_LOGOUT_RESULT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        LogoutResult* logout = (LogoutResult*)szRecv;
        printf("收到服务器端消息：CMD_LOGOUT_RESULT 数据长度: %d \n", _cSock, header->dataLength);
    }
    break;
    case CMD_NEW_USER_JOIN:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        NewUserJoin* userJoin = (NewUserJoin*)szRecv;
        printf("收到服务器端消息：CMD_NEW_USER_JOIN 数据长度: %d \n", _cSock, header->dataLength);

    }
    break;



    }

}


int main() {
    //启动Windows socket 2.x环境
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
    //建立一个socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == _sock) {
    
        printf("错误，建立SOCKET失败....\n");
    }
    else {
       
        printf("建立SOCKET成功....\n");
    }

   

    //连接服务器connect

    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        printf("错误，建立Socket失败....\n");
    }
    else {
        printf("正确，建立Socket成功.....\n");
    
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
            printf("select任务结束");
            break;
        }
        if (FD_ISSET(_sock, &fdReads))
        {

            FD_CLR(_sock, &fdReads);

            if (-1 == processor(_sock))
            {
                printf("select任务结束2\n");
                break;
            }
        }
        printf("空闲时间处理其他业务..\n");
        Login login;
        strcpy(login.UserName, "lyd");
        strcpy(login.PassWord, "lydmm");
        send(_sock, (const char*)&login, sizeof(Login), 0);
    
        
    }
    closesocket(_sock);


    //清除windows scoket环境
    WSACleanup();
    printf("客户端已经退出任务结束");
    getchar();
    return 0;


}
