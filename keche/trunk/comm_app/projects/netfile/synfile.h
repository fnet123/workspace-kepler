/*
 * netfile.h
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 *  Memo: �����ļ�����
 */

#ifndef __NETFILE_H__
#define __NETFILE_H__

#include <inetfile.h>
#include <Mutex.h>

class CSynFile : public INetFile
{
public:
	CSynFile() ;
	~CSynFile() ;

	// ������
	int open( const char *ip, int port , const char *user, const char *pwd ) ;
	// д���ļ�����
	int write( const char *path , const char *data, int len ) ;
	// ��ȡ�ļ�����
	int read( const char *path, DataBuffer &duf ) ;
	// �ر�����
	void close( void ) ;

private:
	// ��������ͷ��
	void buildheader( DataBuffer &buf, unsigned short cmd, unsigned int len ) ;
	// ���ӷ�����
	int  myconnect( const char *ip, unsigned short port ) ;
	// �ر�����
	void myclose( void ) ;
	// ��ȡ����
	int  myread( DataBuffer *buf ) ;
	// д����
	bool mywrite( const char *buf, int len ) ;

private:
	// ���ӵ�FDֵ
	int 		    _fd ;
	// ��Ź������
	unsigned int    _seq ;
	// ͬ��������
	share::Mutex	_mutex ;
};



#endif /* NETFILE_H_ */
