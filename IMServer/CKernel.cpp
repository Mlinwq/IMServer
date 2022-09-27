#include "CKernel.h"
#include"TcpServerMediator.h"
#pragma warning(disable:4996)

#define NetProtocalMap(a) m_netProtocalMap[a- _DEF_PROTOCAL_BASE - 10]
//静态成员类外初始话
CKernel* CKernel::pKernel = NULL;

//初始化协议映射表
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

//开启服务
bool CKernel::startServer()
{
	//1.连接数据库
	if (!m_sql.ConnectMySql((char*)"127.0.0.1", (char*)"root", (char*)"localhost", (char*)"0716im")) {
		cout << "连接数据库失败" << endl;
		return false;
	}
	//2.开启网络
	m_pServer = new TcpServerMediator;
	if (!m_pServer->OpenNet()) {
		cout << "打开网络失败" << endl;
		return false;
	}
	return true;
}

//关闭服务
void CKernel::closeServer()
{
	//资源回收
	//1.断开与数据库连接
	m_sql.DisConnect();
	//2.关闭网络
	if (m_pServer) {
		m_pServer->CloseNet();
		delete m_pServer;
		m_pServer = NULL;
	}
}

//处理数据
void CKernel::DealData(long lSendIp, char* buf, int nLen)
{
	//1.取出协议头
	int nType = *(int*)buf;
	//2.判断协议头是否在映射范围内
	if (nType >= (_DEF_PROTOCAL_BASE + 10) &&
		nType <= (_DEF_PROTOCAL_BASE + 10 + _DEF_PROTOCAL_COUNT)) {
		//3.计算数组下标，获取对应函数
		PFUN pf = NetProtocalMap(nType);
		//4.如果指针存在，调用对应处理函数
		if (pf) {
			(this->*pf)(lSendIp, buf, nLen);
		}
	}
	//5.回收buf
	delete[] buf;
}

//处理注册请求
void CKernel::dealRegisterRq(long lSendIp, char* buf, int nLen)
{
	cout << "dealRegisterRq" << endl;
	//1.拆包
	STRU_REGISTER_RQ* rq = (STRU_REGISTER_RQ*)buf;

	//2.取出tel、password、name，判断合法性(未实现)，按照tel查表
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select tel from t_user where tel = %s;", rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.判断tel是否被注册
	STRU_REGISTER_RS rs;
	if (resultList.size() > 0) {
		//手机号已经注册
		rs.result = user_is_exist;
	}
	else {
		//4.如果没有，向数据库写入注册信息
		rs.result = register_sucess;
		sprintf(sqlBuf, "insert into t_user(tel,password,name,icon,feeling) values('%s','%s','%s',1,'......');",rq->tel,rq->password,rq->name);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
			return;
		}
	}

	//5.给客户端返回注册结果
	m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
}

//处理登录请求
void CKernel::dealLoginRq(long lSendIp, char* buf, int nLen)
{
	cout << "dealLoginRq" << endl;
	//1.拆包
	STRU_LOGIN_RQ* rq = (STRU_LOGIN_RQ*)buf;

	//2.取出tel、password，根据tel查看是否有这个用户
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select id, password from t_user where tel = %s;",rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 2, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.没有这个用户，返回登录失败
	STRU_LOGIN_RS rs;
	int nId = 0;
	string strPassword;
	if (resultList.size() == 0) {
		rs.result = user_not_exist;
	}
	else {
		//4.如果有这个用户，获取这个用户的id和密码
		nId = atoi(resultList.front().c_str());
		resultList.pop_front();
		strPassword = resultList.front();
		resultList.pop_front();

		//5.判断密码是否一致
		if (strcmp(strPassword.c_str(), rq->password)) {
			//6.密码不一致，返回登录失败
			rs.result = password_error;
			//给客户端回复登录结果
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
		}
		else {
			//7.密码一致，返回登录成功和用户id
			rs.result = login_success;
			rs.userid = nId;
			//给客户端回复登录结果
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
			
			//8.保存id和socket的映射关系
			m_mapIdToSock[nId] = lSendIp;

			//9.获取好友列表
			getUserList(nId);
		}
	}
}

//获取好友列表，包括自己
void CKernel::getUserList(int id)
{
	//1.先查询自己信息
	STRU_FRIEND_INFO loginInfo;
	getUserInfoById(id, &loginInfo);

	//2.用户的套接字，根据id从map中取出
	if (m_mapIdToSock.find(id) == m_mapIdToSock.end()) {
		cout << "user is not logged in" << endl;
		return;
	}
	SOCKET loginSock = m_mapIdToSock[id];

	//3.把自己的信息发给客户端
	m_pServer->SendData(loginSock, (char*)&loginInfo, sizeof(loginInfo));
	
	//4.根据用户id查找所有好友id
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select idB from t_friend where idA = %d;", id);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//5.遍历查找结果
	int friendId = 0;
	STRU_FRIEND_INFO friendInfo;
	while (resultList.size() > 0) {
		//6.取出好友id，根据好友id查找好友信息
		friendId = atoi(resultList.front().c_str());
		resultList.pop_front();
		getUserInfoById(friendId, &friendInfo);

		//7.将好友信息发给客户端
		m_pServer->SendData(loginSock, (char*)&friendInfo, sizeof(friendInfo));

		//8.如果好友不在线，则查询下一个
		if (m_mapIdToSock.find(friendId) == m_mapIdToSock.end()) {
			continue; 
		}
		//9.如果好友在线，取出好友套接字，给好友发送上线通知
		SOCKET friendSock = m_mapIdToSock[friendId];
		m_pServer->SendData(friendSock, (char*)&loginInfo, sizeof(loginInfo));
	}

}

