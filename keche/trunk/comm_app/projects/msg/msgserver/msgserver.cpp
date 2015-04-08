/**********************************************
 * ClientAccessServer.cpp
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/
#include "msgserver.h"
#include <comlog.h>
#include <tools.h>
#include <protocol.h>
#include <intercoder.h>
#include <netutil.h>

#define MSGONLINE 	 "msg.online"

ClientAccessServer::ClientAccessServer() :
	_session(true), _nodeid(0), _recvstat(5) , _reportstat(60)
{
	_thread_num    = 10 ;
	_user_file 	   = "" ;
	_max_timeout   = 90 ;
	_pusermgr      = NULL ;
	_enbale_save   = true ;
	_enable_plugin = false ;
}

ClientAccessServer::~ClientAccessServer( void )
{
	Stop() ;
}


bool ClientAccessServer::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	_user_file = _pEnv->GetUserPath() ;
	// ȡ���û��������
	_pusermgr  = pEnv->GetUserMgr() ;

	char szip[128] = {0} ;
	if ( ! pEnv->GetString("msg_listen_ip" , szip) ){
		OUT_ERROR( NULL, 0, NULL, "get msg_listen_ip failed" ) ;
		return false ;
	}
	_listen_ip = szip ;

	int port = 0 ;
	if ( ! pEnv->GetInteger( "msg_listen_port", port ) ){
		OUT_ERROR( NULL, 0, NULL, "get msg_listen_port failed" ) ;
		return false ;
	}
	_listen_port = port ;

	int nvalue = 10 ;
	if ( pEnv->GetInteger( "msg_tcp_thread", nvalue )  ){
		_thread_num = nvalue ;
	}

	// �ڵ�ŵĹ���
	if ( pEnv->GetInteger( "msg_node_id" , nvalue ) ) {
		_nodeid = nvalue ;
	}

	// MSG�û����ʱ��
	if ( pEnv->GetInteger( "msg_user_time" , nvalue ) ) {
		_max_timeout = nvalue ;
	}
	// �Ƿ����洢����
	if ( pEnv->GetInteger( "dbsave_enable", nvalue ) ) {
		_enbale_save = ( nvalue == 1 ) ;
	}
	// �Ƿ����������
	if ( pEnv->GetInteger( "plugin_enable", nvalue ) ) {
		_enable_plugin = ( nvalue == 1 ) ;
	}

	// ���ûص�����
	_session.SetNotify( this ) ;
	// �������ݷְ�����
	_tcp_handle.setpackspliter( &_pack_spliter ) ;

	return true ;
}

bool ClientAccessServer::Start( void )
{
	return StartServer( _listen_port , _listen_ip.c_str() , _thread_num ) ;
}

// ����STOP����
void ClientAccessServer::Stop( void )
{
	StopServer() ;
}

void ClientAccessServer::TimeWork()
{
	OUT_INFO( NULL, 0, NULL , "void ClientAccessServer::TimeWork()" ) ;
	//��������һ���¾��Ǽ����û���Ϣ�б�
	while (1) {
		if  ( ! Check() ) break ;

		string suser = "" ;
		int ncount = 0 ;

		time_t now = time(NULL) ;
		// OUT_WARNING( NULL, 0, "ONLINE" , "recv data split pack count total: %d" , g_packcount ) ;
		// printf( "recv data split pack count total: %d\n" , g_packcount ) ;
		vector<User> vec_user ;
		if ( ! _pusermgr->GetOnlineUsers( vec_user ) ) {
			OUT_RUNNING( NULL, 0, "ONLINE" , "online user: 0, car total: 0, flux 0.0kb, count: 0, total: 0, users:" ) ;
			sleep(15) ;
			continue ;
		}

		for (int i = 0; i < (int)vec_user.size(); ++ i){
			User &user = vec_user[i] ;
			if ( user._user_state != User::OFF_LINE &&
					now - user._last_active_time > _max_timeout ) {
				OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "Time out,close it , last time %d" , (now - user._last_active_time) );
				if ( user._fd != NULL ) {
					CloseSocket(user._fd);
				}
				_pusermgr->DeleteUser(user._user_id);
				continue ;
			}
			++ ncount ;

			if ( ! suser.empty() )
				suser +="," ;
			suser += user._user_id ;
			suser += "." ;
			suser += user._user_type ;
			if ( ! user._user_name.empty() ) {
				suser += ":" ;
				suser += user._user_name ;
			}
			suser += "(" + user._ip + ")" ;
		}

#ifdef _GPS_STAT
		OUT_RUNNING( NULL, 0, "ONLINE" , "online user: %d, car total: %d, flux %fkb, count: %f, total: %d, users: %s" ,
								ncount , GetOnlineSize() , _recvstat.GetFlux() / FLUX_KB ,
								_reportstat.GetFlux(), _pEnv->GetGpsCount(), suser.c_str() ) ;
#else
		OUT_RUNNING( NULL, 0, "ONLINE" , "online user: %d, car total: %d, flux %fkb, count: %f, users: %s" ,
					ncount , GetOnlineSize() , _recvstat.GetFlux() / FLUX_KB , _reportstat.GetFlux(), suser.c_str() ) ;
#endif
		sleep(15);
	}
}

void ClientAccessServer::NoopWork()
{
	OUT_INFO( NULL, 0, NULL , "void ClientAccessServer::NoopWork()" ) ;

	time_t check_session = 0 ;
	while(1) {
		if ( ! Check() ) break ;

		time_t now = time(NULL) ;
		if ( now - check_session > 30 ) {
			//���������ļ��е��û�
			_msg_user.LoadUser( _user_file.c_str() ) ;
			// �Ự��ʱʱ��Ϊ120�룬�������2����û���κ�������ʱ
			_session.CheckTimeOut( _max_timeout ) ;
			// �޸����һ�θ���ʱ��
			check_session = now ;
		}
		// �ȴ�һ���Ӵ���
		sleep(1);
	}
}

void ClientAccessServer::on_data_arrived( socket_t *sock, const void *data, int len)
{
	if ( len <= 0 ) return ;

	CInterCoder coder ;
	// �Խ��յ������ݽ��н��봦��
	if ( ! coder.Decode( (const char *)data, len ) ) {
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "fd %d, recv error data:%d,%s", sock->_fd, len ,( const char *)data ) ;
		return ;
	}

	_reportstat.AddFlux( 1 ) ;
	_recvstat.AddFlux( len ) ;

	const char *ptr = ( const char *) coder.Buffer() ;
	if ( strncmp( ptr, "LOGI" , 4 ) == 0 ||
			strncmp( ptr, "LOGO", 4 ) == 0 || strncmp( ptr, "NOOP", 4 ) == 0 ||
			strncmp( ptr, "CAIT" , 4 ) == 0 || strncmp( ptr, "DMD", 3 ) == 0 ||
			strncmp( ptr, "ADD" , 3 ) == 0 || strncmp( ptr, "UMD", 3 ) == 0 ) {
		// ������Ȳ�һ����˵��Ϊ��������
		HandleInterData( sock, coder.Buffer(),  coder.Length() , ( len != coder.Length() ) );
	}
#ifdef _GPS_STAT
	else if ( strncmp( ptr, "START", 5 ) == 0 ) {
		_pEnv->SetGpsState( true ) ;
		SendAllUser( "START \r\n", 8 ) ;
	} else if ( strncmp( ptr, "STOP", 4 ) == 0 ) {
		_pEnv->SetGpsState( false ) ;
		SendAllUser( "STOP \r\n", 7 ) ;
	}
#endif
}

void ClientAccessServer::on_dis_connection( socket_t *sock )
{
	User user ;
	if ( _pusermgr->GetUserBySocket( sock, user ) ) {
		OUT_DISCONN(user._ip.c_str(),user._port,user._user_id.c_str(),"on_dis_connection fd %d", sock->_fd );
		_pusermgr->DeleteUser(user._user_id);
	}
}

void ClientAccessServer::HandleInterData( socket_t *sock, const char *data, int len, bool flag )
{
	const char *ip    = sock->_szIp ;
	unsigned int port = sock->_port ;

	if ( len < 4 )  {
		OUT_ERROR( ip, port, "Recv" , "fd %d, recv data error, length: %d, data: %s" , sock->_fd, len , (char*)data ) ;
		return ;
	}

	int pos = len-1 ;
	// ȥ��" \r\n"�����ַ�
	while( data[pos] == '\n' || data[pos] == '\r' || data[pos] == ' ' ) {
		if ( pos <= 0 )
			break ;
		-- pos ;
	}
	string line((char *) data, pos+1 );

	OUT_RECV(ip,port,NULL,"msg recv:%s",line.c_str());

	vector<string> vec_temp ;
	splitvector( line, vec_temp, " " , 5 ) ;

	string head = vec_temp[0] ;
	if ( head == "LOGI" ){
		if (vec_temp.size() < 4)
			return ;

		string user_type   = vec_temp[1];
		string user_id     = vec_temp[2];
		string user_passwd = vec_temp[3];
		if ( user_type.empty() || user_id.empty() || user_passwd.empty() ) {
			OUT_ERROR( ip, port, NULL, "fd %d Login param error, data: %s" , sock->_fd, line.c_str() ) ;
			return ;
		}
		if ( user_type.at(0) == 'U' && ! user_type.empty() ) {
			user_type = user_type.substr(1) ;
		}

		string user_group = "" ;
		size_t pos = user_type.find( ':' ) ;
		if ( pos != string::npos ) {
			// ���Ӷ������չ����ΪSAVE:GROUPNAME
			user_group = user_type.substr( pos + 1 ) ;
			user_type  = user_type.substr( 0, pos  ) ;
		}

		if( user_type != WEB_TYPE && user_type != COMPANY_TYPE && user_type != STORAGE_TYPE && user_type != SEND_TYPE ){
			OUT_WARNING(ip,port,user_id.c_str(),"Inter Login, user_id=%s type=%s , fd %d invlid!",
					user_id.c_str(),user_type.c_str(), sock->_fd );
			CloseSocket( sock ) ;
			return;
		}

		string shashval = user_id ;
		// �������������Ĵ���
		pos = shashval.find( ":" ) ;
		if ( pos != string::npos ) {
			shashval = shashval.substr( 0, pos ) ;
		}
		shashval = shashval + user_type ;
		if ( ! user_group.empty() ) {
			shashval += ":" ;
			shashval += user_group ;
		}
		// �����û�������һ��HASH����
		unsigned int accesscode = _pusermgr->GetHash( shashval.c_str(), shashval.length() ) ;
		unsigned int msgflag    = ( (flag) ? MSG_USER_ENCODE : 0x0000 ) ;  // Ĭ���Ƿ�Ϊ���ܴ���

		if ( vec_temp.size() == 5 ) {
			string sdemand = vec_temp[4] ;
			if ( ! sdemand.empty() ) {
				msgflag |= ( ( strcmp( sdemand.c_str(), MSG_DATA_DM ) == 0 ) ? MSG_USER_DEMAND : 0x0000 ) ;
			}
		}

		User olduser ;
		// ����û��Ƿ����
		if (_pusermgr->GetUserByUserId( user_id, olduser ) ){
			if ( olduser._fd == sock ) {
				olduser._fd               = sock;
				olduser._user_id          = user_id;
				olduser._ip               = ip ;
				olduser._login_time       = time(0);
				olduser._last_active_time = time(0);
				olduser._access_code 	  = accesscode ;
				olduser._msg_seq		  = msgflag ;

				char loginresp[128] = {0} ;
				if ( _nodeid > 0 ) {
					// ���Ϊ��Ⱥ����Ҫ��ӽ��ŵĴ���
					sprintf( loginresp, "LACK 0 0 0 %d \r\n", _nodeid ) ;
				} else {
					sprintf( loginresp, "LACK 0 0 0 \r\n" ) ;
				}
				if ( !SendData( sock, loginresp, strlen(loginresp))) {
					OUT_ERROR( ip, port, user_id.c_str(), "Inter Login, Close socket fd %d", sock->_fd ) ;
					CloseSocket(sock) ;
				}else{
					OUT_WARNING(ip,port,user_id.c_str(),"Inter Login, fd %d user_type=%s", sock->_fd , user_type.c_str() );
				}

				// ����д������ݶ��Ĺ�ϵ����Ҫ����״̬
				olduser._user_state = User::ON_LINE;
				_pusermgr->SetUser( user_id, olduser ) ;

				return ;
			}
			OUT_WARNING(ip,port,user_id.c_str(),"Inter Login, user_id=%s user_type=%s, fd %d already login!",
					user_id.c_str(),user_type.c_str(), sock->_fd );
			string s = "LACK -2 \r\n";
			SendData( sock, s.c_str(), s.length());
			CloseSocket( sock ) ;
			return ;
		} else {
			string userid = user_id ;
			pos = userid.find( ":" ) ;
			if ( pos != string::npos ) {
				userid = userid.substr( 0, pos ) ;
			}

			int ret = _msg_user.CheckUser( userid.c_str(), user_passwd.c_str() ) ;
			if(ret == -1){
				OUT_ERROR(ip,port,user_id.c_str(),"Inter Login,user_id=%s,type=%s,password=%s,fd %d user_id invalid!",
						user_id.c_str(),user_type.c_str(),user_passwd.c_str(), sock->_fd );
				string s = "LACK -4 \r\n";
				SendData( sock , s.c_str(), s.length());
				CloseSocket( sock ) ;
				return;
			} else if(ret == -2) {
				OUT_ERROR(ip,port,user_id.c_str(),"Inter Login,user_id=%s,type=%s,password=%s, fd %d user_passwd invalid!",
											user_id.c_str(),user_type.c_str(),user_passwd.c_str(), sock->_fd );
				string s = "LACK -1 \r\n";
				SendData( sock, s.c_str(), s.length());
				CloseSocket( sock ) ;
				return;
			}

			User new_user;
			new_user._fd               = sock;
			new_user._user_id          = user_id;
			new_user._ip               = ip ;
			new_user._login_time       = time(0);
			new_user._user_type        = user_type;
			// new_user._user_state       = User::DISABLED;  // ��ʱ����Ϊ����״̬,��Ҫ��ֹ�������յ����ݣ����յ�Ӧ��
			new_user._last_active_time = time(0);
			new_user._user_name		   = user_group ;  // ���Ϊ������û���¼����������
			new_user._access_code      = accesscode ;
			new_user._msg_seq		   = msgflag ;

			//˵����¼������ʡ����������¼
			if ( !_pusermgr->AddUser( user_id, new_user ) ) {
				OUT_ERROR(ip,port,user_id.c_str(),"Inter Login,user_id=%s,type=%s,password=%s,fd %d already login!",
																user_id.c_str(),user_type.c_str(),user_passwd.c_str(), sock->_fd );
				string s = "LACK -2 \r\n";
				SendData( sock, s.c_str(), s.length());
				CloseSocket( sock ) ;
				return;
			}

			OUT_CONN(ip,port,user_id.c_str(),"Inter Login success, user_id=%s user_type=%s, fd %d",
					user_id.c_str(), user_type.c_str(), sock->_fd );

			char loginresp[128] = {0} ;
			if ( _nodeid > 0 ) {
				// ���Ϊ��Ⱥ����Ҫ��ӽ��ŵĴ���
				sprintf( loginresp, "LACK 0 0 0 %d \r\n", _nodeid ) ;
			} else {
				sprintf( loginresp, "LACK 0 0 0 \r\n" ) ;
			}

			if ( ! SendData( sock, loginresp, strlen(loginresp) ) ) {
				_pusermgr->DeleteUser(user_id); // �ͻ���tcp��������

				OUT_ERROR( ip , port, user_id.c_str(), "Send Data failed, close fd %d", sock->_fd ) ;
				CloseSocket( sock ) ;

				return;
			}
			new_user._user_state = User::ON_LINE ;  // ����Ϊ����״̬
			_pusermgr->SetUser( user_id, new_user ) ;  // ���û�״̬
		}
	}
	else//�ǵ�¼����Ϊһ���������崦��
	{
		User user ;
		if (_pusermgr->GetUserBySocket( sock , user) ) {
			user._last_active_time = time(0);
			_pusermgr->SetUser(user._user_id,user);
		} else if ( head != "CAITS" && head != "NOOP" && head != "CAITR"  && head != "DMD" && head != "ADD" && head != "UMD" ) {
			//δ��¼��
			OUT_WARNING(ip,port,NULL,"Non-Login Message , fd %d", sock->_fd );
			CloseSocket(sock);
			return ;
		}

		if (head == "NOOP") {
			string r = "NOOP_ACK \r\n";
			if ( _tcp_handle.deliver_data( sock, (void*) r.c_str(), r.length()) ){
				OUT_SEND( ip, port, user._user_id.c_str(), "NOOP_ACK" );
			}else{
				OUT_ERROR( ip, port, user._user_id.c_str(), "NOOP_ACK" ) ;
			}
		} else if ( head == "DMD" || head == "ADD" || head == "UMD" ) {
			if ( vec_temp.size() < 3 )
				return ;

			unsigned int cmd   = OP_SUBSCRIBE_DMD ;
			unsigned int group = atoi( vec_temp[1].c_str() ) ;
			if ( head == "DMD" ) {
				cmd = OP_SUBSCRIBE_DMD ;
			} else if ( head == "ADD" ) {
				cmd = OP_SUBSCRIBE_ADD ;
			} else if ( head == "UMD" ) {
				cmd = OP_SUBSCRIBE_UMD ;
			}
			// �����ĳ��������Ϣ����
			_pEnv->GetPublisher()->OnDemand( cmd, group, vec_temp[2].c_str(), user ) ;

		} else if (head == "CAITS" || head == "CAITR") {

			string &packdata = vec_temp[5] ;
			if( vec_temp.size() < 6 || packdata.empty() )
				return ;
			// OUT_RECV(NULL,0,user._user_id.c_str(),"message:%s",line.c_str());
			if ( packdata.at( packdata.length()-1 ) != '}' ){
				OUT_ERROR( user._ip.c_str() , user._port , user._user_id.c_str(), "fd %d, Find end split data failed, %s",
						sock->_fd, packdata.c_str() ) ;
				return ;
			}

			InterData idata ;
			idata._transtype = head ;
			idata._seqid		= vec_temp[1];
			idata._macid 		= vec_temp[2];
			idata._cmtype       = vec_temp[3];
			idata._command 		= vec_temp[4];
			idata._packdata     = packdata;

			if ( idata._macid.empty() || idata._macid.size() < 5 || idata._packdata.empty() ) {
				OUT_ERROR(user._ip.c_str() , user._port , user._user_id.c_str(), "Mac id length error %s" , idata._macid.c_str() ) ;
				return ;
			}

			int cmd = SAVE_SEND_CMD ;
			if (idata._command.substr(0, 1) == "U") {
				line += " \r\n";
				// ��Ϊǰ�û���MSG�ڲ�������֪ͨ
				if ( idata._command != "U_CONN" ) {
					// ��������
					DispatchData( user, cmd , idata ) ;
				}
				// �����Ϊʧ�ܣ��򱣴��û��Ự
				if ( strncmp( idata._macid.c_str() , "0000" , 4 ) != 0
						&& idata._macid.length() > 11 && user._user_type == COMPANY_TYPE)
					_session.AddSession( idata._macid, user._user_id ) ;

#ifdef _GPS_STAT
				// ���ͳ�Ƽ�����
				if ( idata._seqid == "0_1" ) {
					_pEnv->AddGpsCount( 1 ) ;
				}
#endif
			}
			else if (idata._command.substr(0, 1) == "D")
			{
				if ( idata._command == "D_CONN" ) {
					// �յ�ǰ�û�������֪ͨ������������ڲ������
					_session.RemoveSession( idata._macid ) ;
				}
				else
				{
					line += " \r\n";
					// OUT_SEND( user._ip.c_str() , user._port , "storage","%s",line.c_str());

					string userid ;
					if ( _session.GetSession( idata._macid , userid ) && head != "CAITR" && userid != user._user_id ) {
						// ���﷢����ͨ���Ự����������ģ��϶���Ψһ��
						SendDataToUser( line.c_str(), line.length() , userid );
					}
					// ��������
					DispatchData( user, cmd , idata );
				}
			}
			else if (idata._command.substr(0, 1) == "L")
			{
				// ��������
				DispatchData( user, cmd , idata );
			}
		}
	}
}

// �׷����ݴ���
void ClientAccessServer::DispatchData( User &user, unsigned int cmd, InterData &data )
{
	// ��������
	_pEnv->GetPublisher()->Publish( data, cmd, user ) ;
#ifdef _HAVE_SAVE
	if ( _enbale_save ) {
		// �����ݷ׷����洢����
		_pEnv->GetMsgHandler(MSG_MSGPROC)->Process( data , user ) ;
	}
#endif

#ifdef _HAVE_PLUG
	// ���͸�������⴦����Ҫ����˦�ҵĴ���
	if ( _enable_plugin && ( data._command == "U_REPT"  /*|| data._command == "D_SETP"*/) ) {
		// {TYPE:9,VALUE:....}
		if ( data._packdata.length() < 8 )
			return ;

		// �������Ϊ͸��Э����Ҫ�ݽ��ĵ����������������
		if ( strstr( data._packdata.c_str(), "TYPE:9") != NULL ) {
			_pEnv->GetMsgHandler(MSG_PLUGIN)->Process( data, user ) ;
		}
	}
