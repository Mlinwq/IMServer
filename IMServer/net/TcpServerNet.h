#pragma once
#include"INet.h"
#include<list>
#include<map>

class TcpServerNet : public INet
{
public:
	TcpServerNet(INetMediator* pMediator);
	~TcpServerNet();
	//初始化网络
	bool InitNet();
	//发送数据
	bool SendData(long lSendIP, char* buf, int nLen);
	//关闭网络
	void UninitNet();
protected:
	static unsigned int __stdcall RecvThread(void* lpVoid);//线程函数，静态成员函数可以不依赖对象
	static unsigned int __stdcall AcceptThread(void* lpVoid);
	//接收数据
	bool RecvData();
	SOCKET m_sock;//用于接收连接的socket
	map<unsigned int, SOCKET> m_mapThreadIdToSocket;//保存客户端和对应socket的映射关系
	list<HANDLE> m_hThreadHandleList;//保存线程句柄
	bool m_isStop;
};