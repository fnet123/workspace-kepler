/**********************************************
 * pasclient.h
 *
 *  Created on: 2011-07-28
 *      Author: humingqing
 *    Comments: ����ƽ̨�ԽӴ���
 *********************************************/

#ifndef __PASCLIENT_H__
#define __PASCLIENT_H__

#include "interface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseClient.h>
#include <protocol.h>
#include "busloader.h"
#include "mypacker.h"
#include "statinfo.h"

#define PCC_USER_LOGIN    0x01
#define PCC_USER_ACTVIE   0x02
#define PCC_USER_LOOP 	  0x04

class PasClient : public BaseClient , public IPasClient
{
	class PccUser
	{
		enum USERSTATE { OFF_LINE= 0, ON_LINE=1 , WAIT_RESP=2 } ;
	public:
		PccUser(){
			_fd = NULL ;
			_login_time = _active_time = _loop_time = 0 ;
			_user_state = OFF_LINE ;
			_tcp        = false ;
		}

		// �������ʹ��ʱ��
		void Update( int flag ) {
			if ( flag & PCC_USER_LOGIN )
				_login_time = time(NULL);
			if ( flag & PCC_USER_ACTVIE )
				_active_time = time(NULL) ;
			if ( flag & PCC_USER_LOOP )
				_loop_time = time(NULL) ;
		}

		// ����Ƿ�ʱ
		bool Check( int timeout , int flag ){
			time_t now = time(NULL) ;
			if ( flag & PCC_USER_LOGIN ) {
				if ( now - _login_time > timeout )
					return true;
			}
			if ( flag & PCC_USER_ACTVIE ) {
				if ( now - _active_time > timeout )
					return true ;
			}
			if ( flag & PCC_USER_LOOP ) {
				if ( now - _loop_time > timeout )
					return true ;
			}
			return false ;
		}

		// �û�״̬����
		bool IsOnline( void ) {  return ( _user_state == ON_LINE && _fd != NULL ) ; }
		bool IsOffline( void ) { return ( _user_state == OFF_LINE ); }
		void SetOnline( void ) { _user_state = ON_LINE ; }
		void SetOffline( void ) { _user_state = OFF_LINE; }
		void SetWaitResp( void ){ _user_state = WAIT_RESP; }

	public:
		socket_t *_fd ;  			// �û�����FD
		string   _srv_key ;      // �ɹ��󷵻ص�KEYֵ
		string   _srv_ip ; 		// ��½����IP��ַ
		short 	 _srv_port ; 	// ��½½�������˿�
		string   _username;		// ��½�û���
		string   _password;		// ��½������
		bool 	 _tcp ;			// �Ƿ�ΪTCP����

	private:
		time_t  	_login_time ;   // ���һ�ε�½��ʱ��
		time_t  	_active_time ;  // ���һ��ʹ�õ�ʱ��
		time_t		_loop_time;     // ������ʱ��
		USERSTATE	_user_state ;   // �û�״̬�Ƿ�����
	};

	// �û��Ự����
	class UserSession
	{
	public:
		UserSession(){
			_tcp._tcp = true  ;
			_udp._tcp = false ;
		}
		~UserSession(){}

		// �Ƿ�����
		bool IsOnline( bool tcp ) {
			share::Guard guard( _mutex ) ;
			return (tcp) ? _tcp.IsOnline(): _udp.IsOnline();
		}

		// �Ƿ�����
		bool IsOffline( bool tcp ) {
			share::Guard guard( _mutex ) ;
			return (tcp) ? _tcp.IsOffline() : _udp.IsOffline() ;
		}

		// �Ƿ�����
		void SetState( bool online, bool tcp ) {
			share::Guard guard( _mutex ) ;
			if( tcp ){
				( online ) ? _tcp.SetOnline() : _tcp.SetOffline() ;
			} else {
				( online ) ? _udp.SetOnline() : _udp.SetOffline() ;
			}
		}

		void Update( int flag , bool tcp ) {
			share::Guard guard( _mutex ) ;
			if ( tcp )
				_tcp.Update( flag ) ;
			else
				_udp.Update( flag ) ;
		}

