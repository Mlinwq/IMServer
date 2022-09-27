#pragma once
#include"INet.h"
#include<list>
#include<map>

class TcpServerNet : public INet
{
public:
	TcpServerNet(INetMediator* pMediator);
	~TcpServerNet();
	//��ʼ������
	bool InitNet();
	//��������
	bool SendData(long lSendIP, char* buf, int nLen);
	//�ر�����
	void UninitNet();
protected:
	static unsigned int __stdcall RecvThread(void* lpVoid);//�̺߳�������̬��Ա�������Բ���������
	static unsigned int __stdcall AcceptThread(void* lpVoid);
	//��������
	bool RecvData();
	SOCKET m_sock;//���ڽ������ӵ�socket
	map<unsigned int, SOCKET> m_mapThreadIdToSocket;//����ͻ��˺Ͷ�Ӧsocket��ӳ���ϵ
	list<HANDLE> m_hThreadHandleList;//�����߳̾��
	bool m_isStop;
};