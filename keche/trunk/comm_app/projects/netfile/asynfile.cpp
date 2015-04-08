/*
 * fileclient.cpp
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 */

#include "asynfile.h"
#include <tools.h>
#include "waitmgr.h"

CAsynFile::CAsynFile():_sock(NULL),  _connected(false)
{
	_waitmgr = NULL ;
	_waitmgr = new CWaitObjMgr ;

	// ��������
	_tcp_handle.init( 1, 1 ) ;
	_tcp_handle.setpackspliter(&_packspliter) ;
}

CAsynFile::~CAsynFile()
{
	if ( _sock != NULL ) {
		_tcp_handle.close_socket( _sock ) ;
	}
	_tcp_handle.uninit() ;

	if ( _waitmgr != NULL ) {
		delete _waitmgr ;
		_waitmgr = NULL ;
	}
}

// ������
int CAsynFile::open( const char *ip, int port , const char *user, const char *pwd )
{
	// �����ǰ���ӹ���ֱ�ӹر�Ȼ�����������Ӵ���
	if ( _sock != NULL ) _tcp_handle.close_socket( _sock ) ;
	// ���ӷ���˵Ĵ���
	_sock = _tcp_handle.connect_nonb( ip, port, FILE_MAX_WAITTIME ) ;
	if ( _sock == NULL )
		return FILE_RET_NOCONN ;

	unsigned int seq = _waitmgr->GenSequeue() ;

	_waitobj *obj = _waitmgr->AllocObj( seq ) ;
	buildheader( obj->_inbuf, seq, BIG_OPEN_REQ, sizeof(bigloginreq) ) ;

	bigloginreq req ;
	safe_memncpy( (char *)req.user, user , sizeof(req.user) ) ;
	safe_memncpy( (char* )req.pwd,  pwd,   sizeof(req.pwd) ) ;

	obj->_inbuf.writeBlock( &req, sizeof(req) ) ;

	if ( ! mysend( obj->_inbuf.getBuffer() , obj->_inbuf.getLength() ) ) {
		_waitmgr->RemoveObj( seq ) ;
		return FILE_RET_SENDERR ;
	}
	obj->_monitor.wait( FILE_MAX_WAITTIME ) ;
	// ȡ�ý����
	unsigned char result = obj->_result ;
	// �Ƴ�����
	_waitmgr->RemoveObj( seq ) ;
	// ���ؽ��
	return result;
}

// д���ļ�����
int CAsynFile::write( const char *path , const char *data, int len )
{
	if (! _connected )
		return FILE_RET_NOCONN ;

	unsigned int seq = _waitmgr->GenSequeue() ;

	_waitobj *obj = _waitmgr->AllocObj( seq ) ;
	buildheader( obj->_inbuf, seq, BIG_WRITE_REQ, sizeof(bigwritereq) + len ) ;

	bigwritereq req ;
	req.data_len = htonl(len) ;
	safe_memncpy( (char*)req.path, path, sizeof(req.path) ) ;
	obj->_inbuf.writeBlock( &req, sizeof(req) ) ;

	obj->_inbuf.writeBlock( data, len ) ;

	if ( ! mysend( obj->_inbuf.getBuffer(), obj->_inbuf.getLength() ) ) {
		_waitmgr->RemoveObj( seq ) ;
		return FILE_RET_SENDERR ;
	}
	obj->_monitor.wait( FILE_MAX_WAITTIME ) ;

	unsigned char result = obj->_result ;

	_waitmgr->RemoveObj( seq ) ;

	return result ;
}

// ��ȡ�ļ�����
int CAsynFile::read( const char *path, DataBuffer &buf )
{
	if (! _connected )
		return FILE_RET_NOCONN ;

	unsigned int seq = _waitmgr->GenSequeue() ;

	_waitobj *obj = _waitmgr->AllocObj( seq , &buf ) ;
	buildheader( obj->_inbuf, seq, BIG_READ_REQ, sizeof(bigreadreq) ) ;

	bigreadreq req ;
	safe_memncpy( (char*)req.path, path, sizeof(req.path) ) ;
	obj->_inbuf.writeBlock( &req, sizeof(req) ) ;

	if ( ! mysend( obj->_inbuf.getBuffer(), obj->_inbuf.getLength() ) ) {
		_waitmgr->RemoveObj( seq ) ;
		return FILE_RET_SENDERR ;
	}
	obj->_monitor.wait( FILE_MAX_WAITTIME ) ;

	unsigned char result = obj->_result ;

	_waitmgr->RemoveObj( seq ) ;

	return result ;
}

