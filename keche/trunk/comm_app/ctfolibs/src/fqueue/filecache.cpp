/*
 * filecache.cpp
 *
 *  Created on: 2012-4-25
 *      Author: humingqing
 */

#include "filecache.h"
#include <tools.h>
#include <assert.h>
#include <time.h>
#include <filequeue.h>

#define FILECACHE_FAILED   		-1  // ʧ�����
#define FILECACHE_SUCCESS   	0	// �ɹ�
#define FILECACHE_EMPTY  		1	// Ϊ�յ����

CFileCache::CFileCache(IOHandler *handler)
	: _datacache(handler) , _maxsend(0)
{
	_handler = handler ;
}

CFileCache::~CFileCache()
{
}

// ��ʼ��ϵͳ
bool CFileCache::Init( const char *dir, int maxsend )
{
	// ������ٶ�����
	_maxsend = maxsend ;
	// ��ʼ������
	_datacache.Init( dir ) ;

	return true ;
}

// ��������
bool CFileCache::WriteCache( const char *szid , void *buf, int len )
{
	return _datacache.Push( szid , buf, len ) ;
}

// �����û�
void CFileCache::Online( const char *szid )
{
	_mutex.lock() ;
	_userlst.AddUser( szid , _maxsend ) ;
	_mutex.unlock() ;

	// ��������
	_datacache.Load( szid ) ;
}

// �����û�
void CFileCache::Offline( const char *szid )
{
	_mutex.lock() ;
	_userlst.DelUser( szid ) ;
	_mutex.unlock() ;
}

// �߳����ж���
bool CFileCache::Check( void )
{
	_mutex.lock() ;
	CListUser::ListUid &lst = _userlst.ListUser() ;
	if ( lst.empty() ) {
		_mutex.unlock() ;
		return false ;
	}

	time_t now = time(NULL) ;
	// �Ƿ�ɹ�
	bool success = false ;
	CListUser::ListUid::iterator it ;
	// ��Ҫ��v���Ю�ǰ���õ��Ñ�
	for ( it = lst.begin(); it != lst.end(); ) {
		CListUser::_stUid &uid = ( *it ) ;
		// �����ʱ������
		if ( uid.state == USER_ERR_OVERFLUX && uid.now == now ) {
			++ it ;
			continue ;
		}

		// ״̬�Ƿ�û��ȡ������
		if ( uid.state == USER_ERR_EMPTY && now - uid.now < MAX_CHECK_TIME ) {
			++ it ;
			continue ;
		}

		// ��״̬�л�����
		uid.state = USER_ERR_NOMARL ;
		// ��¼��ǰʱ��
		uid.now   = now ;
		// ����������پ���ʱͣһ����ټ�����
		if ( uid.flux.OverSpeed() ) {
			uid.state = USER_ERR_OVERFLUX ;
			++ it ;
			continue ;
		}

		// �������ݴ���
		int ret = _datacache.Pop( uid.userid ) ;
		// ��������
		if ( ret == FILECACHE_EMPTY ){
			uid.state = USER_ERR_EMPTY ;
			++ it ;
			continue ;  // ���û��������Ϣһ��
		} else if ( ret == FILECACHE_FAILED ) {  // ��������ʧ���û�������
			// ɾ������
			_userlst.DelSet( uid.userid ) ;
			lst.erase( it++ ) ;
			continue ;
		}
		success = true ;
		++ it ;
	}
	_mutex.unlock() ;

	return success ;
}

//-------------------------CDataCache ���ݻ������--------------------------------

CDataCache::CDataCache( IOHandler *handler )
{
	_handler = handler ;
}

CDataCache::~CDataCache()
{
	Clear() ;
}

void CDataCache::Init( const char *szroot )
{
	_basedir = szroot ;
}

// �����û�����
void CDataCache::Load( const string &sid )
{
	// ����Ѵ����û�������
	_mutex.lock() ;
	AddNewQueue( sid ) ;
	_mutex.unlock() ;
}

// ���������
bool CDataCache::Push( const string &sid, void *buf, int len )
{
	int ret = 0 ;

	_mutex.lock() ;
	CFileQueue *fqueue = AddNewQueue( sid ) ;
	assert( fqueue != NULL ) ;
	ret = fqueue->push( buf, len ) ;
	_mutex.unlock() ;

	return ( ret == 0 ) ;
}

// ��������
int CDataCache::Pop( const string &sid )
{
	queue_item *item = NULL ;
	{
		share::Guard guard( _mutex ) ;
		{
			if ( _queue.empty() )
				return FILECACHE_EMPTY ;

			CMapQueue::iterator it ;
			it = _queue.find( sid ) ;
			if ( it == _queue.end() ) {
				return true ;
			}

			CFileQueue *fqueue = it->second ;
			item = fqueue->pop() ;
			if ( item == NULL ) {
				// ����ļ�û�����ݾͿ���ɾ����
				fqueue->remove() ;
				_queue.erase( it ) ;
				delete fqueue ;
			}
		}
	}
	// �Ƶ����������������ݣ���Ϊ���ص�����ʧ�ܻ���������
	if ( item != NULL ) {
		// ����ݽ�����
		if ( _handler->HandleQueue( sid.c_str() , &item->data[0] , item->len ) == IOHANDLE_FAILED ) {
			// ���·��������
			Push( sid, &item->data[0] , item->len ) ;
			free( item ) ;
			return FILECACHE_FAILED ;
		}
		free( item ) ;
	}
	return FILECACHE_SUCCESS ;
}

// ����µ����ݶ���
CFileQueue * CDataCache::AddNewQueue( const string &sid )
{
	CMapQueue::iterator it = _queue.find( sid ) ;
	if ( it != _queue.end() ) {
		return it->second ;
	}

	CFileQueue *fqueue = new CFileQueue( (char*)_basedir.c_str(), (char*)sid.c_str() ) ;
	assert( fqueue != NULL ) ;
	_queue.insert( make_pair(sid, fqueue) ) ;
	return fqueue ;
}

// ������������
void CDataCache::Clear( void )
{
	share::Guard guard( _mutex ) ;
	if ( _queue.empty() )
		return ;

	CMapQueue::iterator it ;
	for ( it = _queue.begin(); it != _queue.end(); ++ it ) {
		delete it->second ;
	}
	_queue.clear() ;
}
