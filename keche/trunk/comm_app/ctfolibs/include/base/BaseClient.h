/**********************************************
 * BaseClient.h
 *
 *  Created on: 2010-6-24
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments: 2011-07-24 humingqing
 *    �޸ģ���Ҫȥ��������У����̶߳���ʹ���Լ�д���̳߳�����������
 *********************************************/

#ifndef BASECLIENT_H_
#define BASECLIENT_H_

#include <NetHandle.h>
#include <OnlineUser.h>
#include <BaseTools.h>
#include <Thread.h>

#define THREAD_NOMARL     1
#define THREAD_TIME	      2
#define THREAD_NOOP 	  3

class BaseClient : public CNetHandle, public share::Runnable
{
public:
	BaseClient();
	virtual ~BaseClient();

	// ��ʼ��UDP�ķ���
	virtual bool StartUDP( const char * connect_ip, int connect_port, int thread , unsigned int timeout = SOCKET_TIMEOUT ) ;
	// ��ʼ�߳�
	virtual bool StartClient(const char* connect_ip, int connect_port, int thread , unsigned int timeout = SOCKET_TIMEOUT );
	// ֹͣ�߳�
	virtual void StopClient( void ) ;
	// �Ƿ�ʼ�߳�
	virtual bool Check( void ) { return _initalized ; }

	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock ){};
	virtual void on_new_connection( socket_t *sock, const char* ip, int port ){};

protected:
	virtual void run( void *param )  ;

	virtual void TimeWork() = 0 ;
	virtual void NoopWork() = 0 ;
	// ʹ�ô˷����ͱ���ʵ��build_login_msg���ӷ�����
	virtual bool ConnectServer(User &user, unsigned int timeout);
	// ������½����
	virtual int build_login_msg( User &user, char *buf, int buf_len ) { return -1; };

	bool SendData( socket_t *sock, const char* data, int len);
	bool CloseSocket( socket_t *sock );

protected:
	User 			 	  	_client_user;
	User					_udp_user ;
	share::ThreadManager  	_noop_thread ;
	share::ThreadManager  	_time_thread ;
	bool 				  	_initalized  ;
	bool 					_udp_inited ;
	bool 					_tcp_inited ;
};

#endif /* BASECLIENT_H_ */
