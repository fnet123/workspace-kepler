/***********************************************************************
** Copyright (c)2009, ����ǧ���Ƽ��������޹�˾
** All rights reserved.
** 
** File name  : transmit_service.cpp
** Author     : Liubo
** Date       :
** Comments   : ����ȫ��ƽ̨��
***********************************************************************/
#include "transmit.h"
#include "ProtoHeader.h"
#include <tools.h>

Transmit::Transmit()
	: _filecache(this)
{
	_listen_port 		 = 0;
	//�����������ӵ�verify_code,�ݶ�123456��������ζ�����ʵ����������Ľ�
	_verify_code 		 = 123456;
	_client._socket_type = User::TcpConnClient;
	_client._connect_info.keep_alive = AlwaysReConn;
	_client._connect_info.last_reconnect_time = 0;
	_client._connect_info.timeval = 20;   //Ĭ��20s����һ�Ρ�

	_M1  = _IA1 = _IC1 = 0 ;
}

Transmit::~Transmit()
{
	Stop() ;
}


bool Transmit::Init(ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char value[512] = {0};
	if ( ! pEnv->GetString( "transmit_listen_ip", value) )
	{
		INFO_PRT("get transmit_listen_ip failed!\n");
		ERRLOG(NULL, 0, "get transmit_listen_ip failed!");
		return false ;
	}
	_down_ip = _listen_ip  = value ;

	// ���е�IP��Ҫ������IP����Ϊ0.0.0.0
	if ( pEnv->GetString("transmit_down_ip" , value ) ) {
		_down_ip = value ;
	}

	int port = 0 ;
	if ( ! pEnv->GetInteger( "transmit_listen_port", port ) )
	{
		INFO_PRT("get transmit_listen_port failed!\n");
		ERRLOG(NULL, 0, "get transmit_listen_port failed!");
		return false ;
	}
	_listen_port = port ;

	if ( ! pEnv->GetString( "transmit_connect_ip", value) )
	{
		INFO_PRT("get transmit_connect_ip failed!\n");
		ERRLOG(NULL, 0, "get transmit_connect_ip failed!");
		return false ;
	}
	_client._ip  = value ;

	if ( ! pEnv->GetInteger( "transmit_connect_port", port ) )
	{
		INFO_PRT("get transmit_connect_port failed!\n");
		ERRLOG(NULL, 0, "get transmit_connect_port failed!");
		return false ;
	}
	_client._port = port ;

	if ( ! pEnv->GetString( "user_id", value ) )
	{
		printf("get user_id failed!\n");
		ERRLOG(NULL, 0, "get user_id failed!");
		return false ;
	}
	_client._user_id = value ;
	_user_name 		=  atoi(value) ;

	if ( ! pEnv->GetString("user_password", value) )
	{
		printf("get user_password failed!\n");
		ERRLOG(NULL, 0, "get user_password failed!");
		return false ;
	}
	_user_password = value ;

	int code = 0 ;
	if ( ! pEnv->GetInteger( "access_code", code ) )
	{
		printf("get access_code failed!\n");
		ERRLOG(NULL, 0,0, "get access_code failed!");
		return false ;
	}
	_access_code 		 = code ;
	_client._access_code = code ;

	// ȡ����Կ
	if ( pEnv->GetInteger( "M1" , code ) ) {
		_M1 = code ;
	}
	if ( pEnv->GetInteger( "IA1" , code ) ) {
		_IA1 = code ;
	}
	if ( pEnv->GetInteger( "IC1" , code ) ) {
		_IC1 = code ;
	}
	printf( "m1 %d,ia1 %d,ic1 %d\n", _M1, _IA1, _IC1 ) ;

	return _filecache.Init( pEnv , "mas" ) ;
}

bool Transmit::Start( void )
{
	if ( ! _filecache.Start() ) {
		OUT_ERROR( NULL, 0, NULL,  "start filecache failed" ) ;
		return false ;
	}
	return StartServer( _listen_port, _listen_ip.c_str() , 1  ) ;
}

