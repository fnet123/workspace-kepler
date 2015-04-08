/**
 * memo:   ���������TCP���ݴ������,ʹ���µ��̶߳��������뷨��ģ�黯����һ�����,
 * 	���ڵײ����ݷְ������Լ̳�IPackSpliter�ӿڵ������������Ϳ���ʵ���Լ����ݵķְ�
 * date:   2011/07/21
 * author: humingqing
 */

#ifndef __TCP_HANDLE_H__
#define __TCP_HANDLE_H__

#include <list>
#include <set>
#include <stdlib.h>
#ifndef _UNIX
#include "EpollHandle.h"
#else
#ifdef _USE_POLL
#include "PollHandle.h"
#else
#include "KQueueHandle.h"
#endif
#endif
#include <comlog.h>
#include <Thread.h>
#include <Monitor.h>
#include "TQueue.h"
#include "dataqueue.h"
#include "queuethread.h"
#include "protocol.h"

// SOCK���������Ҫ��������״̬
class CSockManager
{
public:
	// ���ݽṹ��
	struct _node
	{
		char  *  _dptr;
		int  	 _len;
		_node *  _next;
	};

	class nodequeue
	{
	public:
		nodequeue();
		~nodequeue();

		// ��ӵ�ͷ��
		void addhead( _node *p ) ;
		// ��ӽڵ���
		bool addtail( const char *buf, int size ) ;
		// ��������
		_node *popnode( void ) ;
		// �Ƶ�������
		void moveto( nodequeue *node ) ;
		// ����
		void reset( void ) ;
		// ��������
		void clear( void ) ;
		// ȡ��ͷ������
		_node * getnodes( void ) { return _head ; }
		// ȡ�����ݳ���
		int  size( void )  { return _size ;}
		// ������´������
		void addref( void ) { ++ _dref ;}

	private:
		// ͷ�ڵ�
		_node  *		_head ;
		// β�ڵ�
		_node  *		_tail ;
		// ����
		unsigned int    _size ;
		// ��������
		unsigned int    _dref ;
	};

	//��ϵͳ������һ����Ч��socket��
	class connect_t: public socket_t
	{
	public:
		connect_t( CSocketHandle *eventer )
			: _read_buff(NULL) , _eventer(eventer) { beginsock(); }
		virtual ~connect_t() { endsock() ; }
		// ����IP�Ͷ˿��Լ�FD
		void init( unsigned int fd, const char *szip , unsigned short port ) ;
		// ��������
		int  write( int &err ) ;
		// ��ȡ����
		struct packet *read( int &ret , int &nerr , IPackSpliter *pack ) ;
		// �����Ҫ���͵�����
		bool deliver( const char *buf, int len ) ;
		// �Ƿ��Ѿ��ͷ�����
		bool close( void ) ;
		// �Ƿ�ʱû���α�����
		bool check( void ) ;

	public:
		// ȡ����������
		_node * readlist( void ) ;

	private:
		// ��ʼ��ϵͳ����
		void beginsock( void ) ;
		// ��������
		void endsock( void ) ;

	private :
		unsigned char  		_status;    	// 1:��ʾ����������У�0:��ʾ���ڡ�
		nodequeue 		   *_inqueue ;      // �������ݵ�Queue
		nodequeue		   *_outqueue ;		// ������ݵ�Queue
		share::Mutex  		_mutex;	    	// ���ݴ�����
		DataBuffer 		   *_read_buff;     // ���ݻ���
		CSocketHandle      *_eventer;  // SOCK�¼��������
	};

public:
	CSockManager( CSocketHandle *eventer) :_eventer(eventer)  {}
	~CSockManager() { clear(); }
	// ǩ������
	socket_t * get( int sockfd, const char *ip, unsigned short port , bool queue = true ) ;
	// ǩ������
	void  put( socket_t *sock ) ;
	// �ر�����
	bool  close( socket_t *sock ) ;
	// ȡ�����л������Ӷ���
	socket_t * recyle( void ) ;
	// ��ⳬʱ���Ӷ���
	int check( int timeout, std::list<socket_t*> &lst ) ;

private:
	// ����������Դ
	void clear( void ) ;

private:
	// ���ݶ���ͷ
	TQueue<socket_t>     _queue ;
	// ���߶��й���
	std::set<socket_t*>  _index ;
	// ���߶��й���
	TQueue<socket_t>	 _online ;
	// ���ն�����
	TQueue<socket_t>	 _recyle;
	// ����ͬ��������
	share::Mutex         _mutex ;
	// SOCK�¼��������
	CSocketHandle     *  _eventer;
} ;

