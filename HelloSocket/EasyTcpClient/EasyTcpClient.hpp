#ifndef _EasyTcpClient_hpp_ //相当于#pragma once
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
	//虚析构函数
	virtual ~EasyTcpClient() {
		Close();
	}
	//初始化socket
	void InitSocket() {
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);//启动window socket 2.x环境
#endif
		if (INVALID_SOCKET != _sock) {
			printf("<socket=%d>关闭旧连接！\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//最有一个参数 也可以写0，自动选择默认协议
		if (INVALID_SOCKET == _sock) {
			printf("错误，建立socket失败！\n");
		}
		else {
			printf("建立<socket=%d>成功！\n", (int)_sock);
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port) {
		//如果是无效的socket，初始化一下
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		//2、连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);//连接的服务器地址
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32	
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("<socket=%d>错误，连接服务器<%s:%d>失败！\n", (int)_sock,ip,port);
		}
		else {
			printf("<socket=%d>连接服务器<%s:%d>成功！\n", (int)_sock, ip, port);
		}
		return ret;
	}
	//关闭套接字 closesocket
	void Close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			closesocket(_sock);
			//清除 Windows socket网络环境
			WSACleanup();
#else
			close(_sock);
#endif // _WIN32
			_sock = INVALID_SOCKET;
		}

	}
	//查询网络消息
	bool OnRun() {
		if (IsRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			//传指针可以传空,就是这里的0
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0) {
				printf("<socket=%d>select任务结束1！\n", (int)_sock);
				Close();
				return false;
			}
			//检查_sock是否在集合fdReads里
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock)) {
					printf("<socket=%d>select任务结束2(处理后)！\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//判断端口是否在工作中
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}

	//缓冲区最小单元大小
#define recv_buff_size 10240
	//接收缓冲区
	char _szrecv[recv_buff_size] = {};
	//第二缓冲区 消息缓冲区
	char _szmsgbuf[recv_buff_size * 10] = {};
	//消息缓冲区数据尾部位置
	int _lastpos = 0;
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET csock) {
		//5、接收服务端的数据
		int nlen = recv(csock, _szrecv, recv_buff_size, 0);
		if (nlen <= 0) {
			printf("<socket=%d>与服务器断开连接！\n", (int)csock);
			return -1;
		}
		//将接收缓冲区的数据拷贝到消息缓冲区
		memcpy(_szmsgbuf+_lastpos, _szrecv, nlen);
		//消息缓冲区数据尾部标志位置后移
		_lastpos += nlen;

		//判断消息缓冲区的数据长度是否大于消息头datahead的长度
		//这时就可以知道当前消息体的长度
		while (_lastpos >= sizeof(DataHead)) {
			DataHead* head = (DataHead*)_szmsgbuf;
			//判断消息缓冲区的长度是否大于消息长度
			if (_lastpos >= head->dataLength) {
				//剩余未处理消息缓冲区数据的长度
				int nsize = _lastpos - head->dataLength;
				//处理网络消息
				OnNetMsg(head);
				//将缓冲区的未处理的数据前移，覆盖处理过的数据
				memcpy(_szmsgbuf, _szmsgbuf + head->dataLength, nsize);
				//消息缓冲区数据尾部标志位置前移
				_lastpos = nsize;
			}
			else {
				//消息缓冲区剩余消息不够一条完整消息
				break;
			}
		}
		return 0;
	}
	
	//响应网络消息 处理请求
	virtual void OnNetMsg(DataHead* head) {
		switch (head->cmd)
		{
		case CMD_LOGIN_RES: {
			LoginRes* loginres = (LoginRes*)head;
			//printf("<socket=%d>收到服务端消息：CMD_LOGIN_RES 数据长度：%d\n", (int)_sock, head->dataLength);
			break;
		}
		case CMD_LOGOUT_RES: {
			LogoutRes* logoutres = (LogoutRes*)head;
			//printf("<socket=%d>收到服务端消息：CMD_LOGOUT_RES 数据长度：%d\n", (int)_sock,head->dataLength);
			break;
		}
		case CMD_NEW_USER_JION: {
			NewUserJoin* nuj = (NewUserJoin*)head;
			//printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JION 数据长度：%d\n", (int)_sock,head->dataLength);
			break;
		}
		case CMD_ERROR: {
			printf("<socket=%d>收到错误消息 数据长度：%d\n", (int)_sock, head->dataLength);
			break;
		}
		default: {
			printf("<socket=%d>收到未定义消息 数据长度：%d\n", (int)_sock, head->dataLength);
			break;
		}
		}
	}
	//发送数据
	int SendData(DataHead* head) {
		if (IsRun() && head) {
			return send(_sock, (const char*)head, head->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:

	};

#endif