#include "pasclient.h"
#include <ProtoHeader.h>
#include <Base64.h>
#include "pccutil.h"
#include <crc16.h>
#include <comlog.h>
#include <tools.h>
#include "pconvert.h"
#include <BaseTools.h>

#include "httpquery.h"

PasClient::PasClient(CServiceCaller &srvCaller):
	_macid2seqid(true), _filecache(this), _srvCaller(srvCaller), _statinfo("PasClient")
{
	_last_handle_user_time = time(NULL) ;
	_down_port = 0 ;

	_smsNotifyUrl = "http://192.168.111.111/mail_sms/sms.php";
	_smsNotifyMail = "yanwei@ctfo.com;wangleij@yutong.com";
	_smsNotifyTitle = "pcc connect server fail";
}

PasClient::~PasClient()
{
	Stop() ;
}

bool PasClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char value[1024] = {0} ;
	if ( _pEnv->GetString("pcc_down_ip", value) ){
		_down_ip = value ;
	}
	// ȡ���Զ�Ӧ���ƽ̨��ڵ�����
	if ( _pEnv->GetString( "pcc_postquery", value ) ) {
		_postpath = value ;
	}

	int port = 0 ;
	if ( _pEnv->GetInteger("pcc_listen_port", port ) ){
		_down_port = port ;
	}

	// ��������ӿڵ�ַ
	if(_pEnv->GetString("sms_notify_url", value)) {
		_smsNotifyUrl = value;
	}

	// ���������ַ�����ŷָ������
	if(_pEnv->GetString("sms_notify_mail", value)) {
		_smsNotifyMail = value;
	}

	// �����ʼ�������
	if(_pEnv->GetString("sms_notify_title", value)) {
		_smsNotifyTitle = value;
	}

	// �����û�����ص�����
	_pEnv->SetNotify( PAS_USER_TAG , this ) ;

	// ��ʼ���������
	if ( ! _filecache.Init( pEnv ) ) {
		printf( "init file cache failed\n" ) ;
		return false ;
	}

	return true ;
}

void PasClient::Stop( void )
{
	_filecache.Stop() ;
	StopClient() ;
}

bool PasClient::Start( void )
{
	if ( ! _filecache.Start() ) {
		printf( "start file cache failed, %s:%d" , __FILE__, __LINE__ ) ;
		OUT_ERROR( NULL, 0, NULL, "start file cache failed") ;
		return false ;
	}
	return StartClient( "0.0.0.0", 0, 3 ) ;
}

bool PasClient::ConnectServer(User &user, int timeout /*= 10*/)
{
	if(time(0) - user._connect_info.last_reconnect_time < user._connect_info.timeval)
			return false;

	bool ret = false;
	if (user._fd  != NULL)
	{
		OUT_WARNING( user._ip.c_str(), user._port ,NULL,"fd %d close socket", user._fd->_fd );
		CloseSocket(user._fd);
	}
	user._fd = _tcp_handle.connect_nonb(user._ip.c_str(), user._port, timeout);
	ret = (user._fd != NULL) ? true:false;

	user._last_active_time = time(0);
	user._login_time       = time(0);
	user._connect_info.last_reconnect_time = time(0);

	if(ret ) {
		// ����ƽ̨����Ҫ���͵�¼������
		if(user._user_name.empty() && user._user_pwd.empty()) {
			user._user_state = User::ON_LINE;
			_online_user.AddUser(user._user_id, user);

			// ����û����ߴ���
			_filecache.Online( GetAreaCode(user) ) ;

			// �����¼������ĳɹ�����ɾ������״̬
			_smsNotifyMtx.lock();
			_smsNotifySet.erase(user._user_id);
			_smsNotifyMtx.unlock();
		} else {
			user._user_state = User::WAITING_RESP;

			// ����Ӻ�������
			_online_user.AddUser(user._user_id, user);

			UpConnectReq req;
			req.header.msg_seq = ntouv32(_proto_parse.get_next_seq());
			req.header.access_code = ntouv32(user._access_code);
			req.user_id = ntouv32(atoi(user._user_name.c_str()));
			memcpy(req.password, user._user_pwd.c_str(), sizeof(req.password));

			// ����������о���Ҫ��������
			if (_down_port > 0) {
				safe_memncpy((char*) req.down_link_ip, _down_ip.c_str(),
						sizeof(req.down_link_ip));
				req.down_link_port = ntouv16(_down_port);
			}
			OUT_INFO(user._ip.c_str(), user._port, user._user_id.c_str(), "Send UpConnectReq,down-link state:CONNECT_WAITING_RESP");
			SendCrcData(user._fd, (const char*) &req, sizeof(req));
		}
	} else {
		user._user_state = User::OFF_LINE;
	}

	/**
	if(user._connect_info.keep_alive == ReConnTimes)
		user._connect_info.reconnect_times--;
	*/
	return ret;
}

void PasClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
	C5BCoder coder ;
	if ( ! coder.Decode( (const char *)data, len ) )
	{
		OUT_WARNING(sock->_szIp, sock->_port, NULL,"Except packet header or tail");
		return;
	}

	// ����ӽ�������
	EncryptData( (unsigned char*) coder.GetData() , (unsigned int)coder.GetSize() , false ) ;
	// ��������
	HandleOnePacket( sock,(const char*)coder.GetData() , coder.GetSize() );
}

// �������ŵ�����ʡƽ̨DOWN������
void PasClient::HandlePasDownData( const int access, const char *data, int len )
{
	User user = _online_user.GetUserByAccessCode( access ) ;
	if ( user._user_id.empty() || user._user_state != User::ON_LINE ) {
		OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "HandlePasDownData user not online" ) ;
		return ;
	}
	// �����������·��������
	HandleOnePacket( user._fd , data , len ) ;
}

void PasClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
	if ( ! user._user_id.empty() || user._fd == NULL ) {
		// �������״̬
		_filecache.Offline( GetAreaCode(user) ) ;
		// �������״̬����
		_srvCaller.updateConnectState( UP_DISCONNECT_RSP , _pEnv->GetSequeue() , GetAreaCode(user) , CONN_MASTER , CONN_DISCONN ) ;
	}
	if ( user._user_state != User::DISABLED ) {
		OUT_WARNING( sock->_szIp, sock->_port, user._user_id.c_str(), "Disconnection fd %d", sock->_fd );
		user._user_state = User::OFF_LINE ;
	}
	user._fd = NULL ;
	_online_user.SetUser( user._user_id, user ) ;
}

void PasClient::TimeWork()
{
	/*
	 * 1.����ʱ������ȥ����
	 * 2.��ʱ����NOOP��Ϣ
	 * 3.Reload�����ļ��е��µ����ӡ�
	 * 4.
	 */
	while(1) {
		if ( ! Check() ) break ;
		// ��������
		HandleOfflineUsers() ;

		// ���û���ʮ���ӳ�ʱ
		_pEnv->GetMsgCache()->CheckData( 600 ) ;
		// ����ӳ�ʱʱ��
		_macid2seqid.CheckTimeOut( 300 ) ;

		sleep(2) ;
	}
}

void PasClient::NoopWork()
{
	while(1)
	{
		// ��������û�����������
		HandleOnlineUsers( 30 ) ;
		// ��ӡͳ����Ϣ����
		_statinfo.Check() ;

		sleep(3) ;
	}
}

// ��PAS������ͨ��������
void PasClient::HandlePasUpData( const int access, const char *data, int len )
{
	User user = _online_user.GetUserByAccessCode( access ) ;
	if ( user._user_id.empty() ) {
		OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "HandlePasDownData user empty" ) ;
		return ;
	}

	// �û�û�����ߵ����
	if ( user._user_state != User::ON_LINE ) {
		OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "HandlePasDownData user not online" ) ;
		// ��ǰ�û������߼�¼����
		_filecache.WriteCache( GetAreaCode( user ) , (void*)data, len ) ;
		return ;
	}

	Header *header = ( Header *) data ;
	header->access_code = ntouv32( user._access_code ) ;
	// ���������������ѭ����Ĵ���
	if ( ! SendCrcData( user._fd, data, len ) ) {
		// Todo: Send failed
	}
}

void PasClient::HandleClientData( const char *code, const char *data, int len )
{
	if ( SendDataToUser( code, data, len ) ) {
		OUT_SEND( NULL, 0, code, "Send data %s", _proto_parse.Decoder(data, len).c_str() ) ;
	} else {
		OUT_ERROR( NULL, 0, code, "Send Data %s Failed", _proto_parse.Decoder(data, len).c_str() ) ;
	}
}

