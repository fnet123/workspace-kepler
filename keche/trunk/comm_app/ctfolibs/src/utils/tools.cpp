#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef _NI_WIN_
#include <sys/socket.h>  
#include <arpa/inet.h> 
#include <iconv.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <netinet/in.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define OUTLEN 255

//����ת��:��һ�ֱ���תΪ��һ�ֱ���
int code_convert(const char *from_charset, const char *to_charset, char *inbuf, int inlen, char *outbuf, int& outlen)
{
	// ��iconv
	iconv_t transer = iconv_open( to_charset, from_charset ) ;

	memset( outbuf , 0 , outlen ) ;
	if ( transer != (iconv_t)-1 )
	{
		size_t ilen       = inlen + 1 ;
		size_t olen       = outlen ;
		size_t result_len = iconv( transer , &inbuf , &ilen , &outbuf , &olen )  ;
		iconv_close( transer ) ;

		if ( result_len != (size_t)-1 )
		{
			outlen = outlen - olen ;
			return 0 ;
		}

	} else {

		memcpy( outbuf, inbuf, inlen ) ;
		outlen = inlen ;

		return 0 ;
	}

	return -1 ;
}
//UNICODE��תΪGB2312��
int u2g( char *inbuf, int inlen, char *outbuf, int& outlen)
{
	return code_convert("UTF-8","GBK",inbuf,inlen,outbuf,outlen);
}
//GB2312��תΪUNICODE��
int g2u( char *inbuf, size_t inlen, char *outbuf, int& outlen)
{
	return code_convert("GBK", "UTF-8",inbuf,inlen,outbuf,outlen);
}

// ���IP�ĺϷ���
bool check_addr( const char *ip )
{
	if ( ip == NULL )
		return false ;
	if ( inet_addr(ip) == (in_addr_t)-1 )
		return false ;
	return true ;
}

// ��ȫ�ڴ濽��
char * safe_memncpy( char *dest, const char *src, int len )
{
	if ( src == NULL )
		return NULL ;

	int nsize  = len ;
	int nlen   = (int) strlen( src ) ;
	if ( nlen < len ) {
		nsize = nlen ;
	}

	memset( dest, 0, len ) ;
	memcpy( dest, src, nsize ) ;

	return dest ;
}

// �Զ����ǰ����ָ������
bool splitvector( const string &str, std::vector<std::string> &vec, const std::string &split , const int count )
{
	if ( str.empty() )
		return false ;

	std::size_t end = 0 ;
	std::size_t len = split.length() ;
	std::size_t pos = str.find(split.c_str()) ;

	int index = 0 ;
	while( pos != std::string::npos ) {
		vec.push_back( str.substr(end,pos-end) ) ;

		end = pos + len ;
		if ( count > 0 && ++ index == count ) {
			break ;
		}
		pos = str.find( split.c_str(), end ) ;
	}

	if ( end < str.length() ) {
		vec.push_back( str.substr(end) ) ;
		if ( index != count && count > 0 ) {
			++ index ;
		}
	}

	return ( count == 0 || index == count ) ;
}

// ������Ҫ�������·���д��� env:LBS_ROOT/lbs ֮���·��
bool getenvpath( const char *value, char *szbuf )
{
	if ( value == NULL )
		return false ;

	char buf[1024] = {0} ;
	strcpy( buf, value ) ;

	if ( strncmp(buf,"env:", 4) == 0  ) {
		char *q = strstr( buf, "/" ) ;
		if ( q == NULL ) {
			q = strstr( buf , "\\" ) ;
		}
		if ( q == NULL ) {
			return false ;
		}
		*q = 0 ;

		sprintf( szbuf, "%s/%s", getenv(buf+4) , q+1 ) ;

	} else {
		sprintf( szbuf, "%s", buf ) ;
	}
	return true ;
}

/**
 *  ȡ�õ�ǰ��������·��,
 *	env Ϊ�����������ƣ�buf ���·���Ļ���, sz Ϊ���Ӻ�׺, def Ĭ�ϵ��о�
 */
const char * getrunpath( const char *env, char *buf, const char *sz, const char *def )
{
	if ( env == NULL ) {
		sprintf( buf, "%s" , def ) ;
		return buf ;
	}

	char *ptr = getenv(env) ;
	if ( ptr == NULL ) {
		sprintf( buf, "%s", def ) ;
		return buf ;
	}

	if ( sz != NULL ) {
		sprintf( buf, "%s/%s" , ptr, sz ) ;
	} else {
		sprintf( buf, "%s" , ptr ) ;
	}
	return buf ;
}

// ȡ��Ĭ�ϵ�CONF·��
const char * getconfpath( const char *env, char *buf, const char *sz, const char *def, const char *conf )
{
	char temp[512] = {0} ;
	getrunpath( env, temp, sz, def ) ;
	sprintf( buf, "%s/%s" , temp, conf ) ;
	return buf ;
}

// д���ļ�����
bool AppendFile( const char *szName, const char *szBuffer, const int nLen )
{
#ifdef _WIN32
	FILE *fp = fopen( szName, "a+" ) ;
	if ( fp == NULL )
	{
		return false;
	}
	fwrite( szBuffer, nLen, 1, fp ) ;
	fclose( fp ) ;
#else
	int fp = open( szName, O_CREAT | O_APPEND | O_WRONLY ) ;
	if ( fp < 0 )
	{
		return false;
	}
	write( fp, szBuffer, nLen ) ;
	close( fp ) ;
	chmod( szName, 0777 ) ;
#endif

	return true;
}

// д���ļ�����
bool WriteFile( const char *szName, const char *szBuffer, const int nLen )
{
#ifdef _WIN32
	FILE *fp = fopen( szName, "wb" ) ;
	if ( fp == NULL )
	{
		return false;
	}
	fwrite( szBuffer, nLen, 1, fp ) ;
	fclose( fp ) ;
#else
	int fp = open( szName, O_CREAT | O_TRUNC | O_WRONLY ) ;
	if ( fp < 0 )
	{
		return false;
	}
	write( fp, szBuffer, nLen ) ;
	close( fp ) ;
	chmod( szName, 0777 ) ;
#endif

	return true;
}

// ��ȡ�ļ�
char *ReadFile( const char *szFile , int &nLen )
{
	char *szBuffer = NULL ;
#ifdef _WIN32

	FILE* fp = fopen( szFile , "rb" ) ;
	if ( fp != NULL )
	{
		int len = 0 ;
		fseek( fp , 0 , SEEK_END ) ;
		len = ftell( fp ) ;

		szBuffer = new char[len+1];

		fseek( fp , 0 , SEEK_SET ) ;
		fread( szBuffer , 1 , len , fp ) ;
		szBuffer[len] = 0 ;

		nLen  = len ;

		fclose( fp );
	}
	else
	{
		nLen  = 0 ;
		return NULL ;
	}

#else

	int fp = open( szFile , O_RDONLY ) ;

	if ( fp >= 0 )
	{
		// �ȵõ��ļ��Ĵ�С
		struct stat buf ;
		fstat( fp , &buf ) ;

		int len = buf.st_size ;

		szBuffer = new char[len+1];

		read( fp , szBuffer , len ) ;
		szBuffer[len] = 0 ;

		nLen  = len ;

		close( fp );
	}
	else
	{
		nLen =  0 ;
		return NULL ;
	}

#endif

	return szBuffer;
}


// �ͷ�����
void  FreeBuffer( char *buf )
{
	if ( buf == NULL )
		return ;
	delete [] buf ;
	buf = NULL ;
}
