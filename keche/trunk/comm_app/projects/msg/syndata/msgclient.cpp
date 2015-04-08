#include "msgclient.h"
#include <tools.h>
#include <intercoder.h>
#include "picclient.h"

#define MSG_BACK_ID  "syndata_msg"

MsgClient::MsgClient():
	_filecache(this)
{
	_enable = false ;
	_picclient = new CPicClient;
}

MsgClient::~MsgClient( void )
{
	Stop() ;
	if ( _picclient != NULL ) {
		delete _picclient ;
		_picclient = NULL ;
	}
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	int nvalue = 0 ;
	if ( pEnv->GetInteger( "syn_data", nvalue ) ) {
		_enable = ( nvalue == 1 ) ;
	}
	if ( ! _enable ) return true ;

	char ip[128] = {0} ;
	// ͬ�����ķ�������IP
	if ( !pEnv->GetString( "syn_server", ip ) ) {
		printf( "remote syn_server center ip failed" ) ;
		OUT_ERROR( NULL, 0, NULL,"remote syn center ip failed" ) ;
		return false ;
	}
	// ͬ�����ķ������Ķ˿�
	if ( ! pEnv->GetInteger( "syn_port", nvalue ) ) {
		printf( "remote syn_port center port failed" ) ;
		OUT_ERROR( NULL, 0, NULL,"remote syn center port failed" ) ;
		return false ;
	}

	// ��ʼ��ͼƬ�������
	if ( ! _picclient->Init( pEnv ) ) {
		OUT_ERROR( NULL, 0, NULL, "init pic client failed" ) ;
	}

	User user ;
	user._fd        			  = NULL ;
	user._ip					  = ip ;
	user._port					  = nvalue ;

	char szval[128] = {0} ;
	if ( ! pEnv->GetString( "syn_user", szval ) ) {
		printf( "remote syn_user failed\n" ) ;
		OUT_ERROR( NULL, 0, NULL, "remote syn user failed" ) ;
		return false ;
	}

	user._user_id   			  = szval ;
	user._user_name 		 	  = szval ;

	if ( ! pEnv->GetString( "syn_pwd", szval ) ) {
		printf( "remote syn_pwd failed\n" ) ;
		OUT_ERROR( NULL, 0, NULL, "remote syn_pwd failed" ) ;
		return false ;
	}

	user._user_pwd				  = szval ;
	user._user_type				  = "PIPE" ;
	user._user_state 			  = User::OFF_LINE ;
	user._socket_type			  = User::TcpConnClient ;
	user._connect_info.keep_alive = AlwaysReConn ;
	user._connect_info.timeval	  = 30 ;

	_online_user.AddUser( user._user_id, user ) ;

	setpackspliter( &_packspliter ) ;

	char szbuf[512] = {0};
	if ( ! pEnv->GetString( "base_filedir" , szbuf ) ) {
		printf( "load base_filedir failed\n" ) ;
		return false ;
	}

	nvalue = 0 ;
	pEnv->GetInteger( "sendcache_speed", nvalue ) ;

	char temp[1024] = {0} ;
	sprintf( temp, "%s/msgdata", szbuf ) ;

	return _filecache.Init( temp, nvalue ) ;
}


void MsgClient::Stop( void )
{
	if ( ! _enable ) return ;

	OUT_INFO("Msg",0,"MsgClient","stop");

	StopClient() ;

	_picclient->Stop() ;
}

bool MsgClient::Start( void )
{
	if ( ! _enable ) return true ;

	_picclient->Start() ;

	return StartClient( "0.0.0.0", 0, 3 ) ;
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
//	FUNTRACE("void ClientAccessServer::HandleInterData(int fd, const void *data, int len)");
	if ( len < 4 ) {
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "recv data error length: %d", len ) ;
		return ;
	}

    // ���ܴ�������
	CInterCoder coder;
	coder.Decode( (const char *)data, len );

	OUT_RECV( sock->_szIp, sock->_port, NULL, "fd %d,on_data_arrived:[%d]%s", sock->_fd, coder.Length(), coder.Buffer() );

	const char *ptr = ( const char *) coder.Buffer() ;
	if ( strncmp( ptr, "CAIT" , 4 ) == 0  ) {
		// �׷���������
		HandleInnerData( sock, ptr, coder.Length() ) ;
	} else {
		// �����½���
		HandleSession( sock, ptr, coder.Length() ) ;
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket(sock);
	OUT_WARNING( user._ip.c_str() , user._port, user._user_id.c_str(), "Disconnection fd %d" , sock->_fd );
	user._user_state = User::OFF_LINE ;
	_online_user.SetUser( user._user_id, user ) ;
}

