#ifndef _EASYTCPCLIENT_HPP_
#define _EASYTCPCLIENT_HPP_
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
//scoket头文件放在windows头文件之前
#ifdef _WIN32
#include <WinSock2.h>
#include<Windows.h>
#else
#include<unistd.h>//uni std
#include<arpa/iner.h>
#include<string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif // _WIN32
#include<thread>
using namespace std;
//引入WIndows本地的静态库
#pragma comment(lib,"ws2_32.lib")
#include"MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _socket;


public:
	EasyTcpClient()
	{
		_socket = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient()
	{

	}

	///初始化socket
	void InitSocket()
	{
		//启动WINsocket环境
#ifdef _WIN32
//版本号,启动windows socket2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32

		if (INVALID_SOCKET != _socket)
		{
			close();
			cout << "关闭旧的连接" << endl;
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			cout << "Error:建立socket" << endl;
		}
		else
		{
			cout << "success:建立socket" << endl;
		}

	}

	//连接服务器
	int Connect(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == _socket)
		{
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

#else
		_sin.sin_addr.s_addr = inet_addr("192.168.0.161");
#endif // _WIN32
		int ret = connect(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));

		if (SOCKET_ERROR == ret)
		{
			cout << "建立socket失败" << endl;
		}
		else
		{
			cout << "建立成功" << endl;

		}
		return ret;
	}

	//关闭服务器
	void close()
	{
		if (INVALID_SOCKET!= _socket)
		{
			//4、关闭socket连接
#ifdef _WIN32

			closesocket(_socket);
#else
			close(_socket);
#endif // _WIN32


#ifdef _WIN32
			//5、清除windowssocket环境
			WSACleanup();
#endif // _WIN32
			_socket = INVALID_SOCKET;
		}

	}

	

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_socket, &fdReads, NULL);
			timeval t = { 0,0 };
			int redt = select(_socket + 1, &fdReads, 0, 0, &t);
			if (redt < 0)
			{
				cout << _socket << ":\tselect任务结束" << endl;
				return false;
			}

			///如果有数据可读就进行处理
			if (FD_ISSET(_socket, &fdReads))
			{
				FD_CLR(_socket, &fdReads);
				if (-1 == RecvData())
				{
					cout << _socket << ":\tSelect任务完成" << endl;
					return false;
				}
			}
		}
	}
	//是否在运行中
	bool isRun()
	{
		return _socket != INVALID_SOCKET;
	}

	//接收数据:处理粘包、拆分包
	int RecvData()
	{
		//设置一个缓冲来接收数据
		char szRecv[4096] = {};
		//5、接收客户端数据
		DataHeader* header = (DataHeader*)szRecv;
		int n_Len = (int)recv(_socket, (char*)&szRecv, sizeof(DataHeader), 0);
		if (n_Len <= 0)
		{
			cout << "与服务器断开连接，任务结束" << endl;
			return -1;
		}
		recv(_socket, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		OnNetMsg( header);

		return 0;
	}

	//响应网络消息
	void OnNetMsg( DataHeader* header )
	{
		switch (header->cmd)
		{
		case CND_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			cout << "收到服务端消息" << _socket << "的请求\t";
			cout << " 指令CND_LOGIN_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
		}
		break;
		case CND_LOGINOUT_RESULT:
		{
			LoginoutResult* login = (LoginoutResult*)header;

			cout << "收到服务端消息" << _socket << "的请求\t";
			cout << " 指令CND_LOGINOUT_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
		}
		break;
		case CMD_NEW_USER_JOIN://新用户加入的消息
		{
			NEWUSERJOIN* login = (NEWUSERJOIN*)header;

			cout << "收到服务端消息" << _socket << "的请求\t";
			cout << " 指令CMD_NEW_USER_JOIN：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
		}
		break;
		}
	}

	int SendData(DataHeader* header)
	{
		if (&header&&isRun())
		{
		 	return send(_socket, (const char*)header, header->dataLength, 0);

		}
	}

private:

};




#endif // !_EASYTCPCLIENT_HPP_
