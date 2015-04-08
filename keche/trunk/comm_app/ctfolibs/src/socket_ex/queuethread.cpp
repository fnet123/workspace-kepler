/*
 * queuethread.cpp
 *
 *  Created on: 2012-6-19
 *      Author: humingqing
 */

#include "queuethread.h"

CQueueThread::CQueueThread( IPackQueue *queue, IQueueHandler *handler )
	:_queue(queue), _handler(handler), _inited(false)
{

}

CQueueThread::~CQueueThread()
{
	Stop() ;
}

//  ��ʼ��
bool CQueueThread::Init( int thread )
{
	if ( !_threadmgr.init( thread, NULL, this ) ) {
		return false ;
	}
	_inited = true ;
	_threadmgr.start() ;
	return true ;
}

// ֹͣ
void CQueueThread::Stop( void )
{
	if ( ! _inited )
		return ;
	_inited = false ;
	_monitor.notifyEnd() ;
	_threadmgr.stop() ;
}

// �������
bool CQueueThread::Push( void *packet )
{
	_mutex.lock() ;
	if ( ! _queue->Push( packet ) ) {
		_mutex.unlock() ;
		return false ;
	}
	_mutex.unlock() ;
	_monitor.notify() ;

	return true ;
}

// ��������
void CQueueThread::Process( void )
{
	void *p = NULL ;
	// �����ݶ�����ȡ����
	_mutex.lock() ;
	p = _queue->Pop() ;
	_mutex.unlock() ;
	// �������Ϊ������
	if ( p == NULL ) {
		_monitor.lock() ;
		_monitor.wait() ;
		_monitor.unlock() ;
		return ;
	}
	// �ص����ݴ������
	_handler->HandleQueue( p ) ;
	// �ͷ�����
	_queue->Free( p ) ;
}

// �߳����нӿڶ���
void CQueueThread::run( void *param )
{
	while( _inited ) {
		Process() ;
	}
}




