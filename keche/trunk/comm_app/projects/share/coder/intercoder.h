/*
 * intercoder.h
 *
 *  Created on: 2012-4-26
 *      Author: humingqing
 */

#ifndef __INTERCODER_H__
#define __INTERCODER_H__

#define INTER_MAX_LEN  65535
// ǰ�û����ݼӽ���
class CInterCoder
{
public:
	CInterCoder() ;
	~CInterCoder() ;
	// �����㷨
	bool Encode( const char *data, int len ) ;
	// ���ܴ���
	bool Decode( const char *data, int len ) ;
	// ȡ�û���
	const char * Buffer( void ) { return _ptr; }
	// ȡ�ó���
	int Length( void ) { return _len; }

private:
	// ����ָ��
	char *_ptr ;
	// ���ݻ���
	char  _buf[INTER_MAX_LEN] ;
	// ���ݳ���
	int   _len ;
};


#endif /* WASCODER_H_ */
