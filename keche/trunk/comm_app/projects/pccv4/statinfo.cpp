/*
 * statinfo.cpp
 *
 *  Created on: 2012-7-27
 *      Author: humingqing
 *  Memo: ͳ�Ƶ�ǰ�û����г����������Լ���ǰ�����û������Լ����͵���������
 */

#include "statinfo.h"
#include <tools.h>
#include <comlog.h>

// ���˫�����Ľڵ��
#define ADD_NODE( head, tail, p )   { p->_pre  = p->_next = NULL ; \
	if ( head == NULL ) {  head = tail = p ; } else { tail->_next = p ; p->_pre = tail  ; tail   = p ; } }

// ɾ��˫�����Ľڵ��
#define DEL_NODE( head, tail, p )  { \
	if ( p == head ) { if ( head == tail ) { head = tail = NULL ; } else { head = p->_next ; head->_pre = NULL ; } \
	} else if ( p == tail ) { tail = p->_pre ; tail->_next = NULL ; } else { p->_pre->_next = p->_next ; p->_next->_pre = p->_pre  ;  } }


CStatInfo::CStatInfo( const char *name ) : _name(name)
{
	_head = _tail = NULL ;
	_size = 0 ;
	_lasttime = time(NULL) ;
	_ncar = 0 ;
}

CStatInfo::~CStatInfo()
{
	Clear() ;
}

// ��ӿͻ���
CStatInfo::_ClientInfo *CStatInfo::AddClient( unsigned int id )
{
	_ClientInfo *p = NULL ;

	time_t now = time(NULL) ;

	CMapClient::iterator it = _mpClient.find( id ) ;
	if ( it == _mpClient.end() ) {
		p = new _ClientInfo ;
		p->_id     = id ;
		p->_errcnt = 0 ;
		p->_time   = now ;
		p->_tail   = p->_head = NULL ;
		p->_size   = 0 ;
		p->_send   = 0 ;
		p->_recv   = 0 ;

		ADD_NODE( _head, _tail, p ) ;

		++ _size ;

		_mpClient.insert( CMapClient::value_type(id, p ) ) ;

	} else {
		p = it->second ;
		p->_time = now ;
	}
	return p ;
}

// �Ƿ����
void CStatInfo::AddVechile( unsigned int id, const char *macid, int flag )
{
	if ( macid == NULL )
		return ;

	share::Guard g( _mutex ) ;

	_ClientInfo *p = AddClient( id ) ;
	if ( p == NULL )
		return ;

	_CarInfo *info = NULL ;

	CMapCar::iterator it = p->_mpcar.find( macid ) ;
	if ( it == p->_mpcar.end() ) {
		info = new _CarInfo ;
		info->_time     = time(NULL) ;
		info->_macid    = macid ;
		info->_errcnt   = 0 ;
		if ( flag & STAT_ERROR ) {
			++ info->_errcnt ;
			++ p->_errcnt ;
		}
		ADD_NODE( p->_head, p->_tail, info ) ;
		p->_mpcar.insert( CMapCar::value_type( macid, info ) ) ;
		++ p->_size ;

	} else {
		info = it->second ;
		info->_time = time(NULL) ;
		if ( flag & STAT_ERROR ) {
			info->_errcnt = info->_errcnt + 1 ;
		}
	}
	// һ�δ���ͳ��ҵ��
	if ( flag & STAT_RECV ) ++ p->_recv ;
	if ( flag & STAT_SEND ) ++ p->_send ;
}

// ���յ��ĸ���
void CStatInfo::AddRecv( unsigned int id )
{
	share::Guard g( _mutex ) ;

	_ClientInfo *p = AddClient( id ) ;
	if ( p == NULL )
		return ;

	++ p->_recv ;
}

// ���ͳ�ȥ�ĸ���
void CStatInfo::AddSend( unsigned int id )
{
	share::Guard g( _mutex ) ;

	_ClientInfo *p = AddClient( id ) ;
	if ( p == NULL )
		return ;

	++ p->_send ;
}

// ����Ƿ�ʱ
void CStatInfo::Check( void )
{
	share::Guard g( _mutex ) ;

	time_t ntime = _lasttime ;
	time_t now = time(NULL) ;
	if ( now - _lasttime < 30 ) {
		return ;
	}
	_lasttime = now ;

	if ( _head == NULL )
		return ;

	std::string s ;
	char szbuf[512] = {0} ;

	int count = 0 ;

	_ClientInfo *tmp = NULL ;
	_ClientInfo *p   = _head ;
	while( p != NULL ) {
		tmp = p ;
		p = p->_next ;

		// ���6������û�����ݾ�ֱ�ӳ�ʱ
		if ( now - tmp->_time > 180 ) {
			CMapClient::iterator it = _mpClient.find( tmp->_id ) ;
			if ( it != _mpClient.end() ) {
				DEL_NODE( _head, _tail, tmp ) ;
				DelClient( tmp ) ;
				-- _size ;
				_mpClient.erase( it ) ;
			}
		} else {
			// ���ͻ��˵�����
			count += CheckClient( tmp , now ) ;
		}

		sprintf( szbuf, "%d(%d/%d/%d)",
				tmp->_id, tmp->_size, tmp->_send, tmp->_recv ) ;
		tmp->_recv  = 0 ;
		tmp->_send  = 0 ;

		if ( ! s.empty() ) {
			s += "," ;
		}
		s += szbuf ;
	}
	// ��¼��ǰ���ߵ��ܵĳ�����
	_ncar = count ;
	// ��ӡͳ����־
	OUT_RUNNING( NULL, 0, "ONLINE", "%s client: %d, total car: %d, time span: %d, %s",
			_name.c_str(), _size, _ncar, now-ntime, s.c_str() ) ;
}

// ���ͻ�������
int CStatInfo::CheckClient( _ClientInfo *p , time_t now )
{
	if ( p->_head == NULL )
		return 0;

	_CarInfo *tmp = NULL ;
	_CarInfo *info = p->_head ;
	while( info != NULL ) {
		tmp = info ;
		info = info->_next ;
		if ( now - tmp->_time > 360 ) {
			CMapCar::iterator it = p->_mpcar.find( tmp->_macid ) ;
			if ( it != p->_mpcar.end() )
				p->_mpcar.erase( it ) ;

			DEL_NODE( p->_head, p->_tail, tmp ) ;
			if ( tmp->_errcnt > 0 ) {
				-- p->_errcnt ;
			}
			delete tmp ;
			-- p->_size ;
		}
	}
	return p->_size ;
}

// �ͷſͻ�������
void CStatInfo::DelClient( _ClientInfo *p )
{
	if ( p->_size > 0 ) {
		CMapCar::iterator itx ;
		for ( itx = p->_mpcar.begin(); itx != p->_mpcar.end(); ++ itx ) {
			delete itx->second ;
		}
		p->_mpcar.clear() ;
		p->_size = 0 ;
		p->_head = p->_tail = NULL ;
	}
	delete p ;
}

// �����������
void CStatInfo::Clear( void )
{
	share::Guard g( _mutex ) ;

	if ( _head == NULL )
		return ;

	CMapClient::iterator it ;
	for ( it = _mpClient.begin(); it != _mpClient.end(); ++ it ) {
		DelClient( it->second ) ;
	}
	_mpClient.clear() ;
	_head = _tail = NULL ;
	_size = 0 ;
}