#endif
}

// �����û����߻�����֪ͨ
void ClientAccessServer::NotifyChange( const char *key, const char *val , const int op )
{
	//CAITS 0_0 MACID 0 U_REPT {TYPE:5,18:������״̬/ǰ�û�������ID/�ܵ����Ʒ���ID/��Ϣ������id}
	InterData idata ;
	idata._transtype = "CAITS" ;
	idata._seqid     = "0_0" ;
	idata._macid     = key  ;
	idata._cmtype    = "0" ;
	idata._command   = "U_REPT" ;

	char buf[512] = {0};
	switch( op )
	{
	case SESSION_ADDED:
		{
			char sznode[128] = {0} ;
			sprintf( sznode, "%d", _nodeid ) ;
			// ���������ȫ�ֵ�Redis������
			_pEnv->GetRedisCache()->HSet( MSGONLINE , key, sznode ) ;
			// �齨������֪ͨ
			sprintf( buf, "{TYPE:5,18:1/0/0/%d}", _nodeid ) ;
		}
		break ;
	case SESSION_REMOVE:
		{
			// �ӻ������Ƴ�����
			_pEnv->GetRedisCache()->HDel( MSGONLINE, key ) ;
			// �齨����֪ͨ
			sprintf( buf, "{TYPE:5,18:0/0/0/%d}", _nodeid ) ;
		}
		break ;
	}
	idata._packdata = buf ;

	// �����Ҫ����֪ͨ
	if ( op ) {
		User user ;
		// ����֪ͨ������֪ͨ��������
		OUT_PRINT( NULL, 0 , key, "%s Notify Change:%s", key, buf ) ;
		// �������ݣ����͵�WEB�ʹ洢ͨ��������
		DispatchData( user, UWEB_SEND_CMD | SAVE_SEND_CMD|SEND_SEND_CMD , idata ) ;
	}
}

