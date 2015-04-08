/*
 * queuemgr.cpp
 *
 *  Created on: 2012-10-22
 *      Author: humingqing
 *
 *  memo: ��Ҫʵ�ֵĹ���
 *   1. �첽���͵����ݵȴ����У������յ���Ӧ����ն���,���ȴ���Ӧ��ʱʱ����Ҫ���·���
 *   2. ���·���ÿһ�εȴ���ʱʱ�䣬��ε�ʱ��Ϊ��2�ĵȴ���ϵ��Ҳ�����ط�����Խ��ȴ�ʱ��Խ��
 *   3. ����ն˴����������ޣ���֤����Ҫ��������ֱ������еȴ����ͻ�˳����
 *   4. ��˳����ʱ��ƽ̨�յ��ն���һ���·�����Ӧ���Զ�������һ���ȴ���Ҫ���͵���Ϣ
 *   5. ����ȴ��������ݳ�ʱ��ƽ̨���Զ��·���Ϣ������ж����ʱ�·����ݣ���ֻ���͵������ݣ�ֱ���յ���һ��Ӧ������ٴγ�ʱ���·�
 */

#include "queuemgr.h"
#include <assert.h>

CQueueMgr::CQueue::CQueue( IQCaller *pCaller , int ent )
{
	_pCaller = pCaller ;
	_send 	 = 0 ;
	_next    = _pre  = NULL ;
	_maxent  = ent ;
}

CQueueMgr::CQueue::~CQueue()
{
	int size = 0 ;
	_QData *p = _queue.move(size) ;
	if ( size == 0 )
		return ;

	while( p->_next != NULL ) {
		p = p->_next ;
		_pCaller->Destroy( p->_pre->_ptr ) ;
		delete p->_pre ;
	}
	if ( p != NULL ) {
		_pCaller->Destroy( p->_ptr ) ;
		delete p ;
	}

	_index.clear() ;
	_send = 0 ;
}

// �������
bool CQueueMgr::CQueue::Add( unsigned int seq, void *data, int timeout, bool send )
{
	// ���Ƴ�ԭ�е�����
	Remove( seq ) ;

	_QData *p = new _QData ;
	assert( p != NULL ) ;

	p->_seq   = seq ;
	p->_ntime = timeout ;
	p->_time  = time(NULL) + timeout ;
	p->_ptr   = data ;

	if ( send ) {
		p->_ent = -1 ;
	} else {
		p->_ent = _maxent ;
	}

	Add( p ) ;

	_index.insert( std::make_pair(seq, p ) ) ;

	return true ;
}

// ��ӵ��ڵ���
void CQueueMgr::CQueue::Add( _QData *p )
{
	_queue.insert( p ) ;

	if ( p->_ent < 0 )
		_send = _send + 1 ;

	// printf( "add seq id %d, size %d, send %d \n", p->_seq , _size, _send ) ;
}

// ɾ������
void CQueueMgr::CQueue::Remove( unsigned int seq )
{
	_QIndex::iterator it = _index.find( seq ) ;
	if ( it == _index.end() )
		return ;

	_QData *p = it->second ;
	_index.erase( it ) ;

	Remove( p ) ;

	_pCaller->Destroy( p->_ptr ) ;
	delete p ;
}

// �Ƴ����е����ݽڵ�
void CQueueMgr::CQueue::Remove( _QData *p )
{
	// printf( "remove seq id %d\n", p->_seq ) ;
	if ( p->_ent < 0 )
		_send = _send - 1 ;

	_queue.erase( p ) ;
}

// ��������е����ݷ���һ��
void CQueueMgr::CQueue::Send( void )
{
	if ( _send == 0 )
		return ;

	_QData *p = _queue.begin() ;
	while( p != NULL ) {
		if ( p->_ent < 0 )
			break ;
		p = _queue.next(p) ;
	}

	if ( p != NULL ) {
		Remove( p ) ;
		p->_ent  = _maxent ;
		p->_time = time(NULL) + p->_ntime ;
		Add( p ) ;

		_pCaller->OnReSend( p->_ptr ) ;

		// printf( "send seq %d\n", p->_seq ) ;
	}
}

