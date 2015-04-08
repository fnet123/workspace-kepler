/*
 * filecache.cpp
 *
 *  Created on: 2011-12-21
 *      Author: humingqing
 */

#include "filecacheex.h"
#include <tools.h>
#include <assert.h>
#include <time.h>
#include <filequeue.h>

#define FILECACHE_FAILED   		-1  // ʧ�����
#define FILECACHE_SUCCESS   	0	// �ɹ�
#define FILECACHE_EMPTY  		1	// Ϊ�յ����

CFileCacheEx::CFileCacheEx(IOHandler *handler):
	_inited(false), _datacache(handler), _arcosscache(handler)
{
	_handler = handler ;
}

CFileCacheEx::~CFileCacheEx()
{
	Stop() ;
}

// ��ʼ��ϵͳ
bool CFileCacheEx::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char temp[256] = {0};
	if ( ! pEnv->GetString( "base_filedir" , temp ) ) {
		printf( "load base_filedir failed\n" ) ;
		return false ;
	}

	char sz[512] = {0} ;
	getenvpath( temp, sz );

	char buf[1024] = {0};
	sprintf( buf, "%s/cache", sz ) ;
	// ��ʼ������
	_datacache.Init( buf ) ;

	// ��ʼ������ʱ����
	sprintf( buf, "%s/arcoss", sz ) ;
	_arcosscache.Init( buf ) ;

	_thread.init( 1, NULL, this ) ;

	_inited = true ;

	return true ;
}

// ����ϵͳ
bool CFileCacheEx::Start( void )
{
	_thread.start() ;
	return true ;
}

// ֹͣϵͳ
void CFileCacheEx::Stop( void )
{
	if ( ! _inited )
		return ;
	_inited = false ;
	_thread.stop() ;
}

// ��������
bool CFileCacheEx::WriteCache( int areacode, void *buf, int len )
{
	char szid[128] = {0} ;
	sprintf( szid, "%u", areacode ) ;
	return _datacache.Push( szid, buf, len ) ;
}

// ��ӿ�����Ϣ
void CFileCacheEx::AddArcoss( int areacode, char color, char *vechile )
{
	_arcosscache.Push( areacode, color, vechile ) ;
}

// ɾ��������Ϣ
void CFileCacheEx::DelArcoss( int areacode, char color, char *vechile )
{
	_arcosscache.Pop( areacode, color, vechile ) ;
}

// �޸Ŀ���ʱ��
void CFileCacheEx::ChgArcoss( int areacode, char color, char *vechile )
{
	_arcosscache.Change( areacode, color, vechile ) ;
}

// �����û�
void CFileCacheEx::Online( int areacode )
{
	char szid[128] = {0} ;
	sprintf( szid, "%u", areacode ) ;

	_userlst.AddUser( areacode ) ;
	// ��������
	_datacache.Load( szid ) ;
	// ��������
	_arcosscache.Load( areacode ) ;
}

// �����û�
void CFileCacheEx::Offline( int areacode )
{
	_userlst.DelUser( areacode ) ;
	// �������ʱ����
	_arcosscache.Offline( areacode ) ;
}

// �߳����ж���
void CFileCacheEx::run( void *param )
{
	while( _inited ){

		int uid = 0 ;
		if ( ! _userlst.PopUser(uid) ) {
			sleep(10) ;
			continue ;
		}

		char szid[128] = {0} ;
		sprintf( szid, "%u", uid ) ;
		// �������ݴ���
		int ret = _datacache.Pop( szid ) ;
		// ��������
		if ( ret == FILECACHE_EMPTY ){
			sleep(1) ;  // ���û��������Ϣһ��
		} else if ( ret == FILECACHE_FAILED ) {  // ��������ʧ���û�������
			_userlst.DelUser( uid ) ;
			continue ;
		}
		// ��������������
		_arcosscache.Check( uid ) ;
	}
}

//----------------------------����������ݲ���-----------------------------------
CArcossCache::CArcossCache( IOHandler *handler ):_handler(handler)
{

}

CArcossCache::~CArcossCache()
{
	Clear() ;
}

// �����ļ�·��
void CArcossCache::Init( const char *szroot )
{
	_basedir = szroot ;
}

// ���߼�������
void CArcossCache::Load( int id )
{
	// ���л�
	unserialize( id ) ;
	serialize( id ) ;
}

// ���������
void CArcossCache::Push( int id, char color, char *vechile )
{
	if ( AddNew(id, color, vechile) ) {
		// ���л�����
		serialize( id ) ;
	}
}

// ���������
bool CArcossCache::AddNew( int id, char color, char *vechile )
{
	share::Guard guard( _mutex ) ;

	return AddData(id, color, vechile, (uint64_t)time(NULL) , VECHILE_ON ) ;
}