bool ClientAccessServer::HasLogin(const string &user_id)
{
	User user ;
	if ( !_pusermgr->GetUserByUserId(user_id, user ) )
		return false ;
	return  true;
}

// ��������
bool ClientAccessServer::DeliverEx( const char *userid, const char *data, int len )
{
	int pos = len-1 ;
	// ȥ��" \r\n"�����ַ�
	while( data[pos] == '\n' || data[pos] == '\r' || data[pos] == ' ' ) {
		if ( pos <= 0 )
			break ;
		-- pos ;
	}
	string line((char *) data, pos+1 );

	vector<string> vec_temp ;
	splitvector( line, vec_temp, " " , 5 ) ;

	string &packdata = vec_temp[5] ;
	if( vec_temp.size() < 6 || packdata.empty() )
		return false;
	// OUT_RECV(NULL,0,user._user_id.c_str(),"message:%s",line.c_str());
	if ( packdata.at( packdata.length()-1 ) != '}' ){
		OUT_ERROR( NULL , 0, NULL, "Find end split data failed, %s", packdata.c_str() ) ;
		return false;
	}

	InterData idata ;
	idata._transtype 	= vec_temp[0] ;
	idata._seqid		= vec_temp[1] ;
	idata._macid 		= vec_temp[2] ;
	idata._cmtype       = vec_temp[3] ;
	idata._command 		= vec_temp[4] ;
	idata._packdata     = packdata;

	if ( idata._macid.empty() || idata._macid.size() < 5 || idata._packdata.empty() ) {
		OUT_ERROR( NULL , 0 , userid , "Mac id length error %s" , idata._macid.c_str() ) ;
		return false ;
	}

	string uid ;
	// �����uid��ֱ���·�
	if ( userid != NULL ) {
		uid = userid ;
	} else {
		// �ӻỰ��ȡ������MAC��Ӧ�ĻỰ�����û�оͼ�¼��־
		if ( ! _session.GetSession( idata._macid.c_str() , uid ) ) {
			OUT_ERROR( NULL, 0, idata._macid.c_str() , "Get mac id %s Session Failed, %s" , idata._macid.c_str(),  data ) ;
			return false ;
		}
	}

	// �������ݵ��û�
	if ( ! SendDataToUser( data, len , uid.c_str() ) ) {
		return false ;
	}

	User user ;
	// �������ݣ����͵�WEB�ʹ洢ͨ��������
	DispatchData( user, SAVE_SEND_CMD , idata ) ;

	return true ;
}

