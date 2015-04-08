#include "msgclient.h"
#include "pccutil.h"
#include "pconvert.h"
#include <tools.h>

MsgClient::MsgClient( PConvert *convert )
	: _convert(convert)
{
	_last_handle_user_time = time(NULL) ;
}

MsgClient::~MsgClient( void )
{
	Stop() ;
}

// ����MSG���û��ļ�
bool MsgClient::LoadMsgUser( const char *userfile )
{
	if ( userfile == NULL ) return false ;

	char buf[1024] = {0};
	FILE *fp = NULL;
	fp = fopen( userfile, "r" );
	if (fp == NULL){
		OUT_ERROR( NULL, 0, NULL, "Load msg user file %s failed", userfile ) ;
		return false;
	}

	int count = 0 ;
	while (fgets(buf, sizeof(buf), fp)){
		unsigned int i = 0;
		while (i < sizeof(buf)){
			if (!isspace(buf[i]))
				break;
			i++;
		}
		if (buf[i] == '#')
			continue;

		char temp[1024] = {0};
		for (int i = 0, j = 0; i < (int)strlen(buf); ++ i ){
			if (buf[i] != ' ' && buf[i] != '\r' && buf[i] != '\n'){
				temp[j++] = buf[i];
			}
		}

		string line = temp;

		//1:10.1.99.115:8880:user_name:user_password:A3
		vector<string> vec_line ;
		if ( ! splitvector( line, vec_line, ":" , 7 ) ){
			continue ;
		}

		if ( vec_line[0] != MSG_USER_TAG ) {
			continue ;
		}

		User user ;
		user._user_id     =  vec_line[0] + vec_line[1] ;
		user._access_code = atoi( vec_line[1].c_str() ) ;
		user._ip          =  vec_line[2] ;
		user._port        =  atoi( vec_line[3].c_str() ) ;
		user._user_name   =  vec_line[4] ;
		user._user_pwd    =  vec_line[5] ;
		user._user_type   =  vec_line[6] ;
		user._user_state  = User::OFF_LINE ;
		user._socket_type = User::TcpConnClient ;
		user._connect_info.keep_alive = AlwaysReConn ;
		user._connect_info.timeval    = 30 ;

		// ��ӵ��û�������
		_online_user.AddUser( user._user_id, user ) ;

		++ count ;
	}
	fclose(fp);
	fp = NULL;

	OUT_INFO( NULL, 0, NULL, "load msg user success %s, count %d" , userfile , count ) ;
	printf( "load msg user count %d success %s\n" , count, userfile ) ;

	return true ;
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	// ��ʼ��ת����������
	_convert->initenv( pEnv ) ;

	char temp[256] = {0} ;
	// HTTP��URL�Ļ���ַ��·��
	if ( pEnv->GetString( "base_picurl", temp ) ) {
		_picUrl = temp ;
	}

	// ���ػ��������б�·��
	if ( pEnv->GetString( "base_dmddir" ,temp ) ) {
		_dmddir.SetString( temp ) ;
		OUT_INFO( NULL, 0 , NULL, "Load base dmddir %s", temp ) ;
	}

	if ( ! pEnv->GetString( "user_filepath" , temp ) ) {
		printf( "load user file failed\n" ) ;
		return false ;
	}

	int nvalue = 0 , send_thread = 1, recv_thread = 1, queue_size = 1000 ;
	// �����߳�
	if ( pEnv->GetInteger( "http_send_thread" , nvalue ) ) {
		send_thread = nvalue ;
	}
	// �����߳�
	if ( pEnv->GetInteger( "http_recv_thread" , nvalue ) ) {
		recv_thread = nvalue ;
	}
	// HTTP�������Ķ��г���
	if ( pEnv->GetInteger( "http_queue_size" , nvalue ) ) {
		queue_size = nvalue ;
	}

	// ��ʼ��HTTP�ķ���
	if ( ! _httpcaller.Init( send_thread, recv_thread, queue_size ) ) {
		OUT_ERROR( NULL, 0, NULL, "init http caller failed" ) ;
		return false ;
	}
	_httpcaller.SetReponse( this ) ;

	// ���÷ְ�����
	setpackspliter( &_packspliter ) ;

	return LoadMsgUser( temp ) ;
}


void MsgClient::Stop( void )
{
	OUT_INFO("Msg",0,"MsgClient","stop");

	StopClient() ;

	_httpcaller.Stop() ;
}

