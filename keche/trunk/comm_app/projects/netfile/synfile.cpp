/*
 * netfile.cpp
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 */

#include "synfile.h"
#include <tools.h>
#include <interheader.h>
#include <UtilitySocket.h>
#include <errno.h>

#define WRITE_BUFFER 102400

CSynFile::CSynFile(): _fd(-1), _seq(0)
{

}

CSynFile::~CSynFile()
{
	myclose() ;
}

// ������
int CSynFile::open( const char *ip, int port , const char *user, const char *pwd )
{
	share::Guard guard( _mutex ) ;

	// �����ǰ���ӹ���ֱ�ӹر�Ȼ�����������Ӵ���
	if ( _fd > 0 )  myclose() ;
	// ���ӷ���˵Ĵ���
	_fd = myconnect( ip, port ) ;
	if ( _fd == -1 )
		return FILE_RET_NOCONN ;

	DataBuffer buf ;
	buildheader( buf, BIG_OPEN_REQ, sizeof(bigloginreq) ) ;

	bigloginreq req ;
	safe_memncpy( (char *)req.user, user , sizeof(req.user) ) ;
	safe_memncpy( (char* )req.pwd,  pwd  , sizeof(req.pwd) ) ;

	buf.writeBlock( &req, sizeof(req) ) ;

	if ( ! mywrite( buf.getBuffer() , buf.getLength() ) ) {
		return FILE_RET_SENDERR ;
	}
	// ���ؽ��
	return myread( NULL ) ;
}

// д���ļ�����
int CSynFile::write( const char *path , const char *data, int len )
{
	share::Guard guard( _mutex ) ;

	if ( _fd < 0 )
		return FILE_RET_NOCONN ;

	DataBuffer buf ;
	buildheader( buf, BIG_WRITE_REQ, sizeof(bigwritereq) + len ) ;

	bigwritereq req ;
	req.data_len = htonl(len) ;
	safe_memncpy( (char*)req.path, path, sizeof(req.path) ) ;
	buf.writeBlock( &req, sizeof(req) ) ;
	buf.writeBlock( data, len ) ;

	if ( ! mywrite( buf.getBuffer(), buf.getLength() ) ) {
		return FILE_RET_SENDERR ;
	}

	return myread( NULL ) ;
}

// ��ȡ�ļ�����
int CSynFile::read( const char *path, DataBuffer &buf )
{
	share::Guard guard( _mutex ) ;

	if ( _fd < 0 )
		return FILE_RET_NOCONN ;

	DataBuffer out ;
	buildheader( out, BIG_READ_REQ, sizeof(bigreadreq) ) ;

	bigreadreq req ;
	safe_memncpy( (char*)req.path, path, sizeof(req.path) ) ;
	out.writeBlock( &req, sizeof(req) ) ;

	if ( ! mywrite( out.getBuffer(), out.getLength() ) ) {
		return FILE_RET_SENDERR ;
	}

	return myread( &buf ) ;
}

// �ر��ļ�����
void CSynFile::close( void )
{
	share::Guard guard( _mutex ) ;

	if ( _fd < 0 )
		return ;

	// ȡ�ö�Ӧ�����
	DataBuffer buf ;

	buildheader( buf, BIG_CLOSE_REQ, 0 ) ;

	if ( ! mywrite( buf.getBuffer(), buf.getLength() ) ) {
		myclose() ;
		return ;
	}

	// ��ȡ����
	int result = myread( NULL ) ;
	if ( result == FILE_RET_SUCCESS ) {

	}

	myclose() ;
}

// ��������ͷ��
void CSynFile::buildheader( DataBuffer &buf, unsigned short cmd, unsigned int len )
{
	bigheader header ;
	header.cmd = htons( cmd ) ;
	header.seq = htonl( ++_seq ) ;
	header.len = htonl( len ) ;
	buf.writeBlock( &header, sizeof(header) ) ;
}

