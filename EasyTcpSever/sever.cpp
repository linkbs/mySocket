#include "EasyTcpSever.hpp"

int main() {
  
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(nullptr, 4567);
    server.Listen(5);
   
    while (server.isRun())
    {
   
        server.OnRun();
       //printf("����ʱ�䣬��������ҵ��\n");
        

    }
    server.Close();
    printf("���˳���\n");
    getchar();
    return 0;

 }


