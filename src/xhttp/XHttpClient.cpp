#include "XHttpClient.h"
#include <thread>

using namespace std;

XHttpClient::XHttpClient() {

}

XHttpClient::~XHttpClient() {

}

//创建通信线程(也可以在server中做)
bool XHttpClient::Start(XTcp client) {
	this->client = client;
	//绑定入口函数
	thread sth(&XHttpClient::Main, this);
	sth.detach();
	return true;
}


//与客户通信的入口函数
void XHttpClient::Main() {
	//接收数据
    char buf[10240] = { 0 };
	int size = sizeof(buf);

	//HTTP1.1 支持连接
	while (true) {
		int RcvLen = client.Recv(buf, sizeof(buf) - 1);
		if (RcvLen <= 0) {
			break;
		}
		buf[RcvLen] = '\0';

		//分析数据
		if (!res.SetRequest(buf)) {
			break;
		}

		//发送头数据
		string head = res.GetHead(); 
		if (client.Send(head.c_str(), head.size()) <= 0) {
			break;
		}

		//发送内容
		//有个问题,图片/视频数据不便于一次性收发
		//需要循环,每次收一部分,发一部分
		bool error = false;
		while (true) {
			int RdLen = res.Read(buf, size);
			if (RdLen < 0) {
				error = true;
				break;
			}
			if (RdLen == 0) {
				//读到结尾处,终止,进行下一轮处理,不认为是错误
				break;
			}
			if (client.Send(buf, RdLen) <= 0) {
				error = true;
				break;
			}
		}
	}
	
	
	client.Close();
	delete this;
	return;
}