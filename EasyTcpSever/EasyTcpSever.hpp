#ifndef _EasyTcpServer_hpp_
#define _EasyTcpSever_hpp_


#ifdef _WIN32
#define FD_SETSIZE     10000
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
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

//��������С��Ԫ��С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 1024
#endif // !RECV_BUFF_SZIE



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

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	CELLTimestamp _tTime;
	int _recvCount;
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
			NewUserJoin userJoin;
			SendDataAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			printf("socket = <%d>�¿ͻ���<%d>����:socket = %d��IP = %s\n",(int)_sock,_clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return (int)cSock;

		}
	

	//�ر�Socket
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

			for (int i = 0;i < (int)_clients.size();i++) {

				FD_SET(_clients[i]->sockfd(), &fdRead);
			}
			timeval t = { 0,0 };
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
			for (int i = 0;i < (int)_clients.size();i++) {

				if (FD_ISSET(_clients[i]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[i])) {
						auto iter = _clients.begin() + i;
						if (iter != _clients.end())
						{
							delete _clients[i];
							_clients.erase(iter);
						}

					}
				}
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
		while ( pClient->getLastPos() >= sizeof(DataHeader)) {

			DataHeader* header = (DataHeader*)pClient->MsgBuf();
			//���β�����ڵ�ǰ��Ϣ���ȣ��Ϳ��Դ����ˡ�
			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				//δ�������ݳ���
				OnNetMsg(pClient->sockfd(),header);
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
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock,DataHeader* header) {
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{

			printf("time<%lf>\t������ = <%d>\t_recvCount<%d>\n",t1,_clients.size(),_recvCount);
			_recvCount = 0;
			_tTime.updata();
		}
		
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
	
   
	


	//��ָ���ͻ��˷�������
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
			for (int i = 0;i < (int)_clients.size();i++) {

				SendData(_clients[i]->sockfd(), header);
			}
		}
	}


};



#endif // !_EasyTcpServer_hpp_
