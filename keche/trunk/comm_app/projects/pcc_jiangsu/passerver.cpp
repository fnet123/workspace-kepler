/*
 * passerver.cpp
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  ��Ӽ򵥷�����ͨ�ſ��
 */

#include "passerver.h"
#include <comlog.h>
#include <tools.h>
#include <protocol.h>
#include <arpa/inet.h>
#include "mybase64.h"
#include "pconvert.h"

PasServer::PasServer(CStatInfo *stat)
	: _user_mgr(this), _statinfo(stat)
{
	_thread_num  = 10 ;
	_last_check  = time(NULL) ;
	_xmlpath     = "./runing.xml" ;
}

PasServer::~PasServer( void )
{
	Stop() ;
}

bool PasServer::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char szip[128] = {0} ;
	if ( ! pEnv->GetString("pas_listen_ip" , szip) ){
		OUT_ERROR( NULL, 0, NULL, "get pas_listen_ip failed" ) ;
		return false ;
	}
	_listen_ip = szip ;

	int port = 0 ;
	if ( ! pEnv->GetInteger( "pas_listen_port", port ) ){
		OUT_ERROR( NULL, 0, NULL, "get pas_listen_port failed" ) ;
		return false ;
	}
	_listen_port = port ;

	int nvalue = 10 ;
	if ( pEnv->GetInteger( "pas_tcp_thread", nvalue )  ){
		_thread_num = nvalue ;
	}

	char szbuf[1024] = {0} ;
	if ( pEnv->GetString( "pcc_run_xml", szbuf) ) {
		_xmlpath = szbuf ;
	}

	// �������ݷְ�����
	setpackspliter( &_pack_spliter ) ;

	return true ;
}

bool PasServer::Start( void )
{
	/**
	if ( ! StartUDP( _listen_port , _listen_ip.c_str() , _thread_num ) ){
		OUT_ERROR( NULL, 0, "PasServer", "start udp failed" ) ;
		return false ;
	}*/
	if ( ! _udp_handle.init( _thread_num, _thread_num ) ) {
		OUT_ERROR( NULL, 0, "PasServer", "init udp client failed" ) ;
		return false ;
	}

	return StartServer( _listen_port , _listen_ip.c_str() , _thread_num ) ;
}

// ����STOP����
void PasServer::Stop( void )
{
	StopServer() ;
}

void PasServer::TimeWork()
{
	OUT_INFO( NULL, 0, NULL , "void PasServer::TimeWork()" ) ;
	//��������һ���¾��Ǽ����û���Ϣ�б�
	while (1){
		if ( ! Check() ) break ;

		time_t now = time(NULL) ;
		if ( now - _last_check > 30 ) {
			_user_mgr.Check( 120 ) ;
			_last_check = now ;
		}
		// ���ͳ������
		_statinfo->Check() ;
		// ��������
		_statinfo->Print( _xmlpath.c_str() ) ;

        sleep(10);
	}
}

void PasServer::NoopWork()
{
	OUT_INFO( NULL, 0, NULL , "void PasServer::NoopWork()" ) ;
}

void PasServer::on_data_arrived( socket_t *sock, const void *data, int len) //�ӿͻ������ݹ���������
{
	if ( len < 3 ) {
		OUT_ERROR( sock->_szIp, sock->_port , "Data", "Recv fd %d, len %d data %s", sock->_fd, len, ( const char *)data ) ;
		return ;
	}

	const char *ptr = ( const char *) data ;
	if ( (char)(*ptr) == '*' ) {
		// ����UDP������ͨ��������
		HandleUDPData( sock, ptr, len ) ;
	} else {
		// ����TCP�Ŀ���ͨ��������
		HandleTcpData( sock, ptr, len ) ;
	}
}

