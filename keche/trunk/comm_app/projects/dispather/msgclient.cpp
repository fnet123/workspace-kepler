#include "msgclient.h"
#include <tools.h>

#include <string>
#include <vector>
#include <fstream>
using std::string;
using std::vector;
using std::ifstream;

MsgClient::MsgClient( const char *strtype ):
	_session( true ), _dataroute(false)
{
	_pMsgClient = NULL ;
	_last_handle_user_time = time(NULL) ;
	_strclient = strtype ;
}

MsgClient::~MsgClient( void )
{
	Stop() ;
}

// ����MSG���û��ļ�
bool MsgClient::LoadMsgUser( const char *userfile )
{
	if ( userfile == NULL ) return false ;

	int  len  = 0 ;
	char *ptr = ReadFile(userfile, len) ;
	if ( ptr == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "Read msg user failed, %s", userfile ) ;
		return false ;
	}

	int i = 0 ;
	// ���ָ������
	for( i = 0; i < len; ++ i ) {
		if ( ptr[i] != '\r' ) {
			continue ;
		}
		ptr[i] = '\n' ;
	}

	vector<string> vec ;
	if ( ! splitvector( ptr, vec, "\n", 0 ) ) {
		FreeBuffer( ptr ) ;
		OUT_ERROR( NULL, 0, NULL, "Split msg data failed" ) ;
		return false ;
	}

	int count = 0 ;
	for( i = 0; i < (int)vec.size(); ++ i )
	{
		string &line = vec[i] ;
		//1:10.1.99.115:8880:user_name:user_password:A3
		vector<string> vec_line ;
		if ( ! splitvector( line, vec_line, ":" , 7 ) ){
			continue ;
		}

		if ( vec_line[0] != _strclient ) {
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
	FreeBuffer( ptr ) ;

	OUT_INFO( NULL, 0, NULL, "load msg user success %s, count %d" , userfile , count ) ;
	printf( "load msg user count %d success %s\n" , count, userfile ) ;

	return true ;
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	// ȡ���û��ļ�·��
	char user_file[256] = {0} ;
	if ( ! pEnv->GetString( "user_filepath" , user_file ) ) {
		printf( "load user file failed\n" ) ;
		return false ;
	}

	int nvalue = 0 ;
	// �Ƿ��ֻ������·��
	if ( pEnv->GetInteger( "dataroute", nvalue ) ) {
		_dataroute = ( nvalue == 1 ) ;
	}

	char temp[512] = {0} ;
	// ���ػ��������б�·��
	if ( pEnv->GetString( "base_dmddir" ,temp ) ) {
		_dmddir.SetString( temp ) ;
		OUT_INFO( NULL, 0 , NULL, "Load base dmddir %s", temp ) ;
	}

	setpackspliter( &_packspliter ) ;

	return LoadMsgUser( user_file ) ;
}


void MsgClient::Stop( void )
{
	OUT_INFO("Msg",0,"MsgClient","stop");

	StopClient() ;
}

bool MsgClient::Start( void )
{
	return StartClient( "0.0.0.0", 0, 3 ) ;
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
//	FUNTRACE("void ClientAccessServer::HandleInterData(int fd, const void *data, int len)");
	if ( len < 4 ) {
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "recv data error length: %d", len ) ;
		return ;
	}

    const char *ip 	    =  sock->_szIp;
    unsigned short port =  sock->_port;

	OUT_RECV( ip, port, NULL, "on_data_arrived:[%d]%s", len, (const char*)data );

	const char *ptr = ( const char *) data ;
	if ( strncmp( ptr, "CAIT" , 4 ) == 0  ) {
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
	User user = _online_user.GetUserBySocket( sock );
	OUT_WARNING( sock->_szIp , sock->_port, user._user_id.c_str(), "Disconnection fd %d" , sock->_fd );
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
		HandleOnlineUsers( 30 ) ;

		// ����ʱ�Ķ���
		_session.CheckTimeOut( 120 ) ;
		//����ط�����ʱ��ͬ��������
		sleep(5);
	}
}

void MsgClient::NoopWork()
{

}

// ��MSG�ϴ���Ϣ
void MsgClient::HandleMsgData( const char *macid, const char *data, int len )
{
	OUT_INFO( NULL, 0, "msg", "HandleUpMsgData %s", data ) ;
	if ( _strclient == MSG_SAVE_CLIENT ) {
		if ( _dataroute ) {
			// ���ֻ���������ɾ�ֻ�������ݽӹ������������ݷ��͹�ȥ
			return ;
		}

		string val ;
		if ( ! _session.GetSession(macid,val) ) {
			OUT_ERROR( NULL, 0, macid, "Get Session Failed, Data %s" , data ) ;
			return ;
		}
		User user = _online_user.GetUserByUserId( val ) ;
		if ( user._user_id.empty() || user._fd == NULL ) {
			OUT_ERROR( NULL, 0, macid, "Get User empty , User id: %s, data %s" , val.c_str() , data ) ;
			return ;
		}
		// ��������
		SendData( user._fd, data, len ) ;
		return ;
	}

	// �㲥ģʽ
	vector<User> vec = _online_user.GetOnlineUsers() ;

	int count = vec.size() ;
	//  ֱ��Ⱥ������
	for ( int i = 0; i < count; ++ i ) {
		User &user = vec[i] ;
		if ( user._fd == NULL || user._user_state != User::ON_LINE )
			continue ;
		SendData( user._fd, data ,len ) ;
	}
}

// ������½��Ϣ����
int  MsgClient::build_login_msg(User &user, char *buf, int buf_len)
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

