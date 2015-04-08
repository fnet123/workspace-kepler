#include "msgclient.h"
#include <tools.h>
#include <netutil.h>
#include <intercoder.h>

MsgClient::MsgClient( PConvert *convert ) : _convert( convert )
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
	sprintf(szid, "%lu", netutil::strToAddr(ip, port));

	User user = _online_user.GetUserByUserId(szid);
	if (!user._user_id.empty())
		return;

	user._user_id = szid;
	user._ip = ip;
	user._port = port;
	user._user_name = username;
	user._user_pwd = pwd;
	user._user_type = "UPIPE";
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
	sprintf(szid, "%lu", netutil::strToAddr(ip, port));

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
	if (len < 4){
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "recv data error length: %d", len );
		return;
	}

	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	OUT_RECV( ip, port, NULL, "fd %d, on_data_arrived:[%d]%s", sock->_fd, len, (const char*)data);

	CInterCoder coder;
	// ���ݽ��ܴ���
	if (!coder.Decode((const char *) data, len)) {
		OUT_ERROR( ip, port, "Decode", "fd %d, MsgClient recv error data:%d,%s", sock->_fd , len, ( const char *)data );
		return;
	}

	const char *ptr = (const char *) coder.Buffer();
	if (strncmp(ptr, "CAIT", 4) == 0)
	{
		// �׷���������
		HandleInnerData( sock , ptr, coder.Length());
	}
	else
	{
		// �����½���
		HandleSession( sock , ptr, coder.Length());
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket(sock);
	OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "Disconnection");
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
		// _session.CheckTimeOut(120);
		//����ط�����ʱ��ͬ��������
		sleep(5);
	}
}

void MsgClient::NoopWork()
{

}

// ��MSG�ϴ���Ϣ
bool MsgClient::Deliver( const char *data, int len )
{
	// ���û�п����ڵ�����ֱ�ӷ�����
	vector<User> vec = _online_user.GetOnlineUsers() ;
	if ( vec.empty() ) {
		OUT_ERROR( NULL, 0, "msg", "HandleUpMsgData %s", data ) ;
		return false ;
	}

	int send = 0 ;
	for ( int i = 0; i < (int) vec.size(); ++ i ) {
		User &user = vec[i] ;
		if ( user._fd == NULL )
			continue ;

		if ( SendData( user._fd, data, len ) ) {
			++ send ;
		}
	}

	if ( send > 0 ) {
		OUT_PRINT( NULL , 0, "msg", "HandleUpMsgData %s", data );
	}

	return true ;
}

int MsgClient::build_login_msg(User &user, char *buf, int buf_len)
{
	sprintf(buf, "LOGI UPIPE %s %s \r\n", user._user_name.c_str(), user._user_pwd.c_str());
	return (int) strlen(buf);
}

void MsgClient::HandleSession( socket_t *sock, const char *data, int len)
{
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	char *ptr = strstr( data, " " ) ;
	if ( ptr == NULL ) {
		OUT_ERROR( ip, port, NULL, "split session data failed" ) ;
		return ;
	}

	// ���Ϊ��½��Ϣ
	if ( strncmp( data, "LACK", 4 ) == 0 )
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
		 */// LACK 0 0 0 %d \r\n
		int ret = atoi(ptr+1);
		switch (ret)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			{
				User user = _online_user.GetUserBySocket(sock);
				if (user._user_id.empty())
				{
					OUT_WARNING(ip, port, NULL, "Can't find the syn_user");
					return;
				}
				user._user_state = User::ON_LINE;
				user._last_active_time = time(NULL);
				// ���´����û�״̬
				_online_user.SetUser(user._user_id, user);

				OUT_CONN( ip , port, user._user_name.c_str(), "Login success, fd %d access code %d" , sock->_fd, user._access_code );
			}
			break;
		case -1:
			{
				OUT_ERROR(ip, port, NULL, "LACK,password error!");
			}
			break;
		case -2:
			{
				OUT_ERROR(ip, port, NULL, "LACK,the user has already login!");
			}
			break;
		case -3:
			{
				OUT_ERROR(ip, port, NULL, "LACK,user name is invalid!");
			}
			break;
		default:
			{
				OUT_ERROR( ip, port, NULL, "unknow result" );
			}
			break;
		}

		// ������ش�����ֱ�Ӵ���
		if (ret < 0)
		{
			_tcp_handle.close_socket(sock);
		}
	}
	else if ( strncmp( data, "NOOP_ACK" , 8 ) == 0 )
	{
		User user = _online_user.GetUserBySocket(sock);
		user._last_active_time = time(NULL);
		_online_user.SetUser(user._user_id, user);

		OUT_INFO( ip, port, user._user_name.c_str(), "NOOP_ACK");
	}
	else
	{
		OUT_WARNING( ip, port, NULL, "except message:%s", (const char*)data);
	}
}