// ����TCP������
void PasServer::HandleTcpData( socket_t *sock, const char *data, int len )
{
	OUT_RECV( sock->_szIp, sock->_port, "PasServer", "TCP fd %d, Recv: %s", sock->_fd, data ) ;

	_PairUser *user = _user_mgr.GetUser( sock ) ;
	if ( user == NULL ) {
		OUT_ERROR( sock->_szIp, sock->_port, "Error", "fd %d, recv data %s not find user", sock->_fd, data ) ;
		return ;
	}

	// �������в�ֺ������
	string line( data, len - 2 ) ;
	vector<string> veck ;
	if ( ! splitvector( line, veck, " ", 0 ) ){
		OUT_ERROR( sock->_szIp, sock->_port, "Error", "fd %d, split data %s error", sock->_fd, data ) ;
		return ;
	}
	if ( veck.size() < 3 ) {
		OUT_ERROR( sock->_szIp, sock->_port,  "Error", "fd %d, data format %s error", sock->_fd, data ) ;
		return ;
	}
	if ( veck[0] != "SZ" || veck[2].empty() ){
		OUT_ERROR( sock->_szIp, sock->_port, "Error", "fd %d, param %s error", sock->_fd, data ) ;
		return ;
	}

	string val = veck[2] ;
	if ( veck[1] == "A" )
	{  // ����ͨ������
		vector<string> vk ;
		if ( ! splitvector( val, vk, "|", 3 ) ){
			OUT_ERROR( sock->_szIp, sock->_port, "Error", "fd %d split param failed" , sock->_fd ) ;
			return ;
		}

		string ckey ;

		char szbuf[256] = {0};
		if ( ! _user_mgr.OnAuth( sock, vk[0].c_str(), vk[2].c_str() , ckey ) ) {
			sprintf( szbuf, "SZE A -2\r\n" ) ;
			OUT_ERROR( sock->_szIp, sock->_port, "Error", "fd %d auth lkey failed", sock->_fd ) ;
		} else {
			sprintf( szbuf, "SZE A %s\r\n" , ckey.c_str() ) ;
			OUT_INFO( sock->_szIp, sock->_port, "Auth", "fd %d auth key %s success", sock->_fd , ckey.c_str() ) ;
		}
		SendData( sock, szbuf, strlen(szbuf) ) ;
	}
	else if ( veck[1] == "P" )
	{ // �û���Ȩ, ��������ͨ��ע��
		vector<string> vk ;
		if ( ! splitvector( val, vk, "|", 3 ) ){
			OUT_ERROR( sock->_szIp, sock->_port,  "Error", "fd %d split param failed" , sock->_fd ) ;
			return ;
		}

		if( vk[0].empty() || vk[1].empty() || vk[2].empty() ){
			return ;
		}

		string pkey ;
		char szbuf[256] = {0};

		const char *szip = vk[1].c_str() ;
		int udpport = atoi( vk[2].c_str() ) ;

		// ��¼���û�
		_statinfo->SetClient( sock->_szIp, sock->_port,  udpport ) ;

		socket_t *nfd = ConnectUDP( szip , udpport ) ;
		if ( nfd == NULL ) {
			OUT_ERROR( sock->_szIp, sock->_port,  "Error", "connect udp server ip %s port %d failed", szip, udpport ) ;
			return ;
		}

		const char *ckey = vk[0].c_str() ;
		if ( ! _user_mgr.OnRegister( sock, ckey, szip , udpport , nfd, pkey ) ) {
			sprintf( szbuf, "SZE P -1\r\n" ) ;
			OUT_ERROR( sock->_szIp, sock->_port,  "Error", "fd %d register pkey failed", sock->_fd ) ;
			CloseSocket( nfd ) ;
			nfd = NULL;
		} else {
			sprintf( szbuf, "SZE P %s\r\n" , pkey.c_str() ) ;
			OUT_INFO( sock->_szIp, sock->_port, "Auth", "fd %d register key %s success", sock->_fd , pkey.c_str() ) ;
		}
		SendData( sock, szbuf, strlen(szbuf) ) ;

		// ����һ��������ȥ
		if ( nfd != NULL ) {
			sprintf( szbuf, "*%s|NOOP|%s#", user->udp._code.c_str(), user->udp._key.c_str() ) ;
			SendData( nfd , szbuf, strlen(szbuf) ) ;
		}
	}
	else if ( veck[1] == "N" )
	{ // ����ͨ������ά��
		if ( _user_mgr.OnLoop( sock, val.c_str() ) ) {
			char buf[128] = {0};
			sprintf( buf, "SZE N %s\r\n", val.c_str() ) ;
			SendData( sock, buf, strlen(buf) ) ;
			OUT_INFO(sock->_szIp, sock->_port,  user->tcp._code.c_str() , "NOOP" ) ;
		} else {
			OUT_ERROR( sock->_szIp, sock->_port, user->tcp._code.c_str() , "fd %d , check NOOP error" , sock->_fd ) ;
		}
	}
}

