#pragma once
#include "XTcp.h"

class XHttpServer
{
public:
	XHttpServer();
	~XHttpServer();
	XTcp server;
	bool isexit = false;
	//可以有多个服务器绑定不同端口号
	bool Start(unsigned short port);
	void Main();
	bool Stop();
};

