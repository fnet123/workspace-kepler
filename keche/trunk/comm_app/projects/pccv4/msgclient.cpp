#include "msgclient.h"
#include "pccutil.h"
#include "pconvert.h"
#include <tools.h>

#include "../tools/utils.h"

MsgClient::MsgClient( CServiceCaller &srvCaller ): /**_session( true ) ,*/
	 _srvCaller(srvCaller) ,  _statinfo("MsgClient")
{
	_last_handle_user_time = time(NULL) ;
	_convert			   = new PConvert ;
}

MsgClient::~MsgClient( void )
{
	Stop() ;

	if ( _convert != NULL ) {
		delete _convert ;
		_convert = NULL ;
	}
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	// ��ʼ��ת����������
	_convert->initenv( pEnv ) ;

	// ���ô���MSG�û��Ļص�����
	_pEnv->SetNotify( MSG_USER_TAG , this ) ;

	char temp[512] = {0} ;
	// ��ȡͼƬ�Ļ���ַ
	if ( ! pEnv->GetString( "base_picurl" , temp ) ) {
		printf( "get picture base url failed\n" ) ;
		return false ;
	}
	_picUrl.SetString( temp ) ;

	// ���ػ��������б�·��
	if ( pEnv->GetString( "base_dmddir" ,temp ) ) {
		_dmddir.SetString( temp ) ;
		OUT_INFO( NULL, 0 , NULL, "Load base dmddir %s", temp ) ;
	}

	// �����û��������б�
	if ( pEnv->GetString( "user_whitelist", temp ) ) {
		_whiteLst.LoadList( temp ) ;
		OUT_INFO( NULL, 0 , NULL, "Load white list macid %s", temp ) ;
	}

	int nvalue = 0;
	int send_thread = 1 ;
	// �����߳�
	if ( pEnv->GetInteger( "http_send_thread" , nvalue ) ) {
		send_thread = nvalue ;
	}

	int recv_thread = 1 ;
	// �����߳�
	if ( pEnv->GetInteger( "http_recv_thread" , nvalue ) ) {
		recv_thread = nvalue ;
	}

	int queue_size = 1000 ;
	// HTTP�������Ķ��г���
	if ( pEnv->GetInteger( "http_queue_size" , nvalue ) ) {
		queue_size = nvalue ;
	}
	// ��ʼ���첽��HTTPͼƬ����
	_httpCall.Init( send_thread, recv_thread, queue_size ) ;
	// ���ûص�����
	_httpCall.SetReponse( this ) ;

	// ���÷ְ�������ְ�
	setpackspliter( &_packspliter ) ;

	return true ;
}

void MsgClient::Stop( void )
{
	OUT_INFO("Msg",0,"MsgClient","stop");

	_httpCall.Stop() ;

	StopClient() ;
}

