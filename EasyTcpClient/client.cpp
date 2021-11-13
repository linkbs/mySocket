#include "EasyTcpClient.hpp"
#include<thread>



void cmdThread(EasyTcpClient* client)
{
    while (true) {
        //�߳�thread
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            client->Close();
            printf("�˳�cmdTread�߳�\n");
            break;

        }
        else if (0 == strcmp(cmdBuf, "login"))
        {
            Login login;
            strcpy(login.UserName, "lyd");
            strcpy(login.PassWord, "lydmm");
            client->SendData(&login);

        }
        else if (0 == strcmp(cmdBuf, "logout"))
        {
            Logout logout;
            strcpy(logout.UserName, "lyd");
            client->SendData(&logout);

        }
        else {
            printf("��֧�����\n");

        }
    }
}

int main() 
{
 
    EasyTcpClient client;
    //client.initSocket();������sockʱʹ��
    client.Connect("127.0.0.1", 4567);
    //���̴߳�������
    std::thread t1(cmdThread, &client);
    t1.detach();

    while (client.isRun())
    {
        client.OnRun();
        //printf("����ʱ�䴦������ҵ��..\n");

    }
    client.Close();
    printf("�ͻ����Ѿ��˳��������");
    getchar();
    return 0;


}
