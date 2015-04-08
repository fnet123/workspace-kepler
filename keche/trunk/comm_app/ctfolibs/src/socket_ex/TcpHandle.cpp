/**
 * memo:   ���������TCP���ݴ������,ʹ���µ��̶߳��������뷨��ģ�黯����һ�����
 * date:   2011/07/21
 * author: humingqing
 *
 * <2012/06/20> ���������ݽ��պͷ���ʹ�õ��߳�ģ�����������ݣ�������ǰ����������ϴ�����ٶ�ԶС��CPU�Ĵ���������������£���ʵ�ʵ��������Ҳ�Ǵ��ڣ�
 * �������CPU�����������紫��˵�����������߳����бȽϷ�ʱ�������������ݵݽ����ⲿʹ����ˮ�ķ�ʽ���䣬�յ��������ݺ������ְ������ݶ����У�
 * Ȼ�����ݵݽ����̶߳����У����̶߳����������ⲿ�ݽ����ݡ�
 *
 * �������ܲ��ԣ����ַ����������������ʱ�������ܺ�Ч����Խϸߣ���������Ӧ����������������Ȼ����ֶ���������������ʱ���ڴ�������������������ʱ�ָ�����������ԽϿ졣
 *
 */
#include "TcpHandle.h"
#include "list.h"
#include <assert.h>
#include "UtilitySocket.h"
#include "protocol.h"
#include <errno.h>
#include <poll.h>
#include <arpa/inet.h>
#ifndef _UNIX
#include <sys/time.h>
#include <sys/resource.h>
#endif

#define MAXQUEUE_LENGTH        102400    // �����г���
#define SOCKET_CLOSE    		0      	// ����״̬
#define SOCKET_LIVE     		1      	// ����״̬
#define SOCKET_WAITCLOSE 		2       // SOCKET�ȴ��ر�״̬
#define SOCKET_SENDBUFFER       3		// �������ݲ���
#define SOCKET_EWOULDBLOCK      2		// ���ͻ�������

////////////////////////////CConnection ���ӹ�������� ////////////////////
CSockManager::nodequeue::nodequeue(){
	_head = _tail = NULL ;
	_size = _dref =  0   ;
};
CSockManager::nodequeue::~nodequeue(){
	clear() ;
};

// ��ӵ�ͷ��
void CSockManager::nodequeue::addhead( CSockManager::_node *p )
{
	if ( _head == NULL ) {
		_head = _tail = p ;
		_tail->_next  = NULL ;
		_size         = 1 ;
	} else {
		p->_next  = _head ;
		_head     = p ;
		_size     = _size + 1 ;
	}
}

// ��ӽڵ���
bool CSockManager::nodequeue::addtail( const char *buf, int size )
{
	if ( buf == NULL || size == 0 )
		return false ;

	if( _size >= MAXQUEUE_LENGTH ) {
		// printf( "over max data queue length 102400\n" ) ;
		// �����������ݴ���10M����֪ͨ����ʧ��, �������ӱ��ͷ�
		return false;
	}

	_node * dn = (_node *)malloc(sizeof(_node));
	memset(dn, 0, sizeof(_node));

	dn->_dptr = (char*)malloc(size+1);
	memset(dn->_dptr, 0, size);
	memcpy(dn->_dptr, buf, size);
	dn->_len  = size;
	dn->_next = NULL;

	_size = _size + 1;

	if( _head != NULL ) {
		_tail->_next = dn;
		_tail = dn;
	} else {
		_head = dn;
		_tail = dn;
	}
	return true;
}

// ��������
CSockManager::_node *CSockManager::nodequeue::popnode( void )
{
	if ( _size == 0 )
		return NULL ;

	CSockManager::_node *p = NULL ;
	if ( _head == _tail ) {
		p = _head ;
		_head = _tail = NULL ;
		_size = 0 ;
	} else {
		p = _head ;
		_head = p->_next  ;
		_size = _size - 1 ;
	}
	return p ;
}

// �Ƶ�������
void CSockManager::nodequeue::moveto( nodequeue *node )
{
	// ����������ϵ����ݲ�һ����Ҫ������һ����������
	if ( _dref != node->_dref ) {
		clear() ;
	}
	// ������ݲ�һ����������һ�ε����ݣ����ж����ݴ�С
	if ( node->_size == 0 || _size > 1024 )
		return ;

	if ( _tail == NULL ) {
		_tail = node->_tail ;
		_head = node->_head ;
		_size = node->_size ;
	} else {
		_tail->_next = node->_head ;
		_tail        = node->_tail ;
		_size = _size + node->_size ;
	}
	_dref = node->_dref ;

	// ȡ����������
	node->reset() ;
}