void MsgClient::HandleInnerData( socket_t *sock, const char *data, int len)
{
	const char *ip 		= sock->_szIp;
	unsigned short port = sock->_port;

	User user = _online_user.GetUserBySocket( sock ) ;
	if ( user._user_id.empty() ) {
		OUT_ERROR( ip, port, NULL, "get fd %d user failed", sock->_fd ) ;
		return ;
	}

	// CAITS 0_0 MACID 0 U_REPT {TYPE:5,18:������״̬/ǰ�û�������ID/�ܵ����Ʒ���ID/��Ϣ������id}
	string line(data, len);
	vector<string> vec;
	if ( !splitvector( line, vec, " ", 6 ) ) {
		OUT_ERROR( ip, port, user._user_name.c_str() , "fd %d data error: %s", sock->_fd, data );
		return;
	}

	string macid = vec[2] ;
	if ( macid.empty() ){
		OUT_ERROR( ip, port, user._user_name.c_str(), "fd %d macid empty", sock->_fd ) ;
		return ;
	}

	string vechile ;
	if ( ! _macid2carnum.GetSession( macid, vechile ) ) {
		OUT_ERROR( ip, port, user._user_name.c_str(), "fd %d get vechile num by mac id %s failed", macid.c_str() ) ;
		return ;
	}

	string acode ;
	DataBuffer buf ;
	// ת������ָ��
	if (vec[4] == "D_CTLM") {
		// ת������ָ��
		_convert->convert_ctrl( vec[1], macid , vec[5] , vechile, buf , acode ) ;
	} else if ( vec[4] == "D_SNDM" ) {
        OUT_INFO( ip, port, user._user_name.c_str(), "D_SNDM"); //xifengming
		_convert->convert_sndm( vec[1] , macid, vec[5], vechile, buf, acode ) ;
	}
	if ( ! acode.empty() && buf.getLength() > 0 ) {
		// �·���PAS��Ӧ�Ľ�����
		_pEnv->GetPasServer()->HandlePasDown( acode.c_str(), buf.getBuffer(), buf.getLength() ) ;
	}

	user._last_active_time = time(NULL);
	_online_user.SetUser(user._user_id, user);
}

void MsgClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3 * 60);
	for (int i = 0; i < (int) vec_users.size(); i++)
	{
		User &user = vec_users[i];
		if (user._socket_type == User::TcpClient)
		{
			if ( user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(), "fd %d close socket", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
		else if (user._socket_type == User::TcpConnClient)
		{
			if ( user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(), "fd %d close socket", user._fd->_fd );
				user.show();
				CloseSocket(user._fd);
				user._fd = NULL;
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
		if (user._socket_type == User::TcpConnClient)
		{
			string loop = "NOOP \r\n";
			SendData(user._fd, loop.c_str(), loop.length());
			OUT_SEND(vec_users[i]._ip.c_str(), vec_users[i]._port, vec_users[i]._user_id.c_str(),
					"NOOP");
		}
	}
}

// ����ֻ�MAC
void MsgClient::AddMac2Car( const char *macid, const char *vechile )
{
	_macid2carnum.AddSession( macid, vechile ) ;
}

