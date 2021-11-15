#include "EasyTcpSever.hpp"
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
int main() {
  
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(5);
    DataHeader header;
    LoginResult rlogin;

    std::thread t1(cmdThread);
    t1.detach();
    while (g_bRun)
    {
   
        server.OnRun();
        server.SendDataAll(&rlogin);
       //printf("空闲时间，处理其他业务\n");
        

    }
    server.Close();
    printf("已退出。\n");
    getchar();
    return 0;

 }


