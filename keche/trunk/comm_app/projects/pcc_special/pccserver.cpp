/*
 * pccserver.cpp
 *
 *  Created on: 2011-11-28
 *      Author: humingqing
 *      ����������Դ�����֣�һ���������ڸ�ʡƽ̨���������ݣ�һ���������ڲ�����������
 *      ʡƽ̨����������Ҫ��������ƽ̨��������ҵƽ̨�ڲ�������������Ҫ����ʡƽ̨����
 */

#include "pccserver.h"
#include <comlog.h>
#include <crc16.h>
#include <netutil.h>

#define REDIS_PCCKEY "pcc.list"

CPccServer::CPccServer()
{
	_thread_num = 15 ;
	_areaids    = "";
}

CPccServer::~CPccServer()
{
	Stop() ;
}

// ��ʼ��
bool CPccServer::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char value[1024] = {0} ;
	if ( ! _pEnv->GetString("pcc_listen_ip", value) )
	{
		printf("get pcc_listen_ip failed!\n");
		return false ;
	}

	_listen_ip = value ;

	int port = 0 ;
	if ( !_pEnv->GetInteger("pcc_listen_port", port ) )
	{
		printf("get pcc_listen_port failed!\n");
		return false ;
	}
	_listen_port = port ;


	int thread = 0 ;
	if ( ! _pEnv->GetInteger("pcc_recv_thread" , thread ) ){
		thread = 10 ;
	}
	_thread_num = thread ;

	return true ;
}

// ��ʼ�߳�
bool CPccServer::Start( void )
{
	return StartServer( _listen_port, _listen_ip.c_str() , _thread_num ) ;
}

// ֹͣ����
void CPccServer::Stop( void )
{
	StopServer() ;
}

// ʵ�ַ������Ľӿ�
void CPccServer::on_data_arrived( socket_t *sock, const void* data, int len )
{
	if ( len <= 0 || data == NULL ){
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "fd %d , data error length %d" , sock->_fd ,len ) ;
		OUT_HEX( sock->_szIp, sock->_port, "PccServer" , (const char *)data, len ) ;
		return ;
	}

	const char *ptr = (const char *)data ;
	if ( *ptr == '[' ) { //0x5B  ��ͷ
		// ���809��Ҫ���봦��
		C5BCoder coder ;
		if ( ! coder.Decode( (const char *)data, len ) ) {
			OUT_WARNING( sock->_szIp, sock->_port, NULL, "Except packet header or tail" ) ;
			OUT_HEX( sock->_szIp, sock->_port, NULL, (const char *) data, len ) ;
			return;
		}
		// �׷���������
		HandleOutData( sock, coder.GetData(), coder.GetSize() );
	}else{
		// �����ڲ�����
		HandleInnerData( sock, (const char *)data, len );//MCS->PCC
	}
}

