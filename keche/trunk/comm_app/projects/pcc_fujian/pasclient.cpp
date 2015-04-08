#include "pasclient.h"
#include <Base64.h>
#include "pccutil.h"
#include <crc16.h>
#include <comlog.h>
#include <tools.h>
#include "header.h"

PasClient::PasClient(PConvert *convert)
	: _convert(convert)
{
	_last_noop = time(NULL) ;
}

PasClient::~PasClient()
{
	Stop() ;
}

bool PasClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char buf[1024] = {0};
	if ( ! _pEnv->GetString( "pcc_client_ip", buf ) ) {
		printf( "serverip empty\n") ;
		OUT_ERROR( NULL, 0, NULL, "pcc_client_ip empty" ) ;
		return false ;
	}
	_client_user._ip = buf ;

	if ( ! _pEnv->GetString( "pcc_client_port", buf ) ) {
		printf( "tcpport empty\n") ;
		OUT_ERROR( NULL, 0, NULL, "pcc_client_port empty" ) ;
		return false ;
	}
	_client_user._port = atoi(buf) ;

	// ���÷ְ���������ݽ��зְ�����
	setpackspliter( &_packspliter ) ;

	// ���س�����̬��Ϣ
	return true ;
}

void PasClient::Stop( void )
{
	StopClient() ;
}

bool PasClient::Start( void )
{
	// ������ҪTCP������ͨ��
	return StartClient( _client_user._ip.c_str() , _client_user._port , 3 ) ;
}

void PasClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
	if ( data == NULL || len < (int) sizeof(_Header) ){
		OUT_ERROR( sock->_szIp, sock->_port, "Recv", "recv fd %d data %s error, data len:%d", sock->_fd, (char *)data, len ) ;
		OUT_HEX( sock->_szIp, sock->_port, "Recv", (const char *)data, len ) ;
		return ;
	}

	_Header *header = ( _Header *) data ;

	unsigned int result  = ntohl( header->result ) ;
	unsigned int nlen    = ntohl( header->len ) ;
	unsigned int seq     = ntohl( header->seq ) ;

	char szphone[17] = {0} ;
	bcd2str( (const char *)header->termid, sizeof(header->termid), szphone ) ;

	// ���Ϊ��������ֱ�ӷ�����
	if ( nlen == 0 ) {
		if ( result == 0x00 )
			SendData( sock, (const char*)data, len ) ;
		OUT_ERROR( sock->_szIp, sock->_port, szphone, "Recv data error" ) ;
		OUT_HEX(sock->_szIp, sock->_port, szphone, (const char *)data, len ) ;
		return ;
	}

	// ����ϴ����ݵ�FD�뵱ǰCLIENTһ�¾͸������ʱ��
	if ( _client_user._fd == sock )
		_client_user._last_active_time = time(NULL) ;
	// 0xF002	��	�����ɹ�
	// 0xF003	��	�����ʧ��

	result = 0xF002 ;

	DataBuffer resp ;
	resp.writeBlock( data, sizeof(_Header) ) ;

	const char *ptr = (const char *)data + sizeof(_Header) ;

	unsigned short cmd = 0 ;
	memcpy( &cmd, ptr, sizeof(short) ) ;
	cmd = ntohs( cmd ) ;

	OUT_RECV( sock->_szIp, sock->_port, szphone, "Recv msg cmd %04x" , cmd ) ;
	OUT_HEX( sock->_szIp, sock->_port, "Recv", (const char *)data, len ) ;

	switch( cmd )
	{
	case 0x0102:  // ����ָ��
		if ( ! _convert->build_caller( seq, szphone, ptr , nlen ) ) {
			result = 0xF003 ;
		}
		break ;
	case 0x0301:  // ����ָ��
		if ( ! _convert->build_photo( seq, szphone, ptr , nlen ) ) {
			result = 0xF003 ;
		}
		break ;
	case 0x0401:  // ����ָ��
		if ( ! _convert->build_sendmsg( seq, szphone, ptr, nlen ) ) {
			result = 0xF003 ;
		}
		break ;
	}

	// ���ؽ��
	resp.fillInt32( sizeof(short), sizeof(_Header)-sizeof(int) ) ;
	resp.writeInt16( result ) ;

	// ����ͨ��Ӧ��
	if ( ! SendData( sock, resp.getBuffer(), resp.getLength() ) ) {
		OUT_ERROR( sock->_szIp, sock->_port, szphone, "send response failed" ) ;
	} else {
		OUT_ERROR( sock->_szIp, sock->_port, szphone, "send response success" ) ;
	}
	OUT_HEX( sock->_szIp, sock->_port, szphone, resp.getBuffer(), resp.getLength() ) ;
}

void PasClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	OUT_INFO( sock->_szIp, sock->_port, "Conn", "Recv disconnect fd %d", sock->_fd ) ;

	_client_user._login_time = 0;
	_client_user._user_state = User::OFF_LINE ;
}

