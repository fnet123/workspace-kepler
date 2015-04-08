/**
 * author: humingqing
 * date:   2011-09-07
 * memo:   �첽HTTP�������࣬��Ҫʵ���첽����HTTP���󷵻ض�Ӧ����ֵ��������Ҫ�Ȼ�HTTP���������ţ�
 *  	�ȴ�ŵȴ������У�Ȼ���ٵ��ã��첽������Ҫ������У�Ȼ���ٲ�������Ϊ�첽�����п��ܳ�����Ӧ�Ȼص����
 */

#ifndef  __ASYNC_HTTP_CLIENT_H__
#define  __ASYNC_HTTP_CLIENT_H__

#include <map>
#include <vector>
#include <time.h>
#include "NetHandle.h"
#include "qstring.h"
#include "httpclient.h"
#include <protocol.h>
#include <Thread.h>
#include <Monitor.h>
#include <allocmgr.h>

// �����첽��HTTP����
class IHttpCallbacker
{
public:
	virtual ~IHttpCallbacker() {}
	// �첽�ص�����
	virtual void ProcHTTPResponse( unsigned int seq_id , const int err , const CHttpResponse& resp ) = 0 ;
};

// �첽��HTTP�Ĵ������
class CAsyncHttpClient : public CNetHandle, public share::Runnable , public IPackSpliter
{
	// ������
	struct _REQ_DATA
	{
		// ���ӵ�FDֵ
		socket_t  	*_fd ;
		// ���ID
		unsigned int _seq ;
		// ���һ�ηõ�ʱ��
		time_t   	 _time ;
		// ������IP
		CQString  	 _ip ;
		// �������Ķ˿�
		unsigned int _port ;
		// ��Ҫ��������
		CQString  	 _senddata ;
		// ���ü�������
		int 		 _ref ;

		_REQ_DATA   *_next ;
		_REQ_DATA   *_pre ;
	};

	// �ȴ�����
	class CWaitListReq
	{
	public:
		CWaitListReq() ;
		~CWaitListReq() ;
		// ��ŵȴ����������
		void PushReq( _REQ_DATA *data ) ;
		// ������Ҫ���������
		_REQ_DATA *PopReq ( void ) ;
		// ���ٿռ�
		_REQ_DATA *AllocReq( void ) ;
		// ���ն���
		void FreeReq( _REQ_DATA *req ) ;
		// ȡ�õ�ǰԪ�ظ���
		int GetQueueSize() ;

	private:
		// �ȴ���������
		typedef std::list<_REQ_DATA*>	CListReq ;
		CListReq				_list_req ;
		// ��¼��ǰ�������
		int 					_size ;
		// �ȴ��ĸ���
		int 					_waitsize;
		// ������
		share::Mutex  			_mutex ;
		share::Monitor  		_monitor ;
		// �ڴ�������
		TAllocMgr<_REQ_DATA>    _allocmgr ;
	};

public:
	CAsyncHttpClient();
	~CAsyncHttpClient();

public:
	// ��ʼ�߳�
	bool Start( unsigned int nsend = 1 , unsigned int nrecv = 1 ) ;
	// ֹͣ�߳�
	void Stop( void ) ;
	// ����HTTP�����󣬷��ض�Ӧ���������
	int  HttpRequest( CHttpRequest& request , unsigned int seq_id ) ;
	// �������ݴ���ص�����
	void SetDataProcessor( IHttpCallbacker* p ){ _pCallbacker = p ; } ;
	// ȡ���������
	unsigned int GetSequeue( void ) ;
	// ����HTTP�����������󳤶�
	void SetQueueSize( int size ){ _maxsize = size; };

public:
	// �����߳�
	virtual void run( void *param ) ;
	virtual void on_data_arrived( socket_t *fd, const void* data, int len);
	virtual void on_dis_connection( socket_t *fd ) ;
	virtual void on_new_connection( socket_t *fd, const char* ip, int port){};
	// �ְ�����
	virtual struct packet * get_kfifo_packet( DataBuffer *fifo ) ;
	// �ͷ����ݰ�
	virtual void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}

private:
	// ����ȴ������е�����
	void ProcessWaitReq( void ) ;

private:
	// ���ݻص�����
	IHttpCallbacker* 		_pCallbacker ;
	// ���������߳�
	share::ThreadManager  	_check_thread ;
	// ���Ŵ���
	unsigned int 			_seq_id ;
	// �����
	share::Mutex 			_mutex_seq ;
	// �Ƿ��ʼ��
	bool 					_initalized ;
	// �ȴ���������
	CWaitListReq			_list_req ;
	// ����HTTP�Ķ��г���
	int 					_maxsize ;
	// ���ü�����
	share::Mutex		    _mutex_ref ;
};



#endif
