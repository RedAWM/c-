#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
//scoket头文件放在windows头文件之前
#include<Windows.h>
#include<thread>

//引入WIndows本地的静态库
#pragma comment(lib,"ws2_32.lib")
using namespace std;



enum CMD
{
	CMD_LOGIN,
	CND_LOGIN_RESULT,
	CMD_LOGINOUT,
	CND_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,

	CMD_ERROR
};
//数据包头
struct DataHeader
{
	short dataLength;//数据长度
	short cmd;//数据命令
};

//实际包体
struct Login :public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWorld[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CND_LOGIN_RESULT;
		result = 0;
	}
	int result;

};
struct Loginout :public DataHeader
{
	Loginout()
	{
		dataLength = sizeof(Loginout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];

};
struct LoginoutResult :public DataHeader
{
	LoginoutResult()
	{
		dataLength = sizeof(LoginoutResult);
		cmd = CND_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NEWUSERJOIN :public DataHeader
{
	NEWUSERJOIN()
	{
		dataLength = sizeof(NEWUSERJOIN);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};

int Processor(SOCKET _clientsock)
{

	//设置一个缓冲来接收数据
	char szRecv[4096] = {};
	//5、接收客户端数据
	DataHeader* header = (DataHeader*)szRecv;
	int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);
	if (n_Len <= 0)
	{
		cout << "与服务器断开连接，任务结束" << endl;
		return -1;
	}

	switch (header->cmd)
	{
	case CND_LOGIN_RESULT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		LoginResult* login = (LoginResult*)szRecv;

		cout << "收到服务端消息" << _clientsock << "的请求\t";
		cout << " 指令CND_LOGIN_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
	}
	break;
	case CND_LOGINOUT_RESULT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		LoginoutResult* login = (LoginoutResult*)szRecv;

		cout << "收到服务端消息" << _clientsock << "的请求\t";
		cout << " 指令CND_LOGINOUT_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
	}
	break;
	case CMD_NEW_USER_JOIN://新用户加入的消息
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		NEWUSERJOIN* login = (NEWUSERJOIN*)szRecv;

		cout << "收到服务端消息" << _clientsock << "的请求\t";
		cout << " 指令CMD_NEW_USER_JOIN：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
	}
	break;
	}
}


bool g_bRun = true;
///输入命令所使用
void CmdThread(SOCKET _socket)
{
	char cmdBuffer[256] = {};
	while (true)
	{
		cin >> cmdBuffer;
		if (0 == strcmp(cmdBuffer, "exit"))
		{
			cout << "退出" << endl;
			g_bRun = false;
			return;
		}
		else if (0 == strcmp(cmdBuffer, "login"))
		{
			Login login;
			strcpy(login.userName, "ly");
			strcpy(login.passWorld, "12345");
			send(_socket, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuffer, "loginout"))
		{
			Loginout login;
			strcpy(login.userName, "ly");
			send(_socket, (const char*)&login, sizeof(Loginout), 0);
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}



}



int main()
{
	//版本号,启动windows socket2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1\建立简易tcp客户端
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET==_socket)
	{
		cout << "Error:建立socket" << endl;
	}
	else
	{
		cout << "success:建立socket" << endl;

	}
	//2、连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret= connect(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		cout << "建立socket失败" << endl;
	}
	else
	{
		cout << "建立成功" << endl;

	}

	//3、接收服务器信息
	char recvBuf[128] = {};

	///启动线城
	thread t1(CmdThread,_socket);
	//将线程与主线城分离
	t1.detach();

	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_socket, &fdReads,NULL);
		timeval t = { 2,0 };
		int redt= select(_socket, &fdReads, 0, 0, &t);
		if (redt < 0)
		{
			cout << "select任务结束" << endl;
		}

		///如果有数据可读就进行处理
		if (FD_ISSET(_socket, &fdReads))
		{
			FD_CLR(_socket, &fdReads);
			if (-1 == Processor(_socket))
			{
				cout << "Select任务完成" << endl;
				break;
			}
		}



		Sleep(1500);
	}




	//4、关闭socket连接
	closesocket(_socket);


	//5、清除windowssocket环境
	WSACleanup();




	getchar();


	return 0;
}