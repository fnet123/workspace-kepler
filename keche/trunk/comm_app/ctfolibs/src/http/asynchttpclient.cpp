#include <list.h>
#include "asynchttpclient.h"

//////////////////////////////////////////////////////////////////////////
CAsyncHttpClient::CAsyncHttpClient():_initalized(false)
{
	_maxsize     = 1000 ;
	_seq_id		 = 0 ;
	_pCallbacker = NULL ;
}

CAsyncHttpClient::~CAsyncHttpClient()
{
	Stop() ;
}

// ��ʼ�߳�
bool CAsyncHttpClient::Start( unsigned int nsend , unsigned int nrecv )
{
	// �������ݷְ�����
	_tcp_handle.setpackspliter( this ) ;
	// ��ʼ���߳�
	_tcp_handle.init( nrecv, 60 );

	_initalized = true ;

	if ( !_check_thread.init( nsend , NULL , this ) )
	{
		printf( "start check thread failed, %s:%d", __FILE__, __LINE__ ) ;
		_initalized = false ;
		return false ;
	}
	_check_thread.start() ;

	return true ;
}

// ֹͣ�߳�
void CAsyncHttpClient::Stop( void )
{
	if ( ! _initalized )
		return ;

	_initalized = false ;

	_check_thread.stop() ;

	_tcp_handle.uninit() ;
}

// ����HTTP����ʵ���Ͼ����봦�����
int CAsyncHttpClient::HttpRequest( CHttpRequest& request , unsigned int req_id )
{
	// ����HTTPʵ����Ӧ���������е��ȹ���
	if ( _maxsize > 0 && _list_req.GetQueueSize() >= _maxsize ) {
		// ������������HTTP�������HTTP�ĳ��Ⱦ�ֱ�ӷ�����
		return E_HTTPCLIENT_REQQUEUELENGTH ;
	}

	const char* send_data = NULL ;
	int len = 0 ;

	string server ;
	unsigned short port ;
	send_data = request.GetHTTPRequest( server , port , len ) ;

	if ( send_data == NULL || len == 0 )
		return E_HTTPCLIENT_REQUESTINVALID ;

	if ( strlen( server.c_str() ) == 0 )
		return E_HTTPCLIENT_SERVERINVALID ;

	_REQ_DATA *req = _list_req.AllocReq() ;
	req->_fd   = 0 ;
	req->_seq  = req_id ;
	req->_port = port ;
	req->_ref  = 0 ;
	req->_time = time(NULL) ;
	req->_ip.SetString( server.c_str() ) ;
	req->_senddata.SetString( send_data, len ) ;

	// ��ӵ����������
	_list_req.PushReq( req ) ;

	return E_HTTPCLIENT_SUCCESS ;
}

// �����������ⲿ�߳�������
void CAsyncHttpClient::ProcessWaitReq( void )
{
	_REQ_DATA *req = _list_req.PopReq() ;
	if ( req == NULL ) {
		return ;
	}
	// �޸�HTTP����������ʱ�䣬��Ȼ��HTTP���ö������������ӵȴ�����
	socket_t *sockfd = _tcp_handle.connect_nonb( (const char *)req->_ip , req->_port , 1 ) ;
	// ������ӷ�����ʧ����ֱ�ӷ�����
	if ( sockfd == NULL ) {
		if ( _pCallbacker ) {
			CHttpResponse resp ;
			_pCallbacker->ProcHTTPResponse( req->_seq, E_HTTPCLIENT_SERVERINVALID , resp ) ;
		}
		_list_req.FreeReq( req ) ;
		return ;
	}
	req->_fd = sockfd ;

	_mutex_ref.lock() ;
	sockfd->_ptr = req ;
	++ req->_ref ;
	_mutex_ref.unlock() ;

	// ����HTTP��������
	if ( ! _tcp_handle.deliver_data( sockfd, (const void*) req->_senddata.GetBuffer(), req->_senddata.GetLength() ) ) {
		// ����ʧ���Ƴ�
		if ( _pCallbacker ) {
			CHttpResponse resp ;
			_pCallbacker->ProcHTTPResponse( req->_seq, E_HTTPCLIENT_REQUESTINVALID , resp ) ;
		}
		OUT_ERROR( (const char *)req->_ip , req->_port , "AsynHTTP" , "send data error %s" , req->_senddata.GetBuffer() ) ;

		_mutex_ref.lock() ;
		sockfd->_ptr = NULL ;
		_list_req.FreeReq( req ) ;
		_mutex_ref.unlock() ;

		// �ر�����
		_tcp_handle.close_socket( sockfd ) ;
	}
}

// �����߳�
void CAsyncHttpClient::run( void *param )
{
	while( _initalized ) {
		// ����ȴ�����������
		ProcessWaitReq() ;
	}
}

void CAsyncHttpClient::on_data_arrived(socket_t *sock, const void* data, int len)
{
	_mutex_ref.lock() ;
	// ���յ����ݵ���
	if ( sock->_ptr == NULL ) {
		_mutex_ref.unlock() ;
		return ;
	}

	_REQ_DATA *req = (_REQ_DATA*) sock->_ptr ;
	// ����ر���������Ҫ�����ͷ�
	++ req->_ref ;
	int seqid = req->_seq ;
	sock->_ptr = NULL ;
	// �Ƴ�����
	_list_req.FreeReq( req ) ;

	_mutex_ref.unlock() ;

	CHttpResponse resp ;
	// ����HTTP���������ص�����
	int ret = resp.Parse( (const char *)data , len ) ;
	if ( _pCallbacker ) {
		// �ص��ӿڴ�������
		_pCallbacker->ProcHTTPResponse( seqid , ( ret == HTTPPARSER_ERR_SUCCESS ) ? E_HTTPCLIENT_SUCCESS : E_HTTPCLIENT_FAILED , resp ) ;
	}

	// ���ͳɹ��ر�����
	_tcp_handle.close_socket( sock ) ;
}

