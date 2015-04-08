/*
 * fileclient.h
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 *
 *  Memo: �ļ���д�ͻ��˳���
 */

#ifndef __FILECLIENT_H_
#define __FILECLIENT_H_

#include <inetfile.h>
#include <NetHandle.h>
#include <protocol.h>
#include <interpacker.h>

class CWaitObjMgr ;
class CAsynFile : public CNetHandle, public INetFile
{
public:
	CAsynFile() ;
	~CAsynFile() ;

	// ������
	int open( const char *ip, int port , const char *user, const char *pwd ) ;
	// д���ļ�����
	int write( const char *path , const char *data, int len ) ;
	// ��ȡ�ļ�����
	int read( const char *path, DataBuffer &duf ) ;
	// �ر�����
	void close( void ) ;

protected:
	// ��������
	bool mysend( void *data, int len ) ;
	// ��������ͷ��
	void buildheader( DataBuffer &buf, unsigned int seq, unsigned short cmd, unsigned int len ) ;
	// �������ݵ����¼�
	void on_data_arrived( socket_t *sock, const void* data, int len ) ;
	// �Ͽ�����
	void on_dis_connection( socket_t *sock ) ;
	// �����ӵ���
	void on_new_connection( socket_t *sock, const char* ip, int port){}

private:
	// ���ӵ�FDֵ
	socket_t	   *_sock ;
	// ���ݰ��������
	CBigSpliter  	_packspliter ;
	// �Ƿ����ӳɹ�
	bool 			_connected ;
	// �ȴ����ݶ���
	CWaitObjMgr    *_waitmgr ;
};

#endif /* FILECLIENT_H_ */
