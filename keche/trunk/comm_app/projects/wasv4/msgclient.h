/**********************************************
 * MinitoryClient.h
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/

#ifndef __MSGCLIENT_H_
#define __MSGCLIENT_H_

#include "interface.h"
#include <DataPool.h>
#include <BaseClient.h>
#include <GbProtocolHandler.h>
#include <protocol.h>
#include <Monitor.h>
#include <statflux.h>
#include "filecache.h"
#include <interpacker.h>

#define MAX_TIMEOUT_USER     90
#ifdef  MAX_SPLITPACK_LEN
#undef  MAX_SPLITPACK_LEN
#endif
#define MAX_SPLITPACK_LEN    1000   // ������ݳ���
#define WAS_CLIENT_ID        "wascache"

class MsgClient :
	public BaseClient , public IClient , public IOHandler
{
	struct _SetData
	{
		unsigned short msgid  ;
		DataBuffer     buffer ;

		_SetData() {
			msgid  = 0 ;
		}
	};
public:
	MsgClient(CacheDataPool &cache_data_pool) ;
	~MsgClient();

	// ����
	bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ
	bool Start( void ) ;
	// ֹͣ
	void Stop( void ) ;
	// �׷�����
	void HandleUpData( const char *data, int len ) ;
	// �������ݷ׷��ӿ�
	int HandleQueue( const char *sid , void *buf, int len , int msgid = 0 ) ;

protected:
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );
	virtual void TimeWork();
	virtual void NoopWork();
	virtual int  build_login_msg(User &user, char *buf, int buf_len);
	// ����ʧ�ܵĻ�������
	virtual void on_send_failed( socket_t *sock , void* data, int len) ;

private:
	// �������ݴ���
	void SendMsgData( _SetData &val, const string &car_id, const string &mac_id, const string &command , const string &seq ) ;
	// ά���û�����״̬
	void HandleUserStatus();
	// ����RC4����
	bool SendRC4Data( socket_t *sock , const char *data, int len ) ;

public:
	void HandleDsetpMsg(string &line);
	void HandleDcallMsg(string &line);
	void HandleDctlmMsg(string &line);
	void HandleDsndmMsg(string &line);
	void HandleReqdMsg( string &line) ;
	void HandleDgetpMsg(string &line) ;

private:
	// ��������
	ISystemEnv		  * _pEnv ;
	// �����߳���
	unsigned int 		_thread_num ;
	// Э���������
	GbProtocolHandler * _gb_proto_handler;
	// ���ݶ����
	CacheDataPool 	   &_cache_data_pool;
	// �ְ�����
	CInterSpliter		_pack_spliter ;
	// ��������ͳ��
	CStatFlux			_sendstat ;
	// ���ݶ���
	CFileCache			_dataqueue ;
};

#endif /* MINITORYCLIENT_H_ */
