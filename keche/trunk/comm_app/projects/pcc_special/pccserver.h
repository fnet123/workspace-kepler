/*
 * pccserver.h
 *
 *  Created on: 2011-11-28
 *      Author: humingqing
 *      ��Ҫʵ��PAS�����������Լ��ڲ�Э�����ݵĴ���
 */

#ifndef __PCCSERVER_H__
#define __PCCSERVER_H__

#include "interface.h"
#include <BaseServer.h>
#include <OnlineUser.h>
#include <ProtoParse.h>

#define COMPANY_TYPE 	"PIPE"
#define WEB_TYPE 		"WEB"
#define STORAGE_TYPE 	"SAVE"
#define PROXY_TYPE		"PROXY"
#define ROUTE_TYPE      "ROUTE"
#define MAX_TIMEOUT  180

class CPccServer : public BaseServer, public IPccServer
{
public:
	CPccServer() ;
	~CPccServer() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ�߳�
	virtual bool Start( void ) ;
	// ֹͣ����
	virtual void Stop( void ) ;
	// ����·��������Ͽ�������������
	virtual void Close( int accesscode , unsigned short msgid, int reason ) ;
	// ��PCC·�ɸ���·������
	virtual void updateAreaids(const string &areaids);
protected:
	// ʵ�ַ������Ľӿ�
	virtual void on_data_arrived( socket_t *sock, const void* data, int len );
	virtual void on_dis_connection( socket_t *sock );
	virtual void on_new_connection( socket_t *sock, const char* ip, int port ){};

	// �����̺߳Ͷ�ʱ�߳�
	virtual void TimeWork() ;
	virtual void NoopWork() ;

private:
	// ����809Э�������
	void HandleOutData( socket_t *sock, const char *data, int len ) ;
	// �����ڲ��·�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// ����ʱ����
	void HandleOfflineUsers() ;
	// ����809Э������
	bool SendCrcData( socket_t *sock, const char* data, int len ) ;
	// �Ƿ���Ҫ����ӽ���
	bool EncryptData( unsigned char *data, unsigned int len , bool encode ) ;
	// ����PCC��ǰ��Ϣ
	void UpdatePcc( void ) ;

private:
	// ��������ָ��
	ISystemEnv		   *_pEnv ;
	// �����û�����
	OnlineUser 			_online_user;
	// �����߳�
	unsigned int 		_thread_num ;
	// 809 Э�����
	ProtoParse			_proto_parse;
	//���м��ƽ̨��ʡ����
	string              _areaids;
	//����·�����ü���
	share::Mutex        _mutex ;
};

#endif /* PCCSERVER_H_ */
