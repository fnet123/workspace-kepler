#include "qstring.h"
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#define strnicmp  strncasecmp
#endif

CQString::CQString( void )
{
	_szTemp   = NULL ;
	_szBuffer = NULL ;
	_nLength  = 0 ;
	_nMemLen  = 0 ;
}

CQString::CQString( const char *sz )
{
	_szTemp   = NULL ;
	_szBuffer = NULL ;
	_nLength  = 0 ;
	_nMemLen  = 0 ;
	SetString( sz ) ;
}

CQString::CQString( const CQString &ss )
{
	_szTemp   = NULL ;
	_szBuffer = NULL ;
	_nLength  = 0 ;
	_nMemLen  = 0 ;
	*this      = ss ;
}

CQString::~CQString( void )
{
	if ( _szTemp != NULL )
	{
		free( _szTemp ) ;
		_szTemp = NULL ;
	}

	if ( _szBuffer != NULL ){
		free( _szBuffer ) ;
		return;
	}

	_nLength  = 0 ;
	_nMemLen  = 0 ;
	_szBuffer = NULL  ;
}
// ���صȺ�����
const CQString &CQString::operator = ( const char *sz )
{
	SetString( sz ) ;
	return (*this) ;
}

// ���ط�������
const CQString &CQString::operator = ( const CQString &ss )
{
	SetString( ss._szBuffer, ss._nLength ) ;
	return (*this) ;
}

// ��������
CQString::operator const char* ( void )
{
	return _szBuffer ;
}
// ��������
CQString::operator char* ( void )
{
	return _szBuffer ;
}

// �����ڴ�
void CQString::Expand( const int size )
{
	// ��Ҫ���ڴ�ĳ���
	int need = size + 256 + _nLength;
	// ���ַ������ڴ��Լ�ʹ�÷������,�������ٵĿ����ڴ����
	if ( _szBuffer == NULL ) {
		int len = 256;
		while (len < need ) len <<= 1;
		_szBuffer = (char*)malloc(len);
		memset( _szBuffer, 0, len ) ;
		_nMemLen  = len;
	} else if ( need > _nMemLen ) { // �ռ䲻��
		int bufsize = (int)( _nMemLen * 2 );
		while ( bufsize < need )
			bufsize <<= 1;

		char *newbuf = (char *)malloc(bufsize);
		assert(newbuf != NULL);
		memset( newbuf, 0, bufsize ) ;
		if ( _nLength > 0) {
			memcpy(newbuf, _szBuffer, _nLength );
		}
		free(_szBuffer);

		_szBuffer = newbuf ;
		_nMemLen  = bufsize;
	}
}

// ���Ƶ��ڴ���
void CQString::Memcopy( const char *ptr, int len )
{
	Expand( len ) ;
	if ( ptr != NULL ) {
		memcpy( _szBuffer+_nLength, ptr, len ) ;
	}
	_nLength = _nLength + len ;
	_szBuffer[_nLength] = 0 ;
}

// ��������
void CQString::SetString( const char *data, const int len )
{
	// �����ǰָ����Ǳ����Լ���û��Ҫ������
	if ( data == _szBuffer )
		return;

	int data_len = len ;
	if ( len == 0 && data != NULL )
		data_len = (int) strlen( data ) ;

	// ���ԭ������
	Clear() ;

	if ( data_len > 0 ){
		Memcopy( data, data_len ) ;
	}
}

// �������
void CQString::Clear( void )
{
	// ���ԭ������
	_nLength = 0 ;
	if ( _szBuffer != NULL ) {
		_szBuffer[0] = 0 ;
	}
}

// ʵ���ַ�������
const CQString& CQString::operator + ( const char *sz )
{
	AppendBuffer( sz  ) ;
	return *this;
}

// ʵ���ַ�������
const CQString& CQString::operator + ( const CQString &ss )
{
	AppendBuffer( ss._szBuffer, ss._nLength ) ;
	return *this;
}

// ������ݵĻ�����
void CQString::AppendBuffer( const char *data, const int len )
{
	if ( ( data == NULL && len == 0 ) || len < 0 ) return;

	int data_len =  len ;
	if ( data_len == 0 )
		data_len = (int) strlen(data)  ;

	Memcopy( data, data_len ) ;
}

// ��Ӷ�Ԫ����
const CQString &CQString::operator += ( const char *sz )
{
	AppendBuffer( sz ) ;
	return ( *this ) ;
}

// ��Ӷ�Ԫ����
const CQString &CQString::operator += ( const CQString &ss )
{
	AppendBuffer( ss._szBuffer, ss._nLength ) ;
	return ( *this ) ;
}

// ȡ�����ݳ���
const int CQString::GetLength( void ) const
{
	return _nLength;
}

// ȡ������
const char* CQString::GetBuffer( void ) const
{
	if ( _nLength == 0 || _szBuffer == NULL )
		return NULL;

	return _szBuffer ;
}