// �׷���½�û��Ự����
void MsgClient::HandleSession( socket_t *sock, const char *data, int len )
{
	string line = data;

	vector<string> vec_temp ;
	if ( ! splitvector( line, vec_temp, " " , 1 ) ) {
		return ;
	}

	const char *ip 	    = sock->_szIp ;
	unsigned short port = sock->_port ;

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
				if( user._user_id.empty() ) {
					OUT_WARNING(ip,port,NULL,"Can't find the syn_user");
					return;
				}
				user._user_state 		= User::ON_LINE ;
				user._last_active_time  = time(NULL) ;
				// ���´����û�״̬
				_online_user.SetUser( user._user_id, user ) ;

				OUT_CONN( ip , port, user._user_name.c_str(), "Login success, fd %d access code %d" , sock->_port, user._access_code ) ;
				// ��½�ɹ������Ϊ���ݶ������Ӿ�ֱ����Ҫ�����Ͷ��Ĵ���
				if ( user._user_type == "DMDATA" ) LoadSubscribe( user ) ;
			}
			break ;
		case -1:
			{
				OUT_ERROR(ip, port, NULL , "LACK,password error!");
			}
			break ;
		case -2:
			{
				OUT_ERROR(ip, port, NULL ,"LACK,the user has already login!");
			}
			break ;
		case -3:
			{
				OUT_ERROR(ip, port, NULL, "LACK,user name is invalid!");
			}
			break ;
		default:
			{
				OUT_ERROR( ip, port, NULL,  "unknow result" ) ;
			}
			break;
		}

		// ������ش�����ֱ�Ӵ���
		if ( ret < 0 ) {
			_tcp_handle.close_socket( sock );
		}
	}
	else if (head == "NOOP_ACK")
	{
		User user = _online_user.GetUserBySocket( sock ) ;
		user._last_active_time  = time(NULL) ;
		_online_user.SetUser( user._user_id, user ) ;

		OUT_INFO( ip, port, user._user_name.c_str() , "NOOP_ACK");
	}
	else
	{
		OUT_WARNING( ip , port , NULL, "except message:%s", (const char*)data );
	}
}

void MsgClient::HandleInnerData( socket_t *sock, const char *data, int len )
{
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	User user = _online_user.GetUserBySocket( sock ) ;
	if ( user._user_id.empty()  ) {
		OUT_ERROR( ip, port, "CAIS" , "find fd %d user failed, data %s", sock->_fd, data ) ;
		return ;
	}

	string line( data, len ) ;
	vector<string>  vec ;
	if ( ! splitvector( line, vec, " " , 6 )  ){
		OUT_ERROR( ip, port, user._user_name.c_str() , "fd %d data error: %s", sock->_fd, data ) ;
		return ;
	}

	string macid     = vec[2] ;
	// if store type need save session
	if ( _strclient == MSG_SAVE_CLIENT ) {
		_session.AddSession( macid, user._user_id ) ;
	}

	// add end split string "\r\n"
	string sdata = line + "\r\n" ;

	// ת������
	_pMsgClient->HandleMsgData( macid.c_str(), sdata.c_str(), sdata.length() ) ;

	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;
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
				OUT_INFO( user._ip.c_str() , user._port , user._user_name.c_str() , "HandleOffline Users close socket fd %d", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
		else if(user._socket_type == User::TcpConnClient)
		{
			if(user._fd != NULL )
			{
				OUT_INFO( user._ip.c_str() , user._port , user._user_name.c_str() ,"TcpConnClient close socket fd %d", user._fd->_fd );
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
		if( user._socket_type == User::TcpConnClient)
		{
			string loop = "NOOP \r\n" ;
			SendData( user._fd, loop.c_str(), loop.length() ) ;
			OUT_SEND(vec_users[i]._ip.c_str(),vec_users[i]._port,vec_users[i]._user_id.c_str(),"NOOP");
		}
	}
}

// ���ض�������
void MsgClient::LoadSubscribe( User &user )
{
	if ( _dmddir.IsEmpty() )
		return ;

	char szbuf[1024] = {0};
	sprintf( szbuf, "%s/%d", _dmddir.GetBuffer(), user._access_code ) ;

    string line;
    ifstream ifs;

	size_t prev;
	size_t next;
	size_t size;
	string value;
	string inner;
	string order;

	inner = "";
	order = "DMD";
	ifs.open(szbuf);
	while (getline(ifs, line)) {
		if ((size = line.size()) > 0 && line[size - 1] == '\r') {
			line.erase(size - 1);
		}

		if (line.empty() || line[0] == '#') {
			continue;
		}

		prev = 0;
		size = line.size();
		while (prev < size) {
			if ((next = line.find_first_of(", ", prev)) == string::npos) {
				next = size;
			}
			value = line.substr(prev, next - prev);
			prev = next + 1;

			if (value.empty()) {
				continue;
			}

			if (inner.empty()) {
				inner.assign(order + " 0 {" + value);
			} else {
				inner.append("," + value);
			}

			if (inner.size() > 4096) {
				inner.append("} \r\n");
				SendData(user._fd, inner.c_str(), inner.length());
				OUT_SEND(user._ip.c_str(), user._port, "SEND", "%s", line.c_str());
				inner = "";
				order = "ADD";
			}
		}
	}
	ifs.close();

	if (!inner.empty()) {
		inner.append("} \r\n");
		SendData(user._fd, inner.c_str(), inner.length());
		OUT_SEND(user._ip.c_str(), user._port, "SEND", "%s", line.c_str());
	}
}

