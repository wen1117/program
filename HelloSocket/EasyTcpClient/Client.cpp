#include"EasyTcpClient.hpp"
#include<thread>//标准线程库

bool g_bRun = true;
//用于输入命令
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
	 
	const int cCount = 1000;//windows默认最大连接数 64个set，这里1个服务器，63个客户端
	EasyTcpClient* client[cCount];
	for (int i = 0; i< cCount; i++) {
		if (!g_bRun) return 0;
		client[i] = new EasyTcpClient();
	}
	for (int i = 0; i < cCount; i++) {
		if (!g_bRun) return 0;
		client[i]->Connect("127.0.0.1", 4567);
		printf("connect=%d\n", i);
	}
	

	//启动线程函数
	std::thread t1(cmdThread);
	t1.detach();//用来分离cmdThread线程与主线程
	
	Login login;
	strcpy(login.userName, "rww");
	strcpy(login.passWord, "rwwmm");
	

	while (g_bRun) {
		for (int i = 0; i < cCount; i++) {
			client[i]->SendData(&login);
			//client[i]->OnRun();
		}
		
		
		//printf("空闲时间处理其他业务..\n");
	}
	for (int i = 0; i < cCount; i++) {
		client[i]->Close();
	}
	printf("客户端退出，任务结束！\n");

	return 0;
}