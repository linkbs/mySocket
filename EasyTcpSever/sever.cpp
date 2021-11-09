#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

//�ýṹ����ͻ��˷�����Ϣ
enum CMD
{
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

struct DataHeader
{
    short dataLength;
    short cmd;

};


//DataPackage
struct  Login
{
    char UserName[32];
    char PassWord[32];
    char name[32];

};

struct LoginResult
{
    int result;
};

struct Logout {

    char UserName[32];
};

struct LogoutResult
{

    int result;
};


int main() {
     //����Windows socket 2.x����
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);

    //-��Scoket API��������TCP�����
    //1.����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    sockaddr_in _sin = {};  //={}������ʼ��һ������ṹ��ʵ��
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
 
    if(SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("���󣬰�����˿�ʧ��....\n");
    
    }
    else {
    
        printf("������˿ڳɹ�...\n");
    }
    
    if(SOCKET_ERROR ==listen(_sock, 5))
    {
        printf("���󣬼�������˿�ʧ��....\n");

    }
    else {

        printf("��������˿ڳɹ�...\n");
    }
    //���տͻ��˷��ص�socket
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(sockaddr_in);
    SOCKET _cSock = INVALID_SOCKET;
    //������ʾ��Ч��socket��ַ��

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("���󣬽��ܵ���Ч�ͻ���SOCKET..\n");
    }
    printf("�¿ͻ��˼���:socket = %d��IP = %s\n", (int)_cSock,inet_ntoa(clientAddr.sin_addr));

   
    while (true)
    {
        DataHeader header = {};
        //5 ���տͻ��˵���������
        int nLen = recv(_cSock, (char*)&header, sizeof(header), 0);
        if (nLen <= 0) {
        
            printf("�ͻ������˳����������");
            break;
        }
        printf("�յ�����:%d ���ݳ���:%d \n", header.cmd,header.dataLength);
        switch (header.cmd) 
        {
        case CMD_LOGIN:
        {
            Login login = {};
            recv(_cSock, (char*)&login, sizeof(Login), 0);
            //�����ж��û������Ƿ���ȷ�Ĺ���
            LoginResult ret = {1};
            send(_cSock, (char*)&header, sizeof(DataHeader), 0);
            send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

        }
        break;
        case CMD_LOGOUT:
        {
            Logout logout = {};
            recv(_cSock, (char*)&logout, sizeof(logout), 0);
            //�˳���¼
            LoginResult ret = { 1 };
            send(_cSock, (char*)&header, sizeof(header), 0);
            send(_cSock, (char*)&ret, sizeof(ret), 0);

        }
        break;
        
        default:
            header.cmd = CMD_ERROR;
            header.dataLength = 0;
            send(_cSock, (char*)&header, sizeof(header), 0);
        break;
        }
         //6 ��������
       
        

        

    }

    closesocket(_sock);

    //���windows scoket����
    WSACleanup();
    printf("������Ѿ��˳����������");
    getchar();
    return 0;

    
}