bool MsgClient::Start( void )
{
	if ( ! _httpcaller.Start() ) {
		OUT_ERROR( NULL, 0, NULL, "start http caller failed" ) ;
		return false ;
	}
	return StartClient( "0.0.0.0", 0, 3 ) ;
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
	if ( len < 4 ) return ;
	
	OUT_RECV3( sock->_szIp, sock->_port,  NULL, "on_data_arrived:[%d]%s", len, (const char*)data );

	const char *ptr = ( const char *)data ;
	if ( strncmp( ptr, "CAIT" , 4 ) == 0 ) {
		// �׷���������
		HandleInnerData( sock, ptr, len ) ;
	} else {
		// �����½���
		HandleSession( sock, ptr, len ) ;
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket(sock);
	OUT_WARNING( sock->_szIp, sock->_port, user._user_id.c_str(), "Disconnection fd %d" , sock->_fd );
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
	while(1)
	{
		if ( ! Check() ) break ;

		HandleOfflineUsers() ;

		//����ط�����ʱ��ͬ��������
		sleep(5);
	}
}

void MsgClient::NoopWork()
{
	while(1) {
		if ( !Check() ) break ;

		HandleOnlineUsers( 30 ) ;

		sleep(5) ;
	}
}

bool MsgClient::SendDataToUser( const string &area_code, const char *data, int len)
{
	if ( area_code == SEND_ALL )
	{
		vector<User>  users = _online_user.GetOnlineUsers() ;
		if ( users.empty() ) {
			return false ;
		}

		for ( size_t i = 0; i < users.size(); ++ i ) {
			// Ⱥ������
			SendData( users[i]._fd, data, len ) ;
		}

		return true ;
	}

	char buf[512] = {0};
	sprintf( buf, "%s%s", MSG_USER_TAG, area_code.c_str() ) ;

	User user = _online_user.GetUserByUserId( buf );
	if( user._user_id.empty() || user._user_state != User::ON_LINE )
	{
		OUT_ERROR( user._ip.c_str() , user._port , buf , "SendDataToUser %s failed" , data ) ;
		return false;
	}

	// ���������������ѭ����Ĵ���
	return SendData( user._fd, data, len ) ;
}

// ��MSG�ϴ���Ϣ
bool MsgClient::HandleUpMsgData( const char *code, const char *data, int len )
{
	OUT_INFO( NULL, 0, "msg", "HandleUpMsgData %s", data ) ;

	if ( strstr( code, "_" ) != NULL ) {
		string userid ;
		// ȡ��MACID��Ӧ�Ľ�������
		if ( !_session.GetSession( code, userid , false ) ) {
			OUT_ERROR( NULL, 0, "Msg", "find macid %s userid failed, data: %s", code, data ) ;
			return false;
		}
		User user = _online_user.GetUserByUserId( userid ) ;
		if ( user._user_id.empty() || user._user_state != User::ON_LINE ) {
			OUT_ERROR( NULL, 0, userid.c_str() , "%s ,user not online, data: %s", code, data ) ;
			return false ;
		}
		// ��������
		return SendData( user._fd, data, len ) ;
	}
	// ����MSG�����������·��
	return SendDataToUser( code, data, len ) ;
}

// ���ض�������
void MsgClient::LoadSubscribe( User &user )
{
	if ( _dmddir.IsEmpty() )
		return ;

	char szbuf[1024] = {0};
	sprintf( szbuf, "%s/%d", _dmddir.GetBuffer(), user._access_code ) ;

	int   len = 0 ;
	char *ptr = ReadFile( szbuf, len ) ;
	if ( ptr == NULL ){
		OUT_ERROR( NULL, 0, NULL, "load subscribe file %s failed", szbuf ) ;
		return ;
	}

	// �Ƴ������лس��ո���ַ�
	while( len > 0 ) {
		if ( ptr[len-1] == '\r' || ptr[len-1] == '\n' || ptr[len-1] == ' ' || ptr[len-1] == '\t' ){
			ptr[len-1] = 0 ;
			-- len ;
			continue ;
		}
		break ;
	}

	// DMD 0 {E005_13571198041} \r\n
	CQString sz ;
	sz.AppendBuffer( "DMD 0 {" ) ;
	sz.AppendBuffer( ptr , len ) ;
	sz.AppendBuffer( "} \r\n" ) ;
	FreeBuffer( ptr ) ;

	if ( ! SendData( user._fd, sz.GetBuffer(), sz.GetLength() ) ) {
		OUT_ERROR( NULL, 0, NULL, "Send Data : %s Failed", sz.GetBuffer() ) ;
	} else {
		OUT_PRINT( NULL, 0, NULL, "Send Sub: %s", sz.GetBuffer() ) ;
	}
}

// ������½����
int MsgClient::build_login_msg( User &user, char *buf, int buf_len )
{
	string stype = "SAVE" , sext = "\r\n" ;
	if ( user._user_type == "DMDATA" ) {
		stype = "SAVE" ; sext = "DM \r\n" ;
	} else {
		stype = user._user_type ;
	}
	sprintf( buf, "LOGI %s %s %s %s",
			stype.c_str() , user._user_name.c_str(), user._user_pwd.c_str() , sext.c_str() ) ;

	return strlen(buf) ;
}

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
					OUT_WARNING(sock->_szIp, sock->_port, NULL,"Can't find the syn_user");
					return;
				}
				user._user_state 		= User::ON_LINE ;
				user._last_active_time  = time(NULL) ;
				// ���´����û�״̬
				_online_user.SetUser( user._user_id, user ) ;

				OUT_CONN( sock->_szIp, sock->_port, user._user_name.c_str(), "Login success, fd %d access code %d" , sock->_fd, user._access_code ) ;
				// ��½�ɹ������Ϊ���ݶ������Ӿ�ֱ����Ҫ�����Ͷ��Ĵ���
				if ( user._user_type == "DMDATA" ) LoadSubscribe( user ) ;
			}
			break ;
		case -1:
			{
				OUT_ERROR( sock->_szIp, sock->_port,  NULL , "LACK,password error!");
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
				OUT_ERROR( sock->_szIp, sock->_port,  NULL,  "unknow result" ) ;
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
	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;

	string line( data, len ) ;
	vector<string>  vec ;
	if ( ! splitvector( line, vec, " " , 6 )  ){
		OUT_ERROR( sock->_szIp, sock->_port, user._user_name.c_str() , "fd %d data error: %s", sock->_fd, data ) ;
		return ;
	}

	DataBuffer buf ;

	string head  = vec[0] ;
	string seqid = vec[1] ;
	string macid = vec[2] ;
	string code  = vec[3] ;  // ͨ���룬���ڵ������ݵ�����
	string cmd   = vec[4] ;
	string val 	 = vec[5] ;

	if ( head == "CAITS" ) {
		if( cmd == "U_REPT" ){
			// �ϱ�����Ϣ����
			_convert->convert_urept( macid , val , buf , ( code == "201") ) ;
		} else if( cmd == "D_CTLM" ) {
			// ToDo: ��������Ϣ����

		} else if( cmd == "D_SNDM" ) {
			// ToDo : ��Ϣ���͵Ĵ���
		} else {
			OUT_WARNING( sock->_szIp, sock->_port, user._user_name.c_str() , "except message:%s", (const char*)data ) ;
		}
	} else {
		// ����ͨӦӦ����Ϣ
		_convert->convert_comm( seqid, macid, val, buf ) ;
	}

	if( buf.getLength() > 0 ) {
		// ����û��Ự��
		_session.AddSession( macid, user._user_id ) ;
		// ����ָ���ĵ����û�
		_pEnv->GetPasClient()->HandleData( buf.getBuffer(), buf.getLength() ) ;
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
			if(user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str() , user._port , user._user_name.c_str() , "fd %d close socket", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
		else if(user._socket_type == User::TcpConnClient)
		{
			if( user._fd != NULL )
			{
				OUT_INFO( user._ip.c_str() , user._port , user._user_name.c_str() ,"conn fd %d close socket", user._fd->_fd );
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
	if( now - _last_handle_user_time < timeval){
		return;
	}
	_last_handle_user_time = now ;

	vector<User> vec_users = _online_user.GetOnlineUsers();
	for(int i = 0; i < (int)vec_users.size(); i++)
	{
		User &user = vec_users[i] ;
		if( user._socket_type == User::TcpConnClient || user._fd != NULL )
		{
			string loop = "NOOP \r\n" ;
			SendData( user._fd, loop.c_str(), loop.length() ) ;
			OUT_SEND( user._ip.c_str(), user._port, user._user_id.c_str(),"NOOP");
		}
	}
}

//////////////////////////////////////////// ����ͨ��HTTP����ͼƬ  /////////////////////////////////////////////////////
// ͨ��HTTP������ȡͼƬ
void MsgClient::LoadUrlPic( unsigned int seq, const char *path )
{
	char url[1024] = {0};
	sprintf( url, "%s/%s", (const char *)_picUrl, path ) ;

	// ���ͻ�ȡ��Ƭ������
	if ( ! _httpcaller.Request( seq, url ) ) {
		// ��¼�����������
		OUT_ERROR( NULL, 0, NULL, "request url %s seq id %d failed", url, seq ) ;
	}
}

// ����HTTP����Ӧ�ص�����
void MsgClient::ProcessResp( unsigned int seqid, const char *data, const int len , const int err )
{
	// ��������
	if ( data == NULL || err != HTTP_CALL_SUCCESS || len == 0 ) {
		OUT_ERROR( NULL, 0, "Pic" , "pic seqid %u, error %d" , seqid, err ) ;
		return ;
	}
	_convert->sendpicture( seqid, data, len ) ;
}

