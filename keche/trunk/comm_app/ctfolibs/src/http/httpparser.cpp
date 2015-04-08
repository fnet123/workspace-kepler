#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "httpparser.h"
#ifndef _WIN32
#include <strings.h>
#define strnicmp  strncasecmp
#define stricmp   strcasecmp
#endif

static char* qtrim(char* str)
{
	char* p = str;

	char* result;

	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' )
		p++;

	result = p;

	p += strlen(p) - 1;

	while (p >= result && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ))
		p--;

	*(p + 1) = '\0';

	return result;
}

CParamList::CParamList()
{
	memset( &_HeaderList , 0 , sizeof( _HeaderList ) ) ;
}

CParamList::~CParamList()
{
	CleanHeaderList() ;
}

const char* CParamList::GetValue( const char* header ) const
{
	LPHEADERNODE p = _HeaderList.next ;

	while( p ) {
		if ( p->header != NULL ) {
			if ( stricmp( header , p->header ) == 0 ) {
				return p->value ;
			}
		}

		p = p->next ;
	}

	return NULL ;

}

void CParamList::AddNode( const char* name , const char* value )
{
	LPHEADERNODE node = new HEADERNODE;

	node->header = strdup( name );
	node->value = strdup( value );

	if ( _HeaderList.next == NULL ){
		//first header
		node->next = NULL;
		_HeaderList.next = node;

	} else {
		LPHEADERNODE temp_node = _HeaderList.next ;
		LPHEADERNODE tail      = &_HeaderList ;
		while ( temp_node ) {
			tail = temp_node;
			temp_node = temp_node->next;
		}

		node->next = NULL;
		tail->next = node;
	}

}

void CParamList::CleanHeaderList()
{
	LPHEADERNODE p = _HeaderList.next ;
	while ( p ){
		LPHEADERNODE temp = p->next ;
		if ( p->header != NULL )
			free( p->header ) ;
		if ( p->value != NULL )
			free( p->value ) ;

		delete p ;

		p = temp ;
	}

	_HeaderList.next = NULL ;
}

CHttpParser::CHttpParser()
{
	_pBody 		= NULL ;
	_iBodySize  = 0 ;

}

CHttpParser::~CHttpParser()
{
	CleanBody() ;
}

void CHttpParser::CleanBody( void )
{
	if ( _pBody != NULL ) {
		free( _pBody ) ;
	}

	_pBody 		= NULL ;
	_iBodySize  = 0  ;
}


int CHttpParser::SetChunkBody( const char* data , const int size )
{
	const char* trailer = "\r\n" ;
	int trailer_size = (int)strlen( trailer ) ;

	int ret = HTTPPARSER_ERR_DATAERROR ;

	// CHUNK ����ɣ�CHUNK-SIZE<CRLF>CHUNK-BODY<CRLF> ֱ��CHUNK-SIZEΪ0�����������
	const char* start = data ;
	string str ;
	int left_size = size ;
	int chunk_size = 0 ;

	// ��ʱBODY
	char* temp_body = NULL ;
	int body_size = 0 ;

	while (1){
		const char* p = strstr( start , trailer ) ;
		if ( p == NULL ){
			break ;
		}

		str.assign( start , p - start ) ;
		chunk_size = hex2int( str.c_str() ) ;

		if ( chunk_size == 0 ){
			ret = HTTPPARSER_ERR_SUCCESS ;
			break ;
		}

		const char* chunk_data = NULL ;

		start = p ;
		start += trailer_size ;
		chunk_data = start ;
		start += chunk_size ;
		start += trailer_size ;

		left_size = (int)( size - ( start - data ) ) ;

		if ( left_size <= 0 ){
			break ;
		}

		// ����CHUNK����
		if ( temp_body == NULL ){
			temp_body = (char*)malloc( chunk_size + 1 ) ;
		}else{
			temp_body = (char*)realloc( temp_body , body_size+chunk_size+1 ) ;
		}

		memcpy( temp_body + body_size , chunk_data , chunk_size ) ;
		body_size += chunk_size ;
		temp_body[body_size] = 0 ;
	}

	if ( ret != HTTPPARSER_ERR_SUCCESS ){
		free( temp_body ) ;
	} else {
		_pBody 		= temp_body ;
		_iBodySize  = body_size ;
	}

	return ret ;
}

int CHttpParser::SetBody( const char* data , const int size )
{
	CleanBody() ;
	if ( data == NULL || size == 0 )
		return HTTPPARSER_ERR_NODATA ;

	// �����CHUNK��ʽ��Ҫ��CHUNKȥ��������CHUNK����Ϊһ��BODY
	const char* transfer_code = GetHeader( "Transfer-Encoding" ) ;
	if ( transfer_code != NULL ) {
		if ( stricmp( transfer_code , "chunked" ) == 0 ){
			// ��CHUNKģʽ
			return SetChunkBody( data , size ) ;
		}
	}

	_pBody = (char*)malloc( size+1 ) ;
	if ( _pBody == NULL )
		return HTTPPARSER_ERR_NOMEM ;

	memcpy( _pBody , data , size ) ;
	_pBody[size] = 0 ;
	_iBodySize   = size ;

	return HTTPPARSER_ERR_SUCCESS ;

}


