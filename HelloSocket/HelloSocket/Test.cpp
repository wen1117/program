
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<winsock2.h>
#pragma comment (lib,"ws2_32.lib")

int main() {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver,&dat);//����window socket 2.x����

	//��socket API��������TCP�ͻ���




	WSACleanup();//�ر�socket���绷��
	return 0;
}