void Transmit::Stop( void )
{
	StopServer() ;
	_filecache.Stop() ;
}

bool Transmit::ConnectServer(User &user, unsigned int timeout)
{
	if(time(0) - user._connect_info.last_reconnect_time < user._connect_info.timeval)
		return false;

	bool ret = false;
	if ( user._fd != NULL ) {
		OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "Transmit::ConnectServer fd %d close socket",
				user._fd->_fd );
		CloseSocket( user._fd );
	}

	user._fd = _tcp_handle.connect_nonb( user._ip.c_str(), user._port, timeout );
	ret = ( user._fd != NULL ) ? true : false;

	if ( ret ) {
		//���͵�¼��
		UpConnectReq req;
		req.header.msg_seq = ntouv32( _proto_parse.get_next_seq());
		req.header.access_code = ntouv32(_access_code);
		req.user_id = ntouv32(_user_name);
		req.down_link_port = ntouv16(_listen_port);
		strncpy( ( char* ) ( req.down_link_ip ), _down_ip.c_str(), _down_ip.length() );
		strncpy( ( char * ) ( req.password ), _user_password.c_str(), 8 );
		// req.crc_code = ntouv16(GetCrcCode((const char*)&req,sizeof(req)));

		if ( SendCrcData( user._fd, ( const char* ) & req, sizeof ( req ) ) ) {
			OUT_CONN( user._ip.c_str(), user._port, user._user_id.c_str(), "connect success,send login message" );
		} else {
			OUT_ERROR( user._ip.c_str(), user._port, user._user_id.c_str(),
					"connect success,send login message failed" );
		}
		user._last_active_time = time( 0 );
		user._user_state = User::WAITING_RESP;
	} else {
		user._user_state = User::OFF_LINE;
	}
	user._last_active_time = time( 0 );
	user._login_time = time( 0 );
	user._connect_info.last_reconnect_time = time( 0 );
	if ( user._connect_info.keep_alive == ReConnTimes )
		user._connect_info.reconnect_times --;

	return ret;
}

// ���ܴ�������
bool Transmit::EncryptData( unsigned char *data, unsigned int len , bool encode )
{
	// ��Կ�Ƿ�Ϊ�����Ϊ�ղ���Ҫ����
	if ( _M1 == 0  && _IA1 == 0 && _IC1 == 0 ) {
		return false ;
	}

	Header *header = ( Header *) data ;
	// �Ƿ���Ҫ���ܴ���
	if ( ! header->encrypt_flag && ! encode ) {
		return false;
	}

	// ���Ϊ���ܴ���
	if ( encode ) {
		// ���ü��ܱ�־λ
		header->encrypt_flag =  1 ;
		// ��Ӽ�����Կ
		header->encrypt_key  =  ntouv32( CEncrypt::rand_key() ) ;
	}

	// ��������
	return CEncrypt::encrypt( _M1, _IA1, _IC1, (unsigned char *)data, (unsigned int) len ) ;
}

