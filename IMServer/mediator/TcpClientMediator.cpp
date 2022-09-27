#include"TcpClientMediator.h"
#include"TcpClientNet.h"

TcpClientMediator::TcpClientMediator()
{
	m_pNet = new TcpClientNet(this);
}
TcpClientMediator::~TcpClientMediator()
{
	if (m_pNet) {
		delete m_pNet;
		m_pNet = NULL;
	}
}
//������
bool TcpClientMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//��������
bool TcpClientMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//�ر�����
void TcpClientMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//��������
bool TcpClientMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// �������ݸ����Ĵ�����
	cout << buf << endl;
	return true;
}
