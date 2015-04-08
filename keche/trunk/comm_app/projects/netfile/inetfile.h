/*
 * inetfile.h
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 *  Memo: �����ļ���д�ӿڶ���
 */

#ifndef __INETFILE_H__
#define __INETFILE_H__

#include <databuffer.h>

#define FILE_RET_NOCONN	   -1	 // û������״̬
#define FILE_RET_SUCCESS   	0  	 // �ɹ����ؽ��
#define FILE_RET_FAILED	   	1  	 // ����ʧ�ܵĽ��
#define FILE_RET_TIMEOUT   	2  	 // ��ʱ���ش���
#define FILE_RET_SENDERR    3    // ��������ʧ��
#define FILE_RET_READERR    4    // ��ȡ���ݴ���
#define FILE_MAX_WAITTIME   10   // ��ȴ���ʱ��

// �����ļ�����ӿ�
class INetFile
{
public:
	virtual ~INetFile() {}
	// ������
	virtual int open( const char *ip, int port , const char *user, const char *pwd ) = 0 ;
	// д���ļ�����
	virtual int write( const char *path , const char *data, int len ) = 0 ;
	// ��ȡ�ļ�����
	virtual int read( const char *path, DataBuffer &duf ) = 0 ;
	// �ر�����
	virtual void close( void ) = 0 ;
};

// �����ļ��������
class NetFileMgr
{
public:
	enum NET_MODE{ SYN_MODE = 0, ASYN_MODE = 1 } ;
	// ȡ���ļ���ȡ����
	static INetFile * getfileobj( NET_MODE mode ) ;
	// �ͷ��ļ�����
	static void release( INetFile *p ) ;
} ;

#endif /* INETFILE_H_ */
