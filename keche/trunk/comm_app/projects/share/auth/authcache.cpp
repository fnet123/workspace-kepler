/*
 * authcache.cpp
 *
 *  Created on: 2012-5-14
 *      Author: humingqing
 *  memo: ��Ȩ�־û��Ĵ���
 */


#include "authcache.h"
#include <databuffer.h>
#include <tools.h>
#include <comlog.h>

CAuthCache::CAuthCache()
{
	_change = 0 ;
	_last   = 0 ;
}

CAuthCache::~CAuthCache()
{
	Clear() ;
}

// ���ؼ�Ȩע���ļ�  4C54|15001088478|13344444444
bool CAuthCache::LoadFile( const char *filename )
{
	share::Guard guard( _mutex ) ;

	_filename = filename ;

	return UnSerialize() ;
}

// �����Ȩ��Ϣ
bool CAuthCache::AddAuth( const char *oem, const char *phone, const char *authcode )
{
	share::Guard guard( _mutex ) ;

	if ( oem == NULL || phone == NULL || authcode == NULL ) {
		OUT_ERROR( NULL, 0, "AuthCache", "param error" ) ;
		return false ;
	}

	_stAuth * p = NULL ;
	CMapAuth::iterator it = _mpAuth.find( phone ) ;
	if ( it == _mpAuth.end() ) {
		p = new _stAuth;
		_queue.push( p ) ;
		_mpAuth.insert( make_pair( phone, p ) ) ;
		_change = _change + 1 ;
	}else {
		p = it->second ;
		if ( strcmp( oem , p->oem ) != 0 || strcmp( authcode, p->authcode ) != 0 ) {
			_change = _change + 1 ;
		}
	}
	safe_memncpy( p->phone 	  , phone    , sizeof(p->phone) ) ;
	safe_memncpy( p->oem   	  , oem      , sizeof(p->oem) ) ;
	safe_memncpy( p->authcode , authcode , sizeof(p->authcode) ) ;

	p->time = time(NULL) ;

	return true ;
}

// ��ʱ�������л�����
void CAuthCache::Check( int timeout )
{
	time_t now = time( NULL) ;
	if ( now - _last < timeout ) {
		return ;
	}
	_last = now ;

	share::Guard guard( _mutex ) ;

	if ( _change == 0 )
		return ;
	_change = 0 ;

	Serialize() ;
}


// ��������Ȩ
int CAuthCache::TermAuth( const char *phone, const char *auth, CQString &ome , time_t n )
{
	share::Guard guard( _mutex ) ;

	if ( _mpAuth.empty() ){
		return AUTH_ERR_FAILED;
	}

	CMapAuth::iterator it = _mpAuth.find( phone ) ;
	if ( it == _mpAuth.end() ){
		return AUTH_ERR_FAILED ;
	}

	_stAuth *p = it->second ;
	if ( auth != NULL ) {
		if ( strcmp( p->authcode, auth ) != 0 ){
			OUT_ERROR( NULL, 0, phone , "car auth code  %s current auth code %s" , auth , p->authcode ) ;
			return AUTH_ERR_FAILED ;
		}
	}
	ome = p->oem ;

	// ����Ȩ�Ƿ�ʱ
	if ( n > 0 ) {
		// �����ʱ��ֱ�ӷ�����
		if ( time(NULL) - p->time > n ) {
			return AUTH_ERR_TIMEOUT ;
		}
	}
	return AUTH_ERR_SUCCESS;
}

// �ӻ�����ȡ��OME��
bool CAuthCache::GetCache( const char *phone, CQString &ome )
{
	return ( TermAuth( phone, NULL, ome ) != AUTH_ERR_FAILED ) ;
}

// ��������
void CAuthCache::Clear( void )
{
	share::Guard guard( _mutex ) ;

	if ( _mpAuth.empty() ){
		return ;
	}

	Serialize() ;

	_mpAuth.clear() ;
}

// �Ƴ��������OEMֵ
void CAuthCache::Remove( const char *phone )
{
	share::Guard guard( _mutex ) ;
	{
		CMapAuth::iterator it = _mpAuth.find( phone ) ;
		if ( it == _mpAuth.end() ) {
			return ;
		}
		_stAuth *p = it->second ;
		// �������
		_mpAuth.erase( it ) ;
		// ɾ������
		delete _queue.erase( p ) ;
		// �������л�һ��
		Serialize() ;
	}
}

// ���л�����
bool CAuthCache::Serialize( void )
{
	if ( _filename.IsEmpty() ) {
		OUT_ERROR( NULL, 0, "AuthCache", "serialize filename empty" ) ;
		return false ;
	}

	if ( _mpAuth.empty() ) {
		OUT_ERROR( NULL, 0, "AuthCache", "serialize data empty" ) ;
		return false ;
	}

	// ɾ���Ѵ����ļ�
	if ( access( _filename.GetBuffer(), 0 ) == 0 )
		unlink( _filename.GetBuffer() ) ;

	_stAuth *p = _queue.begin() ;
	while( p != NULL ) {
		AppendFile( _filename.GetBuffer(), (const char*)p, sizeof(_stAuth) ) ;
		p = p->_next ;
	}
	return true ;
}

// �����л�����
bool CAuthCache::UnSerialize( void )
{
	if ( _filename.IsEmpty() ) {
		OUT_ERROR( NULL, 0, "AuthCache", "unserialize filename empty" ) ;
		return false ;
	}

	int   len = 0 ;
	char *ptr = ReadFile( _filename.GetBuffer() , len ) ;
	if ( ptr == NULL ) {
		OUT_ERROR( NULL, 0, "AuthCache", "unserialize filename not exist " ) ;
		return false ;
	}

	DataBuffer buf ;
	buf.writeBlock( ptr, len ) ;
	FreeBuffer( ptr ) ;

	int count = 0 ;

	// OME , PHONE, AUTHCODE
	int pos = 0 ;
	while ( pos < len ) {
		_stAuth *info = new _stAuth;
		if ( ! buf.readBlock( info, sizeof(_stAuth) ) ){
			delete info ;
			break ;
		}
		pos += sizeof(_stAuth) ;

		CMapAuth::iterator it = _mpAuth.find( info->phone ) ;
		if ( it != _mpAuth.end() ) {
			delete info ;
			continue ;
		}
		_queue.push( info ) ;
		_mpAuth.insert( make_pair( info->phone, info ) ) ;

		++ count ;
	}

	return ( count > 0 ) ;
}




