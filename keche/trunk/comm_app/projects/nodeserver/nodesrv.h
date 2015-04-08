/*
 * nodesrv.h
 *
 *  Created on: 2011-11-4
 *      Author: humingqing
 */

#ifndef __NODESRV_H__
#define __NODESRV_H__

#include "nodemgr.h"
#include <BaseServer.h>
#include <packspliter.h>

class CNodeSrv : public BaseServer , public INodeSrv
{
public:
	CNodeSrv() ;
	~CNodeSrv() ;

	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	// �������ݵ���
	virtual void on_data_arrived( socket_t *sock, const void *data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	// �����ӵ���
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};
	// ʱ���߳�
	virtual void TimeWork();
	// �����߳�
	virtual void NoopWork();
	// �������ݴ���
	virtual bool HandleData( socket_t *sock, const char *data, int len ) ;
	// �ڲ�����쳣�Ķ�������
	virtual void CloseClient( socket_t *sock ) ;

private:
	// ��������ָ��
	ISystemEnv    *_pEnv ;
	// �����߳�
	unsigned int   _thread_num ;
	// �ڵ�������
	CNodeMgr	  *_pNodeMgr ;
	// �ְ�����
	CPackSpliter   _pack_spliter ;
};


#endif /* NODESRV_H_ */
