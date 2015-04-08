/*
 * mybase64.h
 *
 *  Created on: 2012-3-2
 *      Author: humingqing
 *
 *  ������ƽ̨�����BASE64����
 */

#ifndef __MYBASE64_H__
#define __MYBASE64_H__

#include <string>

class CBase64Ex
{
public:
	CBase64Ex() ;
	~CBase64Ex() ;
	// ���ܴ���
	bool Encode( const char *data, int len ) ;
	// ���ܴ���
	bool Decode( const char *data, int len ) ;
	// ȡ������
	char * GetBuffer( void ) { return _pData; }
	// ȡ�ó���
	int    GetLength() { return _size; }
	// ��������
	const std::string  EncodeEx( const char *data, int len ) ;
	// ��������
	const std::string  DecodeEx( const char *data, int len ) ;

private:
	// ���ݶ���
	char *_pData ;
	// ���ݴ�С
	int   _size  ;
};


#endif /* MYBASE64_H_ */
