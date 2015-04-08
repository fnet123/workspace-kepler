#include "workthread.h"

CWorkThread::CWorkThread() 
	: _inited(false)
{

}

CWorkThread::~CWorkThread() 
{
	Stop() ;
	Clear() ;
}

// ��ʼ���߳�
bool CWorkThread::Init( int nthread ) 
{
	if ( ! _threadmgr.init( nthread, NULL, this ) ) {
		return false ;
	}
	_inited = true ;
	_threadmgr.start() ;
	return true ;
}

// ֹͣ�߳�
void CWorkThread::Stop( void ) {
	if ( ! _inited )
		return ;
	_inited = false ;
	_monitor.notifyEnd() ;
	_threadmgr.stop() ;
}

// ע���߳�ִ�ж���,��ʱ��Ϊ����Ĭ��ִ��һ��
int  CWorkThread::Register( share::Runnable *pProc, void *ptr , int t )
{
	if ( pProc == NULL )
		return THREAD_REG_ERROR ;

	int id = _sequeue.get_next_seq() ;

	_ThreadUnit *unit = new _ThreadUnit ;
	unit->_id    = id ;
	unit->_pProc = pProc ;
	unit->_span  = t ;
	unit->_ptr   = ptr ;
	unit->_time  = time(NULL) + t ;

	_mutex.lock() ;
	_queue.insert( unit ) ;
	_index[id] = unit ;
	_mutex.unlock() ;

	_monitor.notify() ;

	return id ;
}

// ����ִ�ж���
void CWorkThread::UnRegister( int id ) 
{
	_mutex.lock() ;
	CMapUnit::iterator it = _index.find( id ) ;
	if ( it == _index.end() ) {
		_mutex.unlock() ;
		return ;
	}
	_ThreadUnit *p = it->second ;
	_index.erase( it ) ;
	_queue.erase( p ) ;
	_mutex.unlock() ;

	delete p ;
}

// ����Ƿ�����Ҫ���ж���
int CWorkThread::Check( void ) 
{
	_mutex.lock() ;

	if ( _queue.size() == 0 ) {
		_mutex.unlock() ;
		return 0 ;
	}

	time_t now = time(NULL) ;

	_ThreadUnit *t = NULL ;
	_ThreadUnit *p = _queue.begin() ;
	// ��������ִ����
	while( p != NULL ) {
		t = p ;
		p = _queue.next( p ) ;

		if ( now < t->_time )
			break ;
		
		t->_time = now + t->_span ;
		// �����һ��û��ִ����ɣ�ֱ��ת����һ��ִ��
		if ( ! t->_run ) {
			t->_run  = true ;

			bool berase = false ;
			if ( t->_span == 0 ) {
				// ����Ϊִ��һ�ζ���
				_index.erase( t->_id ) ;
				_queue.erase( t ) ;
				berase = true ;
			}
			_mutex.unlock() ;

			t->_pProc->run( t->_ptr ) ;
			if ( berase ) {
				delete t ;
				// ���ִ��һ�ξʹ���һ��
				_mutex.lock() ;
				// ���ִ��һ�ε������ֱ�ӷ��ش�ͷ��ʼ
				p = _queue.begin() ;

				continue; 
			}

			_mutex.lock() ;
			t->_run  = false ;
		} 

		t =_queue.erase( t ) ;
		// ���Ϊ��ʱ������Ҫ�ظ���ʱִ��
		_queue.insert( t ) ;
	}
	// ���ִ�ж���Ϊ��
	if ( _queue.size() == 0 ) {
		_mutex.unlock() ;
		return 0 ;
	}

	// ������һ��ִ����ĵ�ʱ��
	int span = (int)( _queue.begin()->_time - time(NULL) ) ;
	
	_mutex.unlock() ;

	return ( span <= 0 ) ? -1 : span ;
}

// �߳����нӿڶ���
void CWorkThread::run( void* param )
{
	while( _inited ) {
		// ���ִ�ж���
		int time = Check();
		// ֻ�д��ڻ��ߵ�����ʱ�Ž���ȴ�״̬
		if ( time >= 0 ) {
			// �ȴ�
			_monitor.wait( time ) ;
		}
	}
}

// �������ִ�ж���
void CWorkThread::Clear( void ) 
{
	_mutex.lock() ;

	int size = 0;
	_ThreadUnit *p = _queue.move( size ) ;
	if ( size == 0 ) {
		_mutex.unlock() ;
		return ;
	}

	_ThreadUnit *t = NULL ;
	while( p != NULL ) {
		t = p ;
		p = p->_next ;
		delete t ;
	}
	_index.clear() ;

	_mutex.unlock() ;
}
