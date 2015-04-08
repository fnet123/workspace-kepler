/**********************************************
 * ClientAccessServer.cpp
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/
#include "wasserver.h"
#include "comlog.h"
#include <Base64.h>
#include "proto/GBProtoParse.h"

#include "../tools/utils.h"

ClientAccessServer::ClientAccessServer( CacheDataPool &cache_data_pool )
	: _cache_data_pool(cache_data_pool),_recvstat(5)
{
	_gb_proto_handler = NULL ;
	_gb_proto_handler = GbProtocolHandler::getInstance() ;
	_queuemgr		  = NULL ;
	_queuemgr		  = NULL ;
	_thread_num 	  = 10 ;
	_online_count     = 0 ;
	_max_timeout      = 180 ;
	_max_pack_live    = 180 ;
	_secureType       = 1;
	_defaultOem       = "4C54";
}

ClientAccessServer::~ClientAccessServer()
{
	Stop() ;
	if ( _queuemgr != NULL ) {
		delete _queuemgr ;
		_queuemgr = NULL ;
	}
	if ( _gb_proto_handler != NULL ) {
		GbProtocolHandler::FreeHandler( _gb_proto_handler ) ;
		_gb_proto_handler = NULL ;
	}
}

bool ClientAccessServer::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	int nvalue = 0 ;

	// ��Ӷ������ط�����
	int nsend = 2 , ntime = 5 ;
	// ���ݰ��ط�����
	if ( pEnv->GetInteger( "was_pack_resend", nvalue ) ) {
		nsend = nvalue ;
	}
	// �ȴ���Ӧ���ݰ���ʱʱ��
	if ( pEnv->GetInteger( "was_pack_timeout", nvalue ) ) {
		ntime = nvalue ;
	}
	_queuemgr = new CQueueMgr(this, ntime, nsend ) ;

	char szbuf[512] = {0};

	char szip[128] = {0} ;
	if ( ! pEnv->GetString("was_listen_ip" , szip) )
	{
		OUT_ERROR( NULL, 0,"ClientAccessServer", "Get was_listen_ip failed\n" ) ;
		return false ;
	}

	_listen_ip = szip;

	int port = 0 ;
	if ( ! pEnv->GetInteger( "was_listen_port", port ) )
	{
		OUT_ERROR( NULL, 0,"ClientAccessServer", "Get was_listen_port failed\n" ) ;
		return false ;
	}
	_listen_port = port;

	// ���������UDP�ļ�����ʹ��UDP
	if ( pEnv->GetInteger( "was_udp_port" , port ) ) {
		_udp_listen_port = port ;
		_udp_listen_ip   = szip ;
	}

	nvalue = 10 ;
	if ( pEnv->GetInteger( "was_tcp_thread", nvalue )  ){
		_thread_num = nvalue ;
	}

	// ȡ���������ʱ��
	if ( pEnv->GetInteger( "was_user_time" , nvalue ) ) {
		_max_timeout = nvalue ;
	}

	// ������ݰ����ʱ��������
	if ( pEnv->GetInteger( "was_pack_live",  nvalue ) ) {
		_max_pack_live = nvalue ;
	}
	// ���غ������ļ���·��
	if ( pEnv->GetString( "was_black_list", szbuf ) ) {
		_blackpath = szbuf ;
	}
	// ����IP���б���ļ���
	if ( pEnv->GetString( "was_black_ip", szbuf ) ) {
		_ippath = szbuf ;
	}

	// �Ƿ�����������
	if ( pEnv->GetInteger("was_tcpauth", nvalue ) ) {
		_secureType = nvalue;
	}

	if (pEnv->GetString("default_ome", szbuf)) {
		_defaultOem = szbuf;
	}

	// ��ʼ��SCP���ϴ�
	if ( ! _scp_media.Init( pEnv ) ) {
		OUT_ERROR( NULL, 0,"ClientAccessServer", "Init scp media failed" ) ;
		return false ;
	}

	// ���÷ְ�����
	setpackspliter( & _pack_spliter ) ;

	return true;
}

bool ClientAccessServer::Start( void )
{
	if ( ! _scp_media.Start() ) {
		OUT_ERROR( NULL, 0, "ClientAccessServer", "Start scp thread failed") ;
		return false ;
	}

	if ( _udp_listen_port > 0 ) {
		//  �������UDP�����ݴ����ֱ��ʹ��UDP����
		 if ( ! StartUDP( _udp_listen_port , _udp_listen_ip.c_str() , _thread_num , _max_timeout ) ) {
			 OUT_ERROR( NULL, 0,"ClientAccessServer", "Start udp server failed, ip %s port %d" ,
					 _udp_listen_ip.c_str(), _udp_listen_port ) ;
			 return false ;
		 }
	}
	return StartServer( _listen_port, _listen_ip.c_str(), _thread_num , _max_timeout ) ;
}

void ClientAccessServer::Stop( void )
{
	StopServer() ;
	// ֹͣSCP�߳�
	_scp_media.Stop() ;
}

void ClientAccessServer::TimeWork()
{
	//��������һ���¾��Ǽ����û���Ϣ�б�
	while (1)
	{
		if ( ! Check() ) break ;

		if ( ! _blackpath.empty() ) {
			// ���غ�����������
			_blacklist.LoadBlack( _blackpath.c_str() ) ;
		}
		// ����IP�������б�
		if ( ! _ippath.empty() ) {
			_ipblack.LoadIps( _ippath.c_str() ) ;
		}
		// ���������ĳ�ʱ
		_pack_mgr.CheckTimeOut(_max_pack_live) ;
		// ���ý�����ݳ�ʱ����
		_scp_media.CheckTimeOut() ;

		sleep(5);
	}
}

void ClientAccessServer::NoopWork()
{
	time_t last_show = 0 ;
	while(1) {

		if ( ! Check() ) break ;

		time_t now = time(NULL) ;

		vector<User> vec = _online_user.GetOnlineUsers();
		if ( vec.empty()) {
#ifdef _GPS_STAT
			if ( now - last_show > 30 ) {
				last_show = now ;
				// ͳ�Ƶ�ǰ���߳�����
				OUT_RUNNING( NULL, 0, "ONLINE" , "online user count 0, recv flux 0kb, gps data count: %d" , _pEnv->GetGpsCount() ) ;
			}
#endif
			sleep(5);
			continue;
		}

		map<socket_t*,int>  mapref ;  // ��¼����ʹ�����
		map<socket_t*,int>::iterator it ;

		// �����û�ͳ��
		int count = 0 ;
		// �������״̬
		for ( int i = 0 ; i < (int)vec.size(); ++ i ) {
			User &user = vec[i] ;

			int flag = 0 ;
			if ( now - user._last_active_time > _max_timeout ) {
				// �Ͽ����Ӵ�ӡһ����־
				OUT_WARNING( user._ip.c_str(), user._port, user._user_id.c_str(), "Delete User fd %d" , user._fd->_fd ) ;

				// 2013-02-20,ycq
				bool bnotify = true ;  // �Ƿ�Ҫ��������֪ͨ
				if (strncmp(user._user_id.c_str(), "UDP_", 4) == 0) {
					// ���ΪUDP�ȼ��TCP�����Ƿ���������Ͳ�������֪ͨ
					User temp = _online_user.GetUserByUserId(user._user_id.substr(4));
					if ( ! temp._user_id.empty()) {
						bnotify = false;
					}
				}
				if (bnotify) {
					string caits = "CAITS 0_0 " + user._user_name + " 0 D_CONN {TYPE:0} \r\n";
					_pEnv->GetMsgClient()->HandleUpData(caits.c_str(), caits.length());
				}

				// ɾ���û���ID
				_online_user.DeleteUser(user._user_id);
			} else {
				flag = 1 ; ++ count ;
			}

			// �����û�����
			if ( user._fd != NULL ) {
				// ������������Ƿ���ʹ��
				it = mapref.find( user._fd ) ;
				if ( it == mapref.end() ) {
					mapref.insert( pair<socket_t*,int>(user._fd, flag ) ) ;
				} else {
					// �������ʹ�������������˵����������
					it->second = it->second + flag ;
				}
			}
		}

#ifdef _GPS_STAT
		if ( now - last_show > 30 ) {
			last_show = now ;
			 // ����GPSͳ������
			// ͳ�Ƶ�ǰ���߳�����
			OUT_RUNNING( NULL, 0, "ONLINE" , "online user count %d, recv flux %fkb, gps data count: %d" ,
						count , _recvstat.GetFlux() / FLUX_KB , _pEnv->GetGpsCount());
		}
#else
		// ͳ�Ƶ�ǰ���߳�����
		//OUT_WARNING( NULL, 0, "ONLINE" , "online user count %d, recv flux %fkb" , count , _recvstat.GetFlux() / FLUX_KB ) ;
#endif
		// ���߳�������
		_online_count = count ;

		if ( mapref.empty() )  continue ;
		// ��������
		for ( it = mapref.begin(); it != mapref.end(); ++ it ) {
			// ���������������ʹ��
			if ( it->second > 0 )
				continue ;
			// ���������Ϊ����ֱ�ӹر�����
			CloseSocket( it->first ) ;
		}
		sleep(5);
	}
}

// ��BCD����ȡ���ֻ���
static bool getphonenum( char *bcd , string &phone )
{
	string temp = BCDtostr(bcd) ;
	if ( temp.length() == 0 || temp.length() < 12 )
		return false ;
	phone = temp.substr( 1, 11 ) ;
	return true ;
}

void ClientAccessServer::on_data_arrived( socket_t *sock , const void *data, int len)
{
	OUT_HEX( sock->_szIp, sock->_port, "RECV", (const char *)data, len ) ;
	// ��ӵ�����ͳ����
	_recvstat.AddFlux( len ) ;
	/*
	 * �����ܹ�����������������������ܴ������ְ�ذ�������������Ҫ���������ơ�
	 */
	const char *ip      = sock->_szIp ;
	unsigned short port = sock->_port ;

	const int min_len = int(sizeof(GBheader) + sizeof(GBFooter));
	const int max_len = int((min_len + sizeof(MsgPart) + 0x3ff) * 2);
	if(len < min_len || len > max_len) {
		OUT_WARNING(ip, port, NULL, "invalid message, drop it! length %d", len) ;
		return;
	}

	vector<unsigned char> dstBuf;
	unsigned char *dstPtr;
	size_t dstLen;

	dstBuf.resize(len);
	dstPtr = &dstBuf[0];
	dstLen = Utils::deCode808((unsigned char*)data, len, dstPtr);

	handle_one_packet( sock, (char*)dstPtr, dstLen);
}

