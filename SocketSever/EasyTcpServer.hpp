#ifndef _EASYTCPSERVER_HPP_
#define _EASYTCPSERVER_HPP_

#ifdef _WIN32
#define FD_SETSIZE      2506
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
#include<thread>
#include<mutex>
#include<atomic>
#include"MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _socket;
	std::vector<SOCKET>  g_clients;
	

public:
	EasyTcpServer()
	{
		_socket = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化客户端
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif		
		if (INVALID_SOCKET != _socket)
		{
			printf("<socket=%d>关闭旧连接...\n", _socket);
			Close();
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			printf("错误，建立Socket失败...\n");
		}
		else {
			printf("建立Socket成功...\n");
		}
		return _socket;
	}
	//绑定端口
	int Bind(const char* ip,unsigned short port)
	{

		//2、绑定接受客户端连接的网络端口
		sockaddr_in _sin = {};
		//类型
		_sin.sin_family = AF_INET;
		//htons:主机到网络字节序
		_sin.sin_port = htons(port);
		//绑定服务其地址
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		//当不知道IP地址时，可以使用默认的系统提供的默认地址,对地址不做限定
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip==nullptr)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif // _WIN32

		///绑定：socket、scoket详细的网络地址、长度
		int ret = bind(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf_s("错误，绑定网络端口<%d>失败",port);
		}
		else
		{
			printf_s("绑定端口<%d>成功...\n",port);
		}
		return ret;
	}
	//监听端口号,n：可等待的连接数
	int Listen(int n)
	{
		//3、监听网络端口
		int ret = listen(_socket, n);
		if (SOCKET_ERROR == ret)
		{
			printf_s("SOCKET=<%d>错误，监听网络端口失败...\n", (int)_socket);
			//cout << "监听失败" << endl;
		}
		else
		{
			//cout << "监听成功" << endl;
			printf_s("SOCKET=<%d>监听网络端口成功...\n", (int)_socket);

		}
		return ret;
	}

	//接受客户端消息
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _clientsock = INVALID_SOCKET;//无效的Scoket地址
#ifdef _WIN32
		_clientsock = accept(_socket, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_clientsock = accept(_socket, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif // _WIN32

		if (INVALID_SOCKET == _clientsock)
		{
			printf_s("SOCKET=<%d>错误。。。无效客户端SOCKET\n",(int)_socket);
		}
		else
		{
			//在添加之前将新用户加入的消息广播给其他客户端
			NEWUSERJOIN userjoin;
			SendDataToAll(&userjoin);
			//将客户端加入vector
			g_clients.push_back(_clientsock);
			//inet_ntoa将网络ip变成可读
			printf_s( "SOCKET=<%d>新的客户端加入:socket=%d , IP=%s\n" ,(int)_socket,(int)_clientsock,inet_ntoa(clientAddr.sin_addr));
		}
		return _clientsock;
	}
	//处理网络信息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;//描述符集合
			fd_set fdWrite;
			fd_set fdExp;
			//清空
			FD_ZERO(&fdRead);//清理集合
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_socket, &fdRead);//将描述符加入集合
			FD_SET(_socket, &fdWrite);
			FD_SET(_socket, &fdExp);

			///计算出最大描述符的长度
			SOCKET maxSock = _socket;

			for (int i = (int)g_clients.size() - 1; i >= 0; i--)
			{
				//接收是否有可读，有的话就放到可读集合里边
				FD_SET(g_clients[i], &fdRead);
				if (maxSock < g_clients[i])
				{
					maxSock = g_clients[i];
				}
			}
			//加入时间timeval，非阻塞
			timeval t = { 0,0 };
			int rret = select(_socket + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (rret < 0)
			{
			//	cout << "Select任务结束" << endl;
				printf_s("Select任务结束");
				return false;
			}
			//判断描述符是否在集合中
			if (FD_ISSET(_socket, &fdRead))
			{
				//清理可读
				FD_CLR(_socket, &fdRead);
				//4、accept等待接收客户端连接
				Accept();
			}
			//for (int i = (int)g_clients.size()-1; i >=0; i--)
			//{
			//	if (FD_ISSET(g_clients[i], &fdRead))
			//	{
			//		if (-1 == RecvData(fdRead.fd_array[i]))
			//		{
			//			auto iter = g_clients.begin()+i;
			//			if (iter!=g_clients.end())
			//			{
			//				g_clients.erase(iter);
			//			}
			//		}

			//	}
			//}
			for (size_t i = 0; i < fdRead.fd_count; i++)
			{
				//如果等于-1，则客户端退出
				if (-1 == RecvData(fdRead.fd_array[i]))
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
					if (iter != g_clients.end())
					{
						//移除
						g_clients.erase(iter);
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _socket != INVALID_SOCKET;
	}

	//接收数据 处理粘包 拆包
	int RecvData(SOCKET _clientsock)
	{

		//设置一个缓冲来接收数据
		char szRecv[4096] = {};
		//5、接收客户端数据
		DataHeader* header = (DataHeader*)szRecv;
		int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);
		if (n_Len <= 0)
		{
			//cout << "客户端" << _clientsock << "已经退出，任务结束" << endl;
			printf_s("客户端socket<%d>,已经退出，任务结束\n", (int)_clientsock);
			return -1;
		}
		//再次接收的时候需要注意接收的地址偏移，在接收头的时候已经将数据完全接收过来了
		recv(_clientsock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_clientsock, header);
		return 0;
	}

	virtual void OnNetMsg(SOCKET _clientsock, DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf_s("收到客户端<Socket=%d>请求 :指令CMD_LOGIN：%d\t,命令长度:%d\n",(int)_clientsock, login->cmd, login->dataLength);
			//忽略帐号密码是否正确
			LoginResult ret;
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
			send(_clientsock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			Loginout* loginout = (Loginout*)header;
			printf_s("指令CMD_LOGINOUT：<%d>\t,命令长度:<%d>\n", loginout->cmd, loginout->dataLength);
			//忽略帐号密码是否正确
			LoginoutResult reout;
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
			send(_clientsock, (char*)&reout, sizeof(LoginoutResult), 0);
		}
		break;
		default:
		{
			//cout << "收到" << _clientsock << "的请求\t";
			DataHeader header = { 0,CMD_ERROR };
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		}
	}

	//发送指定Socket数据
	int SendData(SOCKET _sock,DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//群发
	void SendDataToAll( DataHeader* header)
	{
		for (int i = (int)g_clients.size() - 1; i >= 0; i--)
		{
			SendData(g_clients[i], header);
		}
	}


	//关闭socket
	void Close()
	{
		if (_socket != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (size_t i = (int)g_clients.size() - 1; i >= 0; i--)
			{
				closesocket(g_clients[i]);
			}
			closesocket(_socket);


			//清除Windows socket环境
			WSACleanup();
#else

			for (int i = (int)g_clients.size() - 1; i >= 0; i--)
			{
				closesocket(g_clients[i]);
			}
			close(_socket);

#endif // _WIN32
			g_clients.clear();
		}
	}

private:

};







#endif // !_EASYTCPSERVER_HPP

