#include"EasyTcpClient.hpp"
using namespace std;


#pragma region Processor
//int Processor(SOCKET _clientsock)
//{
//
//	//����һ����������������
//	char szRecv[4096] = {};
//	//5�����տͻ�������
//	DataHeader* header = (DataHeader*)szRecv;
//	int n_Len = (int)recv(_clientsock, (char*)&szRecv,sizeof(DataHeader), 0);
//	if (n_Len <= 0)
//	{
//		cout << "��������Ͽ����ӣ��������" << endl;
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
//		cout << "�յ��������Ϣ" << _clientsock << "������\t";
//		cout << " ָ��CND_LOGIN_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
//	}
//	break;
//	case CND_LOGINOUT_RESULT:
//	{
//		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
//		LoginoutResult* login = (LoginoutResult*)szRecv;
//
//		cout << "�յ��������Ϣ" << _clientsock << "������\t";
//		cout << " ָ��CND_LOGINOUT_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
//	}
//	break;
//	case CMD_NEW_USER_JOIN://���û��������Ϣ
//	{
//		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
//		NEWUSERJOIN* login = (NEWUSERJOIN*)szRecv;
//
//		cout << "�յ��������Ϣ" << _clientsock << "������\t";
//		cout << " ָ��CMD_NEW_USER_JOIN��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
//	}
//	break;
//	}
//
//	return 0;
//}
//

#pragma endregion

///����������ʹ��
void CmdThread(EasyTcpClient* _client)
{
	char cmdBuffer[256] = {};
	while (true)
	{
		cin >> cmdBuffer;
		if (0 == strcmp(cmdBuffer, "exit"))
		{
			_client->close();
			cout << "�˳�" << endl;
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
			cout << "��֧�ֵ�����" << endl;
		}
	}



}



int main()
{
	EasyTcpClient client;
	client.InitSocket();
	client.Connect("127.0.0.1", 4567);
	///�����߳�
	thread t1(CmdThread, &client);
	//���߳������߳Ƿ���
	t1.detach();

	while (client.isRun())
	{
		client.isRun();

	}


	client.close();



	return 0;
}