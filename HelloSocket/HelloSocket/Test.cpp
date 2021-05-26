
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<winsock2.h>
#pragma comment (lib,"ws2_32.lib")

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver,&dat);//启动window socket 2.x环境

	//用socket API建立简易TCP客户端




	WSACleanup();//关闭socket网络环境
	return 0;
}