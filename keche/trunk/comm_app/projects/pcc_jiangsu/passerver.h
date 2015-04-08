/*
 * passerver.h
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  �������ͷ���
 */

#ifndef __PASSERVER_H__
#define __PASSERVER_H__

#include <interface.h>
#include <BaseServer.h>
#include "mypacker.h"
#include "usermgr.h"
#include "statinfo.h"

//�û��������ӵ�ʱ����û�������ϴ�������Ϊ�ǳ�ʱ�ˡ�
#define SOCK_TIME_OUT  (3 * 10)

class PasServer :
	public BaseServer , public IPasServer, public IPairNotify
{
public:
	PasServer(CStatInfo *stat) ;
	~PasServer() ;
	//////////////////////////ICasClientServer//////////////////////////////
	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	////////////////////////////////////////////////////////////
	virtual void on_data_arrived( socket_t *sock, const void *data, int len);
	// ���յ��Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	// ���յ������ӵ�������
	virtual void on_new_connection( socket_t *sock, const char* ip, int port) ;
	// ��MSGת������������
	virtual void HandleData( const char *data, int len ) ;
	// ��ʱ�߳�
	virtual void TimeWork();
	// ��������
	virtual void NoopWork();
	// �ر��û�֪ͨ
	virtual void CloseUser( socket_t *sock ) ;
	// ֪ͨ�û�����
	virtual void NotifyUser( socket_t *sock , const char *key ) ;

private:
	// ����TCP������
	void HandleTcpData( socket_t *sock, const char *data, int len ) ;
	// ����UDP������
	void HandleUDPData( socket_t *sock, const char *data, int len ) ;
	// ����UDP�ķ�����
	socket_t * ConnectUDP( const char *ip, int port ) ;

private:
	// ��������ָ��
	ISystemEnv			  *_pEnv ;
	// �����߳���
	unsigned  int		   _thread_num ;
	// ���ݷְ�����
	CMyPackSpliter		   _pack_spliter ;
	// �û��������
	CUserMgr			   _user_mgr ;
	// ���һ�μ��
	time_t				   _last_check ;
	// ͳ��ģ��
	CStatInfo			  *_statinfo ;
	// �����XML�ļ�·��
	string 				   _xmlpath ;
};


#endif /* PUSHSERVER_H_ */