//根据id查用户信息
void CKernel::getUserInfoById(int id, STRU_FRIEND_INFO* info)
{
	//1.保存id
	info->userid = id;
	//2.根据id查询用户信息
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

	//3.遍历map，查找用户在不在线
	if (m_mapIdToSock.find(id) != m_mapIdToSock.end()) {
		info->state = 1;
	}
	else {
		info->state = 0;
	}
}

//处理聊天请求
void CKernel:: dealChatRq(long lSendIp, char* buf, int nLen)
{
	//1.拆包
	STRU_TCP_CHAT_RQ* rq = (STRU_TCP_CHAT_RQ*)buf;
	//2.看接收方是否在线
	if (m_mapIdToSock.find(rq->friendid) != m_mapIdToSock.end()) {
		//2.1如果在线，获取对方套接字，转发数据
		SOCKET sock = m_mapIdToSock[rq->friendid];
		m_pServer->SendData(sock, buf, nLen);
	}
	else {
		//2.2不在线，回复客户端好友不在线
		STRU_TCP_CHAT_RS rs;
		rs.result = user_offline;
		rs.userid = rq->userid;
		rs.friendid = rq->friendid;
		m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
	}	
	
}

//处理添加好友请求
void CKernel::dealAddFriendRq(long lSendIp, char* buf, int nLen)
{
	//1.拆包
	STRU_ADD_FRIEND_RQ* rq = (STRU_ADD_FRIEND_RQ*)buf;

	//2.查看待添加好友是否存在
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select id from t_user where name = '%s';", rq->friendname);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}

	//3.如果好友存在，取出id
	if (resultList.size() > 0) {
		int friendid = atoi(resultList.front().c_str());
		resultList.pop_front();
		//4.看要添加好友是否在线
		if (m_mapIdToSock.find(friendid) != m_mapIdToSock.end()) {
			//4.1如果在线，取出socket，发送添加好友请求
			SOCKET sock = m_mapIdToSock[friendid];
			m_pServer->SendData(sock, buf, nLen);
		}
		else {
			//4.2如果不在线，回复客户端待添加好友不在线
			STRU_ADD_FRIEND_RS rs;
			rs.result = user_offline;
			rs.friendid = friendid;
			rs.userid = rq->userid;
			strcpy(rs.friendname, rq->friendname);
			m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
		}
	}
	else {
		//5.如果待添加好友不存在，回复客户端待添加好友不存在
		STRU_ADD_FRIEND_RS rs;
		rs.result = no_this_user;
		rs.friendid = 0;
		rs.userid = rq->userid;
		strcpy(rs.friendname, rq->friendname);
		m_pServer->SendData(lSendIp, (char*)&rs, sizeof(rs));
	}
}

//处理添加好友回复
void CKernel::dealAddFriendRs(long lSendIp, char* buf, int nLen)
{
	//1.拆包
	STRU_ADD_FRIEND_RS* rs = (STRU_ADD_FRIEND_RS*)buf;

	//2.判断对方是否同意
	if (rs->result == add_success) {
		//3.如果同意，向数据库中添加好友关系
		char sqlBuf[1024] = "";
		sprintf(sqlBuf, "insert into t_friend values(%d, %d);",rs->friendid, rs->userid);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
		}
		sprintf(sqlBuf, "insert into t_friend values(%d, %d);", rs->userid, rs->friendid);
		if (!m_sql.UpdateMySql(sqlBuf)) {
			cout << "UpdateMySql error:" << sqlBuf << endl;
		}

		//4.更新好友列表
		getUserList(rs->friendid);
	}
	
	//5.无论是否同意，均转发回复
	if (m_mapIdToSock.count(rs->userid) > 0) {
		SOCKET sock = m_mapIdToSock[rs->userid];
		m_pServer->SendData(sock, (char*)&rs, sizeof(rs));	
	}
}

//处理下线请求
void CKernel::dealOfflineRq(long lSendIp, char* buf, int nLen)
{
	//1.拆包
	STRU_TCP_OFFLINE_RQ* rq = (STRU_TCP_OFFLINE_RQ*)buf;

	//2.获取下线用户id
	int userid = rq->userid;
	 
	//3.根据下线用户id查询好友id
	list<string> resultList;
	char sqlBuf[1024] = "";
	sprintf(sqlBuf, "select idB from t_friend where idA = '%d';", userid);
	if (!m_sql.SelectMySql(sqlBuf, 1, resultList)) {
		cout << "SelectMySql error:" << sqlBuf << endl;
		return;
	}
	 
	//4.遍历查询结果
	int friendid = 0;
	while (resultList.size() > 0) {
		//5.给在线好友发送下线通知
		//5.1取出好友id
		friendid = atoi(resultList.front().c_str());
		resultList.pop_front();

		//5.2判断好友是否在线
		if (m_mapIdToSock.count(friendid) > 0) {
			//5.3发送下线通知
			SOCKET sock = m_mapIdToSock[friendid];
			m_pServer->SendData(sock, buf, nLen);
		}
	}

	//6.将下线用户的socket从map中移除
	if (m_mapIdToSock.count(userid) > 0) {
		m_mapIdToSock.erase(userid);
	}
}