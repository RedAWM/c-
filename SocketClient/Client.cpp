#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
//scoketͷ�ļ�����windowsͷ�ļ�֮ǰ
#include<Windows.h>
#include<thread>

//����WIndows���صľ�̬��
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
//���ݰ�ͷ
struct DataHeader
{
	short dataLength;//���ݳ���
	short cmd;//��������
};

//ʵ�ʰ���
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

	//����һ����������������
	char szRecv[4096] = {};
	//5�����տͻ�������
	DataHeader* header = (DataHeader*)szRecv;
	int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);
	if (n_Len <= 0)
	{
		cout << "��������Ͽ����ӣ��������" << endl;
		return -1;
	}

	switch (header->cmd)
	{
	case CND_LOGIN_RESULT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		LoginResult* login = (LoginResult*)szRecv;

		cout << "�յ��������Ϣ" << _clientsock << "������\t";
		cout << " ָ��CND_LOGIN_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
	}
	break;
	case CND_LOGINOUT_RESULT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		LoginoutResult* login = (LoginoutResult*)szRecv;

		cout << "�յ��������Ϣ" << _clientsock << "������\t";
		cout << " ָ��CND_LOGINOUT_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
	}
	break;
	case CMD_NEW_USER_JOIN://���û��������Ϣ
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		NEWUSERJOIN* login = (NEWUSERJOIN*)szRecv;

		cout << "�յ��������Ϣ" << _clientsock << "������\t";
		cout << " ָ��CMD_NEW_USER_JOIN��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
	}
	break;
	}
}


bool g_bRun = true;
///����������ʹ��
void CmdThread(SOCKET _socket)
{
	char cmdBuffer[256] = {};
	while (true)
	{
		cin >> cmdBuffer;
		if (0 == strcmp(cmdBuffer, "exit"))
		{
			cout << "�˳�" << endl;
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
			cout << "��֧�ֵ�����" << endl;
		}
	}



}



int main()
{
	//�汾��,����windows socket2.x����
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1\��������tcp�ͻ���
	SOCKET _socket = socket(AF_INET, SOCK_STREAM, 0);

	if (INVALID_SOCKET==_socket)
	{
		cout << "Error:����socket" << endl;
	}
	else
	{
		cout << "success:����socket" << endl;

	}
	//2�����ӷ�����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret= connect(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));

	if (SOCKET_ERROR == ret)
	{
		cout << "����socketʧ��" << endl;
	}
	else
	{
		cout << "�����ɹ�" << endl;

	}

	//3�����շ�������Ϣ
	char recvBuf[128] = {};

	///�����߳�
	thread t1(CmdThread,_socket);
	//���߳������߳Ƿ���
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
			cout << "select�������" << endl;
		}

		///��������ݿɶ��ͽ��д���
		if (FD_ISSET(_socket, &fdReads))
		{
			FD_CLR(_socket, &fdReads);
			if (-1 == Processor(_socket))
			{
				cout << "Select�������" << endl;
				break;
			}
		}



		Sleep(1500);
	}




	//4���ر�socket����
	closesocket(_socket);


	//5�����windowssocket����
	WSACleanup();




	getchar();


	return 0;
}