// ����
void CSockManager::nodequeue::reset(void)
{
	_head = _tail = NULL;
	_size = 0;
}

// ��������
void CSockManager::nodequeue::clear( void )
{
	if ( _size == 0 || _head == NULL )
		return ;

	_node* p 	 = NULL;
	_node* next  = NULL;

	p = _head;
	while( p != NULL ) {
		next = p->_next;
		free(p->_dptr);
		p->_dptr = NULL;
		p->_len = 0;
		free(p);
		p = next;
	}
	_head = NULL ;
	_tail = NULL ;
	_size = 0 ;
}

//////////////////////////  Connection ////////////////////////////////////
// ��ʼ��ϵͳ����
void CSockManager::connect_t::beginsock( void )
{
	_fd        = 0;
	_status    = SOCKET_LIVE;
	_inqueue   = new nodequeue ;
	_outqueue  = new nodequeue ;
	_read_buff = new DataBuffer ;
}

// ��������
void CSockManager::connect_t::endsock( void )
{
	if ( _read_buff != NULL ) {
		delete _read_buff ;
		_read_buff = NULL ;
	}

	if ( _inqueue != NULL ){
		delete _inqueue ;
		_inqueue = NULL ;
	}
	if ( _outqueue != NULL ) {
		delete _outqueue ;
		_outqueue = NULL ;
	}
}

// ȡ����������
CSockManager::_node * CSockManager::connect_t::readlist( void )
{
	CSockManager::_node *p = NULL;

	_mutex.lock() ;
	if ( _inqueue->size() <= 0 ) {
		_mutex.unlock() ;
		return NULL ;
	}
	p = _inqueue->getnodes() ;
	_inqueue->reset() ;
	_mutex.unlock() ;

	return p ;
}

// �Ƿ��Ѿ��ͷ�����
bool CSockManager::connect_t::close( void )
{
	if ( _fd <= 0 ) {
		return false  ;
	}
	_status = SOCKET_CLOSE ;
	return true ;
}

// ��������
int  CSockManager::connect_t::write( int &nerr )
{
	if ( _fd <= 0 || _status != SOCKET_LIVE ) {
		return SOCKET_FAILED ;
	}
	// ����Ҫд���������Ƶ���Ҫ���͵�������
	_mutex.lock() ;
	_outqueue->moveto( _inqueue ) ;
	_mutex.unlock() ;

	if ( _outqueue->size() == 0 ) {
		// ���д�¼�����
		_mutex.lock() ;
		_eventer->modify( this, ReadableEvent ) ;
		_mutex.unlock() ;

		return SOCKET_SUCCESS ;
	}

	// ��¼д�����ݵĳ���
	int wsize = 0 , nsize = 0 ;
	_node *p = _outqueue->popnode() ;
	while( p != NULL && _status == SOCKET_LIVE ) {
	// printf( "send fd %d length %d data %s\n", fd,  p->_len , (const char *) p->_dptr ) ;
		int sendlen = UtilitySocket::BaseSocket::write( _fd, (char*)p->_dptr, p->_len );
		if ( sendlen == p->_len ) {
			// printf( "socket fd %d size %d send length %d\n", pitem->_fd, pitem->_size, sendlen ) ;
			free(p->_dptr);
			free(p);

			// �ۼ�д�������
			wsize = wsize + sendlen ;
			// ȡ�õȴ��������ݶ��г���
			nsize = _outqueue->size() ;
			// ���д������ݴ��ڻ�������С��ȴ���һ��д�¼�������
			if ( wsize < MAX_SOCKET_BUF ) {
				// �������������Ҫ������Ӧ�ü�������
				if ( nsize > 0 ) {
					p = _outqueue->popnode() ;
					continue ;
				}
			}

			// ���һ���Ƿ���������Ҫ����
			_mutex.lock() ;
			// �����г����ۼӴ���
			nsize = nsize + _inqueue->size() ;
			if ( nsize == 0 ) { // ���û�����ݾͽ�д�¼����
				_eventer->modify( this, ReadableEvent ) ;
			}
			_mutex.unlock() ;

			// �����������Ҫ���;ͷ�����Ҫ���͵����ݸ���
			return ( nsize > 0 ) ? nsize: SOCKET_SUCCESS ;

		} else if(sendlen > 0 && sendlen < p->_len) {
			// ������Ͳ������ݳ����У����������
			p->_len = p->_len - sendlen ;
			// �ƶ����ͳ�ȥ������
			memmove( p->_dptr, p->_dptr+sendlen, p->_len ) ;
			// �Żص�ͷ��
			_outqueue->addhead( p ) ;

			// printf( "socket fd %d size %d send length %d\n", pitem->_fd, pitem->_size, sendlen ) ;
			nerr = errno ;

			// ����û�з�����ɣ���������
			//return SOCKET_CONTINUE ;
			return SOCKET_SENDBUFFER ;

		} else if (sendlen < 0) {
			// �������ʧ��ֱ����ӻض��д���
			_outqueue->addhead( p ) ;
			nerr = errno ;
			if ( errno == EWOULDBLOCK || errno == EAGAIN ){
				//����������
				OUT_WARNING( _szIp , _port , NULL, "Send EAGAIN, send buffer is full" ) ;
				// ��������
				// return SOCKET_CONTINUE ;
				return SOCKET_EWOULDBLOCK ;
			}
			_status = SOCKET_WAITCLOSE ;
			// �յ�����������
			return SOCKET_DISCONN ;
		}
		p = _outqueue->popnode() ;
	}

	// ���д�¼�
	_mutex.lock() ;
	_eventer->modify( this, ReadableEvent ) ;
	_mutex.unlock() ;

	return SOCKET_SUCCESS ;
}

