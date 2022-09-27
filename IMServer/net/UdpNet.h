#pragma once
#include"INet.h"

class UdpNet : public INet
{
public:
	UdpNet(INetMediator* pMediator);
	~UdpNet();
	//初始化网络
	bool InitNet();
	//发送数据
	bool SendData(long lSendIP, char* buf, int nLen);
	//关闭网络
	void UninitNet();
protected:
	static unsigned int __stdcall RecvThread(void* lpVoid);//线程函数，静态成员函数可以不依赖对象
	//接收数据
	bool RecvData();
	HANDLE m_handle;
	SOCKET m_sock;
	bool m_isStop;
};