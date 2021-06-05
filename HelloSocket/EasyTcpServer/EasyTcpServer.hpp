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
	//����Ŀͻ��ˣ���һ��ȫ�ֶ�̬����洢
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{	
		Close();
	}
	//��ʼ��socket
	void InitSocket() {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);//����window socket 2.x����
#endif // _WIN_32
	//��socket API��������TCP�����
	// 1������һ��socket�׽���
	//��һ������ ipv4 �ڶ��� ������ ������tcpЭ��
		 if (INVALID_SOCKET != _sock) {
			 printf("<socket=%d>�رվ����ӣ�\n",(int) _sock);
			 Close();
		 }
		 _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//����һ������ Ҳ����д0���Զ�ѡ��Ĭ��Э��
		 if (INVALID_SOCKET == _sock) {
			 printf("���󣬽���socketʧ�ܣ�\n");
		 }
		 else {
			 printf("����<socket=%d>�ɹ���\n", (int)_sock);
		 }
	}
	//��ip�Ͷ˿ں�
	int Bind(const char* ip,unsigned short port) {
		////�������Ч��socket����ʼ��һ��
		//if (INVALID_SOCKET == _sock) {
		//	InitSocket();
		//}
		//2��bind �����ڽ��տͻ��˵�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		// host���� to net unsigned short
		_sin.sin_port = htons(port);
		//��������IP��ַ
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else _sin.sin_addr.S_un.S_addr = INADDR_ANY;//����ip��ַ
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
	}
		else _sin.sin_addr.s_addr = INADDR_ANY;//����ip��ַ
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {
			printf("ERROR!�����ڽ��տͻ��˵�����˿�<%d>ʧ��!\n",port);
		}
		else {
			printf("������˿�<%d>�ɹ���\n", port);
		}
		return ret;
	}
	//�����˿� nΪ����֧�ֵ�������
	int Listen(int n) {
		// 3��listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			printf("<socket=%d>��������ʧ�ܣ�\n",(int)_sock);
		}
		else {
			printf("<socket=%d>��������ɹ���\n",(int)_sock);
		}
		return ret;
	}
	//���ܿͻ�������
	SOCKET Accept() {
		//4���ȴ����ܿͻ������� accept
			//�ͻ��˵�ַ
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(clientAddr);
		//�ͻ���
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == _cSock)
		{
			printf("<socket=%d>���󣬽��յ���Ч�ͻ���!\n",(int)_sock);
		}
		else {
			printf("<socket=%d>�¿ͻ��˼��룺socket=%d,IP= %s \n", (int)_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
			//���¿ͻ��˼���ʱ���������ͻ��˷�����Ϣ֪ͨ
			NewUserJoin nuj;
			SendDataToALL(&nuj);
			//���¼���Ŀͻ��˴����� ���ö�̬����
			g_clients.push_back(_cSock);
		}
		return  _cSock;
	}
	// ����������Ϣ
	bool OnRun() {
		if (IsRun()) {
			//������ socket 
			fd_set fdRead;//�ɶ����׽��ּ���
			fd_set fdWrite;
			fd_set fdExp;//������׽���

			FD_ZERO(&fdRead);//���ü������
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);//�����������뼯��
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;

			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {//�����һ�㣬ֻ�õ���һ��size
				FD_SET(g_clients[n], &fdRead);//���ͻ��˷���ɶ����ϣ�֮����в�ѯ�Ƿ��в���
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}
			//nfds ��һ��������һ������ֵ����ָfd_set������������������socket���ķ�Χ�������������������������������ֵ+1
			//��windows�µ�һ������������,����д0
			//���һ��������������һ��ʱ�䳤�ȣ���������ʱ���ڣ���Ҫ���ӵ�������û���¼������������أ�����ֵΪ0��nullΪ����ģʽ(select��һֱ��������ֱ��ĳ���ļ��������Ϸ������¼�)��

			timeval t = { 1,0 };//���õȴ�ʱ��Ϊ0�����û�������������
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

			//�������ֱ���˳�ѭ��
			if (ret < 0) {
				printf("select���������\n");
				Close();
				return false;
			}
			//�ж�һ����û�пɶ���socket���ж��������Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);//�������ļ�������������ɾ��һ���ļ�������,ֻ���count
				Accept();
			}
			//ѭ������ͻ�������
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(g_clients[n], &fdRead)) {
					if (-1 == RecvData(g_clients[n])) {
						//��������Ӷ�̬�������Ƴ��ÿͻ���
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
	// �Ƿ�����
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	// �������� ����ճ�� ��ְ�(��������)
	int RecvData(SOCKET _cSock) {
		//������
		char szRecv[4096] = {};
		//5�����տͻ��˵���������
		int nlen = recv(_cSock, szRecv, sizeof(DataHead), 0);
		DataHead* head = (DataHead*)szRecv;
		if (nlen <= 0) {
			printf("�ͻ���<socket=%d>���˳�,���������\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHead), head->dataLength - sizeof(DataHead), 0);
		OnNetMsg(_cSock, head);
		return 0;
	}
	// ��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET _cSock , DataHead* head) {
		//6����������		
		switch (head->cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)head;
			printf("�յ��ͻ���<socket=%d>����CMD_LOGIN  ���ݳ��ȣ�%d  uesrname��%s password��%s\n",(int) _cSock, login->dataLength, login->userName, login->passWord);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginRes loginres;
			//�������� �������ݰ�
			send(_cSock, (char*)&loginres, sizeof(LoginRes), 0);
			break;
		}
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)head;
			printf("�յ��ͻ���<socket=%d>����CMD_LOGOUT  ���ݳ��ȣ�%d  uesrname��%s \n",(int) _cSock, logout->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LogoutRes logoutres;
			//�������� �������ݰ�
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
	// ���͸�ָ���ͻ���socket����
	int SendData(SOCKET _csock,DataHead* head) {
		if (IsRun() && head) {
			return send(_csock, (const char*)head, head->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	// ���͸����пͻ���socket���ݣ�Ⱥ��
	void SendDataToALL( DataHead* head) {
		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			SendData(g_clients[n], head);
		}
	}
	//�ر�socket
	void Close() {
	//���select���������ֱ��break��
	//Ҫ�������socketȫ���ر�
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				closesocket(g_clients[n]);
			}
			//8���ر��׽��� closesocket �����
			closesocket(_sock);
			//���windows socket����
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			// 8 �ر��׽���closesocket
			close(_sock);
#endif
		}
	}
	//
};
#endif