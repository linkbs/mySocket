#include "EasyTcpClient.hpp"
#include<thread>



void cmdThread(EasyTcpClient* client)
{
    while (true) {
        //线程thread
        char cmdBuf[256] = {};
        scanf("%s", cmdBuf);
        if (0 == strcmp(cmdBuf, "exit"))
        {
            client->Close();
            printf("退出cmdTread线程\n");
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
            printf("不支持命令。\n");

        }
    }
}

int main() 
{
 
    EasyTcpClient client;
    //client.initSocket();创建新sock时使用
    client.Connect("127.0.0.1", 4567);
    //子线程处理输入
    std::thread t1(cmdThread, &client);
    t1.detach();

    while (client.isRun())
    {
        client.OnRun();
        //printf("空闲时间处理其他业务..\n");

    }
    client.Close();
    printf("客户端已经退出任务结束");
    getchar();
    return 0;


}