// ת����Сд
const char* CQString::ToLower( void )
{
	if ( _nLength == 0 || _szBuffer == NULL )
		return NULL ;


	char *p = (char*) _szBuffer ;
	for ( int i = 0 ; i < _nLength; ++ i  ) {
		if ( p[i] < 'A' || p[i] > 'Z' ){
			continue ;
		}
		p[i] = p[i] + 32 ;
	}

	return _szBuffer ;
}

// ת���ɴ�д
const char* CQString::ToUpper( void )
{
	if ( _nLength == 0 || _szBuffer == NULL )
		return NULL ;

	char *p = (char*) _szBuffer ;
	for ( int i = 0; i < _nLength; ++ i ) {
		if ( p[i] < 'a' || *p > 'z' ) {
			continue ;
		}
		p[i] = p[i] - 32 ;
	}

	return _szBuffer ;
}

// �Ƴ�����
const char * CQString::Remove( const char *sz )
{
	if ( sz == NULL || _szBuffer == NULL )
		return _szBuffer ;

	// �Ƴ��ַ���,������ĸ���ĳ��Ȼ����,ֻ��Ҫ��ԭ���ڴ��ϴ���
	char *end = (char*)(_szBuffer + _nLength);
	int size  = (int)strlen(sz) ;
	char *ptr = strstr( _szBuffer, sz ) ;
	while( ptr != NULL ) {
		if ( end == _szBuffer )
			break;

		memmove( ptr, ptr+size, ( end - ptr - size ) ) ;

		end		  = end - size ;
		_nLength  = _nLength - size ;
		*end	  = 0 ;
		ptr		  = strstr( _szBuffer, sz ) ;
	}
	_szBuffer[_nLength] = 0 ;

	return _szBuffer ;
}

// ȥ��ǰ�󲿷ֿո��"\r\n\t"
const char* CQString::Trim( void )
{
	if ( _szBuffer == NULL )
		return NULL ;

	char *p = _szBuffer ;
	// ȥ��ǰ�沿�ֿո��"\r\n\t"
	while ( *p == '\r' || *p == ' '
			|| *p == '\n' || *p == '\t' ){
		++ p ;
	}

	_nLength = _nLength - (int)(p-_szBuffer) ;
	memmove( _szBuffer , p , _nLength ) ;
	_szBuffer[_nLength] = 0 ;

	// ȥ�����沿�ֿո��"\r\n\t"
	p = _szBuffer + _nLength - 1 ;
	while ( *p == '\r' || *p == ' '
		|| *p == '\n' || *p == '\t' ) {
		-- p ;
		-- _nLength ;
	}
	_szBuffer[_nLength] = 0 ;

	return _szBuffer ;
}

// �滻�ַ���
const char* CQString::Replace( const char *src, const char *dest )
{
	if ( src == NULL || dest == NULL || _szBuffer == NULL )
		return _szBuffer ;

	int nsrc  = (int) strlen( src ) ;
	int ndest = (int) strlen( dest ) ;
	// �����ǱȽϺô����,�����ڴ�ʹ����Ҳ�Ƚϸ�
	if ( nsrc >= ndest ) {
		char *end = (char*)(_szBuffer + _nLength);
		char *ptr = strstr( _szBuffer, src ) ;
		while( ptr != NULL ) {
			if ( end == _szBuffer )
				break;

			memmove( ptr, dest, ndest ) ;
			memmove( ptr+ndest, ptr+nsrc, ( end - ptr - nsrc ) ) ;

			end		  = end - nsrc + ndest ;
			_nLength = _nLength  - nsrc + ndest ;
			*end	  = 0 ;
			ptr		  = strstr( _szBuffer, src ) ;
		}

		return _szBuffer ;
	}

	// �ֲ������ڴ�Ŀ���
	char * tmp = (char*) malloc( _nLength + 1 ) ;
	memcpy( tmp, _szBuffer, _nLength ) ;
	tmp[_nLength] = 0 ;

	int  offset  = 0 ;
	int  nsize   = 0 ;

	char * e = tmp + _nLength ;
	char * p = tmp ;
	char * q = strstr( p , src ) ;

	if ( q != NULL ) {
		// ���ԭ��������
		Clear() ;
	}
	while( q != NULL && q < e ) {
		nsize  = (int)( q - p ) ;
		// ���Ƿ��ڴ��ѿ������,����ʹ�ÿ����ڴ�Ĵ���
		Memcopy( p, nsize ) ;
		offset  += nsize ;
		Memcopy( dest, ndest ) ;
		offset  += ndest ;

		p = q + nsrc ;
		q = strstr( p, src ) ;
	}
	// ������ſ����滻���ַ������滻��������Ҫ�޸��ַ���������
	if ( offset > 0 ) {
		// ƴ�Ӻ�벿��û�������滻���ַ���
		if ( p < e ) {
			nsize = (int) ( e- p ) ;
			Memcopy( p, nsize ) ;
			offset += nsize ;
		}
		_szBuffer[offset] = 0 ;
		_nLength = offset ;
	}
	free( tmp ) ;

	return _szBuffer ;
}

