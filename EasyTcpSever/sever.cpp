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
        char szRecv[1024] = {};
        //5 接收客户端的请求数据
        int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
        DataHeader* header = (DataHeader*)szRecv;
        if (nLen <= 0) {
        
            printf("客户端已退出，任务结束");
            break;
        }
        
        switch (header->cmd) 
        {
        case CMD_LOGIN:
        {
            recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
            Login* login = (Login*)szRecv;
            printf("收到命令：CMD_LOGIN 数据长度: %d  userName = %s PassWord = % s\n", login->dataLength ,login->UserName,login->PassWord);
            //忽略判断用户密码是否正确的过程
            LoginResult ret;
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

        }
        break;
        case CMD_LOGOUT:
        {
            recv(_cSock, szRecv + sizeof(DataHeader),header->dataLength - sizeof(DataHeader), 0);
            Logout *logout = (Logout*)szRecv;
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
         //6 处理请求
       
        

        

    }

    closesocket(_sock);

    //清除windows scoket环境
    WSACleanup();
    printf("服务端已经退出，任务结束");
    getchar();
    return 0;

    
}