// ����809Э�������
void CPccServer::HandleOutData( socket_t *sock, const char *data, int len )
{
	// ����809
	if (data == NULL || len < (int)sizeof(Header)) {
		OUT_HEX( sock->_szIp, sock->_port, "PccServer" , (const char *)data, len ) ;
		return;
	}

	Header *header = (Header *) data;
	unsigned int access_code = ntouv32(header->access_code);
	string str_access_code = uitodecstr(access_code);

	unsigned short msg_type = ntouv16(header->msg_type);

	const char *ip    = sock->_szIp ;
	unsigned int port = sock->_port ;

	// ����ӽ�������
	EncryptData( (unsigned char*) data , len , false ) ;

	OUT_RECV( ip, port, str_access_code.c_str(), "%s,from pccserver", _proto_parse.Decoder(data,len).c_str() ) ;
	OUT_HEX( ip, port, str_access_code.c_str(), data, len ) ;

	if (msg_type == DOWN_CONNECT_REQ){
		// ��������
		User user = _online_user.GetUserByUserId(str_access_code);
		if ( user._fd != NULL && user._fd != sock ) {
			CloseSocket(user._fd) ;
		}

		DownConnectReq* req = (DownConnectReq*) data;

		user._fd          = sock;
		user._ip          = ip ;
		user._port        = port ;
		user._login_time  = time(0);
		user._msg_seq     = 0;
		user._user_id     = str_access_code ;
		user._access_code = access_code ;
		user._user_state  = User::ON_LINE;

		//һ��Ҫ�����������Ϊ��������·��û�н���������ʱ�򣬵�һ�ε�¼��ʱ���ϵ��Ǹ��ǲ���ִ�еġ�
		user._last_active_time = time(0);

		// ����û�
		if ( ! _online_user.AddUser( str_access_code, user ) ){
			_online_user.SetUser( str_access_code, user  ) ;
		}

		DownConnectRsp resp;
		resp.header.msg_seq 	= ntouv16(_proto_parse.get_next_seq());
		resp.header.access_code = req->header.access_code;
		resp.result = 0;

		if ( SendCrcData( sock,(const char *)&resp,sizeof(resp) ) ) {
			OUT_INFO(ip,port,str_access_code.c_str(),"DOWN_CONNECT_REQ: send DOWN_CONNECT_RSP downlink is online");
		} else {
			OUT_ERROR(ip,port,str_access_code.c_str(),"DOWN_CONNECT_REQ: send DOWN_CONNECT_RSP downlink is online failed");
		}
		// ��������״̬
		_pEnv->GetPasClient()->UpdateSlaveConn( access_code , CONN_CONNECT ) ;

		return;
	}

	User user = _online_user.GetUserBySocket( sock );

	if (user._user_id.empty()){
		OUT_ERROR(ip,port,str_access_code.c_str(),"msg type %04x, user havn't login,close it %d", msg_type, sock->_fd );
		CloseSocket( sock );
		return;
	}

	// ���Ϊ����
	if (msg_type == DOWN_LINKTEST_REQ){
		OUT_RECV(ip,port,user._user_id.c_str(),"DOWN_LINKTEST_REQ");
		DownLinkTestRsp resp;
		resp.header.access_code = header->access_code;
		resp.header.msg_seq 	= ntouv32(_proto_parse.get_next_seq());

		if ( SendCrcData( sock, (const char*) &resp, sizeof(resp) ) )
			OUT_SEND(ip,port,user._user_id.c_str(),"DOWN_LINKTEST_RSP");
		else
			OUT_ERROR(ip,port,user._user_id.c_str(),"DOWN_LINKTEST_RSP") ;
	}
	else if (msg_type == DOWN_DISCONNECT_REQ ) {  // ����·ע������
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));

		DownDisconnectRsp resp ;
		resp.header.msg_len     = ntouv32( sizeof(resp) ) ;
		resp.header.msg_type    = ntouv16( DOWN_DISCONNECT_RSP ) ;
		resp.header.access_code = header->access_code ;
		resp.header.msg_seq     = ntouv32( _proto_parse.get_next_seq() ) ;

		if ( SendCrcData( sock, (const char*) &resp, sizeof(resp) ) )
			OUT_SEND(ip,port,user._user_id.c_str(),"DOWN_DISCONNECT_RSP");
		else
			OUT_ERROR(ip,port,user._user_id.c_str(),"DOWN_DISCONNECT_RSP") ;

		// ����������·�������ر����ӣ�������״̬���߳��Զ�������·
		// user._user_state = User::OFF_LINE ;
	}
	else if (msg_type == DOWN_DISCONNECT_INFORM){ // ����·��Ϣ
		//���������ɶ�ʱ�߳�����ɡ�
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));
	}
	else if (msg_type == DOWN_CLOSELINK_INFORM ){ // ��������·

	}
	else{
		// ֱ�ӵ�CLIENT����
		_pEnv->GetPasClient()->HandlePasDownData( access_code, data, len ) ;
	}

	user._last_active_time = time(0);
	_online_user.SetUser(user._user_id,user);
}

