#include "CKernel.h"
#include"TcpServerMediator.h"
#pragma warning(disable:4996)

#define NetProtocalMap(a) m_netProtocalMap[a- _DEF_PROTOCAL_BASE - 10]
//��̬��Ա�����ʼ��
CKernel* CKernel::pKernel = NULL;

//��ʼ��Э��ӳ���
void CKernel::setProtocalMap()
{
	memset(m_netProtocalMap, 0, _DEF_PROTOCAL_COUNT);
	NetProtocalMap(_DEF_PACK_REGISTER_RQ) = &CKernel::dealRegisterRq;
	NetProtocalMap(_DEF_PACK_LOGIN_RQ) = &CKernel::dealLoginRq;
	NetProtocalMap(_DEF_PACK_CHAT_RQ) = &CKernel::dealChatRq;
	NetProtocalMap(_DEF_PACK_ADDFRIEND_RQ) = &CKernel::dealAddFriendRq;
	NetProtocalMap(_DEF_PACK_ADDFRIEND_RS) = &CKernel::dealAddFriendRs;
	NetProtocalMap(_DEF_PACK_OFFLINE_RQ) = &CKernel::dealOfflineRq;
}

CKernel::CKernel(): m_pServer(NULL)
{
	pKernel = this;
	setProtocalMap();
}

CKernel::~CKernel()
{
	closeServer();
}

//��������
bool CKernel::startServer()
{
	//1.�������ݿ�
	if (!m_sql.ConnectMySql((char*)"127.0.0.1", (char*)"root", (char*)"localhost", (char*)"0716im")) {
		cout << "�������ݿ�ʧ��" << endl;
		return false;
	}
	//2.��������
	m_pServer = new TcpServerMediator;
	if (!m_pServer->OpenNet()) {
		cout << "������ʧ��" << endl;
		return false;
	}
	return true;
}

//�رշ���
void CKernel::closeServer()
{
	//��Դ����
	//1.�Ͽ������ݿ�����
	m_sql.DisConnect();
	//2.�ر�����
	if (m_pServer) {
		m_pServer->CloseNet();
		delete m_pServer;
		m_pServer = NULL;
	}
}

//��������
void CKernel::DealData(long lSendIp, char* buf, int nLen)
{
	//1.ȡ��Э��ͷ
	int nType = *(int*)buf;
	//2.�ж�Э��ͷ�Ƿ���ӳ�䷶Χ��
	if (nType >= (_DEF_PROTOCAL_BASE + 10) &&
		nType <= (_DEF_PROTOCAL_BASE + 10 + _DEF_PROTOCAL_COUNT)) {
		//3.���������±꣬��ȡ��Ӧ����
		PFUN pf = NetProtocalMap(nType);
		//4.���ָ����ڣ����ö�Ӧ������
		if (pf) {
			(this->*pf)(lSendIp, buf, nLen);
		}
	}
	//5.����buf
	delete[] buf;
}

//����ע������
void CKernel::dealRegisterRq(long lSendIp, char* buf, int nLen)
{
	cout << "dealRegisterRq" << endl;
	//1.���
	STRU_REGISTER_RQ* rq = (STRU_REGISTER_RQ*)buf;

	//2.ȡ��tel��password��name���жϺϷ���(δʵ��)������tel���
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select tel from t_user where tel = %s;", rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.�ж�tel�Ƿ�ע��
	STRU_REGISTER_RS rs;
	if (resultList.size() > 0) {
		//�ֻ����Ѿ�ע��
		rs.result = user_is_exist;
	}
	else {
		//4.���û�У������ݿ�д��ע����Ϣ
		rs.result = register_sucess;
		sprintf(sqlBuf, "insert into t_user(tel,password,name,icon,feeling) values('%s','%s','%s',1,'......');",rq->tel,rq->password,rq->name);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
			return;
		}
	}

	//5.���ͻ��˷���ע����
	m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
}

