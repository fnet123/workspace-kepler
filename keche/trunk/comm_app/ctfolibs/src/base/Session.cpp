#include "Session.h"

CSessionMgr::CSessionMgr( bool timeout ) : _btimeout(timeout)
{
	_last_check   = time(NULL) ;
	_notfiy       = NULL ;
}

CSessionMgr::~CSessionMgr()
{
	RecycleAll() ;
}

// ��ӻỰ
void CSessionMgr::AddSession( const string &key, const string &val )
{
	share::Guard guard( _mutex ) ;
	{
		_Session *p = NULL ;
		CMapSession::iterator it = _mapSession.find( key ) ;
		if ( it != _mapSession.end() ) {
			p = it->second ;
			p->_time  = time(NULL) ;
			p->_key   = key ;
			p->_value = val ;

			// �����ж���ͷ������
			if ( _btimeout ) {
				RemoveValue( p, false ) ;
			}
		} else {
			p = new _Session ;
			p->_time  = time(NULL) ;
			p->_key   = key ;
			p->_value = val ;
			// ��ӵ�MAP�д���
			_mapSession.insert( pair<string,_Session*>( key, p ) ) ;
			// ֪ͨΪ�������
			if( _notfiy ) {
				// ֪ͨ�ⲿ���������
				_notfiy->NotifyChange( key.c_str(), val.c_str(), SESSION_ADDED ) ;
			}
		}
		// �����Ҫ����ʱ����ӵ��Ự�Զ���
		if ( _btimeout ) AddValue( p ) ;
	}
}

// ȡ�ûỰ
bool CSessionMgr::GetSession( const string &key, string &val , bool update )
{
	share::Guard guard( _mutex ) ;
	{
		CMapSession::iterator it = _mapSession.find( key ) ;
		if ( it == _mapSession.end() ) {
			return false ;
		}
		_Session *p = it->second ;
		val = p->_value ;

		// �������һ��ʹ��ʱ��
		if ( _btimeout && update ) {
			p = _queue.erase( p ) ;
			p->_time = time(NULL) ;
			_queue.push( p ) ;
		}

		return true ;
	}
}

// �Ƴ��Ự
void CSessionMgr::RemoveSession( const string &key )
{
	share::Guard guard( _mutex ) ;
	{
		CMapSession::iterator it = _mapSession.find(key) ;
		if ( it == _mapSession.end() ) {
			return ;
		}
		_Session *p = it->second ;
		_mapSession.erase( it ) ;
		// ֪ͨΪ�������
		if( _notfiy ) {
			// ֪ͨ�ⲿ���������
			_notfiy->NotifyChange( key.c_str(), p->_value.c_str() , SESSION_REMOVE ) ;
		}
		// �Ƿ��г�ʱ����
		if ( _btimeout ) {
			// ����г�ʱֱ������
			RemoveValue( p , true ) ;
		} else {
			delete p ;
		}
	}
}

// ��ⳬʱ����
void CSessionMgr::CheckTimeOut( int timeout )
{
	share::Guard guard( _mutex ) ;
	{
		if ( ! _btimeout ) {
			return  ;
		}

		time_t now = time(NULL) ;
		if ( now - _last_check < SESSION_CHECK_SPAN ) {
			return ;
		}
		_last_check = now ;

		// ���㳬ʱ
		time_t t = now - timeout ;

		_Session *tmp,*p = _queue.begin() ;
		// ֻ��Ҫ���ͷ�������ݾͿ�����
		while( p != NULL ) {
			// �����ʱʱ����ڵ�ǰ��Сʱ�������
			if ( p->_time > t )
				break ;

			tmp = p ;
			// ֱ��ָ����һ��Ԫ��
			p   = _queue.next( p ) ;
			// �Ƴ�MAP����
			RemoveIndex( tmp->_key ) ;
			// �Ƴ������нڵ�
			RemoveValue( tmp , true ) ;
		}
	}
}

// ȡ�õ�ǰ�Ự��
int CSessionMgr::GetSize( void )
{
	share::Guard guard( _mutex ) ;
	return _queue.size() ;
}

// ��ӽڵ�����
void CSessionMgr::AddValue( _Session *p )
{
	_queue.push( p ) ;
}

void CSessionMgr::RemoveIndex( const string &key )
{
	CMapSession::iterator it = _mapSession.find( key ) ;
	if ( it == _mapSession.end() ) {
		return ;
	}

	_Session *p = it->second ;
	_mapSession.erase( it ) ;

	// ֪ͨΪ�������
	if( _notfiy ) {
		// ֪ͨ�ⲿ���������
		_notfiy->NotifyChange( p->_key.c_str() , p->_value.c_str() , SESSION_REMOVE ) ;
	}
}

// �Ƴ�����
void CSessionMgr::RemoveValue( _Session *p , bool clean )
{
	p = _queue.erase( p ) ;
	if ( clean ) delete p ;
}

// �����ڴ�
void CSessionMgr::RecycleAll( void )
{
	share::Guard guard( _mutex ) ;
	{
		if ( ! _btimeout ) {
			CMapSession::iterator it ;
			for ( it = _mapSession.begin(); it != _mapSession.end(); ++ it ) {
				delete it->second ;
			}
		} else {
			int size = 0 ;
			_Session *p = _queue.move( size ) ;
			if ( size > 0 ) {
				_Session *tmp ;
				while ( p != NULL ) {
					tmp = p ;
					p   = p->_next ;
					delete tmp ;
				}
			}
		}
		_mapSession.clear() ;
	}
}
