#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>

#include<vector>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
//用结构体给客户端返回消息
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


vector<SOCKET> g_clients;

int processor(SOCKET _cSock) 
{
    char szRecv[4096] = {};
    //5 接收客户端的请求数据,处理请求
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {

        printf("客户端已退出，任务结束");
        return -1;
    }

    switch (header->cmd)
    {
    case CMD_LOGIN:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Login* login = (Login*)szRecv;
        printf("收到命令：CMD_LOGIN 数据长度: %d  userName = %s PassWord = % s\n", login->dataLength, login->UserName, login->PassWord);
        //忽略判断用户密码是否正确的过程
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

    }
    break;
    case CMD_LOGOUT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Logout* logout = (Logout*)szRecv;
        printf("收到命令:CMD_LOGIN,数据长度: %d, userName = %s \n", logout->dataLength, logout->UserName);
        //退出登录
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(ret), 0);

    }
    break;

    default:
        DataHeader header = { 0,CMD_ERROR };
        send(_cSock, (char*)&header, sizeof(header), 0);
        break;
    }
   





}


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

        printf("打开监听网络端口成功...\n");
    }
   
   
    while (true)
    {
        //伯克利 socket
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);

        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExp);

        for (int i = 0;i < (int)g_clients.size();i++) {
         
            FD_SET(g_clients[i], &fdRead);
        }
       
        //第一个参数nfds是一个整数，是指fd_set集合中所以描述符（socket）的范围
        //就是里面最大的scoket值+1,在windows中参数可以写0.
        int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
        if (ret < 0)
        {
            printf("select任务结束。 \n");
            break;
        }
        if (FD_ISSET(_sock, &fdRead))                  
        {
            FD_CLR(_sock, &fdRead);       
            //只是清理计数器
            //4 accept等待客户端连接,接收客户端的socket
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
            //这个宏表示无效的socket地址。

            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
            if (INVALID_SOCKET == _cSock) {
                printf("错误，接受到无效客户端SOCKET..\n");
            }
            g_clients.push_back(_cSock);
            printf("新客户端加入:socket = %d，IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

        }
        for (int i = 0;i < (int)fdRead.fd_count;i++)
        {
            if (-1 == processor(fdRead.fd_array[i])) 
            {
               auto iter = find(g_clients.begin(),g_clients.end(),fdRead.fd_array[i]);
                   if (iter != g_clients.end())
                   {
                       g_clients.erase(iter);
                   }
            
            }
        }
           

    }

    for (int i= 0; i < (int)g_clients.size();i++) 
    {
        closesocket(g_clients[i]);
    
    }

    //清除windows scoket环境
    WSACleanup();
    printf("服务端已经退出，任务结束");
    getchar();
    return 0;

    
}


