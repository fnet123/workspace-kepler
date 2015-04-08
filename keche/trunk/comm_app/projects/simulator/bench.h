/**
 * author: humingqing
 * date:   2011-09-19
 * memo:   ʵʱͳ�������������
 */
#ifndef __BENCH_H__
#define __BENCH_H__

#include <time.h>
#include <Mutex.h>
#include <Thread.h>

enum EBenchType
{
	BENCH_ALL_USER = 0 ,
	BENCH_ON_LINE  ,
	BENCH_OFF_LINE ,
	BENCH_CONNECT  ,
	BENCH_DISCONN  ,
	BENCH_MSGSEND  ,
	BENCH_MSGRECV
};

class CBench : public share::Runnable
{
	struct _BenchData
	{
		unsigned int alluser_ ;  // ����û���
		unsigned int online_  ;  // ��ǰ�����û���
		unsigned int offline_ ;  // �����û���
		unsigned int connect_ ;  // ���Ӵ���
		unsigned int disconn_ ;  // ��������
		unsigned int allconn_ ;  // �����Ӵ���
		unsigned int alldis_  ;  // �ܶ�������
		unsigned int msgsend_ ;  // ������Ϣ��
		unsigned int msgrecv_ ;  // ������Ϣ��
		unsigned int allsend_ ;  // ���з�����Ϣ��
		unsigned int allrecv_ ;  // ���н�����Ϣ��
		time_t 		 spantime_;  // ����ʱ��
		time_t		 lasttime_ ; // ���һ��ʱ��

		_BenchData() {
			alluser_  =  online_ = offline_ = 0 ;
			connect_  = disconn_ = 0 ;
			msgsend_  = msgrecv_ = 0 ;
			allsend_  = allrecv_ = 0 ;
			spantime_ =  0 ;
			lasttime_ = time(NULL) ;
		}
	};
public:
	CBench() ;
	~CBench() ;

	// ��ʼ��
	bool Init( void ) ;
	// ����
	bool Start( void ) ;
	// ֹͣ
	void Stop( void ) ;
	// ͳ������
	void IncBench( EBenchType type , int n = 1 ) ;

public:
	// �����̶߳���
	virtual void run( void *param ) ;

private:
	// ͳ��������
	share::Mutex 		 _mutex ;
	// �̶߳���
	share::ThreadManager _thread ;
	// ͳ������
	_BenchData		     _data ;
	// ���һ�η��ʵ�ʱ��
	time_t			     _last_access ;
	// �Ƿ������߳�
	bool 			 	 _start ;
};

#endif
