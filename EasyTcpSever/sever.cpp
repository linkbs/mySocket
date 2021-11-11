#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>

#include<vector>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
//�ýṹ����ͻ��˷�����Ϣ
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


vector<SOCKET> g_clients;

int processor(SOCKET _cSock) 
{
    char szRecv[4096] = {};
    //5 ���տͻ��˵���������,��������
    int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {

        printf("�ͻ������˳����������");
        return -1;
    }

    switch (header->cmd)
    {
    case CMD_LOGIN:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Login* login = (Login*)szRecv;
        printf("�յ����CMD_LOGIN ���ݳ���: %d  userName = %s PassWord = % s\n", login->dataLength, login->UserName, login->PassWord);
        //�����ж��û������Ƿ���ȷ�Ĺ���
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(LoginResult), 0);

    }
    break;
    case CMD_LOGOUT:
    {
        recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Logout* logout = (Logout*)szRecv;
        printf("�յ�����:CMD_LOGIN,���ݳ���: %d, userName = %s \n", logout->dataLength, logout->UserName);
        //�˳���¼
        LoginResult ret;
        send(_cSock, (char*)&ret, sizeof(ret), 0);

    }
    break;

    default:
        DataHeader header = { 0,CMD_ERROR };
        send(_cSock, (char*)&header, sizeof(header), 0);
        break;
    }
   





}


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

        printf("�򿪼�������˿ڳɹ�...\n");
    }
   
   
    while (true)
    {
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
       
        //��һ������nfds��һ����������ָfd_set������������������socket���ķ�Χ
        //������������scoketֵ+1,��windows�в�������д0.
        int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
        if (ret < 0)
        {
            printf("select��������� \n");
            break;
        }
        if (FD_ISSET(_sock, &fdRead))                  
        {
            FD_CLR(_sock, &fdRead);       
            //ֻ�����������
            //4 accept�ȴ��ͻ�������,���տͻ��˵�socket
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
            //������ʾ��Ч��socket��ַ��

            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
            if (INVALID_SOCKET == _cSock) {
                printf("���󣬽��ܵ���Ч�ͻ���SOCKET..\n");
            }
            g_clients.push_back(_cSock);
            printf("�¿ͻ��˼���:socket = %d��IP = %s\n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

        }
        for (int i = 0;i < (int)fdRead.fd_count;i++)
        {
            if (-1 == processor(fdRead.fd_array[i])) 
            {
               auto iter = find(g_clients.begin(),g_clients.end(),fdRead.fd_array[i]);
                   if (iter != g_clients.end())
                   {
                       g_clients.erase(iter);
                   }
            
            }
        }
           

    }

    for (int i= 0; i < (int)g_clients.size();i++) 
    {
        closesocket(g_clients[i]);
    
    }

    //���windows scoket����
    WSACleanup();
    printf("������Ѿ��˳����������");
    getchar();
    return 0;

    
}