void CAsynFile::close( void )
{
	if (! _connected )
		return ;

	// ���ùرպ�Ĵ���
	_connected = false ;
	// ȡ�ö�Ӧ�����
	unsigned int seq = _waitmgr->GenSequeue() ;

	_waitobj *obj = _waitmgr->AllocObj( seq ) ;
	buildheader( obj->_inbuf, seq , BIG_CLOSE_REQ, 0 ) ;

	mysend( obj->_inbuf.getBuffer(), obj->_inbuf.getLength() ) ;

	obj->_monitor.wait( FILE_MAX_WAITTIME ) ;

	_waitmgr->RemoveObj( seq ) ;

	_tcp_handle.close_socket( _sock ) ;
}

// ��������
bool CAsynFile::mysend( void *data, int len )
{
	// OUT_HEX( NULL, 0, NULL, (const char *)data, len ) ;
	return _tcp_handle.deliver_data( _sock, data, len ) ;
}

// ��������ͷ��
void CAsynFile::buildheader( DataBuffer &buf, unsigned int seq, unsigned short cmd, unsigned int len )
{
	bigheader header ;
	header.cmd = htons( cmd ) ;
	header.seq = htonl( seq ) ;
	header.len = htonl( len ) ;
	buf.writeBlock( &header, sizeof(header) ) ;
}

// �������ݵ����¼�
void CAsynFile::on_data_arrived( socket_t *sock, const void* data, int len )
{
	//const char *ip    = _tcp_handle.get_remote_ip(fd);
	//unsigned int port = _tcp_handle.get_remote_port(fd);

	if ( len < (int)sizeof(bigheader) ) {
		return ;
	}

	const char *ptr = (const char *) data ;

	bigheader *header  = (bigheader*) (ptr) ;
	unsigned short cmd = ntohs( header->cmd ) ;
	unsigned int   seq = ntohl( header->seq ) ;

	// printf( "on data arrived, cmd %04x, length:%d\n", cmd , len ) ;

	switch( cmd )
	{
	case BIG_OPEN_RSP:
		{
			bigloginrsp *rsp = ( bigloginrsp *)( ptr + sizeof(bigheader) ) ;
			if ( rsp->result == BIG_ERR_SUCCESS ) {
				_connected = true ;
			}
			_waitmgr->ChangeObj( seq, rsp->result, NULL, 0 ) ;
		}
		break ;
	case BIG_READ_RSP:
		{
			bigreadrsp *rsp = ( bigreadrsp *)( ptr + sizeof(bigheader) ) ;
			unsigned int dlen = ntohl( rsp->data_len ) ;

			_waitmgr->ChangeObj( seq, rsp->result, (void*)(ptr+sizeof(bigheader)+sizeof(bigreadrsp)), (int)dlen ) ;
		}
		break ;
	case BIG_WRITE_RSP:
		{
			bigwritersp *rsp = ( bigwritersp *) ( ptr + sizeof(bigheader) ) ;
			_waitmgr->ChangeObj( seq, rsp->result, NULL, 0 ) ;
		}
		break ;
	case BIG_CLOSE_RSP:
		{
			_waitmgr->ChangeObj( seq, FILE_RET_SUCCESS, NULL, 0 ) ;
		}
		break ;
	}
}

// �Ͽ�����
void CAsynFile::on_dis_connection( socket_t *sock )
{
	if ( sock != _sock ) {
		return ;
	}
	_tcp_handle.close_socket( sock ) ;
	_connected = false ;
	_sock = NULL ;

	//printf( "recv disconnect data fd %d" , fd ) ;

	_waitmgr->NotfiyAll() ;
}