bool PasClient::SendDataToUser( const string &area_code, const char *data, int len)
{
	if ( area_code == SEND_ALL ){
		vector<User>  users = _online_user.GetOnlineUsers() ;
		if ( users.empty() ) {
			return false ;
		}

		// �������û�����
		for ( size_t i = 0; i < users.size(); ++ i ) {
			// ����ÿ���û���Ҫ�����Ӧ�Ľ�����
			User &user = users[i] ;
			Header *header = ( Header *) data ;
			header->access_code = ntouv32( user._access_code ) ;
			// Ⱥ������
			SendCrcData( user._fd, data, len ) ; //809 Э��
		}
		return true ;
	}

	char buf[512] = {0};
	sprintf( buf, "%s%s", PAS_USER_TAG, area_code.c_str() ) ;

	User user = _online_user.GetUserByUserId( buf );
	if( user._user_id.empty() ){
		OUT_HEX( "channel not exist" , 0 , buf , data, len );
		return false;
	}

	// �û�û�����ߵ����
	if ( user._user_state != User::ON_LINE ) {
		OUT_WARNING( user._ip.c_str(), user._port, buf, "HandlePasDownData user not online" ) ;
		// ��ǰ�û������߼�¼����
		_filecache.WriteCache( GetAreaCode( user ) , (void*)data, len ) ;
		return false;
	}

	Header *header = ( Header *) data ;
	header->access_code = ntouv32( user._access_code ) ;

	// ���Ϊ��չ��Ϣ����ӳ���ͳ���д���
	if ( ntouv16(header->msg_type) == UP_EXG_MSG ) {
		char szmacid[128] = {0} ;
		ExgMsgHeader *msgheader = (ExgMsgHeader*) (data + sizeof(Header));
		sprintf( szmacid, "%d_%s", msgheader->vehicle_color, (const char *) msgheader->vehicle_no ) ;
		_statinfo.AddVechile( user._access_code, szmacid, STAT_SEND ) ;
	}

	// ���������������ѭ����Ĵ���
	return SendCrcData( user._fd, data, len ) ;
}

void PasClient::sendSmsNotify(const User &user)
{
	SmsNotify sn;
	pair<map<string, SmsNotify>::iterator, bool> ret;

	time_t curTime = time(NULL);
	sn.btime = sn.ltime = curTime;
	sn.count = 0;

	_smsNotifyMtx.lock();
	ret = _smsNotifySet.insert(make_pair(user._user_id, sn));
	if(ret.second == true) {
		// ����ǸնϿ������򲻴�������
		_smsNotifyMtx.unlock();
		return;
	}

	// �����������3�λ򱨾����ʱ��û5���ӣ�����������
	if(ret.first->second.count >= 3 || curTime - ret.first->second.ltime < 300) {
		_smsNotifyMtx.unlock();
		return;
	}

	ret.first->second.count += 1;
	ret.first->second.ltime = curTime;

	curTime = ret.first->second.btime;

	_smsNotifyMtx.unlock();

	char text[1024];
	struct tm *tm = localtime(&curTime);

	snprintf(text, 1024, "%s: %s:%d disconnect, at %04d-%02d-%02d %02d:%02d:%02d, ",\
			user._user_id.c_str(), user._ip.c_str(), user._port,\
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,\
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	string msg = "notify_receiver=" + _smsNotifyMail;
	msg += "&user_name=kcpt&user_passwd=Kpct_sms_passwd";
	msg += "&notify_title=" + _smsNotifyTitle;
	msg += "&notify_body=" + string(text);

	HttpQuery hq;

	hq.post(_smsNotifyUrl, msg);
}

void PasClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3*60);
	for(int i = 0; i < (int)vec_users.size(); i++) {
		User &user = vec_users[i];

		// ����ƽ̨����Ҫ���͵�¼������
		if(user._user_name.empty() && user._user_pwd.empty() && user._user_state == User::ON_LINE && user._fd != NULL) {
			_online_user.AddUser( user._user_id, user ) ;
		} else if(user._socket_type == User::TcpClient) {
			if(user._fd != NULL){
				OUT_WARNING( user._ip.c_str() , user._port , user._user_id.c_str() ,
						"HandleOfflineUsers PasClient TcpClient close socket fd %d", user._fd->_fd );
				CloseSocket(user._fd);
			}
		} else if(user._socket_type == User::TcpConnClient) {
			sendSmsNotify(user);  // �����ʼ���������

			if(user._fd !=NULL ){
				OUT_INFO( user._ip.c_str() , user._port , user._user_id.c_str() ,
						"HandleOfflineUsers PasClient TcpConnClient close socket fd %d", user._fd->_fd );
				user.show();
				CloseSocket(user._fd);
				user._fd = NULL;
			}
			if ( ! ConnectServer(user, 10) ) {
				//����ʧ�ܣ�һ����Ҫ����
				_online_user.AddUser( user._user_id, user ) ;
			}
		}
	}
}

void PasClient::HandleOnlineUsers(int timeval)
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

		// ����ƽ̨����Ҫ���͵�¼������
		if(user._user_name.empty() && user._user_pwd.empty()) {
			continue;
		}

		if( user._socket_type == User::TcpConnClient && user._fd != NULL ) {
			UpLinkTestReq req;
			req.header.access_code = ntouv32(user._access_code);
			req.header.msg_seq 	   = ntouv32(_proto_parse.get_next_seq());
			req.crc_code 		   = ntouv16(GetCrcCode((const char*)&req,sizeof(req)));
			Send5BCodeData( user._fd,(const char*)&req,sizeof(req), false );

			OUT_SEND( user._ip.c_str(), user._port, user._user_id.c_str(),
					"%s", _proto_parse.Decoder((const char*)&req,sizeof(req)).c_str());
		}
	}
}

// �������ݽ���5B���봦��
bool PasClient::Send5BCodeData( socket_t *sock, const char *data, int len  , bool bflush )
{
	if ( sock == NULL )  {
		return false ;
	}
	C5BCoder  coder;
	if ( ! coder.Encode( data, len ) ) {
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "Send5BCodeData failed , socket fd %d", sock->_fd ) ;
		return false ;
	}

	OUT_HEX(sock->_szIp, sock->_port, "SEND", coder.GetData(), coder.GetSize());

	return SendData( sock, coder.GetData(), coder.GetSize() ) ;
}

// �������´���ѭ���������
bool PasClient::SendCrcData( socket_t *sock, const char* data, int len)
{
	// ����ѭ����
	char *buf = new char[len+1] ;
	memcpy( buf, data, len ) ;
	// OUT_HEX( NULL, 0, "Before", buf, len ) ;
	// ����ӽ�������
	EncryptData( (unsigned char*) buf , len , true ) ;
	// OUT_HEX( NULL, 0, "End", buf, len ) ;
	// ͳһ����ѭ�������֤
	unsigned short crc_code = ntouv16( GetCrcCode( buf, len ) ) ;
	unsigned int   offset   = len - sizeof(Footer) ;
	// �滻ѭ�����ڴ��λ������
	memcpy( buf + offset , &crc_code, sizeof(short) ) ;

	bool bSend = Send5BCodeData( sock, buf , len ) ;

	delete [] buf ;

	return bSend ;
}

