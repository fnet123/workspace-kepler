/*
 * packer.h
 *
 *  Created on: 2012-8-31
 *      Author: humingqing
 *
 *  ��������c��������F���������л��c������
 *
 */

#ifndef __PACKER_H__
#define __PACKER_H__

#include <qstring.h>
#include <databuffer.h>

// ���ݽ�������
class CPacker: public DataBuffer
{
public:
	CPacker( const char *p, int len ) ;
	CPacker() {};
	~CPacker(){};
	// ��ȡByte������
	unsigned char  readByte( void ) { return readInt8();}
	// ����Short������
	unsigned short readShort( void ){ return readInt16();};
	// ��ȡ��������
	unsigned int   readInt( void ) { return readInt32(); } ;
	// ���ַ����ε�����
	unsigned int   readString( CQString &s ) ;
	// �����ݿ�
	unsigned int   readBytes( void *buf, int len ) ;
	// ��ȡʱ��
	uint64_t 	   readTime( void ) { return readInt64(); }
	// ��λ��дλ��
	void 		   seekRead( int offset ) { seekPos(offset); }
	// ȡ�ö����ݳ���
	int 		   GetReadLen( void ) { return getLength(); }
	// д��һ���ַ�
	void 		   writeByte( unsigned char c ) { writeInt8(c); }
	// д������ε�����
	void 		   writeShort( unsigned short n ) { writeInt16(n); }
	// д�����ε�����
	void 		   writeInt( unsigned int n ) { writeInt32(n); }
	// д��ʱ�������
	void		   writeTime( uint64_t n ) { writeInt64(n); }
	// д���ַ����ε�����
	void 		   writeString( CQString &s ) ;
	// д�����ݿ�
	void 		   writeBytes( void *buf, int len ) { writeBlock(buf,len); }
	// д��̶���������
	void 		   writeFix( const char *sz, int len , int max ) ;
};


#endif /* PACKER_H_ */