void Transmit::on_data_arrived( socket_t *sock, const void* data, int len)
{
	_recvstat.AddFlux( len ) ;

	C5BCoder coder ;
	if( !coder.Decode( (const char *) data, len ) )
	{
		OUT_WARNING( sock->_szIp, sock->_port, NULL, "Except packet header or tail");
		return;
	}

	const char *data_src = coder.GetData() ;
	const int   data_len = coder.GetSize() ;

	if (data_len < (int)sizeof(Header) + (int)sizeof(Footer))
	{
		OUT_ERROR( NULL, 0, NULL, "Transmit::on_data_arrived packet len error");
		return;
	}	

	Header *header = (Header *) data_src ;
	unsigned short msg_type = ntouv16(header->msg_type);
	unsigned int msg_len = ntouv32(header->msg_len);

	if (msg_len!=(unsigned int)data_len)
	{
		OUT_ERROR( NULL, 0, NULL, "Transmit::on_data_arrived packet len error" );
		return;	
	}
	
	unsigned int access_code = ntouv32(header->access_code);
	const char *ip    = sock->_szIp ;
	unsigned int port = sock->_port ;
	string str_access_code = uitodecstr(access_code);

	// ����ǰ����
//	OUT_HEX( NULL , 0 , str_access_code.c_str(), data_src , data_len ) ;
	// �����������
	EncryptData( (unsigned char *) coder.GetData(), (unsigned int) coder.GetSize(), false ) ;


	string msg_data = _proto_parse.Decoder((const char*)data_src , data_len ) ;
	OUT_RECV3( ip, port, str_access_code.c_str(), "%s", msg_data.c_str() ) ;
//	OUT_HEX( ip, port, str_access_code.c_str(), data_src, data_len ) ;

	time_t now = time(NULL) ;
	if( sock == _client._fd)
		_client._last_active_time = now ;
	else if( sock == _server._fd)
		_server._last_active_time = now ;

	//INFO_PRT("Parse a server packet, msg_type = %x \n",msg_type);
	//ȫ��ƽ̨����������
	if (msg_type == UP_CONNECT_RSP)
	{
		UpConnectRsp *resp = (UpConnectRsp*)(data_src);
		switch(resp->result)
		{
		case 0:
		{
			_client._user_state = User::ON_LINE;
			OUT_INFO(ip,port,str_access_code.c_str(),"login check success,access_code:%d  up-link ON_LINE",access_code);
			if ( _client._fd == sock ) _filecache.Online( _client._access_code ) ;
			break;
		}
		case 1:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,ip is invalid");
			break;
		case 2:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,accesscode is invalid,close it");
			break;
		case 3:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_name:%s is invalid,close it",_client._user_name.c_str());
			break;
		case 4:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_password:%s is invalid,close it",_user_password.c_str());
			break;
		default:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,other error,close it");
			break ;
		}
	}
	//������������·����������
	else if (msg_type == DOWN_CONNECT_REQ)
	{
		DownConnectReq* req = (DownConnectReq*) data_src;
		_server._fd         = sock ;
		_server._ip         = ip ;
		_server._port       = port ;
		_server._login_time = now ;
		_server._msg_seq    = 0;
		_server._user_state = User::ON_LINE;
		//һ��Ҫ�����������Ϊ��������·��û�н���������ʱ�򣬵�һ�ε�¼��ʱ���ϵ��Ǹ��ǲ���ִ�еġ�
		_server._last_active_time = now ;

		DownConnectRsp resp;
		resp.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
	
		resp.header.access_code = req->header.access_code;
		resp.result = 0;
		// resp.crc_code = GetCrcCode((const char*)&resp,sizeof(resp));

		if ( SendCrcData(_server._fd,(const char *)&resp,sizeof(resp) ) ) {
			OUT_INFO(ip,port,str_access_code.c_str(),"DOWN_CONNECT_REQ: send DOWN_CONNECT_RSP downlink is online");
		} else {
			OUT_ERROR(ip,port,str_access_code.c_str(),"DOWN_CONNECT_REQ: send DOWN_CONNECT_RSP downlink is online failed");
		}

		static bool bFirst = true;
		if (bFirst){
			bFirst = false;
			int iStandard_test = 0;
			_pEnv->GetInteger("standard_test", iStandard_test);
			if (iStandard_test == 1)
			{
				UpDisconnectInform test_req;
				test_req.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
				test_req.header.access_code = ntouv32(_access_code);
				test_req.header.msg_type = ntouv16(UP_DISCONNECT_INFORM);
				test_req.header.msg_len = ntouv32(sizeof(UpDisconnectInform));            
				test_req.error_code = 0;

				if ( SendCrcData(_server._fd,(const char *)&test_req,sizeof(test_req) ) ) {
					OUT_INFO(ip,port,str_access_code.c_str(),"UP_DISCONNECT_INFORM successed");
				} else {
					OUT_ERROR(ip,port,str_access_code.c_str(),"UP_DISCONNECT_INFORM failed");
				}
			}
            
			if (iStandard_test == 1)
			{
				UpCloselinkInform test_req;
				test_req.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
				test_req.header.access_code = ntouv32(_access_code);
				test_req.header.msg_type = ntouv16(UP_CLOSELINK_INFORM);
				test_req.header.msg_len = ntouv32(sizeof(UpCloselinkInform));
				test_req.reason_code = 0;

				if ( SendCrcData(_server._fd,(const char *)&test_req,sizeof(test_req) ) ) {
					OUT_INFO(ip,port,str_access_code.c_str(),"UP_CLOSELINK_INFORM successed");
				} else {
					OUT_ERROR(ip,port,str_access_code.c_str(),"UP_CLOSELINK_INFORM failed");
				}
			}

			if (iStandard_test == 1)
			{
				//  ��������·ע������
				UpDisconnectReq test_req;

				test_req.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
				test_req.header.access_code = ntouv32(_access_code);
				test_req.user_id = ntouv32(_user_name);
				safe_memncpy( (char*)test_req.password,  _user_password.c_str(), sizeof(test_req.password) ) ;
				//    strncpy((char *)(test_req.password),_user_password.c_str(),8);

				if ( SendCrcData(_client._fd,(const char *)&test_req,sizeof(test_req) ) ) {
					OUT_INFO(ip,port,str_access_code.c_str(),"UP_DISCONNECT_REQ successed");
				} else {
					OUT_ERROR(ip,port,str_access_code.c_str(),"UP_DISCONNECT_REQ failed");
				}
			}
		}
	}
	else if (msg_type == UP_DISCONNECT_RSP)
	{
		OUT_RECV3( ip, port, "mas", "Recv UP_DISCONNECT_RSP");

		CloseSocket(_client._fd);
		_client._fd = NULL;
	}
	else if(msg_type == UP_LINKTEST_REQ)
	{
//		UpLinkTestRsp resp;
//		resp.header.access_code = header->access_code;
//		resp.header.msg_seq = ntouv32(_proto_parse.get_next_seq());
//		SendCrcData(fd,(const char*)&resp,sizeof(resp));
		return ;
	}
	else if(msg_type == UP_LINKTEST_RSP)
	{
        //  ����ʱʹ�õĴ���,����·������������·�Ͽ���Ϣ
		OUT_INFO( ip, port, str_access_code.c_str(), "UP_LINKTEST_RSP" ) ;
	}
	else if(msg_type == DOWN_LINKTEST_REQ)
	{
		DownLinkTestRsp resp;
		resp.header.access_code = header->access_code;
		resp.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
		// resp.crc_code = ntouv16(GetCrcCode((const char*)&resp,sizeof(resp)));
		if ( SendCrcData( sock, (const char*)&resp, sizeof(resp) ) ) {
			OUT_INFO( ip, port, str_access_code.c_str(), "DownLinkTestRsp: success" ) ;
		} else {
			OUT_ERROR( ip , port, str_access_code.c_str() , "DownLinkTestRsp: failed" ) ;
		}
	}
	else if (msg_type == DOWN_LINKTEST_RSP)
	{
	}
	else if (msg_type == DOWN_DISCONNECT_RSP)
	{
	}
	else if (msg_type == DOWN_DISCONNECT_INFORM)
	{
		//���������ɶ�ʱ�߳�����ɡ�
		OUT_RECV3( ip, port, "mas", "Recv DOWN_DISCONNECT_INFORM");

		DownDisconnectInform * pReq = (DownDisconnectInform *)(data_src);

		if (pReq->error_code == (unsigned char)0){
			OUT_ERROR( ip, port, "mas", "DOWN_DISCONNECT_INFORM: �޷������¼�ƽָ̨���ķ���IP��˿�" ) ;
		}
		else if (pReq->error_code == (unsigned char)1){
		    OUT_ERROR( ip, port, "mas", "DOWN_DISCONNECT_INFORM:�ϼ�ƽ̨�ͻ������¼�ƽ̨����˶Ͽ�");
		}
		else if (pReq->error_code == (unsigned char)2){
		    OUT_ERROR( ip, port, "mas", "DOWN_DISCONNECT_INFORM:����ԭ��");
		}
		else {
		    OUT_ERROR( ip, pReq->error_code, "mas", "DOWN_DISCONNECT_INFORM:δ���������");
		}
	}
	else if (msg_type == DOWN_CLOSELINK_INFORM)
	{
		//  �ϼ�ƽ̨�����ر�������·֪ͨ��Ϣ
		OUT_RECV3( ip, port, "mas", "Recv DOWN_CLOSELINK_INFORM" ) ;
        
		DownCloselinkInform * pReq = (DownCloselinkInform *)(data_src);
		switch(pReq->reason_code){
		case 0:
			OUT_ERROR( ip, port, "mas", "DOWN_CLOSELINK_INFORM:��������") ;            
		    break;            
		case 1:
			OUT_ERROR( ip, port, "mas", "DOWN_CLOSELINK_INFORM:����ԭ��") ;            
		    break;
		default:
		    break;
		}
	}
	else if (msg_type == DOWN_DISCONNECT_REQ)
	{
		OUT_RECV3( ip, port, "mas", "Recv DOWN_DISCONNECT_REQ" ) ;
        
		DownDisconnectRsp req;
		req.header.access_code = header->access_code;
		req.header.msg_seq = ntouv32( _proto_parse.get_next_seq() );
		req.header.msg_len = ntouv32(sizeof(DownDisconnectRsp));
		req.header.msg_type = ntouv16(DOWN_DISCONNECT_RSP);

		if ( SendCrcData(_server._fd,(const char *)&req,sizeof(req) ) ) {
			OUT_INFO(ip,port,str_access_code.c_str(),"DOWN_DISCONNECT_RSP successed");
		} else {
			OUT_ERROR(ip,port,str_access_code.c_str(),"DOWN_DISCONNECT_RSP failed");
		}
	}
	else
	{
		if (msg_type == UP_EXG_MSG)
		{
			// ExgMsgHeader *msg_header = (ExgMsgHeader*) (data_src + sizeof(Header));
		}
		else if(msg_type == DOWN_EXG_MSG)
		{
			//�������Ҫ��������ݡ�
			ExgMsgHeader *msg_header = (ExgMsgHeader*) (data_src + sizeof(Header));

			if (msg_header->vehicle_no[0]==(char)0)
			{
				OUT_ERROR(ip,port,str_access_code.c_str(),"vehicle is null");
				return;
			}
			switch(ntouv16(msg_header->data_type))
			{
			case DOWN_EXG_MSG_CAR_INFO: //4.5.3.2.4 ����������̬��Ϣ��Ϣ  �ϼ�ƽ̨�����·��������ľ�̬��Ϣ
			case DOWN_EXG_MSG_CAR_LOCATION:
				break ;

			case DOWN_EXG_MSG_RETURN_STARTUP://��ƽ̨�жϿ����·�
			{
				OUT_RECV3( ip, port, "mas", "Recv DOWN_EXG_MSG_RETURN_STARTUP" ) ;
				//  ���������ƺų־û�
				char vehicle_no[23] = {0} ;
				safe_memncpy( vehicle_no, msg_header->vehicle_no, sizeof(msg_header->vehicle_no) ) ;
				_filecache.AddArcoss( _access_code, msg_header->vehicle_color, vehicle_no ) ;
				break ;
			}
			case DOWN_EXG_MSG_RETURN_END:
			{
				OUT_RECV3( ip, port, "mas", "Recv DOWN_EXG_MSG_RETURN_END" ) ;
				//  ȡ�����������ƺ�
				char vehicle_no[23] = {0} ;
				safe_memncpy( vehicle_no, msg_header->vehicle_no, sizeof(msg_header->vehicle_no) ) ;
				_filecache.DelArcoss( _access_code, msg_header->vehicle_color, vehicle_no ) ;
				break ;
			}
			case DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK:
			case DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK://4.5.3.2.7 ���뽻��ָ��������λ��ϢӦ����Ϣ
				break ;
			case DOWN_EXG_MSG_HISTORY_ARCOSSAREA:
			case DOWN_EXG_MSG_REPORT_DRIVER_INFO:
			case DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK://�ϼ�ƽ̨��Ӧ
			case DOWN_EXG_MSG_TAKE_WAYBILL_REQ: //4.5.3.2.11 �ϱ����������˵�������Ϣ
				break;
			default:
				break ;
			}
		}
		else if (msg_type == UP_PLATFORM_MSG)
		{

		}
		else if(msg_type == DOWN_PLATFORM_MSG)//4.5.4.2 ����·ƽ̨����Ϣ����ҵ��
		{
		}
		else if (msg_type == UP_WARN_MSG_ADPT_INFO)
		{
		}
		else if(msg_type == DOWN_WARN_MSG)
		{
			//��ƽ̨���еı�����������ֱ��ת����pas
		}
		else if (msg_type == UP_CTRL_MSG)
		{

		}
		else if(msg_type == DOWN_CTRL_MSG)
		{

		}
		else if (msg_type == UP_BASE_MSG)
		{

		}
		else if(msg_type == DOWN_BASE_MSG)
		{
		}
		else if(msg_type == DOWN_TOTAL_RECV_BACK_MSG)//4.5.2.1 ���ճ�����λ��Ϣ����֪ͨ��Ϣ
		{
		}
		else
		{
			OUT_ERROR(ip,port,str_access_code.c_str(),"msg_type=%04x,received an invalid message",msg_type);
		}
		_pEnv->GetPasServer()->HandleClientData( coder.GetData(), coder.GetSize() ) ;
	}
}

