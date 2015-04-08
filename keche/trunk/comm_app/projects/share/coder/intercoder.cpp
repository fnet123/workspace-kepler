/*
 * intercoder.cpp
 *
 *  Created on: 2012-4-26
 *      Author: humingqing
 */

#include "intercoder.h"
#include "interheader.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ���������㷨
static unsigned char * xorcode( unsigned char c, unsigned char *s, unsigned int len )
{
	for ( unsigned int i = 0; i < len; ++ i ) {
		s[i] = s[i] ^ c ;
	}
	return s ;
}

CInterCoder::CInterCoder()
{
	_ptr = NULL ;
	_len = 0 ;
}

CInterCoder::~CInterCoder()
{

}

// �����㷨
bool CInterCoder::Encode( const char *data, int len )
{
	if ( data[0] == 0x5b && data[len-1] == 0x5d ) {
		_ptr = (char*)data ;
		_len = len  ;
		return true ;
	}
	// ����������Э�鳤��
	if ( len > INTER_MAX_LEN ) {
		return false ;
	}

	srand( time(NULL) ) ;

	interheader inter ;
	inter.tag  = 0x5b ;
	inter.len  = htons(len) ;
	inter.flag = (unsigned char)( rand() % 256 ) ;
	inter.type = 0x00;

	int pos = 0 ;
	_len = len + sizeof(interheader) + sizeof(char) ;
	memcpy( _buf+pos, &inter, sizeof(interheader) ) ;
	pos += sizeof(interheader) ;

	memcpy( _buf+pos, data, len ) ;
	pos += len ;

	_buf[pos] = 0x5d ;

	_ptr = _buf ;

	// ���м򵥼��ܴ���
	xorcode( inter.flag ,
			(unsigned char*)( _buf+sizeof(interheader)) ,
			(unsigned int)len ) ;

	return true ;
}

// ���ܴ���
bool CInterCoder::Decode( const char *data, int len )
{
	if ( len > INTER_MAX_LEN )
		return false ;

	if ( data[0] != 0x5b || data[len-1] != 0x5d ){
		_ptr = (char *)data ;
		_len = len  ;
		return true ;
	}

	// ��������
	interheader *header  = ( interheader *) data ;
	unsigned short nlen = ntohs( header->len ) ;
	if ( nlen > len )
		return false ;

	_len = nlen ;
	memcpy( _buf, data+sizeof(interheader) , nlen ) ;
	_ptr = _buf ;

	// ���Ϊ��������
	xorcode( header->flag,
			(unsigned char *) _buf ,
			(unsigned int) _len
			) ;

	return true ;
}

