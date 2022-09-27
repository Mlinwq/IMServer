#pragma once
#include"INetMediator.h"

class TcpClientMediator : public INetMediator
{
public:
	TcpClientMediator();
	~TcpClientMediator();
	//������
	bool OpenNet();
	//��������
	bool SendData(long lSendIP, char* buf, int nLen);
	//�ر�����
	void CloseNet();
	//��������
	bool DealData(long lSendIp, char* buf, int nLen);
protected:
	INet* m_pNet;
};