void PasClient::TimeWork()
{
	/*
	 * 1.����ʱ������ȥ����
	 * 2.��ʱ����NOOP��Ϣ
	 * 3.Reload�����ļ��е��µ����ӡ�
	 * 4.
	 */
	while(1)
	{
		if ( ! Check() ) break ;

		time_t now = time(NULL) ;
		// ����û�״̬
		if ( _client_user._user_state != User::ON_LINE
				&& now - _client_user._login_time > 60 ) {

			if (_client_user._fd  != NULL )
				CloseSocket(_client_user._fd);

			_client_user._fd = _tcp_handle.connect_nonb(_client_user._ip.c_str(), _client_user._port, 30 );
			if ( _client_user._fd > 0 ) {
				_client_user._last_active_time = now ;
				_client_user._login_time       = now ;

				_client_user._user_state = User::ON_LINE ;

				OUT_INFO( NULL, 0, "Conn", "connect ip %s:%d success"  , _client_user._ip.c_str(), _client_user._port ) ;
			} else {
				OUT_ERROR( NULL, 0, "Conn", "connect ip %s:%d failed"  , _client_user._ip.c_str(), _client_user._port ) ;
			}
		} else if ( now - _client_user._last_active_time > 120 ) {
			// ����������ݳ�ʱ�Զ�����������Ҫ����
			_client_user._user_state = User::OFF_LINE ;
		}

		// ʮ���ӳ�ʱ���
		_pEnv->GetMsgCache()->CheckData( 600 ) ;

		sleep(3) ;
	}
}

void PasClient::NoopWork()
{
	while(1) {
		if ( !Check() ) break ;

		time_t now = time(NULL) ;
		if ( now - _last_noop > 30 && _client_user._user_state == User::ON_LINE ) {

			DataBuffer noop ;
			_convert->buildheader( noop, "0000000000000000", 0, 0 ) ;

			HandleData( noop.getBuffer(), noop.getLength() ) ;

			_last_noop = now ;
		}

		sleep(5) ;
	}
}

void PasClient::HandleData( const char *data, int len )
{
	// �������ݵ����ƽ̨,������ͨ������ͨ������������
	if ( _client_user._user_state != User::ON_LINE || _client_user._fd == NULL ) {
		OUT_ERROR( _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str(), "User not online, data len: %d" , len ) ;
		OUT_HEX( _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str(), data, len ) ;
		return ;
	}
	if ( ! SendData( _client_user._fd, data, len ) ){
		OUT_ERROR(  _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str(),  "Send Data Failed, data len: %d" , len ) ;
		OUT_HEX( _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str(), data, len ) ;
	}
	OUT_PRINT( _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str() , "Send Data, data len: %d" , len ) ;
	OUT_HEX(  _client_user._ip.c_str() , _client_user._port , _client_user._user_name.c_str() , data ,len ) ;
}

struct packet * PasClient::CPackSpliter::get_kfifo_packet( DataBuffer *fifo )
{
	unsigned int len = fifo->getLength() ;
	if ( len <= 0 || len > MAX_PACK_LEN ){
		fifo->resetBuf() ;
		return NULL;
	}

	char *p = NULL, *in_begin = NULL ;
	struct list_head *packet_list_ptr = NULL;

	unsigned int ndel = 0 ;
	unsigned int i = 0;
	in_begin = p = (char *) fifo->getBuffer() ;
	// OUT_HEX( NULL, 0, "Spliter", p, len ) ;
	while ( i < len && len > 2 ) {
		if (*p == '&' && *(p+1) == '&' ) {  // "&&"

			_Header *header = (_Header *) p ;
			unsigned int nlen = ntohl( header->len ) ;
			unsigned int pack_len = nlen + sizeof(_Header) ;

			if ( pack_len > len ) {
				++ p ; ++ i ;
				continue ;
			}

			struct packet *item = (struct packet *) malloc(sizeof(struct packet));
			if (item == NULL) break ;

			item->data = (unsigned char *) malloc(pack_len+1);
			memset(item->data, 0, pack_len+1);
			memcpy(item->data, p, pack_len);
			item->len = pack_len;
			item->type = E_PROTO_OUT;

			if (packet_list_ptr == NULL) {
				packet_list_ptr = (struct list_head *) malloc(sizeof(struct list_head));
				if (packet_list_ptr == NULL)
					return NULL;

				INIT_LIST_HEAD(packet_list_ptr);
			}
			list_add_tail(&item->list, packet_list_ptr);
			// OUT_INFO( NULL, 0, "Spliter", "split pack pack length: %d" , pack_len ) ;

			in_begin = p + pack_len ;

			p = p + pack_len - 1 ;  // ֱ��ת��0x5d������ʶ����
			i = i + pack_len - 1 ;
			ndel = i + pack_len ;
			// ++ g_packcount ;
		}

		++ p ;
		++ i ;
	}
	// ��������������ݲ�Ϊ��
	if ( ndel > 0 ) {
		fifo->removePos( ndel );
	}
	//printf("get packet total %d in packets , %d out packets \n", in_counter, out_counter);
	return (struct packet*) packet_list_ptr;
}

