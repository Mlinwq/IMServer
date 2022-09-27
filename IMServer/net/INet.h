#pragma once
#include<iostream>
using namespace std;
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

class INetMediator;//����һ��INetMediator���𣬲���Ҫ����ͷ�ļ�
class INet
{
public:
	INet() {};
	virtual ~INet() {};
	//��ʼ������
	virtual bool InitNet() = 0;
	//��������
	virtual bool SendData(long lSendIP, char* buf, int nLen) = 0;
	//�ر�����
	virtual void UninitNet() = 0;	
protected:
	//��������
	virtual bool RecvData() = 0;
	INetMediator* m_pMediator;
};