#include "UdpHandle.h"
#include <list.h>
#include <assert.h>
#include <UtilitySocket.h>
#include <protocol.h>
#include <errno.h>
#include <arpa/inet.h>
#include <poll.h>
#include <comlog.h>

#define MAXQUEUE_LENGTH   102400
///////////////////////////// CConnection //////////////////////////////
// ��ʼ��ϵͳ����
void CUdpSockManager::udpconnect_t::beginsock( bool bkfifo  , bool breset )
{
	_fd 	 = 0;
	_size    = 0;
	_type    = FD_UDP ;

	if ( bkfifo )  {
		if ( _read_buff != NULL ) {
			delete  _read_buff ;
		}
		_read_buff = new DataBuffer;
	}

	if ( breset && _read_buff ) {
		// ��������λ��
		_read_buff->resetBuf() ;
	}
}

// ��������
void CUdpSockManager::udpconnect_t::endsock( bool bkfifo )
{
	if ( bkfifo ) {
		if ( _read_buff != NULL ) {
			delete _read_buff ;
		}
		_read_buff = NULL ;
	}
}

// ����IP�Ͷ˿�
void  CUdpSockManager::udpconnect_t::init( int fd, const char *szip, unsigned short port , int ctype )
{
	_fd    = fd ;
	_port  = port ;
	_ctype = ctype ;
	_read_buff->resetBuf() ;

	if ( szip != NULL ) {
		// ���IP��ַ
		strcpy( _szIp , szip ) ;
	}
}

// ��ȡ����
struct packet * CUdpSockManager::udpconnect_t::split( const char *buf, int len, IPackSpliter *pack )
{
	// �����������Ϊ���̣߳�����Ҫ������
	_read_buff->resetBuf() ;  // UDP��ȡ�����ǰ������ж�ȡ���������ճ����
	_read_buff->writeBlock( buf, len ) ;
	struct packet *plist = pack->get_kfifo_packet(_read_buff);
	if (plist == NULL) {
		// ����δ���������Ҫ��������
		return NULL ;
	}
	return plist ;
}

// д����
int CUdpSockManager::udpconnect_t::write( const char *data, const int len )
{
	char* left_ptr = (char*) data ;
	int left_len   = len;
	int send_len   = 0;

	int send_times = 0 ;

	// ����߳�֮�����ݷ�����Ҫͨ����ͬ������
	_mutex.lock() ;

	// �����Ϊ���������ӷ���
	if ( _ctype == UDP_SERVER_CONN ) {
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family 	  = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(_szIp) ;
		saddr.sin_port 		  = htons(_port);

		while( left_len > 0 ){
			int ret = ::sendto( _fd, left_ptr, left_len, 0,
					(struct sockaddr *)&(saddr), sizeof(struct sockaddr));
			if ( ret == -1 ){ //������
				if ( ++send_times > 2 ){ // ��¼���ͳ���Ĵ���
					send_len = SOCKET_SENDERR ;
					break ;
				}
				continue ;
			}

			left_len -= ret ;
			left_ptr += ret ;
			send_len += ret ;
		}
	} else {// �����Ϊ�ͻ��˷���
		while( left_len > 0 ){
			int ret = ::write( _fd, left_ptr, left_len ) ;
			if ( ret == -1 ){ //������
				if ( ++send_times > 2 ){ // ��¼���ͳ���Ĵ���
					send_len = SOCKET_SENDERR ;
					break ;
				}
				continue ;
			}

			left_len -= ret ;
			left_ptr += ret ;
			send_len += ret ;
		}
	}

	_mutex.unlock() ;

	if ( send_times > 2 ) {
		OUT_ERROR( _szIp, _port, "Send", "fd %d, send failed len %d, errno %d, %s",
				_fd, len, errno, strerror(errno) ) ;
	} else if ( send_len != len ) {
		ERRLOG( _szIp, _port, "Send", "fd %d, send_len:%d != len:%d, errno %d, %s",
				_fd, send_len, len , errno, strerror(errno) ) ;
	}

	return send_len ;
}

// �ر����Ӷ���
bool CUdpSockManager::udpconnect_t::close( void )
{
	if ( _ctype == UDP_CLIENT_CONN ) {
		// printf( "close udp fd socket %d\n", _fd ) ;
		::close( _fd ) ;
		return true;
	}
	return false ;
}

