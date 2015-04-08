/**********************************************
 * msgclient.h
 *
 *  Created on: 2011-07-28
 *    Author:   humingqing
 *    Comments: ʵ������Ϣ��������ͨ���Լ�����ת��
 *********************************************/

#ifndef __MsgCLIENT_H__
#define __MsgCLIENT_H__

#include "interface.h"
#include <BaseClient.h>
#include <OnlineUser.h>
#include <time.h>
#include "pccsession.h"
#include <packspliter.h>
#include "statinfo.h"

#define MSG_SUB_TYPE  "DMDATA"
#define MSG_USER_TAG  "MSGCLIENT"

class PConvert ;
class MsgClient : public BaseClient , public IMsgClient
{
public:
	MsgClient( CStatInfo *stat ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ��MSG�ϴ���Ϣ
	virtual void HandleUpMsgData( const char *code, const char *data, int len ) ;
	// ���ݳ���ȡ�ö�Ӧ���ֻ�MAC
	virtual bool GetCarMacId( const char *key, char *macid ) { return _session.GetCarMacId( key, macid); };

public:
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½����
	virtual int build_login_msg( User &user, char *buf,int buf_len ) ;

protected:
	// ���͵�ָ����������û�
	bool SendDataToUser( const string &area_code, const char *data, int len ) ;
	// ����MSG���û��ļ�
	bool LoadMsgUser( const char *userfile ) ;
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	// ���������û�
	void HandleOfflineUsers( void ) ;
	// ���������û�
	void HandleOnlineUsers(int timeval) ;
	// ���ض�������
	void LoadSubscribe( socket_t *sock , bool notify ) ;

private:
	// ����ָ��
	ISystemEnv    *_pEnv ;
	// ���һ�η���ʱ��
	time_t		   _last_handle_user_time ;
	// �����û�����
	OnlineUser     _online_user;
	// Э��ת������
	PConvert	  *_convert ;
	// �Ự����,������Ҫ��ų��ƺͳ����ŵĶ�Ӧ��ϵ
	CPccSession    _session ;
	// ӳ���ļ�����
	string 		   _carmapfile ;
	// �ְ�������ְ�
	CBrPackSpliter _packspliter ;
	// ���ĵ�MAC������
	string 		   _submacids ;
	// ͳ����Ϣͳ������
	CStatInfo     *_statinfo ;
};

#endif /* LISTCLIENT_H_ */
