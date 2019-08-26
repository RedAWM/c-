#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include<vector>
//scoketͷ�ļ�����windowsͷ�ļ�֮ǰ
#include<Windows.h>
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


vector<SOCKET>  g_clients;


int Processor(SOCKET _clientsock)
{

	//����һ����������������
	char szRecv[4096] = {};		
	//5�����տͻ�������
	DataHeader* header = (DataHeader*)szRecv;
	int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);	 
	if (n_Len <= 0)
	{
		cout << "�ͻ���"<< _clientsock <<"�Ѿ��˳����������" << endl;
		return -1;
	}

	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		//�ٴν��յ�ʱ����Ҫע����յĵ�ַƫ�ƣ��ڽ���ͷ��ʱ���Ѿ���������ȫ���չ�����
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		Login* login = (Login*)szRecv;
		cout << "�յ�" << _clientsock << "������\t";
		cout << " ָ��CMD_LOGIN��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
		cout << "UserName:" << login->userName << "\tPassWorld:" << login->passWorld << endl;
		//�����ʺ������Ƿ���ȷ
		LoginResult ret;
		send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		send(_clientsock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(_clientsock, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		Loginout* loginout = (Loginout*)szRecv;
		cout << "�յ�" << _clientsock << "������\t";
		cout << " ��ָ��CMD_LOGINOUT��\t" << loginout->cmd << "�����:\t" << loginout->dataLength << endl;
		cout << "UserName:" << loginout->userName << endl;

		//�����ʺ������Ƿ���ȷ
		LoginoutResult reout;

		send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		send(_clientsock, (char*)&reout, sizeof(LoginoutResult), 0);
	}
	break;
	default:
	{
		cout << "�յ�" << _clientsock << "������\t";
		DataHeader header = { 0,CMD_ERROR };
		send(_clientsock, (char*)&header, sizeof(header), 0);
	}
	break;
	}
}

int main()
{
	//�汾��
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//1������һ���׽���
	//�������͡�������ʽ�����������͡�Э������:TCPЭ��
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2���󶨽��ܿͻ������ӵ�����˿�
	sockaddr_in _sin = {};
	//����
	_sin.sin_family = AF_INET;
	//htons:�����������ֽ���
	_sin.sin_port = htons(4567);
	//�󶨷������ַ
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//����֪��IP��ַʱ������ʹ��Ĭ�ϵ�ϵͳ�ṩ��Ĭ�ϵ�ַ,�Ե�ַ�����޶�
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	///�󶨣�socket��scoket��ϸ�������ַ������
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf_s("���󣬰�����˿�ʧ��");
	}
	else
	{
		cout << "�󶨳ɹ�" << endl;
	}
	//3����������˿�
	if (SOCKET_ERROR == listen(_sock, 5))
	{

		cout << "����ʧ��" << endl;
	}
	else
	{
		cout << "�����ɹ�" << endl;

	}
	while (true)
	{
		//������socket
		//select,��һ��������windows�в���ʵ�����壬��Linux\Mac����Ҫע�⣬�������������
		//int
		//	WSAAPI
		//	select(
		//		_In_ int nfds,//��һ������ֵ��ָfd_set������������������socket)�ķ�Χ,���������������������ļ����������ֵ+1,��Windows�����������д0
		//		_Inout_opt_ fd_set FAR * readfds,//�ɶ�����
		//		_Inout_opt_ fd_set FAR * writefds,//��д����
		//		_Inout_opt_ fd_set FAR * exceptfds,//�쳣����
		//		_In_opt_ const struct timeval FAR * timeout//��ʱ
		//	);
		fd_set fdRead;//����������
		fd_set fdWrite;
		fd_set fdExp;
		//���
		FD_ZERO(&fdRead);//������
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);//�����������뼯��
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int i = (int)g_clients.size() - 1; i >= 0; i--)
		{
			//�����Ƿ��пɶ����еĻ��ͷŵ��ɶ��������
			FD_SET(g_clients[i], &fdRead);
		}

		//����ʱ��timeval��������
		timeval t = { 1,0 };
		int rret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (rret < 0)
		{
			cout << "Select�������" << endl;
			break;
		}
		//�ж��������Ƿ��ڼ�����
		if (FD_ISSET(_sock, &fdRead))
		{
			//����ɶ�
			FD_CLR(_sock, &fdRead);
			//4��accept�ȴ����տͻ�������
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(clientAddr);
			SOCKET _clientsock = INVALID_SOCKET;//��Ч��Scoket��ַ
			char msgBuf[] = "HELLOM, IM";
			_clientsock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _clientsock)
			{
				cout << "��Ч�ͻ���SOCKET" << endl;
			}
			else
			{
				//�����֮ǰ�����û��������Ϣ�㲥�������ͻ���
				for ( int i = (int)g_clients.size()-1; i >=0; i--)
				{
					NEWUSERJOIN userjoin;
					send(g_clients[i], (const char*)&userjoin, sizeof(NEWUSERJOIN), 0);
				}

				//���ͻ��˼���vector
				g_clients.push_back(_clientsock);
				//inet_ntoa������ip��ɿɶ�
				cout << "�µĿͻ��˼��룺IP=" << inet_ntoa(clientAddr.sin_addr) << endl;
			}
		}
		//�ӿɶ������н��в���
		for (size_t i = 0; i < fdRead.fd_count; i++)
		{
			//�������-1����ͻ����˳�
			if (-1 == Processor(fdRead.fd_array[i]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
				if (iter != g_clients.end())
				{
					//�Ƴ�
					g_clients.erase(iter);
				}
			}
		}
		//TODO:�жϿͻ��˽��ճ����Ƿ�,�������


		//cout << "����ʱ�䴦����������" << endl;

	}


	//6���ر������׽���
	for (size_t i = g_clients.size() - 1; i >= 0; i--)
	{
		closesocket(g_clients[i]);
	}
	closesocket(_sock);


	//���Windows socket����
	WSACleanup();


	getchar();

	return 0;

}

