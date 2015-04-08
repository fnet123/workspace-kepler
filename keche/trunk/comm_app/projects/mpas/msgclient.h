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
#include <interpacker.h>
#include <set>
#include <string>
#include "pconvert.h"
using namespace std ;

class MsgClient : public BaseClient , public IMsgClient
{
public:
	MsgClient( PConvert *convert ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����
	virtual bool Start( void ) ;
	// ֹͣ����
	virtual void Stop() ;
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len ) ;
	// ��MSG�ϴ���Ϣ
	virtual bool Deliver( const char *data, int len ) ;
	// ����û�����
	virtual void AddUser( const char *ip, unsigned short port, const char *user, const char *pwd ) ;
	// ����ɾ������
	virtual void DelUser( const char *ip, unsigned short port ) ;
	// ����ֻ�MAC
	virtual void AddMac2Car( const char *macid, const char *vechile ) ;

public:
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½��Ϣ����
	virtual int  build_login_msg(User &user, char *buf, int buf_len);

protected:
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	// ���������û�
	void HandleOfflineUsers( void ) ;
	// ���������û�
	void HandleOnlineUsers(int timeval) ;

private:
	// ����ָ��
	ISystemEnv  *	_pEnv ;
	// ���һ�η���ʱ��
	time_t		  	_last_handle_user_time ;
	// �����û�����
	OnlineUser   	_online_user;
	// ��Ϣת������
	IMsgClient     *_pMsgClient ;
	// �ְ�������
	CInterSpliter   _packspliter ;
	// Э��ת������
	PConvert 	   *_convert ;
	// �ֻ����복�ƶ�Ӧ��ϵ
	CSessionMgr     _macid2carnum ;
};

#endif /* LISTCLIENT_H_ */
