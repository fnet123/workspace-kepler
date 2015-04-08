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
using namespace std ;

class MsgClient : public BaseClient , public IMsgClient
{
	class CSubscribeMgr
	{
	public:
		CSubscribeMgr() {}
		~CSubscribeMgr() {}
		// ���MACID������
		bool AddSubId( const char *name )
		{
			share::Guard guard( _mutex ) ;
			set<string>::iterator it = _setid.find( name ) ;
			if ( it != _setid.end() ) {
				return false ;
			}
			_setid.insert( set<string>::value_type(name) ) ;

			return true ;
		}

		// ɾ��MACID������
		bool DelSubId( const char *name )
		{
			share::Guard guard( _mutex ) ;

			set<string>::iterator it = _setid.find( name ) ;
			if ( it == _setid.end() ) {
				return false ;
			}
			_setid.erase(it) ;

			return true ;
		}

		// �Ƿ�Ϊ��
		bool IsEmpty( void )
		{
			share::Guard guard( _mutex ) ;
			return _setid.empty() ;
		}

		// ȡ��������MACID
		set<string> & GetSubIds( void )
		{
			share::Guard guard( _mutex ) ;
			return _setid ;
		}

	private:
		// ��Ҫ����ͬ������
		share::Mutex    _mutex ;
		// ���ĵĵ�����
		set<string>  	_setid ;
	};
public:
	MsgClient( void ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// ��MSG�ϴ���Ϣ
	virtual void HandleMsgData( const char *macid, const char *data, int len ) ;
	// ����û�����
	virtual void AddUser( const char *ip, unsigned short port, const char *user, const char *pwd ) ;
	// ����ɾ������
	virtual void DelUser( const char *ip, unsigned short port ) ;
	// ��Ӷ��Ļ���
	virtual void AddDemand( const char *name, int type ) ;
	// ȡ��������
	virtual void DelDemand( const char *name, int type ) ;

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
	// ֪ͨ��������
	void SendOnlineData( const char *data, int len ) ;
	// �׷���½�����Ķ���
	void SendDemandData( socket_t *sock ) ;

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
	// �ְ�������
	CInterSpliter   _packspliter ;
	// �������ݹ���
	CSubscribeMgr   _macidsubmgr ;
	// �鶩�ĵĹ���
	CSubscribeMgr   _groupsubmgr ;
};

#endif /* LISTCLIENT_H_ */
