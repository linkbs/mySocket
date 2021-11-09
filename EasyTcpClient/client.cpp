#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

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
            break;
        }
        else {
            //5 向服务器发送亲求命令
            send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
        }


        //接收服务器信息 recv
        char recvBuf[128] = {};
        int nlen = recv(_sock, recvBuf, 128, 0);
        if (nlen > 0) {

            printf("接收到的数据：%s\n", recvBuf);
        }
        //关闭套接字
    }
    closesocket(_sock);


    //清除windows scoket环境
    WSACleanup();
    printf("客户端已经退出任务结束");
    getchar();
    return 0;


}
