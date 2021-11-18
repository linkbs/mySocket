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

//��������С��Ԫ��С
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
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SZIE * 10] = {};

	int _lastPos;

};

class INetEvent
{
public:
	//�ͻ����뿪�¼����麯��
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

			//���windows scoket����
			WSACleanup();
			_clients.clear();

		}
	}


	bool OnRun()
	{
		while (isRun()) {

			if (_clientsBuff.size() > 0) 
			{
				//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients.push_back(pClient);
				
				}
				_clientsBuff.clear();
			
			}
			//���û����Ҫ����Ŀͻ��ˣ�����
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//������ socket
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
			//��һ������nfds��һ����������ָfd_set������������������socket���ķ�Χ
			//������������scoketֵ+1,��windows�в�������д0.
			int ret = select(maxSock + 1, &fdRead,0, 0, nullptr);
			if (ret < 0)
			{
				printf("select��������� \n");
				Close();
				return false;
			}
		
			for (int i = 0;i < (int)_clients.size();i++) {

				if (FD_ISSET(_clients[i]->sockfd(), &fdRead))  //��_clients��������ѡ����fdRead��������Ŀͻ��˴���RecvData
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
	//�Ƿ�����
	char szRecv[RECV_BUFF_SZIE] = {};
	int RecvData(ClientSocket* pClient)
	{

		//5 ���տͻ��˵���������,��������
		int nLen = recv(pClient->sockfd(), szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {

			printf("�ͻ���<Scoket = %d>���˳����������. \n", pClient->sockfd());
			return -1;
		}
		//����ȡ�������ݿ������ڶ�����������Ϣ��������
		memcpy(pClient->MsgBuf() + pClient->getLastPos(), szRecv, nLen);
		//��Ϣ������β�������������ݻ�ǰ�ƣ����������ݻ����
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//�жϵڶ��������������Ƿ������Ϣͷ����
		//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
		while (pClient->getLastPos() >= sizeof(DataHeader)) {

			DataHeader* header = (DataHeader*)pClient->MsgBuf();
			//���β�����ڵ�ǰ��Ϣ���ȣ��Ϳ��Դ����ˡ�
			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				//δ�������ݳ���
				cellOnNetMsg(pClient->sockfd(), header);
				memcpy(pClient->MsgBuf(), pClient->MsgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
				//����lastPos Ϊδ�������ݳ���
			}
			else {
				//��Ϣ���Ȳ���һ����Ϣֱ�ӽ���
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
			//printf("�յ��ͻ���<Scoket = %d>����CMD_LOGIN ���ݳ���: %d  userName = %s PassWord = % s\n", _cSock, login->dataLength, login->UserName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			//SendData(cSock, &ret);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("�յ��ͻ���<Scoket = %d>����:CMD_LOGOUT,���ݳ���: %d, userName = %s \n", _cSock, logout->dataLength, logout->UserName);
			//�˳���¼
			LogoutResult ret;
			//SendData(cSock, &ret);

		}
		break;

		default:
			printf("<socket=%d>�յ�δ������Ϣ ���ݳ���: %d \n", cSock, header->dataLength);
			//DataHeader header;
			//SendData(_cSock, &header);
			break;
		}



	}

	void addclient(ClientSocket* pClient)
	{
	    //һ�������߷���˺����������߷���˲ٿ���黺����
		// �����ߺ�������֮��ĳ�ͻ
		//������������ܳ��ָ��ֳ�ͻ
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
    //��ʽ�ͻ�����
	std::vector<ClientSocket*> _clients;
	//����ͻ�����
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
	//��ʼ��Socket
	SOCKET InitSocket()
	{
		//���� win  Sock 2.x����
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32

		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Socketʧ��....\n");
			Close();
		}
		else {

			printf("����Socket�ɹ�...\n");
		}

		return _sock;

	}
	//�󶨶˿�
	int Bind(const char* ip, unsigned short port)
	{
		sockaddr_in _sin = {};  //={}������ʼ��һ������ṹ��ʵ��
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
			printf("���󣬰�����˿�<%d>ʧ��....\n", port);

		}
		else {

			printf("������˿�<%d>�ɹ�...\n", port);
		}
		return ret;
	}
	//�����˿�
	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("Socket=<%d>���󣬼�������˿�ʧ��....\n", _sock);

		}
		else {

			printf("Socket=<%d>�򿪼�������˿ڳɹ�...\n", _sock);
		}
		return ret;

	}
	//���ܿͻ�������
	int Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		//������ʾ��Ч��socket��ַ��

		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == cSock) {
			printf("���󣬽��ܵ���Ч�ͻ���SOCKET..\n");
		}
		else {
			//0NewUserJoin userJoin;
			//SendDataAll(&userJoin);
			
			addClientToCellServer(new ClientSocket(cSock));
			//printf("socket = <%d>�¿ͻ���<%d>����:socket = %d��IP = %s\n",(int)_sock,_clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return (int)cSock;

		}

	void addClientToCellServer(ClientSocket* pClient) 
	{   
		//�ͻ��˼��룬ѡ���ͻ������ٵ������߷����
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
	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET) {
			for (int i = 0; i < (int)_sclients.size();i++)
			{
				closesocket(_sclients[i]->sockfd());
				delete _sclients[i];

			}

			//���windows scoket����
			WSACleanup();
			_sclients.clear();

		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun()) {

			
			//������ socket
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
			//��һ������nfds��һ����������ָfd_set������������������socket���ķ�Χ
			//������������scoketֵ+1,��windows�в�������д0.
			int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//_sock�пɶ��Żᱻ�Ž�fdRead

			if (ret < 0)
			{
				printf("select��������� \n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
				//���տͻ����׽��֣��ͻ��˷���Ϣ���¸�ѭ������
				//ֻ�����������
				//4 accept�ȴ��ͻ�������,���տͻ��˵�socket
			}
			return true;
		}
		return false;

	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	char szRecv[RECV_BUFF_SZIE] = {};
	//�������ݰ� ����ճ�� ��ְ�
	//��Ӧ������Ϣ
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
	
   
	


	//��ָ���ͻ��˷�������
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