// �����ڲ��·�����
void CPccServer::HandleInnerData( socket_t *sock, const char *data, int len )
{
	if ( len < 4 )  return ;

	const char *ip    =  sock->_szIp;
	unsigned int port =  sock->_port;

	OUT_RECV(ip,port,NULL,"%s",(const char*)data);

	string line( data, len - 2 ) ;
	string head = line.substr(0, 4);

	vector<string> vec_temp ;
	splitvector(line, vec_temp, " ", 0 );

	if (head == "LOGI") {
		if (vec_temp.size() < 4)
			return ;

		string user_type   = vec_temp[1];
		string user_id     = vec_temp[2];
		string user_passwd = vec_temp[3];
		if ( user_type.empty() || user_id.empty() || user_passwd.empty() ) {
			return ;
		}
		if ( user_type.at(0) == 'U' && ! user_type.empty() ) {
			user_type = user_type.substr(1) ;
		}

		string user_group = "" ;
		size_t pos = user_type.find( ':' ) ;
		if ( pos != string::npos ) {
			// ���Ӷ������չ����ΪSAVE:GROUPNAME
			user_group = user_type.substr(pos + 1) ;
			user_type  = user_type.substr(0, pos) ;
		}

		if(user_type != WEB_TYPE && user_type != COMPANY_TYPE && user_type != STORAGE_TYPE && user_type != ROUTE_TYPE ){
			OUT_WARNING(ip,port,user_id.c_str(),"Inter Login, user_id=%s type=%s invlid!",user_id.c_str(),user_type.c_str());
			CloseSocket( sock ) ;
			return;
		}

		User user = _online_user.GetUserByUserId(user_id) ;
		if ( ! user._user_id.empty() ) {
			OUT_WARNING(ip,port,user_id.c_str(),"Inter Login, user_id=%s user_type=%s,already login!",user_id.c_str(),user_type.c_str());
			string s = "LACK -2 \r\n";
			SendData( sock, s.c_str(), s.length() ) ;
			return ;
		}
		else {

			User new_user;
			new_user._fd               = sock;
			new_user._user_id          = user_id;
			new_user._ip               = ip ;
			new_user._port			   = port ;
			new_user._login_time       = time(0);
			new_user._user_type        = user_type;
			new_user._user_state       = User::ON_LINE;
			new_user._last_active_time = time(0);

			//˵����¼������ʡ����������¼
			_online_user.AddUser( user_id, new_user ) ;

			OUT_CONN(ip,port,user_id.c_str(),"Inter Login success, user_id=%s user_type=%s",user_id.c_str(),user_type.c_str());

			string sresp = "LACK 0 0 0 \r\n" ;
			if (  user_type == ROUTE_TYPE ) {
				_mutex.lock() ;
				if( !_areaids.empty() ) {
					sresp += _areaids ;
				}
				_mutex.unlock() ;
			}
			SendData( sock,  sresp.c_str() , sresp.length() );
			OUT_PRINT( ip, port, user_id.c_str(), "Send area code %s", sresp.c_str() ) ;
		}
	}
	else//�ǵ�¼����Ϊһ���������崦��
	{
		User user =  _online_user.GetUserBySocket(sock);
		user._last_active_time = time(0);
		if(user._user_id.empty()){
			//δ��¼��
			OUT_WARNING( ip, port, NULL, "fd %d, Non-Login Message,close it" , sock->_fd );
			CloseSocket( sock );
			return ;
		}
		if (head == "LOGO"){

		}
		else if (head == "NOOP"){
			string r = "NOOP_ACK \r\n";
			if ( _tcp_handle.deliver_data( sock, (void*) r.c_str(), r.length()) ){
				OUT_SEND( ip, port, user._user_id.c_str(), "NOOP_ACK" );
			}else{
				OUT_ERROR( ip, port, user._user_id.c_str(), "NOOP_ACK" ) ;
			}
		}
		else if ( head == "CAIT" ){
			// ��Ҫֱ�ӽ�������������
			((IMsgClient*)_pEnv->GetMsgClient())->HandleUserData( user, line.c_str() , line.length());
		}
		else if ( head == "DISC" ) {
			// DISCON 6401 0  | DISCON 6401 1
			if ( vec_temp.size() < 3 ) {
				return ;
			}

			int nflag = PAS_USERLINK_ONLINE ;
			switch( atoi( vec_temp[2].c_str() ) ) {
			case 0:  // ����·�Ͽ�����
				nflag = PAS_MAINLINK_LOGOUT ;
				break ;
			case 1:  // ����������������
				nflag = PAS_USERLINK_ONLINE ;
				break ;
			case 2: // ����·�쳣����Ĵ���
				nflag = PAS_SUBLINK_ERROR ;
				break ;
			case 3: // �����쳣�����
				nflag = PAS_MAINLINK_ERROR ;
				break ;
			}
			// �Ƿ�����·������
			_pEnv->GetPasClient()->Enable( atoi( vec_temp[1].c_str() ) , nflag ) ;

			// ��Ӧ���ڲ�Э��
			string r = "DISCON_ACK "+vec_temp[1]+ " " + vec_temp[2] + " 0 \r\n" ;
			if ( _tcp_handle.deliver_data( sock, (void*) r.c_str(), r.length()) ){
				OUT_SEND( ip, port, user._user_id.c_str(), "DISCON_ACK" );
			}else{
				OUT_ERROR( ip, port, user._user_id.c_str(), "DISCON_ACK" ) ;
			}
		}
		else if ( head == "CLRC" ) {
			//  CLRC mac_id(OEM_�ֻ���,��ɫ_����)
			if ( vec_temp.size() == 1 ) {
				// �������л���
				_pEnv->ClearSession( NULL ) ;
			}else if( vec_temp.size()> 1 ){
				// ���ָ���ĳ����ĵĻ���
				_pEnv->ClearSession( vec_temp[1].c_str() ) ;
			}
		}
		_online_user.SetUser(user._user_id,user);
	}
}

