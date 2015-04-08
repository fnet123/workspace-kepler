#include "msgclient.h"
#include <tools.h>
#include <netutil.h>
#include <intercoder.h>

MsgClient::MsgClient(void) :
		_session(true)
{
	_pMsgClient = NULL;
	_last_handle_user_time = time(NULL);
}

MsgClient::~MsgClient(void)
{
	Stop();
}

// ����û�����
void MsgClient::AddUser(const char *ip, unsigned short port, const char *username, const char *pwd)
{
	char szid[256] = {0};
	sprintf(szid, "%llu", netutil::strToAddr(ip, port));

	User user = _online_user.GetUserByUserId(szid);
	if (!user._user_id.empty())
		return;

	user._user_id = szid;
	user._ip = ip;
	user._port = port;
	user._user_name = username;
	user._user_pwd = pwd;
	user._user_type = "SAVE";
	user._user_state = User::OFF_LINE;
	user._socket_type = User::TcpConnClient;
	user._connect_info.keep_alive = AlwaysReConn;
	user._connect_info.timeval = 30;

	// ��ӵ��û�������
	_online_user.AddUser(user._user_id, user);
}

// ����ɾ������
void MsgClient::DelUser(const char *ip, unsigned short port)
{
	char szid[256] = {0};
	sprintf(szid, "%llu", netutil::strToAddr(ip, port));

	User user = _online_user.GetUserByUserId(szid);
	if (user._user_id.empty())
		return;

	CloseSocket(user._fd);

	_online_user.DeleteUser(szid);
}

bool MsgClient::Init(ISystemEnv *pEnv)
{
	_pEnv = pEnv;

	setpackspliter(&_packspliter);

	return true;
}

void MsgClient::Stop(void)
{
	OUT_INFO("Msg", 0, "MsgClient", "stop");

	StopClient();
}

bool MsgClient::Start(void)
{
	return StartClient("0.0.0.0", 0, 1);
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
//	FUNTRACE("void ClientAccessServer::HandleInterData(int fd, const void *data, int len)");
	if (len < 4)
	{
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "recv data error length: %d", len );
		return;
	}

	OUT_RECV( sock->_szIp, sock->_port,  NULL, "on_data_arrived:[%d]%s", len, (const char*)data);

	CInterCoder coder;
	// ���ݽ��ܴ���
	if (!coder.Decode((const char *) data, len))
	{
		OUT_ERROR( sock->_szIp, sock->_port, "Decode", "fd %d, MsgClient recv error data:%d,%s", sock->_fd, len, ( const char *)data );
		return;
	}

	const char *ptr = (const char *) coder.Buffer();
	if (strncmp(ptr, "CAIT", 4) == 0)
	{
		// �׷���������
		HandleInnerData( sock, ptr, coder.Length());
	}
	else
	{
		// �����½���
		HandleSession( sock, ptr, coder.Length());
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
	OUT_WARNING( sock->_szIp, sock->_port, user._user_id.c_str(), "Disconnection fd %d", sock->_fd );
	user._user_state = User::OFF_LINE;
	_online_user.SetUser(user._user_id, user);
}

void MsgClient::TimeWork()
{
	/*
	 * 1.����ʱ������ȥ����
	 * 2.��ʱ����NOOP��Ϣ
	 * 3.Reload�����ļ��е��µ����ӡ�
	 * 4.
	 */
	while (1)
	{
		if (!Check())
			break;

		HandleOfflineUsers();
		HandleOnlineUsers(30);

		// ����ʱ�Ķ���
		_session.CheckTimeOut(120);
		//����ط�����ʱ��ͬ��������
		sleep(5);
	}
}

void MsgClient::NoopWork()
{

}

// ��MSG�ϴ���Ϣ
void MsgClient::HandleMsgData(const char *macid, const char *data, int len)
{
	OUT_INFO( NULL, 0, "msg", "HandleUpMsgData %s", data );

	string val;
	if (!_session.GetSession(macid, val))
	{
		SendOnlineData(data, len);
		//OUT_ERROR( NULL, 0, macid, "Get Session Failed, Data %s" , data );
		return;
	}
	User user = _online_user.GetUserByUserId(val);
	if (user._user_id.empty() || user._fd == NULL )
	{
		OUT_ERROR( NULL, 0, macid, "Get User empty , User id: %s, data %s" , val.c_str() , data );
		return;
	}
	// ��������
	SendData(user._fd, data, len);
}

