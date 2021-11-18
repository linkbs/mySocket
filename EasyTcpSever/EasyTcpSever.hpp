#ifndef _EasyTcpServer_hpp_
#define _EasyTcpSever_hpp_


#ifdef _WIN32
#define FD_SETSIZE     10006
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
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
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 1024
#endif // !RECV_BUFF_SZIE
#define _CellServer_THREAD_COUNT   4




class ClientSocket
{
public:

	ClientSocket(SOCKET sockfd = INVALID_SOCKET) 
	{
	 
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* MsgBuf() {
	
		return _szMsgBuf;
	}

	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}
private:
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE * 10] = {};

	int _lastPos;

};

class INetEvent
{
public:
	//客户端离开事件纯虚函数
	virtual void OnLeave(ClientSocket* pClient) = 0;
	virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header) = 0;
};

class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET) 
	{
		_sock = sock;
		_pThread = nullptr;
		_recvCount = 0;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
		delete _pThread;

	}
	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET) {
			for (int i = 0; i < (int)_clients.size();i++)
			{
				closesocket(_clients[i]->sockfd());
				delete _clients[i];

			}

			//清除windows scoket环境
			WSACleanup();
			_clients.clear();

		}
	}


	bool OnRun()
	{
		while (isRun()) {

			if (_clientsBuff.size() > 0) 
			{
				//从缓冲队列中取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				
				}
				_clientsBuff.clear();
			
			}
			//如果没有需要处理的客户端，跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利 socket
			fd_set fdRead;
		

			FD_ZERO(&fdRead);

			SOCKET maxSock = _clients[0]->sockfd();
			for (int i = 0;i < (int)_clients.size();i++) {

				FD_SET(_clients[i]->sockfd(), &fdRead);
				if (maxSock < _clients[i]->sockfd())
				{
					maxSock = _clients[i]->sockfd();
				}
			}
			//第一个参数nfds是一个整数，是指fd_set集合中所以描述符（socket）的范围
			//就是里面最大的scoket值+1,在windows中参数可以写0.
			int ret = select(maxSock + 1, &fdRead,0, 0, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。 \n");
				Close();
				return false;
			}
		
			for (int i = 0;i < (int)_clients.size();i++) {

				if (FD_ISSET(_clients[i]->sockfd(), &fdRead))  //在_clients数组里面选择在fdRead集合里面的客户端传入RecvData
				{
					if (-1 == RecvData(_clients[i])) {
						auto iter = _clients.begin() + i;
						if (iter != _clients.end())
						{
							if (_pNetEvent) {
								_pNetEvent->OnLeave(_clients[i]);
							}
							delete _clients[i];
							_clients.erase(iter);
						}

					}
				}
			}

			
		}


	}
	//是否工作中
	char szRecv[RECV_BUFF_SZIE] = {};
	int RecvData(ClientSocket* pClient)
	{

		//5 接收客户端的请求数据,处理请求
		int nLen = recv(pClient->sockfd(), szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {

			printf("客户端<Scoket = %d>已退出，任务结束. \n", pClient->sockfd());
			return -1;
		}
		//将收取到的数据拷贝到第二缓冲区（消息缓冲区）
		memcpy(pClient->MsgBuf() + pClient->getLastPos(), szRecv, nLen);
		//消息缓冲区尾部，处理完数据会前移，拷贝到数据会后移
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//判断第二缓冲区的数据是否大于消息头长度
		//这时就可以知道当前消息的长度
		while (pClient->getLastPos() >= sizeof(DataHeader)) {

			DataHeader* header = (DataHeader*)pClient->MsgBuf();
			//如果尾部大于当前消息长度，就可以处理了。
			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				//未处理数据长度
				cellOnNetMsg(pClient->sockfd(), header);
				memcpy(pClient->MsgBuf(), pClient->MsgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
				//更新lastPos 为未处理数据长度
			}
			else {
				//消息长度不够一条消息直接结束
				break;
			}
		}
		return 0;

	}


	virtual void cellOnNetMsg(SOCKET cSock, DataHeader* header) {
		_recvCount++;

		_pNetEvent->OnNetMsg(new ClientSocket(cSock), header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("收到客户端<Scoket = %d>请求：CMD_LOGIN 数据长度: %d  userName = %s PassWord = % s\n", _cSock, login->dataLength, login->UserName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			//SendData(cSock, &ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<Scoket = %d>请求:CMD_LOGOUT,数据长度: %d, userName = %s \n", _cSock, logout->dataLength, logout->UserName);
			//退出登录
			LogoutResult ret;
			//SendData(cSock, &ret);

		}
		break;

		default:
			printf("<socket=%d>收到未定义消息 数据长度: %d \n", cSock, header->dataLength);
			//DataHeader header;
			//SendData(_cSock, &header);
			break;
		}



	}

	void addclient(ClientSocket* pClient)
	{
	    //一个生产者服务端和三个消费者服务端操控这块缓冲区
		// 生产者和消费者之间的冲突
		//如果不加锁可能出现各种冲突
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();

	}


	void Start() 
	{
		
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun),this);
	    
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
    //正式客户队列
	std::vector<ClientSocket*> _clients;
	//缓冲客户队列
	std::vector<ClientSocket*> _clientsBuff;
	std::mutex _mutex;
	SOCKET _sock;
	std::thread* _pThread;
	INetEvent* _pNetEvent;
