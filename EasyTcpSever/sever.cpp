#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

//用结构体给客户端返回消息
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

    //-用Scoket API建立简易TCP服务端
    //1.建立一个socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    sockaddr_in _sin = {};  //={}用来初始化一下这个结构体实例
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
 
    if(SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("错误，绑定网络端口失败....\n");
    
    }
    else {
    
        printf("绑定网络端口成功...\n");
    }
    
    if(SOCKET_ERROR ==listen(_sock, 5))
    {
        printf("错误，监听网络端口失败....\n");

    }
    else {

        printf("监听网络端口成功...\n");
    }
    //接收客户端返回的socket
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(sockaddr_in);
    SOCKET _cSock = INVALID_SOCKET;
    //这个宏表示无效的socket地址。

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("错误，接受到无效客户端SOCKET..\n");
    }
    printf("新客户端加入:socket = %d，IP = %s\n", (int)_cSock,inet_ntoa(clientAddr.sin_addr));

   
    while (true)
    {
        DataHeader header = {};
        //5 接收客户端的请求数据
        int nLen = recv(_cSock, (char*)&header, sizeof(header), 0);
        if (nLen <= 0) {
        
            printf("客户端已退出，任务结束");
            break;
        }
        printf("收到命令:%d 数据长度:%d \n", header.cmd,header.dataLength);
        switch (header.cmd) 
        {
        case CMD_LOGIN:
        {
            Login login = {};
            recv(_cSock, (char*)&login, sizeof(Login), 0);
            //忽略判断用户密码是否正确的过程
            LoginResult ret = {1};
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

        }
        break;
        case CMD_LOGOUT:
        {
            Logout logout = {};
            recv(_cSock, (char*)&logout, sizeof(logout), 0);
            //退出登录
            LoginResult ret = { 1 };
            send(_cSock, (char*)&header, sizeof(header), 0);
            send(_cSock, (char*)&ret, sizeof(ret), 0);

        }
        break;
        
        default:
            header.cmd = CMD_ERROR;
            header.dataLength = 0;
            send(_cSock, (char*)&header, sizeof(header), 0);
        break;
        }
         //6 处理请求
       
        

        

    }

    closesocket(_sock);

    //清除windows scoket环境
    WSACleanup();
    printf("服务端已经退出，任务结束");
    getchar();
    return 0;

    
}


