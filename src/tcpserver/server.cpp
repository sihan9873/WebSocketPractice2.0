#include "XTcp.h"
#include <thread>
#include <stdlib.h>
#include<string.h>

using namespace std;

/*
基于 XTcp 封装的类实现了一个 TCP 服务端，
采用 “主线程监听 + 子线程处理客户端” 的模式，
能同时响应多个客户端的连接和数据交互
*/

//客户端处理线程
class TcpThread {
public:
    ~TcpThread() {
        client.Close();
    }

    //子线程的核心逻辑，循环接收客户端数据并处理
    void Main() {
        //读取数据
        char buf[1024] = { 0 };
        while (1) {
            //读取客户端发送的内容
            int recvlen = client.Recv(buf, sizeof(buf) - 1);
            if (recvlen <= 0) {
                printf("client %d disconnect or recv error\n", client.sock);
                break;
            }
            //buf[recvlen] = '\0';

            //数据处理规则
            //收到"quit"则断开连接
            if (strstr(buf, "quit") != NULL) {
                const char re[] = "quit success!";
                client.Send(re, strlen(re) + 1);
                break;
            }
            // 只有收到换行符，才回复 ok
            if (buf[recvlen - 1] == '\n') {
                int sendlen = client.Send("ok\n", 3);
                //如果接收长度 ≤0：客户端断开或出错，退出循环并关闭连接
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
    //1.设置端口
    unsigned short port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    //2.创建服务端对象，绑定端口并开始监听
    XTcp server;
    server.CreateSocket();
    server.Bind(port);

    //3.无限循环监听客户端连接
    while (1) {
        XTcp client = server.Accept();

        ////4.阻塞等待客户端连接，得到客户端对象
        if (client.sock <= 0) {
            printf("accept get invalid client socket, skip...\n");
            continue;
        }

        //5.创建新线程处理该客户端
        TcpThread* th = new TcpThread();
        th->client = client;
        thread sth(&TcpThread::Main, th);
        sth.detach();

    }

    server.Close();

    getchar();

    return 0;
}

