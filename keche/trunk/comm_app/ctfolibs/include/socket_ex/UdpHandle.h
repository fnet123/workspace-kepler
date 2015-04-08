/***
 *  memo  : UDP����������
 *  author: humingqing
 *  date:   2012-11-22
 *  description: UDP�����࣬��Ҫʵ��ԭ��UDP��װ���ڲ�������TCP�Ĵ�����ƴ���
 *
 */
#ifndef __UDPHANDLE_H__
#define __UDPHANDLE_H__

#include <map>
#include <vector>
#include <string>
#include <set>
using namespace std;
#ifndef _UNIX
#include "EpollHandle.h"
#else
#ifdef _USE_POLL
#include "PollHandle.h"
#else
#include "KQueueHandle.h"
#endif
#endif
#include <Thread.h>
#include "TQueue.h"
#include "dataqueue.h"
#include "queuethread.h"
#include "protocol.h"

#define  UDP_CLIENT_CONN		1    // UDP�ͻ���
#define  UDP_SERVER_CONN		2    // ��������
#define  UDP_UNKNOW_CONN		3
#define  UDP_THREAD_CONN   		1    // �����߳�
#define  UDP_THREAD_DATA		2	 // ���ݴ����߳�

class CUdpHandle ;
// SOCK��������������ʹ�ö�д������������ʵ�ֶ���߳�ͬʱ���Ĳ���
class CUdpSockManager
{
public:
	//��ϵͳ������һ����Ч��socket��
	class udpconnect_t: public socket_t
	{
	public:
		udpconnect_t() :_read_buff(NULL) { beginsock(true, true); } ;
		virtual ~udpconnect_t() { endsock(true) ; }

		// ����IP�Ͷ˿�
		void  init( int fd, const char *ip, unsigned short port , int ctype ) ;
		// ��ȡ����
		struct packet * split( const char *buf, int len, IPackSpliter *pack ) ;
		// д����
		int  write( const char *data, const int len ) ;
		// �Ƿ�Ϊ�ͻ�������
		bool close( void ) ;

	private:
		// ��ʼ��ϵͳ����
		void beginsock( bool bkfifo  , bool breset ) ;
		// ��������
		void endsock( bool bkfifo ) ;

	private:
		unsigned long  		 _size ;          // �����е����ݴ�С����λ�ֽڡ�
		share::Mutex		 _mutex ;
		DataBuffer 		    *_read_buff ;     // ���ݻ���
		int 				 _ctype ;		  // ��������
	};
public:
	CUdpSockManager()  ;
	virtual ~CUdpSockManager() ;

	// �ְ�����
	socket_t * recv( int server_fd, CUdpHandle *handle , int &ret, int &err , IPackSpliter *pack ) ;
	// ǩ������
	socket_t * get( int sockfd, const char *ip, unsigned short port , int ctype , bool queue = true ) ;
	// ǩ������
	void  put( socket_t *sock ) ;
	// �ر�����
	bool  close( socket_t *sock ) ;
	// ����ʱ�����Ӷ���
	int   check( int timeout, std::list<socket_t*> &lst ) ;

private:
	// ����������Դ
	void clear( void ) ;

private:
	// ���ݶ���ͷ
	TQueue<socket_t>		     	 _queue ;
	// ���߶��в�������
	std::set<socket_t*>  			 _index ;
	// ���߶��й���
	TQueue<socket_t>				 _online ;
	// ����ͬ��������
	share::Mutex         			 _mutex ;
	// ���Ӷ�����ҹ���
	std::map<std::string, socket_t*> _mpsock;
	// ���ݽ��ջ������Ĵ�С
	char 		 _szbuf[READ_BUFFER_SIZE+1];
};

#ifndef _UNIX
class CUdpHandle : public CEpollHandle, public share::Runnable , public IQueueHandler
#else
#ifdef _USE_POLL
class CUdpHandle : public CPollHandle, public share::Runnable , public IQueueHandler
#else
class CUdpHandle : public CKQueueHandle , public share::Runnable ,public IQueueHandler
#endif
#endif
{
	friend class CUdpSockManager ;
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
	CUdpHandle();
	virtual ~CUdpHandle();

	//��ʼ���̳߳أ��첽socket��ʹ��ǰ������øú���
	bool 	init( unsigned int nthread = THREAD_POOL_SIZE , unsigned int timeout = SOCKET_TIMEOUT );
	// ֹͣ�̣߳���������
	bool 	uninit() ;

	bool 	start_server( int port , const char* ip = NULL  ) ;
	bool 	stop_server() ;

	//��Ӧ�ò�����Ͷ�ݵ�fd���������ȴ��̳߳صĵ��ȷ��͡�
	bool 	deliver_data( socket_t *sock , const void* data, int len ) ;
	// �Ͽ����ӣ��Ƿ�ص� on_disconnection��notify��������
	void    close_socket( socket_t *sock , bool notify = true ) ;
	// ���÷ְ�����
	void    setpackspliter( IPackSpliter *pack ) { _pack_spliter = pack ;};
	// ����ͻ����ӹ���
	socket_t * connect_nonb(const char* ip, int port, int nsec = 5 ) ;
	// �������ݻص��ӿ�
	void 	HandleQueue( void *packet ) ;

protected:

	// �����¼�����
	void on_event( socket_t *sock, int events );
	// �߳����ж���
	void run( void *param )  ;
	// д������־
	void write_errlog( socket_t *sock , int err , int nerr ) ;

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
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

private:
	// ��������״����߳�
	void  process_check( void ) ;
	// �����û����������߳�
	void  process_data( void ) ;

protected:
	//����˿�fd
	socket_t 			  * _server_fd ;
	// �Ƿ��������߳�
	bool 					_udp_init ;
	// ���÷ְ�����
	IPackSpliter		 *  _pack_spliter ;
	// �����¼������߳�
	share::ThreadManager    _thread_conn ;
	// ����״̬�������
	share::ThreadManager	_thread_check ;
	// ���ӹ������
	CUdpSockManager 	  * _socketmgr ;
	// ���ݴ������
	CDataQueue<CPacket>	   *_packqueue ;
	// ���ݴ����߳�
	CQueueThread		  * _queuethread;
	// Ĭ�Ϸְ�����
	CDefaultSpliter			_defaultpacker;
	// ���ӳ�ʱ����ʱ��
	unsigned int		    _socktimeout;
};

#endif