void PasClient::HandleOnePacket( socket_t *sock, const char* data , int len )
{
	const char *ip = sock->_szIp ;
	unsigned short port = sock->_port ;

	if(  len < (int)sizeof(Header) || data == NULL ){
		OUT_ERROR( ip, port, NULL, "data length errro length %d", len ) ;
		OUT_HEX( ip, port, NULL , data, len ) ;
		return ;
	}

	Header *header = (Header *) data;
	unsigned int access_code = ntouv32(header->access_code);
	string str_access_code   = uitodecstr(access_code);
	unsigned int msg_len     = ntouv32(header->msg_len);
	unsigned short msg_type  = ntouv16(header->msg_type);
	string mac_id = _proto_parse.GetMacId( data , len );

	// ��ӽ��յ�������ͳ�Ʒ���
	_statinfo.AddRecv( access_code ) ;

	if ( msg_len > len || msg_len == 0 ) {
		OUT_ERROR( ip, port, NULL, "data length errro length %d , msg len %d", len , msg_len ) ;
		OUT_HEX( ip, port, NULL , data, len ) ;
		return ;
	}

	OUT_RECV( ip, port, str_access_code.c_str(), "%s", _proto_parse.Decoder(data,len).c_str() ) ;
	OUT_HEX( ip, port, str_access_code.c_str(), data, len ) ;

	User user = _online_user.GetUserBySocket( sock ) ;

	if (msg_type == UP_CONNECT_RSP)
	{
		UpConnectRsp *rsp = ( UpConnectRsp *) data ;
		switch(rsp->result)
		{
		case 0:
			OUT_INFO(ip,port,str_access_code.c_str(),"login check success,access_code:%d  up-link ON_LINE",access_code);
			user._user_state  = User::ON_LINE ;
			break;
		case 1:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,ip is invalid");
			break;
		case 2:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,accesscode is invalid,close it");
			break;
		case 3:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_name:%d is invalid,close it", user._user_id.c_str());
			break;
		case 4:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_password:%s is invalid,close it",user._user_pwd.c_str());
			break;
		default:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,other error,close it");
			break;
		}

		int areacode = GetAreaCode( user ) ;
		if ( rsp->result != 0 ) {
			CloseSocket( sock ) ;
			return ;
		}

		// ������
		user._last_active_time = time(NULL) ;
		// �����û���̬
		_online_user.SetUser( user._user_id, user ) ;
		// ����û����ߴ���
		_filecache.Online( areacode ) ;
		// ��������״̬����
		_srvCaller.updateConnectState( UP_CONNECT_RSP , _pEnv->GetSequeue() , areacode, CONN_MASTER , CONN_CONNECT ) ;

		// �����¼������ĳɹ�����ɾ������״̬
		_smsNotifyMtx.lock();
		_smsNotifySet.erase(user._user_id);
		_smsNotifyMtx.unlock();

		return ;
	}
	else if (msg_type == UP_LINKTEST_RSP)//"NOOP_ACK")
	{
	}
	else if ( msg_type == UP_DISCONNECT_RSP ) // �յ��Ͽ�������Ӧ��ֱ�Ӵ�������״̬
	{
		// ������������������յ����������Ӧ���ٶϿ�����
		if ( User::DISABLED == user._user_state ) {
			if ( user._fd != NULL ) {
				OUT_ERROR( ip, port, str_access_code.c_str(), "disconnect response fd %d" , user._fd->_fd ) ;
				// �ر�����
				CloseSocket( user._fd ) ;
			}
		} else {
			user._user_state = User::OFF_LINE ;
			// ���͹رմ���·���������Ƿ��쳣�����������������ע��
			_pEnv->GetPccServer()->Close( access_code , UP_DISCONNECT_INFORM , 0x00 ) ;
		}
	}
	else if ( msg_type == DOWN_DISCONNECT_INFORM )   // ����·�·�����·�Ͽ�֪ͨ
	{
		// �����رմ���·����
		// _pEnv->GetPccServer()->Close( user._access_code, 0, 0 ) ;
		OUT_INFO( ip, port, str_access_code.c_str(), "Recv DOWN_DISCONNECT_INFORM" ) ;
	}
	else if ( msg_type == DOWN_CLOSELINK_INFORM )  // �����ر�������·֪ͨ
	{
		// �����ر�������·����
		// _pEnv->GetPccServer()->Close( user._access_code, 0, 0 ) ;
		OUT_INFO( ip, port, str_access_code.c_str(), "Recv DOWN_CLOSELINK_INFORM" ) ;
	}
	else if ( msg_type == DOWN_PLATFORM_MSG )
	{
		DownPlatformMsg *plat_msg = ( DownPlatformMsg * ) ( data + sizeof(Header) ) ;
		unsigned short data_type = ntouv16( plat_msg->data_type ) ;
		switch( data_type ) {
		case DOWN_PLATFORM_MSG_POST_QUERY_REQ:  // ƽ̨�����Ϣ
			{
				if ( len < (int)sizeof(DownPlatformMsgPostQueryReq) ) {
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_PLATFORM_MSG_POST_QUERY_REQ data length error , length %d" , len ) ;
					return ;
				}
				DownPlatformMsgPostQueryReq *msg = (DownPlatformMsgPostQueryReq*)data;

				// ȡ��ƽ̨��ڵĳ���
				int nlen = ntouv32( msg->down_platform_body.info_length ) ;
				if ( nlen < 0 || nlen + (int)sizeof(DownPlatformMsgPostQueryReq) > len ) {
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_PLATFORM_MSG_POST_QUERY_REQ data length error , length %d, content len %d" , len , nlen ) ;
					return ;
				}

				UpPlatformMsgPostQueryAck resp;

				resp.header.msg_len               = 0 ;  // ��ʱȡ0�����յ���Ӧ�������¼���
				resp.header.access_code 	      = ntouv32( user._access_code ) ;
				resp.header.msg_seq 		      = msg->header.msg_seq ;
				resp.up_platform_msg.data_length  = 0 ;
				resp.up_platform_post.info_id	  = msg->down_platform_body.info_id ;
				resp.up_platform_post.msg_len	  = 0 ;
				resp.up_platform_post.object_type = msg->down_platform_body.object_type ;
				safe_memncpy( resp.up_platform_post.object_id, msg->down_platform_body.object_id , sizeof(resp.up_platform_post.object_id) ) ;

				std::string text ;
				// ToDo: Ϊ�˹������ƽ̨ʵ�ֵ��Զ����Ӧ������
				if ( _postquerymgr.GetPostQuery( user._access_code, msg->down_platform_body.object_type, text ) ){

					resp.header.msg_len 			 = ntouv32( sizeof(UpPlatformMsgPostQueryAck)   + text.length() + sizeof(Footer) ) ;
					resp.up_platform_msg.data_length = ntouv32( sizeof(UpPlatformMsgpostqueryData) + text.length() ) ;
					resp.up_platform_post.msg_len    = ntouv32( text.length() ) ;

					DataBuffer buf ;
					buf.writeBlock( &resp, sizeof(resp) ) ;
					buf.writeBlock( text.c_str(), text.length() ) ;

					Footer footer ;
					buf.writeBlock( &footer, sizeof(footer) ) ;
					// �Զ�Ӧ����
					if ( SendCrcData( sock, buf.getBuffer(), buf.getLength() ) ) {
						OUT_PRINT( ip, port , str_access_code.c_str(), "recv DOWN_PLATFORM_MSG_POST_QUERY_REQ , auto reply UP_PLATFORM_MSG_POST_QUERY_ACK , content: %s" , text.c_str() ) ;
					} else {
						OUT_ERROR( ip, port , str_access_code.c_str(), "recv DOWN_PLATFORM_MSG_POST_QUERY_REQ , auto reply UP_PLATFORM_MSG_POST_QUERY_ACK , content: %s" , text.c_str() ) ;
					}
				} else {  // �ֶ�������̴���

					CQString content ;
					content.SetString( (const char*)(data+sizeof(DownPlatformMsgPostQueryReq)) , nlen ) ;

					unsigned int seqid = _pEnv->GetSequeue() ;
					char szKey[256]={0};
					_pEnv->GetCacheKey( seqid, szKey ) ;

					// ��ӵ��ȴ�������
					_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

					char messageId[512] = {0} ;
					sprintf( messageId, "%d" , ntouv32(  msg->down_platform_body.info_id ) ) ;

					char objectType[128] = {0} ;
					sprintf( objectType, "%d", msg->down_platform_body.object_type ) ;

					char objectId[256] = {0} ;
					safe_memncpy( objectId, msg->down_platform_body.object_id , sizeof(msg->down_platform_body.object_id) ) ;

					char areaId[128] = {0} ;
					sprintf( areaId, "%u" , GetAreaCode(user) ) ;

					// ����ƽ̨���
					_srvCaller.addForMsgPost( DOWN_PLATFORM_MSG_POST_QUERY_REQ , seqid, content.GetBuffer() , messageId , objectId, objectType , areaId ) ;

					OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_PLATFORM_MSG_POST_QUERY_REQ seqid %u, messageId %s, objectId %s, objectType %s, areaId %s, content %s, key: %s" ,
							seqid , messageId, objectId, objectType, areaId , content.GetBuffer(), szKey ) ;
				}
			}
			break ;
		case DOWN_PLATFORM_MSG_INFO_REQ:
			{
				if ( len < (int)sizeof(DownPlatformMsgInfoReq) ) {
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_PLATFORM_MSG_INFO_REQ data length error , length %d" , len ) ;
					return ;
				}
				//ƽ̨�·���������
				DownPlatformMsgInfoReq * msg = ( DownPlatformMsgInfoReq *) data ;

				// ȡ��ƽ̨��ڵĳ���
				int nlen = ntouv32( msg->info_length ) ;
				if ( nlen < 0 || nlen + (int)sizeof(DownPlatformMsgInfoReq) > len ) {
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_PLATFORM_MSG_INFO_REQ data length error , length %d, content len %d" , len , nlen ) ;
					return ;
				}

				CQString content ;
				content.SetString( (const char*)(data+sizeof(DownPlatformMsgInfoReq)) , nlen ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				UpPlatFormMsgInfoAck resp;

				resp.header.msg_len               = ntouv32( sizeof(resp) ) ;
				resp.header.msg_type			  = ntouv16( UP_PLATFORM_MSG ) ;
				resp.header.access_code 	      = ntouv32( user._access_code ) ;
				resp.header.msg_seq 		      = msg->header.msg_seq ;
				resp.up_platform_msg.data_type	  = ntouv16( UP_PLATFORM_MSG_INFO_ACK ) ;
				resp.up_platform_msg.data_length  = ntouv32( sizeof(int) ) ;
				resp.info_id 					  = msg->info_id ;

				// ��ӵ��ȴ�������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				char messageId[512] = {0} ;
				sprintf( messageId, "%d" , ntouv32(  msg->info_id ) ) ;

				char objectType[128] = {0} ;
				sprintf( objectType, "%d", msg->object_type ) ;

				char objectId[256] = {0} ;
				safe_memncpy( objectId, msg->object_id , sizeof(msg->object_id) ) ;

				char areaId[128] = {0} ;
				sprintf( areaId, "%u" , GetAreaCode(user) ) ;

				// ����ƽ̨���
				_srvCaller.addForMsgInfo( DOWN_PLATFORM_MSG_INFO_REQ , seqid, content.GetBuffer() , messageId , objectId, objectType , areaId ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_PLATFORM_MSG_INFO_REQ seqid %u, messageId %s, objectId %s, objectType %s, areaId %s, content %s" ,
										seqid , messageId, objectId, objectType, areaId , content.GetBuffer() ) ;
			}
			break ;
		}
	}
	else if ( msg_type == DOWN_CTRL_MSG )
	{
		if ( len < (int)sizeof(DownCtrlMsgHeader) ){
			OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_CTRL_MSG data length error , length %d" , len ) ;
			return ;
		}

		DownCtrlMsgHeader *req = ( DownCtrlMsgHeader *) data ;
		int data_type = ntouv16( req->ctrl_msg_header.data_type ) ;

		char carnum[128]= {0};
		safe_memncpy( carnum, req->ctrl_msg_header.vehicle_no, sizeof(req->ctrl_msg_header.vehicle_no) ) ;
		char carcolor[128] = {0} ;
		sprintf( carcolor, "%d" , req->ctrl_msg_header.vehicle_color ) ;

		switch( data_type )
		{
		case DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ: //����· ������
			{
				if ( len < (int)sizeof(DownCtrlMsgMonitorVehicleReq) ){
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ data length %d error" , len ) ;
					return ;
				}

				DownCtrlMsgMonitorVehicleReq *msg = (DownCtrlMsgMonitorVehicleReq *) data;

				unsigned int seqid = _pEnv->GetSequeue() ;

				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				char szphone[128] = {0};
				safenumber( szphone, msg->monitor_tel, sizeof(msg->monitor_tel) ) ;

				// �����ݻ��棬Ȼ��Ҫ���ݻ������ݶ�Ӧ����
				UpCtrlMsgMonitorVehicleAck resp;//ack Ӧ��ͨ������·�ظ�
				resp.header.msg_len 	= ntouv32(sizeof(UpCtrlMsgMonitorVehicleAck));
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.ctrl_msg_header.vehicle_color = msg->ctrl_msg_header.vehicle_color;
				safe_memncpy(resp.ctrl_msg_header.vehicle_no, msg->ctrl_msg_header.vehicle_no, sizeof(resp.ctrl_msg_header.vehicle_no) );
				resp.ctrl_msg_header.data_length = ntouv32( sizeof(unsigned char) ) ;
				resp.result 			= 0x00;

				// ��ӵ����������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				string macid;
				if( ! _pEnv->getMacid(msg->ctrl_msg_header.vehicle_no, macid)) {
					return;
				}

				int szlen;
				char szbuf[1024] = {0} ;
				szlen = sprintf( szbuf, "CAITS %s %s 4 D_CTLM {TYPE:9,RETRY:0,VALUE:%s} \r\n" , szKey, macid.c_str(), szphone ) ;
				_pEnv->GetMsgClient()->HandleUpMsgData(macid.c_str(), szbuf, szlen);

				// ����HTTP��������
				//_srvCaller.getTernimalByVehicleByTypeEx( DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ, seqid, carnum , carcolor , szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break;
		case DOWN_CTRL_MSG_TAKE_PHOTO_REQ:  // ��������
			{
				if ( len < (int)sizeof(DownCtrlMsgTakePhotoReq) ) {
					OUT_ERROR( ip,port,str_access_code.c_str(), "DOWN_CTRL_MSG_TAKE_PHOTO_REQ data length %d error" , len ) ;
					return ;
				}

				DownCtrlMsgTakePhotoReq *msg = ( DownCtrlMsgTakePhotoReq *) data ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};

				UpCtrlMsgTakePhotoAck  resp ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.ctrl_msg_header.vehicle_color = msg->ctrl_msg_header.vehicle_color ;
				safe_memncpy( resp.ctrl_msg_header.vehicle_no , msg->ctrl_msg_header.vehicle_no , sizeof(msg->ctrl_msg_header.vehicle_no) ) ;
				resp.ctrl_photo_body.lens_id   = msg->lens_id ;
				resp.ctrl_photo_body.size_type = msg->size ;

				string macid;
				if( ! _pEnv->getMacid(msg->ctrl_msg_header.vehicle_no, macid)) {
					return;
				}

				sprintf( szKey, "%s_%d" , macid.c_str(), UP_CTRL_MSG_TAKE_PHOTO_ACK ) ;

				// ��ӵ�������
				_pEnv->GetMsgCache()->AddData( szKey, (const char*)&resp, sizeof(resp) ) ;

				OUT_INFO( NULL, 0, NULL, "DOWN_CTRL_MSG_TAKE_PHOTO_REQ add key %s" , szKey ) ;

				_pEnv->GetCacheKey( seqid, szKey ) ;

				// ����ͷͨ��ID|��������|¼��ʱ��|�����־|�ֱ���|��Ƭ����|����|�Աȶ�|���Ͷ�|ɫ��
				int szlen;
				char szbuf[1024] = {0} ;
				szlen = sprintf( szbuf, "CAITS %s %s 4 D_CTLM {TYPE:10,RETRY:0,VALUE:%d|1|1|0|%d|10|128|128|128|128} \r\n",
						szKey, macid.c_str(), msg->lens_id, msg->size - 1 ) ;
				_pEnv->GetMsgClient()->HandleUpMsgData(macid.c_str(), szbuf, szlen);

				//_srvCaller.getTernimalByVehicleByTypeEx( DOWN_CTRL_MSG_TAKE_PHOTO_REQ, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_CTRL_MSG_TAKE_PHOTO_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_CTRL_MSG_TEXT_INFO:  // �·��ı�
			{
				if ( len < (int)sizeof(DownCtrlMsgTextInfoHeader) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_CTRL_MSG_TEXT_INFO data length %d error" , len ) ;
					return  ;
				}

				DownCtrlMsgTextInfoHeader *msg = ( DownCtrlMsgTextInfoHeader *)data ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				CBase64 base;
				int nlen = ntouv32( msg->msg_len ) ;
				base.Encode( data + sizeof(DownCtrlMsgTextInfoHeader) , nlen ) ;

				//string senddata= "CAITS "+ seq + " "+macID+" 0 D_SNDM {TYPE:1,1:255,2:"+base.GetBuffer()+"} \r\n";
				//_pEnv->GetMsgClient()->HandleUpMsgData( SEND_ALL, senddata.c_str(), senddata.length() ) ;

				UpCtrlMsgTextInfoAck resp ;
				resp.header.msg_len 	= ntouv32(sizeof(UpCtrlMsgTextInfoAck));
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.ctrl_msg_header.vehicle_color = msg->ctrl_msg_header.vehicle_color;
				safe_memncpy(resp.ctrl_msg_header.vehicle_no, msg->ctrl_msg_header.vehicle_no, sizeof(resp.ctrl_msg_header.vehicle_no) );
				resp.ctrl_msg_header.data_length = ntouv32( sizeof(unsigned char) + sizeof(int) ) ;
				resp.msg_id				= msg->msg_sequence ;
				resp.result 			= 0x00;

				string macid;
				if( ! _pEnv->getMacid(msg->ctrl_msg_header.vehicle_no, macid)) {
					return;
				}

				//��ӵ����������
				_pEnv->GetMsgCache()->AddData( szKey , (const char *)&resp, sizeof(resp));
				OUT_INFO( ip, port, str_access_code.c_str() , "Add UpCtrlmsgTexInfoAck key %s", szKey ) ;

				int szlen;
				char szbuf[1024] = {0} ;
				szlen = sprintf(szbuf, "CAITS %s %s 4 D_SNDM {TYPE:1,1:255,2:%s} \r\n", szKey, macid.c_str(), base.GetBuffer()) ;

				_pEnv->GetMsgClient()->HandleUpMsgData(macid.c_str(), szbuf, szlen);
				//_srvCaller.getTernimalByVehicleByTypeEx( DOWN_CTRL_MSG_TEXT_INFO, seqid, carnum , carcolor, inner.c_str() ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_CTRL_MSG_TEXT_INFO seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_CTRL_MSG_TAKE_TRAVEL_REQ:  //2011-11-29 xfm �ϱ�������ʻ��¼����
			{
				if ( len < (int)sizeof(DownCtrlMsgTaketravelReq) ){
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_CTRL_MSG_TAKE_TRAVEL_REQ data length %d error" , len ) ;
					return ;
				}

				DownCtrlMsgTaketravelReq *msg = (DownCtrlMsgTaketravelReq *)data;

				unsigned int seqid = _pEnv->GetSequeue() ;

				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				UpCtrlMsgTaketravel resp;

				resp.header.msg_len 	= ntouv32(sizeof(UpCtrlMsgTaketravel));
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq;
				resp.header.msg_type    = ntouv16( UP_CTRL_MSG ) ;
				resp.command_type       = msg->command_type;
				resp.ctrl_msg_header.data_type     = ntouv16( UP_CTRL_MSG_TAKE_TRAVEL_ACK ) ;
				resp.ctrl_msg_header.vehicle_color = msg->ctrl_msg_header.vehicle_color;
				safe_memncpy(resp.ctrl_msg_header.vehicle_no, msg->ctrl_msg_header.vehicle_no, sizeof(resp.ctrl_msg_header.vehicle_no) );

				resp.ctrl_msg_header.data_length = 0 ;

				string macid;
				if( ! _pEnv->getMacid(msg->ctrl_msg_header.vehicle_no, macid)) {
					return;
				}

				//��ӵ����������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp));
				OUT_INFO( ip, port, str_access_code.c_str() , "Add UpCtrlMsgTaketravelAck key %s", szKey );

				int szlen;
				char szbuf[1024] = {0};
				szlen = sprintf( szbuf, "CAITS %s %s 4 D_REQD {TYPE:4,30:%u} \r\n" , szKey, macid.c_str(), msg->command_type ) ;

				_pEnv->GetMsgClient()->HandleUpMsgData(macid.c_str(), szbuf, szlen);
				//_srvCaller.getTernimalByVehicleByTypeEx( DOWN_CTRL_MSG_TAKE_TRAVEL_REQ, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_CTRL_MSG_TAKE_TRAVEL_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break;
		case DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ:  //2011-11-29 xfm ����������ƽ̨
		    {
		    	if ( len < (int)sizeof(DownCtrlMsgEmergencyMonitoringReq) ){
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ data length %d error" , len ) ;
					return ;
				}

				DownCtrlMsgEmergencyMonitoringReq *msg = (DownCtrlMsgEmergencyMonitoringReq *)data;

				string macid;
				if( ! _pEnv->getMacid(msg->ctrl_msg_header.vehicle_no, macid)) {
					return;
				}

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				string inner = "CAITS " + string(szKey) + " " + macid +" 4 D_CTLM {TYPE:21,VALUE:0;";
				inner += safe2string(msg->authentication_code, sizeof(msg->authentication_code)) + ";" ;
				inner += safe2string( msg->access_point_name, sizeof(msg->access_point_name)) + ";" ;
				inner += safe2string( msg->username, sizeof(msg->username) ) + ";" ;
				inner += safe2string( msg->password, sizeof(msg->password) ) + ";" ;
				inner += safe2string( msg->server_ip , sizeof(msg->server_ip) ) + ";" ;
				inner += uitodecstr( ntouv16(msg->tcp_port) ) + ";" ;
				inner += uitodecstr( ntouv16(msg->udp_port) ) + ";0} \r\n" ;

				UpCtrlMsgEmergencyMonitoringAck resp;

				resp.header.msg_type    = ntouv16( UP_CTRL_MSG ) ;
				resp.header.msg_len 	= ntouv32(sizeof(UpCtrlMsgEmergencyMonitoringAck));
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.ctrl_msg_header.data_type = ntouv16( UP_CTRL_MSG_EMERGENCY_MONITORING_ACK ) ;

				resp.ctrl_msg_header.vehicle_color = msg->ctrl_msg_header.vehicle_color;
				safe_memncpy(resp.ctrl_msg_header.vehicle_no, msg->ctrl_msg_header.vehicle_no, sizeof(resp.ctrl_msg_header.vehicle_no) );

				resp.ctrl_msg_header.data_length = ntouv32( sizeof(unsigned char) );
				resp.result 			= 0x00;
				//��ӵ����������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp));

				_pEnv->GetMsgClient()->HandleUpMsgData(macid.c_str(), inner.c_str(), inner.length());
				//_srvCaller.getTernimalByVehicleByTypeEx( DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ, seqid, carnum , carcolor, inner.c_str() ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
		    }
			break ;
		}
	}
	else if (msg_type == DOWN_WARN_MSG ) {  // ����·������Ϣ������Ϣ
		WarnMsgHeader *warnheader = ( WarnMsgHeader *) (data + sizeof(Header)) ;
		int data_type = ntouv16( warnheader->data_type ) ;

		char carnum[128]= {0};
		safe_memncpy( carnum, warnheader->vehicle_no, sizeof(warnheader->vehicle_no) ) ;
		char carcolor[128] = {0} ;
		sprintf( carcolor, "%d" , warnheader->vehicle_color ) ;

		switch( data_type ) {
		case DOWN_WARN_MSG_URGE_TODO_REQ:  // ������������
			{
				if ( len < (int) sizeof(DownWarnMsgUrgeTodoReq) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_WARN_MSG_URGE_TODO_REQ data length %d error" , len ) ;
					return ;
				}

				DownWarnMsgUrgeTodoReq *req = ( DownWarnMsgUrgeTodoReq *) data ;

				UpWarnMsgUrgeTodoAck resp ;
				resp.header.msg_len				   = ntouv32( sizeof(UpWarnMsgUrgeTodoAck) ) ;
				resp.header.access_code 		   = ntouv32( user._access_code ) ;
				resp.header.msg_seq     		   = req->header.msg_seq ;
				resp.warn_msg_header.vehicle_color = req->warn_msg_header.vehicle_color;
				safe_memncpy(resp.warn_msg_header.vehicle_no, req->warn_msg_header.vehicle_no, sizeof(resp.warn_msg_header.vehicle_no) );

				resp.warn_msg_header.data_length   = ntouv32( sizeof(int) + sizeof(char) ) ;
				resp.supervision_id     		   = req->warn_msg_body.supervision_id ;
				resp.result						   = 0x00 ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				char supervisionEndUtc[128]={0};
				sprintf( supervisionEndUtc, "%llu", (unsigned long long)ntouv64(req->warn_msg_body.supervision_endtime) ) ;

				char supervisionId[128] = {0} ;
				sprintf( supervisionId , "%d" , ntouv32(req->warn_msg_body.supervision_id) ) ;

				char supervisionLevel[128] = {0} ;
				sprintf( supervisionLevel, "%d" , req->warn_msg_body.supervision_level ) ;

				char supervisor[256] = {0} ;
				safe_memncpy( supervisor, req->warn_msg_body.supervisor, sizeof(req->warn_msg_body.supervisor) ) ;

				char supervisorEmail[128] = {0} ;
				safe_memncpy( supervisorEmail, req->warn_msg_body.supervisor_email, sizeof(req->warn_msg_body.supervisor_email) ) ;

				char supervisorTel[128] = {0} ;
				safe_memncpy( supervisorTel, req->warn_msg_body.supervisor_tel, sizeof(req->warn_msg_body.supervisor_tel) ) ;

				char wanSrc[128] = {0} ;
				sprintf( wanSrc, "%d", req->warn_msg_body.warn_src ) ;

				char wanType[128] = {0} ;
				sprintf( wanType, "%d", ntouv16( req->warn_msg_body.warn_type ) ) ;

				char warUtc[128] = {0} ;
				sprintf( warUtc , "%llu", (unsigned long long)ntouv64(req->warn_msg_body.warn_time) ) ;

				_srvCaller.addMsgUrgeTodo( DOWN_WARN_MSG_URGE_TODO_REQ , seqid , supervisionEndUtc , supervisionId ,
						supervisionLevel , supervisor , supervisorEmail , supervisorTel , carcolor, carnum , wanSrc , wanType , warUtc ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_WARN_MSG_URGE_TODO_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_WARN_MSG_INFORM_TIPS: // ����Ԥ��
		case DOWN_WARN_MSG_EXG_INFORM:  // ʵʱ����������Ϣ
			{
				if ( len < (int) sizeof(DownWarnMsgInformTips) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "%s data length %d error" ,
							( data_type == DOWN_WARN_MSG_EXG_INFORM ) ? "DOWN_WARN_MSG_EXG_INFORM" : "DOWN_WARN_MSG_INFORM_TIPS" , len ) ;
					return ;
				}

				DownWarnMsgInformTips *req = ( DownWarnMsgInformTips *) data ;

				int nlen = ntouv32(req->warn_msg_body.warn_length) ;
				if ( len < ( int ) sizeof(req) + nlen || nlen < 0 ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "%s data length %d error" ,
							( data_type == DOWN_WARN_MSG_EXG_INFORM ) ? "DOWN_WARN_MSG_EXG_INFORM" : "DOWN_WARN_MSG_INFORM_TIPS" , len ) ;
					return ;
				}

				unsigned int seqid = _pEnv->GetSequeue() ;

				CQString content ;
				content.SetString( (const char *)(data+sizeof(DownWarnMsgInformTips)) , nlen ) ;

				char alarmFrom[128] = {0} ;
				sprintf( alarmFrom, "%d", req->warn_msg_body.warn_src ) ;

				char alarmTime[128] = {0} ;
				sprintf( alarmTime, "%llu", (unsigned long long)ntouv64(req->warn_msg_body.warn_time) ) ;

				char alarmType[128] = {0} ;
				sprintf( alarmType, "%d", ntouv16(req->warn_msg_body.warn_type) ) ;

				// ���÷��񱣴�
				_srvCaller.addMsgInformTips( data_type , seqid , content.GetBuffer() , alarmFrom, alarmTime, alarmType , carcolor , carnum ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "%s seqid %u, carnum %s, carcolor %s" , ( data_type == DOWN_WARN_MSG_EXG_INFORM ) ? "DOWN_WARN_MSG_EXG_INFORM" : "DOWN_WARN_MSG_INFORM_TIPS", seqid , carnum, carcolor ) ;
			}
			break ;
		}
	}
	else if ( msg_type == DOWN_EXG_MSG ) {  // ����·��̬��Ϣ������Ϣ
		ExgMsgHeader *msgheader = (ExgMsgHeader*) (data + sizeof(Header));
		int data_type = ntouv16( msgheader->data_type ) ;

		char carnum[128]= {0};
		safe_memncpy( carnum, msgheader->vehicle_no, sizeof(msgheader->vehicle_no) ) ;
		char carcolor[128] = {0} ;
		sprintf( carcolor, "%d" , msgheader->vehicle_color ) ;

		switch( data_type ) {
		case DOWN_EXG_MSG_CAR_LOCATION:  //����������λ��Ϣ
			{
				if ( len < (int) sizeof(DownExgMsgCarLocation) ) {
					OUT_ERROR( ip , port, str_access_code.c_str(), "DOWN_EXG_MSG_CAR_LOCATION data length %d error" , len ) ;
					return ;
				}

				DownExgMsgCarLocation *msg = ( DownExgMsgCarLocation *) data ;

				string gps ;
				PConvert::build_gps_info( gps, &msg->gnss ) ;
				string inner = " 4 U_REPT {TYPE:0," + gps + "} \r\n";

				unsigned int seqid = _pEnv->GetSequeue() ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_CAR_LOCATION, seqid, carnum , carcolor, inner.c_str() ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_CAR_LOCATION seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_HISTORY_ARCOSSAREA: // ������λ��Ϣ��������
			{
				if ( len < (int) sizeof(DownExgMsgHistoryArcossareaHeader) ) {
					OUT_ERROR( ip , port, str_access_code.c_str(), "DOWN_EXG_MSG_HISTORY_ARCOSSAREA data length %d error" , len ) ;
					return ;
				}

				DownExgMsgHistoryArcossareaHeader *msg = ( DownExgMsgHistoryArcossareaHeader *) data ;
				if ( len < (int) (sizeof(DownExgMsgHistoryArcossareaHeader) + msg->cnt_num * sizeof(GnssData) + sizeof(Footer)) ) {
					OUT_ERROR( ip , port, str_access_code.c_str(), "DOWN_EXG_MSG_HISTORY_ARCOSSAREA data length %d error" , len ) ;
					return ;
				}

				string inner ;
				// �������ݰ����ڲ�Э��
				const char *ptr = ( const char *) ( data + sizeof(DownExgMsgHistoryArcossareaHeader) ) ;
				unsigned int num = msg->cnt_num ;
				for ( int i = 0; i < (int)num ; ++ i ) {
					GnssData *gnss = ( GnssData *) ptr ;
					if ( ! inner.empty() ) {
						inner += "|" ;
					}
					string gps ;
					PConvert::build_gps_info( gps, gnss ) ;
					inner += gps ;
					ptr = ptr + sizeof(GnssData) ;
				}

				unsigned int seqid = _pEnv->GetSequeue() ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_HISTORY_ARCOSSAREA, seqid, carnum , carcolor, inner.c_str() ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_HISTORY_ARCOSSAREA seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
				// ��ӳ�������ʼ
				_filecache.ChgArcoss( GetAreaCode(user) , msg->exg_msg_header.vehicle_color, msg->exg_msg_header.vehicle_no ) ;
			}
			break ;
		case DOWN_EXG_MSG_CAR_INFO: // ����������̬��Ϣ
			{
				if ( len < (int) sizeof(DownExgMsgCarInfoHeader) ) {
					OUT_ERROR( ip , port, str_access_code.c_str(), "DOWN_EXG_MSG_CAR_LOCATION data length %d error" , len ) ;
					return ;
				}
				DownExgMsgCarInfoHeader *msg = ( DownExgMsgCarInfoHeader *) data ;

				int dlen = (int)ntouv32( msg->exg_msg_header.data_length ) ;
				if ( dlen <= 0 || dlen > len ) {
					OUT_ERROR( ip , port, str_access_code.c_str(), "DOWN_EXG_MSG_CAR_LOCATION contet length %d more than length %d error" , dlen, len ) ;
					return ;
				}

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;

				CBase64  base64 ;
				base64.Encode( (const char *)( data+sizeof(DownExgMsgCarInfoHeader) ) , dlen ) ;

				// U_BASE
				char szbuf[10240] = {0} ;
				sprintf( szbuf, " 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,U_BASE:%s} \r\n" ,
						GetAreaCode(user), szmac, ntouv32(user._access_code), base64.GetBuffer() ) ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_CAR_INFO, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_CAR_INFO seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_RETURN_STARTUP:  // ����������λ��Ϣ��������
			{
				if ( len < (int) sizeof(DownExgMsgReturnStartUp) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_RETURN_STARTUP data length %d error" , len ) ;
					return ;
				}

				DownExgMsgReturnStartUp *msg = (DownExgMsgReturnStartUp *)data;

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szseq[256] = {0};
				_pEnv->GetCacheKey( seqid, szseq ) ;

				UpExgMsgReturnStartupAck  resp ;
				resp.header.msg_len 	= ntouv32( sizeof(resp) ) ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.exg_msg_header.vehicle_color = msg->exg_msg_header.vehicle_color ;
				safe_memncpy( resp.exg_msg_header.vehicle_no, msg->exg_msg_header.vehicle_no , sizeof(resp.exg_msg_header.vehicle_no) ) ;
				resp.exg_msg_header.data_length = ntouv32( 0 ) ;

				_pEnv->GetMsgCache()->AddData( szseq, (const char *)&resp, sizeof(resp) ) ;

				int areacode = GetAreaCode(user) ;

				char szbuf[1024] = {0} ;
				sprintf( szbuf, " 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,RETURNSTARTUP:%d} \r\n" ,
						areacode , szmac, ntouv32(user._access_code), msg->reason_code ) ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_RETURN_STARTUP, seqid, carnum , carcolor, szbuf ) ;
				// ��ӳ�������ʼ
				_filecache.AddArcoss( areacode, msg->exg_msg_header.vehicle_color, msg->exg_msg_header.vehicle_no ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_RETURN_STARTUP seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_RETURN_END: // ����������λ��Ϣ��������
			{
				if ( len < (int) sizeof(DownExgMsgArcossareaend) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_RETURN_END data length %d error" , len ) ;
					return ;
				}
				DownExgMsgArcossareaend *msg = (DownExgMsgArcossareaend *)data;

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szseq[256] = {0};
				_pEnv->GetCacheKey( seqid, szseq ) ;

				UpExgMsgReturnEndAck  resp ;
				resp.header.msg_len 	= ntouv32( sizeof(resp) ) ;
				resp.header.msg_seq 	= msg->header.msg_seq ;
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.exg_msg_header.vehicle_color = msg->exg_msg_header.vehicle_color ;
				safe_memncpy( resp.exg_msg_header.vehicle_no, msg->exg_msg_header.vehicle_no , sizeof(resp.exg_msg_header.vehicle_no) ) ;
				resp.exg_msg_header.data_length = ntouv32( 0 ) ;

				_pEnv->GetMsgCache()->AddData( szseq, (const char *)&resp, sizeof(resp) ) ;

				int areacode = GetAreaCode( user ) ;

				char szbuf[1024] = {0} ;
				sprintf( szbuf, " 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,RETURNEND:%d} \r\n" ,
						areacode, szmac, ntouv32(user._access_code), msg->reason_char ) ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_RETURN_END, seqid, carnum , carcolor, szbuf ) ;
				// ��ӳ����������
				_filecache.DelArcoss( areacode, msg->exg_msg_header.vehicle_color, msg->exg_msg_header.vehicle_no ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_RETURN_END seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK: // ���뽻��ָ���ĳ�����λ��ϢӦ��
			{
				if ( len < (int) sizeof(DownExgMsgApplyforMonitorstartUpAck) ){
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK data length %d error" , len ) ;
					return ;
				}
				DownExgMsgApplyforMonitorstartUpAck *msg = ( DownExgMsgApplyforMonitorstartUpAck *) data ;

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				char szseq[256] = {0};
				if ( ! GetMacId2SeqId( DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK , szmac, szseq ) ) {
					sprintf( szseq, "0_0" ) ;
				}

				char szbuf[1024] = {0} ;
				sprintf( szbuf, "CAITR %s MACID 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,MONITORSTARTUP:%d} \r\n" ,
						szseq, GetAreaCode(user), szmac, ntouv32(user._access_code), msg->result ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK: // ȡ����������ָ���Ķ�λ��ϢӦ��
			{
				if ( len < (int) sizeof(DownExgMsgApplyforMonitorEndAck) ){
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK data length %d error" , len ) ;
					return ;
				}
				DownExgMsgApplyforMonitorEndAck *msg = ( DownExgMsgApplyforMonitorEndAck *) data ;

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				char szseq[256] = {0};
				if ( ! GetMacId2SeqId( DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK , szmac, szseq ) ) {
					sprintf( szseq, "0_0" ) ;
				}

				char szbuf[1024] = {0} ;
				sprintf( szbuf, "CAITR %s MACID 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,MONITOREND:%d} \r\n" ,
						szseq, GetAreaCode(user), szmac, ntouv32(user._access_code), msg->result ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK: // ToDo: ����������λ��ϢӦ�����ﻹ��һ���Զ�����Ҫ�󲹷����
			{
				if ( len < (int) sizeof(DownExgMsgApplyHisgnssdataAck) ){
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK data length %d error" , len ) ;
					return ;
				}
				DownExgMsgApplyHisgnssdataAck *msg = ( DownExgMsgApplyHisgnssdataAck *) data ;

				char szmac[256] = {0};
				sprintf( szmac, "%s_%s" , carcolor, carnum ) ;

				char szseq[256] = {0};
				if ( ! GetMacId2SeqId( DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK , szmac, szseq ) ) {
					sprintf( szseq, "0_0" ) ;
				}

				char szbuf[1024] = {0} ;
				sprintf( szbuf, "CAITR %s MACID 4 L_PROV {AREA_CODE:%d,CARNO:%s,ACCESS_CODE:%d,TYPE:D_MESG,HISGNSSDATA:%d} \r\n" ,
						szseq, GetAreaCode(user), szmac, ntouv32(user._access_code), msg->result ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;

				_srvCaller.getTernimalByVehicleByTypeEx( DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK, seqid, carnum , carcolor, szbuf ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_REPORT_DRIVER_INFO: // �ϱ�������ʻԱ���ʶ����Ϣ����
			{
				if ( len < (int)sizeof(DownExgMsgReportDriverInfo) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_EXG_MSG_REPORT_DRIVER_INFO data length %d error" , len ) ;
					return ;
				}
				//http ��ѯ����
				DownExgMsgReportDriverInfo *msg = (DownExgMsgReportDriverInfo *)data;

				UpExgMsgReportDriverInfo resp;
				resp.header.msg_len 	= ntouv32(sizeof(UpExgMsgReportDriverInfo));
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq;
				resp.header.msg_type    = ntouv16( UP_EXG_MSG);
				resp.exg_msg_header.data_type     = ntouv16(UP_EXG_MSG_REPORT_DRIVER_INFO_ACK);
				resp.exg_msg_header.vehicle_color = msg->exg_msg_header.vehicle_color;
				safe_memncpy(resp.exg_msg_header.vehicle_no, msg->exg_msg_header.vehicle_no, sizeof(resp.exg_msg_header.vehicle_no) );
				resp.exg_msg_header.data_length = ntouv32(  sizeof(UpExgMsgReportDriverInfo) - sizeof(Header) - sizeof(ExgMsgHeader) - sizeof(Footer) );

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				// ��ӵ�������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				// ���÷��񱣴�
				_srvCaller.getDriverOfVehicleByType( DOWN_EXG_MSG_REPORT_DRIVER_INFO, seqid , carnum , carcolor) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_REPORT_DRIVER_INFO seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		case DOWN_EXG_MSG_TAKE_WAYBILL_REQ: // �ϱ����������˵�����
			{
				DownExgMsgTakeWaybillReq *msg = (DownExgMsgTakeWaybillReq *)data;

				UpExgMsgReportEwaybillInfo  resp;
				resp.header.msg_len 	= 0 ;
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.header.msg_seq 	= msg->header.msg_seq;
				resp.header.msg_type    = ntouv16( UP_EXG_MSG );
				resp.exg_msg_header.data_type     = ntouv16(UP_EXG_MSG_TAKE_WAYBILL_ACK);
				resp.exg_msg_header.vehicle_color = msg->exg_msg_header.vehicle_color;
				safe_memncpy(resp.exg_msg_header.vehicle_no, msg->exg_msg_header.vehicle_no, sizeof(resp.exg_msg_header.vehicle_no) );

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				// ��ӵ�������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				// ���÷��񱣴�
				_srvCaller.getEticketByVehicle( DOWN_EXG_MSG_TAKE_WAYBILL_REQ, seqid , carnum , carcolor ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_EXG_MSG_TAKE_WAYBILL_REQ seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		}

	}
	else if ( msg_type == DOWN_BASE_MSG ) { // ����·��̬��Ϣ������Ϣ
		BaseMsgHeader *base_header = ( BaseMsgHeader *) ( data + sizeof(Header) ) ;
		int data_type = ntouv16( base_header->data_type ) ;

		char carnum[128]= {0};
		safe_memncpy( carnum, base_header->vehicle_no, sizeof(base_header->vehicle_no) ) ;
		char carcolor[128] = {0} ;
		sprintf( carcolor, "%d" , base_header->vehicle_color ) ;

		switch( data_type ) {
		case DOWN_BASE_MSG_VEHICLE_ADDED:
			{
				if ( len < (int)sizeof(DownBaseMsgVehicleAdded) ) {
					OUT_ERROR( ip, port, str_access_code.c_str(), "DOWN_BASE_MSG_VEHICLE_ADDED data length %d error" , len ) ;
					return ;
				}

				DownBaseMsgVehicleAdded *msg = (DownBaseMsgVehicleAdded*)data ;

				// ����������̬��Ϣ
				UpbaseMsgVehicleAddedAck resp ;
				resp.header.msg_seq		= msg->header.msg_seq ;
				resp.header.access_code = ntouv32( user._access_code ) ;
				resp.msg_header.vehicle_color = base_header->vehicle_color ;
				safe_memncpy( resp.msg_header.vehicle_no, base_header->vehicle_no, sizeof(base_header->vehicle_no) ) ;

				unsigned int seqid = _pEnv->GetSequeue() ;
				char szKey[256]={0};
				_pEnv->GetCacheKey( seqid, szKey ) ;

				// ��ӵ�������
				_pEnv->GetMsgCache()->AddData( szKey, (const char *)&resp, sizeof(resp) ) ;

				// ���÷��񱣴�
				_srvCaller.getDetailOfVehicleInfo( DOWN_BASE_MSG_VEHICLE_ADDED, seqid , carnum , carcolor ) ;

				OUT_PRINT( ip, port , str_access_code.c_str(), "DOWN_BASE_MSG_VEHICLE_ADDED seqid %u, carnum %s, carcolor %s" , seqid , carnum, carcolor ) ;
			}
			break ;
		}
	}
	else
	{
		OUT_WARNING( ip , port , str_access_code.c_str(), "except message:%s", (const char*)data ) ;
	}

	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;
}

// �ر�����·����������
void PasClient::Close( int accesscode )
{
	User user = _online_user.GetUserByAccessCode( accesscode ) ;
	if ( user._user_id.empty() ) {
		OUT_ERROR( NULL, 0, NULL, "close access code %d user not exist" , accesscode ) ;
		return ;
	}
	user._user_state = User::OFF_LINE ;
	_online_user.SetUser( user._user_id, user ) ;
}

// ���µ�ǰ����·������״̬
void PasClient::UpdateSlaveConn( int accesscode, int state )
{
	User user = _online_user.GetUserByAccessCode( accesscode ) ;
	if ( user._user_id.empty() ) {
		OUT_ERROR( NULL, 0, NULL, "update slave close access code %d user not exist", accesscode ) ;
		return ;
	}
	// ��������״̬����
	_srvCaller.updateConnectState( ( state == CONN_CONNECT ) ? DOWN_CONNECT_RSP : DOWN_DISCONNECT_RSP ,
			_pEnv->GetSequeue() , GetAreaCode(user), CONN_SLAVER ,  state ) ;
}

// ֱ�ӶϿ���Ӧʡ�����Ӵ���
void PasClient::Enable( int areacode , int flag )
{
	char szuser[128] = {0};
	sprintf( szuser, "%s%d" , PAS_USER_TAG , areacode ) ;

	User user = _online_user.GetUserByUserId( szuser ) ;
	if ( user._user_id.empty() ) {
		OUT_ERROR( NULL, 0, NULL, "Enable areacode user %d failed" , areacode ) ;
		return ;
	}

	// �Ƿ�������
	if ( flag & PAS_USERLINK_ONLINE ) {
		OUT_INFO(user._ip.c_str(),user._port,user._user_id.c_str(),"Send user state offline then reconnect");
		// ��������Ϊ��������
		if ( user._user_state != User::ON_LINE ) {
			user._user_state = User::OFF_LINE ;
		}
	} else if ( flag & PAS_MAINLINK_LOGOUT ){
		OUT_INFO(user._ip.c_str(),user._port,user._user_id.c_str(),"Send UpDisconnectReq, UP_DISCONNECT_REQ");
		// �Ͽ�����
		UpDisconnectReq req ;
		req.header.msg_seq 		= ntouv32(_proto_parse.get_next_seq());
		req.header.access_code  = ntouv32( user._access_code);
		req.user_id        		= ntouv32( atoi(user._user_name.c_str()) ) ;
		memcpy( req.password , user._user_pwd.c_str(), sizeof(req.password) ) ;

		SendCrcData( user._fd , (const char*)&req, sizeof(req) );

		user._user_state = User::DISABLED ;

	} else if ( flag & PAS_SUBLINK_ERROR ) {  // �������·�쳣�����
		OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "PAS_SUBLINK_ERROR : UP_CLOSELINK_INFORM close double link" ) ;
		// ����·�쳣����
		_pEnv->GetPccServer()->Close( user._access_code, UP_CLOSELINK_INFORM, 0x00 ) ;
		// ģ����Ϊ�쳣ֱ�ӹر�
		if ( user._fd > 0 ) {
			CloseSocket( user._fd ) ;
			user._user_state = User::OFF_LINE ;
		}
	} else if ( flag & PAS_MAINLINK_ERROR ) { // ��������·�쳣�����
		OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "PAS_MAINLINK_ERROR : UP_DISCONNECT_INFORM message" ) ;
		// ���͹رմ���·����
		_pEnv->GetPccServer()->Close( user._access_code , UP_DISCONNECT_INFORM , 0x00 ) ;
		/**
		// ģ����Ϊ�쳣ֱ�ӹر�
		if ( user._fd > 0 ) {
			CloseSocket( user._fd ) ;
			user._user_state = User::OFF_LINE ;
		}*/
	} else {  // ��������
		OUT_ERROR( user._ip.c_str(), user._port, user._user_id.c_str(), "Recv Error Enable flag %x", flag ) ;
	}
	// ����״̬Ϊ����
	_online_user.SetUser( szuser , user ) ;
}

// ���MACID��SEQID��ӳ���ϵ
void PasClient::AddMacId2SeqId( unsigned short msgid, const char *macid, const char *seqid )
{
	// ������Ӧ��������������,0x8000
	if ( ! ( msgid & 0x8000 ) ) {
		msgid |= 0x8000 ;
	}

	char key[512] = {0};
	// MAC����Ϣ�������ֶ�Ӧ���������
	sprintf( key, "%s_%d" , macid, msgid ) ;

	_macid2seqid.AddSession( key, seqid ) ;
}

// ͨ��MACID����Ϣ����ȡ�ö�Ӧ����
bool PasClient::GetMacId2SeqId( unsigned short msgid, const char *macid, char *seqid )
{
	// ������Ӧ��������������,0x8000
	if ( ! ( msgid & 0x8000 ) ) {
		msgid |= 0x8000 ;
	}

	char key[512] = {0};
	// MAC����Ϣ�������ֶ�Ӧ���������
	sprintf( key, "%s_%d" , macid, msgid ) ;

	string val ;
	if ( ! _macid2seqid.GetSession( key, val ) ){
		return false ;
	}
	sprintf( seqid, "%s" , val.c_str() ) ;

	return true ;
}

// ȡ�õ�ǰ�û����������
int PasClient::GetAreaCode( User &user )
{
	if ( user._user_id.empty() ) {
		return 0 ;
	}
	return atoi((const char *)( user._user_id.c_str() + PAS_TAG_LEN ));
}

// ���ܴ�������
bool PasClient::EncryptData( unsigned char *data, unsigned int len , bool encode )
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
	// printf( "M1: %d, IA1: %d, IC1: %d\n" , M1, IA1, IC1 ) ;

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

//-----------------------------�������ݴ���ӿ�-----------------------------------------
// �����ⲿ����
int PasClient::HandleQueue( const char *uid, void *buf, int len , int msgid )
{
	OUT_INFO( NULL, 0, NULL, "HanldeQueue msg id %d , areacode %s , data length %d" , msgid, uid, len ) ;

	switch( msgid ) {
	case DATA_FILECACHE:  // �û����������ݻ���
		{
			// ������ݳ��Ȳ���ȷֱ�ӷ���
			if ( len < (int)sizeof(Header) ) {
				OUT_ERROR( NULL, 0, NULL, "HandleQueue filecache data length %d error", len ) ;
				return IOHANDLE_ERRDATA;
			}

			OUT_INFO( NULL, 0, NULL, "HandleQueue %s" , _proto_parse.Decoder( (const char *)buf, len ).c_str() ) ;

			Header *header = (Header *) buf;
			unsigned short msg_type  = ntouv16(header->msg_type);
			// �����Ϊ��չ����Ϣ
			if ( msg_type != UP_EXG_MSG ) {
				// SendDataToUser( uid, (const char *)buf , len ) ;
				return IOHANDLE_ERRDATA;
			}

			// ���Ϊ��չ����Ϣ
			ExgMsgHeader *exg = (ExgMsgHeader*) ((const char *)(buf) + sizeof(Header) ) ;
			unsigned short data_type = ntouv16( exg->data_type ) ;
			if ( data_type != UP_EXG_MSG_REAL_LOCATION ) {
				// SendDataToUser( uid, (const char *)buf , len ) ;
				return IOHANDLE_ERRDATA;
			}

			UpExgMsgRealLocation *req = ( UpExgMsgRealLocation *) buf ;

			UpExgMsgHistoryHeader msg ;
			memcpy( &msg, buf, sizeof(msg) ) ;
			msg.exg_msg_header.data_length = ntouv32( sizeof(char) + sizeof(GnssData) ) ;
			msg.exg_msg_header.data_type   = ntouv16(UP_EXG_MSG_HISTORY_LOCATION) ;
			msg.header.msg_len			   = ntouv32( sizeof(msg) + sizeof(char) + sizeof(GnssData) + sizeof(Footer) ) ;

			DataBuffer dbuf ;
			dbuf.writeBlock( &msg, sizeof(msg) ) ;
			dbuf.writeInt8( 1 ) ;
			dbuf.writeBlock( &req->gnss_data, sizeof(GnssData) ) ;

			Footer footer ;
			dbuf.writeBlock( &footer, sizeof(footer) ) ;

			if ( ! SendDataToUser( uid, dbuf.getBuffer(), dbuf.getLength() ) ) {
				OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_HISTORY_LOCATION:%s" , msg.exg_msg_header.vehicle_no ) ;
				return IOHANDLE_FAILED ;
			}

			OUT_SEND( NULL, 0, NULL, "UP_EXG_MSG_HISTORY_LOCATION:%s" , msg.exg_msg_header.vehicle_no ) ;
		}
		break;
	case DATA_ARCOSSDAT: // �����ݲ�����Ϣ
		{
			// С��ָ�������ݳ���ֱ�ӷ�����
			if ( len < (int)sizeof(ArcossData) ) {
				OUT_ERROR( NULL, 0, NULL, "HandleQueue arcossdat data length %d error", len ) ;
				return IOHANDLE_ERRDATA;
			}

			// ����������λ��Ϣ������Ϣ
			ArcossData *p = ( ArcossData *) buf ;

			UpExgApplyHisGnssDataReq req ;
			req.header.msg_len  = ntouv32( sizeof(req) ) ;
			req.header.msg_seq  = ntouv32( _pEnv->GetSequeue() ) ;
			req.header.msg_type = ntouv16( UP_EXG_MSG ) ;
			req.exg_msg_header.vehicle_color = p->color ;
			safe_memncpy( req.exg_msg_header.vehicle_no, p->vechile, sizeof(req.exg_msg_header.vehicle_no) ) ;
			req.exg_msg_header.data_length   = ntouv32( sizeof(uint64) * 2 ) ;
			req.exg_msg_header.data_type	 = ntouv16( UP_EXG_MSG_APPLY_HISGNSSDATA_REQ ) ;
			req.start_time = ntouv64( p->time ) ;
			req.end_time   = ntouv64( time(NULL) ) ;

			if ( ! SendDataToUser( uid, (const char *)&req, sizeof(req) ) ) {
				OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_APPLY_HISGNSSDATA_REQ:%s" , p->vechile ) ;
				return IOHANDLE_FAILED ;
			}

			OUT_SEND( NULL, 0, NULL, "UP_EXG_MSG_APPLY_HISGNSSDATA_REQ:%s" , p->vechile ) ;
		}
		break;
	}

	return IOHANDLE_SUCCESS ;
}

//========================================= �û�����  ===============================================
// ��USERINFOת��ΪUser����
void PasClient::ConvertUser( const _UserInfo &info, User &user )
{
	user._user_id     =  info.tag + info.code ;
	user._access_code =  atoi( info.type.c_str() ) ;  // ���������
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

void PasClient::NotifyUser( const _UserInfo &info , int op )
{
	string key = info.tag + info.code ;
	User user  = _online_user.GetUserByUserId( key ) ;

	OUT_PRINT( info.ip.c_str(), info.port, key.c_str() , "PasClient operate %d user, username %s, password %s" ,
				op , info.user.c_str(), info.pwd.c_str() ) ;

	switch( op ){
	case USER_ADDED:
		{
			ConvertUser( info, user ) ;
			// ����µ��û�
			if ( ! _online_user.AddUser( key, user ) ) {
				if ( user._fd != NULL ) {
					OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() ,
							"PasClient Add New user close fd %d" , user._fd->_fd ) ;
					CloseSocket( user._fd ) ;
				}
				_online_user.SetUser( key, user ) ;
			}
			// �����û���ƽ̨��ڵ�����
			_postquerymgr.LoadPostQuery( _postpath.c_str(), (int)user._access_code ) ;
		}
		break ;
	case USER_DELED:
		if ( ! user._user_id.empty() ) {
			if ( user._fd != NULL ) {
				OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() ,
						"PasClient Delete User fd %d" , user._fd->_fd ) ;
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
			if ( user._fd != NULL ) {
				OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str() ,
						"PasClient Change User close fd %d" , user._fd->_fd ) ;
				CloseSocket( user._fd ) ;
			}
			_online_user.SetUser( key, user ) ;
			// �����û���ƽ̨��ڵ�����
			_postquerymgr.LoadPostQuery( _postpath.c_str(), (int)user._access_code ) ;
		}
		break ;
	}
}

