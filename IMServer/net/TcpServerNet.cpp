#include"TcpServerNet.h"
#include"PackDef.h"
#include"TcpServerMediator.h"
#include<process.h>
#pragma warning(disable:4996)

TcpServerNet::TcpServerNet(INetMediator* pMediator) :m_sock(INVALID_SOCKET), m_isStop(false)
{
	m_pMediator = pMediator;
}

TcpServerNet::~TcpServerNet()
{
	UninitNet();
}

//初始化网络
bool TcpServerNet::InitNet()
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

	////2.创建套接字	

	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("socket function failed with error = %d\n", WSAGetLastError());
		UninitNet();
		return false;
	}

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

	//4.监听
	err = listen(m_sock, 10);
	if (err == SOCKET_ERROR)
	{
		printf("listen failed with error %u\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//5.接受连接
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &AcceptThread, this, 0, NULL);
	if (handle) {
		m_hThreadHandleList.push_back(handle);
	}

	return true;
}

unsigned int __stdcall TcpServerNet::AcceptThread(void* lpVoid)//静态成员函数无法调用普通成员函数
{
	TcpServerNet* pThis = (TcpServerNet*)lpVoid;
	sockaddr_in addrClient;
	int nSize = sizeof(addrClient);
	while (!pThis->m_isStop) {
		SOCKET sock = accept(pThis->m_sock, (sockaddr*)&addrClient, &nSize);
		cout << "ip:" << inet_ntoa(addrClient.sin_addr) << endl;
		//给每个客户端创建一个对应的接收数据的线程
		unsigned int nThreadId = 0;
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, pThis, 0, &nThreadId);
		if (handle) {
			pThis->m_hThreadHandleList.push_back(handle);
			pThis->m_mapThreadIdToSocket[nThreadId] = sock;
		}
	}

	return 0;
}

unsigned int __stdcall TcpServerNet::RecvThread(void* lpVoid)//静态成员函数无法调用普通成员函数
{
	TcpServerNet* pThis = (TcpServerNet*)lpVoid;
	pThis->RecvData();
	return 0;
}

//发送数据
bool TcpServerNet::SendData(long lSendIP, char* buf, int nLen)
{
	//lSendIP里面装的是socket
	//1.先判断传入参数是否合法
	if (!buf || nLen <= 0) {
		return false;
	}

	//2.防止粘包，先发包大小，再发包内容
	if (send(lSendIP, (const char*)&nLen, sizeof(int), 0) <= 0) {
		cout << "send error:" << WSAGetLastError() << endl;
		return false;
	}
	if (send(lSendIP, (const char*)buf, nLen, 0) <= 0) {
		cout << "send error:" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}

//关闭网络
void TcpServerNet::UninitNet()
{
	//退出线程
	m_isStop = true;
	for (auto ite = m_hThreadHandleList.begin(); ite != m_hThreadHandleList.end();) {
		if (*ite) {
			if (WAIT_TIMEOUT == WaitForSingleObject(*ite, 100)) {
				TerminateThread(*ite, -1);
			}
			CloseHandle(*ite);
			*ite = NULL;
		}
		m_hThreadHandleList.erase(ite);
	}


	//关闭套接字
	if (m_sock && m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
	}
	for (auto ite = m_mapThreadIdToSocket.begin(); ite != m_mapThreadIdToSocket.end();) {
		if (ite->second && ite->second != INVALID_SOCKET) {
			closesocket(ite->second);
		}
		m_mapThreadIdToSocket.erase(ite);
	}
	//卸载库
	WSACleanup();
}

//接收数据
bool TcpServerNet::RecvData()
{
	//1.获取与客户端通信的socket
	Sleep(100);//线程创建以后立即执行，但是线程id和socket存入map中需要一定时间，因此需要先休眠一段时间
	SOCKET sock = m_mapThreadIdToSocket[GetCurrentThreadId()];

	//2.判断socket的合法性
	if (!sock || sock == INVALID_SOCKET) {
		return false;
	}

	//3.接收数据
	int nPackSize = 0;//包大小
	int nRecvNum = 0;//实际接收大小
	int offset = 0;//偏移量

	while (!m_isStop) {
		//先收包大小，再收包内容
		nRecvNum = recv(sock, (char*)&nPackSize, sizeof(int), 0);
		if (nRecvNum <= 0) {
			break;
		}
		char* recvBuf = new char[nPackSize];
		while (nPackSize) {
			nRecvNum = recv(sock, recvBuf + offset, nPackSize, 0);
			if (nRecvNum > 0) {
				offset += nRecvNum;
				nPackSize -= nRecvNum;
			}
		}
		this->m_pMediator->DealData(sock, recvBuf, offset);
		offset = 0;
	}


	return true;
}