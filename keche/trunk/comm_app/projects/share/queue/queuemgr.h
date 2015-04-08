/*
 * queuemgr.h
 *
 *  Created on: 2012-10-22
 *      Author: humingqing
 *
 *	memo:
 *  ���ݷ����ط��ȴ����У�������ݵ�����յ�ͨ��Ӧ�����գ����û���յ�ͨ��Ӧ��ʱ��ʱ��Ҫ�ط�
 *  ���У�ʵ���˿��Խ������Ӻ��͵Ļ��ƣ��Ƚ����ݴ�ŵ����ݶ����У�������ݲ����������ͣ���ô����Ҫ���ų�ʱ������Ӧ������
 *
 *   1. �첽���͵����ݵȴ����У������յ���Ӧ����ն���,���ȴ���Ӧ��ʱʱ����Ҫ���·���
 *   2. ���·���ÿһ�εȴ���ʱʱ�䣬��ε�ʱ��Ϊ��2�ĵȴ���ϵ��Ҳ�����ط�����Խ��ȴ�ʱ��Խ��
 *   3. ����ն˴����������ޣ���֤����Ҫ��������ֱ������еȴ����ͻ�˳����
 *   4. ��˳����ʱ��ƽ̨�յ��ն���һ���·�����Ӧ���Զ�������һ���ȴ���Ҫ���͵���Ϣ
 *   5. ����ȴ��������ݳ�ʱ��ƽ̨���Զ��·���Ϣ������ж����ʱ�·����ݣ���ֻ���͵������ݣ�ֱ���յ���һ��Ӧ������ٴγ�ʱ���·�
 */

#ifndef __QUEUEMGR_H_
#define __QUEUEMGR_H_

#include <map>
#include <string>
#include <list>
#include <Thread.h>
#include <Monitor.h>
#include <sortqueue.h>

#define QUEUE_MAXRESEND   2   // ����ط�����
#define QUEUE_SENDTIME    5   // ���з���ʱ�����

// ���ݻص�����
class IQCaller
{
public:
	virtual ~IQCaller() {}
	// ���ó�ʱ�ط�����
	virtual bool OnReSend( void *data ) = 0 ;
	// ���ó�ʱ���ط�������ɾ������
	virtual void Destroy( void *data ) = 0 ;
};

// ���ݶ��й������
class CQueueMgr: public share::Runnable
{
	class CQueue
	{
		struct _QData
		{
			time_t  _time ;  // ��������ʱ��
			int     _ntime;  // ��Գ�ʱʱ��
			int     _ent ;   // �ط�����
			int     _seq ;   // ���ID
			void *  _ptr ;   // ���͵���������
			_QData *_pre ;   // ǰ���ڵ�
			_QData *_next ;  // �����ڵ�
		};
		typedef std::map<int, _QData*> _QIndex;
	public:
		CQueue( IQCaller *pCaller , int ent ) ;
		~CQueue() ;
		// �������
		bool Add( unsigned int seq, void *data, int timeout, bool send ) ;
		// ɾ������
		void Remove( unsigned int seq ) ;
		// ��������е����ݷ���һ��
		void Send( void ) ;
		// ��ⳬʱ������
		bool Check( int &nexttime ) ;

	private:
		// ��ӵ��ڵ���
		void Add( _QData *p ) ;
		// �Ƴ��ڵ�
		void Remove( _QData *p ) ;

	private:
		// ʹ���������������
		TSortQueue<_QData> _queue ;
		// ���ݶ�������
		_QIndex 		   _index ;
		// ���ݴ���ص�����
		IQCaller   *	   _pCaller;
		// ��Ҫ���͵ĸ���
		int 			   _send ;
		// ����ط�����
		int 	    	   _maxent ;

	public:
		// ���ݶ��е�ID��
		std::string _id ;
		// �����һ��Ԫ�ؽ���
		CQueue    * _next ;
		// ����ǰһ��Ԫ��ָ��
		CQueue    * _pre ;
	};
	typedef std::map<std::string,CQueue*>  CMapQueue;

public:
	CQueueMgr( IQCaller *pCaller , int time = QUEUE_SENDTIME, int ent = QUEUE_MAXRESEND ) ;
	virtual ~CQueueMgr() ;
	// ��ӵ��ȴ����Ͷ����У��Ƿ�Ϊ�Ӻ����̶߳���������,��Ҫ��������ͨ�����յ���Ӧ�󴥷�����
	bool Add( const char *id, unsigned int seq, void *data , bool send = false ) ;
	// ɾ��ID�Ŷ���
	void Del( const char *id ) ;
	// �Ƴ�����,�Ƿ�����Ҫ������Ϣ����
	void Remove( const char *id, unsigned int seq , bool check = true ) ;

protected:
	// ʵ���̶߳������м��ӿ�
	virtual void run( void *param ) ;
	// ��յ���������
	void Clear( void ) ;

private:
	// ͬ���ȴ�����
	share::Monitor       _monitor ;
	// ����������
	share::Mutex		 _mutex ;
	// �ȴ������̹߳������
	CMapQueue  			 _index ;
	// �߳�ִ�ж���
	share::ThreadManager _thread;
	// �Ƿ��ʼ��
	bool 				 _inited ;
	// ���ݻص�����
	IQCaller		    *_pCaller ;
	// ʹ�ö���ģ��
	TQueue<CQueue>		 _queue ;
	// ����ʱ����
	int 				 _maxspan ;
	// ���ʹ�������
	int 				 _maxent ;
};


#endif /* MYQUEUE_H_ */
