#pragma once
#include<iostream>
using namespace std;
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

class INetMediator;//声明一个INetMediator类吗，不需要包含头文件
class INet
{
public:
	INet() {};
	virtual ~INet() {};
	//初始化网络
	virtual bool InitNet() = 0;
	//发送数据
	virtual bool SendData(long lSendIP, char* buf, int nLen) = 0;
	//关闭网络
	virtual void UninitNet() = 0;	
protected:
	//接收数据
	virtual bool RecvData() = 0;
	INetMediator* m_pMediator;
};