/*
 * usermgr.h
 *
 *  Created on: 2012-5-17
 *      Author: humingqing
 *  �û��������
 */

#ifndef __USERMGR_H__
#define __USERMGR_H__

#include <map>
#include <time.h>
#include <string>
#include <Mutex.h>
#include <TQueue.h>
#include <SocketHandle.h>

// �û�������Ϣ
struct _UserInfo
{
	enum State{OFF_LINE,WAITING_RESP,ON_LINE,DISABLED};
	_UserInfo()
	{
		_fd     = NULL ;
		_active = 0 ;
		_login  = 0 ;
		_state  = OFF_LINE ;
	}

	socket_t   *_fd ;
	std::string _key ;
	std::string _code ;
	std::string _ip ;
	int 		_port ;
	time_t  	_active;
	time_t  	_login ;
	State 		_state;
};

// �û��б�
struct _PairUser
{
	_UserInfo tcp , udp ;
	_PairUser *_next, *_pre ;
};

// �û�����֪ͨ
class IPairNotify
{
public:
	virtual ~IPairNotify(){}
	// �ر��û�֪ͨ
	virtual void CloseUser( socket_t *sock ) = 0 ;
	// ֪ͨ�û�����
	virtual void NotifyUser( socket_t *sock, const char *key ) = 0 ;
};

// �û��������
class CUserMgr
{
	typedef std::map<std::string,_PairUser *> CMapUser ;
	typedef std::map<socket_t*,_PairUser*> CMapFds ;
public:
	CUserMgr(IPairNotify *notify) ;
	~CUserMgr() ;
	// ����û�
	bool AddUser( socket_t *sock ) ;
	// ע��
	bool OnRegister( socket_t *sock, const char *ckey, const char *ip, int port , socket_t *usock, std::string &key ) ;
	// ��Ȩ
	bool OnAuth( socket_t *sock, const char *akey, const char *code , std::string &key ) ;
	// ��������
	bool OnLoop( socket_t *sock, const char *akey ) ;
	// ����Ƿ�ʱ
	void Check( int timeout ) ;
	// ��������ȡ���û�
	_PairUser * GetUser( socket_t *sock ) ;
	// ���ݽ�����ȡ���û�
	_PairUser * GetUser( const char *key ) ;

private:
	// ����KEY������
	const std::string GenKey( void ) ;
	// ����µ��û�
	void AddList( _PairUser *p ) ;
	// ɾ���û�����
	void DelList( _PairUser *p , bool notify ) ;
	// �Ƴ�����
	void RemoveIndex( _PairUser *p ) ;
	// ��������
	void Clear( void ) ;
	// ��Ӷ�����
	bool AddMapFd( socket_t *sock, _PairUser *p ) ;
	// ȡ������
	_PairUser * GetMapFd( socket_t *sock ) ;
	// ��ӽ���������
	bool AddMapCode( const char *key, _PairUser *p ) ;

private:
	// �������Ĵ���
	share::Mutex 	  _mutex ;
	// �û�����Զ�
	TQueue<_PairUser> _queue ;
	// �û�����������
	CMapUser   		  _kuser;
	// TCP��FD����
	CMapFds    		  _tcps ;
	// �û�֪ͨ
	IPairNotify *	  _notify;
};

#endif /* USERMGR_H_ */