// ��ȡ����
struct packet * CSockManager::connect_t::read( int &ret , int &nerr, IPackSpliter *pack )
{
	struct packet *pdata = NULL ;

	ret = SOCKET_SUCCESS ;

	if ( _fd <= 0 || _status != SOCKET_LIVE ) {
		ret = SOCKET_FAILED ;
		return NULL ;
	}

	while( true ) {
		// ȷ�����㹻�Ŀռ�
		_read_buff->ensureFree( READ_BUFFER_SIZE ) ;
		// ���õĳ���
		int nfreelen = _read_buff->getFreeLen() ;
		// ��ȡ�����������
		int recvlen = UtilitySocket::BaseSocket::read( _fd, _read_buff->getFree(), nfreelen ) ;
		if( recvlen > 0 ) {
			// LOGDEBUG( NULL, 0 , "recv fd %d length %d data %s\n", fd, recvlen, (const char *) buffer ) ;
			// ��������ָ��
			_read_buff->pourData( recvlen ) ;
			// ������յ�������Ϊ�����������ͼ�����һ��
			if ( recvlen == nfreelen ) {
				// �������˵���������л�������û�н������
				// ret = SOCKET_CONTINUE ;
				// return pdata ;
				if ( _read_buff->getLength() < MAX_SOCKET_BUF ) continue ;
			}

			// �������
			pdata = pack->get_kfifo_packet( _read_buff ) ;

		} else if( recvlen == 0 ) {

			nerr 	= errno ;
			ret  	= SOCKET_DISCONN ;
			_status = SOCKET_WAITCLOSE ;
			return NULL ;

		} else if(recvlen < 0) {

			nerr = errno ;
			if ( errno == EWOULDBLOCK || errno == EAGAIN ) {
				// OUT_WARNING( _szIp , _port, NULL, "RECV EWOULDBLOCK, recv buffer is empty" );
				ret = SOCKET_SUCCESS ;
				return NULL ;
			}

			_status = SOCKET_WAITCLOSE ;
			ret = SOCKET_RECVERR ;

			return NULL ;
		}

		// ֻ�������ɹ���Чʱ��ĸ���
		_last = time(NULL) ;

		return pdata ;
	}
	return pdata ;
}

// �����Ҫ���͵�����
bool CSockManager::connect_t::deliver( const char *buf, const int len )
{
	_mutex.lock() ;
	// �����Ϊ���״̬��ֱ�ӷ�����
	if ( _status != SOCKET_LIVE || len <= 0 ) {
		_mutex.unlock() ;
		OUT_ERROR( _szIp, _port , "Env", "fd %u, message len %d, socket state %d", _fd, len, _status ) ;
		return false ;
	}

	bool success = _inqueue->addtail( buf, (int) len ) ;
	if ( ( !(_events & WritableEvent ) ) && success ) {
		// ��������ݾͽ�״̬��Ϊ������Ҫд
		_eventer->modify( this,  ReadableEvent|WritableEvent ) ;
	}
	_mutex.unlock() ;
	// ��ӡ����д������־
	if ( ! success ) {
		OUT_ERROR( _szIp, _port, "Env", "fd %d, inqueue over max send length" , _fd ) ;
		return false ;
	}
	return true ;
}

