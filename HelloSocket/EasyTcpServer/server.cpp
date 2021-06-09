#include"EasyTcpServer.hpp"
#include<thread>
//用于输入命令
bool g_bRun = true;
void cmdThread() {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令！\n");
		}
	}
}
int main() {
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{	
		server.OnRun();
		//如果客户端没有请求，就会执行到这里，服务器可以进行一些其他的任务
		//printf("空闲时间处理其他业务……\n");
	}
	server.Close();
	printf("服务器退出！\n");
	getchar();//实现的功能，暂停
	return 0;
}