#include "XTcp.h"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <WinSock2.h> 
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define socklen_t int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
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

//在linux中输入./client运行

int main() {
	XTcp  client;
	client.SetBlock(false);
	client.Connect("192.168.199.163",8080,3000);
	client.Send("cleint",6);
	char buff[1024] = { 0 };
	client.Recv(buff, sizeof(buff));
	printf("%s\n", buff);

	return 0;
}