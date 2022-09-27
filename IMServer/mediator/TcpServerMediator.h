#pragma once
#include"INetMediator.h"

class TcpServerMediator : public INetMediator
{
public:
	TcpServerMediator();
	~TcpServerMediator();
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