//�����¼����
void CKernel::dealLoginRq(long lSendIp, char* buf, int nLen)
{
	cout << "dealLoginRq" << endl;
	//1.���
	STRU_LOGIN_RQ* rq = (STRU_LOGIN_RQ*)buf;

	//2.ȡ��tel��password������tel�鿴�Ƿ�������û�
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select id, password from t_user where tel = %s;",rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 2, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.û������û������ص�¼ʧ��
	STRU_LOGIN_RS rs;
	int nId = 0;
	string strPassword;
	if (resultList.size() == 0) {
		rs.result = user_not_exist;
	}
	else {
		//4.���������û�����ȡ����û���id������
		nId = atoi(resultList.front().c_str());
		resultList.pop_front();
		strPassword = resultList.front();
		resultList.pop_front();

		//5.�ж������Ƿ�һ��
		if (strcmp(strPassword.c_str(), rq->password)) {
			//6.���벻һ�£����ص�¼ʧ��
			rs.result = password_error;
			//���ͻ��˻ظ���¼���
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
		}
		else {
			//7.����һ�£����ص�¼�ɹ����û�id
			rs.result = login_success;
			rs.userid = nId;
			//���ͻ��˻ظ���¼���
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
			
			//8.����id��socket��ӳ���ϵ
			m_mapIdToSock[nId] = lSendIp;

			//9.��ȡ�����б�
			getUserList(nId);
		}
	}
}

//��ȡ�����б������Լ�
void CKernel::getUserList(int id)
{
	//1.�Ȳ�ѯ�Լ���Ϣ
	STRU_FRIEND_INFO loginInfo;
	getUserInfoById(id, &loginInfo);

	//2.�û����׽��֣�����id��map��ȡ��
	if (m_mapIdToSock.find(id) == m_mapIdToSock.end()) {
		cout << "user is not logged in" << endl;
		return;
	}
	SOCKET loginSock = m_mapIdToSock[id];

	//3.���Լ�����Ϣ�����ͻ���
	m_pServer->SendData(loginSock, (char*)&loginInfo, sizeof(loginInfo));
	
	//4.�����û�id�������к���id
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select idB from t_friend where idA = %d;", id);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//5.�������ҽ��
	int friendId = 0;
	STRU_FRIEND_INFO friendInfo;
	while (resultList.size() > 0) {
		//6.ȡ������id�����ݺ���id���Һ�����Ϣ
		friendId = atoi(resultList.front().c_str());
		resultList.pop_front();
		getUserInfoById(friendId, &friendInfo);

		//7.��������Ϣ�����ͻ���
		m_pServer->SendData(loginSock, (char*)&friendInfo, sizeof(friendInfo));

		//8.������Ѳ����ߣ����ѯ��һ��
		if (m_mapIdToSock.find(friendId) == m_mapIdToSock.end()) {
			continue; 
		}
		//9.����������ߣ�ȡ�������׽��֣������ѷ�������֪ͨ
		SOCKET friendSock = m_mapIdToSock[friendId];
		m_pServer->SendData(friendSock, (char*)&loginInfo, sizeof(loginInfo));
	}

}

