#ifndef _EasyTcpServer_hpp_
#define _EasyTcpSever_hpp_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>



#define SOCKET int
#define INVALID_SOCKET   (SOCKET)(~0)
#define SOCKET_ERROR             (-1)
#endif
#include<string.h>
#include <vector>
#include <stdio.h>
#include "MessageHeader.hpp"


class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer() {

		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {

		Close();
	}
	//初始化Socket
	SOCKET InitSocket()
	{
		//启动 win  Sock 2.x环境
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败....\n");
			Close();
		}
		else {

			printf("建立Socket成功...\n");
		}

		return _sock;

	}
	//绑定端口
	int Bind(const char* ip, unsigned short port)
	{
		sockaddr_in _sin = {};  //={}用来初始化一下这个结构体实例
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("错误，绑定网络端口<%d>失败....\n", port);

		}
		else {

			printf("绑定网络端口<%d>成功...\n", port);
		}
		return ret;
	}
	//监听端口
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("Socket=<%d>错误，监听网络端口失败....\n", _sock);

		}
		else {

			printf("Socket=<%d>打开监听网络端口成功...\n", _sock);
		}
		return ret;

	}
	//接受客户端链接
	int Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
		//这个宏表示无效的socket地址。

		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			printf("错误，接受到无效客户端SOCKET..\n");
		}
		else {
			NewUserJoin userJoin;
			SendDataAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("新客户端加入:socket = %d，IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return (int)_cSock;

		}
	

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET) {
			for (int i = 0; i < (int)g_clients.size();i++)
			{
				closesocket(g_clients[i]);

			}

			//清除windows scoket环境
			WSACleanup();
			printf("服务端已经退出，任务结束");
			getchar();

		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun()) {
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
			timeval t = { 2,0 };
			//第一个参数nfds是一个整数，是指fd_set集合中所以描述符（socket）的范围
			//就是里面最大的scoket值+1,在windows中参数可以写0.
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//_sock有可读才会被放进fdRead

			if (ret < 0)
			{
				printf("select任务结束。 \n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				//接收客户端套接字，客户端发消息在下个循环处理
				//只是清理计数器
				//4 accept等待客户端连接,接收客户端的socket
			}
			for (int i = 0;i < (int)fdRead.fd_count;i++)
			{
				if (-1 == RecvData(fdRead.fd_array[i]))
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);
					}

				}
			}

			return true;
		}
		return false;

	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//接收数据包 处理粘包 拆分包
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		//5 接收客户端的请求数据,处理请求
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {

			printf("客户端<Scoket = %d>已退出，任务结束. \n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		OnNetMsg(_cSock, header);
		return 0;

	}
	//响应网络消息
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("收到客户端<Scoket = %d>请求：CMD_LOGIN 数据长度: %d  userName = %s PassWord = % s\n", _cSock, login->dataLength, login->UserName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("收到客户端<Scoket = %d>请求:CMD_LOGOUT,数据长度: %d, userName = %s \n", _cSock, logout->dataLength, logout->UserName);
			//退出登录
			LogoutResult ret;
			send(_cSock, (char*)&ret, sizeof(ret), 0);

		}
		break;

		default:
			DataHeader header = { 0,CMD_ERROR };
			send(_cSock, (char*)&header, sizeof(header), 0);
			break;
		}

	
	
	}
	
   
	


	//给指定客户端发送数据
	int SendData(SOCKET _cSock ,DataHeader* header)
	{
		if (isRun() && header) {

			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	
	void SendDataAll( DataHeader* header)
	{
		if (isRun() && header) {
			for (int i = 0;i < (int)g_clients.size();i++) {

				SendData(g_clients[i], header);
			}
		}
	}


};



#endif // !_EasyTcpServer_hpp_