public:
	std::atomic_int _recvCount;
};



class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _sclients;
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime;
	
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
		SOCKET cSock = INVALID_SOCKET;
		//这个宏表示无效的socket地址。

		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == cSock) {
			printf("错误，接受到无效客户端SOCKET..\n");
		}
		else {
			//0NewUserJoin userJoin;
			//SendDataAll(&userJoin);
			
			addClientToCellServer(new ClientSocket(cSock));
			//printf("socket = <%d>新客户端<%d>加入:socket = %d，IP = %s\n",(int)_sock,_clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return (int)cSock;

		}

	void addClientToCellServer(ClientSocket* pClient) 
	{   
		//客户端加入，选出客户端最少的消费者服务端
		_sclients.push_back(pClient);

		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers) 
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
				pMinServer = pCellServer;
		
		}

		pMinServer->addclient(pClient);
	
	}
	
	void Start()
	{


		for (int i = 0;i < _CellServer_THREAD_COUNT;i++)
		{
		
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			ser->setEventObj(this);
			ser->Start();
		}
	
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET) {
			for (int i = 0; i < (int)_sclients.size();i++)
			{
				closesocket(_sclients[i]->sockfd());
				delete _sclients[i];

			}

			//清除windows scoket环境
			WSACleanup();
			_sclients.clear();

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


			timeval t = { 0,10 };
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
				return true;
				//接收客户端套接字，客户端发消息在下个循环处理
				//只是清理计数器
				//4 accept等待客户端连接,接收客户端的socket
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

	char szRecv[RECV_BUFF_SZIE] = {};
	//接收数据包 处理粘包 拆分包
	//响应网络消息
     void time4msg() {
		 auto t1 = _tTime.getElapsedSecond();
		 if (t1 >= 1.0)
		 {
			int recvCount = 0;
		    for (auto ser : _cellServers) {
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
			printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n",_cellServers.size(), t1, _sock, _sclients.size(),recvCount);
			_tTime.updata();
		 
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
	
	/*void SendDataAll(DataHeader* header)
	{
		if (isRun() && header) {
			for (int i = 0;i < (int)_clients.size();i++) {

				SendData(_clients[i]->sockfd(), header);
			}
		}
	}

	*/
	virtual void OnLeave(ClientSocket* pClient)
	{
		for (int i = 0;i < (int)_sclients.size();i++) {

			if (_sclients[i] == pClient)
			{
			    auto iter = _sclients.begin() + i;
				if (iter != _sclients.end()) 
				{
					_sclients.erase(iter);
				}
			}
		}
	}
	virtual void OnNetMsg(ClientSocket* pClient,DataHeader* header)
	{
		time4msg();
	}
};



#endif // !_EasyTcpServer_hpp_