void ClientAccessServer::handle_one_packet( socket_t *sock ,const char *data,int len)
{
	GBheader *header = (GBheader*)data;
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	string str_car_id ;
	if ( ! getphonenum( header->car_id, str_car_id ) ) {
		OUT_ERROR( ip, port ,"ClientAccessServer", "Handle_one_packet get phone num failed" ) ;
		OUT_HEX( ip, port, str_car_id.c_str() , data , len ) ;
		// ���FD����
		return ;
	}

	// ����ں��������û��У���ֱ�ӹر����Ӳ���������
	if ( _blacklist.OnBlack( str_car_id.c_str() ) ) {
		OUT_ERROR( ip, port, str_car_id.c_str(), "user in black list close socket fd %d", sock->_fd ) ;
		CloseSocket( sock ) ;
		return ;
	}

	unsigned short _s = 0x03FF;
	unsigned short nmsg = 0 ;
	memcpy(&nmsg,&(header->msgtype),sizeof(unsigned short));
	nmsg = ntohs( nmsg ) ;

	// ȡ��ǰ10λֵΪ����
	unsigned short msg_len = nmsg & _s;
	unsigned short msg_id  = ntohs(header->msgid);
	unsigned short seq     = ntohs(header->seq);

	int need_len = msg_len  + sizeof(GBFooter) + sizeof(GBheader) ;
	// ���ȼ��㳤���Ƿ���ȷ
	if ( len < need_len ) {
		OUT_ERROR( ip, port, str_car_id.c_str(), "msg id %x, data length error, len %d, msg len %d, need length %d" ,
				msg_id, len , msg_len , need_len ) ;
		// ���FD����
		return ;
	}

	// �����ݰ���BUF
	DataBuffer dbuf ;
	// �����13λΪ1��Ϊ�ְ���Ϣ
	if ( nmsg & 0x2000 ) {
		// ���Ϊ�ְ���Ϣ
		if ( len != (int)( msg_len + sizeof(GBFooter) + sizeof(GBheader) + sizeof(MediaPart)) ){
			OUT_ERROR( ip, port, str_car_id.c_str(), "msg id %x, long message data length error, len %d, msg len %d,need length %d" ,
					msg_id, len , msg_len ,need_len + sizeof(MediaPart) ) ;
			// ���FD����
			return ;
		}

		// �ְ��ṹ
		MediaPart *part = (MediaPart *) ( data + sizeof(GBheader) ) ;
		// �ڴ����������
		if ( ! _pack_mgr.AddPack( dbuf, str_car_id.c_str(), msg_id, ntohs( part->seq ) , ntohs( part->total ) , seq , data, len )  ) {
			// ������ݰ���û�����ֻ���سɹ�����Ӧ
			unsigned short downseq = _pEnv->GetSequeue(str_car_id.c_str());
			if(downseq!=0xffff){
				PlatFormCommonResp resp = _gb_proto_handler->BuildPlatFormCommonResp( header, downseq, 0 ) ;
				SendResponse( sock, str_car_id.c_str(), (const char *)&resp, sizeof(resp) ) ;
			}
			return ;
		}
	} else { // �������ͨ��Ϣ
		if ( len != (int)( msg_len  + sizeof(GBFooter) + sizeof(GBheader)) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "msg id %x, message length error, len %d, msg len %d, need length %d" ,
					msg_id, len , msg_len , need_len ) ;
			// ���FD����
			return ;
		}
	}

	if(dbuf.getLength() > 0) {
		data = dbuf.getBuffer();
		len = dbuf.getLength();
	}

	processMsgGb808(sock, data, len, str_car_id);
}

