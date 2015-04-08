/*
 * postquery.cpp
 *
 *  Created on: 2012-7-5
 *      Author: humingqing
 *  ����ƽ̨����Զ���Ӧ
 */

#include "postquery.h"
#include <tools.h>

CPostQueryMgr::CPostQueryMgr()
{

}

CPostQueryMgr::~CPostQueryMgr()
{
	ClearPost() ;
}

// 1{[��ǰ����ҵ���б�:=10002006030|1000,1001,1002 10002006031|1000,1001][]}
// ����ƽ̨��ڵ�����
bool CPostQueryMgr::LoadPostQuery( const char *path, int accesscode )
{
	char szbuf[1024] = {0};
	sprintf( szbuf, "%s/%d", path, accesscode ) ;
	// printf( "load %s\n", szbuf ) ;

	int  len  = 0 ;
	char *ptr = ReadFile( szbuf, len ) ;
	if ( ptr == NULL )
		return false ;

	int count = 0 ;
	char *p = ptr ;
	char *q = strstr( p , "\n" ) ;
	while( q != NULL ) {
		*q = 0 ;  q = q + 1 ;
		if ( SplitItem( accesscode, p ) ) {
			++ count ;
		}
		p = q  ;
		q = strstr( p, "\n" ) ;
	}
	// �������һ���ַ���
	if ( p != NULL ) {
		if ( SplitItem( accesscode, p ) ) {
			++ count ;
		}
	}
	FreeBuffer( ptr ) ;

	return ( count > 0 ) ;
}

// �����Ŀ����
bool CPostQueryMgr::SplitItem( int accesscode, char * p )
{
	// �Ƴ��ո�������ַ�
	while( *p == ' ' || *p == '\r' || *p == '\n' ) {
		++ p ;
	}
	if ( p == NULL ) return false ;

	// ���ҿ�ʼ��ʶ
	char *begin = strstr( p, "{") ;
	if ( begin == NULL )
		return false ;
	*begin = 0 ;

	char *body = begin+1 ;
	// ���ҽ�����ʶ
	char *end  = strstr( body, "}" ) ;
	if ( end == NULL )
		return false ;
	*end = 0 ;

	// �齨KEYֵ������
	char szkey[128] = {0};
	sprintf( szkey, "%u_%u", atoi( p ) , accesscode ) ;
	return SplitContent( szkey, body ) ;
}

// ��������������
bool CPostQueryMgr::SplitContent( const char *key, char *body )
{
	_PostQuery *obj = new _PostQuery ;
	obj->_index = 0 ;
	obj->_size  = 0 ;

	char *q = NULL ;
	char *p = strstr( body, "[" ) ;
	while( p != NULL ) {
		*p = 0 ;
		p = p + 1 ;
		q = strstr( p , "]" ) ;
		if ( q == NULL ) break ;

		*q = 0 ;
		if ( q > p ) {
			obj->_vec.push_back( p ) ;
			obj->_size = obj->_size + 1 ;
		}
		q = q + 1 ;
		p = strstr( q , "[" ) ;
	}

	if ( obj->_size == 0 ) {
		delete obj ;
		return false ;
	}

	_mutex.lock() ;
	CMapPostQuery::iterator it = _mpPost.find( key ) ;
	if ( it == _mpPost.end() ){
		_mpPost.insert( CMapPostQuery::value_type( key, obj ) ) ;
	} else {
		delete it->second ;
		it->second = obj ;
	}
	_mutex.unlock() ;

	return true ;
}

// ȡ��ƽ̨��ڵ�����
bool CPostQueryMgr::GetPostQuery( int accesscode, unsigned char type, std::string &content )
{
	char szkey[128] = {0} ;
	sprintf( szkey, "%u_%u", type, accesscode ) ;

	_mutex.lock() ;

	CMapPostQuery::iterator it = _mpPost.find( szkey ) ;
	if ( it == _mpPost.end() ){
		_mutex.unlock() ;
		return false ;
	}

	_PostQuery *p = it->second ;
	if ( p->_size == 0 ) {
		_mutex.unlock() ;
		return false ;
	}

	if ( p->_size > 1 ) {
		p->_index = ( p->_index + 1 ) % p->_size ;
	}
	content = p->_vec[p->_index] ;
	_mutex.unlock() ;

	return true ;
}

// ����ƽ̨��ڵ�����
void CPostQueryMgr::ClearPost( void )
{
	_mutex.lock() ;
	if ( _mpPost.empty() ){
		_mutex.unlock() ;
		return ;
	}

	CMapPostQuery::iterator it ;
	for ( it = _mpPost.begin(); it != _mpPost.end(); ++ it ){
		delete it->second ;
	}
	_mpPost.clear() ;
	_mutex.unlock() ;
}



