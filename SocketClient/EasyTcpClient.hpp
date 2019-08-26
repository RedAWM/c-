#ifndef _EASYTCPCLIENT_HPP_
#define _EASYTCPCLIENT_HPP_
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
//scoketͷ�ļ�����windowsͷ�ļ�֮ǰ
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
//����WIndows���صľ�̬��
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

	///��ʼ��socket
	void InitSocket()
	{
		//����WINsocket����
#ifdef _WIN32
//�汾��,����windows socket2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif // _WIN32

		if (INVALID_SOCKET != _socket)
		{
			close();
			cout << "�رվɵ�����" << endl;
		}
		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _socket)
		{
			cout << "Error:����socket" << endl;
		}
		else
		{
			cout << "success:����socket" << endl;
		}

	}

	//���ӷ�����
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
			cout << "����socketʧ��" << endl;
		}
		else
		{
			cout << "�����ɹ�" << endl;

		}
		return ret;
	}

	//�رշ�����
	void close()
	{
		if (INVALID_SOCKET!= _socket)
		{
			//4���ر�socket����
#ifdef _WIN32

			closesocket(_socket);
#else
			close(_socket);
#endif // _WIN32


#ifdef _WIN32
			//5�����windowssocket����
			WSACleanup();
#endif // _WIN32
			_socket = INVALID_SOCKET;
		}

	}

	

	//����������Ϣ
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
				cout << _socket << ":\tselect�������" << endl;
				return false;
			}

			///��������ݿɶ��ͽ��д���
			if (FD_ISSET(_socket, &fdReads))
			{
				FD_CLR(_socket, &fdReads);
				if (-1 == RecvData())
				{
					cout << _socket << ":\tSelect�������" << endl;
					return false;
				}
			}
		}
	}
	//�Ƿ���������
	bool isRun()
	{
		return _socket != INVALID_SOCKET;
	}

	//��������:����ճ������ְ�
	int RecvData()
	{
		//����һ����������������
		char szRecv[4096] = {};
		//5�����տͻ�������
		DataHeader* header = (DataHeader*)szRecv;
		int n_Len = (int)recv(_socket, (char*)&szRecv, sizeof(DataHeader), 0);
		if (n_Len <= 0)
		{
			cout << "��������Ͽ����ӣ��������" << endl;
			return -1;
		}
		recv(_socket, szRecv + sizeof(header), header->dataLength - sizeof(header), 0);
		OnNetMsg( header);

		return 0;
	}

	//��Ӧ������Ϣ
	void OnNetMsg( DataHeader* header )
	{
		switch (header->cmd)
		{
		case CND_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			cout << "�յ��������Ϣ" << _socket << "������\t";
			cout << " ָ��CND_LOGIN_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
		}
		break;
		case CND_LOGINOUT_RESULT:
		{
			LoginoutResult* login = (LoginoutResult*)header;

			cout << "�յ��������Ϣ" << _socket << "������\t";
			cout << " ָ��CND_LOGINOUT_RESULT��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
		}
		break;
		case CMD_NEW_USER_JOIN://���û��������Ϣ
		{
			NEWUSERJOIN* login = (NEWUSERJOIN*)header;

			cout << "�յ��������Ϣ" << _socket << "������\t";
			cout << " ָ��CMD_NEW_USER_JOIN��\t" << login->cmd << "�����:\t" << login->dataLength << endl;
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
