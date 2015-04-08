/**********************************************
 * PasServer.cpp
 *
 *  Created on: 2011-08-04
 *      Author: humingqing
 *       Email:qshuihu@gmail.com
 *    Comments: ��Ҫʵ�־ɵ���Ӫ��Э��Ĵ���Ȼ��ת�����¹���Э�齻�����ƽ̨���д���
 *********************************************/

#include "passerver.h"
#include <BaseTools.h>
#include "ProtoHeader.h"
#include "ProtoParse.h"
#include "crc16.h"
#include <tools.h>
#include "comlog.h"

#define LBS_CARAREA    "KCTX.PLATE2SIM"  // ���ƺ����ֻ��Ŷ�Ӧ��ϵ��hash��
#define LBS_MPASCODE   "lbs.code.mpas"   // ��Ӫ�̽��������

PasServer::PasServer( PConvert *convert ): _convert( convert )
{
	//�����������ӵ�verify_code,�ݶ�123456��������ζ�����ʵ����������Ľ�
	_thread_num            = 10 ;
	_last_handle_user_time = time(NULL) ;
	_verify_code           = 1234;
	_rootpath			   = "/usr/local/lbs/scp" ;
}

PasServer::~PasServer()
{
	Stop() ;
}

// ��ʼ��
bool PasServer::Init(ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	int thread = 0 ;
	if ( ! _pEnv->GetInteger("mpas_recv_thread" , thread ) ){
		thread = 10 ;
	}
	_thread_num = thread ;

	char value[1024] = {0} ;
	if ( ! _pEnv->GetString("mpas_listen_ip", value) )
	{
		printf("get mpas_listen_ip failed!\n");
		return false ;
	}
	_ip = value ;

	int port = 0 ;
	if ( !_pEnv->GetInteger("mpas_listen_port", port ) )
	{
		printf("get mpas_listen_port failed!\n");
		return false ;
	}
	_port = port ;
	// ȡ��ͼƬ����·��
	if ( pEnv->GetString( "picsave_dir" , value ) ) {
		_rootpath = value ;
	}

	return true ;
}

// ��ʼ�߳�
bool PasServer::Start( void )
{
	OUT_INFO(NULL, 0, NULL, "bool PasServer::Start( void ))");

	return StartServer( _port, _ip.c_str() , _thread_num ) ;
}

// ֹͣ����
void PasServer::Stop( void )
{
	StopServer() ;
}

