#include"TcpClientNet.h"
#include"PackDef.h"
#include"TcpClientMediator.h"
#include<process.h>
#pragma warning(disable:4996)

TcpClientNet::TcpClientNet(INetMediator* pMediator) :m_sock(INVALID_SOCKET), m_handle(0), m_isStop(false)
{
	m_pMediator = pMediator;
}

TcpClientNet::~TcpClientNet()
{
	UninitNet();
}

//初始化网络
bool TcpClientNet::InitNet()
{
	//1.加载 
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return false;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		system("pause");
		return false;
	}

	/* The Winsock DLL is acceptable. Proceed to use it. */

	/* Add network programming using Winsock here */

	/* then call WSACleanup when done using the Winsock dll */

	//2.创建套接字	
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("socket function failed with error = %d\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//3.建立连接
	sockaddr_in  addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(_DEF_TCP_PORT);
	addrServer.sin_addr.s_addr = inet_addr(_DEF_SERVER_IP);
	err = connect(m_sock, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (SOCKET_ERROR == err)
	{
		printf("connect failed with error %u\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//4.接收数据——阻塞函数，创建线程
	//CreateThread 当函数中有C++运行时库函数开辟了空间，在退出线程时不会回收空间
	//CreateThread和ExitThread(系统自动调用)与_beginthreadex和_endthreadex(系统自动调用)
	//_endthreadex先回收C++运行时库函数开辟的空间，再调用ExitThread退出线程
	m_handle = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, this, 0, NULL);

	return true;
}

unsigned int __stdcall TcpClientNet::RecvThread(void* lpVoid)//静态成员函数无法调用普通成员函数
{
	TcpClientNet* pThis = (TcpClientNet*)lpVoid;
	pThis->RecvData();
	return 0;
}

//发送数据
bool TcpClientNet::SendData(long lSendIP, char* buf, int nLen)
{
	//1.先判断传入参数是否合法
	if (!buf || nLen <= 0) {
		return false;
	}

	//2.防止粘包，先发包大小，再发包内容
	if (send(m_sock, (const char*)&nLen, sizeof(int), 0) <= 0) {
		return false;
	}
	if (send(m_sock, (const char*)buf, nLen, 0)<= 0) {
		return false;
	}

	return true;
}

//关闭网络
void TcpClientNet::UninitNet()
{
	//退出线程
	m_isStop = true;
	if (m_handle) {
		if (WAIT_TIMEOUT == WaitForSingleObject(m_handle, 100)) {
			TerminateThread(m_handle, -1);
		}
		CloseHandle(m_handle);
		m_handle = NULL;
	}

	//关闭套接字
	if (m_sock && m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
	}
	//卸载库
	WSACleanup();
}

//接收数据
bool TcpClientNet::RecvData()
{
	int nPackSize = 0;
	int nRecvNum = 0;
	int offset = 0;

	while (!m_isStop) {
		//先收包大小，再收包内容
		nRecvNum = recv(m_sock, (char*)&nPackSize, sizeof(int), 0);
		if (nRecvNum <= 0) {
			break;
		}
		char* recvBuf = new char[nPackSize];
		while (nPackSize) {
			nRecvNum = recv(m_sock, recvBuf + offset, nPackSize, 0);
			if (nRecvNum > 0) {
				offset += nRecvNum;
				nPackSize -= nRecvNum;
			}
		}
	
		this->m_pMediator->DealData(m_sock, recvBuf, offset);	
		offset = 0;
	}

	return true;
}