/*
 * nodeclient.h
 *
 *  Created on: 2011-11-10
 *      Author: humingqing
 */

#ifndef __NODECLIENT_H__
#define __NODECLIENT_H__

#include <interface.h>
#include <inodeface.h>
#include <BaseClient.h>
#include <nodeheader.h>
#include <msgbuilder.h>
#include <packspliter.h>

class CNodeClient : public BaseClient,
	public INodeClient, public IMsgNotify
{
public:
	CNodeClient() ;
	~CNodeClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ�߳�
	virtual bool Start( void ) ;
	// ֹͣ�߳�
	virtual void Stop( void ) ;

	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};
	// ��ʱ�߳�
	virtual void TimeWork();
	// �����߳�
	virtual void NoopWork();
	// ������½����Ϣ
	virtual int  build_login_msg( User &user , char *buf, int buf_len ) ;
	// ������ɾ��֪ͨ���ݶ���
	virtual void NotifyMsgData( socket_t *sock, MsgData *p , ListFd &fds, unsigned int op ) ;
	// �����û���
	virtual void UserName( void ) ;
	// ���������õ�MSG
	virtual void GetServerMsg( void ) ;

private:
	// ����������·����
	void SendLinkTest( void ) ;
	// ��������
	bool SendDataEx( socket_t *sock, const char *data, int len ) ;

private:
	// ��������ָ��
	ISystemEnv   *_pEnv ;
	// �ڵ�ID��
	unsigned int  _nodeid ;
	// �Ƿ�����ģ��
	bool 		  _enable ;
	// MSG��������IP
	string 		  _send_ip ;
	// MSG�������Ķ˿�
	int 		  _send_port ;
	// ��Ϣ�ڴ�������
	IAllocMsg	 *_pAlloc ;
	// ��Ϣ�������
	CMsgBuilder	 *_pBuilder ;
	// �첽�ȴ�����
	IWaitGroup	 *_pWaitQueue;
	// ���÷ְ�����
	CPackSpliter  _pack_spliter;
	// ȡ�ýڵ��û���
	string 		   _user_name ;
	// ȡ������
	string 		   _user_pwd ;
	// MSG������
	string 		   _msg_pwd ;
	// MSG���û���
	string 		   _msg_user ;
	// ��Ϣ����IP
	string 		   _msg_ip ;
	// ��Ϣ����˿�
	int 		   _msg_port ;
	// ���һ������ʱ��
	time_t		   _last_req ;
};

#endif /* NODECLIENT_H_ */