bool PasServer::EncryptData( unsigned char *data, unsigned int len , bool encode )
{
	Header *header = ( Header *) data ;
	// �Ƿ���Ҫ���ܴ���
	if ( ! header->encrypt_flag && ! encode ) {
		return false;
	}

	unsigned int access = ntouv32(header->access_code) ;
	unsigned int M1 = 0 , IA1 = 0 , IC1 = 0 ;
	// ȡ�ö�Ӧ�û��Ľ�����Կ
	if ( ! _keymgr.GetEncryptKey( access , M1 , IA1 , IC1 ) ) {
		// ����û�û��Կ�Ͳ���Ҫ���������
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


void PasServer::on_data_arrived( socket_t *sock, const void* data, int len)
{
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	C5BCoder coder ;
	if ( ! coder.Decode( (const char *)data, len ) )
	{
		OUT_WARNING( ip , port , NULL, "Except packet header or tail" ) ;
		OUT_HEX( ip, port, NULL, (const char *) data, len ) ;
		return;
	}

	HandleOnePacket( sock,(const char*)coder.GetData() , coder.GetSize() );
}

// �����ɵ��ϴ�����ת����Э���ת����PPAS
void PasServer::HandleOnePacket( socket_t *sock,  const char *data, int len )
{
	if (data == NULL || len < (int)sizeof(Header))
		return;

	const char *ip    = sock->_szIp ;
	unsigned int port = sock->_port ;

	Header *header = (Header *) data;
	unsigned int nlen  = ntouv32(header->msg_len) ;
	// ������Ϣ���ݳ��Ȳ���ȷ�����COREDUMP
	if ( nlen != len ) {
		OUT_ERROR( ip, port, NULL, "recver data length %d, message length %d" , len,nlen ) ;
		OUT_HEX( ip, port, "pas", data, len ) ;
		return ;
	}
	// �Ƿ���Ҫ���ܴ���
	if ( EncryptData( (unsigned char *)data, len , false ) ) {
		// �����ܱ�־���
		header->encrypt_flag = 0 ;
	}
	
	unsigned int access_code    = ntouv32(header->access_code);
	string str_access_code      = uitodecstr(access_code);
	unsigned short msg_type     = ntouv16(header->msg_type);

	OUT_RECV3( ip, port, str_access_code.c_str(), "%s, message length %d, length %d", get_type(msg_type) , nlen, len );
	OUT_HEX( ip, port, str_access_code.c_str(), data, len ) ;
	if ( msg_type == UP_CONNECT_REQ )
	{
		if ( header->encrypt_flag ) {
			// ������ݻ����ڼ���״̬���Ϳ��ܵ�һ�μ���û�д�����Կ��ԭ��,�ȼ���һ����Կ�ٽ�һ������
			CheckLogin( access_code ) ;
			// �Ƿ���Ҫ���ܴ���
			if ( EncryptData( (unsigned char *)data, len , false ) ) {
				// �����ܱ�־���
				header->encrypt_flag = 0 ;
			}
		}

		User user = _online_user.GetUserByUserId("U_" + str_access_code);
		UpConnectReq *out_login = (UpConnectReq *) data;
		OUT_RECV(ip,port,str_access_code.c_str(),"fd %d, UP_CONNECT_REQ,down-link listen port:%d", sock->_fd, ntouv16(out_login->down_link_port));

		//�ж��ǲ���ͬһ��socket��������UP_CONNECT_REQ��
		if (!user._user_id.empty())
		{
			//˵�������б������д��û���
			if (user._fd == sock)
				return;
			else
			{
				OUT_WARNING(ip,port,str_access_code.c_str(),"fd %d, one user already login", sock->_fd );
				CloseSocket( sock );
				return;
			}
		}

		string user_name = uitodecstr(ntouv32(out_login->user_id));
		char password[16] = {0};
		strncpy(password,(const char*)out_login->password,8);
		string user_password(password);

		unsigned char ret = CheckLogin( access_code , user_name.c_str() , user_password.c_str() );
		UpConnectRsp resp;
		resp.header.access_code = ntouv32(access_code);
		resp.header.msg_len  = ntouv32(sizeof(UpConnectRsp));
		resp.header.msg_type = ntouv16(UP_CONNECT_RSP);
		resp.header.msg_seq  = ntouv32(_convert->get_next_seq());
		resp.verify_code 	 = ntouv32(_verify_code);
		resp.result 		 = ret;

		switch(ret)
		{
		case 0:
			OUT_INFO(ip,port,str_access_code.c_str(),"login check success,access_code:%d  up-link ON_LINE",access_code);
			break;
		case 1:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,ip is invalid");
			break;
		case 2:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,accesscode is invalid,close it");
			break;
		case 3:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_name:%d is invalid,close it",out_login->user_id);
			break;
		case 4:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,user_password:%s is invalid,close it",password);
			break;
		default:
			OUT_WARNING(ip,port,str_access_code.c_str(),"login check fail,other error,close it");
			break;
		}
		// ���ڵ�½�����ӵ���Ӧ����
		if ( SendCrcData( sock, (const char*)&resp, sizeof(resp) ) )
			OUT_SEND(ip,port,str_access_code.c_str(),"UP_CONNECT_RSP") ;
		else
			OUT_ERROR(ip,port,str_access_code.c_str(),"UP_CONNECT_RSP") ;

		//˯��һ����ȴ����ݷ��ͳ�ȥ��
		user._access_code = access_code;
		user._user_name = user_name;
		user._user_id = "U_" + str_access_code; //������������·
		user._login_time = time(0);

		user._socket_type = User::TcpClient;
		user._fd = sock;
		user._ip = ip;
		user._port = port;
		user._last_active_time = time(0);

		unsigned short down_link_port = ntouv16( out_login->down_link_port ) ;
		string down_ip((char*)out_login->down_link_ip,32);
		OUT_CONN( ip, port, str_access_code.c_str() , "fd %d, down connection down ip %s , down port %d",
				sock->_fd, down_ip.c_str(), down_link_port ) ;
		if( ret == 0 )
		{
			user._user_state = User::ON_LINE;
			User down_user = _online_user.GetUserByUserId("D_" + str_access_code);
			if ( ! down_user._user_id.empty() ) {  // �����������л���ַ��BUG
				if ( down_user._fd != NULL ) { // ����ѽ���������ر�
					CloseSocket( down_user._fd ) ;
				}
			}
			if ( down_link_port > 0 && check_addr( down_ip.c_str() ) )
			{
				//˵��������·��û�н���������
				down_user._user_id = "D_" + str_access_code;
				down_user._access_code = access_code;
				down_user._user_name = user_name;
				down_user._login_time = time(0);

				down_user._socket_type = User::TcpConnClient;
				down_user._fd   = NULL ;
				down_user._user_state = User::OFF_LINE;
				down_user._ip   = down_ip ;
				down_user._port = down_link_port ;
				down_user._last_active_time = time(0);

				//������·ֻ����3�Ρ���������������ʱ�ͽ����user erase����
				down_user._connect_info.keep_alive = ReConnTimes;
				down_user._connect_info.last_reconnect_time = 0;
				down_user._connect_info.timeval = 0;
				down_user._connect_info.reconnect_times = 3;
				down_user._connect_info.timeval = 60 ;

				// ConnectServer(down_user, 10);

				if ( ! _online_user.AddUser(down_user._user_id,down_user) ) {
					// ����Ѵ�����ֱ���滻ԭ���ĻỰ
					_online_user.SetUser( down_user._user_id, down_user ) ;
				}
			}
			// ֻ��½�ɹ�����ӵ��û�������
			_online_user.AddUser(user._user_id ,user);
		}
		else
			user._user_state = User::OFF_LINE;

		user._last_active_time = time(0);
		_online_user.SetUser(user._user_id,user);

		return;
	}

	User user = _online_user.GetUserBySocket( sock );
	if ( user._user_id.empty() )
	{
		OUT_ERROR(ip,port,str_access_code.c_str(),"PasServer user havn't login,close it %d" , msg_type) ;
		CloseSocket( sock ) ;
		return;
	}

	//δ��¼�ɹ��������������������������
	if (msg_type == UP_DISCONNECT_REQ)
	{
		OUT_RECV(ip,port,user._user_id.c_str(),"UP_DISCONNECT_REQ");

		//  ���ʹ���·ע������
		DownDisconnectReq req;
		req.header.msg_type = ntouv16(DOWN_DISCONNECT_REQ);
		req.header.msg_len = ntouv32(sizeof(DownDisconnectReq));
		req.header.msg_seq = ntouv32(_convert->get_next_seq());
		req.header.access_code = header->access_code;
		req.verify_code = ntouv32(_verify_code);

		char buf[200];
		sprintf(buf,"D_%u",user._access_code);
		string user_id = string(buf);
		//  ȡ����·fd
		User sub_user = _online_user.GetUserByUserId(user_id);

		if (!sub_user._user_id.empty())
		{
			if( SendCrcData(sub_user._fd, (const char*) &req, sizeof(req) ) )
			{
				OUT_SEND(sub_user._ip.c_str(),sub_user._port,user._user_id.c_str(),"DOWN_DISCONNECT_REQ");
			}
			else
				OUT_ERROR(sub_user._ip.c_str(),sub_user._port,user._user_id.c_str(),"DOWN_DISCONNECT_REQ") ;
		}

		UpDisconnectRsp resp;
		resp.header.msg_seq = ntouv32(_convert->get_next_seq());
		resp.header.access_code = header->access_code;

		if( SendCrcData( sock, (const char*) &resp, sizeof(resp) ) )
			OUT_SEND(ip,port,user._user_id.c_str(),"UP_DISCONNECT_RSP");
		else
			OUT_ERROR(ip,port,user._user_id.c_str(),"UP_DISCONNECT_RSP") ;
	}
	else if (msg_type == UP_LINKTEST_REQ)
	{
		OUT_RECV(ip,port,user._user_id.c_str(),"UP_LINKTEST_REQ");
		UpLinkTestRsp resp;
		resp.header.access_code = header->access_code;
		resp.header.msg_seq 	= ntouv32(_convert->get_next_seq());

		if ( SendCrcData( sock, (const char*) &resp, sizeof(resp) ) )
			OUT_SEND(ip,port,user._user_id.c_str(),"UP_LINKTEST_RSP");
		else
			OUT_ERROR(ip,port,user._user_id.c_str(),"UP_LINKTEST_RSP") ;
	}
	else if (msg_type == UP_CLOSELINK_INFORM)
	{
		//  ����Ҫ�����ɶ�ʱ�������̽��
		OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, UP_CLOSELINK_INFORM", sock->_fd );

		char buf[200];
		sprintf(buf,"U_%u",user._access_code);
		string user_id = string(buf);
		//  ȡ����·fd
		User main_user = _online_user.GetUserByUserId(user_id);
		if (!main_user._user_id.empty())
		{
			{
				DownDisconnectInform req;
				req.header.access_code = ntouv32(user._access_code);
				req.header.msg_len = ntouv32(sizeof(DownDisconnectInform));
				req.header.msg_type = ntouv16(DOWN_DISCONNECT_INFORM);
				req.header.msg_seq 	= ntouv32(_convert->get_next_seq());

				if ( SendCrcData(main_user._fd, (const char*) &req, sizeof(req) ) )
					OUT_SEND(ip,port,user._user_id.c_str(),"fd %d, DOWN_DISCONNECT_INFORM", sock->_fd );
				else
					OUT_ERROR(ip,port,user._user_id.c_str(),"fd %d, DOWN_DISCONNECT_INFORM", sock->_fd );
			}

			{
				DownCloselinkInform req;
				req.header.access_code = ntouv32(user._access_code);
				req.header.msg_len = ntouv32(sizeof(DownCloselinkInform));
				req.header.msg_type = ntouv16(DOWN_CLOSELINK_INFORM);
				req.header.msg_seq 	= ntouv32(_convert->get_next_seq());

				if ( SendCrcData(main_user._fd, (const char*) &req, sizeof(req) ) )
					OUT_SEND(ip,port,user._user_id.c_str(),"fd %d, DOWN_CLOSELINK_INFORM", sock->_fd );
				else
					OUT_ERROR(ip,port,user._user_id.c_str(),"fd %d, DOWN_CLOSELINK_INFORM", sock->_fd );
			}

			main_user._last_active_time = time(0);
			_online_user.SetUser(main_user._user_id,main_user);

			user._user_state = User::OFF_LINE;

		 //   main_user._user_state = User::OFF_LINE;
		 }
	}
	else if (msg_type == UP_DISCONNECT_INFORM)
	{
		//  ����Ҫ�����ɶ�ʱ�������̽��
		OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, UP_DISCONNECT_INFORM", sock->_fd );
	}
	else if (msg_type == UP_CLOSELINK_INFORM)
	{
		//  ����Ҫ�����ɶ�ʱ�������̽��
		OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, UP_CLOSELINK_INFORM", sock->_fd );
		//  ȡ����·fd
		{
			DownDisconnectInform req;
			req.header.msg_len		= ntouv32( sizeof(req) ) ;
			req.header.msg_type		= ntouv16( DOWN_DISCONNECT_INFORM ) ;
			req.header.access_code 	= ntouv32(user._access_code);
			req.header.msg_seq 		= ntouv32(_convert->get_next_seq());

			if ( SendCrcData( sock, (const char*) &req, sizeof(req) ) )
				OUT_SEND(ip,port,user._user_id.c_str(),"fd %d, DOWN_DISCONNECT_INFORM", sock->_fd );
			else
				OUT_ERROR(ip,port,user._user_id.c_str(),"fd %d, DOWN_DISCONNECT_INFORM", sock->_fd );
		}

		{
			DownCloselinkInform req;
			req.header.msg_len 		= ntouv32( sizeof(req) ) ;
			req.header.msg_type     = ntouv16( DOWN_CLOSELINK_INFORM ) ;
			req.header.access_code 	= ntouv32(user._access_code);
			req.header.msg_seq 		= ntouv32(_convert->get_next_seq());

			if ( SendCrcData( sock , (const char*) &req, sizeof(req) ) )
				OUT_SEND(ip,port,user._user_id.c_str(),"fd %d, DOWN_CLOSELINK_INFORM", sock->_fd );
			else
				OUT_ERROR(ip,port,user._user_id.c_str(),"fd %d, DOWN_CLOSELINK_INFORM", sock->_fd );
		}
	}
	else if (msg_type == UP_DISCONNECT_INFORM)
	{
		//  ����Ҫ�����ɶ�ʱ�������̽��
		OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, UP_DISCONNECT_INFORM", sock->_fd );
	}
	else if (msg_type == DOWN_CONNECT_RSP)
	{
		DownConnectRsp *_down_resp = (DownConnectRsp*) data;
		if (_down_resp->result == 0){
			OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, DOWN_CONNECT_RSP,Down-Link ON_LINE", sock->_fd );
			user._connect_info.reconnect_times = 3;
			user._user_state = User::ON_LINE;
		} else {
			OUT_RECV(ip,port,user._user_id.c_str(),"fd %d, DOWN_CONNECT_RSP,connect fail,result=%d", sock->_fd, _down_resp->result);
		}
	}
	else if (msg_type == DOWN_LINKTEST_RSP)
	{
		// ���ﴦ����Щ��Ӫ�̻ظ�������·����ʱʹ����������Ӧ
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));
	}
	else if (msg_type == DOWN_DISCONNECT_RSP)
	{
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));
	//	user._user_state = User::OFF_LINE;
		if (user._user_state == User::ON_LINE && user._fd != NULL )
		{
			CloseSocket(user._fd);
		}
	}
	else if (msg_type == DOWN_DISCONNECT_INFORM)
	{
		//���������ɶ�ʱ�߳�����ɡ�
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));
	}
	else if (msg_type == DOWN_DISCONNECT_RSP)
	{
		OUT_RECV(ip,port,user._user_id.c_str(),get_type(msg_type));
		user._user_state = User::OFF_LINE;
	}
	else if (msg_type == UP_CTRL_MSG)
	{
		unsigned short data_type = ntouv16( ((CtrlMsgHeader*)(data+sizeof(Header)))->data_type);
		OUT_RECV(ip,port,user._user_id.c_str(),"UP_CTRL_MSG:%s",get_type(data_type));
		switch( data_type )
		{
		case UP_CTRL_MSG_MONITOR_VEHICLE_ACK:
			{
				UpCtrlMsgMonitorVehicleAck *moni = (UpCtrlMsgMonitorVehicleAck *)data;
				if ( len < (int)sizeof(UpCtrlMsgMonitorVehicleAck) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_MONITOR_VEHICLE_ACK length %d error" , len ) ;
					return ;
				}

				string macid ;
				// ȡ�ó��ƺ����ֻ��Ŷ�Ӧ��ϵ
				if ( ! GetMacIdByVechicle( moni->ctrl_msg_header.vehicle_no, moni->ctrl_msg_header.vehicle_color, macid ) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_MONITOR_VEHICLE_ACK get %d_%s macid failed",
							moni->ctrl_msg_header.vehicle_color, moni->ctrl_msg_header.vehicle_no ) ;
					return ;
				}
				_convert->AddMac2Access( macid, str_access_code ) ;

				string sdata ;
				if ( ! _convert->BuildMonitorVehicleResp( macid, moni, sdata ) ) {
					return ;
				}
				// �·����ݵ�MSG
				_pEnv->GetMsgClient()->Deliver( sdata.c_str(), sdata.length() ) ;
			}
			break ;
		case UP_CTRL_MSG_TAKE_PHOTO_ACK:
			{
				UpCtrlMsgTakePhotoAck *header = (UpCtrlMsgTakePhotoAck *)data ;;

				int data_len = ntohl(header->ctrl_photo_body.photo_len);
				if ( data_len < 0 || len < (int)sizeof(UpCtrlMsgTakePhotoAck) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_TAKE_PHOTO_ACK PasServer %s data len %d, len %d error" ,
							get_type(msg_type) , data_len , len ) ;
					return ;
				}
				// �����Ƭ���ݳ��ȴ�����Ϣ��������˵���Ƿְ�������Ӧ�ô���ʵ�ʵĳ���
				if ( data_len > len ) {
					data_len = len - sizeof(UpCtrlMsgTakePhotoAck) - sizeof(Footer) ;
				}

				string macid ;
				// ȡ�ó��ƺ����ֻ��Ŷ�Ӧ��ϵ
				if ( ! GetMacIdByVechicle( header->ctrl_msg_header.vehicle_no, header->ctrl_msg_header.vehicle_color, macid ) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_TAKE_PHOTO_ACK get %d_%s macid failed",
							header->ctrl_msg_header.vehicle_color, header->ctrl_msg_header.vehicle_no ) ;
					return ;
				}
				_convert->AddMac2Access( macid, str_access_code ) ;

				string sdata ;
				if ( ! _convert->BuildUpCtrlMsgTakePhotoAck( macid, _rootpath, data, len , sdata ) ) {
					return ;
				}
				// �·����ݵ�MSG
				_pEnv->GetMsgClient()->Deliver( sdata.c_str(), sdata.length() ) ;
			}
			break ;
		case UP_CTRL_MSG_TEXT_INFO_ACK:
			{
				OUT_INFO(ip,port,user._user_id.c_str(),"UP_CTRL_MSG_TEXT_INFO_ACK\n");

				UpCtrlMsgTextInfoAck *text = (UpCtrlMsgTextInfoAck *)data;
				if ( len < (int)sizeof(UpCtrlMsgTextInfoAck) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_TEXT_INFO_ACK length %d error" , len ) ;
					return ;
				}


				string macid ;
				// ȡ�ó��ƺ����ֻ��Ŷ�Ӧ��ϵ
				if ( ! GetMacIdByVechicle( text->ctrl_msg_header.vehicle_no, text->ctrl_msg_header.vehicle_color, macid ) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_CTRL_MSG_TEXT_INFO_ACK get %d_%s macid failed",
							text->ctrl_msg_header.vehicle_color, text->ctrl_msg_header.vehicle_no ) ;
					return ;
				}
				_convert->AddMac2Access( macid, str_access_code ) ;

				string sdata ;
				if ( ! _convert->BuildUpCtrlMsgTextInfoAck( macid, text , sdata ) ) {
					return ;
				}
						OUT_HEX(NULL,0, macid.c_str(),sdata.c_str(),sdata.length());
				// �·����ݵ�MSG
				_pEnv->GetMsgClient()->Deliver( sdata.c_str(), sdata.length() ) ;
			}
			break ;
		case UP_CTRL_MSG_TAKE_TRAVEL_ACK:
			break ;
		case UP_CTRL_MSG_EMERGENCY_MONITORING_ACK:
			break ;
		}
	}
	else if (msg_type == UP_EXG_MSG)  // UP_EXG_MSG
	{
		ExgMsgHeader *msg_header = (ExgMsgHeader*)(data + sizeof(Header));

		if (msg_header->vehicle_no[0]==(char)0)
		{
			OUT_ERROR(ip,port,user._user_id.c_str(),"vehicle is null");
			return;
		}

		unsigned short data_type = ntouv16( msg_header->data_type ) ;
		switch( data_type )
		{
		case UP_EXG_MSG_REAL_LOCATION:
			{
				OUT_RECV3(ip,port,user._user_id.c_str(),"UP_EXG_MSG_REAL_LOCATION:%s",msg_header->vehicle_no);

				UpExgMsgRealLocation *upmsg = (UpExgMsgRealLocation *) data;
				if ( len < (int)sizeof(UpExgMsgRealLocation) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_EXG_MSG_REAL_LOCATION length %d error" , len ) ;
					return ;
				}

				string macid ;
				// ȡ�ó��ƺ����ֻ��Ŷ�Ӧ��ϵ
				if ( ! GetMacIdByVechicle( upmsg->exg_msg_header.vehicle_no, upmsg->exg_msg_header.vehicle_color, macid ) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_EXG_MSG_REAL_LOCATION get %d_%s macid failed",
							upmsg->exg_msg_header.vehicle_color, upmsg->exg_msg_header.vehicle_no ) ;
					return ;
				}
				_convert->AddMac2Access( macid, str_access_code ) ;

				string sdata ;
				if ( ! _convert->BuildUpRealLocation( macid, upmsg , sdata ) ) {
					return ;
				}
				// �·����ݵ�MSG
				_pEnv->GetMsgClient()->Deliver( sdata.c_str(), sdata.length() ) ;
			}
			break ;
		case UP_EXG_MSG_HISTORY_LOCATION:
			{
				OUT_RECV3(ip,port,user._user_id.c_str(),"UP_EXG_MSG_HISTORY_LOCATION:%s",msg_header->vehicle_no);
				UpExgMsgHistoryHeader *upmsg = (UpExgMsgHistoryHeader *) data;

				unsigned char num =  ( unsigned char ) data[ sizeof(UpExgMsgHistoryHeader) ] ;
				if ( len < (int)sizeof(UpExgMsgHistoryHeader) + (int)sizeof(GnssData)*num ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_EXG_MSG_HISTORY_LOCATION length %d error" , len ) ;
					return ;
				}

				string macid ;
				// ȡ�ó��ƺ����ֻ��Ŷ�Ӧ��ϵ
				if ( ! GetMacIdByVechicle( upmsg->exg_msg_header.vehicle_no, upmsg->exg_msg_header.vehicle_color, macid ) ) {
					OUT_ERROR( ip, port, user._user_id.c_str(), "UP_EXG_MSG_HISTORY_LOCATION get %d_%s macid failed",
							upmsg->exg_msg_header.vehicle_color, upmsg->exg_msg_header.vehicle_no ) ;
					return ;
				}
				_convert->AddMac2Access( macid, str_access_code ) ;

				string sdata ;
				if ( ! _convert->BuildUpHistoryLocation( macid, data, len , num, sdata ) ) {
					return ;
				}
				// �·����ݵ�MSG
				_pEnv->GetMsgClient()->Deliver( sdata.c_str(), sdata.length() ) ;
			}
			break ;
		default:
			OUT_RECV(ip,port,user._user_id.c_str(),get_type(data_type));
			break;
		}
	}
	// ������ݲ�Ϊ��
	// ToDo: ��Ҫ�����·���MSG

	user._last_active_time = time(0);
	_online_user.SetUser( user._user_id, user ) ;
}