////////////////////////////////////////// CUdpSockManger ////////////////////////////////////////////
// ������Ҫʵ��UDP�����ӹ�����UDP��װ������TCP�����ӹ�����ƣ�ʵ�ֽ��ڲ�ÿһ����������תΪһ�����ӣ�
CUdpSockManager::CUdpSockManager()
{
}

CUdpSockManager::~CUdpSockManager()
{
	clear() ;
}

// �ְ�����
socket_t * CUdpSockManager::recv( int server_fd, CUdpHandle *handle , int &ret, int &err, IPackSpliter *pack )
{
	udpconnect_t *sock = NULL ;

	struct packet *pdata = NULL ;
	//�������ݱ����׽ӿڣ������е�һ�����ݱ��е����ݱ����������಻�����������Ĵ�С��
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	// ���﷢��UDP���ݷֶ�ν��վͻ���ֶ��������
	int n = recvfrom( server_fd, (char*)_szbuf, READ_BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len );
	// printf( "recv length %d\n" , n ) ;
	const char *remote_ip      = inet_ntoa(cliaddr.sin_addr) ;
	unsigned short remote_port = ntohs(cliaddr.sin_port);

	sock = (udpconnect_t*) get( server_fd , remote_ip, remote_port , UDP_SERVER_CONN ) ;

	if( n > 0 ){
		// ��ӵ����ݴ��������
		pdata = sock->split( _szbuf , n , pack ) ;
		ret = ( ( n == READ_BUFFER_SIZE ) ? SOCKET_CONTINUE : SOCKET_SUCCESS ) ;

	} else {  // ����ʧ��û��ʲô������Ҫ���������

		if ( n == 0 ) { // ���յ��Ͽ�����������
			OUT_ERROR( remote_ip, remote_port , "DATA" , "recv disconnection request, fd %d", server_fd ) ;
			ret = SOCKET_DISCONN ;
		} else if ( errno == EWOULDBLOCK ){ // ���û�пɽ��������򷵻سɹ�
			ret = SOCKET_SUCCESS ;
		} else {
			err = errno ;
			// �������ݴ���
			OUT_ERROR( remote_ip, remote_port , "DATA" , "data error, disconnection request, fd %d", server_fd ) ;
			// �������ݴ���
			ret = SOCKET_RECVERR ;
		}
	}

	// �ŵ��������������ݴ���ص�����ֹ�ⲿ���ý������������
	if ( pdata != NULL ) {
		// �����յ����ݷ��뵽������
		CPacket *pack = new CPacket( sock , pdata ) ;
		if ( ! handle->_queuethread->Push(pack) ) {
			delete pack ;
		}
		// ֻ������Ч��ʱ������ݰ�
		sock->_last = time(NULL) ;
	}

	return sock ;
}

// ǩ������
socket_t * CUdpSockManager::get( int sockfd, const char *ip, unsigned short port , int ctype , bool queue )
{
	udpconnect_t *p = NULL ;

	_mutex.lock() ;

	char szid[256] = {0} ;
	sprintf( szid, "%d_%s_%d", sockfd, ip, port ) ;

	std::map<std::string,socket_t*>::iterator it = _mpsock.find( szid ) ;
	if ( it != _mpsock.end() ) {
		p = (udpconnect_t*)it->second ;
		_mutex.unlock() ;
		return p ;
	}

	p = (udpconnect_t*)_queue.pop() ;
	if ( p == NULL ) {
		p = new udpconnect_t ;
	}
	p->_type = FD_UDP ;
	p->init( sockfd, ip, port , ctype ) ;
	p->_last = time(NULL) ;  // ���һ��ʹ��ʱ��
	p->_ptr  = NULL ; // ��������չ����
	p->_next = NULL ;
	p->_pre  = NULL ;

	// ��Է�������FD��Դ�����Լ�����
	if ( queue ) {
		_online.push( p ) ;
		_index.insert( std::set<socket_t*>::value_type(p) ) ;
		_mpsock.insert( std::make_pair( szid, p ) ) ;
	}
	_mutex.unlock() ;

	return p ;
}

// ǩ������
void CUdpSockManager::put( socket_t *fd )
{
	_mutex.lock() ;

	std::set<socket_t*>::iterator it = _index.find( fd ) ;
	if ( it != _index.end() ) {
		_index.erase( it ) ;
		_online.erase( fd ) ;
	}

	char szid[256] = {0} ;
	sprintf( szid, "%d_%s_%d", fd->_fd, fd->_szIp, fd->_port ) ;
	_mpsock.erase( szid ) ;

	_queue.push( fd ) ;

	_mutex.unlock() ;
}

