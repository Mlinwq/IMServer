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
//打开网络
bool TcpClientMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//发送数据
bool TcpClientMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//关闭网络
void TcpClientMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//处理数据
bool TcpClientMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// 发送数据给核心处理类
	cout << buf << endl;
	return true;
}
