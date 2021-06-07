#ifndef _EasyTcpClient_hpp_ //�൱��#pragma once
#define _EasyTcpClient_hpp_

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include<unistd.h>//uni std
	#include<arpa/inet.h>
	#include<string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif // _WIN32
#include<stdio.h>
#include"MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}
	//����������
	virtual ~EasyTcpClient() {
		Close();
	}
	//��ʼ��socket
	void InitSocket() {
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);//����window socket 2.x����
#endif
		if (INVALID_SOCKET != _sock) {
			printf("<socket=%d>�رվ����ӣ�\n", (int)_sock);
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
	//���ӷ�����
	int Connect(const char* ip, unsigned short port) {
		//�������Ч��socket����ʼ��һ��
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		//2�����ӷ����� connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);//���ӵķ�������ַ
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32	
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ�ܣ�\n", (int)_sock,ip,port);
		}
		else {
			printf("<socket=%d>���ӷ�����<%s:%d>�ɹ���\n", (int)_sock, ip, port);
		}
		return ret;
	}
	//�ر��׽��� closesocket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			closesocket(_sock);
			//��� Windows socket���绷��
			WSACleanup();
#else
			close(_sock);
#endif // _WIN32
			_sock = INVALID_SOCKET;
		}

	}
	//��ѯ������Ϣ
	bool OnRun() {
		if (IsRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			//��ָ����Դ���,���������0
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0) {
				printf("<socket=%d>select�������1��\n", (int)_sock);
				Close();
				return false;
			}
			//���_sock�Ƿ��ڼ���fdReads��
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket=%d>select�������2(�����)��\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//�ж϶˿��Ƿ��ڹ�����
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}

	//��������С��Ԫ��С
#define recv_buff_size 10240
	//���ջ�����
	char _szrecv[recv_buff_size] = {};
	//�ڶ������� ��Ϣ������
	char _szmsgbuf[recv_buff_size * 10] = {};
	//��Ϣ����������β��λ��
	int _lastpos = 0;
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET csock) {
		//5�����շ���˵�����
		int nlen = recv(csock, _szrecv, recv_buff_size, 0);
		if (nlen <= 0) {
			printf("<socket=%d>��������Ͽ����ӣ�\n", (int)csock);
			return -1;
		}
		//�����ջ����������ݿ�������Ϣ������
		memcpy(_szmsgbuf+_lastpos, _szrecv, nlen);
		//��Ϣ����������β����־λ�ú���
		_lastpos += nlen;

		//�ж���Ϣ�����������ݳ����Ƿ������Ϣͷdatahead�ĳ���
		//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
		while (_lastpos >= sizeof(DataHead)) {
			DataHead* head = (DataHead*)_szmsgbuf;
			//�ж���Ϣ�������ĳ����Ƿ������Ϣ����
			if (_lastpos >= head->dataLength) {
				//ʣ��δ������Ϣ���������ݵĳ���
				int nsize = _lastpos - head->dataLength;
				//����������Ϣ
				OnNetMsg(head);
				//����������δ���������ǰ�ƣ����Ǵ����������
				memcpy(_szmsgbuf, _szmsgbuf + head->dataLength, nsize);
				//��Ϣ����������β����־λ��ǰ��
				_lastpos = nsize;
			}
			else {
				//��Ϣ������ʣ����Ϣ����һ��������Ϣ
				break;
			}
		}
		return 0;
	}
	
	//��Ӧ������Ϣ ��������
	virtual void OnNetMsg(DataHead* head) {
		switch (head->cmd)
		{
		case CMD_LOGIN_RES: {
			LoginRes* loginres = (LoginRes*)head;
			//printf("<socket=%d>�յ��������Ϣ��CMD_LOGIN_RES ���ݳ��ȣ�%d\n", (int)_sock, head->dataLength);
			break;
		}
		case CMD_LOGOUT_RES: {
			LogoutRes* logoutres = (LogoutRes*)head;
			//printf("<socket=%d>�յ��������Ϣ��CMD_LOGOUT_RES ���ݳ��ȣ�%d\n", (int)_sock,head->dataLength);
			break;
		}
		case CMD_NEW_USER_JION: {
			NewUserJoin* nuj = (NewUserJoin*)head;
			//printf("<socket=%d>�յ��������Ϣ��CMD_NEW_USER_JION ���ݳ��ȣ�%d\n", (int)_sock,head->dataLength);
			break;
		}
		case CMD_ERROR: {
			printf("<socket=%d>�յ�������Ϣ ���ݳ��ȣ�%d\n", (int)_sock, head->dataLength);
			break;
		}
		default: {
			printf("<socket=%d>�յ�δ������Ϣ ���ݳ��ȣ�%d\n", (int)_sock, head->dataLength);
			break;
		}
		}
	}
	//��������
	int SendData(DataHead* head) {
		if (IsRun() && head) {
			return send(_sock, (const char*)head, head->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:

	};

#endif