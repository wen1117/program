#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include<Windows.h>
#include<winsock2.h>
#include<stdio.h>
#pragma comment (lib,"ws2_32.lib")



int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);//启动window socket 2.x环境

	//用socket API建立简易TCP服务端
	
	// 1、建立一个socket套接字
	//第一个参数 ipv4 第二个 面向流 第三个tcp协议
	SOCKET _sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//2、bind 绑定用于接收客户端的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	// host主机 to net unsigned short
	_sin.sin_port =htons(4567);
	//绑定主机的IP地址
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//任意ip地址
		//inet_addr("127.0.0.1");内网
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("ERROR!绑定用于接收客户端的网络端口失败!\n");
	}
	else {
		printf("绑定端口成功！\n");
	}
	
	// 3、listen 监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("监听网络失败！\n");
	}
	else {
		printf("监听网络成功！\n");
	}

	//4、等待接受客户端连接 accept
	//客户端地址
	sockaddr_in clientAddr = {};
	int nAddrLen = sizeof(clientAddr);
	//客户端
	SOCKET _cSock = INVALID_SOCKET;
	//发送的内容
	//char msgBuf[] = "";

	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("错误，接收到无效客户端socket!\n");
	}
	printf("新客户端加入：socket=%d,IP= %s \n",(int)_cSock, inet_ntoa(clientAddr.sin_addr));
	
	//缓冲区
	char _recvBuf[128] = {};
	while (true)
	{	//5、接收客户端的请求数据
		int nlen=recv(_cSock, _recvBuf, sizeof(_recvBuf), 0);
		if (nlen <= 0) {
			printf("客户端已退出,任务结束！\n");
			break;
		}
		//6、处理请求		
		printf("收到命令：%s\n", _recvBuf);
		if (0 == strcmp(_recvBuf, "getName"))
		{
			char msgBuf[] = "Name: Wen Ren";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		
		else if (0 == strcmp(_recvBuf, "getAge")) {
			char msgBuf[] = "Age: 18";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else {
			char msgBuf[] = "?????";
			send(_cSock, msgBuf, strlen(msgBuf) + 1, 0);
		}
		
		
	}
	

	//8、关闭套接字 closesocket
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("服务器退出！\n");
	return 0;
}