void MsgClient::TimeWork()
{
	/*
	 * 1.����ʱ������ȥ����
	 * 2.��ʱ����NOOP��Ϣ
	 * 3.Reload�����ļ��е��µ����ӡ�
	 * 4.
	 */
	while(1){

		if ( ! Check() ) break ;
		HandleOfflineUsers() ;
		HandleOnlineUsers( 30 ) ;

		//����ط�����ʱ��ͬ��������
		sleep(5);
	}
}

void MsgClient::NoopWork()
{
	while(1){
		if ( ! Check() ) break ;
		// ����Ƿ��л�������Ҫд��
		if ( ! _filecache.Check() ) {
			sleep(1) ;
		}
	}
}

// ������½��Ϣ����
int  MsgClient::build_login_msg(User &user, char *buf, int buf_len)
{
	char szbuf[1024] = {0} ;
	sprintf( szbuf, "LOGI %s %s %s \r\n",
			user._user_type.c_str(), user._user_name.c_str(), user._user_pwd.c_str() ) ;

	// ���ܴ�������
	CInterCoder coder;
	coder.Encode( szbuf, strlen(szbuf) );

	memcpy( buf, coder.Buffer(), coder.Length() ) ;

	return coder.Length() ;
}

// �׷���½�û��Ự����
void MsgClient::HandleSession( socket_t *sock, const char *data, int len )
{
	string line = data;

	vector<string> vec_temp ;
	if ( ! splitvector( line, vec_temp, " " , 1 ) ) {
		return ;
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
		int ret = atoi( vec_temp[1].c_str() ) ;
		switch( ret )
		{
		case 0:
		case 1:
		case 2:
		case 3:
			{
				User user = _online_user.GetUserBySocket( sock ) ;
				if( user._user_id.empty() )
				{
					OUT_WARNING( sock->_szIp, sock->_port, NULL,"Can't find the syn_user");
					return;
				}
				user._user_state 		= User::ON_LINE ;
				user._last_active_time  = time(NULL) ;
				// ���´����û�״̬
				_online_user.SetUser( user._user_id, user ) ;

				OUT_CONN( sock->_szIp, sock->_port, user._user_name.c_str(), "Login success, fd %d access code %d" , sock->_fd, user._access_code ) ;
				// �û���½����Ϊ����״̬
				_filecache.Online( MSG_BACK_ID ) ;
			}
			break ;
		case -1:
			{
				OUT_ERROR( sock->_szIp, sock->_port, NULL , "LACK,password error!");
			}
			break ;
		case -2:
			{
				OUT_ERROR( sock->_szIp, sock->_port, NULL ,"LACK,the user has already login!");
			}
			break ;
		case -3:
			{
				OUT_ERROR( sock->_szIp, sock->_port, NULL, "LACK,user name is invalid!");
			}
			break ;
		default:
			{
				OUT_ERROR( sock->_szIp, sock->_port, NULL,  "unknow result" ) ;
			}
			break;
		}

		// ������ش�����ֱ�Ӵ���
		if ( ret < 0 )
		{
			_tcp_handle.close_socket(sock);
		}
	}
	else if (head == "NOOP_ACK")
	{
		User user = _online_user.GetUserBySocket( sock ) ;
		user._last_active_time  = time(NULL) ;
		_online_user.SetUser( user._user_id, user ) ;

		OUT_INFO( sock->_szIp, sock->_port, user._user_name.c_str() , "NOOP_ACK");
	}
	else
	{
		OUT_WARNING( sock->_szIp, sock->_port, NULL, "except message:%s", (const char*)data );
	}
}

