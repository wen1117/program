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

//��������С��Ԫ��С
#ifndef recv_buff_size
#define recv_buff_size 10240
#endif // !recv_buff_size

class ClientSocket {
public:
	ClientSocket(SOCKET sockfd=INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastpos = 0;
	}
	~ClientSocket()
	{

	}
	SOCKET sockfd() {
		return _sockfd;
	}
	char* msgBuf() {
		return _szMsgBuf;
	}
	int getLastPos() {
		return _lastpos;
	}
	void setLastPos(int pos) {
		_lastpos = pos;
	}
private:
	SOCKET _sockfd;//socket fd_set file desc set//���ջ�����
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[recv_buff_size * 10] ;
	//��Ϣ����������β��λ��
	int _lastpos ;
};

class EasyTcpServer {
private:
	SOCKET _sock;
	//��Ҫֱ��vector<ClientSocket> ����Ϊ����new�������������������ջ����ջ���ռ��С���������ױ�ջ
	std::vector<ClientSocket*> _clients;
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
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("<socket=%d>���󣬽��յ���Ч�ͻ���!\n",(int)_sock);
		}
		else {
			printf("<socket=%d>�¿ͻ��˼��룺socket=%d,IP= %s \n", (int)_sock,(int)cSock, inet_ntoa(clientAddr.sin_addr));
			//���¿ͻ��˼���ʱ���������ͻ��˷�����Ϣ֪ͨ
			NewUserJoin nuj;
			SendDataToALL(&nuj);
			//���¼���Ŀͻ��˴����� ���ö�̬����
			_clients.push_back(new ClientSocket(cSock));
		}
		return  cSock;
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

			for (int n = (int)_clients.size() - 1; n >= 0; n--) {//�����һ�㣬ֻ�õ���һ��size
				FD_SET(_clients[n]->sockfd(), &fdRead);//���ͻ��˷���ɶ����ϣ�֮����в�ѯ�Ƿ��в���
				if (maxSock < _clients[n]->sockfd())
				{
					maxSock = _clients[n]->sockfd();
				}
			}
			//nfds ��һ��������һ������ֵ����ָfd_set������������������socket���ķ�Χ�������������������������������ֵ+1
			//��windows�µ�һ������������,����д0
			//���һ��������������һ��ʱ�䳤�ȣ���������ʱ���ڣ���Ҫ���ӵ�������û���¼������������أ�����ֵΪ0��nullΪ����ģʽ(select��һֱ��������ֱ��ĳ���ļ��������Ϸ������¼�)��

			timeval t = { 1,0 };//���õȴ�ʱ��Ϊ0�����û�������������
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);//&t

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
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead)) {
					if (-1 == RecvData(_clients[n])) {
						//��������Ӷ�̬�������Ƴ��ÿͻ���
						auto iter = _clients.begin() + n;
						if (iter != _clients.end()) {
							delete _clients[n];
							_clients.erase(iter);
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

	////�ڶ�������
	char szRecv[recv_buff_size] = {};
	// �������� ����ճ�� ��ְ�(��������)
	int RecvData(ClientSocket* pClient) {
		//5�����տͻ��˵���������
		int nlen = (int)recv(pClient->sockfd(), szRecv, recv_buff_size, 0);
		if (nlen < 0) {
			printf("�ͻ���<socket=%d>���˳����������\n",(int) pClient->sockfd());
			return -1;
		}
		//�����ܵ������ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(),szRecv, nlen);

		//��Ϣ����������β����־λ�ú���
		pClient->setLastPos(pClient->getLastPos()+ nlen);

		//�ж���Ϣ�����������ݳ����Ƿ������Ϣͷdatahead�ĳ���
		//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
		while (pClient->getLastPos() >= sizeof(DataHead)) {
			DataHead* head = (DataHead*)pClient->msgBuf();
			//�ж���Ϣ�������ĳ����Ƿ������Ϣ����
			if (pClient->getLastPos() >= head->dataLength) {
				//ʣ��δ������Ϣ���������ݵĳ���
				int nsize = pClient->getLastPos() - head->dataLength;
				//����������Ϣ
				OnNetMsg(pClient->sockfd(), head);
				//����������δ���������ǰ�ƣ����Ǵ����������
				memcpy(pClient->msgBuf(), pClient->msgBuf() + head->dataLength, nsize);
				//��Ϣ����������β����־λ��ǰ��
				pClient->setLastPos(nsize);
			}
			else {
				//��Ϣ������ʣ����Ϣ����һ��������Ϣ
				break;
			}
		}
		return 0;
	}
	// ��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock , DataHead* head) {
		//6����������		
		switch (head->cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)head;
			//printf("�յ��ͻ���<socket=%d>����CMD_LOGIN  ���ݳ��ȣ�%d  uesrname��%s password��%s\n",(int) cSock, login->dataLength, login->userName, login->passWord);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LoginRes ret;
			//�������� �������ݰ�
			SendData(cSock, &ret);
			break;
		}
		case CMD_LOGOUT: {
			Logout* logout = (Logout*)head;
			//printf("�յ��ͻ���<socket=%d>����CMD_LOGOUT  ���ݳ��ȣ�%d  uesrname��%s \n",(int) cSock, logout->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ�Ĺ���
			LogoutRes ret;
			//�������� �������ݰ�
			SendData(cSock, &ret);
			break;
		}
		default: {
			printf("<socket=%d>�յ�δ����������Ϣ  ���ݳ��ȣ�%d  uesrname��%s \n", (int)cSock, head->dataLength);
			//DataHead header ;
			//SendData(cSock, &header);
			break;
		}
		}
	}
	// ���͸�ָ���ͻ���socket����
	int SendData(SOCKET cSock,DataHead* head) {
		if (IsRun() && head) {
			return send(cSock, (const char*)head, head->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	// ���͸����пͻ���socket���ݣ�Ⱥ��
	void SendDataToALL( DataHead* head) {
		for (int n = (int)_clients.size() - 1; n >= 0; n--) {
			SendData(_clients[n]->sockfd(), head);
		}
	}
	//�ر�socket
	void Close() {
	//���select���������ֱ��break��
	//Ҫ�������socketȫ���ر�
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--) {
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			//8���ر��׽��� closesocket �����
			closesocket(_sock);
			//���windows socket����
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]->sockfd());
				delete _clients[n];
			}
			// 8 �ر��׽���closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}
	//
};
#endif