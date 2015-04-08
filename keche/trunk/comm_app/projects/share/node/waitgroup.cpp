/*
 * waitgroup.cpp
 *
 *  Created on: 2011-11-8
 *      Author: humingqing
 */
#include "waitgroup.h"
#include <comlog.h>

CWaitGroup::CWaitGroup(IAllocMsg *pAlloc) :
	_pAlloc( pAlloc ),_inited(false)
{
	_notify = NULL ;
}

CWaitGroup::~CWaitGroup()
{
	Stop() ;
	ClearAll() ;
}

// ��ʼ��
bool CWaitGroup::Init( void )
{
	// ��ʼ���̶߳���
	if ( ! _thread.init( 1, this, this ) ) {
		OUT_ERROR( NULL, 0, "WaitGroup" , "start thread failed" ) ;
		return false ;
	}
	_inited = true ;
	return true ;
}

// ��ʼ�߳�
bool CWaitGroup::Start( void )
{
	_thread.start() ;
	_inited = true ;
	return true ;
}

// ֹͣ
void CWaitGroup::Stop( void )
{
	if ( ! _inited )
		return  ;

	_inited = false ;
	_thread.stop() ;
}

// ����������
bool CWaitGroup::AddGroup( socket_t *sock, unsigned int seq, MsgData *msg )
{
	share::Guard guard( _mutex ) ;
	{
		CMapQueue::iterator it = _index.find( seq ) ;
		if ( it != _index.end() )
			return false ;

		Queue *p = new Queue ;
		p->seq   = seq ;
		p->now   = time(NULL) ;
		p->msg   = msg ;
		p->sock  = sock ;

		_queue.push( p ) ;
		_index.insert( make_pair( seq, p ) ) ;

		return true ;
	}
}

// ��ӵ���Ӧ��������
bool CWaitGroup::AddQueue( unsigned int seq, socket_t *sock )
{
	share::Guard guard( _mutex ) ;
	{
		CMapQueue::iterator it = _index.find( seq ) ;
		if ( it == _index.end() )
			return false ;

		Queue *p = it->second ;
		p->fdlst.push_back( sock ) ;

		return true ;
	}
}

// ɾ����Ӧ��ֵ
bool CWaitGroup::DelQueue( unsigned int seq, socket_t *sock , bool bclear )
{
	bool bfind = false ;
	Queue *p = NULL ;
	{
		share::Guard guard( _mutex ) ;
		{
			CMapQueue::iterator it = _index.find( seq ) ;
			if ( it == _index.end() )
				return false ;

			p = it->second ;
			if ( ! p->fdlst.empty() ) {
				ListFd::iterator itx  ;
				for ( itx = p->fdlst.begin(); itx != p->fdlst.end(); ++ itx ) {
					if ( *itx != sock )
						continue ;
					p->fdlst.erase( itx ) ;
					bfind = true ;
					break ;
				}
			}
			// �Ƿ�Ϊ��
			if ( p->fdlst.empty() && bclear ) {
				// �Ƴ���������
				_index.erase( it ) ;
				// ���ȫ������ɹ���ֱ���Ƴ�
				p = _queue.erase( p ) ;
			}
		}
	}

	// �ŵ�����ص���ֹ��������
	if ( p->fdlst.empty() && bclear ) {
		// ToDo: NotifyMsg
		if ( _notify ) {
			// �ص��ɹ�����
			_notify->NotifyMsgData( p->sock , p->msg , p->fdlst , MSG_COMPLETE ) ;
		}
		_pAlloc->FreeMsg( p->msg ) ;

		delete p ;
	}

	return bfind ;
}

// ȡ��û����Ӧ��FD
int  CWaitGroup::GetCount( unsigned int seq )
{
	share::Guard guard( _mutex ) ;
	{
		CMapQueue::iterator it = _index.find( seq ) ;
		if ( it == _index.end() )
			return 0 ;

		Queue *p = it->second ;
		// ���ض�Ӧ������
		return p->fdlst.size() ;
	}
}

// ȡ�ö�Ӧ��FD��LISTֵ
bool CWaitGroup::GetList( unsigned int seq, ListFd &fds )
{
	share::Guard guard( _mutex ) ;
	{
		CMapQueue::iterator it = _index.find( seq ) ;
		if ( it == _index.end() )
			return false ;

		Queue *p = it->second ;
		if ( p->fdlst.empty() )
			return false ;

		fds = p->fdlst ;
		// ���ض�Ӧ������
		return true ;
	}
}

void CWaitGroup::run( void *param )
{
	while( _inited ) {
		// ��ⳬʱ��������
		CheckTime() ;
		sleep(5) ;
	}
}

// ��ⳬʱ
void CWaitGroup::CheckTime( void )
{
	if ( _queue.size() == 0 ) {
		return  ;
	}

	Queue *t,*p = NULL ;
	std::list<Queue*> lst ;
	time_t last = time(NULL) - MAX_WAITGROUP_TIMEOUT ;
	{
		// ����������ʱ����
		share::Guard guard( _mutex ) ;
		{
			// ����������������
			p = _queue.begin() ;
			while( p != NULL ) {
				t = p ;
				p = p->_next ;
				if ( t->now > last )
					break ;

				RemoveMsg( t->seq ) ;

				t = _queue.erase( t ) ;
				if ( t != NULL ) {
					lst.push_back( t ) ;
				}
			}
		}
	}

	// �ŵ��������ص������ֹ����
	if ( ! lst.empty() ) {
		std::list<Queue*>::iterator itx ;
		for ( itx = lst.begin(); itx != lst.end(); ++ itx ) {
			p = ( *itx ) ;
			if ( _notify ) {
				// �ص���ʱ����
				_notify->NotifyMsgData( p->sock , p->msg , p->fdlst , MSG_TIMEOUT ) ;
			}
			_pAlloc->FreeMsg( p->msg ) ;

			delete p ;
		}
		lst.clear() ;
	}
}

CWaitGroup::Queue* CWaitGroup::RemoveMsg( unsigned int seq )
{
	CMapQueue::iterator it = _index.find( seq ) ;
	if ( it == _index.end() )
		return NULL;

	Queue *p = it->second ;
	_index.erase( it ) ;

	return p ;
}

// �����������
void CWaitGroup::ClearAll( void )
{
	share::Guard guard( _mutex ) ;
	{
		int size = 0 ;
		Queue *p = _queue.move(size) ;
		if ( size == 0 )
			return ;

		while( p != NULL ) {
			p = p->_next ;
			// ���������ڴ�
			_pAlloc->FreeMsg( p->_pre->msg ) ;

			delete p->_pre ;
		}
		_index.clear() ;
	}
}
