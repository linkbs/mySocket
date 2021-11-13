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
	//������
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//��ʼ��socket
	void initSocket()
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
	
	}

	//���ӷ�����
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
			printf("���󣬽���Socketʧ��....\n");
		}
		else {
			printf("��ȷ������Socket�ɹ�.....\n");

		}
		return ret;
	
	}

	//�ر�Sockt
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
		//�ر� win  Sock 2.x����
}
	
	//��ѯ������Ϣ
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
				printf("<socket=%d>select�������", _sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{

				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������2\n", _sock);
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
	


	//�������� ����ճ�� ���
	int RecvData(SOCKET _cSock)
	{
		char szRecv[4096] = {};
		//5 ���տͻ��˵���������,��������
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {

			printf("��������Ͽ����ӣ��������. \n", _cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
	
		onNetMsg( header);
		return 0;

	}
	//��Ӧ������Ϣ
	void onNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//����ָ��ת��Ϊ����ָ�룬����ָ��ָ������󣬵����������Ա
			//���Ҫ����Ա�������Ƿ�������д�ĺ��������࣬������Լ��ġ�
			LoginResult* login = (LoginResult*)header;
			printf("�յ�����������Ϣ��CMD_LOGIN_RESULT ���ݳ���: %d \n", login->dataLength);

		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			
			LogoutResult* logout = (LogoutResult*)header;
			printf("�յ�����������Ϣ��CMD_LOGOUT_RESULT ���ݳ���: %d \n", logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("�յ�����������Ϣ��CMD_NEW_USER_JOIN ���ݳ���: %d \n", userJoin->dataLength);

		}
		break;



		}
	
	}
	//��������
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