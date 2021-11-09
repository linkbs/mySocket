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
        //3输入请求命令
        char cmdBuf[128] = {};
        scanf("%s", cmdBuf);
        //4处理请求
        if (0 == strcmp(cmdBuf, "exit"))
        {
            printf("收到exit命令，任务结束");
           break;
        }
        else if(0 == strcmp(cmdBuf,"login")) {
            Login login = { "lyd","lydmm"};
            DataHeader dh = {sizeof(login),CMD_LOGIN};
            //5 向服务器发送亲求命令
            send(_sock, (const char*)& dh,sizeof(dh), 0);
            send(_sock, (const char*)&login, sizeof(login), 0);
            //接收服务器返回数据
            DataHeader retHeader = {};
            LoginResult loginRet = {};
            recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
            recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
            printf("LoginResult: %d", loginRet.result);
        }
        else if (0 == strcmp(cmdBuf, "logout")) {
            Logout logout = { "lyd" };
            DataHeader dh = {sizeof(logout),CMD_LOGOUT };
            //向服务器端发送请求命令
            send(_sock, (const char*)&dh, sizeof(dh), 0);
            send(_sock, (const char*)&logout, sizeof(logout), 0);
            DataHeader retHeader = {};
            LogoutResult logoutRet = {};
            recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
            recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
            
         
        }
        else{
        
            printf("不支持的命令，请重新输入。\n");
        }


        
    }
    closesocket(_sock);


    //清除windows scoket环境
    WSACleanup();
    printf("客户端已经退出任务结束");
    getchar();
    return 0;


}
