#pragma once

class INet;
class INetMediator
{
public:
	INetMediator() {};
	virtual ~INetMediator() {};
	//打开网络
	virtual bool OpenNet() = 0;
	//发送数据
	virtual bool SendData(long lSendIP, char* buf, int nLen) = 0;
	//关闭网络
	virtual void CloseNet() = 0;	
	//处理数据	
	virtual bool DealData(long lSendIp, char* buf, int nLen) = 0;
protected:
	INet* m_pNet;
};