// ����IP�Ͷ˿�
void CSockManager::connect_t::init( unsigned int fd, const char *szip , unsigned short port )
{
	_fd    = fd ;
	_port  = port ;
	if ( szip != NULL ) {
		// ���IP��ַ
		strcpy( _szIp , szip ) ;
	}
	_status = SOCKET_LIVE;
	// �����ԭ����������
	_read_buff->resetBuf() ;

	// ����Ҫд���������Ƶ���Ҫ���͵�������
	_mutex.lock() ;
	_inqueue->clear() ;
	_inqueue->addref() ;
	_mutex.unlock() ;
}

// �Ƿ�ʱû���α�����
bool CSockManager::connect_t::check( void )
{
	return ( _status == SOCKET_LIVE ) ;
}

//////////////////////////////////////////// ���ӹ�������ظ�ʹ���ڴ�  /////////////////////////////////////////
// ǩ������
socket_t * CSockManager::get( int sockfd, const char *ip, unsigned short port , bool queue )
{
	connect_t *p = NULL ;

	_mutex.lock() ;
	p = (connect_t *) _queue.pop() ;

	if ( p == NULL ) {
		p = new connect_t(_eventer) ;
	}
	p->_type = FD_TCP ;
	p->init( sockfd, ip, port ) ;
	p->_last = time(NULL) ;  // ���һ��ʹ��ʱ��
	p->_ptr  = NULL ; // ��������չ����
	p->_next = NULL ;
	p->_pre  = NULL ;

	// ������Դ���Ƿ���Ҫ���̶߳���ͳһ����
	if ( queue ) {
		// ���û���ӵ����߶����й���
		_online.push( p ) ;
		_index.insert( std::set<socket_t*>::value_type(p) ) ;
	}
	_mutex.unlock() ;

	return p ;
}

// ǩ������
void CSockManager::put( socket_t *fd )
{
	_mutex.lock() ;
	// �ȴ����߶��������������
	std::set<socket_t*>::iterator it = _index.find( fd ) ;
	if ( it != _index.end() ) {
		_index.erase( it ) ;
		_online.erase( fd ) ;
	}
	_queue.push( fd ) ;
	_mutex.unlock() ;
}

// ����������Դ
void CSockManager::clear( void )
{
	_mutex.lock() ;
	_index.clear() ;
	_mutex.unlock() ;
}

// �ر�����
bool CSockManager::close( socket_t *sock )
{
	_mutex.lock() ;
	// �ȴ����߶��������������
	std::set<socket_t*>::iterator it = _index.find( sock ) ;
	if ( it == _index.end() ) {
		_mutex.unlock() ;
		return false ;
	}
	_index.erase( it ) ;

	_online.erase( sock ) ;
	_recyle.push( sock ) ;
	_mutex.unlock() ;

	return true ;
}

// ȡ�����л������Ӷ���
socket_t * CSockManager::recyle()
{
	socket_t *p = NULL ;
	_mutex.lock() ;
	int size = 0 ;
	p = _recyle.move( size ) ;
	if ( size == 0 ) {
		p = NULL ;
	}
	_mutex.unlock() ;
	return p ;
}

// ��ⳬʱ���Ӷ���
int CSockManager::check( int timeout, std::list<socket_t*> &lst )
{
	int count = 0 ;

	_mutex.lock() ;
	if ( _online.size() == 0 ) {
		_mutex.unlock() ;
		return false ;
	}

	time_t now = time(NULL) - timeout ;
	socket_t *p = _online.begin() ;
	while( p != NULL ) {
		if ( now > p->_last ) {
			lst.push_back( p ) ;
			++ count ;
		}
		p = p->_next ;
	}
	_mutex.unlock() ;

	return count ;
}

/////////////////////////////////////// ������������CTcpHandle���� //////////////////////////////////////////

CTcpHandle::CTcpHandle():
	_tcp_init(false) , _pack_spliter(&_defaultspliter), _socktimeout(SOCKET_TIMEOUT)
{
	_server_fd   = NULL ;
	// ��ÿ���̷߳��䲻ͬ�������ݶ���
	_packqueue   = new CDataQueue<CPacket>(MAXQUEUE_LENGTH);
	_queuethread = new CQueueThread( _packqueue, this ) ;
	_socketmgr   = new CSockManager(this) ;
}

