#ifndef _MESSAGEHEADER_HPP_
#define _MESSAGEHEADER_HPP_
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

#endif // !_MESSAGEHEADER_HPP_

