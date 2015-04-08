/**
 * memo:   �̹߳�����󣬲��ֲο�apache�̳߳�ʵ�֣����У�apache���̶߳������ʹ��boost������ָ�룬
 *         ����ʹ���Լ������ü����������������ù��������̶߳���ʹ����Ҫ�̳� Runnable�ӿ�ʵ��run�ӿڣ�
 *         ���У�param���̸߳��Ի���������Ҫ���ͬһ������ʹ��Runnable��ʵ�ֶ����̳߳���ɲ�ͬ��������Ϳ���ͨ�����Ի�����������
 *
 * date:   2011/07/21
 * author: humingqing
 */

#ifndef __SHARE_THREAD_H__
#define __SHARE_THREAD_H__

#include <Share.h>
#include <Ref.h>
#include <list>
#include <Mutex.h>
#include <pthread.h>

/**
 * �߳����ж���ӿ�
 *
 * @version $Id:$
 */

namespace share {

/**
 * �߳�ִ�ж���
 */
class Runnable {
public:
  virtual ~Runnable() {} ;
  /**
   * �߳����ж���
   */
  virtual void run( void *param ) = 0 ;
};

/**
 * �̴߳������
 */
class Thread : public Ref
{
	 /**
	   * POSIX Thread scheduler policies
	   */
	  enum POLICY {
	    OTHER,
	    FIFO,
	    ROUND_ROBIN
	  };

	  /**
	   * POSIX Thread scheduler relative priorities,
	   *
	   * Absolute priority is determined by scheduler policy and OS. This
	   * enumeration specifies relative priorities such that one can specify a
	   * priority withing a giving scheduler policy without knowing the absolute
	   * value of the priority.
	   */
	  enum PRIORITY {
	    LOWEST = 0,
	    LOWER = 1,
	    LOW = 2,
	    NORMAL = 3,
	    HIGH = 4,
	    HIGHER = 5,
	    HIGHEST = 6,
	    INCREMENT = 7,
	    DECREMENT = 8
	  };

	  enum STATE {
	    uninitialized,
	    starting,
	    started,
	    stopping,
	    stopped
	  };

	  static const int MB = 1024 * 1024;
public:
	  /**
	   * �̹߳������
	   */
	  Thread( Runnable *runner , void *param = NULL ,  int policy = FIFO , int priority = NORMAL , int stackSize = 8 , bool detached = false ) ;

	  virtual ~Thread() ;

	  /**
	   * Starts the thread. Does platform specific thread creation and
	   * configuration then invokes the run method of the Runnable object bound
	   * to this thread.
	   */
	  virtual void start( void ) ;

	  /**
	   * Join this thread. Current thread blocks until this target thread
	   * completes.
	   */
	  virtual void join() ;

	  /**
	   * Gets the thread's platform-specific ID
	   */
	  virtual pthread_t getId() { return _pthread ; } ;

public:
	  /**
	   * ִ���̵߳�������
	   */
	  static void * ThreadMain( void *param ) ;

	  /**
	   * ȡ�����ж���
	   */
	  Runnable * runable( void )  { return _runner ; }

private:
	  /**
	   *  �߳����ж���
	   */
	  Runnable *_runner ;

	  /**
	   *  �߳�ID
	   */
	  pthread_t _pthread ;

	  /**
	   *  �����Ĳ���
	   */
	  void * 	_param ;

	  /**
	   * �߳�״̬
	   */
	  STATE 	_state;

	  /**
	   * POSIX Thread scheduler policies
	   */
	  int 		_policy;

	  /**
	   *  �߳����ȼ�
	   */
	  int 		_priority;

	  /**
	   * ջ�ռ��С
	   */
	  int 		_stackSize;

	  /**
	   * ����
	   */
	  bool 		_detached;

	  /**
	   * �̶߳���
	   */
	  Thread   *_selfRef ;
};

/**
 * �̹߳������
 */
class ThreadManager
{
public:
	ThreadManager():_thread_state(false) {}

	virtual ~ThreadManager() ;
	/**
	 *  ��ʼ���̶߳���
	 */
	virtual bool init( unsigned int nthread, void *param, Runnable *runner )  ;

	/**
	 *  ��ʼ�����߳�
	 */
	virtual void start( void ) ;

	/**
	 *  ֹͣ�߳�
	 */
	virtual void stop( void ) ;

private:
	/**
	 * �̶߳����б�
	 */
	typedef std::list<Thread*>  ThreadList ;

	/**
	 * �̴߳�Ŷ���
	 */
	ThreadList	 _thread_lst ;

	/**
	 * �̳߳ص�״̬
	 */
	bool 		 _thread_state ;
};

}
#endif