		bool Check( int timeout, int flag, bool tcp ){
			share::Guard gurad( _mutex ) ;
			if ( tcp )
				return _tcp.Check( timeout, flag ) ;

			return _udp.Check( timeout, flag ) ;
		}

		void SetUser( PccUser &user, bool tcp ) {
			share::Guard guard( _mutex ) ;
			if ( tcp ){
				_tcp 	  = user ;
				_tcp._tcp = true ;
			}else{
				_udp 	  = user ;
				_udp._tcp = false ;
			}
		}

		PccUser & GetUser( bool tcp ) {
			share::Guard guard( _mutex ) ;
			if ( tcp )
				return _tcp ;
			return _udp ;
		}

		bool IsKey( bool tcp ) {
			share::Guard guard( _mutex ) ;
			if ( tcp )
				return (!_tcp._srv_key.empty()) ;

			return (!_udp._srv_key.empty()) ;
		}

		const char * GetKey( bool tcp ) {
			share::Guard guard( _mutex ) ;
			if ( tcp )
				return _tcp._srv_key.c_str() ;
			return _udp._srv_key.c_str() ;
		}

		void SetSrvId( const char *srvid ){
			share::Guard guard( _mutex ) ;
			_serverid = srvid ;
		}

		const char * GetSrvId( void ) {
			share::Guard guard( _mutex ) ;
			return _serverid.c_str() ;
		}

		void DisConnect( socket_t *sock ) {
			share::Guard guard( _mutex ) ;

			if ( _tcp._fd == sock ) {
				_tcp._fd = NULL ;
				_tcp.SetOffline() ;
			} else if ( _udp._fd == sock ){
				_udp._fd = NULL ;
				_udp.SetOffline() ;
			}
		}

		// ����FD��ֵ��ȡ����TCP����UDP
		bool GetUser( socket_t *fd , PccUser &user ) {
			share::Guard guard( _mutex ) ;

			if ( _tcp._fd == fd ) {
				user = _tcp ;
				return true ;
			}

			if ( _udp._fd == fd ) {
				user = _udp ;
				return true ;
			}
			return false ;
		}

	private:
		// ��ǰTCP���û�
		PccUser 		_tcp ;
		// ��ǰUDP���û�
		PccUser			_udp ;
		// �û�״̬������
		share::Mutex    _mutex ;
		// ��ͨ��������ID
		string 			_serverid ;
	};

public:
	PasClient( CStatInfo *stat ) ;
	virtual ~PasClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ
	virtual bool Start( void ) ;
	// ֹͣ
	virtual void Stop( void ) ;
	// ��PAS������
	virtual bool HandleData( const char *data, int len ) ;

	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ȡ�÷�������IP
	virtual const char * GetSrvId( void ) {  return _user.GetSrvId(); }
	// ��ʼ��UDP�ķ���
	virtual bool StartUDP( const char * ip, int port, int thread ) ;
	// �Ƿ�����
	virtual bool IsOnline( void ) { return (_user.IsOnline(false) && _user.IsOnline(true)) ; }

private:
	// �׷����յ�������
	void HandleRecvData( socket_t *sock, const char *data, int len ) ;
	// �׷���������
	void HandleCtrlData( socket_t *sock, const char *data, int len ) ;
	// ����û�״̬
	void CheckUserState( UserSession &user ) ;
	// ����û�����
	void CheckUserLoop( UserSession &user ) ;
	// �����������ӻ�����������
	void ConnectServer( UserSession &user, bool tcp ) ;

private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// ��ǰ�û�����
	UserSession		    _user ;
	// ���ݷְ�����
	CMyPackSpliter      _packspliter;
	// UDP�������ò���
	unsigned short      _port ;
	// ������IP��ַ
	string 				_ip ;
	// ������̬��Ϣ���ض���
	BusLoader  			_busloader ;
	// ����������IP
	string 			    _srvip ;
	// ����ͳ��
	CStatInfo		   *_statinfo ;
};

#endif /* LISTCLIENT_H_ */
