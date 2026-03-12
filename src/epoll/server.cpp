#include "XTcp.h"
#include <thread>
#include <stdlib.h>
#include<string.h>
#ifdef _WIN32

#else
#include <sys/epoll.h>
#include <errno.h>
#endif

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
#ifdef _WIN32

#else

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

    //通过epoll接收用户连接,替代多线程
    //1.创建epoll,其中最多配置256个套接字
    int epfd = epoll_create(256);

    //注册事件
    //事件结构体
    epoll_event ev;
    //把相应的socket绑定到事件上
    ev.data.fd = server.sock;
    //指定类型
    ev.events = EPOLLIN | EPOLLET;//边缘触发

    //注册
    epoll_ctl(epfd, EPOLL_CTL_ADD, server.sock, &ev);
    //用于存储事件,最多等待20个就会返回
    epoll_event events[20];

    //接收缓冲区
    char buf[1024] = { 0 };
    //回复消息
    const char* msg = "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nX";
/*
    const char* msg =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n"
        "\r\n"
        "hello";
*/
    int size = strlen(msg);

    server.SetBlock(false);

    //3.无限循环监听客户端连接
    while (1) {
        //2.等待
        int count = epoll_wait(epfd, events, 20,500);//超时时间500ms,过了一次代码就会往下走
        if (count <= 0) {
            continue;
        }
        for (int i = 0; i < count; i++) {
            //遍历事件,有两种类型:建立连接/接收数据
            //events[i].data.fd即socket
            if (events[i].data.fd == server.sock) {
                //建立连接
                XTcp client = server.Accept();
                if (client.sock <= 0) {
                    break;
                }
                
                //产生新的socket也需要在epoll中注册
                //当然,使用完需要清理,否则epoll池子epfd会满
                ev.data.fd = client.sock;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client.sock, &ev);
            }
            else {
                XTcp client;
                client.sock = events[i].data.fd;
                client.Recv(buf, 1024);
                client.Send(msg, size);
                // HTTP/1.0 默认短连接，直接关闭
                epoll_ctl(epfd, EPOLL_CTL_DEL, client.sock, NULL);
                client.Close();
            }
        }
    }

    server.Close();
    getchar();
    return 0;
}

#endif