int CHttpParser::hex2int( const char* str )
{
	int len = (int) strlen( str ) ;
	int i;

	unsigned int ret_value = 0 ;

	for ( i=0; i<len; ++ i ) {
		unsigned int num ;
		if ( str[i] <= '9' && str[i] >= '0' ) {
			num = str[i] - '0' ;
		} else if ( str[i] == 'a' || str[i] == 'A' ) {
			num = 10 ;
		} else if ( str[i] == 'b' || str[i] == 'B' ) {
			num = 11 ;
		} else if ( str[i] == 'c' || str[i] == 'C' ) {
			num = 12 ;
		} else if ( str[i] == 'd' || str[i] == 'D' ) {
			num = 13 ;
		} else if ( str[i] == 'e' || str[i] == 'E' ) {
			num = 14 ;
		} else if ( str[i] == 'f' || str[i] == 'F' ) {
			num = 15 ;
		}

		ret_value <<= 4 ;
		ret_value |= num ;
	}

	return ret_value ;
}

int CHttpParser::DetectCompleteChunk( const char* body , int body_size )
{
	char* temp = new char[ body_size + 1 ] ;
	memcpy( temp , body , body_size ) ;
	temp[body_size] = 0;

	const char* trailer = "\r\n" ;
	int trailer_size = (int)strlen( trailer ) ;

	int ret = HTTPPARSER_ERR_NODATA ;

	// CHUNK ����ɣ�CHUNK-SIZE<CRLF>CHUNK-BODY<CRLF> ֪��CHUNK-SIZEΪ0�����������
	const char* start = temp ;
	string str ;
	int left_size = body_size ;
	int chunk_size = 0 ;

	while (1){
		const char* p = strstr( start , trailer ) ;
		if ( p == NULL ){
			break ;
		}

		str.assign( start , p - start ) ;
		chunk_size = hex2int( str.c_str() ) ;

		start = p ;
		start += trailer_size ;
		start += chunk_size ;
		start += trailer_size ;

		left_size = (int)( body_size - ( start - temp ) ) ;

		if ( left_size <= 0 ){
			if ( chunk_size == 0 ){
				ret = HTTPPARSER_ERR_SUCCESS ;
			}

			break ;
		}
	}

	delete [] temp ;

	return ret ;
}

int CHttpParser::DetectCompleteReq( const char* data , const int size )
{
	char* temp = new char[ size + 1 ] ;
	memcpy( temp , data , size ) ;
	temp[size] = 0 ;

	int content_len = 0 ;

	char* p , *q ;

	// ����,����û��ͷ�Ľ�β
	p = temp ;
	q = strstr( p , "\n\r\n" ) ;
	if ( q == NULL )
		q = strstr( p , "\n\n" ) ;

	if ( q == NULL ){
		delete [] temp ;
		return HTTPPARSER_ERR_NOHEADER ;
	}

	// �Ѿ���β����
	q++ ;
	*q = '\0' ;
	q++ ;
	q = strchr(q , '\n' ) ;
	q++ ;

	int hdr_size = (int)( q - temp ) ;

	int cur_body_size = size - hdr_size ;

	// �����Ƿ���content-lengthͷ

	my_strlwr( temp ) ;

	// ֻ�Ƚ�ǰ�����ַ������ǲ���GET����
	if ( strncmp( temp , "get" , 3 ) == 0 ){
		// GET ������û���������
		delete [] temp ;
		return HTTPPARSER_ERR_SUCCESS ;
	}

	p = strstr( temp , "content-length" ) ;

	if ( p != NULL ){
		q = strchr( p , '\n' ) ;
		if ( q == NULL ){
			delete [] temp ;
			return HTTPPARSER_ERR_DATAERROR ;
		}

		*q = '\0' ;
		p = strchr( p , ':' ) ;

		if ( p == NULL )
		{
			delete [] temp ;
			return HTTPPARSER_ERR_DATAERROR ;
		}

		p++ ;
		content_len = atoi( qtrim(p) ) ;

		delete [] temp ;

		if ( cur_body_size != content_len )
			return HTTPPARSER_ERR_NODATA ;

		return HTTPPARSER_ERR_SUCCESS ;

	} else {
		// û��content-length

		// �п�����CHUNK��ʽ
		p = strstr( temp , "transfer-encoding" ) ;

		if ( p == NULL ){
			// ��û��CONTENT-LENGTH���ֲ���CHUNK���룬���ش���
			delete [] temp ;
			return HTTPPARSER_ERR_NOCONTENTLEN ;
		} else {
			// ȡ��TRANSFER-ENCODING ��ֵ
			q = strchr( p , '\n' ) ;
			if ( q == NULL ){
				delete [] temp ;
				return HTTPPARSER_ERR_DATAERROR ;
			}

			*q = '\0' ;

			q -- ;
			if ( *q == '\r' ) {
				*q = '\0' ;
			}

			p = strchr( p , ':' ) ;

			if ( p == NULL ) {
				delete [] temp ;
				return HTTPPARSER_ERR_DATAERROR ;
			}

			p++ ;

			if ( stricmp( qtrim(p) , "chunked" ) != 0 ) {
				// ����CHUNKEDģʽ
				delete [] temp ;
				return HTTPPARSER_ERR_NOCONTENTLEN ;
			}

			// ��CHUNK��ʽ�������ж��Ƿ��ֽ�β
			int ret = DetectCompleteChunk( temp+hdr_size , cur_body_size ) ;
			delete [] temp ;
			return ret ;
		}
	}
}


