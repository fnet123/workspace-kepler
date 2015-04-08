/**********************************************
 * ClientAccessServer.h
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/


#ifndef __CLIENTACCESSSERVER_H_
#define __CLIENTACCESSSERVER_H_

#include <std.h>
#include "interface.h"
#include <DataPool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseServer.h>
#include "GbProtocolHandler.h"
#include <protocol.h>
#include "scpmedia.h"
#include "packmgr.h"
#include <statflux.h>
#include "blacklist.h"
#include "iplist.h"
#include "queuemgr.h"
#include <interpacker.h>

#ifdef  BUF_LEN
#undef  BUF_LEN
#endif
#define BUF_LEN 		4096
#define CARID_OFFSET 	10000000000
#define STORAGE_SEQ 	"000000_0000000000_0"
#define SEND_TIME_OUT 	20
#define MAX_BUF_LEN     2048
#define MAX_CONN_TIME   60    // ���1������û��λ����Ϣ��Ҫ����һ������

class ClientAccessServer : public BaseServer, public IServer, public IQCaller
{
public:
	ClientAccessServer( CacheDataPool &cache_data_pool ) ;
	~ClientAccessServer() ;
	// ��ʼ��ϵͳ
	bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����ϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// �׷����ݴ���
	void HandleDownData( const char *userid, const char *data_ptr,int  data_len , unsigned int seq = 0 , bool send = true );
	// ȡ�����߳���
	int  GetOnlineSize( void ) { return _online_count; }
	// ����TTS��������
	void SendTTSMessage( const char *userid, const char *msg, int len ) ;
	// ���ó�ʱ�ط�����
	virtual bool OnReSend( void *data ) ;
	// ���ó�ʱ���ط�������ɾ������
	virtual void Destroy( void *data ) ;

protected:
	virtual void on_data_arrived( socket_t *sock , const void *data, int len );
	virtual void on_dis_connection( socket_t *sock );
	virtual void on_new_connection( socket_t *sock , const char* ip, int port );
	// ���IP�Ƿ�Ϊ�Ϸ�����Ч��IP����
	virtual bool check_ip( const char *ip ) ;

	// ����һ�����ݰ�
	void handle_one_packet( socket_t *sock ,const char *data,int len);
	// ��Ϣ����
	void processMsgGb808(socket_t *sock ,const char *data, int len, const string &str_car_id);
	// ���͵�ָ�����û�
	bool SendDataToUser( const string &user_id,const char *data,int data_len);
	// ����7E���������
	bool Send7ECodeData( socket_t *sock , const char *data, int len ) ;
	// ������Ӧ���ݣ���Ҫʵ�������TCPͨ������TCP
	bool SendResponse( socket_t *sock, const char *id , const char *data, int len ) ;
	// �����Զ���ָ���ϴ�
	void sendOtherCmd(const string &macid, uint16_t msgid, const char* ptr, int len);

	virtual void TimeWork();
	virtual void NoopWork();
	virtual bool HasLogin(string &user_id);
private:
	// ��������
	ISystemEnv		    *_pEnv ;
	// �����û�����
	unsigned int 		_online_count ;
	// ����_cache_data_pool����ش�������������У��ʽ�_cache_data_pool��������˽�еġ�
	CacheDataPool 		&_cache_data_pool;
	// �����û�
	OnlineUser 			 _online_user;
	// Э���������
	GbProtocolHandler 	*_gb_proto_handler;
	// �߳���
	unsigned int 		 _thread_num ;
	// �ְ�����
	C808Spliter		 	 _pack_spliter ;
	// ��ý�ϴ�
	CScpMedia			 _scp_media ;
	// �������������
	CPackMgr			 _pack_mgr ;
	// ������ͳ��
	CStatFlux			 _recvstat ;
	// ����û����ʱ��
	unsigned int 		 _max_timeout ;
	// ��û����ݴ��ʱ��
	unsigned int 		 _max_pack_live;
	// ͳ���ֻ���
	string 				 _statphone ;
	// �������б�
	CBlackList		     _blacklist ;
	// �����������ļ�·��
	string				 _blackpath ;
	// IP�������б�
	CIpList				 _ipblack ;
	// ����IP���б�
	string 				 _ippath ;
	// �������ݶ��й������
	CQueueMgr			*_queuemgr ;
	// 0�ر�ע���Ȩ��1����ע���Ȩ
	int                  _secureType;
	// Ĭ�ϵ�OEM��4C54
	string               _defaultOem;
};


#endif /* CLIENTACCESSSERVER_H_ */
