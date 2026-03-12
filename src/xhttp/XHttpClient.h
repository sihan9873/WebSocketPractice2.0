#pragma once
#include "XTcp.h"
#include "XHttpResponse.h"

class XHttpClient
{
	//用于与客户端进行网络通信的类,不要望文生义,产生误解
	//启动客户端线程,接收用户数据,调用算法/php,把生成的结果返回给用户
public:
	XHttpClient();
	~XHttpClient();
	XTcp client;
	XHttpResponse res;
	bool Start(XTcp client);//创建通信线程(也可以在server中做)
	void Main();//与客户通信的入口函数
};

