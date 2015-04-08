/*
 * queuethread.h
 *
 *  Created on: 2012-6-19
 *      Author: humingqing
 *  ���ݴ�������̶߳���
 */

#ifndef __QUEUETHREAD_H__
#define __QUEUETHREAD_H__

#include <Monitor.h>
#include <Thread.h>

// ���ݰ�����
class IPackQueue
{
public:
	virtual ~IPackQueue() {}
	// �������
	virtual bool Push( void *packet ) = 0 ;
	// ��������
	virtual void * Pop( void ) = 0 ;
	// �ͷ�����
	virtual void  Free( void *packet ) = 0 ;
};

// ���ݴ��ͽӿ�
class IQueueHandler
{
public:
	virtual ~IQueueHandler() {}
	// �������ݻص��ӿ�
	virtual void HandleQueue( void *packet ) = 0 ;
};

// �߳����ݴ������
class CQueueThread : public share::Runnable
{
public:
	CQueueThread( IPackQueue *queue, IQueueHandler *handler ) ;
	~CQueueThread() ;
	//  ��ʼ��
	bool Init( int thread ) ;
	// ֹͣ
	void Stop( void ) ;
	// �������
	bool Push( void *packet ) ;

public:
	// �߳����нӿڶ���
	void run( void *param ) ;
	// ��������
	void Process( void ) ;

private:
	// ���ݴ�Ŷ���
	IPackQueue 			 *_queue ;
	// ���ݻص��ӿ�
	IQueueHandler   	 *_handler ;
	// ����ͬ��������
	share::Mutex		  _mutex ;
	// �źŹ������
	share::Monitor		  _monitor ;
	// �̹߳������
	share::ThreadManager  _threadmgr ;
	// �Ƿ��ʼ������
	bool 				  _inited ;
};

#endif /* QUEUETHREAD_H_ */
