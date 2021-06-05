#include"EasyTcpServer.hpp"
int main() {
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4568);
	server.Listen(5);
	while (server.IsRun())
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