void PasServer::on_new_connection( socket_t *sock, const char* ip, int port)
{
	OUT_CONN( ip , port, NULL, "PasServer New connection fd %d, time %d" , sock->_fd, time(NULL) );
}

void PasServer::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
	if ( user._user_id.empty() ) {
		return ;
	}

	OUT_WARNING( user._ip.c_str() , user._port, user._user_id.c_str(), "PasServer::on_dis_connection Disconnection %d" , sock->_fd );

	_online_user.DeleteUser( sock );
}

bool PasServer::ConnectServer(User &user, unsigned int timeout)
{
	if(time(0) - user._connect_info.last_reconnect_time < user._connect_info.timeval)
		return false;

	bool ret = false;
	if ( user._fd  != NULL )
	{
		OUT_INFO(user._ip.c_str(), user._port , user._user_id.c_str(), "PasServer::ConnectServer %s:%d close socket");
		CloseSocket( user._fd ) ;
	}
	user._fd = _tcp_handle.connect_nonb(user._ip.c_str(), user._port, timeout);
	ret = (user._fd > 0) ? true:false;

	if(ret )
	{
		user._user_state       = User::WAITING_RESP;

		DownConnectReq req;
		req.header.msg_len     = ntouv32( sizeof(req) ) ;
		req.header.msg_seq     = ntouv32(_convert->get_next_seq());
		req.header.access_code = ntouv32(user._access_code);
		req.verify_code        = ntouv32(_verify_code);

		if ( SendCrcData(user._fd ,(const char*)&req,sizeof(req)) ) {
			OUT_INFO(user._ip.c_str(),user._port,user._user_id.c_str(),"PasServer Send DownConnectReq,down-link state:CONNECT_WAITING_RESP");
		} else {
			OUT_ERROR( user._ip.c_str(),user._port,user._user_id.c_str(),"PasServer Send DownConnectReq,down-link state:CONNECT_WAITING_RESP" ) ;
		}
	}
	else
	{
		user._user_state = User::OFF_LINE;
	}
	user._last_active_time = time(0);
	user._login_time = time(0);
	user._connect_info.last_reconnect_time = time(0);
	if(user._connect_info.keep_alive == ReConnTimes) {
		user._connect_info.reconnect_times--;
	}

	return ret ;
}

