#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include<Windows.h>
#include<winsock2.h>
#include<stdio.h>
#pragma comment (lib,"ws2_32.lib")

#include<vector>

enum CMD {//枚举
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_LOGIN_RES,
	CMD_LOGOUT_RES,
	CMD_ERROR
};
struct DataHead {
	short dataLength;//数据长度
	short cmd;//命令
};

//DataPackage
struct Login :public DataHead {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
struct LoginRes :public DataHead {
	LoginRes() {
		dataLength = sizeof(LoginRes);
		cmd = CMD_LOGIN_RES;
		result = 0;
	}
	int result;
	
};
struct Logout :public DataHead {
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogoutRes :public DataHead {
	LogoutRes() {
		dataLength = sizeof(LogoutRes);
		cmd = CMD_LOGOUT_RES;
		result = 0;
	}
	int result;

};
//加入的客户端，用一个全局动态数组存储
std::vector<SOCKET> g_clients;

//处理请求，封装
int processor(SOCKET _cSock) {
	//缓冲区
	char szRecv[1024] = {};
	//5、接收客户端的请求数据
	int nlen = recv(_cSock, szRecv, sizeof(DataHead), 0);
	DataHead* head = (DataHead*)szRecv;
	if (nlen <= 0) {
		printf("客户端<socket=%d>已退出,任务结束！\n", _cSock);
		return -1;
	}
	//6、处理请求		
	switch (head->cmd)
	{
	case CMD_LOGIN: {
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		Login* login = (Login*)szRecv;
		printf("收到客户端<socket=%d>请求：CMD_LOGIN  数据长度：%d  uesrname：%s password：%s\n", _cSock, login->dataLength, login->userName, login->passWord);

		//忽略判断用户名密码是否正确的过程

		LoginRes loginres;
		//返回数据 发送数据包
		send(_cSock, (char*)&loginres, sizeof(LoginRes), 0);

		break;
	}
	case CMD_LOGOUT: {
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		Logout* logout = (Logout*)szRecv;
		printf("收到客户端<socket=%d>请求：CMD_LOGOUT  数据长度：%d  uesrname：%s \n", _cSock,logout->dataLength, logout->userName);

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
	
	while (true)
	{	//伯克利 socket 
		fd_set fdRead;//可读的套接字集合
		fd_set fdWrite;
		fd_set fdExp;//有误的套接字

		FD_ZERO(&fdRead);//将该集合清空
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);//将服务端放入集合
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {//倒序快一点，只用调用一次size
				FD_SET(g_clients[n], &fdRead);//将客户端放入可读集合，之后进行查询是否有操作
			}

		//nfds 第一个参数是一个整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量，即是所有描述符最大值+1
		//在windows下第一个参数无意义,可以写0
		//最后一个参数用于描述一段时间长度，如果在这个时间内，需要监视的描述符没有事件发生则函数返回，返回值为0，null为阻塞模式(select将一直被阻塞，直到某个文件描述符上发生了事件)，
		
		timeval t = { 1,0 };//设置等待时间为0，如果没有命令，立即返回
		//int ret=select(_sock + 1, &fdRead, &fdWrite, &fdExp, NULL);
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);

		//如果出错，直接退出循环
		if (ret < 0) {
			printf("select任务结束！\n");
			break;
		}
		
		//判断一下有没有可读的socket
		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);//用于在文件描述符集合中删除一个文件描述符,只清除count


			//4、等待接受客户端连接 accept
			//客户端地址
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(clientAddr);
			//客户端
			SOCKET _cSock = INVALID_SOCKET;


			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSock)
			{
				printf("错误，接收到无效客户端socket!\n");
			}
			printf("新客户端加入：socket=%d,IP= %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
			//将新加入的客户端存起来 利用动态数组
			g_clients.push_back(_cSock);
		}
		//循环处理客户端请求
		for (size_t n = 0; n <fdRead.fd_count; n++) {
			if (-1 == processor(fdRead.fd_array[n])) {
				//如果出错，从动态数组中移除该客户端
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end()) {
					g_clients.erase(iter);
				}
			}
		}

		//如果客户端没有请求，就会执行到这里，服务器可以进行一些其他的任务
		printf("空闲时间处理其他业务……\n");
	}
	
	//如果select任务结束，直接break了
	//要将服务端socket全部关闭
	for (int n = (int)g_clients.size()-1; n >= 0; n--) {
		closesocket(g_clients[n]);
	}

	//8、关闭套接字 closesocket 服务端
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("服务器退出！\n");
	getchar();//实现的功能，暂停
	return 0;
}