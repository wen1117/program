#include"EasyTcpClient.hpp"
#include<thread>//标准线程库


//用于输入命令
void cmdThread(EasyTcpClient *client) {
	while (true) {
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit")) {
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "ren wen");
			strcpy(login.passWord, "ren wen password");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "ren wen");
			client->SendData(&logout);
		}
		else {
			printf("不支持的命令！\n");
		}
	}
}
int main() {
	EasyTcpClient client;
	//client.InitSocket();
	client.Connect("127.0.0.1", 4567);

	//启动线程函数
	//std::thread t1(cmdThread, &client);
	//t1.detach();//用来分离cmdThread线程与主线程
	
	Login login;
	strcpy(login.userName, "rww");
	strcpy(login.passWord, "rwwmm");
	
	while (client.IsRun()) {
		client.OnRun();
		client.SendData(&login);
		//printf("空闲时间处理其他业务..\n");
	}
	client.Close();	
	printf("客户端退出，任务结束！\n");
	getchar();
	return 0;
}