// ȡ�ó��ƺ��ֻ���֮���Ӧ��ϵ
bool PasServer::GetMacIdByVechicle( const char *vechicle, unsigned char color, string &macid )
{
	char szkey[128] = {0};
	sprintf( szkey, "%d_%s" , color, vechicle ) ;

	// ��������ھ��Ǵ�Redis������ȡ����������ھͲ�������
	if ( ! _pEnv->GetRedisCache()->HGet( LBS_CARAREA, szkey, macid ) ){
			return false ;
	}

	// ���MacId�Գ���ɫ�ͳ��ƺ�֮���ϵ
	_pEnv->GetMsgClient()->AddMac2Car( macid.c_str(), szkey ) ;

	return true ;
}

// ����û��Ƿ�Ϸ�
int PasServer::CheckLogin( unsigned int accesscode, const char *username, const char *password )
{
	char szkey[512] = {0};
	sprintf( szkey, "%u", accesscode ) ;

	string val ;
	// ��Redis��ȡ�ý����������Ϣ
	if ( ! _pEnv->GetRedisCache()->HGet( LBS_MPASCODE, szkey, val ) ) {
		OUT_ERROR( NULL, 0, username, "get key %s failed", szkey ) ;
		return 2 ;
	}
	// ����м�����Կ
	const char *ptr = strstr( val.c_str(), ":" ) ;
	if ( ptr != NULL ) {
		_keymgr.AddEncryptKey( accesscode, ptr+1 ) ;
	}
	// �û�������Ϊ�յ����������Կ����
	if ( username == NULL || password == NULL ) {
		return 3 ;
	}

	char szval[512] = {0};
	// ƥ����Ϣ�Ƿ���ȷ
	sprintf( szval , "%s_%s", username, password ) ;
	// �Ƚ��û����Ƿ���ȷ
	if ( strncmp( szval, val.c_str() , strlen(szval)) != 0 ) {
		OUT_ERROR( NULL, 0, username, "check username and password error, cur user: %s, value: %s", szval, val.c_str() ) ;
		return 4 ;
	}

	return 0 ;
}