void CPccServer::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
	if ( user._user_id.empty() ) {
		return ;
	}
	// �������·������
	if ( user._access_code > 0 ) {
		// ��������״̬
		_pEnv->GetPasClient()->UpdateSlaveConn( user._access_code , CONN_DISCONN ) ;
	}
	OUT_WARNING( sock->_szIp, sock->_port, user._user_id.c_str(), "CPccServer Disconnection fd %d" , sock->_fd );
	_online_user.DeleteUser( sock );
}

// �����̺߳Ͷ�ʱ�߳�
void CPccServer::TimeWork()
{
	//����ʱ���ʹ��
	while (1){
		if ( ! Check() )  break ;
		// ����ʱ����������
		HandleOfflineUsers();

		sleep(1);
	}
}

void CPccServer::NoopWork()
{
	sleep(3) ;

	time_t last_check = 0 ;
	while( 1 ) {
		if ( ! Check() ) break ;
		// ����PCC���б�
		time_t now = time(NULL) ;
		if ( now - last_check > 180 ) {
			last_check = now ;
			UpdatePcc() ;
		}
		sleep(1) ;
	}
}

// ���͹ر���������, msgid:UP_DISCONNECT_INFORM,UP_CLOSELINK_INFORM
void CPccServer::Close( int accesscode , unsigned short msgid, int reason )
{
	OUT_PRINT( NULL, 0, NULL, "close accesscode %d down link, msgid %d, reason %d" , accesscode, msgid, reason ) ;

	char uid[128] = {0};
	sprintf( uid, "%d", accesscode ) ;

	User user = _online_user.GetUserByUserId( uid ) ;
	if ( user._user_id.empty() ) {
		return ;
	}

	switch( msgid ) {
	case UP_CLOSELINK_INFORM:  // �ر�������·
	case UP_DISCONNECT_INFORM: // �ر�����·
		{
			UpDisconnectInform req ;
			req.header.msg_len 		= ntouv32( sizeof(req) ) ;
			req.header.msg_seq 		= ntouv32( _proto_parse.get_next_seq() ) ;
			req.header.access_code  = ntouv32( accesscode ) ;
			req.header.msg_type		= ntouv16( msgid ) ;
			req.error_code			= reason ;
			// ���Ͷ�������
			SendCrcData( user._fd, (const char *)&req, sizeof(req) ) ;
			/**
			if ( msgid == UP_CLOSELINK_INFORM ) {
				// �ر�����·������
				_pEnv->GetPasClient()->Close( accesscode ) ;
			}*/
			OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "Send %s",
					(msgid == UP_CLOSELINK_INFORM) ? "UP_CLOSELINK_INFORM" : "UP_DISCONNECT_INFORM" ) ;
		}
		break ;
	default:  // �Ͽ����Ӵ���
		{
			user._user_state = User::OFF_LINE ;
			_online_user.SetUser( uid, user ) ;

			OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "Close Sublink set user state offline" ) ;
		}
		break ;
	}
}

