#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <iostream>


#include<vector>
//using namespace std;
#include"EasyTcpServer.hpp"







int main()
{
	EasyTcpServer server ;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	while (server.isRun())
	{
		server.OnRun();
	}
	server.Close();
	printf_s("服务端已经退出");




#pragma region MyRegion
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
//从可读集合中进行查找
//TODO:判断客户端接收长度是否,拆包处理
//cout << "空闲时间处理其他事情" << endl;  
#pragma endregion

		



	//6、关闭清理套接字



	getchar();

	return 0;

}

