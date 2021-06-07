#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

struct DataHead {
	short dataLength;//数据长度
	short cmd;//命令
};
enum CMD {//枚举
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_LOGIN_RES,
	CMD_LOGOUT_RES,
	CMD_ERROR,
	CMD_NEW_USER_JION
};

//DataPackage
struct Login :public DataHead {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];

};
struct LoginRes :public DataHead {
	LoginRes() {
		dataLength = sizeof(LoginRes);
		cmd = CMD_LOGIN_RES;
		result = 0;
	}
	int result;
	char data[1024];
};
struct Logout :public DataHead {
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogoutRes :public DataHead {
	LogoutRes() {
		dataLength = sizeof(LogoutRes);
		cmd = CMD_LOGOUT_RES;
		result = 0;
	}
	int result;

};
struct NewUserJoin :public DataHead {
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JION;
		csocket = 0;
	}
	int csocket;
};
#endif // !_Message_Header_
