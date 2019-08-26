#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include<vector>
//scoket头文件放在windows头文件之前
#include<Windows.h>
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


vector<SOCKET>  g_clients;


int Processor(SOCKET _clientsock)
{

	//设置一个缓冲来接收数据
	char szRecv[4096] = {};		
	//5、接收客户端数据
	DataHeader* header = (DataHeader*)szRecv;
	int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);	 
	if (n_Len <= 0)
	{
		cout << "客户端"<< _clientsock <<"已经退出，任务结束" << endl;
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		//再次接收的时候需要注意接收的地址偏移，在接收头的时候已经将数据完全接收过来了
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		Login* login = (Login*)szRecv;
		cout << "收到" << _clientsock << "的请求\t";
		cout << " 指令CMD_LOGIN：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
		cout << "UserName:" << login->userName << "\tPassWorld:" << login->passWorld << endl;
		//忽略帐号密码是否正确
		LoginResult ret;
		send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		send(_clientsock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		Loginout* loginout = (Loginout*)szRecv;
		cout << "收到" << _clientsock << "的请求\t";
		cout << " 的指令CMD_LOGINOUT：\t" << loginout->cmd << "命令长度:\t" << loginout->dataLength << endl;
		cout << "UserName:" << loginout->userName << endl;

		//忽略帐号密码是否正确
		LoginoutResult reout;

		send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		send(_clientsock, (char*)&reout, sizeof(LoginoutResult), 0);
	}
	break;
	default:
	{
		cout << "收到" << _clientsock << "的请求\t";
		DataHeader header = { 0,CMD_ERROR };
		send(_clientsock, (char*)&header, sizeof(header), 0);
	}
	break;
	}
}

int main()
{
	//版本号
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1、建立一个套接字
	//网络类型、数据形式：数据流类型、协议类型:TCP协议
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2、绑定接受客户端连接的网络端口
	sockaddr_in _sin = {};
	//类型
	_sin.sin_family = AF_INET;
	//htons:主机到网络字节序
	_sin.sin_port = htons(4567);
	//绑定服务其地址
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//当不知道IP地址时，可以使用默认的系统提供的默认地址,对地址不做限定
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	///绑定：socket、scoket详细的网络地址、长度
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf_s("错误，绑定网络端口失败");
	}
	else
	{
		cout << "绑定成功" << endl;
	}
	//3、监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{

		cout << "监听失败" << endl;
	}
	else
	{
		cout << "监听成功" << endl;

	}
	while (true)
	{
		//伯克利socket
		//select,第一个参数在windows中并无实际意义，在Linux\Mac中需要注意，最大描述符交易
		//int
		//	WSAAPI
		//	select(
		//		_In_ int nfds,//是一个整数值是指fd_set集合中所有描述符（socket)的范围,而不是数量，既是所有文件描述符最大值+1,在Windows这个参数可以写0
		//		_Inout_opt_ fd_set FAR * readfds,//可读集合
		//		_Inout_opt_ fd_set FAR * writefds,//可写集合
		//		_Inout_opt_ fd_set FAR * exceptfds,//异常集合
		//		_In_opt_ const struct timeval FAR * timeout//延时
		//	);
		fd_set fdRead;//描述符集合
		fd_set fdWrite;
		fd_set fdExp;
		//清空
		FD_ZERO(&fdRead);//清理集合
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);//将描述符加入集合
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int i = (int)g_clients.size() - 1; i >= 0; i--)
		{
			//接收是否有可读，有的话就放到可读集合里边
			FD_SET(g_clients[i], &fdRead);
		}

		//加入时间timeval，非阻塞
		timeval t = { 1,0 };
		int rret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (rret < 0)
		{
			cout << "Select任务结束" << endl;
			break;
		}
		//判断描述符是否在集合中
		if (FD_ISSET(_sock, &fdRead))
		{
			//清理可读
			FD_CLR(_sock, &fdRead);
			//4、accept等待接收客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(clientAddr);
			SOCKET _clientsock = INVALID_SOCKET;//无效的Scoket地址
			char msgBuf[] = "HELLOM, IM";
			_clientsock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _clientsock)
			{
				cout << "无效客户端SOCKET" << endl;
			}
			else
			{
				//在添加之前将新用户加入的消息广播给其他客户端
				for ( int i = (int)g_clients.size()-1; i >=0; i--)
				{
					NEWUSERJOIN userjoin;
					send(g_clients[i], (const char*)&userjoin, sizeof(NEWUSERJOIN), 0);
				}

				//将客户端加入vector
				g_clients.push_back(_clientsock);
				//inet_ntoa将网络ip变成可读
				cout << "新的客户端加入：IP=" << inet_ntoa(clientAddr.sin_addr) << endl;
			}
		}
		//从可读集合中进行查找
		for (size_t i = 0; i < fdRead.fd_count; i++)
		{
			//如果等于-1，则客户端退出
			if (-1 == Processor(fdRead.fd_array[i]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
				if (iter != g_clients.end())
				{
					//移除
					g_clients.erase(iter);
				}
			}
		}
		//TODO:判断客户端接收长度是否,拆包处理


		//cout << "空闲时间处理其他事情" << endl;

	}


	//6、关闭清理套接字
	for (size_t i = g_clients.size() - 1; i >= 0; i--)
	{
		closesocket(g_clients[i]);
	}
	closesocket(_sock);


	//清除Windows socket环境
	WSACleanup();


	getchar();

	return 0;

}

