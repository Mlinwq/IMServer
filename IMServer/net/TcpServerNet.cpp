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

//��ʼ������
bool TcpServerNet::InitNet()
{
	//1.���� 
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

	////2.�����׽���	

	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("socket function failed with error = %d\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//3.��IP
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

	//4.����
	err = listen(m_sock, 10);
	if (err == SOCKET_ERROR)
	{
		printf("listen failed with error %u\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//5.��������
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &AcceptThread, this, 0, NULL);
	if (handle) {
		m_hThreadHandleList.push_back(handle);
	}

	return true;
}

unsigned int __stdcall TcpServerNet::AcceptThread(void* lpVoid)//��̬��Ա�����޷�������ͨ��Ա����
{
	TcpServerNet* pThis = (TcpServerNet*)lpVoid;
	sockaddr_in addrClient;
	int nSize = sizeof(addrClient);
	while (!pThis->m_isStop) {
		SOCKET sock = accept(pThis->m_sock, (sockaddr*)&addrClient, &nSize);
		cout << "ip:" << inet_ntoa(addrClient.sin_addr) << endl;
		//��ÿ���ͻ��˴���һ����Ӧ�Ľ������ݵ��߳�
		unsigned int nThreadId = 0;
		HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, pThis, 0, &nThreadId);
		if (handle) {
			pThis->m_hThreadHandleList.push_back(handle);
			pThis->m_mapThreadIdToSocket[nThreadId] = sock;
		}
	}

	return 0;
}

unsigned int __stdcall TcpServerNet::RecvThread(void* lpVoid)//��̬��Ա�����޷�������ͨ��Ա����
{
	TcpServerNet* pThis = (TcpServerNet*)lpVoid;
	pThis->RecvData();
	return 0;
}

//��������
bool TcpServerNet::SendData(long lSendIP, char* buf, int nLen)
{
	//lSendIP����װ����socket
	//1.���жϴ�������Ƿ�Ϸ�
	if (!buf || nLen <= 0) {
		return false;
	}

	//2.��ֹճ�����ȷ�����С���ٷ�������
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

//�ر�����
void TcpServerNet::UninitNet()
{
	//�˳��߳�
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


	//�ر��׽���
	if (m_sock && m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
	}
	for (auto ite = m_mapThreadIdToSocket.begin(); ite != m_mapThreadIdToSocket.end();) {
		if (ite->second && ite->second != INVALID_SOCKET) {
			closesocket(ite->second);
		}
		m_mapThreadIdToSocket.erase(ite);
	}
	//ж�ؿ�
	WSACleanup();
}

//��������
bool TcpServerNet::RecvData()
{
	//1.��ȡ��ͻ���ͨ�ŵ�socket
	Sleep(100);//�̴߳����Ժ�����ִ�У������߳�id��socket����map����Ҫһ��ʱ�䣬�����Ҫ������һ��ʱ��
	SOCKET sock = m_mapThreadIdToSocket[GetCurrentThreadId()];

	//2.�ж�socket�ĺϷ���
	if (!sock || sock == INVALID_SOCKET) {
		return false;
	}

	//3.��������
	int nPackSize = 0;//����С
	int nRecvNum = 0;//ʵ�ʽ��մ�С
	int offset = 0;//ƫ����

	while (!m_isStop) {
		//���հ���С�����հ�����
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