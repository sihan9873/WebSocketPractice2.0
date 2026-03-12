#include "XTcp.h"
#include <thread>
#include <stdlib.h>
#include<string.h>

using namespace std;

class TcpThread {
public:
    ~TcpThread() {
        client.Close();
    }

    void Main() {
        //读取数据
        char buf[1024] = { 0 };
        while (1) {

            int recvlen = client.Recv(buf, sizeof(buf) - 1);
            if (recvlen <= 0) {
                printf("client %d disconnect or recv error\n", client.sock);
                break;
            }
            buf[recvlen] = '\0';

            if (strstr(buf, "quit") != NULL) {
                const char re[] = "quit success!";
                client.Send(re, strlen(re) + 1);
                break;
            }
            // 只有收到换行符，才回复 ok
            if (buf[recvlen - 1] == '\n') {
                int sendlen = client.Send("ok\n", 3);
                if (sendlen <= 0) {
                    printf("send to client %d failed\n", client.sock);
                    break;
                }
            }

            printf("recv from client %d: %s\n", client.sock, buf);
        }
        client.Close();
        //释放new的TcpThread对象,避免内存泄漏
        delete this;
    }

    XTcp client;
};

int main(int argc, char** argv)
{
    unsigned short port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    XTcp server;
    server.CreateSocket();
    server.Bind(port);

    while (1) {
        XTcp client = server.Accept();

        if (client.sock <= 0) {
            printf("accept get invalid client socket, skip...\n");
            continue;
        }

        //创建线程
        TcpThread* th = new TcpThread();
        th->client = client;
        thread sth(&TcpThread::Main, th);
        sth.detach();

    }

    server.Close();

    getchar();

    return 0;
}