int MsgClient::build_login_msg(User &user, char *buf, int buf_len)
{
	sprintf(buf, "LOGI SAVE %s %s DM \r\n", user._user_name.c_str(), user._user_pwd.c_str());
	return (int) strlen(buf);
}

void MsgClient::HandleSession( socket_t *sock, const char *data, int len)
{
	string line(data, len - 2);

	vector<string> vec_temp;
	if (!splitvector(line, vec_temp, " ", 1))
	{
		return;
	}

	string head = vec_temp[0];
	if (head == "LACK")
	{
		/*
		 RESULT
		 >=0:Ȩ��ֵ
		 -1:�������
		 -2:�ʺ��Ѿ���¼
		 -3:�ʺ��Ѿ�ͣ��
		 -4:�ʺŲ�����
		 -5:sql��ѯʧ��
		 -6:δ��¼���ݿ�
		 */
		int ret = atoi(vec_temp[1].c_str());
		switch (ret)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			User user = _online_user.GetUserBySocket( sock );
			if (user._user_id.empty())
			{
				OUT_WARNING(sock->_szIp, sock->_port,  NULL, "Can't find the syn_user");
				return;
			}
			user._user_state = User::ON_LINE;
			user._last_active_time = time(NULL);
			// ���´����û�״̬
			_online_user.SetUser(user._user_id, user);

			OUT_CONN( sock->_szIp, sock->_port, user._user_name.c_str(), "Login success, fd %d access code %d" ,
					sock->_fd, user._access_code );

			// �����½�ɾͷ��͵�ǰ���ĵ�����
			SendDemandData( sock );
		}
			break;
		case -1:
		{
			OUT_ERROR(sock->_szIp, sock->_port, NULL, "LACK,password error!");
		}
		break;
		case -2:
		{
			OUT_ERROR(sock->_szIp, sock->_port, NULL, "LACK,the user has already login!");
		}
		break;
		case -3:
		{
			OUT_ERROR(sock->_szIp, sock->_port, NULL, "LACK,user name is invalid!");
		}
		break;
		default:
		{
			OUT_ERROR( sock->_szIp, sock->_port, NULL, "unknow result" );
		}
		break;
		}

		// ������ش�����ֱ�Ӵ���
		if (ret < 0)
		{
			_tcp_handle.close_socket( sock );
		}
	}
	else if (head == "NOOP_ACK")
	{
		User user = _online_user.GetUserBySocket( sock );
		user._last_active_time = time(NULL);
		_online_user.SetUser(user._user_id, user);

		OUT_INFO( sock->_szIp, sock->_port, user._user_name.c_str(), "NOOP_ACK");
	}
	else
	{
		OUT_WARNING( sock->_szIp, sock->_port, NULL, "except message:%s", (const char*)data);
	}
}

void MsgClient::HandleInnerData( socket_t *sock, const char *data, int len)
{
	User user = _online_user.GetUserBySocket( sock );
	if (user._user_id.empty() || sock != user._fd )
	{
		OUT_ERROR( sock->_szIp, sock->_port, "CAIS" , "find fd %d user failed, data %s", sock->_fd, data );
		return;
	}

	string line(data, len);
	vector<string> vec;
	if (!splitvector(line, vec, " ", 6))
	{
		OUT_ERROR( sock->_szIp, sock->_port, user._user_name.c_str() , "fd %d data error: %s", sock->_fd, data );
		return;
	}

	string macid = vec[2];
	// if store type need save session
	_session.AddSession(macid, user._user_id);

	// ToDo: ��Ҫ�������ݽ����ݵݽ�������ģ����д���
	_pEnv->GetPushServer()->HandleData(data, len);

	user._last_active_time = time(NULL);
	_online_user.SetUser(user._user_id, user);
}

// ��Ӷ��Ļ���
void MsgClient::AddDemand(const char *name, int type)
{
	if (name == NULL)
		return;

	char buf[1024] = {0};
	switch (type)
	{
	case DEMAND_MACID:
		if (_macidsubmgr.AddSubId(name))
		{
		}
		sprintf(buf, "ADD 0 {%s} \r\n", name);
		break;
	case DEMAND_GROUP:
		if (_groupsubmgr.AddSubId(name))
		{
		}
		sprintf(buf, "ADD 1 {%s} \r\n", name);
		break;
	}

	// �����Ҫ����֪ͨ
	int len = strlen(buf);
	if (len > 0)
	{
		SendOnlineData(buf, len);
	}
}