CTcpHandle::~CTcpHandle()
{
	uninit() ;
	// �Żض���,�����Զ���������,������Ϊ��ʱ�Զ����ն���
	if ( _socketmgr != NULL ) {
		delete _socketmgr ;
		_socketmgr = NULL ;
	}

	if ( _queuethread != NULL ) {
		delete _queuethread ;
		_queuethread = NULL ;
	}
	// �ͷ����ݶ���
	if ( _packqueue != NULL ) {
		delete _packqueue ;
		_packqueue = NULL ;
	}
	if ( _server_fd != NULL ) {
		delete _server_fd ;
		_server_fd = NULL ;
	}
}

//��ʼ���̳߳أ��첽socket��ʹ��ǰ������øú���
//windows��һ��Ҫ����5000����Ϊwindows�׽��ֲ��Ǵ�1��ʼ�ģ���Ҫ����
bool CTcpHandle::init( unsigned int nthread , unsigned int timeout )
{
	FUNTRACE("bool CTcpHandle::init(int max_fd /*= MAX_SOCKET_NUM*/)");

	/*
	* ����ÿ����������򿪵�����ļ���
	*/
#ifndef _UNIX
	struct rlimit rt;
	rt.rlim_max = rt.rlim_cur = MAX_FD_OFFSET ;
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
	{
		perror("setrlimit\n");
	}
#endif

	destroy();

	bool ret = create(MAX_FD_OFFSET);
	if ( !ret )
	{
		OUT_ERROR(NULL, 0, NULL, "CEpollHandle::create(max_fd:%d);", MAX_FD_OFFSET );
		return ret;
	}

	// ���TCP���ӳ�ʱʱ��
	_socktimeout  = timeout ;
	_tcp_init 	  = true ;
	// ���̴߳���ģ��
	if ( ! _thread_conn.init( 1 , this, this ) ){
		OUT_ERROR( NULL, 0, NULL, "CTcpHandle::init connect thread failed" ) ;
		return false ;
	}
	// ������ӳ�ʱ����
	if ( ! _thread_check.init( 1, NULL, this ) ) {
		OUT_ERROR( NULL, 0, NULL, "CTcpHandle::init connect check thread failed" ) ;
		return false ;
	}
	// ���ݴ����߳�
	if ( !_queuethread->Init(nthread) ) {
		OUT_ERROR( NULL, 0, NULL, "CTcpHandle::init data queue thread failed" ) ;
		return false ;
	}
	_thread_conn.start() ;
	_thread_check.start() ;

	return true ;
}

bool CTcpHandle::uninit()
{
	FUNTRACE("bool CTcpHandle::uninit_fds()");

	if ( ! _tcp_init )
		return false ;

	_tcp_init = false ;
	_thread_check.stop() ;
	_thread_conn.stop() ;
	_queuethread->Stop() ;

	if ( _server_fd != NULL ) {
		::close( _server_fd->_fd ) ;
	}

	destroy();

	return true ;
}

bool CTcpHandle::start_server( int port , const char* ip )
{
	FUNTRACE("bool CTcpHandle::start_server(int port, const char* ip, int max_fd = MAX_SOCKET_NUM)");

	bool ret = false;

	if ( !_tcp_init ){
		OUT_ERROR( NULL, 0, NULL, "CTcpHandle::start_server not init" ) ;
		return false;
	}
	// ����˿�Ϊ����Ϊ�ͻ���
	if ( port == 0  ) return true ;

	int fd = UtilitySocket::BaseSocket::socket() ;
	if ( fd == -1 )
	{
		OUT_ERROR(NULL,0,NULL,"BaseSocket::socket:%s",strerror(errno));
		return false;
	}

	UtilitySocket::BaseSocket::setNonBlocking(fd) ;
	UtilitySocket::BaseSocket::setReuseAddr(fd) ;
	UtilitySocket::BaseSocket::setLinger( fd ) ;
	// UtilitySocket::BaseSocket::setNoCheckSum( _server_fd ) ;
	UtilitySocket::BaseSocket::setRecvBuf( fd, MAX_SOCKET_BUF ) ;
	UtilitySocket::BaseSocket::setSendBuf( fd, MAX_SOCKET_BUF ) ;

	ret = UtilitySocket::BaseSocket::bind( fd , port, ip);
	if ( !ret )
	{
		OUT_ERROR(NULL,0,NULL,"BaseSocket::bind:%s",strerror(errno));
		UtilitySocket::BaseSocket::close( fd );
		return false;
	}

	ret = UtilitySocket::BaseSocket::listen( fd , 10);
	if ( !ret )
	{
		OUT_ERROR(NULL,0,NULL,"BaseSocket::listen:%s",strerror(errno));
		UtilitySocket::BaseSocket::close( fd );
		return false;
	}

	_server_fd = _socketmgr->get( fd , ( ip == NULL ) ? "0.0.0.0" : ip , port , false ) ;
	if ( _server_fd == NULL ) {
		OUT_ERROR(NULL,0,NULL,"init server fd object failed" );
		UtilitySocket::BaseSocket::close( fd );
		return false;
	}
	add(_server_fd, ReadableEvent/*|WritableEvent*/);//

	OUT_PRINT(NULL, 0, NULL, "tcp, start_server ok, fd:%d", _server_fd);
	INFO_PRT("CTcpHandle start server %s:%d.........success",ip,port);

	return true;
}

