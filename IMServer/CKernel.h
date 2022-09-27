#pragma once
#include"CMySql.h"
#include"INetMediator.h"
#include"PackDef.h"
#include<iostream>
#include<map>

using namespace std;

class CKernel;
//����ָ��
typedef void (CKernel::* PFUN)(long lSendIp, char* buf, int nLen);

class CKernel
{
public:
	CKernel();
	~CKernel();
	//��������
	bool startServer();
	//�رշ���
	void closeServer();
	//��������
	void DealData(long lSendIp, char* buf, int nLen);

private:
	//��ʼ��Э��ӳ���
	void setProtocalMap();
	//��ȡ�����б������Լ�
	void getUserList(int id);
	//����id���û���Ϣ
	void getUserInfoById(int id,STRU_FRIEND_INFO* info);
	//����ע������
	void dealRegisterRq(long lSendIp, char* buf, int nLen);
	//�����¼����
	void dealLoginRq(long lSendIp, char* buf, int nLen);
	//������������
	void dealChatRq(long lSendIp, char* buf, int nLen);
	//������Ӻ�������
	void dealAddFriendRq(long lSendIp, char* buf, int nLen);
	//������Ӻ��ѻظ�
	void dealAddFriendRs(long lSendIp, char* buf, int nLen);
	//������������
	void dealOfflineRq(long lSendIp, char* buf, int nLen);

private:
	CMySql m_sql;
	INetMediator* m_pServer;

public:
	static CKernel* pKernel;
	//Э��ӳ���(����ָ������)[Э��ͷ-base] -> ������
	PFUN m_netProtocalMap[_DEF_PROTOCAL_COUNT];
	//����id��socket��ӳ���ϵ
	map<int, int> m_mapIdToSock;
};