// �ͻ����ڷ׷�����
void PasServer::HandlePasDown( const char *code , const char *data, const int len )
{
	// �������Ӧ���ǲ����ڣ�����ͻ���BUG
	// OUT_ERROR( NULL, 0 , NULL, "PasServer::HandlePasDown code %s",  code ) ;
	SendDataToPasUser( code, data, len ) ;
	// ��ӡʮ������
	OUT_HEX( NULL, 0, code, data, len ) ;
}

void PasServer::NoopWork()
{
	OUT_INFO( NULL, 0, NULL,"void	PasServer::NoopWork()");
	while(1)
	{
		if ( ! Check() )  break ;
		sleep(1);
		
		HandleOnlineUsers(20);
	}
}

void PasServer::TimeWork()
{
	OUT_INFO( NULL, 0, NULL,"void	PasServer::TimeWork()");

	//����ʱ���ʹ��
	while (1)
	{
		if ( ! Check() )  break ;

		/*1.��ⳬʱ�����ӳ�ʱ����Ҫ�����ģ��������Ӻ�ŵ�on_line��*/
		HandleOfflineUsers();
	//	HandleOnlineUsers(30);

		sleep( 5 );
	}
}

void PasServer::HandleOfflineUsers()
{
	int    count[]  = { 0, 0, 0 , 0 } ;
	string arr[] 	= { "","","","" } ;

	vector<User> vec_users = _online_user.GetOfflineUsers(USER_TIMEOUT);
	for(int i = 0; i < (int)vec_users.size(); i++)
	{
		User user = vec_users[i];
		if(user._socket_type == User::TcpClient)
		{
			if( user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str() , user._port , user._user_id.c_str() , "PasServer HandleOfflineUsers close TcpClient socket, fd %d", user._fd->_fd );
				CloseSocket( user._fd ) ;
			}
		}
		else if(user._socket_type == User::TcpConnClient)
		{
			if( user._fd != NULL )
			{
				OUT_WARNING( user._ip.c_str() , user._port , user._user_id.c_str() ,"PasServer HandleOfflineUsers close TcpConnClient socket,fd %d", user._fd->_fd );
			//	user.show( "PasServer UserInfo" );
				CloseSocket( user._fd ) ;
				user._fd = NULL ;
			}
			if ( ConnectServer( user, 10 ) ) {
				//���ӳɹ������¼ӵ������б��С�
				_online_user.AddUser(user._user_id, user);
			}
			else if (user._connect_info.keep_alive == AlwaysReConn
					|| user._connect_info.reconnect_times > 0 )
			{
				_online_user.AddUser(user._user_id, user);
			}
		}

		++ count[user._user_state] ;
		if ( ! arr[user._user_state].empty() ) {
			 arr[user._user_state] += "," ;
		}
		arr[user._user_state] += uitodecstr( user._access_code ) ;
	}

	string ctype = "" ;
	for ( int i = 0 ; i <= User::ON_LINE; ++ i ) {
		if ( i == User::OFF_LINE ) {
			ctype = "OffLine" ;
		} else if ( i == User::WAITING_RESP ) {
			ctype = "WaitingResp" ;
		} else if ( i == User::ON_LINE ) {
			ctype = "Online" ;
		}
		if ( count[i] > 0 ) {
			OUT_INFO( NULL, 0, NULL, "PasServer HandleOfflineUsers Count %d, %s: %s", count[i], ctype.c_str(), arr[i].c_str() ) ;
		}
	}
}

