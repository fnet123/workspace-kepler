#ifndef __SOCKETHANDLE_H__
#define __SOCKETHANDLE_H__

#include <errno.h>
#include <databuffer.h>

#ifdef _XDEBUG
#define FUNTRACE(fmt, ...)  printf( fmt , ##__VA_ARGS__ )
#define LOGDEBUG( ip, port, fmt , ... )   \
	OUT_INFO( ip, port, NULL, fmt, ## __VA_ARGS__ )
#else
#define FUNTRACE(fmt, ...)
#define LOGDEBUG( ip, port, fmt , ... )
#endif

#define ERRLOG( ip, port, fmt , ... )  \
	OUT_ERROR( ip, port, NULL, fmt, ## __VA_ARGS__ )

#define MAX_SOCKET_NUM 1024
#define MAX_EVENTS_NUM 1024

#ifdef _MAC_OS
#define MAX_FD_OFFSET			12000    // MAC OS���Ϊ
#else
#define MAX_FD_OFFSET 			0xfffff   // ������������ܳ���65535*16
#endif
#define MIN_FD_OFFSET  			64
#define SOCKET_CONTINUE     	1   	// ��������
#define SOCKET_SUCCESS 	 		0  		// �������ݳɹ�
#define SOCKET_FAILED			-1  	// �������ݴ���
#define SOCKET_DISCONN 			-2  	// ���ն���������
#define SOCKET_RECVERR   		-3  	// �������ݴ���
#define SOCKET_SENDERR			-4  	// �������ݴ���
#define SOCKET_BUFFER		    -5  	// TCP���ͻ�������

#define THREAD_POOL_SIZE         2  	// Ĭ���̸߳���
#ifndef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE 		4096	// �������ݴ�С
#endif
#define UDP_FD_MASK				0xffff    // UDP����
#define UDP_FD_OFSSET			0x10000   // UDP��ʼλ��
#define MAX_SOCKET_BUF			640000    // ����ͺͽ��ջ������Ĵ�С
#define MIN_RECV_THREAD			8		  // �����̴߳���
#define SOCKET_TIMEOUT		    180       // Ĭ������������û���ݳ�ʱ
#define SOCKET_CHECK			30		  // 30��������

struct packet ;
//struct kfifo ;
// �صײ���ְ�����
class IPackSpliter
{
public:
	virtual ~IPackSpliter() {}
	// �ְ�����
	virtual struct packet * get_kfifo_packet( DataBuffer *fifo )  = 0 ;
	// �ͷ����ݰ�
	virtual void free_kfifo_packet( struct packet *packet ) = 0 ;
};

enum SocketType{
	FD_TCP = 1 ,   // TCP���ӷ�ʽ
	FD_UDP = 2 ,   // UDP���ӷ�ʽ
};

enum EventType {
	ReadableEvent = 1,    //!< data available to read
	WritableEvent = 2,    //!< connected/data can be written without blocking
	Exception     = 4     //!< uh oh
};

// ���Ӷ���
class socket_t
{
public:
	socket_t() {
		_type   = FD_TCP ;
		_fd     = -1 ;
		_events = 0 ;
		_last   = 0 ;
		_ptr    = NULL ;
		_next   = NULL ;
		_pre    = NULL ;
	}

	virtual ~socket_t() {}

public:
	// ��������
	unsigned char 		_type ;
	// ������SOCKET
	int 		  		_fd ;
	// �¼�����
	unsigned int  		_events ;
	// ����IP��ַ
	char		   		_szIp[32];      // ��������IP
	// ����Ķ˿�
	unsigned short		_port ;          // �������Ķ˿�
	// ���һ�ν�������ʱ��
	time_t			    _last ;
	// �����û���չ����ָ��
	void *				_ptr ;
	// ָ����һ������ָ��
	socket_t  	 	   *_next ;
	// ǰ���ڵ�
	socket_t		   *_pre ;
};

class CSocketHandle
{
public:
	CSocketHandle(){};
	virtual ~CSocketHandle(){};

public:	
	//����
    virtual bool create(int max_socket_num = MAX_SOCKET_NUM) = 0 ;
	virtual bool destroy() { return false; } ;
    virtual bool add(socket_t *fd, unsigned int events) = 0 ;
	virtual bool del(socket_t *fd, unsigned int events) = 0 ;
	virtual bool modify(socket_t *fd, unsigned int events) = 0 ;
	virtual int  poll(int timeout = 5) = 0 ;
	
	virtual bool is_read(int events){return false;};
	virtual bool is_write(int events){return false;};
	virtual bool is_excep(int events){return false;};
	
    virtual void on_event( socket_t *fd, int events ){};
};

#endif
