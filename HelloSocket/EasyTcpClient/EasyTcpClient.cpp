#define WIN32_LEAN_AND_MEAN

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
	CMD_ERROR,
	CMD_NEW_USER_JION
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
struct NewUserJoin :public DataHead{
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JION;
		csocket = 0;
	}
	int csocket;
};

int processor(SOCKET _cSock) {
	//缓冲区
	char szRecv[4096] = {};
	//5、接收服务端的数据
	int nlen = recv(_cSock, szRecv, sizeof(DataHead), 0);
	DataHead* head = (DataHead*)szRecv;
	if (nlen <= 0) {
		printf("与服务器断开连接！\n");
		return -1;
	}
	//6、处理请求		
	switch (head->cmd)
	{
	case CMD_LOGIN_RES: {
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		LoginRes* loginres = (LoginRes*)szRecv;
		printf("收到服务端消息：CMD_LOGIN_RES 数据长度：%d\n", head->dataLength);
		break;
	}
	case CMD_LOGOUT_RES: {
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		LogoutRes* logoutres = (LogoutRes*)szRecv;
		printf("收到服务端消息：CMD_LOGOUT_RES 数据长度：%d\n", head->dataLength);
		break;
	}
	case CMD_NEW_USER_JION: {
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		NewUserJoin* nuj = (NewUserJoin*)szRecv;
		printf("收到服务端消息：CMD_NEW_USER_JION 数据长度：%d\n", head->dataLength);
		break;
	}
	
	}
}
int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);//启动window socket 2.x环境

	//用socket API建立简易TCP客户端
	//1、建立一个socket
	SOCKET _sock=socket(AF_INET, SOCK_STREAM, 0);//0自动选择默认协议
	if (INVALID_SOCKET == _sock) {
		printf("错误，建立socket失败！\n");
	}
	else {
		printf("建立socket成功！\n");
	}

	//2、连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret=connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf("连接服务器失败！\n");
	}
	else {
		printf("连接服务器成功！\n");
	}

	while (true) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		//传指针可以传空,就是这里的0
		int ret=select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select任务结束1！\n");
			break;
		}
		//检查_sock是否在集合fdReads里
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock)) {
				printf("select任务结束2(处理后)！\n");
				break;
			}
		}
		printf("空闲时间处理其他业务..\n");
		Login login;
		strcpy(login.userName, "ren wen");
		strcpy(login.passWord, "ren wen password");
		send(_sock, (const char *)&login, sizeof(Login), 0);
		Sleep(1000);
	}
	
	//7、关闭套接字 closesocket
	getchar();
	closesocket(_sock);
	
	//清除 Windows socket网络环境
	WSACleanup();
	printf("客户端退出，任务结束！\n");
	//getchar();
	return 0;
	
}