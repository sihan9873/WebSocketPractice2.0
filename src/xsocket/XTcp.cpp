#define _CRT_SECURE_NO_WARNINGS
#include "XTcp.h"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <WinSock2.h> 
#include <WS2tcpip.h> 
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define socklen_t int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define closesocket close
typedef int socket_t;
#endif
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <string>

using namespace std;

/*
封装了 TCP 通信的核心操作:
创建套接字、绑定端口、监听、接收连接、收发数据、关闭连接
并且做了 Windows/Linux 跨平台兼容处理
*/

XTcp::XTcp() {
#ifdef _WIN32
    static bool first = true;
    if (first) {
        first = false;
        WSADATA ws;
        WSAStartup(MAKEWORD(2, 2), &ws);
    }
#endif
}

//创建 TCP 套接字
int XTcp::CreateSocket() {
    //创建socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("create socket failed!\n");
    }
    printf("[%d] \n", sock);
    return sock;
}

//绑定端口号，同时调用 listen() 开始监听客户端连接
bool XTcp::Bind(unsigned short port) {
    if (sock <= 0) {
        CreateSocket();
    }
    //配置服务端地址
    //定义一个 IPv4 地址结构体变量
    sockaddr_in saddr;
    //设置地址格式
    saddr.sin_family = AF_INET;
    //设置端口号，并转换为网络字节序
    saddr.sin_port = htons(port);
    //设置IP地址为「本机任意地址」，并转换为网络字节序
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //saddr.sin_addr.s_addr = inet_addr("192.168.199.163");

    //绑定Socket和地址
    if (::bind(sock, (sockaddr*)&saddr, sizeof(saddr)) != 0) {
        printf("bind port %d failed!\n", port);
        return false;
    }
    printf("bind port %d success!\n", port);

    //监听,接收用户的连接
    //参数:套接字描述符;套接字请求队列的长度,是一个缓冲大小

    listen(sock, 10);

    return true;
}

//阻塞等待客户端连接
XTcp XTcp::Accept() {
    XTcp tcp;

    sockaddr_in caddr;
#ifdef _WIN32
    int len = sizeof(caddr);
#else
    socklen_t len = sizeof(caddr);
#endif

    //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int client = ::accept(sock, (sockaddr*)&caddr, &len);
    if (client <= 0) {
        return tcp;
    }
    printf("accept client %d\n", client);
    //tcp.ip = inet_ntoa(caddr.sin_addr);
    char * ip = inet_ntoa(caddr.sin_addr);
    strcpy(tcp.ip, ip);
    tcp.port = ntohs(caddr.sin_port);
    tcp.sock = client;
    printf("client ip is %s,port is %d\n", tcp.ip, tcp.port);

    return tcp;
}

//关闭套接字，释放资源
void XTcp::Close() {
    if (sock <= 0) {
        return;
    }
    closesocket(sock);
    sock = 0;
}

//从客户端接收数据，封装底层 recv() 函数
int XTcp::Recv(char* buf, int bufsize) {

    return recv(sock, buf, bufsize, 0);
}

/*
//向客户端发送数据，循环发送确保所有数据都发出去
int XTcp::Send(const char* buf, int sendsize) {
    if (buf == nullptr || sendsize <= 0) return 0;

    int sendedSize = 0;
    while (sendedSize != sendsize) {
        int len = send(sock, buf + sendedSize, sendsize - sendedSize, 0);
        if (len <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 如果是非阻塞模式，这里应该退出并等待下一次 EPOLLOUT
                // 但现在为了简单，我们可以稍微 continue 或者处理逻辑
                continue;
            }
            break;
        }
        sendedSize += len;
    }
    return sendedSize;
}
*/

int XTcp::Send(const char* buf, int sendsize) {
    if (buf == nullptr || sendsize <= 0) return 0;
    // 简单的做法：直接发，如果发不完就返回实际长度，不要死循环
    int len = send(sock, buf, sendsize, 0);
    return len;
}

/*
超时处理
网络系统病不稳定可靠,有各种异常状况,需要学会判定处理
比如connect的过程中,三次握手可能失败,可能会超时,会引起阻塞...这就需要超时处理

方案:
1.sockey设置参数,windows,linux中效果不一样
2.用select多路复用,把socket变成非阻塞的模式


*/
bool XTcp::Connect(const char* ip, unsigned short port, int timeouttms) {
    if (sock <= 0) {
        CreateSocket();
    }
    //配置服务端地址
    //定义一个 IPv4 地址结构体变量
    sockaddr_in saddr;
    //设置地址格式
    saddr.sin_family = AF_INET;
    //设置端口号，并转换为网络字节序
    saddr.sin_port = htons(port);
    //设置IP地址
    saddr.sin_addr.s_addr = inet_addr(ip);

    //实际的生产环境中还要做大量的容错处理,但代码量很大,此处只写核心代码
    //改成非阻塞模式
    SetBlock(false);
    //文件句柄数组,保存每个文件句柄当前状态
    fd_set set;

    if (connect(sock, (sockaddr*)&saddr, sizeof(saddr)) != 0) {
        //把文件句柄置空
        FD_ZERO(&set);
        //往文件句柄数组中加入网络(文件)句柄
        FD_SET(sock, &set);
        //select监听这个文件序列是否有可读/可写
        // 超时结构体
        timeval tm;
        tm.tv_sec = 0;
        tm.tv_usec = timeouttms*1000;//传入的6'
        //参数:最大值+1,可读序列,可写序列,错误处理,设置超时结构体
        //成功返回文件描述符数组(sets),失败(比如超时)返回<=0...
        if (select(sock + 1, 0, &set, 0, &tm) <= 0) {
            printf("connect timeout or error!\n");
            printf("connect %s:%d failed:%s\n", ip, port, strerror(errno));
            return false;
        }
        
    }

    //成功
    SetBlock(true);
    printf("connect %s:%d success\n", ip, port);
    return true;
}

bool XTcp::SetBlock(bool isblock) {
    if (sock <= 0) {
        return false;
    }
#ifdef _WIN32 
    //ul=0表示阻塞模式,ul=1表示非阻塞模式
    unsigned long ul = 0;
    if (!isblock) {
        ul = 1;
    }
    ioctlsocket(sock,FIONBIO,&ul);
#else
    //获取文件描述符属性
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    if (isblock) {
        //修改阻塞位
        flags = flags & ~O_NONBLOCK;
    }
    else {
        flags = flags | O_NONBLOCK;
    }
    if (fcntl(sock, F_SETFL, flags) != 0) {
        return false;
    }
#endif

    return true;
}

XTcp::~XTcp()
{

}
