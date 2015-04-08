/*
 * waitgroup.h
 *
 *  Created on: 2011-11-8
 *      Author: humingqing
 */

#ifndef __WAITGROUP_H__
#define __WAITGROUP_H__

#include <inodeface.h>
#include <map>
#include <msgbuilder.h>
#include <Thread.h>
#include <TQueue.h>

#define MAX_WAITGROUP_TIMEOUT  30  // �ȴ���Ӧ���ʱʱ��

class CAllocMsg : public IAllocMsg
{
public:
	CAllocMsg(){}
	~CAllocMsg(){}

	// �����ڴ�
	MsgData * AllocMsg(){
		return new MsgData;
	}
	// �����ڴ�
	void FreeMsg( MsgData *p ){
		if ( p == NULL )
			return ;
		delete p ;
	}
};

class CWaitGroup : public IWaitGroup, public share::Runnable
{
	// ��������
	struct Queue
	{
		int        seq ;   // ���ID��
		socket_t  *sock ;  // which fd cope
		time_t     now ;   // current time
		MsgData *  msg ;   // msg data
		ListFd     fdlst ; // notify fd
		Queue 	  *_next;
		Queue     *_pre ;
	};
	// ��Ŷ�Ӧ�鴦��
	typedef std::map<uint32_t,Queue*> CMapQueue ;
public:
	CWaitGroup(IAllocMsg *pAlloc) ;
	~CWaitGroup() ;

	// ��ʼ��
	bool Init( void ) ;
	// ��ʼ�߳�
	bool Start( void ) ;
	// ֹͣ
	void Stop( void ) ;

	// ����������
	bool AddGroup( socket_t *sock, unsigned int seq, MsgData *msg ) ;
	// ��ӵ���Ӧ��������
	bool AddQueue( unsigned int seq, socket_t *sock ) ;
	// ɾ����Ӧ��ֵ
	bool DelQueue( unsigned int seq, socket_t *sock , bool bclear ) ;
	// ȡ��û����Ӧ��FD
	int  GetCount( unsigned int seq ) ;
	// ȡ�ö�Ӧ��FD��LISTֵ
	bool GetList( unsigned int seq, ListFd &fds ) ;
	// ���ûص�����
	void SetNotify( IMsgNotify *pNotify ) { _notify = pNotify ; }
	// �߳����ж���
	void run( void *param ) ;

private:
	// ��ⳬʱ
	void CheckTime( void ) ;
	// �����������
	void ClearAll( void ) ;
	// �Ƴ���ʱ����
	Queue* RemoveMsg( unsigned int seq ) ;

private:
	IAllocMsg		    *_pAlloc ;
	// �����������
	share::Mutex  		 _mutex ;
	// ����SEQ����
	CMapQueue     		 _index ;
	// ʱ������
	TQueue<Queue>	  	 _queue ;
	// �̶߳���
	share::ThreadManager _thread ;
	// �Ƿ��ʼ���߳�
	bool 				 _inited ;
	// ֪ͨ�ص�����
	IMsgNotify 			*_notify ;
};

#endif /* GROUPWAIT_H_ */
