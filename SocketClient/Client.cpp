#include"EasyTcpClient.hpp"
using namespace std;


#pragma region Processor
//int Processor(SOCKET _clientsock)
//{
//
//	//设置一个缓冲来接收数据
//	char szRecv[4096] = {};
//	//5、接收客户端数据
//	DataHeader* header = (DataHeader*)szRecv;
//	int n_Len = (int)recv(_clientsock, (char*)&szRecv,sizeof(DataHeader), 0);
//	if (n_Len <= 0)
//	{
//		cout << "与服务器断开连接，任务结束" << endl;
//		return -1;
//	}
//
//	switch (header->cmd)
//	{
//	case CND_LOGIN_RESULT:
//	{
//		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
//		LoginResult* login = (LoginResult*)szRecv;
//
//		cout << "收到服务端消息" << _clientsock << "的请求\t";
//		cout << " 指令CND_LOGIN_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
//	}
//	break;
//	case CND_LOGINOUT_RESULT:
//	{
//		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
//		LoginoutResult* login = (LoginoutResult*)szRecv;
//
//		cout << "收到服务端消息" << _clientsock << "的请求\t";
//		cout << " 指令CND_LOGINOUT_RESULT：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
//	}
//	break;
//	case CMD_NEW_USER_JOIN://新用户加入的消息
//	{
//		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
//		NEWUSERJOIN* login = (NEWUSERJOIN*)szRecv;
//
//		cout << "收到服务端消息" << _clientsock << "的请求\t";
//		cout << " 指令CMD_NEW_USER_JOIN：\t" << login->cmd << "命令长度:\t" << login->dataLength << endl;
//	}
//	break;
//	}
//
//	return 0;
//}
//

#pragma endregion

///输入命令所使用
void CmdThread(EasyTcpClient* _client)
{
	char cmdBuffer[256] = {};
	while (true)
	{
		cin >> cmdBuffer;
		if (0 == strcmp(cmdBuffer, "exit"))
		{
			_client->close();
			cout << "退出" << endl;
			return;
		}
		else if (0 == strcmp(cmdBuffer, "login"))
		{
			Login login;
			strcpy(login.userName, "ly");
			strcpy(login.passWorld, "12345");
			_client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuffer, "loginout"))
		{
			Loginout loginout;
			strcpy(loginout.userName, "ly");
			_client->SendData(&loginout);
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}



}



int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);
	///启动线城
	thread t1(CmdThread, &client);
	//将线程与主线城分离
	t1.detach();

	while (client.isRun())
	{
		client.isRun();

	}


	client.close();



	return 0;
}