#ifndef XTCP_H
#define XTCP_H

#ifdef WIN32
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
	bool Connect(const char* ip,unsigned short port);
	XTcp();
	virtual ~XTcp();
	int sock = 0;
	unsigned short port = 0;
	char ip[16];
};

#endif