void MsgClient::HandleInnerData( socket_t *sock, const char *data, int len )
{
	User user = _online_user.GetUserBySocket( sock ) ;
	if ( user._user_id.empty()  ) {
		OUT_ERROR( sock->_szIp, sock->_port, "CAIS" , "find fd %d user failed, data %s", sock->_fd, data ) ;
		return ;
	}

	// ToDo: �·���MSG
	_pEnv->GetMsgClientServer()->DeliverEx( NULL, data, len ) ;

	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;
}

// �������ݵ�MSG
void MsgClient::HandleData( const char *data, int len , bool pic )
{
	if ( ! _enable ) return ;

	// ���ΪͼƬֱ�����ͼƬ������������
	if ( pic ) {
		_picclient->AddMedia( data, len ) ;
		return ;
	}
	// ���ܴ�������
	CInterCoder coder;
	coder.Encode( data, len );

	vector<User> vec = _online_user.GetOnlineUsers() ;
	if ( vec.empty() ) {
		_filecache.WriteCache( MSG_BACK_ID, (void*)coder.Buffer(), coder.Length() ) ;
		return ;
	}

	bool send = false ;
	int nsize = vec.size();
	for ( int i = 0; i < nsize; ++ i ) {
		// �����ݽ����ܴ���
		if ( ! SendData( vec[i]._fd, coder.Buffer(), coder.Length() ) ) {
			continue ;
		}
		send = true ;
	}
	if ( ! send ) {
		_filecache.WriteCache( MSG_BACK_ID, (void*)coder.Buffer(), coder.Length() ) ;
	}
}

void MsgClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3*60);
	for(int i = 0; i < (int)vec_users.size(); i++)
	{
		User &user = vec_users[i];
		if(user._socket_type == User::TcpClient)
		{
			if( user._fd != NULL ){
				OUT_WARNING( user._ip.c_str() , user._port , user._user_name.c_str() , "HandleOffline Users close socket fd %d", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
		else if(user._socket_type == User::TcpConnClient)
		{
			if(user._fd != NULL)
			{
				OUT_WARNING( user._ip.c_str() , user._port , user._user_name.c_str() ,"TcpConnClient close socket fd %d", user._fd->_fd );
				user.show();
				CloseSocket(user._fd);
				user._fd = NULL;
			}
			if ( ConnectServer(user, 10) ) {
				// ����б��С�
				_online_user.AddUser( user._user_id, user ) ;
			} else if ( user._connect_info.keep_alive == AlwaysReConn ) {
				// ����û�
				_online_user.AddUser( user._user_id, user ) ;
			}
		}
	}
}

void MsgClient::HandleOnlineUsers(int timeval)
{
	time_t now = time(NULL) ;

	static time_t last_handle_user_time = 0;
	if( now - last_handle_user_time < timeval){
		return;
	}
	last_handle_user_time = now ;

	vector<User> vec_users = _online_user.GetOnlineUsers();
	if ( vec_users.size() == 0 )
		return ;

	char szbuf[128] = {"NOOP \r\n"} ;
	// ���ܴ�������
	CInterCoder coder;
	coder.Encode( szbuf, strlen(szbuf) );

	int nsize = vec_users.size() ;
	for(int i = 0; i < nsize; ++ i ){
		User &user = vec_users[i] ;
		if( user._socket_type == User::TcpConnClient && user._fd != NULL )
		{
			SendData( user._fd, coder.Buffer(), coder.Length() ) ;
			OUT_SEND(vec_users[i]._ip.c_str(),vec_users[i]._port,vec_users[i]._user_id.c_str(),"NOOP");
		}
	}
	// �û���½����Ϊ����״̬
	_filecache.Online( MSG_BACK_ID ) ;
}

// �������ݻص��ӿ�
int MsgClient::HandleQueue( const char *sid, void *buf, int len , int msgid )
{
	// ���ж�һ���Ƿ��������û�
	vector< User > vec = _online_user.GetOnlineUsers();
	if ( vec.empty() ) {
		return IOHANDLE_FAILED;
	}

	// ����·����������ת����
	int nsize = vec.size();
	for ( int i = 0; i < nsize; ++ i ) {
		// �����ݽ����ܴ���
		SendData( vec[i]._fd, (const char *)buf, len ) ;
	}
	return IOHANDLE_SUCCESS;
}
