#include "EasyTcpSever.hpp"

int main() {
  
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(5);
   
    while (server.isRun())
    {
   
        server.OnRun();
       //printf("空闲时间，处理其他业务\n");
        

    }
    server.Close();
    printf("已退出。\n");
    getchar();
    return 0;

 }


