#pragma once
#include"INetMediator.h"

class TcpServerMediator : public INetMediator
{
public:
	TcpServerMediator();
	~TcpServerMediator();
	//打开网络
	bool OpenNet();
	//发送数据
	bool SendData(long lSendIP, char* buf, int nLen);
	//关闭网络
	void CloseNet();
	//处理数据
	bool DealData(long lSendIp, char* buf, int nLen);
protected:
	INet* m_pNet;
};