// ����UDP������
void PasServer::HandleUDPData( socket_t *sock, const char *data, int len )
{
	OUT_RECV( sock->_szIp, sock->_port, "PasServer", "UDP fd %d, Recv: %s", sock->_fd, data ) ;

	const char *ptr = strstr( data, "|" ) ;
	if ( ptr == NULL ) {
		OUT_ERROR( sock->_szIp, sock->_port,  "Recv", "fd %d data error", sock->_fd ) ;
		return ;
	}

	char srvid[128] = {0};
	memcpy( srvid, data+1, ptr- data -1 ) ;

	_PairUser *user = _user_mgr.GetUser( srvid ) ;
	if ( user == NULL ){
		OUT_ERROR( sock->_szIp, sock->_port,  srvid, "fd %d user not login", sock->_fd ) ;
		return ;
	}
	user->udp._active = time(NULL) ;
	user->udp._fd     = sock ;

	// ��ʱֻ��������ͨ��������
	if ( strncmp( ptr+1 , "NOOP", 4 ) == 0 ) {
		char *begin = strstr( ptr+1, "|" ) ;
		if ( begin == NULL ) {
			OUT_ERROR( sock->_szIp, sock->_port,  srvid, "fd %d data error", sock->_fd ) ;
			return ;
		}
		char *end = strstr( begin+1, "#" ) ;
		if ( end == NULL || end <= begin+1 ) {
			OUT_ERROR( sock->_szIp, sock->_port, srvid, "fd %d data error", sock->_fd ) ;
			return ;
		}

		char pkey[128] = {0};
		memcpy( pkey, begin+1, end - begin-1 ) ;
		if ( strcmp( pkey, user->udp._key.c_str()) != 0 ) {
			OUT_ERROR( sock->_szIp, sock->_port, srvid, "fd %d data error", sock->_fd ) ;
		} else {
			SendData( sock, data, len ) ;
			OUT_INFO( sock->_szIp, sock->_port,  srvid, "fd %d NOOP", sock->_fd ) ;
		}
	} else {
		_statinfo->AddRecv( sock->_szIp ) ;
		vector<string> vec;
		if ( ! splitvector( data, vec, "|", 0 ) ) {
			OUT_ERROR( sock->_szIp, sock->_port, "Recv", "fd %d, split data error: %s", sock->_fd, data ) ;
			return ;
		}
		// 7
		CBase64Ex base64 ;
		base64.Decode( vec[7].c_str(), vec[7].length() ) ;

		string sdata = "*";
		sdata += _pEnv->GetPasClient()->GetSrvId() ;
		sdata += ptr ;
		if ( _pEnv->GetPasClient()->HandleData( sdata.c_str(), sdata.length() ) ){
			_statinfo->AddSend( sock->_szIp ) ;
		} else {
			PCCMSG_LOG( _pEnv->GetLogger(), sock->_szIp, sock->_port, srvid, "ERROR", "���ͳ��ƺ�%s��������ʧ��", base64.GetBuffer() ) ;
		}

		OUT_PRINT( sock->_szIp, sock->_port, srvid, "%s,UDP fd %d, HandleData: %s" , base64.GetBuffer(), sock->_fd, sdata.c_str() ) ;

		bool error = false ;
		char szmacid[512] = {0} ;
		if ( _pEnv->GetMsgClient()->GetCarMacId( base64.GetBuffer(), szmacid ) ) {
			CQString buf , msg ;
			if ( PConvert::buildintergps( vec, szmacid, buf , msg ) ) {
				// ת�����ڲ�Э�鷢�͸�MSG
				_pEnv->GetMsgClient()->HandleUpMsgData( SEND_ALL, buf.GetBuffer(), buf.GetLength() ) ;
				// ��ӡ������
				OUT_INFO( sock->_szIp, sock->_port, srvid, "fd %d, PasServer %s, Send Data: %s", sock->_fd,  base64.GetBuffer(),  buf.GetBuffer() ) ;
			} else {
				error = true ;
				OUT_ERROR( sock->_szIp, sock->_port, srvid, "fd %d, %s, PasServer build inter gps macid %s", sock->_fd, base64.GetBuffer(), szmacid ) ;
				PCCMSG_LOG( _pEnv->GetLogger(), sock->_szIp, sock->_port, srvid, "ERROR", "���ƺ�%s,����%s", base64.GetBuffer() , msg.GetBuffer() ) ;
			}
		} else {
			error = true ;
			OUT_ERROR( sock->_szIp, sock->_port, srvid, "PasServer UDP fd %d, %s get macid failed", sock->_fd, base64.GetBuffer() ) ;
			//PCCMSG_LOG( _pEnv->GetLogger(), ip, port, srvid, "ERROR", "ȡ�ó��ƺ�%sֵ��Ӧ��ϵʧ��", base64.GetBuffer() ) ;
		}
		_statinfo->AddVechile( sock->_szIp, base64.GetBuffer(), my_atoi(vec[4].c_str()) , error ) ;
	}
}

void PasServer::on_dis_connection( socket_t *sock )
{
	// Nothing do
}

// ���յ������ӵ�������
void PasServer::on_new_connection( socket_t *sock, const char* ip, int port)
{
	if ( check_udp_fd( sock ) )
		return ;
	// ��ӵ��û�����
	_user_mgr.AddUser( sock ) ;
}

// ��MSGת������������
void PasServer::HandleData( const char *data, int len )
{

}

// �ر��û�֪ͨ
void PasServer::CloseUser( socket_t *sock )
{
	// �ر����Ӵ���
	CloseSocket( sock ) ;
}

// ֪ͨ�û�����
void PasServer::NotifyUser( socket_t *sock , const char *key )
{
	char szbuf[128] = {0} ;
	sprintf( szbuf, "SZE L %s\r\n", key ) ;
	SendData( sock, szbuf, strlen(szbuf) ) ;
}

// ����UDP�ķ�����
socket_t *PasServer::ConnectUDP( const char *ip, int port )
{
	socket_t *fd = _udp_handle.connect_nonb( ip , port , 10 ) ;
	if ( fd == NULL ) {
		OUT_ERROR( ip, port, "Conn", "connect remote udp server failed\n" ) ;
		return NULL;
	}
	return fd ;
}


