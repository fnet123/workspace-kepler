/*
 * packer.cpp
 *
 *  Created on: 2012-8-31
 *      Author: humingqing
 *
 *  ʵ��һ�����ݲ����������һ����Ϊֻ���������ԣ�һ����Ϊ�ڲ���������
 *
 */

#include <stdio.h>
#include "packer.h"

CPacker::CPacker( const char *p, int len )
{
	_ronly = 1 ; // ����ͨ���ⲿ��ֵ����������
	_pbuf  = _pdata = (unsigned char*)p ;
	_pend  = _pfree = (unsigned char*)(p+len);
}

// ���ַ����ε�����
unsigned int CPacker::readString( CQString &buf )
{
	int len = readInt();
	// �����ȡ�����ݳ��ȴ�����󳤶Ⱦ�ֱ�ӷ�����
	if ( len <= 0 || len > getLength() )
		return 0;

	// �����ڴ�ռ�
	buf.AppendBuffer( NULL, len );
	// ֱ�ӽ��ڴ����ݿ��������󻺴���
	return readBytes( ( char* ) buf.GetBuffer(), len );
}

// �����ݿ�
unsigned int CPacker::readBytes( void *buf, int len )
{
	memset( buf, 0, len );

	if ( ! readBlock( buf, len ) )
		return 0;
	return len;
}

// д���ַ����ε�����
void CPacker::writeString( CQString &s )
{
	writeInt32( s.GetLength() );
	if ( s.GetLength() > 0 ) {
		writeBlock( s.GetBuffer(), s.GetLength() );
	}
}

// д��̶���������
void CPacker::writeFix( const char *sz, int len , int max )
{
	if ( len > max ) {
		writeBlock( sz, max ) ;
	} else {
		writeBlock( sz, len ) ;
		writeFill( 0, max-len ) ;
	}
}
