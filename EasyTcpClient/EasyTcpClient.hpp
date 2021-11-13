#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>


#define SOCKET int
#define INVALID_SOCKET   (SOCKET)(~0)
#define SOCKET_ERROR             (-1)
#endif
#include<string.h>
#include <vector>
#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient 
{
	SOCKET _sock;
public:
	EasyTcpClient() 
	{
	   
		_sock = INVALID_SOCKET;
	
	}
	//虚析构
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void initSocket()
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
	
	}

	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) 
		{
			initSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32

		
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("错误，建立Socket失败....\n");
		}
		else {
			printf("正确，建立Socket成功.....\n");

		}
		return ret;
	
	}

	//关闭Sockt
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);

			WSACleanup();

#else
			close(_sock);
#endif // _WIN32
			_sock = INVALID_SOCKET;
		}
		//关闭 win  Sock 2.x环境
}
	
	//查询网络消息
	bool OnRun()
	{
		if (isRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select任务结束", _sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{

				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束2\n", _sock);
					return false;
				}
			}
			return true;
		}
		return false;

	}
	bool isRun() {
	
		return _sock != INVALID_SOCKET;
	}
	


	//接收数据 处理粘包 拆包
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		//5 接收客户端的请求数据,处理请求
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {

			printf("与服务器断开链接，任务结束. \n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
	
		onNetMsg( header);
		return 0;

	}
	//响应网络消息
	void onNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//父类指针转化为子类指针，子类指针指向父类对象，调的是子类成员
			//如果要调成员函数，那发生过重写的函数调父类，其余调自己的。
			LoginResult* login = (LoginResult*)header;
			printf("收到服务器端消息：CMD_LOGIN_RESULT 数据长度: %d \n", login->dataLength);

		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			
			LogoutResult* logout = (LogoutResult*)header;
			printf("收到服务器端消息：CMD_LOGOUT_RESULT 数据长度: %d \n", logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("收到服务器端消息：CMD_NEW_USER_JOIN 数据长度: %d \n", userJoin->dataLength);

		}
		break;



		}
	
	}
	//发送数据
	int SendData(DataHeader* header) 
	{
		if (isRun() && header) {

		   send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:

};

#endif