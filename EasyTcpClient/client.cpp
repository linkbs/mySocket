#include "EasyTcpClient.hpp"
#include<thread>



bool g_bRun = true;
void cmdThread()
{
    while (true) {
        //�߳�thread
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            printf("�˳�cmdTread�߳�\n");
            break;

        }
      
        else {
            printf("��֧�����\n");

        }
    }
}

int main() 
{
 
    const int cCount = 63;
    EasyTcpClient* client[cCount];
    //client.initSocket();������sockʱʹ��
    for (int i = 0;i < cCount;i++)
    {
        client[i] = new EasyTcpClient();
        client[i]->Connect("127.0.0.1", 4567);

    }
    //���̴߳�������
    std::thread t1(cmdThread);
    t1.detach();

    Login login;
    strcpy(login.UserName, "lyd");
    strcpy(login.PassWord, "lydmm");
    while (g_bRun)
    {
        for (int i = 0;i < cCount;i++) {
             client[i]->OnRun();
            //printf("����ʱ�䴦������ҵ��..\n");
             client[i]->SendData(&login);
        }
    }

    for (int i = 0;i < cCount;i++) {
        //printf("����ʱ�䴦������ҵ��..\n");
        client[i]->Close();
    }
    printf("�ͻ����Ѿ��˳��������");
    getchar();
    return 0;


}
