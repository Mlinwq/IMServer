#pragma once
#include"INet.h"

class UdpNet : public INet
{
public:
	UdpNet(INetMediator* pMediator);
	~UdpNet();
	//��ʼ������
	bool InitNet();
	//��������
	bool SendData(long lSendIP, char* buf, int nLen);
	//�ر�����
	void UninitNet();
protected:
	static unsigned int __stdcall RecvThread(void* lpVoid);//�̺߳�������̬��Ա�������Բ���������
	//��������
	bool RecvData();
	HANDLE m_handle;
	SOCKET m_sock;
	bool m_isStop;
};