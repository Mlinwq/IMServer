#include"UdpMediator.h"
#include"UdpNet.h"

UdpMediator::UdpMediator()
{
	m_pNet = new UdpNet(this);
}
UdpMediator::~UdpMediator()
{
	if (m_pNet) {
		delete m_pNet;
		m_pNet = NULL;
	}
}
//������
bool UdpMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//��������
bool UdpMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//�ر�����
void UdpMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//��������
bool UdpMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// �������ݸ����Ĵ�����

	cout << buf << endl;
	return true;
}
