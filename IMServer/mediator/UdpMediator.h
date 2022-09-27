#pragma once
#include"INetMediator.h"

class UdpMediator : public INetMediator
{
public:
	UdpMediator();
	~UdpMediator();
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