//����id���û���Ϣ
void CKernel::getUserInfoById(int id, STRU_FRIEND_INFO* info)
{
	//1.����id
	info->userid = id;
	//2.����id��ѯ�û���Ϣ
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select name,icon,feeling from t_user where id = %d;", id);
	if (!m_sql.SelectMySql(sqlBuf, 3, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}
	
	if (resultList.size() == 3) {
		strcpy(info->name,resultList.front().c_str());
		resultList.pop_front();

		info->icon = atoi(resultList.front().c_str());
		resultList.pop_front();

		strcpy(info->feeling, resultList.front().c_str());
		resultList.pop_front();
	}

	//3.����map�������û��ڲ�����
	if (m_mapIdToSock.find(id) != m_mapIdToSock.end()) {
		info->state = 1;
	}
	else {
		info->state = 0;
	}
}

//������������
void CKernel:: dealChatRq(long lSendIp, char* buf, int nLen)
{
	//1.���
	STRU_TCP_CHAT_RQ* rq = (STRU_TCP_CHAT_RQ*)buf;
	//2.�����շ��Ƿ�����
	if (m_mapIdToSock.find(rq->friendid) != m_mapIdToSock.end()) {
		//2.1������ߣ���ȡ�Է��׽��֣�ת������
		SOCKET sock = m_mapIdToSock[rq->friendid];
		m_pServer->SendData(sock, buf, nLen);
	}
	else {
		//2.2�����ߣ��ظ��ͻ��˺��Ѳ�����
		STRU_TCP_CHAT_RS rs;
		rs.result = user_offline;
		rs.userid = rq->userid;
		rs.friendid = rq->friendid;
		m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
	}	
	
}

//������Ӻ�������
void CKernel::dealAddFriendRq(long lSendIp, char* buf, int nLen)
{
	//1.���
	STRU_ADD_FRIEND_RQ* rq = (STRU_ADD_FRIEND_RQ*)buf;

	//2.�鿴����Ӻ����Ƿ����
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select id from t_user where name = '%s';", rq->friendname);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.������Ѵ��ڣ�ȡ��id
	if (resultList.size() > 0) {
		int friendid = atoi(resultList.front().c_str());
		resultList.pop_front();
		//4.��Ҫ��Ӻ����Ƿ�����
		if (m_mapIdToSock.find(friendid) != m_mapIdToSock.end()) {
			//4.1������ߣ�ȡ��socket��������Ӻ�������
			SOCKET sock = m_mapIdToSock[friendid];
			m_pServer->SendData(sock, buf, nLen);
		}
		else {
			//4.2��������ߣ��ظ��ͻ��˴���Ӻ��Ѳ�����
			STRU_ADD_FRIEND_RS rs;
			rs.result = user_offline;
			rs.friendid = friendid;
			rs.userid = rq->userid;
			strcpy(rs.friendname, rq->friendname);
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
		}
	}
	else {
		//5.�������Ӻ��Ѳ����ڣ��ظ��ͻ��˴���Ӻ��Ѳ�����
		STRU_ADD_FRIEND_RS rs;
		rs.result = no_this_user;
		rs.friendid = 0;
		rs.userid = rq->userid;
		strcpy(rs.friendname, rq->friendname);
		m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
	}
}

//������Ӻ��ѻظ�
void CKernel::dealAddFriendRs(long lSendIp, char* buf, int nLen)
{
	//1.���
	STRU_ADD_FRIEND_RS* rs = (STRU_ADD_FRIEND_RS*)buf;

	//2.�ж϶Է��Ƿ�ͬ��
	if (rs->result == add_success) {
		//3.���ͬ�⣬�����ݿ�����Ӻ��ѹ�ϵ
		char sqlBuf[1024] = "";
		sprintf(sqlBuf, "insert into t_friend values(%d, %d);",rs->friendid, rs->userid);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
		}
		sprintf(sqlBuf, "insert into t_friend values(%d, %d);", rs->userid, rs->friendid);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
		}

		//4.���º����б�
		getUserList(rs->friendid);
	}
	
	//5.�����Ƿ�ͬ�⣬��ת���ظ�
	if (m_mapIdToSock.count(rs->userid) > 0) {
		SOCKET sock = m_mapIdToSock[rs->userid];
		m_pServer->SendData(sock, (char*)&rs, sizeof(rs));	
	}
}

//������������
void CKernel::dealOfflineRq(long lSendIp, char* buf, int nLen)
{
	//1.���
	STRU_TCP_OFFLINE_RQ* rq = (STRU_TCP_OFFLINE_RQ*)buf;

	//2.��ȡ�����û�id
	int userid = rq->userid;
	 
	//3.���������û�id��ѯ����id
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select idB from t_friend where idA = '%d';", userid);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}
	 
	//4.������ѯ���
	int friendid = 0;
	while (resultList.size() > 0) {
		//5.�����ߺ��ѷ�������֪ͨ
		//5.1ȡ������id
		friendid = atoi(resultList.front().c_str());
		resultList.pop_front();

		//5.2�жϺ����Ƿ�����
		if (m_mapIdToSock.count(friendid) > 0) {
			//5.3��������֪ͨ
			SOCKET sock = m_mapIdToSock[friendid];
			m_pServer->SendData(sock, buf, nLen);
		}
	}

	//6.�������û���socket��map���Ƴ�
	if (m_mapIdToSock.count(userid) > 0) {
		m_mapIdToSock.erase(userid);
	}
}