#define  TCP_THREAD_CONN   		1    // �����¼��߳�
#define  TCP_THREAD_DATA   		2    // ���������߳�
#define  TCP_MAX_THREAD        128   // ����̸߳���

#ifndef _UNIX
class CTcpHandle : public CEpollHandle, public share::Runnable ,public IQueueHandler
#else
#ifdef _USE_POLL
class CTcpHandle : public CPollHandle, public share::Runnable ,public IQueueHandler
#else
class CTcpHandle : public CKQueueHandle , public share::Runnable ,public IQueueHandler
#endif
#endif
{
	// Ĭ�ϵķְ�����
	class CDefaultSpliter: public IPackSpliter
	{
	public:
		CDefaultSpliter(){}
		~CDefaultSpliter(){}

		// �ְ�����
		struct packet * get_kfifo_packet( DataBuffer *fifo ){
			// ȡ����Э�����ݵķְ�
			return get_packet_from_kfifo(fifo) ;
		}

		// �ͷ����ݰ�
		void free_kfifo_packet( struct packet *packet ){
			// �ͷ����ݰ�
			free_packet( packet ) ;
		}
	};
public:
	CTcpHandle() ;
	virtual ~CTcpHandle() ;

	//��ʼ���̳߳أ��첽socket��ʹ��ǰ������øú���
	bool 	init( unsigned int nthread = THREAD_POOL_SIZE , unsigned int timeout = SOCKET_TIMEOUT );
	// ֹͣ�̣߳���������
	bool 	uninit() ;

	bool 	start_server( int port , const char* ip = NULL ) ;
	bool 	stop_server() ;

	//��Ӧ�ò�����Ͷ�ݵ�fd���������ȴ��̳߳صĵ��ȷ��͡�
	bool 	deliver_data( socket_t *sock , const void* data, int len ) ;
	// �Ͽ����ӣ��Ƿ�ص� on_disconnection��notify��������
	void    close_socket( socket_t *sock , bool notify = true ) ;
	// ���÷ְ�����
	void    setpackspliter( IPackSpliter *pack ) { _pack_spliter = pack; }
	// �߳����ж���
	void 	run( void *param )  ;
	// �������ݻص��ӿ�
	void 	HandleQueue( void *packet ) ;
	//����fd
	socket_t * connect_nonb( const char* ip, int port, int nsec = 5 );

protected:
	// �����¼�����
	void on_event( socket_t *sock , int events );
	//������on_disconnection
	bool close_fd( socket_t *sock ) ;

protected:
	/*
	 *  �û��Զ���ص��������˺���ִ��ʱ��fd����״̬Ϊlocked������
	 *  �˺����л�˺������ú����� ������ͼlock fd������������
	 */
	virtual void on_data_arrived( socket_t *sock, void* data, int len){}
	//����ʧ��
	virtual void on_send_failed( socket_t *sock, void* data, int len){}
	/*
	 *  ���ӹر�ʱ�Ļص�������
	 */
	virtual void on_dis_connection( socket_t *sock ){};
	// �µ����ӵ���
	virtual void on_new_connection( socket_t *sock , const char* ip, int port){};
	// ���IP�Ƿ���Ч
	virtual bool invalidate_ip( const char *ip ) = 0 ;

private:
	// д�������־
	void  write_errlog( socket_t *sock , int err , int nerr ) ;
	// ������
	void  read_data( socket_t *sock ) ;
	// д����
	bool  write_data( socket_t *sock ) ;
	// ��������״����߳�
	void  process_check( void ) ;
	// �����û����������߳�
	void  process_data( void ) ;

private:
	//����˿�fd
	socket_t 			* _server_fd ;
	// �Ƿ�ֹͣ����
	bool 				  _tcp_init ;
	// ���÷ְ�����
	IPackSpliter		 *_pack_spliter ;
	// �����¼��߳�
	share::ThreadManager  _thread_conn ;
	// ���Ӵ������߳�
	share::ThreadManager  _thread_check;
	// ���ݴ������
	CDataQueue<CPacket>	* _packqueue ;
	// SOCKET�������
	CSockManager		* _socketmgr ;
	// ���ݴ����߳�
	CQueueThread		* _queuethread;
	// Ĭ�ϵķְ�����
	CDefaultSpliter		  _defaultspliter;
	// Ĭ�����ӳ�ʱʱ��
	unsigned int 		  _socktimeout;
};

#endif
