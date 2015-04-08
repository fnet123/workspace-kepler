/*
 * exchserver.h
 *
 *  Created on: 2014-5-23
 *      Author: ycq
 *  ��Ϣת������
 */

#ifndef _PHOTOSVR_H_
#define _PHOTOSVR_H_ 1

#include "interface.h"
#include "exchspliter.h"
#include <BaseServer.h>

#include <set>
using std::set;
#include <vector>
using std::vector;
#include <string>
using std::string;

class ExchServer : public BaseServer , public IExchServer {
	// ��������ָ��
	ISystemEnv			    *_pEnv ;
	// �����߳���
	unsigned  int		     _thread_num ;
	// ���ݷְ�����
	CExchSpliter   		     _pack_spliter;
	// �����û�����
	OnlineUser   		     _online_user;
private:
	size_t enCode(const uint8_t *src, size_t len, uint8_t *dst);
	size_t deCode(const uint8_t *src, size_t len, uint8_t *dst);
public:
	ExchServer() ;
	~ExchServer() ;

	// �����ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	////////////////////////////////////////////////////////////
	virtual void on_data_arrived( socket_t *sock, const void *data, int len);
	virtual void on_new_connection( socket_t *sock, const char* ip, int port);
	virtual void on_dis_connection( socket_t *sock );


	virtual void TimeWork();
	virtual void NoopWork();

	virtual void HandleData(const char *data, int len) ;
};

#endif//_PHOTOSVR_H_
