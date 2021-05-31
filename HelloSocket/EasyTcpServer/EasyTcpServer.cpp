#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include<Windows.h>
#include<winsock2.h>
#include<stdio.h>
#pragma comment (lib,"ws2_32.lib")

struct DataHead {
	short dataLength;//数据长度
	short cmd;//命令
};
enum CMD {//枚举
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_LOGIN_RES,
	CMD_LOGOUT_RES,
	CMD_ERROR
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
	//char _recvBuf[128] = {};

	while (true)
	{	//5、接收客户端的请求数据
		DataHead head = {};
		int nlen=recv(_cSock, (char*)&head, sizeof(DataHead), 0);
		if (nlen <= 0) {
			printf("客户端已退出,任务结束！\n");
			break;
		}
		//6、处理请求		
		switch (head.cmd)
		{
		case CMD_LOGIN: {
			Login login = {};
			recv(_cSock, (char*)&login+sizeof(DataHead), sizeof(Login)- sizeof(DataHead), 0);
			printf("收到命令：CMD_LOGIN  数据长度：%d  uesrname：%s password：%s\n", login.dataLength,login.userName,login.passWord);

			//忽略判断用户名密码是否正确的过程

			LoginRes loginres;
			//返回数据 发送数据包
			send(_cSock, (char*)&loginres, sizeof(LoginRes), 0);

			break;
		}
		case CMD_LOGOUT: {
			Logout logout = {};
			recv(_cSock, (char*)&logout + sizeof(DataHead), sizeof(Logout)-sizeof(DataHead), 0);
			printf("收到命令：CMD_LOGOUT  数据长度：%d  uesrname：%s \n", logout.dataLength, logout.userName);

			//忽略判断用户名密码是否正确的过程

			LogoutRes logoutres ;
			//返回数据 发送数据包
			send(_cSock, (char*)&logoutres, sizeof(LogoutRes), 0);

			break; 
		}
		default: {
			head.cmd = CMD_ERROR;
			head.dataLength = 0;//??
			send(_cSock, (char*)&head, sizeof(DataHead), 0);
			break;
		}	
		}	
		
	}
	

	//8、关闭套接字 closesocket
	closesocket(_sock);
	//清除windows socket环境
	WSACleanup();
	printf("服务器退出！\n");
	getchar();//实现的功能，暂停
	return 0;
}