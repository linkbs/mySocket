#include "EasyTcpClient.hpp"
#include<thread>



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

int main() 
{
 
    const int cCount = 63;
    EasyTcpClient* client[cCount];
    //client.initSocket();创建新sock时使用
    for (int i = 0;i < cCount;i++)
    {
        client[i] = new EasyTcpClient();
        client[i]->Connect("127.0.0.1", 4567);

    }
    //子线程处理输入
    std::thread t1(cmdThread);
    t1.detach();

    Login login;
    strcpy(login.UserName, "lyd");
    strcpy(login.PassWord, "lydmm");
    while (g_bRun)
    {
        for (int i = 0;i < cCount;i++) {
             client[i]->OnRun();
            //printf("空闲时间处理其他业务..\n");
             client[i]->SendData(&login);
        }
    }

    for (int i = 0;i < cCount;i++) {
        //printf("空闲时间处理其他业务..\n");
        client[i]->Close();
    }
    printf("客户端已经退出任务结束");
    getchar();
    return 0;


}