bool CTcpHandle::stop_server()
{
	FUNTRACE("bool CTcpHandle::stop_server()");

	UtilitySocket::BaseSocket::close(_server_fd->_fd);

	return true;
}

//��Ӧ�ò�����Ͷ�ݵ�fd���������ȴ��̳߳صĵ��ȷ��͡�
bool CTcpHandle::deliver_data( socket_t *sock, const void* data, int len )
{
	if ( sock == NULL ) return false ;
	// ��ӵ��������ݶ�����
	return ((CSockManager::connect_t*)sock)->deliver( (const char*)data, len ) ;
}

//����on_disconnection
void CTcpHandle::close_socket( socket_t *sock , bool notify )
{
	if ( sock == NULL ) return ;
	// ������У�Ȼ�����ɵ��߳�������
	if ( ! _socketmgr->close( sock ) ){
		return ;
	}
	/**
	// ��������ǰû�з��ͳ�ȥ������
	CSockManager::_node *p = ((CSockManager::connect_t*)sock)->readlist() ;
	if ( p != NULL ) {
		CSockManager::_node *tmp ;
		while( p != NULL ) {
			tmp = p ;
			p   = p->_next ;
			if ( tmp->_dptr != NULL ) {
				// ����ر���ǰû�з��ͳ�ȥ������
				on_send_failed( sock, tmp->_dptr, tmp->_len ) ;
				free(tmp->_dptr) ;
			}
			free(tmp) ;
		}
	}*/

	// �ص�����
	if ( notify )
		on_dis_connection( sock ) ;
}

//������on_disconnection
bool CTcpHandle::close_fd( socket_t *sock )
{
	//�ŵ������棬����������
	if ( ! del( sock , ReadableEvent|WritableEvent ) ) {
		OUT_ERROR( sock->_szIp , sock->_port , "ENV", "epoll del fd %d failed" , sock->_fd ) ;
		return false ;
	}

	// �رն���
	if ( !((CSockManager::connect_t*)sock)->close() ) {
		OUT_ERROR( sock->_szIp, sock->_port , "ENV", "fd %d socket manager delete failed", sock->_fd ) ;
		return false ;
	}

	// �ر�SOCKET���ӵĴ���
	UtilitySocket::BaseSocket::close( sock->_fd ) ;
	// ��ӡ��־��¼�Ͽ�����
	OUT_INFO( sock->_szIp , sock->_port , "ENV", "close_socket fd:%d closed", sock->_fd );

	// ����Ͽ�������
	return true ;
}

