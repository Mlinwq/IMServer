#pragma once
#define _DEF_UDP_PORT (8899)
#define _DEF_BUFFER_SIZE (4096)
#define _DEF_NAME_SIZE (60)
#define _DEF_CONTENT_SIZE (1024)
#define _DEF_SERVER_IP ("192.168.31.78")
#define _DEF_TCP_PORT (9988)
#define _DEF_MAX_SIZE (100)
#define _DEF_PROTOCAL_COUNT (100)

//协议头
//UDP协议头
#define _DEF_PROTOCAL_BASE (1000)
//上线
#define _DEF_PROTOCAL_ONLINE_RQ (_DEF_PROTOCAL_BASE + 1)
#define _DEF_PROTOCAL_ONLINE_RS (_DEF_PROTOCAL_BASE + 2)
//下线
#define _DEF_PROTOCAL_OFFLINE_RQ (_DEF_PROTOCAL_BASE + 3)
//聊天
#define _DEF_PROTOCAL_CHAT_RQ (_DEF_PROTOCAL_BASE + 4)
//传文件
#define _DEF_PROTOCAL_FILEINFO_RQ (_DEF_PROTOCAL_BASE + 5)
//......

//TCP协议头
//注册
#define _DEF_PACK_REGISTER_RQ (_DEF_PROTOCAL_BASE + 10)
#define _DEF_PACK_REGISTER_RS (_DEF_PROTOCAL_BASE + 11)
//登录
#define _DEF_PACK_LOGIN_RQ (_DEF_PROTOCAL_BASE + 12)
#define _DEF_PACK_LOGIN_RS (_DEF_PROTOCAL_BASE + 13)
//好友信息
#define _DEF_PACK_FRIEND_INFO (_DEF_PROTOCAL_BASE + 14)
//聊天
#define _DEF_PACK_CHAT_RQ (_DEF_PROTOCAL_BASE + 15)
#define _DEF_PACK_CHAT_RS (_DEF_PROTOCAL_BASE + 16)
//添加好友
#define _DEF_PACK_ADDFRIEND_RQ (_DEF_PROTOCAL_BASE + 17)
#define _DEF_PACK_ADDFRIEND_RS (_DEF_PROTOCAL_BASE + 18)
//离线
#define _DEF_PACK_OFFLINE_RQ (_DEF_PROTOCAL_BASE + 19)

//TCP返回结果
//注册请求结果
#define user_is_exist (0)
#define register_sucess (1)
//登录请求结果
#define user_not_exist (0)
#define password_error (1)
#define login_success (2)
//添加好友请求结果
#define no_this_user (0)
#define user_refuse (1)
#define user_offline (2)
#define add_success (3)
//好友状态
#define status_online (0)
#define status_offline (1)
//聊天信息发送结果
#define send_error (0)
#define send_success (1)
#define user_offline (2)



//请求结构体
//UDP
//上线通知：自己IP、自己主机名
//上线回复：自己IP、自己主机名
struct STRU_ONLINE
{
	STRU_ONLINE():nType(_DEF_PROTOCAL_ONLINE_RQ)
	{
		memset(szName, 0, _DEF_NAME_SIZE);
	}
	int nType;
	//long ip;
	char szName[100];
};

//下线通知：自己IP
struct STRU_OFFLINE_RQ
{
	STRU_OFFLINE_RQ() :nType(_DEF_PROTOCAL_OFFLINE_RQ)
	{
	}
	int nType;
};

//聊天请求：自己IP、聊天内容
struct STRU_CHAT_RQ
{
	STRU_CHAT_RQ() :nType(_DEF_PROTOCAL_CHAT_RQ)
	{
		memset(szContent, 0, _DEF_CONTENT_SIZE);
	}
	int nType;
	char szContent[_DEF_CONTENT_SIZE];
};

//TCP
typedef int PackType;
//注册
typedef struct STRU_REGISTER_RQ 
{
	STRU_REGISTER_RQ():type(_DEF_PACK_REGISTER_RQ)
	{
		memset(tel, 0, _DEF_MAX_SIZE);
		memset(name, 0, _DEF_MAX_SIZE);
		memset(password, 0, _DEF_MAX_SIZE);
	}
	//协议头、手机号、昵称、密码
	PackType type;
	char tel[_DEF_MAX_SIZE];
	char name[_DEF_MAX_SIZE];
	char password[_DEF_MAX_SIZE];
}STRU_REGISTER_RQ;

