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
	printf_s("������Ѿ��˳�");




#pragma region MyRegion
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
//�ӿɶ������н��в���
//TODO:�жϿͻ��˽��ճ����Ƿ�,�������
//cout << "����ʱ�䴦����������" << endl;  
#pragma endregion

		



	//6���ر������׽���



	getchar();

	return 0;

}

