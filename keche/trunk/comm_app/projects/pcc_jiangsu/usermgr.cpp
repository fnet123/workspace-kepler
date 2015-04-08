/*
 * usermgr.cpp
 *
 *  Created on: 2012-5-17
 *      Author: humingqing
 *
 *  �����û��Ự
 */
#include "usermgr.h"
#include <comlog.h>

CUserMgr::CUserMgr(IPairNotify *notify)
	:_notify(notify)
{
}

CUserMgr::~CUserMgr()
{
	Clear() ;
}

// ����û�
bool CUserMgr::AddUser( socket_t *fd )
{
	share::Guard guard( _mutex ) ;

	_PairUser *p = new _PairUser;
	p->tcp._login = time(NULL) ;
	p->tcp._fd    = fd ;
	p->_next = p->_pre = NULL ;
	p->tcp._active = time(NULL) ;

	if ( ! AddMapFd( fd, p ) ) {
		return false ;
	}
	AddList( p ) ;
	// ��ӵ�TCP������
	return true ;
}

// ע��
bool CUserMgr::OnRegister( socket_t *fd, const char *ckey, const char *ip, int port , socket_t *usock, std::string &key )
{
	share::Guard guard( _mutex ) ;

	_PairUser *p = GetMapFd( fd ) ;
	if ( p == NULL || ckey == NULL )
		return false ;
	if ( strcmp( p->tcp._key.c_str(), ckey ) != 0 )
		return false ;

	key = GenKey() ;
	p->udp._fd  = usock ;
	p->udp._key = key ;
	p->udp._login = time(NULL) ;
	p->tcp._active = time(NULL) ;

	return true ;
}

// ��Ȩ
bool CUserMgr::OnAuth( socket_t *fd, const char *akey, const char *code , std::string &key )
{
	share::Guard guard( _mutex ) ;

	_PairUser *p = GetMapFd( fd ) ;
	if ( p == NULL || akey == NULL || code == NULL )
		return false ;
	if ( p->tcp._key.empty() )
		return false ;

	if ( strcmp( p->tcp._key.c_str(), akey ) != 0 )
		return false ;
	p->tcp._code = code ;
	p->udp._code = code ;
	key = GenKey() ;
	p->tcp._key  = key ;
	p->tcp._active = time(NULL) ;

	// ��ӵ��û���Ȩ����
	return AddMapCode( code, p ) ;
}

// ��������
bool CUserMgr::OnLoop( socket_t *fd, const char *akey )
{
	share::Guard guard( _mutex ) ;

	_PairUser *p = GetMapFd( fd ) ;
	if ( p == NULL || akey == NULL )
		return false ;

	const char *key = p->tcp._key.c_str() ;
	if ( strcmp( akey , key ) != 0 )
		return false ;

	p->tcp._active = time(NULL) ;

	return true ;
}

// ����Ƿ�ʱ
void CUserMgr::Check( int timeout )
{
	time_t now = time(NULL) ;

	int    count = 0 ;
	string suser ;

	_mutex.lock() ;
	_PairUser *tmp, *p = _queue.begin() ;
	while( p != NULL ) {
		tmp = p ;
		p = _queue.next( p ) ;

		// ����ʱ������
		if ( now - tmp->tcp._active > timeout ){
			// ɾ������
			DelList( tmp , true ) ;
		} else {
			if ( ! suser.empty() ) {
				suser += "," ;
			}
			suser += tmp->tcp._code ;
			++ count ;
		}
	}
	_mutex.unlock() ;

	OUT_WARNING( NULL, 0, "ONLINE", "online user count %d, users: %s" , count,  suser.c_str() ) ;
}

// ��������ȡ���û�
_PairUser * CUserMgr::GetUser( socket_t *fd )
{
	share::Guard guard( _mutex ) ;

	_PairUser *p = GetMapFd( fd ) ;
	if ( p == NULL )
		return NULL;

	return p ;
}

// ���ݽ�����ȡ���û�
_PairUser * CUserMgr::GetUser( const char *key )
{
	share::Guard guard( _mutex ) ;

	CMapUser::iterator it = _kuser.find( key ) ;
	if ( it == _kuser.end() )
		return NULL ;

	return (it->second) ;
}

//===========================================================================
// ��Ӷ�����
bool CUserMgr::AddMapFd( socket_t * fd, _PairUser *p )
{
	CMapFds::iterator it  = _tcps.find( fd ) ;
	if ( it != _tcps.end() ) {
		DelList( it->second , false ) ;
	}
	_tcps.insert( std::make_pair(fd, p ) ) ;
	return true ;
}

// ȡ������
_PairUser * CUserMgr::GetMapFd( socket_t *fd )
{
	CMapFds::iterator it  = _tcps.find( fd ) ;
	if ( it == _tcps.end() )
		return NULL ;

	return it->second ;
}

// ��ӽ���������
bool CUserMgr::AddMapCode( const char *key, _PairUser *p )
{
	CMapUser::iterator it = _kuser.find( key ) ;
	if ( it != _kuser.end() ){
		it->second = p ;
	}
	_kuser.insert( std::make_pair(key,p) ) ;
	return true ;
}

// ����KEY������
const std::string CUserMgr::GenKey( void )
{
	time_t now = time(NULL) ;
	char buf[128] = {0};
	sprintf( buf , "%llu", now ) ;
	return buf ;
}

// ����µ��û�
void CUserMgr::AddList( _PairUser *p )
{
	_queue.push( p ) ;

	p->tcp._key = GenKey() ;

	_notify->NotifyUser( p->tcp._fd , p->tcp._key.c_str() )  ;
}

// ɾ���û�����
void CUserMgr::DelList( _PairUser *p , bool notify )
{
	p = _queue.erase( p ) ;

	// �Ƴ�����
	RemoveIndex( p ) ;

	if ( notify )
		_notify->CloseUser( p->tcp._fd ) ;

	delete p ;
}

// �Ƴ�����
void CUserMgr::RemoveIndex( _PairUser *p )
{
	// ��������벻�վ�ֱ���Ƴ�
	if ( ! p->tcp._code.empty() ) {
		CMapUser::iterator it = _kuser.find( p->tcp._code ) ;
		if ( it != _kuser.end() ){
			_kuser.erase( it ) ;
		}
	}

	CMapFds::iterator itx ;
	// �Ƴ�TCP������
	itx = _tcps.find( p->tcp._fd ) ;
	if ( itx != _tcps.end() ) {
		_tcps.erase( itx ) ;
	}
}

// ������������
void CUserMgr::Clear( void )
{
	share::Guard guard( _mutex ) ;

	_kuser.clear() ;
	_tcps.clear() ;
}