typedef struct STRU_REGISTER_RS
{
	STRU_REGISTER_RS() :type(_DEF_PACK_REGISTER_RS), result(register_sucess)
	{
	}
	//协议头、注册结果
	PackType type;
	int result;
}STRU_REGISTER_RS;

//登录
typedef struct STRU_LOGIN_RQ
{
	STRU_LOGIN_RQ() :type(_DEF_PACK_LOGIN_RQ)
	{
		memset(tel, 0, _DEF_MAX_SIZE);
		memset(password, 0, _DEF_MAX_SIZE);
	}
	//协议头、手机号、密码
	PackType type;
	char tel[_DEF_MAX_SIZE];
	char password[_DEF_MAX_SIZE];
}STRU_LOGIN_RQ;

typedef struct STRU_LOGIN_RS
{
	STRU_LOGIN_RS() :type(_DEF_PACK_LOGIN_RS),result(login_success),userid(0)
	{
	}
	//协议头、登录结果、用户id
	PackType type;
	int result;
	int userid;
}STRU_LOGIN_RS;

//好友信息
typedef struct STRU_FRIEND_INFO
{
	STRU_FRIEND_INFO() :type(_DEF_PACK_FRIEND_INFO),userid(0),icon(0),state(status_online)
	{
		memset(tel, 0, _DEF_MAX_SIZE);
		memset(name, 0, _DEF_MAX_SIZE);
		memset(feeling, 0, _DEF_MAX_SIZE);
	}
	//协议头、用户id、手机号、昵称、签名、状态、头像
	PackType type;
	int userid;
	int icon;
	int state;
	char tel[_DEF_MAX_SIZE];
	char name[_DEF_MAX_SIZE];
	char feeling[_DEF_MAX_SIZE];
}STRU_FRIEND_INFO;

//聊天
typedef struct STRU_TCP_CHAT_RQ
{
	STRU_TCP_CHAT_RQ() :type(_DEF_PACK_CHAT_RQ), userid(0), friendid(0)
	{
		memset(content, 0, _DEF_CONTENT_SIZE);
	}
	//协议头、用户id、好友id、聊天内容
	PackType type;
	int userid;
	int friendid;
	char content[_DEF_CONTENT_SIZE];
}STRU_TCP_CHAT_RQ;

typedef struct STRU_TCP_CHAT_RS
{
	STRU_TCP_CHAT_RS() :type(_DEF_PACK_CHAT_RS), userid(0), friendid(0), result(send_success)
	{
	}
	//协议头、发送方id、接收方id、发送请求结果
	PackType type;
	int userid;
	int friendid;
	int result;
}STRU_TCP_CHAT_RS;

//添加好友
typedef struct STRU_ADD_FRIEND_RQ
{
	STRU_ADD_FRIEND_RQ():type(_DEF_PACK_ADDFRIEND_RQ), userid(0)
	{
		memset(username, 0, _DEF_MAX_SIZE);
		memset(friendname, 0, _DEF_MAX_SIZE);
	}
	//协议头、用户1(本人)，用户1昵称，用户2(好友)昵称
	PackType type;
	int userid;
	char username[_DEF_MAX_SIZE];
	char friendname[_DEF_MAX_SIZE];
}STRU_ADD_FRIEND_RQ;

typedef struct STRU_ADD_FRIEND_RS
{
	STRU_ADD_FRIEND_RS() :type(_DEF_PACK_ADDFRIEND_RS), userid(0), friendid(0), result(add_success)
	{
		memset(friendname, 0, _DEF_MAX_SIZE);
	}
	//协议头、用户1，用户2id，用户2昵称、添加好友请求返回结果
	PackType type;
	int userid;
	int friendid;
	int result;
	char friendname[_DEF_MAX_SIZE];
}STRU_ADD_FRIEND_RS;

//离线
typedef struct STRU_TCP_OFFLINE_RQ
{
	STRU_TCP_OFFLINE_RQ():type(_DEF_PACK_OFFLINE_RQ), userid(0)
	{
	}
	//协议头、用户id
	PackType type;
	int userid;
}STRU_TCP_OFFLINE_RQ;