int CHttpParser::Parse( const char* data , const int size )
{
	_ParamList.CleanHeaderList() ;

	CleanBody() ;

	if ( data == NULL || size == 0 )
		return HTTPPARSER_ERR_DATAERROR ;

	char* temp = new char[ size+1 ] ;

	if ( temp == NULL )
		return HTTPPARSER_ERR_NOMEM ;

	memcpy( temp , data , size ) ;
	temp[size] = 0 ;

	// ����HTTP����,������ͷ,ͷ�б������

	char* p = temp ;
	char* q = NULL ;
	char* next_line = NULL ;

	// HTTP/1.1 200 OK\r\nDate: Wed, 07 Sep 2011 11:25:52 GMT\r\nServer: BWS/1.0\r\nContent-Length: 8774\r\nContent-Type: text/html;charset=gb2312\r\nCache-Control: private\r\nExpires: Wed, 07 Sep 2011 11:25:52 GMT\r\n
	while ( p != NULL ) {
		// ����\r\n������һ��TAB����һ���ո񣬸�\r\n��Ч
		q = strchr( p , '\n' );

		if ( q == NULL )
			break ;

		*q = '\0';

		next_line = q+1 ;

		while ( 1 ) {
			q = strchr ( p , '\r' );
			if ( q )
				*q = ' ';
			else
				break ;
		}

		p = qtrim(p) ;

		if ( strlen(p) != 0 ) {
			char* header = p ;

			// ������header��value
			q = strchr( header , ':' ) ;

			if ( q != NULL ) {
				*q = '\0' ;

				char* value = q+1 ;

				_ParamList.AddNode( qtrim(header) ,qtrim( value ) ) ;
			} else {
				// �ǵ�һ��    GET /favicon.ico HTTP/1.1 �������˵����,�ͻ��˵���� HTTP/1.0 200 OK
				p = strchr( header , ' ' ) ;
				if ( p == NULL ){
					// ����
					delete [] temp ;
					return HTTPPARSER_ERR_DATAERROR ;
				}
				*p = '\0' ;

				char* code = p+1 ;
				p = strchr( code , ' ' ) ;
				if ( p == NULL ){
					// ����
					delete [] temp ;
					return HTTPPARSER_ERR_DATAERROR ;
				}

				*p = '\0' ;
				p++ ;

				char *phttp = NULL ;
				// �����Ϊ�ͻ��˴���
				if ( strnicmp( header, "http", 4 ) == 0 )  {
					_ParamList.AddNode( EXT_HEADER_CODE , code ) ;
					_ParamList.AddNode( EXT_HEADER_STATUSTEXT , p ) ;
					phttp = header ;
				} else {  // ��Ϊ�����������ݽ���
					_ParamList.AddNode( EXT_HEADER_METHOD, header ) ;
					_ParamList.AddNode( EXT_HEADER_URI, code ) ;
					phttp = p ;
				}

				my_strlwr( phttp ) ;
				if ( strstr( phttp , "http/1.1" ) != NULL )
					_ParamList.AddNode( EXT_HEADER_VERSION , "1.1" ) ;
				else if ( strstr( phttp  , "http/1.0" ) != NULL )
					_ParamList.AddNode( EXT_HEADER_VERSION , "1.0" ) ;
			}

			p = next_line ;

		} else {
			// ����
			SetBody( next_line , (int)( size - ( next_line - temp )) ) ;
			p = NULL ;
		}
	}

	delete [] temp ;

	return HTTPPARSER_ERR_SUCCESS ;
}



const char* CHttpParser::GetHeader( const char* header ) const
{
	return _ParamList.GetValue( header ) ;
}

void CHttpParser::my_strlwr( char* str )
{
	int i;
	int count = (int) strlen( str ) ;
	for ( i=0;i<count;i++ ) {
		if ( str[i] <= 'Z' && str[i] >= 'A' ) {
			str[i] += 32 ;
		}
	}
}
