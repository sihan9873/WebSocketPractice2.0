#ifndef XTCP_H
#define XTCP_H

#ifdef _WIN32
	#ifdef XSOCKET_EXPORTS
		#define XSOCKET_API __declspec(dllexport)
	#else
		#define XSOCKET_API __declspec(dllimport)
	#endif
#else
	#define XSOCKET_API
#endif

#include <string>
#include <cstring>

class XSOCKET_API XTcp
{
public:
	int CreateSocket();
	bool Bind(unsigned short port);
	//不能在析构中关闭socket,否则赋值等过程中会经常重复关闭
	XTcp Accept();
	void Close();
	int Recv(char* buf, int bufsize);
	int Send(const char* buf, int sendsize);
	//在原有的基础上增加timeout参数,通过设置非阻塞+多路复用实现超时处理
	bool Connect(const char* ip, unsigned short port,int timeouttms=1000);
	//建立链接的时候西川王世非阻塞模式,收发数据时希望是阻塞模式
	bool SetBlock(bool isblock);
	XTcp();
	virtual ~XTcp();
	int sock = 0;
	unsigned short port = 0;
	char ip[16];
};

#endif
