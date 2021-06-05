#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<vector>
#include"MessageHeader.hpp"
class EasyTcpServer {
private:
	SOCKET _sock;
	//加入的客户端，用一个全局动态数组存储
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{	
		Close();
	}
	//初始化socket
	void InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);//启动window socket 2.x环境
#endif // _WIN_32
	//用socket API建立简易TCP服务端
	// 1、建立一个socket套接字
	//第一个参数 ipv4 第二个 面向流 第三个tcp协议
		 if (INVALID_SOCKET != _sock) {
			 printf("<socket=%d>关闭旧连接！\n",(int) _sock);
			 Close();
		 }
		 _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//最有一个参数 也可以写0，自动选择默认协议
		 if (INVALID_SOCKET == _sock) {
			 printf("错误，建立socket失败！\n");
		 }
		 else {
			 printf("建立<socket=%d>成功！\n", (int)_sock);
		 }
	}
	//绑定ip和端口号
	int Bind(const char* ip,unsigned short port) {
		////如果是无效的socket，初始化一下
		//if (INVALID_SOCKET == _sock) {
		//	InitSocket();
		//}
		//2、bind 绑定用于接收客户端的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		// host主机 to net unsigned short
		_sin.sin_port = htons(port);
		//绑定主机的IP地址
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else _sin.sin_addr.S_un.S_addr = INADDR_ANY;//任意ip地址
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
	}
		else _sin.sin_addr.s_addr = INADDR_ANY;//任意ip地址
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("ERROR!绑定用于接收客户端的网络端口<%d>失败!\n",port);
		}
		else {
			printf("绑定网络端口<%d>成功！\n", port);
		}
		return ret;
	}
	//监听端口 n为可以支持的连接数
	int Listen(int n) {
		// 3、listen 监听网络端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("<socket=%d>监听网络失败！\n",(int)_sock);
		}
		else {
			printf("<socket=%d>监听网络成功！\n",(int)_sock);
		}
		return ret;
	}
	//接受客户端连接
	SOCKET Accept() {
		//4、等待接受客户端连接 accept
			//客户端地址
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(clientAddr);
		//客户端
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>错误，接收到无效客户端!\n",(int)_sock);
		}
		else {
			printf("<socket=%d>新客户端加入：socket=%d,IP= %s \n", (int)_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
			//有新客户端加入时，向其他客户端发送消息通知
			NewUserJoin nuj;
			SendDataToALL(&nuj);
			//将新加入的客户端存起来 利用动态数组
			g_clients.push_back(_cSock);
		}
		return  _cSock;
	}
	// 处理网络消息
	bool OnRun() {
		if (IsRun()) {
			//伯克利 socket 
			fd_set fdRead;//可读的套接字集合
			fd_set fdWrite;
			fd_set fdExp;//有误的套接字

			FD_ZERO(&fdRead);//将该集合清空
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);//将描述符放入集合
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;

			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {//倒序快一点，只用调用一次size
				FD_SET(g_clients[n], &fdRead);//将客户端放入可读集合，之后进行查询是否有操作
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			//nfds 第一个参数是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量，即是所有描述符最大值+1
			//在windows下第一个参数无意义,可以写0
			//最后一个参数用于描述一段时间长度，如果在这个时间内，需要监视的描述符没有事件发生则函数返回，返回值为0，null为阻塞模式(select将一直被阻塞，直到某个文件描述符上发生了事件)，

			timeval t = { 1,0 };//设置等待时间为0，如果没有命令，立即返回
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

			//如果出错，直接退出循环
			if (ret < 0) {
				printf("select任务结束！\n");
				Close();
				return false;
			}
			//判断一下有没有可读的socket，判断描述符是否在集合中
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);//用于在文件描述符集合中删除一个文件描述符,只清除count
				Accept();
			}
			//循环处理客户端请求
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(g_clients[n], &fdRead)) {
					if (-1 == RecvData(g_clients[n])) {
						//如果出错，从动态数组中移除该客户端
						auto iter = g_clients.begin() + n;
						if (iter != g_clients.end()) {
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	// 是否工作中
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	// 接收数据 处理粘包 拆分包(处理请求)
	int RecvData(SOCKET _cSock) {
		//缓冲区
		char szRecv[4096] = {};
		//5、接收客户端的请求数据
		int nlen = recv(_cSock, szRecv, sizeof(DataHead), 0);
		DataHead* head = (DataHead*)szRecv;
		if (nlen <= 0) {
			printf("客户端<socket=%d>已退出,任务结束！\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		OnNetMsg(_cSock, head);
		return 0;
	}
	// 响应网络消息
	virtual void OnNetMsg(SOCKET _cSock , DataHead* head) {
		//6、处理请求		
		switch (head->cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)head;
			printf("收到客户端<socket=%d>请求：CMD_LOGIN  数据长度：%d  uesrname：%s password：%s\n",(int) _cSock, login->dataLength, login->userName, login->passWord);
			//忽略判断用户名密码是否正确的过程
			LoginRes loginres;
			//返回数据 发送数据包
			send(_cSock, (char*)&loginres, sizeof(LoginRes), 0);
			break;
		}
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)head;
			printf("收到客户端<socket=%d>请求：CMD_LOGOUT  数据长度：%d  uesrname：%s \n",(int) _cSock, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确的过程
			LogoutRes logoutres;
			//返回数据 发送数据包
			send(_cSock, (char*)&logoutres, sizeof(LogoutRes), 0);
			break;
		}
		default: {
			DataHead header;
			header.cmd = CMD_ERROR;
			header.dataLength = 0;
			send(_cSock, (char*)&head, sizeof(DataHead), 0);
			break;
		}
		}
	}
	// 发送给指定客户端socket数据
	int SendData(SOCKET _csock,DataHead* head) {
		if (IsRun() && head) {
			return send(_csock, (const char*)head, head->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	// 发送给所有客户端socket数据，群发
	void SendDataToALL( DataHead* head) {
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			SendData(g_clients[n], head);
		}
	}
	//关闭socket
	void Close() {
	//如果select任务结束，直接break了
	//要将服务端socket全部关闭
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				closesocket(g_clients[n]);
			}
			//8、关闭套接字 closesocket 服务端
			closesocket(_sock);
			//清除windows socket环境
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			// 8 关闭套节字closesocket
			close(_sock);
#endif
		}
	}
	//
};
#endif