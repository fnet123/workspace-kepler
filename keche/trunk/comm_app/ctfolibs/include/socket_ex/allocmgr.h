#ifndef __ALLOCMGR_H__
#define __ALLOCMGR_H__

#include <stdio.h>
#include <TQueue.h>
#include <Mutex.h>

// ���п����ڴ����
template<typename T>
class TAllocMgr
{
public:
	TAllocMgr() {}
	~TAllocMgr() {}
	// ȡ�ö���
	T * alloc( void ) {
		T *p = NULL ;

		_mutex.lock();
		if ( _queue.size() == 0 ) {
			_mutex.unlock() ;
			return NULL ;
		}
		p = _queue.begin() ;
		_queue.erase( p ) ;
		_mutex.unlock() ;

		return p ;
	}

	// �������
	void free( T *obj ) {
		if ( obj == NULL )
			return ;

		_mutex.lock() ;
		_queue.push( obj ) ;
		_mutex.unlock() ;
	}

	// �ڴ���г���
	int size( void ) {
		int nsize = 0 ;
		_mutex.lock() ;
		nsize = _queue.size() ;
		_mutex.unlock() ;
		return nsize ;
	}

private:
	// ͬ��������
	share::Mutex	 _mutex;
	// ������ݶ���
	TQueue<T>		 _queue;
};

#endif