// �����¼�����
void CTcpHandle::on_event( socket_t *sock, int events )
{
	if( is_read(events) )
	{
		//�����ӵ���
		if( sock == _server_fd ) {
			sockaddr_in sa;

			while(1)  { //������ͬʱ�ж��������������Ҫ��ѯ
				memset(&sa, 0, sizeof(sa));
				socklen_t salen = sizeof(sa);
				int client = accept(_server_fd->_fd, (struct sockaddr *)&sa, &salen);
				if( client < 0 ) {
					// OUT_ERROR(NULL, 0, NULL, "accept, errno:%d, %s, ret is %d", errno, strerror(errno), client);
					return;
				}

				const  char* ip     = inet_ntoa(sa.sin_addr); // 117.136.26  221.7.7
				unsigned short port = ntohs(sa.sin_port);
				// �Ƿ���Խ����IP�Ļ���IP��
				if ( ! invalidate_ip( ip ) ) {
					// ��IP��������
					OUT_ERROR( ip, port, "ENV", "in black ip list fd %d", client ) ;
					UtilitySocket::BaseSocket::close( client ) ;
					return ;
				}
				OUT_CONN( ip , port, "ENV", "one new connection come from %s, port %d, fd is %d", ip, port, client ) ;

				UtilitySocket::BaseSocket::setNonBlocking(client);

				socket_t *csock = _socketmgr->get( client , ip , port ) ;
				if ( csock == NULL ) {
					OUT_ERROR( ip , port , "ENV", "connection over max fd , fd %d" , client ) ;
					UtilitySocket::BaseSocket::close(  client ) ;
					return ;
				}

				// ����ӵ��¼�������
				if( ! add( csock , ReadableEvent/*|WritableEvent*/) ) {
					OUT_ERROR( ip, port, "ENV", "add epoll ctrl failed , fd %d" , client ) ;
					_socketmgr->put( csock ) ;
					UtilitySocket::BaseSocket::close( client ) ;
					return ;
				}
				// ��ӹ���Զ�����
				on_new_connection( csock, ip, port ) ;
			}

		} else { //socket�����ݵ�����
			// printf("tcp fd:%d can be read!\n", fd);
			read_data( sock ) ;
		}
	}

	//֪ͨSocket fd ��д��
	if(  is_write(events) ) { //��TCP���ͻ�������ʱ���˴�ִ�еؿ��ܻ�Ƚ�Ƶ����
		//LOGDEBUG(NULL, 0, "tcp fd:%d can be written!", fd);
		write_data( sock ) ;
	}
}

// �������ݻص��ӿ�
void CTcpHandle::HandleQueue( void *packet )
{
	CPacket *tmp = ( CPacket *) packet ;
	struct list_head *head = (struct list_head *) tmp->_pack ;
	struct packet* one;
	struct list_head *q, *next;

	list_for_each_safe(q, next, head) {
		one = list_entry(q, struct packet, list);
		// �������ݰ�����������
		on_data_arrived( (socket_t*)tmp->_sock , (void *)one->data, (int) one->len ) ;
		_pack_spliter->free_kfifo_packet( one ) ;
	}
	free( head ) ;
	tmp->_pack = NULL ;
}

// ������
void CTcpHandle::read_data( socket_t *sock )
{
	int err = 0 , ret = 0 ;
	CSockManager::connect_t *conn = ( CSockManager::connect_t *)sock ;
	do {
		struct packet *lst = conn->read( ret, err, _pack_spliter ) ;
		if ( lst == NULL ) {
			continue ;
		}
		CPacket *pack = new CPacket( sock, lst ) ;
		if ( ! _queuethread->Push( pack ) ) {
			delete ( pack ) ;
		}
	} while( ret == SOCKET_CONTINUE ) ;

	if ( ret != SOCKET_SUCCESS ){
		write_errlog( sock, ret, err ) ;
		close_socket( sock ) ;
	}
}

// д����
bool CTcpHandle::write_data( socket_t *sock )
{
	int err = 0 ;
	CSockManager::connect_t *conn = ( CSockManager::connect_t *) sock ;
	// printf( "begin write fd %d\n", fd ) ;
	int ret = conn->write( err ) ;
	while ( ret == SOCKET_CONTINUE ) {
		ret = conn->write( err ) ;
	}
	if ( ret < SOCKET_SUCCESS ){
		write_errlog( sock, ret, err ) ;
		close_socket( sock ) ;
	}
	return ( ret == SOCKET_SUCCESS ) ;
}

void CTcpHandle::write_errlog( socket_t *sock, int err , int nerr )
{
	const char *serr = "Unknow Error" ;
	switch( err ) {
		case SOCKET_FAILED:
			serr = "Send Data Failed" ;
			break;
		case SOCKET_DISCONN:
			serr = "Recv Disconnection" ;
			break;
		case SOCKET_RECVERR:
			serr = "Recv data error" ;
			break;
		case SOCKET_SENDERR:
			serr = "Send data error" ;
			break;
	}
	OUT_ERROR( sock->_szIp , sock->_port , "ENV", "%s, tcp fd %d, errno %d, errmsg %s" , serr, sock->_fd , nerr , strerror(nerr) ) ;
}

