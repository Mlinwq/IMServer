#include"TcpServerMediator.h"
#include"TcpServerNet.h"
#include"../CKernel.h"

TcpServerMediator::TcpServerMediator()
{
	m_pNet = new TcpServerNet(this);
}
TcpServerMediator::~TcpServerMediator()
{
	if (m_pNet) {
		delete m_pNet;
		m_pNet = NULL;
	}
}
//������
bool TcpServerMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//��������
bool TcpServerMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//�ر�����
void TcpServerMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//��������
bool TcpServerMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// �������ݸ����Ĵ�����
	cout << buf << endl;
	CKernel::pKernel->DealData(lSendIp, buf, nLen);
	return true;
}
