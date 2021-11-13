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
		SOCKET _cSock = INVALID_SOCKET;
		//������ʾ��Ч��socket��ַ��

		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
		if (INVALID_SOCKET == _cSock) {
			printf("���󣬽��ܵ���Ч�ͻ���SOCKET..\n");
		}
		else {
			NewUserJoin userJoin;
			SendDataAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("�¿ͻ��˼���:socket = %d��IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
		}
		return (int)_cSock;

		}
	

	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET) {
			for (int i = 0; i < (int)g_clients.size();i++)
			{
				closesocket(g_clients[i]);

			}

			//���windows scoket����
			WSACleanup();
			printf("������Ѿ��˳����������");
			getchar();

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

			for (int i = 0;i < (int)g_clients.size();i++) {

				FD_SET(g_clients[i], &fdRead);
			}
			timeval t = { 2,0 };
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
				//���տͻ����׽��֣��ͻ��˷���Ϣ���¸�ѭ������
				//ֻ�����������
				//4 accept�ȴ��ͻ�������,���տͻ��˵�socket
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
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//�������ݰ� ����ճ�� ��ְ�
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		//5 ���տͻ��˵���������,��������
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {

			printf("�ͻ���<Scoket = %d>���˳����������. \n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);

		OnNetMsg(_cSock, header);
		return 0;

	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock,DataHeader* header) {
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("�յ��ͻ���<Scoket = %d>����CMD_LOGIN ���ݳ���: %d  userName = %s PassWord = % s\n", _cSock, login->dataLength, login->UserName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			LoginResult ret;
			send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("�յ��ͻ���<Scoket = %d>����:CMD_LOGOUT,���ݳ���: %d, userName = %s \n", _cSock, logout->dataLength, logout->UserName);
			//�˳���¼
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
			for (int i = 0;i < (int)g_clients.size();i++) {

				SendData(g_clients[i], header);
			}
		}
	}


};



#endif // !_EasyTcpServer_hpp_