void CPccServer::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(MAX_TIMEOUT);

	// �����û���������
	for(int i = 0; i < (int)vec_users.size(); ++i){
		User &user = vec_users[i];
		if(user._socket_type == User::TcpClient){
			if(user._fd != NULL ){
				OUT_WARNING( user._ip.c_str() , user._port , user._user_id.c_str() ,
						"CPccServer close socket fd %d", user._fd->_fd );
				CloseSocket(user._fd);
			}
		}
	}
}

bool CPccServer::SendCrcData( socket_t *sock, const char* data, int len )
{
	if ( sock == NULL )
		return false ;

	// ����ѭ����
	char *buf = new char[len+1] ;
	memset( buf, 0 , len+1 ) ;
	memcpy( buf, data, len ) ;

	// ����ӽ�������
	EncryptData( (unsigned char*) buf , len , true ) ;

	// ͳһ����ѭ�������֤
	unsigned short crc_code = ntouv16( GetCrcCode( buf, len ) ) ;
	unsigned int   offset   = len - sizeof(Footer) ;
	// �滻ѭ�����ڴ��λ������
	memcpy( buf + offset , &crc_code, sizeof(short) ) ;

	C5BCoder coder ;
	coder.Encode( buf , len ) ;

	delete [] buf ;

	return SendData( sock, coder.GetData(), coder.GetSize() ) ;
}

// ���ܴ�������
bool CPccServer::EncryptData( unsigned char *data, unsigned int len , bool encode )
{
	if ( len < sizeof(Header) )
		return false ;

	Header *header = ( Header *) data ;
	// �Ƿ���Ҫ���ܴ���
	if ( ! header->encrypt_flag && ! encode ) {
		return false;
	}

	int M1 = 0, IA1 = 0 , IC1 = 0 ;
	int accesscode = ntouv32( header->access_code ) ;
	// ��Կ�Ƿ�Ϊ�����Ϊ�ղ���Ҫ����
	if ( ! _pEnv->GetUserKey(accesscode, M1, IA1, IC1 ) ) {
		return false ;
	}

	// ���Ϊ���ܴ���
	if ( encode ) {
		// ���ü��ܱ�־λ
		header->encrypt_flag =  1 ;
		// ��Ӽ�����Կ
		header->encrypt_key  =  ntouv32( CEncrypt::rand_key() ) ;
	}

	// ��������
	return CEncrypt::encrypt( M1, IA1, IC1, (unsigned char *)data, (unsigned int) len ) ;
}

void CPccServer::updateAreaids(const string &areaids)
{
	vector<User> users;
	vector<User>::iterator userIte;

	share::Guard guard(_mutex);

	if(_areaids == areaids) {
		return;
	}
	_areaids = areaids;

	users = _online_user.GetOnlineUsers();
	for(userIte = users.begin(); userIte != users.end(); ++userIte) {
		//��ÿ��·�ɿ��������ͱ�PCC��ʡ����
		if(userIte->_user_type != ROUTE_TYPE ) {
			continue;
		}

		SendData(userIte->_fd, _areaids.c_str(), _areaids.length());
	}
}

// ����PCC��ǰ��Ϣ
void CPccServer::UpdatePcc( void )
{
	char szbuf[1024] = {0};
	// ȡ���豸������
	if ( !_pEnv->GetString( "pcc_dev_name" , szbuf ) ) {
		OUT_ERROR( NULL, 0, NULL,  "get pcc dev name failed" ) ;
		return;
	}
	// ȡ�ñ�����IP�ĵ�ַ
	string localip = netutil::addrToString( netutil::getLocalAddr( szbuf ) ) ;

	sprintf(szbuf, "%s:%d", localip.c_str(), _listen_port );

	_pEnv->GetRedisCache()->SAdd( REDIS_PCCKEY , szbuf ) ;
}
