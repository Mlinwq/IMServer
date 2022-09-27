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
//打开网络
bool UdpMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//发送数据
bool UdpMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//关闭网络
void UdpMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//处理数据
bool UdpMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// 发送数据给核心处理类

	cout << buf << endl;
	return true;
}