bool ClientAccessServer::SendDataToUser(const char *data, int len, const string &userid)
{
	// FUNTRACE("bool ClientAccessServer::SendDataToUser(void *data, int len, string userid)");
	if (data == NULL || len < 0 || userid.empty())
		return false;

	User user ;
	if ( !_pusermgr->GetUserByUserId(userid, user) ) {
		// �������ʧ�ܼ�¼������־
		OUT_ERROR( user._ip.c_str() , user._port , user._user_id.c_str()  ,"Failed send data to user %s, data: %s", userid.c_str() , data );
		return false ;
	}

	// �û�������
	if( user._user_state != User::ON_LINE || user._fd == NULL ) {
		// �������ʧ�ܼ�¼������־
		OUT_ERROR( user._ip.c_str() , user._port , user._user_id.c_str()  ,"Failed send data to user %s, data: %s", userid.c_str() , data );
		return false ;
	}
	// ����ʧ�ܾ�ֱ�ӶϿ�������
	if ( ! SendUserData( user ,data, len ) ) {
		OUT_ERROR( user._ip.c_str(), user._port, user._user_id.c_str(), "Send Data failed, close fd %d", user._fd ) ;
		CloseSocket( user._fd ) ;
		return false ;
	}
	return true ;
}

// ���͵������û�
bool ClientAccessServer::SendAllUser( const char *data, int len )
{
	vector<User> vec ;
	if ( !_pusermgr->GetOnlineUsers( vec ) ) {
		OUT_ERROR( NULL, 0, "Msg" , "Send all user : %s", data ) ;
		return false;
	}

	int size = vec.size() ;
	// ֪ͨ�����û�
	for( int i = 0; i < size; ++ i ) {
		User &user = vec[i] ;
		if ( user._fd == NULL )
			continue ;
		SendUserData( user , data, len ) ;
	}

	return true ;
}

// �����û�����
bool ClientAccessServer::SendUserData( User &user, const char *data, int len )
{
	if ( user._fd == NULL )
		return false ;

	// ����û���������
	if ( user._msg_seq & MSG_USER_ENCODE ) {
		CInterCoder coder ;
		coder.Encode( data, len ) ;
		return SendData( user._fd, coder.Buffer(), coder.Length() ) ;
	}
	return SendData( user._fd, data, len ) ;
}

// ��������
bool ClientAccessServer::Deliver( socket_t *sock, const char *data, int len )
{
	return SendData( sock, data, len ) ;
}

// �ر�����
void ClientAccessServer::Close( socket_t *sock )
{
	CloseSocket( sock ) ;
}

// ȡ���ڳ�������ѹ��
int ClientAccessServer::GetOnlineSize( void )
{
	return  _session.GetSize() ;
}

// ��������û��б�
void ClientAccessServer::AddNodeUser( const char *user, const char *pwd )
{
	_msg_user.AddUser( user, pwd ) ;
}

