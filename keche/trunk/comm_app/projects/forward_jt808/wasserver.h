/**********************************************
 * wasserver.h
 *
 *  Created on: 2014-8-19
 *      Author: ycq
 *********************************************/
#ifndef __WASSERVER_H__
#define __WASSERVER_H__

#include <std.h>
#include "interface.h"
#include <BaseServer.h>
#include <protocol.h>
#include <interpacker.h>

class WasServer: public BaseServer, public IWasServer {
	// ��������
	ISystemEnv		    *_pEnv ;
	// �����û�
	OnlineUser 			 _online_user;
	// �߳���
	unsigned int 		 _thread_num ;
	// ���ӿ���ʱ�䣬��
	unsigned int       _max_timeout;
	// �ְ�����
	C808Spliter		 	 _pack_spliter ;
public:
	WasServer();
	~WasServer();
	// ��ʼ��ϵͳ
	bool Init(ISystemEnv *pEnv);
	// ��ʼ����ϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// �������ݴ�����
	bool HandleData(const string &sim, const uint8_t *ptr, size_t len);
	// �ж��ն��Ƿ�����
	bool ChkTerminal(const string &sim);
protected:
	virtual void on_data_arrived( socket_t *sock , const void *data, int len );
	virtual void on_dis_connection( socket_t *sock );
	virtual void on_new_connection( socket_t *sock , const char* ip, int port );

	virtual void TimeWork();
	virtual void NoopWork();

	// ����7E���������
	bool Send7ECodeData( socket_t *sock , const char *data, int len ) ;
	// ������Ӧ����
	bool SendResponse( socket_t *sock, const char *id , const char *data, int len ) ;
};

#endif//__WASSERVER_H__
