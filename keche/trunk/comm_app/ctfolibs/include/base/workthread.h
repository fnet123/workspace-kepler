/************************************************************************/
/* Author: humingqing                                                   */
/* Date:   2013-02-05													*/
/* Memo:   �߳�ִ�гض���												*/
/************************************************************************/
#ifndef __WORKTHREAD_H__
#define __WORKTHREAD_H__

#include <map>
#include <Monitor.h>
#include <Thread.h>
#include <sortqueue.h>

#define THREAD_REG_ERROR  -1  // ע��ִ�ж���ʧ��

class CWorkThread: public share::Runnable
{
	struct _ThreadUnit
	{
		int			 	 _id ;
		int			 	 _span ;
		share::Runnable *_pProc ;
		void        *	 _ptr ;
		bool		 	 _run ;
		time_t		 	 _time ;
		_ThreadUnit *	 _next ;
		_ThreadUnit *	 _pre ;

		_ThreadUnit() 
		{
			_next  = _pre = NULL ;
			_pProc = NULL ;
			_ptr   = NULL ;
			_run   = false ;
		}
	};
	typedef std::map<int,_ThreadUnit*> CMapUnit;

	// ���в�������
	class CSequeue
	{
	public:
		CSequeue():_id(0){}
		~CSequeue(){}

		// ��������
		int get_next_seq( void ) {
			int id = 0 ;

			_mutex.lock() ;
			_id = _id + 1 ;
			if ( _id < 0 ) {
				_id = 1 ;
			}
			id = _id;
			_mutex.unlock() ;

			return id ;
		}

	private:
		// �߳�ID�Ŷ���
		int  		  _id ;
		// ����ͬ����
		share::Mutex  _mutex;
	};
public:
	CWorkThread() ;
	~CWorkThread() ;

	// ��ʼ���߳�
	bool Init( int nthread ) ;
	// ֹͣ�߳�
	void Stop( void ) ;
	// ע���߳�ִ�ж���,��ʱ��Ϊ����Ĭ��ִ��һ��
	int  Register( share::Runnable *pProc, void *ptr = NULL, int time = 0 ) ;
	// ����ִ�ж���
	void UnRegister( int id ) ;

protected:
	// ����Ƿ�����Ҫ���ж���
	int  Check( void ) ;
	// �߳����нӿڶ���
	void run( void* param ) ;
	// �������ִ�ж���
	void Clear( void ) ;

private:
	// �źŹ������
	share::Monitor			 _monitor ;
	// �̹߳������
	share::ThreadManager	 _threadmgr ;
	// �Ƿ��ʼ������
	bool 					 _inited ;
	// ���й������
	CSequeue				 _sequeue;
	// ͬ����������
	share::Mutex			 _mutex;
	// ִ�ж������
	TSortQueue<_ThreadUnit>  _queue;
	// ��������
	CMapUnit				 _index;
};

#endif
