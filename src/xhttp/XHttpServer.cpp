#include "XHttpServer.h"
#include "XHttpClient.h"
#include <thread>

using namespace std;

bool XHttpServer::Start(unsigned short port) {
	isexit = false;
	server.CreateSocket();
	//绑定端口
	//很容易失败,一定要做判断
	if (!server.Bind(port)) {
		return false;
	}
	//开启线程,接收用户连接
	thread sth(&XHttpServer::Main, this);
	//释放主线程资源
	sth.detach();
	return true;
}

//保证线程正常退出  
bool XHttpServer::Stop() {
	isexit = true;
	return true;
}

//在线程中接收用户数据,此函数与线程绑定
void XHttpServer::Main() {
	while (!isexit) {
		XTcp client = server.Accept();
		if (client.sock <= 0) {
			continue;
		}
		XHttpClient* th = new XHttpClient();
		th->Start(client);
	}
}

XHttpServer::XHttpServer() {

}

XHttpServer::~XHttpServer() {

}