void Transmit::on_dis_connection( socket_t *sock )
{
	if ( sock == _client._fd )
	{
		_client._user_state = User::OFF_LINE;
		OUT_WARNING(_client._ip.c_str(),_client._port,_client._user_id.c_str(),"uplink:on_dis_connnection");
		_filecache.Offline( _client._access_code ) ;
	}
	else if ( sock == _server._fd )
	{
		_server._user_state = User::OFF_LINE;
		OUT_WARNING(_server._ip.c_str(),_server._port,_server._user_id.c_str(),"downlink:on_dis_connnection");
	}
}

// ��������������Ǵ��ڲ���������ļ������������Ҫ�������
void Transmit::HandleMasUpData( const char *data, int len )
{
	if ( len < (int)sizeof(Header) + (int)sizeof(Footer)){
		OUT_ERROR( NULL, 0, NULL, "Transmit::HandleMasUpData packet len error" );
		return;
	}
	
	//��CAS���ϴ����������ݵ�seq��access_code��������
	Header *header 			 = (Header *) data ;
	unsigned short msg_type  = ntouv16(header->msg_type);
	header->msg_seq 		 = ntouv32( _proto_parse.get_next_seq() );
	header->access_code 	 = ntouv32(_access_code);

	// ����·�ʧ��
	if ( ! SendMasData( data, len ) ) {
		//  �ж���Ϣ���Ƿ������λ��Ϣ
		if ( msg_type == UP_EXG_MSG ) {
			_filecache.WriteCache( (int)_client._access_code, (void*)data, len ) ;
		}
		return ;
	}
	OUT_SEND3(_client._ip.c_str(),_client._port,_client._user_id.c_str(),"%s", _proto_parse.Decoder( data, len ).c_str());
}