// ȡ��������
void MsgClient::DelDemand(const char *name, int type)
{
	if (name == NULL)
		return;

	char buf[1024] = {0};
	switch (type)
	{
	case DEMAND_MACID:
		if (_macidsubmgr.DelSubId(name))
		{
		}
		sprintf(buf, "UMD 0 {%s} \r\n", name);
		break;
	case DEMAND_GROUP:
		if (_groupsubmgr.DelSubId(name))
		{
		}
		sprintf(buf, "UMD 1 {%s} \r\n", name);
		break;
	}

	// �����Ҫ����֪ͨ
	int len = strlen(buf);
	if (len > 0)
	{
		SendOnlineData(buf, len);
	}
}

// ֪ͨ�����û�
void MsgClient::SendOnlineData(const char *data, int len)
{
	vector<User> vec = _online_user.GetOnlineUsers();
	if (vec.empty())
		return;

	for (int i = 0; i < vec.size(); ++i)
	{
		User &user = vec[i];
		SendData(user._fd, data, len);
	}
}

// �׷���½��
void MsgClient::SendDemandData( socket_t *sock )
{
	set<string>::iterator it;
	// ��ǰ�鲻����Ҫ����
	if (!_groupsubmgr.IsEmpty())
	{
		string groups;
		set<string> &temp = _groupsubmgr.GetSubIds();
		for (it = temp.begin(); it != temp.end(); ++it)
		{
			if (!groups.empty())
			{
				groups += ",";
			}
			groups += *it;
		}

		string scmd = "DMD 1 {" + groups + "} \r\n";
		SendData( sock, scmd.c_str(), scmd.length());
		OUT_INFO( sock->_szIp, sock->_port, NULL, "fd %d, Send group cmd %s", sock->_fd, scmd.c_str() );
	}


	// �����ǰMACID��Ϊ��Ҳ��Ҫ����
	if (!_macidsubmgr.IsEmpty())
	{
		string macids;
		set<string> &temp = _macidsubmgr.GetSubIds();
		for (it = temp.begin(); it != temp.end(); ++it)
		{
			if (!macids.empty())
			{
				macids += ",";
			}
			macids += *it;
		}

		string scmd = "ADD 0 {" + macids + "} \r\n";
		SendData( sock, scmd.c_str(), scmd.length());
		OUT_INFO( sock->_szIp, sock->_port, NULL, "fd %d, Send macid cmd %s", sock->_fd, scmd.c_str() );
	}
}

void MsgClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3 * 60);
	for (int i = 0; i < (int) vec_users.size(); i++)
	{
		User &user = vec_users[i];
		if (user._socket_type == User::TcpClient)
		{
			if (user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(), "fd %d close socket", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
		else if (user._socket_type == User::TcpConnClient)
		{
			if (user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(),
						"client fd %d close socket", user._fd->_fd );
				user.show();
				CloseSocket(user._fd);
				user._fd = NULL ;
			}
			if (ConnectServer(user, 10))
			{
				// ����б��С�
				_online_user.AddUser(user._user_id, user);
			}
			else if (user._connect_info.keep_alive == AlwaysReConn)
			{
				// ����û�
				_online_user.AddUser(user._user_id, user);
			}
		}
	}
}

void MsgClient::HandleOnlineUsers(int timeval)
{
	time_t now = time(NULL);
	if (now - _last_handle_user_time < timeval)
	{
		return;
	}
	_last_handle_user_time = now;

	vector<User> vec_users = _online_user.GetOnlineUsers();
	for (int i = 0; i < (int) vec_users.size(); i++)
	{
		User &user = vec_users[i];
		if ( user._socket_type == User::TcpConnClient && user._fd != NULL )
		{
			string loop = "NOOP \r\n";
			SendData(user._fd, loop.c_str(), loop.length());
			OUT_SEND( user._ip.c_str(), user._port, user._user_id.c_str(), "NOOP");
		}
	}
}

