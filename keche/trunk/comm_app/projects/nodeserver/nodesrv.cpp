/*
 * nodesrv.cpp
 *
 *  Created on: 2011-11-4
 *      Author: humingqing
 */

#include "nodesrv.h"
#include "nodeparse.h"

CNodeSrv::CNodeSrv()
{
	_thread_num  = 10 ;
	_pNodeMgr    = NULL ;
}

CNodeSrv::~CNodeSrv()
{
	Stop() ;
	if ( _pNodeMgr != NULL ) {
		delete _pNodeMgr ;
		_pNodeMgr = NULL ;
	}
}

// ͨ��ȫ�ֹ���ָ��������
bool CNodeSrv::Init( ISystemEnv *pEnv )
{
	char szip[128] = {0} ;
	if ( ! pEnv->GetString("node_listen_ip" , szip) ){
		OUT_ERROR( NULL, 0, NULL, "get msg_listen_ip failed" ) ;
		return false ;
	}
	_listen_ip = szip ;

	int port = 0 ;
	if ( ! pEnv->GetInteger( "node_listen_port", port ) ){
		OUT_ERROR( NULL, 0, NULL, "get msg_listen_port failed" ) ;
		return false ;
	}
	_listen_port = port ;

	int nvalue = 10 ;
	if ( pEnv->GetInteger( "node_tcp_thread", nvalue )  ){
		_thread_num = nvalue ;
	}

	// ��ʼ�����������
	_pNodeMgr = new CNodeMgr(pEnv) ;

	// ���÷ְ��Ķ�����
	setpackspliter( &_pack_spliter  );

	return true ;
}

// ��ʼ����������
bool CNodeSrv::Start( void )
{
	return StartServer( _listen_port , _listen_ip.c_str() , _thread_num ) ;
}

// STOP����
void CNodeSrv::Stop( void )
{
	StopServer() ;
}

// �������ݵ���
void CNodeSrv::on_data_arrived( socket_t *sock, const void *data, int len)
{
	// �������ݵ���
	OUT_INFO( sock->_szIp, sock->_port, "NodeSrv" , "%s, fd %d, length %d" , CNodeParser::Decode((char*)data, len ) , sock->_fd , len ) ;
	OUT_HEX( sock->_szIp, sock->_port, "NodeSrv" , (char*)data , len ) ;

	_pNodeMgr->Process( sock, (const char *)data, len ) ;
}

bool CNodeSrv::HandleData( socket_t *sock, const char *data, int len )
{
	// �������ݵ���
	if ( sock == NULL ) return false ;

	if ( ! SendData( sock, data, len ) ) {
		OUT_ERROR( sock->_szIp, sock->_port , "NodeSrv", "%s , fd %d, length %d" , CNodeParser::Decode( data, len ) , sock->_fd, len ) ;
		OUT_HEX( sock->_szIp, sock->_port, "Send" , data ,len ) ;
		return false ;
	}
	OUT_SEND( sock->_szIp, sock->_port, "NodeSrv" , "%s, fd %d, length %d" , CNodeParser::Decode( data, len ) , sock->_fd, len ) ;
	OUT_HEX( sock->_szIp, sock->_port, "Send" , data ,len ) ;

	return true ;
}

void CNodeSrv::on_dis_connection( socket_t *sock )
{
	// �������¼�
	_pNodeMgr->Close( sock ) ;
}

void CNodeSrv::CloseClient( socket_t *sock )
{
	// �ر����Ӳ���������Ҫ֪ͨ�����¼�
	_tcp_handle.close_socket( sock, false ) ;
}

void CNodeSrv::TimeWork()
{
}

void CNodeSrv::NoopWork()
{
}