// ����ʱ���󸽴���SOCKET���������
void CAsyncHttpClient::on_dis_connection( socket_t *fd )
{
	_mutex_ref.lock() ;
	if ( fd->_ptr == NULL ) {
		_mutex_ref.unlock() ;
		return ;
	}

	_REQ_DATA * p = (_REQ_DATA*) fd->_ptr ;
	if ( --p->_ref <= 0 ) {
		fd->_ptr = NULL ;
		_list_req.FreeReq( p ) ;
	}
	_mutex_ref.unlock() ;
}

// ȡ���������
unsigned int CAsyncHttpClient::GetSequeue( void )
{
	share::Guard guard( _mutex_seq ) ;
	{
		if ( _seq_id == 0xffffffff ) {
			_seq_id = 0 ;
		}
		return ++ _seq_id ;
	}
}

// KIFO��ʲô����ȫ���������洦��
struct packet * CAsyncHttpClient::get_kfifo_packet( DataBuffer *fifo )
{
	int len = fifo->getLength() ;
	if ( len <= 0 || len > MAX_PACK_LEN ){
		fifo->resetBuf() ;
		return NULL;
	}

	char* begin = (char *) fifo->getBuffer() ;
	//printf("%s\n",pDeliverBuffer);
	int ret = CHttpParser::DetectCompleteReq( begin , len ) ;
	if ( ret != HTTPPARSER_ERR_SUCCESS ){
		// �������û����ɣ��������ݴ���ֱ�ӷ�����
		return NULL ;
	}
	// printf( "result code: %d\n" , ret ) ;

	struct list_head *packet_list_ptr = NULL;
	struct packet *item = (struct packet *) malloc(sizeof(struct packet));
	if (item == NULL)
		return (struct packet *) packet_list_ptr;

	item->data = (unsigned char *) malloc( len + 1 );
	memset( item->data, 0, len+1 );
	//!! begin copy data from the second '[' and end with first ']'
	memcpy(item->data, begin, len);
	item->len  = len;
	item->type = E_PROTO_OUT;

	if (packet_list_ptr == NULL){
		packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
		if (packet_list_ptr == NULL)
			return NULL;

		INIT_LIST_HEAD(packet_list_ptr);
	}

	list_add_tail(&item->list, packet_list_ptr);

	fifo->resetBuf() ;
	//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);

	return (struct packet*) packet_list_ptr;
}

////////////////////////////////////// �ȴ���������Ķ���  ///////////////////////////////////////////
CAsyncHttpClient::CWaitListReq::CWaitListReq()
{
	_size     = 0 ;
	_waitsize = 0 ;
}

CAsyncHttpClient::CWaitListReq::~CWaitListReq()
{
	share::Guard guard( _mutex ) ;
	if ( _list_req.empty() ) {
		return ;
	}

	CListReq::iterator it ;
	for ( it = _list_req.begin(); it != _list_req.end(); ++ it ) {
		delete *it ;
	}
	_list_req.clear() ;
}

void CAsyncHttpClient::CWaitListReq::PushReq( _REQ_DATA *data )
{
	// �������
	_mutex.lock() ;
	_list_req.push_back( data ) ;
	++ _size ;
	_mutex.unlock() ;

	_monitor.notify() ;
}

CAsyncHttpClient::_REQ_DATA * CAsyncHttpClient::CWaitListReq::PopReq ( void )
{
	share::Synchronized s(_monitor) ;
	{
		_REQ_DATA *req = NULL ;

		_mutex.lock() ;
		if ( _list_req.empty() ) {
			_mutex.unlock() ;
			_monitor.wait( 3 ) ;

			_mutex.lock() ;
			if ( _list_req.empty() ) {
				_mutex.unlock() ;
				return NULL ;
			}
		}

		req = _list_req.front() ;
		_list_req.pop_front() ;
		-- _size ;
		++ _waitsize ;
		_mutex.unlock() ;

		return req ;
	}
}

// ȡ�õȴ����ͺ͵ȴ���Ӧ������Ԫ�ظ�����
int CAsyncHttpClient::CWaitListReq::GetQueueSize( void )
{
	share::Guard guard( _mutex ) ;
	// �������ݶ��з��ͺ͵���Ӧ֮��
	return ( _size + _waitsize );
}

// ���ٿռ�
CAsyncHttpClient::_REQ_DATA *CAsyncHttpClient::CWaitListReq::AllocReq( void )
{
	_REQ_DATA *req = NULL ;

	_mutex.lock() ;
	req = _allocmgr.alloc() ;
	if ( req == NULL ) {
		req = new _REQ_DATA ;
	}
	_mutex.unlock() ;

	return req ;
}

// ���ն���
void CAsyncHttpClient::CWaitListReq::FreeReq( _REQ_DATA *req )
{
	_mutex.lock() ;
	if ( req == NULL ) {
		_mutex.unlock() ;
		return ;
	}
	_allocmgr.free( req ) ;
	-- _waitsize ;
	_mutex.unlock() ;
}