// ���ӷ�������IP�Ͷ˿�
int CSynFile::myconnect( const char *ip, unsigned short port )
{
	int sockfd = -1 ;
	// ����SOCKET
	if((sockfd=UtilitySocket::BaseSocket::socket(SOCK_STREAM))<0) {
		printf( "%s-%d:socket error:%s\n", ip, port, strerror(errno) );
		return -1 ;
	}
	// ���ӷ�����
	if ( ! UtilitySocket::BaseSocket::connect( sockfd, ip , port ) ) {
		printf( "%s-%d:connect:%s\n", ip, port, strerror(errno) );
		UtilitySocket::BaseSocket::close( sockfd ) ;
		return -1 ;
	}
	// ����Ϊ������ģʽ����
	// UtilitySocket::BaseSocket::setNonBlocking( sockfd ) ;
	_fd = sockfd ;

	return sockfd ;
}

// �ر�����
void CSynFile::myclose( void )
{
	if ( _fd <=0 )
		return ;

	UtilitySocket::BaseSocket::close( _fd ) ;
	_fd = -1 ;
}

// ��ȡ��������
inline bool readdata( int fd, DataBuffer &buf, int len )
{
	int ret = 0 ;
	buf.ensureFree( len ) ;

	while( ( ret = ::read( fd, buf.getFree() , len ) )  > 0 ) {
		// �����Ѷ�ȡ���ݵĳ���
		buf.pourData( ret ) ;
		len = len - ret ;
		// �����ȡ����Ϊ���BUF�ͼ����ٶ�
		if ( len > 0 ) {
			// ������������ݹ�������һ�ν���
			continue ;
		}
		break ;
	}
	return ( len == 0 ) ;
}

// ��ȡ����
int CSynFile::myread( DataBuffer *pbuf )
{
	DataBuffer buf ;
	if ( ! readdata( _fd, buf, sizeof(bigheader)) )
		return FILE_RET_READERR ;

	// ����ͷ�¼�
	bigheader *header = (bigheader *) ( buf.getBuffer() ) ;
	if ( header->ver[0] != BIG_VER_0 || header->ver[1] != BIG_VER_1 ) {
		return FILE_RET_READERR ;
	}

	unsigned short cmd = ntohs( header->cmd ) ;
	// unsigned int   seq = ntohl( header->seq ) ;
	// printf( "on data arrived, cmd %04x, length:%d\n", cmd , len ) ;
	unsigned int needlen = ntohl( header->len ) ;
	if ( needlen == 0 )
		return BIG_ERR_SUCCESS ;

	// ��ȡ�����������
	if ( ! readdata( _fd, buf, needlen ) )
		return FILE_RET_READERR ;

	const char *ptr = ( const char *) buf.getBuffer() ;

	unsigned char result = FILE_RET_SUCCESS ;
	switch( cmd )
	{
	case BIG_OPEN_RSP:
		{
			bigloginrsp *rsp = ( bigloginrsp *)( ptr + sizeof(bigheader) ) ;
			if ( rsp->result == BIG_ERR_SUCCESS ) {
			}
			result = rsp->result ;
		}
		break ;
	case BIG_READ_RSP:
		{
			bigreadrsp *rsp = ( bigreadrsp *)( ptr + sizeof(bigheader) ) ;
			unsigned int dlen = ntohl( rsp->data_len ) ;
			if ( dlen > 0 && rsp->result == BIG_ERR_SUCCESS ) {
				pbuf->writeBlock( (void*)(ptr+sizeof(bigheader)+sizeof(bigreadrsp)), (int)dlen ) ;
			}
			result = rsp->result ;
		}
		break ;
	case BIG_WRITE_RSP:
		{
			bigwritersp *rsp = ( bigwritersp *) ( ptr + sizeof(bigheader) ) ;
			result = rsp->result ;
		}
		break ;
	case BIG_CLOSE_RSP:
		{
			result = BIG_ERR_SUCCESS ;
		}
		break ;
	}
	return result ;
}

// д����
bool CSynFile::mywrite( const char *buf, int len )
{
	int offset 		= 0  ;
	const char *ptr = buf ;

	// WRITE_BUFFER
	while( offset < len ) {
		int send = ( ( len - offset ) > WRITE_BUFFER ) ? WRITE_BUFFER : ( len - offset ) ;
		int ret = ::write( _fd, ptr+offset, send ) ;
		if ( ret < 0 ) {
			break ;
		}
		offset = offset + ret ;
	}
	return ( offset == len ) ;
}



