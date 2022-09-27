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
//打开网络
bool TcpServerMediator::OpenNet()
{
	if (!m_pNet->InitNet()) {
		return false;
	}
	return true;
}
//发送数据
bool TcpServerMediator::SendData(long lSendIP, char* buf, int nLen)
{
	if (!m_pNet->SendData(lSendIP, buf, nLen)) {
		return false;
	}
	return true;
}
//关闭网络
void TcpServerMediator::CloseNet()
{
	m_pNet->UninitNet();
}
//处理数据
bool TcpServerMediator::DealData(long lSendIp, char* buf, int nLen)
{
	// 发送数据给核心处理类
	cout << buf << endl;
	CKernel::pKernel->DealData(lSendIp, buf, nLen);
	return true;
}
