#define _CRT_SECURE_NO_WARNINGS
#include "XHttpResponse.h"
#include<string>
#include<regex>

using namespace std;

bool XHttpResponse::SetRequest(string request) {
    string src = request;//源
    string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.][a-zA-Z0-9]*)?)[?]?(.*) HTTP/1"; //模板
    regex r(pattern);//正则表达式
    smatch mas;//结果集
    regex_search(src, mas, r);//匹配
    if (mas.size() == 0) {
        //匹配失败
        printf("%s failed!\n", pattern.c_str());
        return false;
    }
    string type = mas[1];
    string path = "/";
    path += mas[2];
    string filetype = mas[3];
    string query = mas[4];

    if (type != "GET") {
        //本程序只处理GET,其他直接退出
        printf("Not GET!!!\n");
        return false;
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

    fp = fopen(filepath.c_str(), "rb");
    if (fp == NULL) {
        //如果返回404错误,还需要返回404页面
        //此处简化处理,直接退出
        printf("open file %s failed!!!\n", filepath.c_str());
        return false;
    }

    //获取文件大小,修改报文
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);//有缺陷的写法,受int表示范围限制
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

	return true;
}

//获取头信息的接口
string XHttpResponse::GetHead() {
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
	return rmsg;
}

//读内容
int XHttpResponse::Read(char* buf, int bufsize) {
	return fread(buf, 1,bufsize, fp);
}

XHttpResponse::XHttpResponse() {

}

XHttpResponse::~XHttpResponse() {

}