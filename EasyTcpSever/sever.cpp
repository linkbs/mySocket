#include "EasyTcpSever.hpp"
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
int main() {
  
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(5);
    server.Start();

    std::thread t1(cmdThread);
    t1.detach();
    while (g_bRun)
    {
   
        server.OnRun();
       
       //printf("����ʱ�䣬��������ҵ��\n");
        

    }
    server.Close();
    printf("���˳���\n");
    return 0;

 }