// ����������Դ
void CUdpSockManager::clear( void )
{
	_mutex.lock() ;
	_index.clear() ;
	_mpsock.clear() ;
	_mutex.unlock() ;
}

// �ر�����
bool CUdpSockManager::close( socket_t *sock )
{
	_mutex.lock() ;
	std::set<socket_t*>::iterator it = _index.find( sock ) ;
	if ( it == _index.end() ) {
		_mutex.unlock() ;
		return false ;
	}
	_index.erase( it ) ;
	_online.erase( sock ) ;

	char szid[256] = {0} ;
	sprintf( szid, "%d_%s_%d", sock->_fd, sock->_szIp, sock->_port ) ;
	_mpsock.erase( szid ) ;

	bool remove = ((udpconnect_t *) sock )->close() ;

	_queue.push( sock ) ;
	_mutex.unlock() ;

	return remove ;
}

// ��ⳬʱ���Ӷ���
int CUdpSockManager::check( int timeout, std::list<socket_t*> &lst )
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

////////////////////////////////////////////////// UdpHandle�������  ////////////////////////////////////////////////////

CUdpHandle::CUdpHandle() :
		_udp_init(false) , _pack_spliter(&_defaultpacker), _socktimeout(SOCKET_TIMEOUT)
{
	_server_fd   = NULL ;
	_packqueue   = new CDataQueue<CPacket>(MAXQUEUE_LENGTH);
	_queuethread = new CQueueThread( _packqueue, this ) ;
	// ��ʼ��SockManager����
	_socketmgr   = new CUdpSockManager ;
}

CUdpHandle::~CUdpHandle()
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
bool CUdpHandle::init( unsigned int nthread , unsigned int timeout )
{
	if ( _udp_init )
		return true;

	// ����EPOLL���
	if ( !create(MAX_FD_OFFSET) ) {
		OUT_ERROR(NULL, 0, NULL, "create(max_fd:%d) failed", MAX_FD_OFFSET );
		return false ;
	}

	_udp_init    = true ;
	_socktimeout = timeout ;
	// ��ʼ���Ӵ����̣߳�����Ҳʹ�õ��̶߳���������
	if ( ! _thread_conn.init( 1 , this , this ) ){
		OUT_ERROR( NULL, 0, NULL, "CUdpHandle::init connect thread failed") ;
		return false ;
	}
	// ��������״̬����߳�
	if ( ! _thread_check.init( 1, NULL, this ) ) {
		OUT_ERROR( NULL, 0, NULL, "CUpdHandle::init connect check thread failed" ) ;
		return false ;
	}
	// ���ݴ����߳�
	if ( !_queuethread->Init( nthread ) ) {
		OUT_ERROR( NULL, 0, NULL, "CUdpHandle::init data queue thread failed") ;
		return false ;
	}
	_thread_conn.start() ;
	_thread_check.start() ;

	return true ;
}

// ֹͣ�̣߳���������
bool CUdpHandle::uninit()
{
	if ( ! _udp_init )
		return false ;

	_udp_init = false ;
	_thread_check.stop() ;
	_thread_conn.stop() ;
	_queuethread->Stop() ;

	if ( _server_fd != NULL ) {
		// �ر�UDP������������
		::close( _server_fd->_fd ) ;
	}
	// ����epoll����
	destroy();

	return true ;
}

