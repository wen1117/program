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
	CMD_ERROR
};

//DataPackage
struct Login {
	char userName[32];
	char passWord[32];
};
struct LoginRes {
	int result;

};
struct Logout {
	char userName[32];
};
struct LogoutRes {
	int result;

};

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


	char cmdBuf[128] = {};
	while (true) {
		//3、输入请求命令
		scanf("%s", cmdBuf);
		//4、处理请求命令
		if (0 == strcmp(cmdBuf, "exit")) {
			printf("收到退出命令，任务结束！\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			//5、向服务端发送请求命令
			DataHead shead = { sizeof(Login),CMD_LOGIN};
			Login login = { "ren wen","123456" };
			send(_sock, (const char*)&shead, sizeof(DataHead), 0);
			send(_sock, (const char*)&login, sizeof(Login), 0);
			//6、接收服务器发送的信息 recv
			DataHead rhead = {};
			LoginRes loginres = {};
			recv(_sock, (char*)&rhead, sizeof(DataHead), 0);
			recv(_sock, (char*)&loginres, sizeof(LoginRes), 0);
			printf("登录结果：%d\n", loginres.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			//5、向服务端发送请求命令
			DataHead shead = { sizeof(Logout),CMD_LOGOUT };
			Logout logout = { "ren wen" };
			send(_sock, (const char*)&shead, sizeof(DataHead), 0);
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
			//6、接收服务器发送的信息 recv
			DataHead rhead = {};
			LogoutRes logoutres = {};
			recv(_sock, (char*)&rhead, sizeof(DataHead), 0);
			recv(_sock, (char*)&logoutres, sizeof(LogoutRes), 0);
			printf("登出结果：%d\n", logoutres.result);
		}
		else {
			printf("不支持该命令,请重新输入！\n");
			
		}

	}
	
	
	//7、关闭套接字 closesocket
	closesocket(_sock);
	
	//清除 Windows socket网络环境
	WSACleanup();
	printf("客户端退出，任务结束！\n");
	getchar();
	return 0;
	
}