// ����MAS������
bool Transmit::SendMasData( const char *data, int len )
{
	if ( _client._user_state != User::ON_LINE && _server._user_state != User::ON_LINE ) {
		return false ;
	}

	socket_t *sock = NULL ;
	// ��ԭ��ԭ���汾����������·��������
	if ( _client._user_state == User::ON_LINE ) {
		sock = _client._fd;
	} else if(_server._user_state == User::ON_LINE) {
		sock = _server._fd;
	}

	if ( ! SendCrcData( sock, data , len ) ) {
		return false ;
	}
	// ����ϴ�����ͳ��
	_sendstat.AddFlux( len ) ;
	return true ;
}

void Transmit::TimeWork()
{
	OUT_INFO( NULL, 0, NULL, "void	Transmit::TimeWork()");

	time_t cur_time;
	time_t last_noop_time = time(NULL);
	time_t last_running   = time(NULL) ;
	while (1)
	{
		if ( ! Check() ) break ;

		/*ÿ����������һ�ζ�ʱ��⣬���ǳ�ʱ�ģ��ĶϿ���������*/
		cur_time = time(0);

		bool conn = false;
		//as server
		if (_client._user_state != User::OFF_LINE && cur_time - _client._last_active_time > 3*60)
		{
			OUT_WARNING(NULL,0,"TimeWork","uplink timeout");
			_client._user_state = User::OFF_LINE;
		}

		if (_server._user_state != User::OFF_LINE && cur_time - _server._last_active_time > 3*60)
		{
			OUT_WARNING(NULL,0,"TimeWork","downlink timeout");
			_server._user_state = User::OFF_LINE;
		}

		if(_client._user_state == User::ON_LINE && cur_time - last_noop_time > 30) {
			//����NOOP��Ϣ��
			UpLinkTestReq req;
			req.header.access_code = ntouv32(_access_code);
			req.header.msg_seq = ntouv32(_proto_parse.get_next_seq());
			if ( SendCrcData(_client._fd,(const char*)&req,sizeof(req)) ) {
				last_noop_time = cur_time ;
				OUT_INFO( _client._ip.c_str() , _client._port ,  NULL, "UP_LINKTEST_REQ" ) ;
			} else {
				OUT_ERROR( _client._ip.c_str() , _client._port ,  NULL, "UP_LINKTEST_REQ" ) ;
			}
		} else if( _client._user_state == User::OFF_LINE && cur_time - _client._login_time > 30 ) {

			if ( _client._fd != NULL ) {
				OUT_WARNING(_client._ip.c_str(), _client._port, _client._user_id.c_str(),
						"Client %d close socket", _client._fd->_fd );
				CloseSocket(_client._fd);
			}
			_client._fd = NULL;
			_client._user_state = User::OFF_LINE;
			_client._msg_seq = 0;
			_client._last_active_time = cur_time;
			_client._connect_info.keep_alive = AlwaysReConn;
			if(cur_time - _client._connect_info.last_reconnect_time > _client._connect_info.timeval){
				ConnectServer(_client,10);
			}
		}

		if(_server._user_state == User::OFF_LINE && _server._fd != NULL ) {
			OUT_INFO( _server._ip.c_str() , _server._port, _server._user_id.c_str(),
					"Server %d close socket", _server._fd->_fd );
			CloseSocket(_server._fd);
			_server._fd = NULL;
		}
		if(_server._user_state == User::ON_LINE || _client._user_state == User::ON_LINE)
			conn = true;

		if ( cur_time - last_running > 30  ) {

			_recvstat.Check( 30 ) ;
			_sendstat.Check( 30 ) ;

			last_running = cur_time ;
			float count = 0, speed = 0 ;
			_recvstat.GetFlux( count, speed ) ;

			float nsend = 0, nspeed = 0 ;
			_sendstat.GetFlux( nsend, nspeed ) ;

			OUT_RUNNING( NULL, 0, "ONLINE", "mas recv down count %f, speed %fkb, up count %f, speed %fkb",
					count, (float)speed/(float)DF_KB , nsend, (float)nspeed/(float)DF_KB ) ;
		}

		sleep(3);
	}
}

