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

//��ʼ������
bool TcpClientNet::InitNet()
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

	//2.�����׽���	
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("socket function failed with error = %d\n", WSAGetLastError());
		UninitNet();
		return false;
	}

	//3.��������
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

	//4.�������ݡ������������������߳�
	//CreateThread ����������C++����ʱ�⺯�������˿ռ䣬���˳��߳�ʱ������տռ�
	//CreateThread��ExitThread(ϵͳ�Զ�����)��_beginthreadex��_endthreadex(ϵͳ�Զ�����)
	//_endthreadex�Ȼ���C++����ʱ�⺯�����ٵĿռ䣬�ٵ���ExitThread�˳��߳�
	m_handle = (HANDLE)_beginthreadex(NULL, 0, &RecvThread, this, 0, NULL);

	return true;
}

unsigned int __stdcall TcpClientNet::RecvThread(void* lpVoid)//��̬��Ա�����޷�������ͨ��Ա����
{
	TcpClientNet* pThis = (TcpClientNet*)lpVoid;
	pThis->RecvData();
	return 0;
}

//��������
bool TcpClientNet::SendData(long lSendIP, char* buf, int nLen)
{
	//1.���жϴ�������Ƿ�Ϸ�
	if (!buf || nLen <= 0) {
		return false;
	}

	//2.��ֹճ�����ȷ�����С���ٷ�������
	if (send(m_sock, (const char*)&nLen, sizeof(int), 0) <= 0) {
		return false;
	}
	if (send(m_sock, (const char*)buf, nLen, 0)<= 0) {
		return false;
	}

	return true;
}

//�ر�����
void TcpClientNet::UninitNet()
{
	//�˳��߳�
	m_isStop = true;
	if (m_handle) {
		if (WAIT_TIMEOUT == WaitForSingleObject(m_handle, 100)) {
			TerminateThread(m_handle, -1);
		}
		CloseHandle(m_handle);
		m_handle = NULL;
	}

	//�ر��׽���
	if (m_sock && m_sock != INVALID_SOCKET) {
		closesocket(m_sock);
	}
	//ж�ؿ�
	WSACleanup();
}

//��������
bool TcpClientNet::RecvData()
{
	int nPackSize = 0;
	int nRecvNum = 0;
	int offset = 0;

	while (!m_isStop) {
		//���հ���С�����հ�����
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