// ��ⳬʱ������
bool CQueueMgr::CQueue::Check( int &nexttime )
{
	if ( _queue.size() == 0 )
		return false;

	time_t now = time(NULL) ;

	_QData *t,*p  = _queue.begin() ;

	int nsend = 0 ;
	while ( p != NULL ) {
		if ( p->_time > now )
			break ;
		t = p ;
		p = _queue.next(p) ;

		// �ȴ�ǰ�����ߵ�����
		Remove( t ) ;

		// ���Ϊ�ȴ����͵����ݣ���ʱ���Ͱ�˳�򵥸�����
		if ( t->_ent < 0 ) {
			if ( nsend == 0 ) {// ���÷��ͽӿڷ�������
				_pCaller->OnReSend( t->_ptr ) ;
				t->_ent = _maxent ;
				nsend = nsend + 1 ;
			}
		} else { // ����Ϊ�ȴ���Ӧ���͵�����
			// ���÷��ͽӿڷ�������
			_pCaller->OnReSend( t->_ptr ) ;

			t->_ntime = t->_ntime * 2 ; // ÿһ�εȴ�ʱ����������ٶ����ӵ�
			t->_ent   = t->_ent - 1 ;
		}
		t->_time  = now + t->_ntime ;

		// ������ʹ�Ϊ��Ͳ��ٴ���
		if ( t->_ent == 0 ) {  // �Ƴ���ʱ��Ԫ�أ����ط�Ҳ���ٵȴ�
			_index.erase( t->_seq ) ;
			_pCaller->Destroy( t->_ptr ) ;
			delete t ;
		} else {
			Add( t ) ; // ��ʱ��������Ӷ�����
		}
	}
	if ( _queue.size() == 0 )
		return false ;

	// ȡ�����������Ҫ�ȴ���ʱ��
	nexttime = _queue.begin()->_time - now ;

	return true ;
}

CQueueMgr::CQueueMgr( IQCaller *pCaller , int time, int ent )
{
	_maxspan = time ;
	_maxent  = ent ;

	_pCaller = pCaller ;
	if ( ! _thread.init( 1, NULL, this ) ) {
		printf( "init queue thread failed\n" ) ;
		return ;
	}
	_inited = true ;
	_thread.start() ;
}

CQueueMgr::~CQueueMgr()
{
	Clear() ;

	if ( ! _inited )
		return ;

	_inited = false ;
	_monitor.notifyEnd() ;
	_thread.stop() ;
}

// ��ӵ��ȴ����Ͷ����У��Ƿ�Ϊ�Ӻ����̶߳���������,��Ҫ��
bool CQueueMgr::Add( const char *id, unsigned int seq, void *data , bool send )
{
	_mutex.lock() ;

	CQueue *p = NULL ;
	CMapQueue::iterator it = _index.find( id ) ;
	if ( it == _index.end() ) {
		p = new CQueue(_pCaller, _maxent ) ;
		p->_id = id ;
		_queue.push( p ) ;
		_index.insert( std::make_pair(id, p ) ) ;
	} else {
		p = it->second ;
	}
	bool success = p->Add( seq, data, _maxspan , send ) ;

	_mutex.unlock() ;

	_monitor.notify() ;

	return success ;
}

// ɾ��ID�Ŷ���
void CQueueMgr::Del( const char *id )
{
	_mutex.lock() ;
	CMapQueue::iterator it = _index.find( id ) ;
	if ( it == _index.end() ) {
		_mutex.unlock() ;
		return ;
	}
	CQueue *p = it->second ;
	_index.erase( it ) ;
	delete _queue.erase( p ) ;
	_mutex.unlock() ;
}

// �Ƴ�����
void CQueueMgr::Remove( const char *id, unsigned int seq , bool check )
{
	_mutex.lock() ;
	CMapQueue::iterator it = _index.find( id ) ;
	if ( it == _index.end() ) {
		_mutex.unlock() ;
		return ;
	}
	// �յ���ӦӦ�����յȴ�����
	it->second->Remove( seq ) ;
	if ( check ) { // ���͵ȴ���Ҫ���͵�����
		it->second->Send() ;
	}
	_mutex.unlock() ;

	_monitor.notify() ;
}

// ʵ���̶߳������м��ӿ�
void CQueueMgr::run( void *param )
{
	while( _inited ) {

		int ntime = 30 ;
		_mutex.lock() ;
		if( _queue.size() > 0 ) {
			int nval = 0 ;
			CQueue *t, *p = _queue.begin() ;
			while( p != NULL ){
				t = p ;
				p = _queue.next( p ) ;

				// ��������Ƿ���Ҫ����
				if ( ! t->Check( nval ) ) {
					// �Ƴ�����
					_queue.erase( t ) ;
					// ɾ������Ԫ��
					_index.erase( t->_id ) ;
					delete t ;
					continue ;
				}

				// ���������Ԫ�������Ҫ������Ϣʱ��
				if ( nval < ntime ) {
					ntime = nval ;
				}
			}
		}
		_mutex.unlock() ;

		// printf( "run time over %d, size %d\n", ntime , _size ) ;

		ntime = ( ntime <= 0 ) ? 1 : ntime ;
		_monitor.lock() ;
		_monitor.wait(ntime) ;
		_monitor.unlock() ;
	}
}

// �����������
void CQueueMgr::Clear( void )
{
	_mutex.lock() ;

	int size = 0 ;
	CQueue *p = _queue.move( size ) ;
	if ( size == 0 ) {
		_mutex.unlock() ;
		return ;
	}

	while( p != NULL ) {
		p = p->_next ;
		delete p->_pre ;
	}
	_index.clear() ;
	_mutex.unlock() ;
}

