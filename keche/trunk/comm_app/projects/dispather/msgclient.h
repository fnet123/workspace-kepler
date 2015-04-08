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
#include <Session.h>
#include <packspliter.h>
#include <qstring.h>

#define MSG_SAVE_CLIENT   "SAVECLIENT"
#define MSG_PIPE_CLIENT   "PIPECLIENT"

class MsgClient : public BaseClient , public IMsgClient
{
public:
	MsgClient( const char *strtype ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ��MSG�ϴ���Ϣ
	virtual void HandleMsgData( const char *macid, const char *data, int len ) ;
	// ����MSG�Ļص�����
	virtual void SetMsgClient( IMsgClient *pClient ){ _pMsgClient = pClient; }
	// ������½��Ϣ����
	virtual int  build_login_msg(User &user, char *buf, int buf_len);

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
	ISystemEnv  *	_pEnv ;
	// ���һ�η���ʱ��
	time_t		  	_last_handle_user_time ;
	// �����û�����
	OnlineUser   	_online_user;
	// ��Ϣת������
	IMsgClient     *_pMsgClient ;
	// �Ự�������
	CSessionMgr  	_session ;
	// �û�����
	string      	_strclient ;
	// �ְ�������
	CBrPackSpliter  _packspliter ;
	// �Ƿ�Ϊ����·��
	bool 			_dataroute ;
	// ���Ĺ�ϵ�б�
	CQString   	    _dmddir ;
};

#endif /* LISTCLIENT_H_ */