bool CUdpHandle::start_server( int port , const char* ip  )
{
	if ( !_udp_init ){
		OUT_ERROR( NULL, 0, NULL, "CUdpHandle::start_server not init" ) ;
		return false;
	}
	// ��Ϊ�ͻ��˴���
	if ( port == 0  ) return true ;

	int fd = UtilitySocket::BaseSocket::socket(SOCK_DGRAM);
	if ( fd == -1 )
	{
		ERRLOG(NULL, 0, "BaseSocket::socket(SOCK_DGRAM) error");
		return false;
	}

	UtilitySocket::BaseSocket::setNonBlocking(fd);
	UtilitySocket::BaseSocket::setReuseAddr(fd);
	UtilitySocket::BaseSocket::setRecvBuf( fd, MAX_SOCKET_BUF ) ;
	UtilitySocket::BaseSocket::setSendBuf( fd, MAX_SOCKET_BUF ) ;

	if ( port != 0)
	{
		if ( ! UtilitySocket::BaseSocket::bind( fd, port, ip ) )
		{
			OUT_ERROR(NULL, 0, NULL, "BaseSocket::bind() error");
			UtilitySocket::BaseSocket::close(fd);
			return false;
		}
	}

	_server_fd = _socketmgr->get( fd, ip, port , 0 , false ) ;
	if ( _server_fd == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "udp new server fd failed\n" ) ;
		UtilitySocket::BaseSocket::close(fd);
		return false ;
	}

	if ( ! add(_server_fd, ReadableEvent/*|WritableEvent*/) ){
		OUT_ERROR( NULL, 0, NULL, "udp add epoll ctrl failed\n" ) ;
		UtilitySocket::BaseSocket::close(fd);
		return false ;
	}

	OUT_INFO(NULL, 0, NULL, "udp start_server ok, fd:%d", _server_fd );
	INFO_PRT("CUdpHandle start server %s:%d.........success",ip,port);

	return true;
}

bool CUdpHandle::stop_server()
{
	FUNTRACE("bool CUdpHandle::stop()");

	del(_server_fd, ReadableEvent/*|WritableEvent*/);

	::close( _server_fd->_fd ) ;

	return true ;
}

//��Ӧ�ò�����Ͷ�ݵ�fd���������ȴ��̳߳صĵ��ȷ��͡�
bool CUdpHandle::deliver_data( socket_t *sock , const void* data, int len )
{
	if ( sock == NULL ) {
		return false ;
	}
	return ( ((CUdpSockManager::udpconnect_t*)sock)->write((const char *)data, len ) == len ) ;
}

// �Ͽ����ӣ��Ƿ�ص� on_disconnection��notify��������
void  CUdpHandle::close_socket( socket_t *sock , bool notify )
{
	if ( sock == NULL ) {
		return  ;
	}

	// ����Ͽ�������
	if ( _socketmgr->close( sock ) ) {
		OUT_CONN(  sock->_szIp ,  sock->_port , "CONN" , "close_socket client fd:%d closed.", sock->_fd );
		del( sock, ReadableEvent ) ;
	} else {
		OUT_CONN( sock->_szIp,  sock->_port , "CONN" , "close_socket server fd:%d closed.", sock->_fd );
	}

	// �ص�����
	if ( notify ) {
		on_dis_connection( sock ) ;
	}
}

// �����¼�����
void CUdpHandle::on_event( socket_t *sock , int events )
{
	if( is_read(events) ){ //�����ݵ���
		int err = 0 ;
		int ret = SOCKET_SUCCESS ;
		socket_t *tmp = _socketmgr->recv( sock->_fd , this , ret, err, _pack_spliter ) ;
		// �����������ʧ���������ӹ���
		if ( ret != SOCKET_SUCCESS ){
			// ����ʹ���ڲ���FD
			write_errlog( tmp, ret, err ) ;
			close_socket( tmp ) ;
		}
	}
}