// ����ַ��滻�ɶ��ַ�
const char* CQString::NReplace( int n, const char *c[], const char *s[] )
{
	if ( n == 0 || _nLength == 0 )
		return _szBuffer ;

	int i = 0, k = 0 ;
	int *ndest = new int[n] ;
	int *nsrc  = new int[n] ;

	for ( k = 0; k < n; ++ k ) {
		ndest[k] = strlen(s[k]) ;
		nsrc[k]  = strlen(c[k]) ;
	}

	// �ֲ������ڴ�Ŀ���
	char * tmp = (char*) malloc( _nLength + 1 ) ;
	memcpy( tmp, _szBuffer, _nLength ) ;
	tmp[_nLength] = 0 ;

	int offset = 0 , pos = -1 , dlen = 0 , len = 0;
	while ( i < _nLength ) {
		pos = -1 ;
		for ( k = 0; k < n; ++ k ) {
			if ( strncmp( tmp+i, c[k], nsrc[k] ) != 0 ) {
				continue ;
			}
			pos = k ;
			break ;
		}

		dlen = ( pos >= 0 ) ? ndest[pos] : 1 ;
		// ���Ƿ��ڴ��ѿ������
		if ( offset >= _nLength ) {
			Expand( offset + 1 + dlen - _nLength ) ;
		}
		if ( pos >= 0 ) {
			memcpy( _szBuffer+offset, s[pos], dlen ) ;
			i = i + nsrc[pos] ;
		} else {
			_szBuffer[offset] = tmp[i] ;
			i = i + 1 ;
		}
		offset = offset + dlen ;
	}

	_szBuffer[offset] = 0 ;
	_nLength = offset ;

	free( tmp ) ;

	delete [] ndest ;
	delete [] nsrc ;

	return _szBuffer ;
}

// �����ַ���
const int CQString::Find( const char *sz, const int pos )
{
	if ( sz == NULL || _szBuffer == NULL || pos >= _nLength )
		return -1 ;

	char *pdata = (char*)( _szBuffer + pos ) ;
	char *ptr   = strstr( pdata, sz ) ;
	if ( ptr == NULL ) {
		return -1 ;
	}
	// �����ַ�����λ��
	return (int) ( ptr - _szBuffer ) ;
}

// ȡ�Ӵ�
const char* CQString::SubString( const int pos, const int len )
{
	if ( pos < 0 || pos >= _nLength )
		return NULL ;

	int nlen = ( len > _nLength-pos || len <= 0 ) ? (_nLength-pos) : len ;
	if ( nlen == 0  ) {
		return NULL ;
	}
	if ( _szTemp != NULL ) {
		free( _szTemp ) ;
		_szTemp = NULL ;
	}

	// ȡ�Ӵ�����
	_szTemp = (char*) malloc( sizeof(char)*(nlen+1) ) ;
	memcpy( _szTemp, _szBuffer+pos,  nlen ) ;
	_szTemp[nlen] = 0 ;

	return _szTemp ;
}

// �ַ����Ƿ�Ϊ�յĴ���
bool CQString::IsEmpty( void )
{
	return ( _szBuffer == NULL || _nLength == 0 ) ;
}

// ���ڴ����ȥ�����ٸ��ַ�
void CQString::MemTrimLeft( const int count )
{
	if ( _nLength < count )
		return ;

	if ( count == _nLength ) {
		_nLength     = 0 ;
		_szBuffer[0] = '\0' ;

		return ;
	}

	memmove( _szBuffer, _szBuffer + count, _nLength - count ) ;
	_nLength  = _nLength - count ;
	_szBuffer[_nLength] = '\0' ;
}

// ���ڴ��ұ�ȥ�����ٸ��ַ�
void CQString::MemTrimRight( const int count )
{
	if ( _nLength < count )
		return ;

	if ( _nLength == count ) {
		_nLength	  = 0 ;
		_szBuffer[0] = '\0' ;
		return ;
	}

	_nLength = _nLength - count ;
	_szBuffer[_nLength] = '\0' ;
}

// ֻ���¶೤����
void CQString::MemTrimLength( const int count )
{
	if ( _nLength < count )
		return ;

	// ֱ�Ӵ��ڴ���ȥ��
	_nLength 		  = count ;
	_szBuffer[count] = '\0' ;
}

// ת��ʮ������
const char * CQString::ToHex( void )
{
	if ( _nLength == 0 )
		return NULL ;

	if ( _szTemp )
		free( _szTemp ) ;

	_szTemp = ( char *) malloc( _nLength * 3 + 16 ) ;

	char *ptr = _szTemp ;
	for ( int i = 0; i < _nLength; ++ i ){
		sprintf( ptr, "%02x " , (unsigned char) _szBuffer[i] ) ;
		ptr += 3 ;
	}
	return _szTemp ;
}