void PasServer::HandleOnlineUsers(int timeval)
{
	if(time(0) - _last_handle_user_time < timeval)
		return;
	_last_handle_user_time = time(0);

	string online = "" ;
	vector<User> vec_users = _online_user.GetOnlineUsers() ;
	for(int i = 0; i < (int)vec_users.size(); i++)
	{
		User &user = vec_users[i] ;
		if( user._socket_type == User::TcpConnClient && user._fd != NULL )
		{
			DownLinkTestReq req;
			req.header.msg_len     = ntouv32( sizeof(req) ) ;
			req.header.access_code = ntouv32( user._access_code );
			req.header.msg_seq     = ntouv32( _convert->get_next_seq() );

			SendCrcData(user._fd,(const char*)&req,sizeof(req));
			OUT_SEND( user._ip.c_str(), user._port, user._user_id.c_str(), "PasServer DOWN_LINKTEST_REQ" );
			user._last_active_time = time(0) ;
			_online_user.SetUser( user._user_id, user) ;
		}

		if ( ! online.empty() ) online += "," ;
		online += uitodecstr( user._access_code ) ;
	}
	// ��ӡ�����߳��û����
	OUT_INFO( NULL, 0, NULL, "PasServer HandleOnlineUsers Count %d, Online: %s", vec_users.size(), online.c_str() ) ;
}