void Transmit::NoopWork()
{

}

bool Transmit::SendCrcData( socket_t *sock, const char* data, int len )
{
	// ����ѭ����
	char *buf = new char[len+1] ;
	memset( buf, 0 , len+1 ) ;
	memcpy( buf, data, len ) ;

	// �����������
	EncryptData( (unsigned char *) buf , (unsigned int) len , true ) ;

	// ͳһ����ѭ�������֤
	unsigned short crc_code = ntouv16( GetCrcCode( buf, len ) ) ;
	unsigned int   offset   = len - sizeof(Footer) ;
	// �滻ѭ�����ڴ��λ������
	memcpy( buf + offset , &crc_code, sizeof(short) ) ;

	C5BCoder coder ;
	coder.Encode( buf , len ) ;

	delete [] buf ;
	//OUT_HEX( sock->_szIp, sock->_port, "Send", coder.GetData(), coder.GetSize() ) ;

	return SendData( sock, coder.GetData(), coder.GetSize() ) ;
}

//-----------------------------�������ݴ���ӿ�-----------------------------------------
// �����ⲿ����
int Transmit::HandleQueue( const char *uid , void *buf, int len , int msgid )
{
	OUT_PRINT( NULL, 0, NULL, "HanldeQueue msg id %d , accesscode %s , data length %d" , msgid, uid, len ) ;

	switch( msgid ) {
	case DATA_FILECACHE:  // �û����������ݻ���
		{
			// ������ݳ��Ȳ���ȷֱ�ӷ���
			if ( len < (int)sizeof(Header) ) {
				OUT_ERROR( NULL, 0, NULL, "HandleQueue filecache data length %d error", len ) ;
				return IOHANDLE_ERRDATA;
			}

			// OUT_INFO( NULL, 0, NULL, "HandleQueue %s" , _proto_parse.Decoder( (const char *)buf, len ).c_str() ) ;

			Header *header = (Header *) buf;
			unsigned short msg_type  = ntouv16(header->msg_type);
			// �����Ϊ��չ����Ϣ
			if ( msg_type != UP_EXG_MSG ) {
				return IOHANDLE_ERRDATA;
			}

			// ���Ϊ��չ����Ϣ
			ExgMsgHeader *exg = (ExgMsgHeader*) ((const char *)(buf) + sizeof(Header) ) ;
			unsigned short data_type = ntouv16( exg->data_type ) ;
			if ( data_type != UP_EXG_MSG_REAL_LOCATION ) {
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

			if ( ! SendMasData( dbuf.getBuffer(), dbuf.getLength() ) ) {
				OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_HISTORY_LOCATION:%s" , msg.exg_msg_header.vehicle_no ) ;
				return IOHANDLE_FAILED ;
			}
			// ����ϴ�����ͳ��
			_sendstat.AddFlux( dbuf.getLength() ) ;

			OUT_SEND3( NULL, 0, NULL, "UP_EXG_MSG_HISTORY_LOCATION:%s" , msg.exg_msg_header.vehicle_no ) ;
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
			req.header.msg_seq  = ntouv32( _proto_parse.get_next_seq()) ;
			req.header.msg_type = ntouv16( UP_EXG_MSG ) ;
			req.header.access_code = ntouv32( _access_code ) ;
			req.exg_msg_header.vehicle_color = p->color ;
			safe_memncpy( req.exg_msg_header.vehicle_no, p->vechile, sizeof(req.exg_msg_header.vehicle_no) ) ;
			req.exg_msg_header.data_length   = ntouv32( sizeof(uint64) * 2 ) ;
			req.exg_msg_header.data_type	 = ntouv16( UP_EXG_MSG_APPLY_HISGNSSDATA_REQ ) ;
			req.start_time = ntouv64( p->time ) ;
			req.end_time   = ntouv64( time(NULL) ) ;

			if ( ! SendMasData( (const char *)&req, sizeof(req) ) ) {
				OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_APPLY_HISGNSSDATA_REQ:%s" , p->vechile ) ;
				return IOHANDLE_FAILED ;
			}

			OUT_SEND3( NULL, 0, NULL, "UP_EXG_MSG_APPLY_HISGNSSDATA_REQ:%s" , p->vechile ) ;
		}
		break;
	}

	return IOHANDLE_SUCCESS ;
}