// ����������
void CArcossCache::Pop( int id, char color, char *vechile )
{
	if ( Remove(id, color, vechile) ) {
		// ���л�����
		serialize( id ) ;
	}
}

// �Ƴ�����
bool CArcossCache::Remove( int id, char color, char *vechile )
{
	share::Guard gurad( _mutex ) ;

	ArcossData *p = RemoveIndex( id, color, vechile ) ;
	if ( p == NULL ) {
		return false ;
	}

	CMapArcoss::iterator it = _mapArcoss.find( id ) ;
	if ( it == _mapArcoss.end() ) {
		return false ;
	}

	ArcossHeader *header = it->second ;
	if ( header->_tail == p && header->_head == p ) { // ֻ��һ��Ԫ��
		_mapArcoss.erase( it ) ;
		deletefile( id ) ;
		delete header ;
	} else { // �ж��Ԫ�ص����
		if ( p == header->_head ){ // λ��ͷ������
			header->_head = p->_next ;
			if ( p->_next != NULL ) {
				p->_next->_pre = NULL ;
			}
		} else if ( p == header->_tail ){ // λ��β�������
			header->_tail = p->_pre ;
			if ( p->_pre != NULL ) {
				p->_pre->_next = NULL ;
			}
		} else { // λ���м�����
			p->_pre->_next = p->_next ;
			p->_next->_pre = p->_pre ;
		}
		header->_size = header->_size - 1 ;
	}
	delete p ;

	return true ;
}

// �޸�����ʱ��
void CArcossCache::Change( int id, char color, char *vechile )
{
	// ����޸ĳɹ���Ҫ���л�
	if ( ChangeEx( id, color, vechile ) ) {
		serialize( id ) ;
	}
}

// �޸�����
bool CArcossCache::ChangeEx( int id, char color, char *vechile )
{
	share::Guard guard( _mutex ) ;

	char carnum[VECHILE_LEN+1] = {0};
	safe_memncpy( carnum, vechile, VECHILE_LEN ) ;

	char key[512] = {0};
	sprintf( key, "%d_%d_%s", id, color, carnum ) ;

	CMapIndex::iterator itx = _mapIndex.find( key ) ;
	if ( itx == _mapIndex.end() ) {
		return false ;
	}

	ArcossData *p = itx->second ;
	p->time = time(NULL) ;

	return true ;
}

// ���������
void CArcossCache::Offline( int id )
{
	share::Guard guard( _mutex ) ;
	if ( _mapArcoss.empty() )
		return ;

	CMapArcoss::iterator it = _mapArcoss.find( id ) ;
	if ( it == _mapArcoss.end() ) {
		return ;
	}

	ArcossHeader *header = it->second ;
	if ( header->_size > 0 ) {
		ArcossData *p = header->_head ;
		while( p != NULL ) {
			p->state = VECHILE_OFF ;
			p = p->_next ;
		}
	}
}

// �������
bool CArcossCache::Check( int id )
{
	if ( ! CheckEx( id ) ) {
		return false;
	}
	serialize( id ) ;
	return true ;
}

bool CArcossCache::CheckEx( int id )
{
	share::Guard guard( _mutex ) ;
	if ( _mapArcoss.empty() )
		return false;

	CMapArcoss::iterator it = _mapArcoss.find( id ) ;
	if ( it == _mapArcoss.end() )
		return false;

	ArcossHeader *header = it->second ;
	if ( header->_size == 0 ) {
		_mapArcoss.erase( it ) ;
		delete header ;
		deletefile( id ) ;
		return false ;
	}

	char szid[128] = {0} ;
	sprintf( szid, "%u", id ) ;

	int count = 0 ;

	ArcossData *p = header->_head ;
	while( p != NULL ) {
		if ( p->state != VECHILE_OFF ) {
			p = p->_next ;
			continue ;
		}
		ArcossData *temp = p ;
		p = p->_next ;

		// ���ⷢ�����ݴ�����,����ص���粻����������
		if ( _handler->HandleQueue( szid , temp, sizeof(ArcossData) , DATA_ARCOSSDAT ) == IOHANDLE_FAILED ){
			// �����������ʧ�ܾ�ֱ�ӷ�����
			break ;
		}

		// �Ƴ�����
		RemoveIndex( id, temp->color, temp->vechile ) ;

		// ֻ��һ��Ԫ��
		if ( temp == header->_head && temp == header->_tail ){
			_mapArcoss.erase( it ) ;
			deletefile( id ) ;
			delete header ;
			delete temp ;
			break ;
		}

		// �ж��Ԫ�ص����
		if ( temp == header->_head ){ // λ��ͷ������
			header->_head = temp->_next ;
			if ( temp->_next != NULL ) {
				temp->_next->_pre = NULL ;
			}
		} else if ( temp == header->_tail ){ // λ��β�������
			header->_tail = temp->_pre ;
			if ( temp->_pre != NULL ) {
				temp->_pre->_next = NULL ;
			}
		} else { // λ���м�����
			temp->_pre->_next = temp->_next ;
			temp->_next->_pre = temp->_pre ;
		}
		header->_size = header->_size - 1 ;
		delete temp ;

		++ count ;
	}
	return ( count > 0 ) ;
}