bool PasServer::SendDataToPasUser( const string &access_code ,const char *data, int len )
{
	User user = _online_user.GetUserByUserId("D_" + access_code);
	if(user._user_id.empty() || user._user_state != User::ON_LINE)
		user = _online_user.GetUserByUserId("U_" + access_code);

	string msg = ProtoParse::Decoder( data, len ) ;
	if( user._user_state == User::ON_LINE ) {
		// ���������������ѭ����Ĵ���
		if ( SendCrcData( user._fd, data, len ) ) {
			OUT_SEND( user._ip.c_str() , user._port , user._user_id.c_str(), "PasServer SendDataToPasUser %s ,access code %s" ,
					msg.c_str() , access_code.c_str() ) ;
			return true ;
		}
	}

	OUT_ERROR( user._ip.c_str() , user._port , user._user_id.c_str(), "PasServer SendDataToPasUser %s ,access code %s failed" ,
			msg.c_str() , access_code.c_str() ) ;
	return false;
}

// �������ݽ���5B���봦��
bool PasServer::Send5BCodeData( socket_t *sock, const char *data, int len )
{
	if ( sock == NULL )
		return false ;

	C5BCoder  coder;
	if ( ! coder.Encode( data, len ) ) {
		OUT_ERROR( NULL, 0, NULL, "Send5BCodeData failed , socket fd %d", sock->_fd ) ;
		return false ;
	}
	return SendData( sock, coder.GetData(), coder.GetSize() ) ;
}