// ��������״����߳�
void CUdpHandle::process_check( void )
{
	time_t last_check = time(NULL) ;
	while ( _udp_init ){
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
void CUdpHandle::process_data( void )
{
	while ( _udp_init ){
		int n = poll(1000);
		if ( n < 0 ) {
			usleep(5*1000*1000);
		}
	}
}

// �߳����ж���
void CUdpHandle::run( void *param )
{
	if ( param == this ) {
		process_data() ;
	} else {
		process_check() ;
	}
}

// �������ݻص��ӿ�
void CUdpHandle::HandleQueue( void *packet )
{
	CPacket *tmp = (CPacket *)packet ;
	struct packet* one;
	struct list_head *q, *next;
	struct list_head *head = (struct list_head *) tmp->_pack;

	list_for_each_safe(q, next, head) {
		one = list_entry(q, struct packet, list);
		// �������ݰ�����������
		on_data_arrived( (socket_t*)tmp->_sock , (void *)one->data, (int) one->len ) ;
		_pack_spliter->free_kfifo_packet( one ) ;
	}
	free( head ) ;
	tmp->_pack = NULL ;
}

void CUdpHandle::write_errlog( socket_t *sock , int err , int nerr )
{
	const char *serr = "UDP Unknow Error" ;
	switch( err ) {
		case SOCKET_FAILED:
			serr = "UDP Send Data Failed" ;
			break;
		case SOCKET_DISCONN:
			serr = "UDP Recv Disconnection" ;
			break;
		case SOCKET_RECVERR:
			serr = "UDP Recv data error" ;
			break;
		case SOCKET_SENDERR:
			serr = "UDP Send data error" ;
			break;
	}

	OUT_ERROR( sock->_szIp, sock->_port, NULL, "%s, udp fd %d, errno %d, errmsg %s" , serr, sock->_fd, nerr , strerror(nerr) ) ;
}


// ���ӷ�����
socket_t * CUdpHandle::connect_nonb(const char* ip, int port, int nsec )
{
	FUNTRACE("int CTcpHandle::connect_nonb1(const char* ip, int port, int nsec /*= 5*/)");

	int sockfd = -1 , ret = 0 ;
	struct sockaddr_in server_addr;

	if((sockfd=UtilitySocket::BaseSocket::socket(SOCK_DGRAM))<0) {
		perror("socket error:");
		return NULL ;
	}

	// ��ȡ��ǰsocket�����ԣ� ������ noblocking ����
	memset( &server_addr, 0, sizeof(server_addr) );
	server_addr.sin_family     	=	PF_INET;
	server_addr.sin_port       	=	htons(port);
	server_addr.sin_addr.s_addr	=	inet_addr(ip);

	int salen = sizeof(struct sockaddr_in);

	UtilitySocket::BaseSocket::setNonBlocking( sockfd );
	// UtilitySocket::BaseSocket::setNoCheckSum( sockfd ) ;
	UtilitySocket::BaseSocket::setRecvBuf( sockfd, MAX_SOCKET_BUF ) ;
	UtilitySocket::BaseSocket::setSendBuf( sockfd, MAX_SOCKET_BUF ) ;

	if ( connect(sockfd, (struct sockaddr*) &server_addr, salen) == 0 )
		goto done ;

	if ( errno != EINPROGRESS ) {
		OUT_ERROR(ip,port,NULL,"connect:%s",strerror(errno) );
		close(sockfd);
		return NULL ;
	}

	struct pollfd fds[1];
	memset(fds, 0 , sizeof(fds));
	fds[0].fd 	  = sockfd ;
	fds[0].events = POLLOUT | POLLIN | POLLERR | POLLHUP | POLLNVAL | POLLPRI ;

	ret = ::poll(fds, 1, nsec*1000 ) ;
	// �������κ������Ĳ���
	if ( ret > 0 ) {
		// Ensure the socket is connected and that there are no errors set
		int val;
		socklen_t lon = sizeof(int);
		int ret2 = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&val, &lon);
		if (ret2 == -1) {
		  OUT_ERROR( ip, port, "CONN", "CUdpSockManager::Connect getsockopt() errno %d, %s" , errno, strerror(errno) );
		}
		// no errors on socket, go to town
		if (val == 0) {
		  goto done;
		}
		OUT_ERROR( ip, port , "CONN","CUdpSockManager::Connect error on socket (after poll) , errno %d, %s" , errno, strerror(errno) ) ;
	}
	else if ( ret == 0) {
		// socket timed out
		OUT_ERROR( ip, port, "CONN","CUdpSockManager::Connect timed out ,errno %d, %s" , errno, strerror(errno) ) ;
	} else {
		// error on poll()
		OUT_ERROR( ip, port, "CONN","CUdpSockManager::Connect poll() , errno %d, %s" , errno, strerror(errno) ) ;
	}

	close( sockfd ) ;
	return NULL ;

done:
	// ȡ���ڲ����Ӻ�
	socket_t *sock = _socketmgr->get( sockfd , ip, port , UDP_CLIENT_CONN ) ;
	if ( sock == NULL ) {
		OUT_ERROR( ip, port, NULL, "get sock object failed" ) ;
		close(sockfd);
		return NULL ;
	}

	OUT_PRINT( sock->_szIp, sock->_port, NULL, "Connect to UDP server OK, fd:%d", sockfd );
	if ( ! add( sock, ReadableEvent/*|WritableEvent*/ ) ) {
		OUT_ERROR( ip, port, NULL, "add event fd %d failed", sockfd ) ;
		_socketmgr->put( sock ) ;
		close( sockfd ) ;
		return NULL ;
	}

	// ת���ڲ���IP����
	return sock ;
}