// �Ƴ�����
ArcossData *CArcossCache::RemoveIndex( int id, char color , char *vechile )
{
	char carnum[VECHILE_LEN+1] = {0};
	safe_memncpy( carnum, vechile, VECHILE_LEN ) ;

	char key[512] = {0};
	sprintf( key, "%d_%d_%s", id, color, carnum ) ;

	CMapIndex::iterator itx = _mapIndex.find( key ) ;
	if ( itx == _mapIndex.end() ) {
		return NULL ;
	}
	ArcossData *p = itx->second ;
	_mapIndex.erase( itx ) ;

	return p ;
}

// ɾ���ļ�
void CArcossCache::deletefile( int id )
{
	char key[128] = {0};
	sprintf( key, "%d", id ) ;

	CFileQueue file( (char*)_basedir.c_str(), key ) ;
	file.remove() ;
}

// ���л�����
void CArcossCache::serialize( int id )
{
	share::Guard guard( _mutex ) ;

	if ( _mapArcoss.empty() ) return ;

	CMapArcoss::iterator it = _mapArcoss.find( id ) ;
	if ( it == _mapArcoss.end() ) {
		return ;
	}

	char key[128] = {0};
	sprintf( key, "%d", id ) ;

	CFileQueue file( (char*)_basedir.c_str(), key ) ;
	file.remove() ;

	ArcossHeader *header = it->second ;
	if ( header->_size > 0 ) {
		ArcossData *p = header->_head ;
		while( p != NULL ) {
			file.push( p, sizeof(ArcossData) ) ;
			p = p->_next ;
		}
	}
}

// �����л�����
void CArcossCache::unserialize( int id )
{
	share::Guard guard( _mutex ) ;

	char key[128] = {0};
	sprintf( key, "%d", id ) ;

	CFileQueue file( (char*)_basedir.c_str(), key ) ;

	queue_item *item = file.pop() ;
	while( item != NULL ) {
		ArcossData *p = ( ArcossData *) item->data ;
		// ��ӵ�������
		AddData( id, p->color, p->vechile, p->time , VECHILE_OFF ) ;
		free( item ) ;
		item = file.pop() ;
	}
}

// ���������
bool CArcossCache::AddData( int id, char color, char *vechile , uint64_t time , char state )
{
	char carnum[VECHILE_LEN+1] = {0};
	safe_memncpy( carnum, vechile, VECHILE_LEN ) ;

	char key[512] = {0};
	sprintf( key, "%d_%d_%s", id, color, carnum ) ;

	CMapIndex::iterator itx = _mapIndex.find( key ) ;
	if ( itx != _mapIndex.end() ) {
		return false;
	}

	ArcossData *p = new ArcossData ;
	p->areacode = id ;
	p->color    = color ;
	safe_memncpy( p->vechile, vechile, VECHILE_LEN ) ;
	p->time     = time ;
	p->state    = state ;
	p->_next 	= NULL ;
	p->_pre  	= NULL ;

	CMapArcoss::iterator it = _mapArcoss.find( id ) ;
	if ( it == _mapArcoss.end() ) {
		ArcossHeader *header = new ArcossHeader ;
		header->_size = 1 ;
		header->_head = header->_tail = p ;
		_mapArcoss.insert( make_pair( id, header ) ) ;
	} else {
		ArcossHeader *header = it->second ;
		header->_tail->_next = p ;
		p->_pre				 = header->_tail ;
		header->_tail		 = p ;
		header->_size        = header->_size + 1 ;
	}
	_mapIndex.insert( make_pair(key, p) ) ;

	return true ;
}

// ��������
void CArcossCache::Clear( void )
{
	share::Guard guard( _mutex ) ;
	if ( _mapArcoss.empty() )
		return ;

	CMapArcoss::iterator it ;
	for ( it = _mapArcoss.begin(); it != _mapArcoss.end(); ++ it ) {
		ArcossHeader *header = it->second ;
		if ( header->_size > 0 ) {
			ArcossData *p = header->_head ;
			while( p->_next != NULL ) {
				p = p->_next ;
				delete p->_pre ;
			}
			delete p ;
		}
		delete header ;
	}
	_mapArcoss.clear() ;
	_mapIndex.clear() ;
}


