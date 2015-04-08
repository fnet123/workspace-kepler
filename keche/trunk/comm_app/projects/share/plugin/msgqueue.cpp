/*
 * msgqueue.cpp
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 *  ��Ϣ�ȴ�����
 */

#include "msgqueue.h"
#include <comlog.h>

CMsgQueue::CMsgQueue( IMsgCaller *caller ) : _caller(caller)
{

}

CMsgQueue::~CMsgQueue()
{
	Clear() ;
}

// ��Ӷ���
bool CMsgQueue::AddObj( unsigned int seq, unsigned int fd, unsigned int cmd, const char *id, IPacket *msg )
{
	share::Guard guard( _mutex ) ;

	_MsgObj *obj = new _MsgObj ;
	obj->_seq = seq ;
	obj->_fd  = fd ;
	obj->_cmd = cmd ;
	obj->_id  = id ;
	obj->_msg = msg ;
	obj->_now = time(NULL) ;

	msg->AddRef() ;

	_queue.push( obj ) ;
	_index.insert( make_pair(seq, obj ) ) ;

	return true ;
}

// ȡ�ö�Ӧ����
CMsgQueue::_MsgObj * CMsgQueue::GetObj( unsigned int seq )
{
	share::Guard guard( _mutex ) ;
	CMapObj::iterator it = _index.find( seq ) ;
	if ( it == _index.end() )
		return NULL ;

	_MsgObj *obj = it->second ;
	_index.erase( it ) ;

	return _queue.erase( obj ) ;
}

// �ͷŶ���
void CMsgQueue::FreeObj( CMsgQueue::_MsgObj *obj )
{
	if ( obj == NULL )
		return ;

	obj->_msg->Release() ;
	delete obj ;
}

// ɾ������
bool CMsgQueue::Remove( unsigned int seq )
{
	_MsgObj *obj = GetObj( seq ) ;
	if ( obj == NULL )
		return false ;

	FreeObj( obj ) ;

	return true ;
}

// �Ƴ���Ӧ��ֵ
void CMsgQueue::RemoveValue( unsigned int seq , bool callback )
{
	CMapObj::iterator it = _index.find( seq ) ;
	if ( it == _index.end() )
		return ;
	_MsgObj *obj = it->second ;
	_index.erase( it ) ;
	_queue.erase( obj ) ;

	// �����Ҫ�ص�����
	if ( callback ) {
		// �ύ�ⲿ����
		_caller->OnTimeOut( obj->_seq, obj->_fd, obj->_cmd, obj->_id.c_str(), obj->_msg ) ;
	}
	FreeObj( obj ) ;
}


//  ��ⳬʱ�Ķ���
void CMsgQueue::Check( int timeout )
{
	share::Guard guard( _mutex ) ;

	if ( _queue.size() == 0 )
		return ;

	time_t now = time(NULL) ;

	_MsgObj *t = NULL ;
	_MsgObj *p = _queue.begin() ;
	while( p != NULL ) {
		t = p ;
		p = p->_next ;
		if ( now - t->_now < timeout )
			break ;
		// �Ƴ�����
		RemoveValue( t->_seq, true ) ;
	}
}

// �������
void CMsgQueue::Clear( void )
{
	share::Guard guard( _mutex ) ;

	int size = 0 ;
	_MsgObj *p = _queue.move(size) ;
	if ( size == 0 )
		return ;

	while( p != NULL ) {
		p = p->_next ;
		FreeObj( p->_pre ) ;
	}
	_index.clear();
}



