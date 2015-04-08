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
#include <qstring.h>
#include <packspliter.h>
#include "pconvert.h"
#include <Session.h>
#include "httpcaller.h"

#define MSG_USER_TAG  "MSGCLIENT"

class MsgClient : public BaseClient , public IMsgClient, public ICallResponse
{
public:
	MsgClient( PConvert *convert ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ��MSG�ϴ���Ϣ
	virtual bool HandleUpMsgData( const char *code, const char *data, int len ) ;
	// ͨ��HTTP������ȡͼƬ
	virtual void LoadUrlPic( unsigned int seq, const char *path ) ;

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
	// ����HTTP����Ӧ�ص�����
	virtual void ProcessResp( unsigned int seqid, const char *data, const int len , const int err ) ;

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
	// ���ض��Ĺ�ϵ�б�
	void LoadSubscribe( User &user ) ;

private:
	// ����ָ��
	ISystemEnv    *_pEnv ;
	// ���һ�η���ʱ��
	time_t		   _last_handle_user_time ;
	// �����û�����
	OnlineUser     _online_user;
	// Э��ת������
	PConvert	  *_convert ;
	// �ְ�������ְ�
	CBrPackSpliter _packspliter ;
	// �Ự�������
	CSessionMgr	   _session ;
	// HTTP���÷���
	CHttpCaller	   _httpcaller ;
	// ͼƬ���ػ���ַ
	CQString 	   _picUrl ;
	// ���Ĺ�ϵ�б�
	CQString   	   _dmddir ;
};

#endif /* LISTCLIENT_H_ */
