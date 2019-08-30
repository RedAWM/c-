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

	//��ʼ���ͻ���
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif		
		if (INVALID_SOCKET != _socket)
		{
			printf("<socket=%d>�رվ�����...\n", _socket);
			Close();
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			printf("���󣬽���Socketʧ��...\n");
		}
		else {
			printf("����Socket�ɹ�...\n");
		}
		return _socket;
	}
	//�󶨶˿�
	int Bind(const char* ip,unsigned short port)
	{

		//2���󶨽��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		//����
		_sin.sin_family = AF_INET;
		//htons:�����������ֽ���
		_sin.sin_port = htons(port);
		//�󶨷������ַ
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		//����֪��IP��ַʱ������ʹ��Ĭ�ϵ�ϵͳ�ṩ��Ĭ�ϵ�ַ,�Ե�ַ�����޶�
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

		///�󶨣�socket��scoket��ϸ�������ַ������
		int ret = bind(_socket, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf_s("���󣬰�����˿�<%d>ʧ��",port);
		}
		else
		{
			printf_s("�󶨶˿�<%d>�ɹ�...\n",port);
		}
		return ret;
	}
	//�����˿ں�,n���ɵȴ���������
	int Listen(int n)
	{
		//3����������˿�
		int ret = listen(_socket, n);
		if (SOCKET_ERROR == ret)
		{
			printf_s("SOCKET=<%d>���󣬼�������˿�ʧ��...\n", (int)_socket);
			//cout << "����ʧ��" << endl;
		}
		else
		{
			//cout << "�����ɹ�" << endl;
			printf_s("SOCKET=<%d>��������˿ڳɹ�...\n", (int)_socket);

		}
		return ret;
	}

	//���ܿͻ�����Ϣ
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _clientsock = INVALID_SOCKET;//��Ч��Scoket��ַ
#ifdef _WIN32
		_clientsock = accept(_socket, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_clientsock = accept(_socket, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif // _WIN32

		if (INVALID_SOCKET == _clientsock)
		{
			printf_s("SOCKET=<%d>���󡣡�����Ч�ͻ���SOCKET\n",(int)_socket);
		}
		else
		{
			//�����֮ǰ�����û��������Ϣ�㲥�������ͻ���
			NEWUSERJOIN userjoin;
			SendDataToAll(&userjoin);
			//���ͻ��˼���vector
			g_clients.push_back(_clientsock);
			//inet_ntoa������ip��ɿɶ�
			printf_s( "SOCKET=<%d>�µĿͻ��˼���:socket=%d , IP=%s\n" ,(int)_socket,(int)_clientsock,inet_ntoa(clientAddr.sin_addr));
		}
		return _clientsock;
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdRead;//����������
			fd_set fdWrite;
			fd_set fdExp;
			//���
			FD_ZERO(&fdRead);//������
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_socket, &fdRead);//�����������뼯��
			FD_SET(_socket, &fdWrite);
			FD_SET(_socket, &fdExp);

			///���������������ĳ���
			SOCKET maxSock = _socket;

			for (int i = (int)g_clients.size() - 1; i >= 0; i--)
			{
				//�����Ƿ��пɶ����еĻ��ͷŵ��ɶ��������
				FD_SET(g_clients[i], &fdRead);
				if (maxSock < g_clients[i])
				{
					maxSock = g_clients[i];
				}
			}
			//����ʱ��timeval��������
			timeval t = { 0,0 };
			int rret = select(_socket + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (rret < 0)
			{
			//	cout << "Select�������" << endl;
				printf_s("Select�������");
				return false;
			}
			//�ж��������Ƿ��ڼ�����
			if (FD_ISSET(_socket, &fdRead))
			{
				//����ɶ�
				FD_CLR(_socket, &fdRead);
				//4��accept�ȴ����տͻ�������
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
				//�������-1����ͻ����˳�
				if (-1 == RecvData(fdRead.fd_array[i]))
				{
					auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
					if (iter != g_clients.end())
					{
						//�Ƴ�
						g_clients.erase(iter);
					}
				}
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _socket != INVALID_SOCKET;
	}

	//�������� ����ճ�� ���
	int RecvData(SOCKET _clientsock)
	{

		//����һ����������������
		char szRecv[4096] = {};
		//5�����տͻ�������
		DataHeader* header = (DataHeader*)szRecv;
		int n_Len = recv(_clientsock, (char*)&szRecv, sizeof(DataHeader), 0);
		if (n_Len <= 0)
		{
			//cout << "�ͻ���" << _clientsock << "�Ѿ��˳����������" << endl;
			printf_s("�ͻ���socket<%d>,�Ѿ��˳����������\n", (int)_clientsock);
			return -1;
		}
		//�ٴν��յ�ʱ����Ҫע����յĵ�ַƫ�ƣ��ڽ���ͷ��ʱ���Ѿ���������ȫ���չ�����
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
			printf_s("�յ��ͻ���<Socket=%d>���� :ָ��CMD_LOGIN��%d\t,�����:%d\n",(int)_clientsock, login->cmd, login->dataLength);
			//�����ʺ������Ƿ���ȷ
			LoginResult ret;
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
			send(_clientsock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			Loginout* loginout = (Loginout*)header;
			printf_s("ָ��CMD_LOGINOUT��<%d>\t,�����:<%d>\n", loginout->cmd, loginout->dataLength);
			//�����ʺ������Ƿ���ȷ
			LoginoutResult reout;
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
			send(_clientsock, (char*)&reout, sizeof(LoginoutResult), 0);
		}
		break;
		default:
		{
			//cout << "�յ�" << _clientsock << "������\t";
			DataHeader header = { 0,CMD_ERROR };
			send(_clientsock, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
		}
	}

	//����ָ��Socket����
	int SendData(SOCKET _sock,DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//Ⱥ��
	void SendDataToAll( DataHeader* header)
	{
		for (int i = (int)g_clients.size() - 1; i >= 0; i--)
		{
			SendData(g_clients[i], header);
		}
	}


	//�ر�socket
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


			//���Windows socket����
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