// ����CRC��CODE
void PasServer::ResetCrcCode( char *data, int len )
{
	// ͳһ����ѭ�������֤
	unsigned short crc_code = ntouv16( GetCrcCode( (char*)data, len ) ) ;
	unsigned int   offset   = len - sizeof(Footer) ;

	// �滻ѭ�����ڴ��λ������
	memcpy( data + offset , &crc_code, sizeof(short) ) ;
}

// �������´���ѭ���������
bool PasServer::SendCrcData( socket_t *sock, const char* data, int len)
{
	// ����ѭ����
	char *buf = new char[len+1] ;
	memset( buf, 0 , len+1 ) ;
	memcpy( buf, data, len ) ;

	// �����������
	EncryptData( (unsigned char*)buf, len, true ) ;

	// ����ѭ���봦��,������Ҫ�ȼ��ܺ���ѭ����У��
	ResetCrcCode( buf, len ) ;

	// ���5B�Ĵ���
	bool bSend = Send5BCodeData( sock, buf , len ) ;

	delete [] buf ;

	return bSend ;
}

///////////////////////////////////////CKeyMgr//////////////////////////////////////////////////
PasServer::CKeyMgr::CKeyMgr()
{
	_size = 0 ;
}

PasServer::CKeyMgr::~CKeyMgr()
{

}
// ��ӵ�������Կ������
void PasServer::CKeyMgr::AddEncryptKey( unsigned int access, const char *key )
{
	if ( key == NULL )
		return ;

	_Key val ;
	// M1_IA1_IC1
	const char *p = strstr( key, "_" ) ;
	if ( p == NULL )
		return ;
	val.M1  = atoi( key ) ;
	val.IA1 = atoi( p + 1 ) ;
	const char *q = strstr( p + 1, "_" ) ;
	if ( p == NULL )
		return ;
	val.IC1 = atoi( q + 1 ) ;

	_mutex.lock() ;

	CMapKey::iterator it = _keys.find( access ) ;
	if ( it != _keys.end() ) {
		it->second = val ;
	} else {
		++ _size ;
		_keys.insert( make_pair( access, val )) ;
	}
	_mutex.unlock() ;
}

// ȡ�ü�����Կ
bool PasServer::CKeyMgr::GetEncryptKey( unsigned int access, unsigned int &M1 , unsigned int &IA1 , unsigned int &IC1 )
{
	_mutex.lock() ;

	if ( _size == 0 ){
		_mutex.unlock() ;
		return false ;
	}

	CMapKey::iterator it = _keys.find( access ) ;
	if ( it == _keys.end() ) {
		_mutex.unlock() ;
		return false ;
	}

	M1   = it->second.M1 ;
	IA1  = it->second.IA1 ;
	IC1  = it->second.IC1 ;

	_mutex.unlock() ;

	return true ;
}
