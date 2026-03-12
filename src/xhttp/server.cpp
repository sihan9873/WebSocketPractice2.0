#define _CRT_SECURE_NO_WARNINGS
#include "XTcp.h"
#include "XHttpServer.h"
#include <thread>
#include <stdlib.h>
#include<string.h>
#include<string>
#include<regex>
#include<signal.h>



using namespace std;

/*
基于 XTcp 封装的类实现了一个 TCP 服务端，
采用 “主线程监听 + 子线程处理客户端” 的模式，
能同时响应多个客户端的连接和数据交互
*/

/*
//客户端处理线程
class HttpThread {
public:
    //子线程的核心逻辑，循环接收客户端数据并处理
    void Main() {
        //收发数据
        char buf[10000] = { 0 };

        //实现http1.1是比较复杂的,但可以实现保持链接,只需要一直循环
        while (true) {
            //接收http客户端请求
            int recvLen = client.Recv(buf, sizeof(buf) - 1);
            //退出循环,关闭连接的方式也极致简化:
            //没有接收的信息则退出
            //当然一般的GET请求会发送关闭连接的信息
            if (recvLen <= 0) {
                //接收失败
                //关闭客服端套接字资源
                client.Close();
                //关闭服务端资源
                delete this;
                return;
            }
            //结尾记得添加终止符!后需要转化成字符串
            buf[recvLen] = '\0';
            printf("========recv==========\n");
            printf("%s", buf);
            printf("======================\n");

            //根据类型进行回应
            //类型: / /index.html /ms/index.html /ff/index.php /ff
            //新增对php脚本语言的解析
            //新增对url的解析
            //GET /index.php?id=1&name=xcj HTTP/1.1 
            string src = buf;//源
            //string pattern = "^([A-Z]+) (.+) HTTP/1";//模板1
            //string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.].*)?) HTTP/1";//模板2
            // GET / 文件名.类型名?键值对 HTTP/1(.1)
            string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.][a-zA-Z0-9]*)?)[?]?(.*) HTTP/1"; //模板3
            regex r(pattern);//正则表达式
            smatch mas;//结果集
            regex_search(src, mas, r);//匹配
            if (mas.size() == 0) {
                //匹配失败
                printf("%s failed!\n", pattern.c_str());
                //直接退出
                Close();
                return;
            }
            string type = mas[1];
            string path = "/";
            path += mas[2];
            string filetype = mas[3];
            string query = mas[4];

            if (filetype.size() > 0) {
                filetype = filetype.substr(1);
            }
            //输出调试信息
            printf("type: %s\n", type.c_str());
            printf("file path: %s\n", path.c_str());
            printf("file type: %s\n", filetype.c_str());
            printf("query: %s\n", query.c_str());
            if (type != "GET") {
                //本程序只处理GET,其他直接退出
                printf("Not GET!!!\n");
                Close();
                return;
            }
            string filename = path;
            if (filename == "/") {
                filename = "/index.html";
            }

            string filepath = "www";
            filepath += filename;

            if (filetype == "php") {
                //php-cgi www/index.php > www/index.php.html
                string cmd = "php-cgi ";
                cmd += filepath;
                cmd += " ";
                //添加参数传递
                //query id=1&name=xcj,把&换成空格
                for (int i = 0; i < query.size(); i++) {
                    if (query[i] == '&') {
                        query[i] = ' ';
                    }
                }
                cmd += query;
                cmd += " > ";
                filepath += ".html";
                cmd += filepath;

                printf("%s\n", cmd.c_str());
                system(cmd.c_str());
            }

            FILE* fp = fopen(filepath.c_str(), "rb");
            if (fp == NULL) {
                //如果返回404错误,还需要返回404页面
                //此处简化处理,直接退出
                printf("open file %s failed!!!\n",filepath.c_str());
                Close();
                return;
            }

            //获取文件大小,修改报文
            fseek(fp, 0, SEEK_END);
            int filesize = ftell(fp);//有缺陷的写法,受int表示范围限制
            fseek(fp, 0, 0);//恢复文件指针位置
            printf("file size is %d\n", filesize);

            //处理php文件头
            if (filetype == "php") {
                char c = 0;
                //找\r\n\r\n
                int headsize = 0;
                while (fread(&c, 1, 1, fp) > 0) {
                    headsize++;
                    if (c == '\r') {
                        fseek(fp, 3, SEEK_CUR);
                        headsize += 3;
                        break;
                    }
                }
                filesize = filesize - headsize;
            }
            

            //回应GET请求,创建http响应报文
            //消息头
            string rmsg = "";
            rmsg = "HTTP/1.1 200 OK\r\n";
            rmsg += "Server: XHttp\r\n";
            rmsg += "Content-Type: text/html\r\n";
            //rmsg += "Content-Length: 10\r\n";
            rmsg += "Content-Length: ";
            //把文件大小转成字符串
            char bsize[128] = { 0 };
            sprintf(bsize, "%d", filesize);
            rmsg += bsize;
            rmsg += "\r\n";
            rmsg += "\r\n";
            //rmsg += "0123456789";

            //发送消息头
            int sendSize = client.Send(rmsg.c_str(), rmsg.size());
            printf("sendsize = %d\n", sendSize);
            printf("========send==========\n");
            printf("%s\n", rmsg.c_str());
            printf("======================\n");

            //发送正文
            while (true) {
                int len = fread(buf, 1, sizeof(buf), fp);
                if (len <= 0) {
                    //已读完数据
                    break;
                }
                int re = client.Send(buf, len);
                if (re <= 0) {
                    break;
                }
            }
        }

        Close();
    }

    void Close() {
        client.Close();
        delete this;
    }

    XTcp client;
};
*/


int main(int argc, char** argv)
{
    //0.忽略信号

#ifdef _WIN32

#else
    signal(SIGPIPE, SIG_IGN);
#endif

    //1.设置端口
    unsigned short port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

/*
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
        HttpThread* th = new HttpThread();
        th->client = client;
        thread sth(&HttpThread::Main, th);
        sth.detach();

    }
    server.Close();
*/

    XHttpServer server;
    server.Start(port);

    getchar();
    return 0;
}

