#include "EasyTcpClient.hpp"
#include<thread>


int lcount = 0;
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
const int cCount = 10000;
//�ͻ�������
const int tCount = 4;
//�߳�����
EasyTcpClient* client[cCount];

void sendThread(int id) //1~4�߳� 
{
    int c = cCount / tCount;
    int begin = (id - 1) * c;
    int end = id * c;

    
    //client.initSocket();������sockʱʹ��
    for (int i = begin;i < end;i++)
    {
        if (!g_bRun)
        {
            return;

        }
        client[i] = new EasyTcpClient();

    }
    for (int i = begin;i < end;i++)
    {
        if (!g_bRun)
        {
            return;

        }
        lcount++;
        printf("Connect = %d\n", lcount);
        client[i]->Connect("127.0.0.1", 4567);

    }
    
    std::chrono::microseconds t(3000);
    std::this_thread::sleep_for(t);
    

    Login login[10];
    for (int i = 0;i < 10;i++) {
        strcpy(login[i].UserName, "lyd");
        strcpy(login[i].PassWord, "lydmm");
    }
    const int nLen = sizeof(login);
    while (g_bRun)
    {
        for (int i = begin;i < end;i++) {

            if (!g_bRun)
            {

                return;
            }
            //printf("����ʱ�䴦������ҵ��..\n");
            client[i]->SendData(login,sizeof(login));
            //client[i]->OnRun();
        }
    }

    for (int i = begin;i < end;i++) {
        //printf("����ʱ�䴦������ҵ��..\n");
        client[i]->Close();
        delete client[i];
    }

}
int main() 
{
 
  
    //UI�̴߳�������
    std::thread t0(cmdThread);
    t0.detach();

    //���������߳�
    for (int i = 0;i < tCount;i++) {
        std::thread t1(sendThread,i+1);
        t1.detach();
    }
    while (g_bRun)
        Sleep(100);
  
    printf("�ͻ����Ѿ��˳��������");
    return 0;


}
