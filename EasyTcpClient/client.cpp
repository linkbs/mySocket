#include "EasyTcpClient.hpp"
#include<thread>


int lcount = 0;
bool g_bRun = true;
void cmdThread()
{
    while (true) {
        //线程thread
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            g_bRun = false;
            printf("退出cmdTread线程\n");
            break;

        }
      
        else {
            printf("不支持命令。\n");

        }
    }
}
const int cCount = 10000;
//客户端数量
const int tCount = 4;
//线程数量
EasyTcpClient* client[cCount];

void sendThread(int id) //1~4线程 
{
    int c = cCount / tCount;
    int begin = (id - 1) * c;
    int end = id * c;

    
    //client.initSocket();创建新sock时使用
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
            //printf("空闲时间处理其他业务..\n");
            client[i]->SendData(login,sizeof(login));
            //client[i]->OnRun();
        }
    }

    for (int i = begin;i < end;i++) {
        //printf("空闲时间处理其他业务..\n");
        client[i]->Close();
        delete client[i];
    }

}
int main() 
{
 
  
    //UI线程处理输入
    std::thread t0(cmdThread);
    t0.detach();

    //启动发送线程
    for (int i = 0;i < tCount;i++) {
        std::thread t1(sendThread,i+1);
        t1.detach();
    }
    while (g_bRun)
        Sleep(100);
  
    printf("客户端已经退出任务结束");
    return 0;


}