bool MsgClient::Start( void )
{
	// ��ʼ������HTTP����
	if ( ! _httpCall.Start() ) {
		OUT_ERROR( NULL, 0, NULL, "start http caller failed") ;
		return false ;
	}
	return StartClient( "0.0.0.0", 0, 4 ) ;
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
	if ( len < 4 ) return ;

	OUT_RECV( sock->_szIp, sock->_port, NULL, "%.*s", len, (const char*)data );

	const char *ptr = (const char *)data;
	if ( strncmp( ptr, "CAIT" , 4 ) == 0 ) {
		// �׷���������
		HandleInnerData( sock, ptr , len ) ;
	} else {
		// �����½���
		HandleSession( sock, ptr, len ) ;
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
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
	while(1) {
		if ( ! Check() ) break ;

		HandleOfflineUsers() ;
		// ����ʱ�Ķ��� , ��Ϊ��ʱ�������·����ɹ�
		// _session.CheckTimeOut( 120 ) ;
		// �����û�����
		_pEnv->LoadUserData() ;

		_statinfo.Check() ;

		sleep(5);
	}
}

void MsgClient::NoopWork()
{
	while(1) {
		if ( ! Check() ) break ;
		// ������·��������
		HandleOnlineUsers( 30 ) ;
		// ��⻺����Чʱ��
		_srvCaller.CheckTimeOut() ;

		sleep( 5 ) ;
	}
}

bool MsgClient::SendDataToUser( const string &area_code, const char *data, int len)
{
	if ( area_code == SEND_ALL ){
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
	sprintf( buf, "%s%s", MSG_USER_TAG, area_code.c_str() );//area_code.c_str()

	User user = _online_user.GetUserByUserId( buf );
	if( user._user_id.empty() || user._user_state != User::ON_LINE ){
		OUT_ERROR( user._ip.c_str() , user._port , buf , "SendDataToUser %s failed" , data ) ;
		return false;
	}
	// ��ӷ���ͳ������
	_statinfo.AddSend( user._access_code ) ;

	// ���������������ѭ����Ĵ���
	return SendData( user._fd, data, len ) ;
}

// ��MSG�ϴ���Ϣ
void MsgClient::HandleUpMsgData( const char *code, const char *data, int len )
{
	OUT_INFO( NULL, 0, "msg", "HandleUpMsgData %s", data ) ;

	string userid ;
	// �ӻỰ��ȡ�÷��Ͷ���
	if ( _session.GetSession( code , userid ) ) {
		// ������ڻỰ����
		User user = _online_user.GetUserByUserId( userid ) ;
		// �����ǰ�û�����
		if ( ! user._user_id.empty() && user._user_state == User::ON_LINE ) {
			SendData( user._fd, data, len ) ;
		}
		return ;
	}
	SendDataToUser( code , data, len ) ;
}

// ������½����
int MsgClient::build_login_msg( User &user, char *buf,int buf_len )
{
	string stype = "SAVE" , sext = "\r\n" ;
	if ( user._user_type == "DMDATA" ) {
		stype = "SAVE" ; sext = "DM \r\n" ;
	} else {
		stype = user._user_type ;
	}
	sprintf( buf, "LOGI %s %s %s %s",
			stype.c_str() , user._user_name.c_str(), user._user_pwd.c_str() , sext.c_str() ) ;

	return (int)strlen(buf) ;
}

// ���ض�������
void MsgClient::LoadSubscribe( User &user )
{
	string line;
	list<string> macids;
	list<string>::iterator lsIt;

	if( ! _pEnv->getSubscribe(macids) || macids.empty()) {
		return;
	}

	line.reserve(10240);

	line = "DMD 0 {0} \r\n"; // ��ն���
	SendData( user._fd, line.c_str(), line.length());
	line = "";

	for(lsIt = macids.begin(); lsIt != macids.end(); ++lsIt) {
		if(line.empty()) {
			line = "ADD 0 {" + *lsIt;
		} else {
			line += "," + *lsIt;
		}

		if(line.length() > 512) {
			line += "} \r\n";
			SendData( user._fd, line.c_str(), line.length());
			line = "";
		}
	}

	if( ! line.empty()) {
		line += "} \r\n";
		SendData(user._fd, line.c_str(), line.length());
		line = "";
	}
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

				OUT_CONN( sock->_szIp, sock->_port,user._user_name.c_str(),
						"Login success, fd %d access code %d" , sock->_fd, user._access_code ) ;
				// ��½�ɹ������Ϊ���ݶ������Ӿ�ֱ����Ҫ�����Ͷ��Ĵ���
				if ( user._user_type == "DMDATA" ) LoadSubscribe( user ) ;
			}
			break ;
		case -1:
			{
				OUT_ERROR(sock->_szIp, sock->_port, NULL , "LACK,password error!");
			}
			break ;
		case -2:
			{
				OUT_ERROR(sock->_szIp, sock->_port, NULL ,"LACK,the user has already login!");
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
		OUT_ERROR( sock->_szIp, sock->_port , "Msg" , "find fd %d user failed, data %s", sock->_fd, data ) ;
		return ;
	}

	HandleUserData( user, data, len ) ;

	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;
}

void MsgClient::HandleUserData( const User &user, const char *data, int len )
{
	string line( data, len ) ;
	vector<string>  vec ;
	if ( ! splitvector( line, vec, " " , 6 )  ){
		OUT_ERROR( user._ip.c_str() , user._port, user._user_name.c_str() , "fd %d data error: %s", user._fd, data ) ;
		return ;
	}

	// ����û��Ƿ��ڰ��������棬��������ھ�ֱ�ӷ�����
	if ( ! _whiteLst.OnWhite( user._access_code, vec[2].c_str() ) ) {
		OUT_PRINT(  user._ip.c_str() , user._port, user._user_name.c_str() , "fd %d mac id %s not in white list", user._fd, vec[2].c_str() ) ;
		return ;
	}

	string head  = vec[0] ;
 	string seqid = vec[1] ;
	string macid = vec[2] ;
	string cmd   = vec[4] ;
	string args  = vec[5] ;

	int msg_len = 0;
	char *msg_buf = NULL;
	unsigned int msgid = 0;
	unsigned int method = 0;

	string phone , ome ;
	// �����ֻ�����OEM��
	if (!_convert->get_phoneome(macid, phone, ome)) {
		return;
	}

	if(head == "CAITR") { //����ͨ��Ӧ������
		unsigned int accesscode ;

		if(cmd == "D_SNDM" || cmd == "D_CTLM") {
			msg_buf = _convert->convert_comm( seqid, phone, args , msg_len, accesscode ) ;
		} else if(cmd == "L_PLAT") {
			msg_buf = _convert->convert_lplat_r( seqid, phone, args , msg_len, accesscode ) ;
		}

		if(msg_buf == NULL || msg_len == 0) {
			return;
		}

		// ���ݶ�Ӧʡ���͸���Ӧʡ����
		_pEnv->GetPasClient()->HandlePasUpData(accesscode, msg_buf, msg_len); //����PAS
		_convert->free_buffer(msg_buf);

		return;
	} else if(head == "CAITS" && cmd == "U_REPT") {
		msg_buf = _convert->convert_urept(seqid, ome, phone, args, msg_len, msgid, method);

		// ��ӷ���ͳ������,ֻͳ���ϱ����ݵĴ���
		if (msgid == UP_EXG_MSG_REAL_LOCATION) {
			_statinfo.AddVechile(user._access_code, macid.c_str(), STAT_RECV);
		}

		// ��ӵ��Ự������
		_session.AddSession( macid, user._user_id ) ;
	} else if(head == "CAITS" && cmd == "L_PROV") {
		string areacode ;
		// �������չ����ڲ�Э��תΪ�������·�
		msg_buf = _convert->convert_lprov( seqid, seqid, args , msg_len, areacode ) ;
	} else if(head == "CAITS" && cmd == "L_PLAT") {
		string areacode ;
		// ���Ĵ�ʡ��ƽ̨��չ��̬�����ϴ�
		msg_buf = _convert->convert_lplat( seqid, seqid, args , msg_len, areacode ) ;

		if(msg_buf == NULL || msg_len == 0) {
			return;
		}

		int accesscode = atoi(macid.c_str());

		// ���ݶ�Ӧʡ���͸���Ӧʡ����
		_pEnv->GetPasClient()->HandlePasUpData(accesscode, msg_buf, msg_len); //����PAS
		_convert->free_buffer(msg_buf);

		return;
	}

	if(msg_buf == NULL || msg_len == 0) {
		return;
	}

	set<string> channels;
	set<string>::iterator ssit;
	vector<string> fields;
	string value;

	value = "";
	if( ! _pEnv->GetRedisCache()->HGet("KCTX.SECURE", phone.c_str(), value) || value.empty()) {
		OUT_WARNING(user._ip.c_str(), user._port, user._user_name.c_str(), "KCTX.SECURE: %s not exist", phone.c_str());
		_convert->free_buffer(msg_buf);
		return;
	}

	fields.clear();
	if (Utils::splitStr(value, fields, ',') < 10) {
		OUT_WARNING(user._ip.c_str(), user._port, user._user_name.c_str(), "KCTX.SECURE: %s is error", phone.c_str());
		_convert->free_buffer(msg_buf);
		return;
	}

	BaseMsgHeader *msgheader = (BaseMsgHeader*) (msg_buf + sizeof(Header));
	msgheader->vehicle_color = fields[3][0] - '0';
	memset(msgheader->vehicle_no, 0x00, sizeof(msgheader->vehicle_no));
	fields[4].copy(msgheader->vehicle_no, sizeof(msgheader->vehicle_no));

	if (method == METHOD_REG) {
		UpExgMsgRegister *req = (UpExgMsgRegister *) msg_buf;
		memset(req->producer_id, 0x00, sizeof(req->producer_id));
		memset(req->terminal_model_type, 0x00, sizeof(req->terminal_model_type));
		memset(req->terminal_id, 0x00, sizeof(req->terminal_id));
		memset(req->terminal_simcode, 0x00, sizeof(req->terminal_simcode));

		// ���ұ��
		fields[8].copy(req->producer_id, sizeof(req->producer_id));
		// �ն��ͺ�
		fields[1].copy(req->terminal_model_type, sizeof(req->terminal_model_type));
		// �ն˱��
		fields[2].copy(req->terminal_id, sizeof(req->terminal_id));

		string sim = string(sizeof(req->terminal_simcode), '\0') + phone;
		sim = sim.substr(sim.length() - phone.length());
		// �绰����
		sim.copy(req->terminal_simcode, sizeof(req->terminal_simcode));
	}

	channels.clear();
	_pEnv->getChannels(macid, channels);
	channels.insert(fields[9]);
	for (ssit = channels.begin(); ssit != channels.end(); ++ssit) {
		// ���ݶ�Ӧʡ���͸���Ӧʡ����
		_pEnv->GetPasClient()->HandleClientData(ssit->c_str(), msg_buf, msg_len); //����PAS
	}

	_convert->free_buffer(msg_buf);
}

void MsgClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3*60);
	for(int i = 0; i < (int)vec_users.size(); i++ ){
		User &user = vec_users[i];
		if(user._socket_type == User::TcpClient){
			if(user._fd != NULL ) {
				OUT_WARNING( user._ip.c_str() , user._port , user._user_name.c_str() ,
						"HandleOfflineUsers TcpClient close socket fd %d", user._fd->_fd );
				CloseSocket(user._fd);
			}
		} else if(user._socket_type == User::TcpConnClient) {
			if(user._fd != NULL) {
				OUT_INFO( user._ip.c_str() , user._port , user._user_name.c_str() ,
						"HandleOfflineUsers TcpConnClient close socket fd %d", user._fd->_fd );
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
//--------------------------------------------------------------------------------------
void MsgClient::HandleOnlineUsers(int timeval)
{
	time_t now = time(NULL) ;
	if( now - _last_handle_user_time < timeval){
		return;
	}

	_last_handle_user_time = now ;
	vector<User> vec_users = _online_user.GetOnlineUsers();

	for(int i = 0; i < (int)vec_users.size(); i++) {
		User &user = vec_users[i] ;
		if( user._socket_type == User::TcpConnClient && user._fd != NULL ) {
			string loop = "NOOP \r\n" ;
			SendData( user._fd, loop.c_str(), loop.length() ) ;
			OUT_SEND( user._ip.c_str(), user._port, user._user_id.c_str(),"NOOP");
		}
	}
}

//-------------------------------------- ������յ����⴦�� ---------------------------------------------------
// ��ȡ��ͼƬ�����⴦��
void MsgClient::LoadUrlPic( unsigned int seqid , const char *path )
{
	char url[1024] = {0};
	sprintf( url, "%s/%s", (const char *)_picUrl, path ) ;

	// ���ͻ�ȡ��Ƭ������
	if ( ! _httpCall.Request( seqid, url ) ) {
		// ��¼�����������
		OUT_ERROR( NULL, 0, NULL, "request url %s seq id %d failed", url, seqid ) ;
	}
}

// �������صĺ��ͼƬ����
void MsgClient::ProcessResp( unsigned int seqid, const char *data, const int nlen , const int err )
{
	// ��������
	if ( data == NULL || err != HTTP_CALL_SUCCESS || nlen == 0 ) {
		OUT_ERROR( NULL, 0, "Pic" , "pic seqid %u, error %d" , seqid, err ) ;
		return ;
	}
	/**
	char szfile[128] = {0};
	sprintf( szfile, "./%u.jpg", seqid ) ;
	WriteFile( szfile, data, nlen ) ;
	OUT_INFO( NULL, 0, "PHOTO", "write file %s length %d", szfile, nlen ) ;
	 */

	char szKey[512] = {0} ;
	_pEnv->GetCacheKey( seqid, szKey ) ;

	int  dlen = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( szKey, dlen ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, "Pic" , "pic get key %s cache failed" , szKey ) ;
		return ;
	}

	// ת����Ƭ�Ľṹ��
	UpCtrlMsgTakePhotoAck *ack = ( UpCtrlMsgTakePhotoAck * ) msg;

	int len = sizeof(UpCtrlMsgTakePhotoAck) + sizeof(Footer) + nlen ;

	ack->header.msg_len = ntouv32( len ) ;
	ack->ctrl_msg_header.data_length = ntouv32( sizeof(UpCtrlMsgTakePhotoBody) + nlen ) ;
	ack->ctrl_photo_body.photo_len 	 = ntouv32( nlen ) ;

	DataBuffer dbuf ;
	dbuf.writeBlock( ack, sizeof(UpCtrlMsgTakePhotoAck) ) ;
	if ( nlen > 0 ) {
		dbuf.writeBlock( data, nlen ) ;
	}
	Footer footer ;
	dbuf.writeBlock( &footer, sizeof(footer) ) ;

	// ȡ�ý�������Ϣ
	unsigned int accesscode = ntouv32( ack->header.access_code ) ;

	// �������յ���Ƭ����
	_pEnv->GetPasClient()->HandlePasUpData( accesscode, dbuf.getBuffer(), dbuf.getLength() ) ;

	OUT_SEND( NULL, 0, NULL, "UP_CTRL_MSG_TAKE_PHOTO_ACK:%s, picture length %d", ack->ctrl_msg_header.vehicle_no , nlen ) ;

	_pEnv->GetMsgCache()->FreeData( msg ) ;
}

//========================================= �û�����  ===============================================
// ��USERINFOת��ΪUser����
void MsgClient::ConvertUser( const _UserInfo &info, User &user )
{
	user._user_id     =  info.tag + info.code ;
	user._access_code =  atoi( info.code.c_str() ) ;
	user._ip          =  info.ip ;
	user._port        =  info.port ;
	user._user_name   =  info.user ;
	user._user_pwd    =  info.pwd  ;
	user._user_type   =  info.type ;
	user._user_state  = User::OFF_LINE ;
	user._socket_type = User::TcpConnClient ;
	user._connect_info.keep_alive = AlwaysReConn ;
	user._connect_info.timeval    = 30 ;
}

void MsgClient::NotifyUser( const _UserInfo &info , int op )
{
	string key = info.tag + info.code ;
	User  user = _online_user.GetUserByUserId( key ) ;

	OUT_PRINT( info.ip.c_str(), info.port, key.c_str() , "PasClient operate %d user, username %s, password %s" ,
				op , info.user.c_str(), info.pwd.c_str() ) ;

	switch( op ){
	case USER_ADDED:
		{
			ConvertUser( info, user ) ;
			// ����µ��û�
			if ( ! _online_user.AddUser( key, user ) ) {
				if ( user._fd ) {
					OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() , "MsgClient Add New user close fd %d" , user._fd->_fd ) ;
					CloseSocket( user._fd ) ;
				}
				_online_user.SetUser( key, user ) ;
			}
		}
		break ;
	case USER_DELED:
		if ( ! user._user_id.empty() ) {
			if ( user._fd ) {
				OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() , "MsgClient Delete User fd %d" , user._fd->_fd ) ;
				CloseSocket( user._fd ) ;
			}
			// ɾ���û�����
			_online_user.DeleteUser( key ) ;
		}
		break ;
	case USER_CHGED:
		if ( ! user._user_id.empty() ) {
			// �޸��û�����
			ConvertUser( info, user ) ;
			if ( user._fd ) {
				OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() , "MsgClient Change User close fd %d" , user._fd->_fd ) ;
				CloseSocket( user._fd ) ;
			}
			_online_user.SetUser( key, user ) ;
		}
		break ;
	}
}
