#include"UdpNet.h"
#include"PackDef.h"
#include"UdpMediator.h"
#include<process.h>


UdpNet::UdpNet(INetMediator* pMediator):m_sock(INVALID_SOCKET),m_handle(0),m_isStop(false)
{
	m_pMediator = pMediator;
}

UdpNet::~UdpNet()
{
	UninitNet();
}

//初始化网络
bool UdpNet::InitNet()
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
	
	m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sock == INVALID_SOCKET) {
		printf("socket function failed with error = %d\n", WSAGetLastError());
	}

	//有限广播需要申请广播权限
	BOOL bval = TRUE;
	setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, (char*)&bval, sizeof(bval));
	
	//3.绑定IP
	sockaddr_in  addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(_DEF_TCP_PORT);
	addrServer.sin_addr.s_addr = INADDR_ANY;
	err = bind(m_sock, (SOCKADDR*)&addrServer, sizeof(addrServer));
	if (SOCKET_ERROR == err)
	{
		printf("bind failed with error %u\n", WSAGetLastError());
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

unsigned int __stdcall UdpNet::RecvThread(void* lpVoid)//静态成员函数无法调用普通成员函数
{
	UdpNet* pThis = (UdpNet*)lpVoid;
	pThis->RecvData();
	return 0;
}

//发送数据
bool UdpNet::SendData(long lSendIP, char* buf, int nLen)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_DEF_UDP_PORT);
	addr.sin_addr.S_un.S_addr = lSendIP;
	if (!sendto(m_sock, buf, nLen, 0, (const sockaddr*)&addr, sizeof(addr))) {
		cout << "sendto error:" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}

//关闭网络
void UdpNet::UninitNet()
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
bool UdpNet::RecvData()
{
	char recvBuf[_DEF_BUFFER_SIZE] = "";
	sockaddr_in addrClient;
	int nSize = sizeof(addrClient);
	int nRecvNum = 0;
	m_isStop = false;
	while (!m_isStop) {
		nRecvNum = recvfrom(m_sock, recvBuf, _DEF_BUFFER_SIZE, 0, (sockaddr*)&addrClient, &nSize);
		if (nRecvNum > 0) {
			char* pack = new char[nRecvNum];
			if (pack) {
				memcpy(pack, recvBuf, nRecvNum);
				m_pMediator->DealData(m_sock, recvBuf, nRecvNum);
			}	
		}
		else {
			cout << "recvfrom error:" << GetLastError << endl;
		}
	}
	return true;
}