#pragma once
#include <string>

using namespace std;

class XHttpResponse
{
private:
	int filesize = 0;
	FILE* fp = NULL;

public:
	//设置请求
	bool SetRequest(string request);
	//获取头信息的接口
	string GetHead();
	//读内容
	int Read(char* buf, int bufsize);
	XHttpResponse();
	~XHttpResponse();
};