void ClientAccessServer::processMsgGb808(socket_t *sock ,const char *data, int len, const string &str_car_id)
{
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	GBheader *header       = (GBheader*) data ;
	unsigned short msg_id  = ntohs(header->msgid);
	unsigned int   msg_len = len - sizeof(GBheader) - sizeof(GBFooter) ;

	// ȡ�õ�ǰʱ��
	time_t now = time( NULL ) ;
	string caits_car_id = "" ;

	User user;
	string key = str_car_id ;
	// ���ΪUDP�û����������ǰ׺��
	if ( check_udp_fd( sock ) ) {
		//�յ�udp���ݣ�����tcpģʽ�Ự��ʱʱ��
		user = _online_user.GetUserByUserId(key);
		if ( ! user._user_id.empty()) {
			user._last_active_time = time(NULL);
			_online_user.SetUser(key, user);
		}

		key = "UDP_" + str_car_id ;
	}

	if (msg_id == 0x0100) { //�ն�ע��
		TermRegisterHeader *reg_h = (TermRegisterHeader*) data;
		if (len < (int) (sizeof(TermRegisterHeader) + sizeof(GBFooter))) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "msg id %x, message length %d error" , msg_id, len );
			return;
		}

		int carnolen = msg_len + sizeof(GBheader) - sizeof(TermRegisterHeader);
		if (carnolen < 0 || carnolen > 32) {
			return;
		}
		carnolen = strnlen(data + sizeof(TermRegisterHeader), carnolen);

		string provinceID;
		string cityID;
		string producerID;
		string termType;
		string termID;
		string plateColor;
		string plateNum;

		Utils::int2str(ntohs(reg_h->province_id), provinceID);
		Utils::int2str(ntohs(reg_h->city_id), cityID);
		producerID.assign((char*) reg_h->corp_id, strnlen((char*) reg_h->corp_id, 5));
		termType.assign((char*) reg_h->termtype, strnlen((char*) reg_h->termtype, 20));
		termID.assign((char*) reg_h->termid, strnlen((char*) reg_h->termid, 7));
		Utils::int2str(reg_h->carcolor, plateColor);
		plateNum.assign(data + sizeof(TermRegisterHeader), carnolen);

		string filter = string(":,{} \r\n\0", 8);
		producerID = Utils::filter(producerID, filter);
		termType = Utils::filter(termType, filter);
		termID = Utils::filter(termID, filter);
		plateNum = Utils::filter(plateNum, filter);

		string oem = "0000";
		unsigned char result = 2;
		string verifyCode = "";

		if (_secureType == 0) {
			oem = _defaultOem;
			result = 0;
			verifyCode = "1234567890";
		} else {
			string fieldStr;
			vector<string> fieldVec;

			_pEnv->GetRedisCache()->HGet("KCTX.SECURE", str_car_id.c_str(), fieldStr);
			Utils::splitStr(fieldStr, fieldVec, ',');
			if (fieldVec.size() < 8 || fieldVec[7] != "0") {
				result = 4;
			} else if(_secureType == 1 && termID == fieldVec[2]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 2 && reg_h->carcolor != 0 && plateNum == fieldVec[4]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 2 && reg_h->carcolor == 0 && plateNum == fieldVec[5]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 3 && plateColor == fieldVec[3] && plateNum == fieldVec[4]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 3 && reg_h->carcolor == 0 && plateNum == fieldVec[5]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 4 && termID == fieldVec[2] && reg_h->carcolor != 0 && plateNum == fieldVec[4]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 4 && termID == fieldVec[2] && reg_h->carcolor == 0 && plateNum == fieldVec[5]) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			} else if(_secureType == 5) {
				oem = fieldVec[6];
				result = 0;
				verifyCode = fieldVec[0];
			}
		}

		caits_car_id = oem + "_" + str_car_id;
		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:36";
		caits += ",40:" + provinceID + ",41:" + cityID + ",42:" + producerID;
		caits += ",43:" + termType + ",44:" + termID + ",45:" + string(1, result + '0');
		caits += ",202:" + plateColor + ",104:" + plateNum + "} \r\n";

		_pEnv->GetMsgClient()->HandleUpData(caits.c_str(), caits.length());

		vector<unsigned char> respBuf;
		TermRegisterRespHeader *respPtr;
		GBFooter *footer;

		respBuf.resize(sizeof(TermRegisterRespHeader) + verifyCode.size() + sizeof(GBFooter));
		respPtr = (TermRegisterRespHeader*) &respBuf[0];
		footer = (GBFooter*) (&respBuf[0] + sizeof(TermRegisterRespHeader) + verifyCode.size());

		uint16_t msgType = htons(3 + verifyCode.size());

		respPtr->header.begin._begin = 0x7e;
		respPtr->header.msgid = htons(0x8100);
		memcpy(&respPtr->header.msgtype, &msgType, sizeof(uint16_t));
		memcpy(respPtr->header.car_id, header->car_id, 6);
		respPtr->header.seq = 0;
		respPtr->resp_seq = header->seq;
		respPtr->result = result;
		memcpy(&respBuf[0] + sizeof(TermRegisterRespHeader), verifyCode.c_str(), verifyCode.size());
		footer->check_sum = _gb_proto_handler->get_check_sum((char*) &respBuf[0] + 1, respBuf.size() - 3);
		footer->ender._end = 0x7e;

		/*******��������****************/
		SendResponse(sock, str_car_id.c_str(), (char*) &respBuf[0], respBuf.size());

		return;
	} else if (msg_id == 0x0102) { //�ն˼�Ȩ
		string oem = "0000";
		int result = 1;
		string authCode(data + sizeof(GBheader), strnlen(data + sizeof(GBheader), msg_len));

		if (_secureType == 0) {
			oem = _defaultOem;
			result = 0;
		} else {
			string fieldStr;
			vector<string> fieldVec;

			_pEnv->GetRedisCache()->HGet("KCTX.SECURE", str_car_id.c_str(), fieldStr);
			Utils::splitStr(fieldStr, fieldVec, ',');
			if (fieldVec.size() >= 8 && fieldVec[7] == "0" && authCode == fieldVec[0]) {
				oem = fieldVec[6];
				result = 0;
			}
		}

		caits_car_id = oem + "_" + str_car_id;
		if (result == 0) {
			user._fd = sock;
			user._user_id = key;
			user._user_name = caits_car_id; // �û���Ϊ��OEM���ֵ
			user._ip = (ip == NULL) ? "0.0.0.0" : ip;
			user._port = port;
			user._login_time = now;
			user._user_state = User::ON_LINE;
			user._last_active_time = now;

			if (!_online_user.AddUser(key, user)) {
				_online_user.DeleteUser(key);
				_online_user.AddUser(key, user);
			}
		}

		_pEnv->ResetSequeue( str_car_id.c_str() ) ;

		unsigned short downseq = _pEnv->GetSequeue(str_car_id.c_str());
		PlatFormCommonResp resp = _gb_proto_handler->BuildPlatFormCommonResp(header, downseq, result);
		SendResponse(sock, str_car_id.c_str(), (const char *) &resp, sizeof(resp));

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:38";
		caits += ",47:" + authCode + ",48:" + string(1, result + '0') + "} \r\n";
		_pEnv->GetMsgClient()->HandleUpData(caits.c_str(), caits.length());

		return;
	}

	// ֱ��ͨ���û���KEY���������Ч��
	user = _online_user.GetUserByUserId(key);
	if ( ! user._user_id.empty()) {
		user._last_active_time = now;

		// ����FD������
		if (user._fd != sock) {
			// �ر���һ���û�������
			// CloseSocket( user._fd ) ;
			OUT_ERROR( ip, port, str_car_id.c_str(), "fd %d not equal socket old fd %d" , sock->_fd, user._fd->_fd );
			_online_user.DeleteUser(user._fd);

			// ����µ�����
			user._fd = sock;
			user._login_time = now;
			user._ip = sock->_szIp;
			user._port = sock->_port;
		}
		// ������û�����
		_online_user.SetUser(key, user);
	} else if (_secureType == 0) {
		user._fd = sock;
		user._user_id = key;
		user._user_name = _defaultOem + "_" + str_car_id; // �û���Ϊ��OEM���ֵ
		user._ip = (ip == NULL) ? "0.0.0.0" : ip;
		user._port = port;
		user._login_time = now;
		user._user_state = User::ON_LINE;
		user._last_active_time = now;

		_online_user.AddUser(key, user);
	} else if(check_udp_fd( sock )) {
		user = _online_user.GetUserByUserId(str_car_id);
		if(user._user_id.empty()) {
			OUT_WARNING( ip, port, str_car_id.c_str(), "term no auth");
			return;
		}

		user._fd = sock;
		user._user_id = key;
		user._ip = (ip == NULL) ? "0.0.0.0" : ip;
		user._port = port;
		user._login_time = now;
		user._user_state = User::ON_LINE;
		user._last_active_time = now;

		_online_user.AddUser(key, user);
	} else {
		OUT_WARNING( ip, port, str_car_id.c_str(), "term no auth");
		return;
	}

	unsigned char result   = 0;
	bool bresp 			   = false ;
	unsigned short respid  = 0 ;

	caits_car_id = user._user_name;

	if ( msg_id == 0x0101 ||  msg_id == 0x0003 ) {
		// �ն�ע��
		_online_user.DeleteUser( key ) ;

		OUT_PRINT( ip, port, str_car_id.c_str(), "user unregister fd %d" , sock->_fd ) ;
		// ��Ҫת���洢

		char buf[1024] = {0};
		sprintf( buf, "CAITS 0_0 %s 0 U_REPT {TYPE:37,46:%d} \r\n" , caits_car_id.c_str() , result ) ;

		// ����ע���¼�
		_pEnv->GetMsgClient()->HandleUpData( buf , strlen(buf) ) ;

		result = 0x00 ;
		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if(msg_id == 0x0200) { //�ն�λ����Ϣ�㱨
		if ( msg_len < (unsigned int)sizeof(GpsInfo) || len < (int)sizeof(TermLocationHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "TermLocationHeader data length %d error" , msg_len ) ;
			return ;
		}

		TermLocationHeader *_location_header = (TermLocationHeader*)data;
		// ����λ���ϱ�����
		string caits = "CAITS 0_1 " + caits_car_id + " 0 U_REPT " ;

		//���ݳ�����ȷ
		char *append_data = NULL;
		int append_data_len = 0;

		append_data = (char *)data+sizeof(TermLocationHeader);
		append_data_len = len -  sizeof(TermLocationHeader) - 2;
		// �������
		caits += "{TYPE:0,RET:0," ;
		//printf("appenddataoffset:%x,appenddatalen:%x\n",sizeof(TermLocationHeader),append_data_len);
		caits += _gb_proto_handler->ConvertGpsInfo(&(_location_header->gpsinfo),append_data,append_data_len) ;
		// ��ӽ���
		caits += "} \r\n";

		OUT_DEBUG("updata:%s\n",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if (msg_id == 0x0702) { //��ʻԱ�����Ϣ�ɼ��ϱ�
		if ( msg_len < 60 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0702 mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";

		//���ݳ�����ȷ
		char * msgbody_data = NULL;
		string dstr = "";
		msgbody_data = (char *)data+sizeof(GBheader);
		dstr = _gb_proto_handler->ConvertDriverInfo(msgbody_data,msg_len, result);
		if( dstr.length() > 5 ){
			caits +=  dstr + " \r\n";

			OUT_DEBUG("updata:%s\n",caits.c_str());

			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;
		}
		// ��Լ�ʻԱ���ʶ�����⴦��
		result = 0 ;
		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if (msg_id == 0x0104) { //��ѯ�ն˲���Ӧ��
		if ( len < (int)sizeof(QueryTermParamAckHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "QueryTermParamAckHeader mesage length %d error", len ) ;
			return ;
		}

		string caitr = "";
		QueryTermParamAckHeader *rsp =  ( QueryTermParamAckHeader *) data ;

		respid = ntohs(rsp->respseq) ;

		string key = str_car_id+"_"+uitodecstr(0x8104)+"_"+uitodecstr( respid );
		CacheData cache_data = _cache_data_pool.checkData(key);
		if( respid == 0xffff) {
			string content = "";
			/******δ��ʱ*******/
			if(_gb_proto_handler->ConvertGetPara( (char*)data, len, content)) {
				/*****���ݽ�����ȷ****/
				//OUT_WARNING(ip,port,str_car_id.c_str(),"ConvertGetPara is fail car_id=%d",car_id);//����carid
				caitr = "CAITR 0_0 " + caits_car_id + " 0 D_GETP "+content+" \r\n";
			} else {
				/*****���ݽ�������****/
				caitr = "CAITR 0_0 " + caits_car_id + " 0 D_GETP {RET:1} \r\n";
			}
			_pEnv->GetMsgClient()->HandleUpData( caitr.c_str(), caitr.length() );
		} else if (cache_data.str_send_msg == "EXIT" ) {
			string content = "" ;
			/******δ��ʱ*******/
			if(_gb_proto_handler->ConvertGetPara( (char*)data, len, content)){
				/*****���ݽ�����ȷ****/
				//OUT_WARNING(ip,port,str_car_id.c_str(),"ConvertGetPara is fail car_id=%d",car_id);//����carid
				caitr = "CAITR " + cache_data.seq + " " + cache_data.mac_id + " 0 D_GETP "+content+" \r\n";
			} else {
				/*****���ݽ�������****/
				caitr = "CAITR " + cache_data.seq + " " + cache_data.mac_id + " 0 D_GETP {RET:1} \r\n";
			}
			_pEnv->GetMsgClient()->HandleUpData( caitr.c_str(), caitr.length() ) ;
		} else {
			// ��ʱ
			OUT_WARNING(ip,port,str_car_id.c_str(),"msg type: %x, msg id : 0x8104, cant's find seq:%x in cache_data_pool", msg_id, respid );
		}

		result = 0x00 ;
		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if ( msg_id == 0x0002 ) { //9.3���ն�����
		// ���ֻҪ������������
		string caits = "CAITS 0_0 " + user._user_name + " 0 U_CONN {TYPE:1} \r\n";
		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(),caits.length() );

		// ��Ҫ����ͨ��Ӧ��
		result = 0x00 ;
		bresp  = true ;
	} else if(msg_id==0x0201) { //λ����Ϣ��ѯ
		if ( len < (int)sizeof(_TermLocationRespHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "_TermLocationRespHeader mesage length %d error", len ) ;
			return ;
		}

		_TermLocationRespHeader *_location_resp_header = (_TermLocationRespHeader*)data;
		char *append_data = NULL;
		int append_data_len = 0;
		/***�ж��Ƿ��и�������***/
		if(msg_len > sizeof(GpsInfo) + 2) {
			append_data = (char *)data+sizeof(_TermLocationRespHeader);
			append_data_len = msg_len -  (sizeof(GpsInfo) + 2);
		}

		respid = ntohs(_location_resp_header->reqseq) ;

		//printf("get location��C\n");
		string caitr   = "";
		string key = str_car_id+"_"+uitodecstr(0x8201)+"_"+uitodecstr(respid);
		//printf("get loc keyid :%s\n",key.c_str());
		CacheData cache_data = _cache_data_pool.checkData(key);
		if (cache_data.str_send_msg == "EXIT" ){
			//printf("get location��D\n");
			/******δ��ʱ*******/
			string content = _gb_proto_handler->ConvertGpsInfo(&(_location_resp_header->gpsinfo),append_data,append_data_len);
			if(content.length() > 0) {
				//printf("get location��E\n");
				/*****���ݽ�����ȷ****/
				caitr = "CAITS " + cache_data.seq + " " + cache_data.mac_id + " 201 U_REPT {TYPE:0,RET:0,"+content+"} \r\n";
			} else {
				/*****���ݽ�������****/
				caitr = "CAITS " + cache_data.seq + " " + cache_data.mac_id + " 0 U_REPT {TYPE:0,RET:1} \r\n";
			}

			OUT_DEBUG("get location��%s\n",caitr.c_str());

			_pEnv->GetMsgClient()->HandleUpData( caitr.c_str(), caitr.length() ) ;
		} else {
			OUT_WARNING(ip,port,str_car_id.c_str(),"msg type: %x, msg id: 0x8201, cant's find seq:%x in cache_data_pool", msg_id, respid );
		}
		// ��Ҫ����ͨ��Ӧ��
		result = 0x00 ;
		bresp  = true ;
	}
	else if(msg_id == 0x0001) { //�ն�ͨ��Ӧ��
		if ( len < (int)sizeof(TermCommonResp) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "TermCommonResp mesage length %d error", len ) ;
			return ;
		}

		TermCommonResp *resp = (TermCommonResp*)data;
		//key=carid_msgid_seq
		unsigned short rmsgid  = ntohs(resp->resp.resp_msg_id) ;
		respid  = ntohs(resp->resp.resp_seq ) ;

		string key = str_car_id+"_"+ustodecstr(rmsgid) + "_" + uitodecstr(respid);
		CacheData cache_req = _cache_data_pool.checkData(key);
		if(cache_req.str_send_msg == "EXIT" ) {
			//���ͨ�ûظ�����
			string caitr = "CAITR " + cache_req.seq + " " + cache_req.mac_id + " 0 " + cache_req.command + " ";
			caitr += "{RET:"+ chartodecstr(resp->resp.result) +"} \r\n";
			_pEnv->GetMsgClient()->HandleUpData( caitr.c_str(), caitr.length() ) ;

			OUT_DEBUG("terminal response:%s\n",caitr.c_str());

		} else {
			OUT_WARNING(ip,port,str_car_id.c_str(),"msg type: %x, msg id: %x, cant's find seq:%x in cache_data_pool", msg_id, rmsgid, respid );
		}
	}
	else if(msg_id == 0x0301) { //�¼�����
		if ( msg_len < 1 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0301 mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";

		//���ݳ�����ȷ
		unsigned char  eventch = data[sizeof(GBheader)];
		caits = caits +"{TYPE:31,81:"+ chartodecstr(eventch)+"} \r\n";

		OUT_DEBUG("updata:%s\n",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if(msg_id == 0x0302) { //����Ӧ��
		if ( len < (int)sizeof(QuestionReplyAsk) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "QuestionReplyAck mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";
		//���ݳ�����ȷ
		QuestionReplyAsk *ack = ( QuestionReplyAsk *)( data ) ;

		respid = ntohs( ack->seq );

		caits = caits +"{TYPE:32,84:"+ uitodecstr(respid)+",82:"+chartodecstr(ack->id)+"} \r\n";

		OUT_DEBUG("updata:%s",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if(msg_id == 0x0303) { //��Ϣ�㲥/ȡ��
		if ( len < (int)sizeof(InfoDemandCancleAck) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "InfoDemandCancleAck mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";

		InfoDemandCancleAck *ack = ( InfoDemandCancleAck * ) data ;
		//���ݳ�����ȷ
		caits = caits +"{TYPE:33,83:"+ chartodecstr( ack->info_type )+"|"+chartodecstr( ack->info_mark )+"} \r\n";

		OUT_DEBUG("updata:%s",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if ( msg_id == 0x0500 ) { // ��������Ӧ����Ҫ�������벻��
		if ( len < (int)sizeof(TermLocationRespHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "TermLocationRespHeader mesage length %d error", len ) ;
			return ;
		}

		TermLocationRespHeader *rsp = ( TermLocationRespHeader *) data ;

		respid = ntohs( rsp->reqseq ) ;
		// ����������Ӧ��
		string content = _gb_proto_handler->ConvertGpsInfo(&(rsp->gpsinfo),NULL, 0 );
		if(content.length() > 0) {
			/*****��Ӧ���ݳ����Ƿ�����****/
			string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:39,RET:0,"+content+"} \r\n";
			// �ϴ�Ӧ����
			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

			// ��Ҫ����ͨ��Ӧ��
			result = 0x00 ;
		}

		bresp  = true ;
	} else if(msg_id == 0x0901) { //����ѹ���ϴ�
		if ( msg_len < 4 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0901 mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";
		unsigned int datalen = 0;

		//���ݳ�����ȷ
		memcpy(&datalen,data+sizeof(GBheader),4);
		datalen = ntohl(datalen);

		string dstr = "";

		char transdata[1500] = {0};
		memcpy(transdata,data+sizeof(GBheader)+4,msg_len-4);
		transdata[msg_len-4]=0;

		OUT_DEBUG("GZipdata:%s\n",transdata);

		CBase64 base64;
		base64.Encode( transdata, msg_len-4 );

		caits = caits +"{TYPE:14,93:"+uitodecstr(datalen)+",92:"+ base64.GetBuffer()+"} \r\n";

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if(msg_id == 0x0700) { //��ʻ��¼�������ϴ�
		if ( msg_len < (unsigned int)sizeof(TachographBody) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "TachographBody mesage length %d error", len ) ;
			return ;
		}

		TachographBody *body = ( TachographBody *) ( data + sizeof(GBheader) ) ;

		respid = ntohs(body->seq) ;

		string sseq = "0_0" ;
		string key  = str_car_id+"_"+ustodecstr(0x8700) + "_" + uitodecstr( respid );
		CacheData cache_req = _cache_data_pool.checkData(key);
		if(cache_req.str_send_msg == "EXIT" ) {
			sseq = cache_req.seq ;
		} else {
			result = 0x00 ;
			//bresp  = true ;
		}

		// ToDo: �ϴ���ʻ��¼	2	��¼������
		string caits = "CAITS "+sseq+" " + caits_car_id + " 0 U_REPT ";

		const char *pdata = ( const char *) ( data + sizeof(GBheader) + sizeof(TachographBody) ) ;

		CBase64 base64 ;
		base64.Encode( pdata , msg_len - 3 ) ;

		// �����ʻ��¼��¼��ʱ����ϴ�����
		caits = caits +"{TYPE:2,70:"+uitodecstr(body->cmd)+",61:"+ base64.GetBuffer()+"} \r\n";

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;
	        bresp  = true ;
	} else if(msg_id == 0x0701) { //�����˵��ϱ�
		if ( len < (int)sizeof(EWayBillReportAckHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "EWayBillReportAckHeader mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";

		//���ݳ�����ȷ
		EWayBillReportAckHeader *ack = ( EWayBillReportAckHeader *) data ;
		unsigned int datalen = ntohl( ack->length );
		if( datalen + 4 == msg_len )
		{
			string dstr = "";

			char transdata[1500] = {0};
			memcpy( transdata, data+sizeof(EWayBillReportAckHeader), datalen ) ;
			transdata[datalen]=0;

			OUT_DEBUG("transdata:%s\n",transdata);

			CBase64 base64;
			base64.Encode( transdata, datalen );

			caits = caits +"{TYPE:35,87:"+ base64.GetBuffer()+"} \r\n";

			OUT_DEBUG("updata:%s",caits.c_str());

			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

			result = 0x00 ;
		}

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if( msg_id == 0x0900 ) {//����͸��-----
		if ( msg_len < 1 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0900 mesage length %d error", msg_len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT ";  // ToDo:

		//���ݳ�����ȷ
		unsigned short dlen = msg_len -1;
		unsigned char dtype = 0;

		string dstr = "";
		dtype = data[sizeof(GBheader)];

		vector<char> datBuf(dlen + 1);
		char *transdata = &datBuf[0];
		//char transdata[1500] = {0};
		memcpy(transdata,data+sizeof(GBheader)+1,dlen);
		transdata[dlen]=0;
		//printf("transdata:%s\n",transdata);

		CBase64 base64;
		base64.Encode(transdata,dlen) ;
		caits = caits +"{TYPE:9,91:"+ustodecstr(dtype)+",90:"+ base64.GetBuffer()+"} \r\n";

		//printf("convert transdata:%s\n",transdata);
		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;

		// ��Ҫ����ͨ��Ӧ��
		bresp  = true ;
	} else if(msg_id == 0x0800) { //��ý���¼���Ϣ�ϴ�
		if ( len < (int)sizeof(EventReort) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "EventReort mesage length %d error", len ) ;
			return ;
		}

		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:39,120:";

		//���ݳ�����ȷ
		EventReportEx *report = ( EventReportEx *) ( data + sizeof(GBheader) ) ;

		//��ý���¼�ID
		unsigned int dword = ntohl(report->id);
		caits += uitodecstr(dword)+",";
		//��ý������
		caits += "121:"+ustodecstr(report->type)+",";
		//��ý���ʽ����
		caits += "122:"+ustodecstr(report->mtype)+",";
		//�¼������
		caits += "123:"+ustodecstr( ( report->event & 0x7f ) )+",";
		//ͨ��ID
		caits += "124:"+ustodecstr(report->chanel);

		caits += "} \r\n";

		OUT_DEBUG("updata:%s",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(),caits.length() ) ;

		result = 0x00 ;

		// ���ԭ��û���ϴ��ɹ����ڴ��ļ�
		_scp_media.RemovePackage( str_car_id.c_str() ) ;

		// ��Ҫ����ͨ��Ӧ��
		bresp = true ;
	} else if ( msg_id == 0x0801 ) { //0x0801����ý�������ϴ���0x8800����ý�������ϴ�Ӧ��
		if ( msg_len < (unsigned int)sizeof(MediaUploadBody) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "MediaUploadBody mesage length %d error", len ) ;
			return ;
		}

		unsigned short ntotal = 1 ;
		unsigned short ncur   = 1 ;

		char *ptr = ( char *)( data + sizeof(GBheader) ) ;

		MediaUploadBody *body = ( MediaUploadBody *) ptr ;
		// ���Ψһ��ʶ
		int   id 	= -1 ;
		int   mdlen = 0  ;
		char* mdptr = NULL ;
		string gps  = "" ;

		mdptr = (char *)( ptr + sizeof(MediaUploadBody) );
		mdlen = msg_len - sizeof(MediaUploadBody);

		// ��Ҫ����ֶ���������
		bool uploadok = _scp_media.SavePackage( sock->_fd, caits_car_id.c_str(), id , ntotal , ncur ,
				body->type , body->mtype , ( body->event & 0x7f ) , body->chanel , mdptr, mdlen , gps.c_str() ) ;

		if( uploadok )//�����һ�ζ�ý���ļ��Ĵ���
		{
			PhotoRespHeader resp;
			memcpy(&(resp.header),header,sizeof(GBheader));
			unsigned short downseq = _pEnv->GetSequeue(str_car_id.c_str());
			resp.header.msgid = htons(0x8800);

			unsigned short prop = htons(0x0005);
			memcpy(&(resp.header.msgtype),&prop,2);
			resp.header.seq 		= htons(downseq);
			resp.retry_package_num  = 0;
			resp.photo_id           = htonl(id);
			// ������ݼ�Ч��
			resp.check_num = _gb_proto_handler->get_check_sum( (const char *) &resp + 1, sizeof(resp) - 3 ) ;

			SendResponse( sock, str_car_id.c_str(), (const char *)&resp, sizeof(resp));
		} else { // ���һ��������Ҫͨ��Ӧ�𣬺Ϲ����в���Ҫ<2012.12.17>
			// �ְ��ϴ���Ҫ��Ӧͨ��Ӧ��  <2012.11.05> ����������һ������ͨ��Ӧ����
			result  = 0x00 ;
			bresp   = true ;
		}
	} else if ( msg_id == 0x0802 ) {
		if ( len < (int)sizeof(DataMediaHeader) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "DataMediaHeader mesage length %d error", len ) ;
			return ;
		}

		// �洢��ý�����ݼ���Ӧ��
		DataMediaHeader *rsp =  ( DataMediaHeader *) data;
		// �����ý����Ӧ��
		unsigned short num = ntohs( rsp->num ) ;
		if ( len < (int)( num * sizeof(DataMediaBody) + sizeof(DataMediaHeader) + sizeof(GBFooter) ) ){
			OUT_ERROR( ip, port, str_car_id.c_str(), "DataMedia length %d error, num %d", len, num ) ;
			return ;
		}

		respid = ntohs(rsp->ackseq) ;

		string key = str_car_id + "_" + ustodecstr(0x8802) + "_" + uitodecstr(respid);
		CacheData cache_req = _cache_data_pool.checkData(key);
		if(cache_req.str_send_msg == "EXIT" ) {
			/*��TYPE:1��1:[120:|0||1|1,1:����,2:γ��,3:�ٶ�,4:ʱ��,5:����,��][��ý��������2][��ý��������N]��
			[�ο����ն������ϴ���ָ���е�key��value����]*/
			string caits = "CAITR "+ cache_req.seq + " " + caits_car_id + " 0 "+cache_req.command+" {TYPE:1,20:" ;

			string temp = "" ;
			char szbuf[1024] = {0} ;
			char *ptr = ( char *) ( data + sizeof(DataMediaHeader));

			for ( unsigned short i = 0; i < num; ++ i ) {
				// ��ý������ID|��ý������|��ý���ʽ����|�¼������|ͨ��ID

				DataMediaBody *body = ( DataMediaBody *) ptr;

				// �����ý��ID,��ҳ���� , 120:��ý������ID,121:��ý������,124:ͨ��ID,123:�¼������
				sprintf( szbuf, "121:%u,123:%u,124:%u," , body->type, body->event, body->id );
				temp = szbuf;
				temp += _gb_proto_handler->ConvertGpsInfo( &(body->gps), NULL, 0 );

				ptr += sizeof(DataMediaBody);

				caits += "[" + temp + "]" ;
			}
			caits += "} \r\n" ;

			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(),caits.length() ) ;
		} else {
			OUT_WARNING(ip,port,str_car_id.c_str(),"msg type: %x, msg id: 0x8802, cant's find seq:%x in cache_data_pool", msg_id, respid );
		}

		// ��Ҫ����ͨ��Ӧ��
		result = 0x00 ;
		bresp  = true ;
	} else if ( msg_id == 0x0f10 ) {  // ä����������
		string heads = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:7,";
		string ends  = "} \r\n" ;

		int offset = sizeof(GBheader) ;
		unsigned char num = data[offset] ;
		if ( num == 0 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f10 number zero" ) ;
			return ;
		}

		offset += 1 ;  // ȥ���ϴ�������

		if ( msg_len < num*sizeof(GpsInfo) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f10 mesage length %d error", len ) ;
			return ;
		}

		string content ,sdata ;
		unsigned short word = 0 ;

		for ( unsigned char i = 0; i < num; ++ i ) {
			memcpy( &word, data+offset, sizeof(unsigned short) ) ;
			word = ntohs( word ) ;
			offset += sizeof(unsigned short) ;  // ȥ������

			if ( word == 0 || word > msg_len ) {
				break ;
			}

			GpsInfo *gps = ( GpsInfo *) ( data + offset ) ;  // ȡʵ��������

			content = _gb_proto_handler->ConvertGpsInfo( gps , data+offset+sizeof(GpsInfo) , word - sizeof(GpsInfo)) ;
			// ����ä����������
			if ( ! content.empty() ) {
				sdata = heads + content + ends  ;
				_pEnv->GetMsgClient()->HandleUpData( sdata.c_str(), sdata.length() ) ;
			} else {
				OUT_ERROR( NULL, 0, caits_car_id.c_str(), "msg id 0x0f10 data error gps content error" ) ;
			}
			offset += word ;
		}

		result = 0x00 ;
		bresp  = true ;
	} else if ( msg_id == 0x0f11 ) {  // �������������ϴ�
		if ( msg_len < sizeof(char)+sizeof(short) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f11 mesage length %d error", len ) ;
			return ;
		}
		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:50,513:";

		int offset = sizeof(GBheader) ;
		unsigned char type  = data[offset] ;  // ���ݿ�����
		offset += 1 ;

		caits += chartodecstr(type+1) + "," ;  // �����ϴ�����, �ڲ�Э�������808��һ��

		unsigned short nlen = 0 ;
		memcpy( &nlen , data+offset, sizeof(short) ) ; // ȡ�����ݳ���
		nlen = ntohs( nlen ) ;
		data += sizeof(short) ;

		if ( nlen == 0 ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f11 data length zero" ) ;
			return  ;
		}

		CBase64 base64;

		if ( ! base64.Encode( data +offset, nlen ) ){
			OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f11 base64 encode failed" ) ;
			return ;
		}

		caits += "514:" ;  caits += base64.GetBuffer() ;
		caits += "} \r\n" ;  // �齨����

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		result = 0x00 ;
		bresp  = true ;
	} else if ( msg_id == 0x0f13 ) {  // ��ʷ�����ϴ�
		if ( msg_len < sizeof(HistoryDataUploadBody) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "HistoryDataUploadBody mesage length %d error", len ) ;
			return ;
		}

		HistoryDataUploadBody *body = ( HistoryDataUploadBody *) ( data + sizeof(GBheader) ) ;

		respid = ntohs(body->seqid) ;
		if ( body->num == 0 ) {
			OUT_ERROR( ip, port, str_car_id.c_str() , "history data error, number zero" ) ;
			return ;
		}

		string key = str_car_id + "_" + ustodecstr(0x8f12) + "_" + uitodecstr(respid) ;
		CacheData cache_req = _cache_data_pool.checkData(key);

		if( cache_req.str_send_msg == "EXIT" ) {
			string caits = "CAITR "+ cache_req.seq + " " + caits_car_id + " 0 U_REPT {TYPE:51,515:" ;
			// ��Ϣ��ˮ��|��ʷ��������|��ʷ������ʼʱ��|��ʷ���ݽ���ʱ��|��ʷ���������|��ʷ����������
			caits += chartodecstr( respid ) + "|" ;
			caits += chartodecstr( body->type+1 ) + "|" ;
			caits += _gb_proto_handler->get_bcd_time( body->starttime ) + "|" ;
			caits += _gb_proto_handler->get_bcd_time( body->endtime ) + "|" ;
			caits += chartodecstr( body->num ) + "|" ;

			const char * ptr = ( const char *) ( data + sizeof(GBheader) + sizeof(HistoryDataUploadBody) ) ;

			string stemp ;
			switch( body->type )
			{
			case 0:	// λ�û㱨��������
				{
					if ( msg_len < sizeof(HistoryDataUploadBody) + body->num * sizeof(GpsInfo) ) {
						OUT_ERROR( ip, port, str_car_id.c_str(), "HistoryDataUploadBody GpsInfo mesage length %d , num %d error", len , body->num ) ;
						return ;
					}

					int offset = 0 ;
					for ( unsigned char i = 0 ; i < body->num; ++ i ) {
						GpsInfo *gps = ( GpsInfo *) ( ptr + offset ) ;
						stemp = _gb_proto_handler->ConvertGpsInfo( gps , NULL , 0 ) ;
						if ( ! stemp.empty() ) {
							caits += "["+stemp+"]" ;
						}
						offset += sizeof(GpsInfo) ;
					}
				}
				break ;
			case 1: // ��������Ч����
				{
					if ( msg_len < sizeof(HistoryDataUploadBody) + body->num * sizeof(EngneerData) ) {
						OUT_ERROR( ip, port, str_car_id.c_str(), "HistoryDataUploadBody EngneerData mesage length %d , num %d error", len , body->num ) ;
						return ;
					}

					int offset = 0 ;
					for ( unsigned char i = 0; i < body->num; ++ i ) {
						EngneerData *p = ( EngneerData *) ( ptr + offset ) ;
						stemp = _gb_proto_handler->ConvertEngeer( p ) ;
						if ( ! stemp.empty() ) {
							caits += "["+stemp+"]" ;
						}
						offset += sizeof(EngneerData) ;
					}
				}
				break ;
			case 2: // ����������������
				{
					if ( msg_len <= sizeof(HistoryDataUploadBody) ) {
						OUT_ERROR( ip, port, str_car_id.c_str(), "HistoryData back data mesage length %d error", len ) ;
						return ;
					}

					CBase64 base64 ;
					if ( ! base64.Encode( ptr , len- sizeof(GBheader) - sizeof(HistoryDataUploadBody) ) ){
						OUT_ERROR( ip, port, str_car_id.c_str(), "0x0f11 base64 encode failed" ) ;
						return ;
					}
					caits += base64.GetBuffer() ;
				}
				break ;
			}
			caits += "} \r\n" ;

			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

			HistoryDataUploadAck ack ; // �ϴ���ʷ����Ӧ��
			ack.header.msgid = htons( 0x8f13 ) ;
			ack.header.seq   = htons( _pEnv->GetSequeue(str_car_id.c_str()) ) ;
			memcpy( ack.header.car_id , header->car_id, sizeof(header->car_id) ) ;

			unsigned short len = sizeof(short) + sizeof(char) ;
			len = htons( len & 0x3ff ) ;
			memcpy( &ack.header.msgtype, &len, sizeof(short) ) ;

			ack.seqid  = header->seq ;
			ack.num    = 0x00 ;

			char buf[1024] = {0} ;
			memcpy( buf, &ack, sizeof(ack) ) ;

			GBFooter footer ;
			footer.check_sum = _gb_proto_handler->get_check_sum( buf + 1, sizeof(ack)-1 ) ;
			memcpy( buf + sizeof(ack) , &footer, sizeof(footer) ) ;

			// ������ӦӦ��
			SendResponse( sock, str_car_id.c_str(),  buf, sizeof(ack)+ sizeof(footer) ) ;
		} else {
			OUT_WARNING(ip,port,str_car_id.c_str(),"msg type: %x, msg id: 0x8f12, cant's find seq:%x in cache_data_pool", msg_id, respid );
		}
	} else if ( msg_id == 0x0f14 ) { //��ʻ��Ϊ�¼�
		if ( len < (int)sizeof(DriverActionEvent) ) {
			OUT_ERROR( ip, port, str_car_id.c_str(), "DriverActionEvent mesage length %d error", len ) ;
			return ;
		}

		DriverActionEvent *req  = ( DriverActionEvent *)data ;
		//516:�¼�����|[��ʼλ��γ��][��ʼλ�þ���][��ʼλ�ø߶�][��ʼλ���ٶ�][��ʼλ�÷���][��ʼλ��ʱ��]|[����λ��γ��][����λ�þ���][����λ�ø߶�][����λ���ٶ�][����λ�÷���][����λ��
		string caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:52,516:";

		caits += chartodecstr( req->type+1 ) + "|" ;
		caits += _gb_proto_handler->ConvertEventGps( &req->startgps ) + "|" ;
		caits += _gb_proto_handler->ConvertEventGps( &req->endgps )  ;

		caits += "} \r\n" ;

		_pEnv->GetMsgClient()->HandleUpData( caits.c_str(), caits.length() ) ;

		// ��Ҫ����ͨ��Ӧ��
		result = 0x00 ;
		bresp  = true ;
	} else if (msg_id == 0x0f15) { //�ն˰汾��Ϣ
		if ( len < (int)sizeof(ProgramVersionEvent) ) {
			 OUT_ERROR( ip, port, str_car_id.c_str(), "ProgramVersionEvent mesage length %d error", len) ;
			 return ;
		}

		ProgramVersionEvent *req = (ProgramVersionEvent *)data;
		int nOffset = sizeof(ProgramVersionEvent);

		VERSION_IINFO *lpInfoVersion = NULL;

		int info_count = req->count;
		unsigned char info_id;
		unsigned char info_len;
		char         *info_buf;

		int i;
		vector<string> infos(0xe);
		const string filter("| ,:\0", 5);

		for(i = 0; i < info_count; ++i) {
	    	if(nOffset + (int)sizeof(VERSION_IINFO) > len) {
	    		break;
	    	}
	    	lpInfoVersion = (VERSION_IINFO *)(data + nOffset);
	    	nOffset += sizeof(VERSION_IINFO);

	    	info_id  = lpInfoVersion->id;
	    	info_len = lpInfoVersion->len;
	    	info_buf = lpInfoVersion->buf;

	    	if(nOffset + info_len > len) {
	    		break;
	    	}

	    	if(info_len == 0) {
	    		continue;
	    	}

	    	if(info_id >= 1 && info_id <= 0xe) {
	    		infos[info_id - 1] = Utils::filter(string(info_buf, info_len), filter);
	    	}

            nOffset  += info_len;
		}

	    string caits;
	    caits.reserve(1024);

	    caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:53,518:";
	    for(i = 0; i < (int)infos.size(); ++i) {
	    	if(i > 0) {
	    		caits += "|";
	    	}

	    	caits += infos[i];
	    }

	    caits += "} \r\n";

		OUT_DEBUG("version:%s\n",caits.c_str());

		_pEnv->GetMsgClient()->HandleUpData(caits.c_str(), caits.length());
		//��Ҫ����ͨ��Ӧ��
		result = 0x00;
		bresp  = true;
	} else if (msg_id == 0x0ff1) { //����ϵ��У������ϱ�
		if ( msg_len < (int)(sizeof(SpeedAdjustReport) - sizeof(GBheader)) ) {
		    OUT_ERROR( ip, port, str_car_id.c_str(), "speed adjust report mesage length %d error", msg_len) ;
		    return ;
		}

		SpeedAdjustReport *report = (SpeedAdjustReport*)data;

		string field;
	    string caits;
	    caits.reserve(1024);

	    caits = "CAITS 0_0 " + caits_car_id + " 0 U_REPT {TYPE:61";

		caits += ",600:" + BCDtostr((char*)report->btime);
	    caits += ",601:" + BCDtostr((char*)report->etime);
	    caits += ",602:" + Utils::int2str(ntohs(report->gpsSpeed), field);
	    caits += ",603:" + Utils::int2str(ntohs(report->vssSpeed), field);
	    caits += ",604:" + Utils::int2str(ntohs(report->recommend), field);
		caits += "} \r\n";

		OUT_DEBUG("Idle Speed:%s\n",caits.c_str());
		_pEnv->GetMsgClient()->HandleUpData(caits.c_str(), caits.length());

	    //��Ҫ����ͨ��Ӧ��
		result = 0x00;
		bresp  = true;
	} else {
		result  = 0x03 ;
		bresp   = true ;
	}
	// �Ƴ�ƽ̨�·��ȴ���Ӧ������
	if ( respid > 0 ) {
		OUT_PRINT( sock->_szIp, sock->_port, str_car_id.c_str(), "msg id %04x, remove sequeue id %d", msg_id, respid ) ;
		_queuemgr->Remove( str_car_id.c_str(), respid, true ) ;
	}
	// �������Ҫ��Ӧ��ֱ�ӷ�����
	if ( ! bresp ) return ;

	unsigned short downseq = _pEnv->GetSequeue(str_car_id.c_str());
	if(downseq!=0xffff) {
		PlatFormCommonResp resp = _gb_proto_handler->BuildPlatFormCommonResp( header, downseq, result ) ;
		SendResponse( sock, str_car_id.c_str(), (const char *)&resp, sizeof(resp) ) ;
	}
}

void ClientAccessServer::on_dis_connection( socket_t *sock )
{
	User user = _online_user.GetUserBySocket( sock ) ;
	if( ! user._user_id.empty() ){
		OUT_WARNING( sock->_szIp , sock->_port , user._user_id.c_str(), "ENV on_dis_connection fd %d", sock->_fd ) ;
		bool bnotify = true ;  // �Ƿ�Ҫ��������֪ͨ
		if ( strncmp( user._user_id.c_str(), "UDP_" , 4 ) == 0  ){
			// ���ΪUDP�ȼ��TCP�����Ƿ���������Ͳ�������֪ͨ
			User temp = _online_user.GetUserByUserId( user._user_id.substr(4) ) ;
			if ( ! temp._user_id.empty() ) {
				bnotify = false ;
			}
		}
		if ( bnotify ) {
			string caits = "CAITS 0_0 " + user._user_name + " 0 D_CONN {TYPE:0} \r\n";
			_pEnv->GetMsgClient()->HandleUpData( caits.c_str(),caits.length() ) ;

			// ����ն˶Ͽ����Ӿ�ֱ�������ǰ�ն�������Ҫ�·�������
			_queuemgr->Del( user._user_id.c_str() ) ;
		}
		_online_user.DeleteUser( sock ) ;
	} else {
		OUT_CONN( sock->_szIp , sock->_port , "ENV", "on_dis_connection fd %d", sock->_fd ) ;
	}
}

// ������Ӧ���ݣ���Ҫʵ�������TCPͨ������TCP
bool ClientAccessServer::SendResponse( socket_t *sock, const char *id , const char *data, int len )
{
	// ���ΪUDP��FD�����Ȳ���TCPͨ������������ھ�ʹ��UDPͨ���·�
	if ( check_udp_fd( sock ) && id ) {
		User user = _online_user.GetUserByUserId( id ) ;
		if ( ! user._user_id.empty() && user._fd->_fd > 0 ) {
			sock = user._fd ;
		}
	}
	//OUT_SEND3( sock->_szIp , sock->_port , id , "fd %d send response len %d", sock->_fd , len ) ;
	//OUT_HEX( sock->_szIp, sock->_port, id , data , len );
	// ��������
	return Send7ECodeData( sock, data, len ) ;
}

bool ClientAccessServer::Send7ECodeData( socket_t *sock, const char *data, int len )
{
	vector<unsigned char> msgBuf;
	unsigned char *msgPtr;
	size_t msgLen;

	msgBuf.resize(len * 2);
	msgPtr = &msgBuf[0];
	msgLen = Utils::enCode808((unsigned char*)data, len, msgPtr);

	OUT_HEX( sock->_szIp, sock->_port, "SEND" , (char*)msgPtr, msgLen );

	return SendData( sock, (char*)msgPtr, msgLen) ;
}

bool ClientAccessServer::SendDataToUser( const string &user_id,const char *data,int data_len)
{
	User user = _online_user.GetUserByUserId( user_id ) ;
	if ( user._user_id.empty() || user._fd == NULL ) {
		user = _online_user.GetUserByUserId( "UDP_" + user_id ) ;
	}

	if( user._user_id.empty() || user._user_state == User::OFF_LINE || user._fd == NULL ){
		//�豸�����ߣ�����һ���豸�����ߵİ����ڴ������Ȳ��������豸�����ߵı�ʶ��
		OUT_WARNING( NULL,0,user_id.c_str(), "user is not login!" );
		OUT_HEX( user._ip.c_str() , user._port , user._user_id.c_str() , data , data_len );

		return false;
	}

	OUT_SEND3( user._ip.c_str() , user._port , user._user_id.c_str() , "fd %d send data len %d", user._fd->_fd , data_len ) ;
	//OUT_HEX( user._ip.c_str() , user._port , user._user_id.c_str() , data , data_len );

	return Send7ECodeData( user._fd, data, data_len );
}

// ���ݽṹ��
struct _qmsgdata
{
	char  id[13];
	char *ptr ;
	int   len ;
};

// ���ó�ʱ�ط�����
bool ClientAccessServer::OnReSend( void *data )
{
	if ( data == NULL )
		return false ;
	_qmsgdata *p = ( _qmsgdata *) data ;

	return SendDataToUser( p->id, (const char *)p->ptr, p->len ) ;
}

// ���ó�ʱ���ط�������ɾ������
void ClientAccessServer::Destroy( void *data )
{
	if ( data == NULL )
		return ;
	_qmsgdata *p = ( _qmsgdata *) data ;
	if ( p->ptr != NULL )
		delete [] p->ptr ;
	delete p ;
}

void ClientAccessServer::HandleDownData( const char *userid, const char *data, int  len , unsigned int seq , bool send )
{
	//�������л�����е����ݣ���Ϊ���л�����е������Ѿ��ǽ�����ϵ��������ݣ�����ֻ�轫�䷢�ͳ�ȥ���ɡ�
	if ( seq > 0 ) {
		_qmsgdata *p = new _qmsgdata;
		safe_memncpy( p->id, userid, sizeof(p->id) ) ;
		p->ptr = new char[len+1] ;
		memcpy( p->ptr, data, len ) ;
		p->len = len ;

		OUT_PRINT( NULL, 0, "QueueMgr", "add user %s seq id %u", userid, seq ) ;
		_queuemgr->Add( userid, seq, p, !send ) ;

		if ( ! send ) return ;
	}
	// ��������
	if ( ! SendDataToUser( userid , data, len ) ) {
		if ( seq > 0 )
			_queuemgr->Remove( userid, seq , false ) ;
	}
}

bool ClientAccessServer::HasLogin(string &user_id)
{
	User user = _online_user.GetUserByUserId( user_id ) ;
	if ( user._user_id.empty() )
		user = _online_user.GetUserByUserId( "UDP_" + user_id ) ;
	if ( user_id.empty() )
		return false;
	return true;
}

// �Ӻ󿽱�����������ַ�
static void reverse_copy( char *buf , int len, const char *src , const char fix )
{
	int nlen   = (int) strlen( src ) ;
	int offset = len - nlen ;
	if ( offset < 0 ) {
		offset = 0 ;
	}
	if ( offset > 0 ) {
		for ( int i = 0; i < offset; ++ i ) {
			buf[i] = fix ;
		}
	}
	memcpy( buf + offset, src, nlen ) ;
}

// ����TTS��������
void ClientAccessServer::SendTTSMessage( const char *userid, const char *msg, int len )
{
	GBheader header ;
	GBFooter footer ;

	char key[128] = {0} ;
	reverse_copy( key, 12 , userid, '0' ) ;
	strtoBCD( key , header.car_id ) ;

	// ����Ϊ��Ϣ���ݵĳ���ȥ��ͷ��β
	unsigned short mlen = htons( (len+1) & 0x03FF ) ;
	memcpy( &(header.msgtype), &mlen, sizeof(short) );

	unsigned short seqid = _pEnv->GetSequeue( userid ) ;
	header.seq 	 = htons(seqid) ;
	header.msgid = htons(0x8300) ;

	DataBuffer buf ;
	// д��ͷ������
	buf.writeBlock( &header, sizeof(header) ) ;
	// �����һ��λ
	buf.writeInt8( 0x08 ); // TTS����
	buf.writeBlock( msg , len ) ;

	footer.check_sum = _gb_proto_handler->get_check_sum( buf.getBuffer()+1 , buf.getLength() - 1 ) ;
	buf.writeBlock( &footer, sizeof(footer) ) ;

	SendDataToUser( userid,  buf.getBuffer(), buf.getLength() ) ;
}

// �������ӵ���ʱ�Ĵ���
void ClientAccessServer::on_new_connection( socket_t *sock, const char* ip, int port)
{
	// ��¼����
	OUT_CONN( ip, port, "ENV", "Recv new connection fd %d", sock->_fd ) ;
	// ���յ��µ����ӵ����
}

// ���IP�Ƿ�Ϊ�Ϸ�����Ч��IP����
bool ClientAccessServer::check_ip( const char *ip )
{
	if ( ! _ipblack.Check( ip ) ) {
		return true ;
	}
	return false ;
}