// ��������״����߳�
void  CTcpHandle::process_check( void )
{
	time_t last_check = time(NULL) ;
	while (_tcp_init ){
		// ����״̬����߳�
		time_t now = time(NULL) ;
		if ( now - last_check < SOCKET_CHECK ) {
			sleep(3) ;
			continue ;
		}
		last_check = now ;

		std::list<socket_t *> lst ;
		int count = _socketmgr->check( _socktimeout, lst ) ;
		if ( count == 0 )  {
			sleep(3) ;
			continue ;
		}

		std::list<socket_t *>::iterator it ;
		for ( it = lst.begin(); it != lst.end(); ++ it ) {
			close_socket( *it ) ;
		}
		lst.clear() ;

		sleep(3) ;
	}
}

// �����û����������߳�
void  CTcpHandle::process_data( void )
{
	while (_tcp_init ){
		// �����ӹ����̶߳��󣬴����������ݵĽ���
		int n = poll(1);
		if ( n < 0 ){
			usleep(5*1000*1000);
		}

		// ������Ҫ�رյ�����
		socket_t *p = _socketmgr->recyle() ;
		if ( p != NULL ) {
			socket_t *tmp = p  ;
			while( p != NULL ) {
				tmp = p ;
				p   = p->_next ;

				close_fd( tmp ) ;

				_socketmgr->put( tmp ) ;
			}
		}
	}
}

// �߳����ж���
void CTcpHandle::run( void *param )
{
	if ( param == this ) {
		process_data() ;
	} else {
		process_check() ;
	}
}

//����fd
socket_t * CTcpHandle::connect_nonb(const char* ip, int port, int nsec )
{
	int sockfd = -1 , ret = 0 ;

	if((sockfd=UtilitySocket::BaseSocket::socket(SOCK_STREAM))<0){
		perror("socket error:");
		return NULL ;
	}

	// ��ȡ��ǰsocket�����ԣ� ������ noblocking ����
	errno = 0;

	UtilitySocket::BaseSocket::setNonBlocking( sockfd );
	// UtilitySocket::BaseSocket::setNoCheckSum( sockfd ) ;

	if ( UtilitySocket::BaseSocket::connect( sockfd, ip , port ) )
		goto done ;

	errno = UtilitySocket::BaseSocket::getError() ;
	if ( errno != EINPROGRESS ) {
		OUT_ERROR(ip,port,NULL,"connect:%s",strerror(errno) );
		UtilitySocket::BaseSocket::close( sockfd ) ;
		return NULL ;
	}

	struct pollfd fds[1];
	memset(fds, 0 , sizeof(fds));
	fds[0].fd 	  = sockfd ;
	fds[0].events = POLLOUT | POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLPRI;

	ret = ::poll(fds, 1, nsec*1000 ) ;
	// �������κ������Ĳ���
	if ( ret > 0 ) {
		// Ensure the socket is connected and that there are no errors set
		int val;
		socklen_t lon = sizeof(int);
		int ret2 = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&val, &lon);
		if (ret2 == -1) {
		  OUT_ERROR( ip, port, "CONN", "CTcpHandle::connect_nonb() getsockopt() errno %d, %s" , errno, strerror(errno) );
		}
		// no errors on socket, go to town
		if (val == 0) {
		  goto done;
		}
		OUT_ERROR( ip, port , "CONN","CTcpHandle::connect_nonb() error on socket (after poll) , errno %d, %s" , errno, strerror(errno) ) ;
	} else if ( ret == 0) {
		// socket timed out
		OUT_ERROR( ip, port, "CONN","CTcpHandle::connect_nonb() timed out ,errno %d, %s" , errno, strerror(errno) ) ;
	} else {
		// error on poll()
		OUT_ERROR( ip, port, "CONN","CTcpHandle::connect_nonb() poll() , errno %d, %s" , errno, strerror(errno) ) ;
	}

	UtilitySocket::BaseSocket::close( sockfd ) ;
	return  NULL;

done:
	socket_t *sock = _socketmgr->get( sockfd, ip, port ) ;
	if ( sock == NULL ) {
		printf( "Connect to server OK, fd:%d, init fd failed\n" , sockfd ) ;
		UtilitySocket::BaseSocket::close( sockfd ) ;
		return NULL;
	}
	OUT_PRINT( sock->_szIp, sock->_port, NULL, "Connect to TCP server OK, fd:%d", sockfd );
	add( sock , ReadableEvent/*|WritableEvent*/ ) ;

	return sock ;
}
