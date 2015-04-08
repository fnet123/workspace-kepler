#include "msgcache.h"
#include <assert.h>

////////////////////////////////// ��Ϣ���ݻ���  ////////////////////////////////////////
MsgCache::MsgCache()
{

}
MsgCache::~MsgCache()
{
	ClearAll() ;
}
// �������
bool MsgCache::AddData( const char *key, const char *buf, const int len )
{
	share::Guard g( _mutex ) ;

	if ( buf == NULL || len <= 0 || key == NULL ) {
		return false ;
	}

	time_t now = time(NULL) ;

	_msg_data *msg = new _msg_data;
	msg->key = key ;
	msg->len = len ;
	msg->buf = new char[ len+1 ] ;
	assert( msg->buf != NULL ) ;
	msg->ent = now ;
	memset( msg->buf, 0,  len+1 ) ;
	memcpy( msg->buf, buf, len ) ;

	_queue.push( msg ) ;
	_index.insert( std::pair<std::string,_msg_data*>( key, msg ) ) ;

	return true ;
}

// ȡ������
char * MsgCache::GetData( const char *key, int &len , bool erase )
{
	share::Guard g( _mutex ) ;

	if ( _queue.size() == 0 ) {
		return NULL ;
	}

	MapMsgData::iterator it = _index.find( key ) ;
	if ( it == _index.end() ) {
		return NULL ;
	}

	_msg_data *msg = it->second ;

	_queue.erase( msg ) ;
	if ( erase ) {
		_index.erase( it ) ;
	}

	char *ptr = msg->buf ;
	len = msg->len ;

	if ( erase ) {
		delete msg ;
	} else {
		// ���Ϊ��ɾ�������͸������һ�ε�ʱ��
		msg->ent = time(NULL) ;
		_queue.push( msg ) ;
	}
	return ptr;
}

// �ͷ�����
void MsgCache::FreeData( char *data )
{
	if ( data == NULL ) {
		return ;
	}
	delete [] data ;
	data = NULL ;
}

// �Ƴ�����
bool MsgCache::Remove( const char *key )
{
	if ( key == NULL )
		return false ;

	int   len = 0 ;
	char *ptr = GetData( key , len ) ;
	if ( ptr == NULL || len == 0 )
		return false ;

	delete [] ptr ;
	return true ;
}

// ����ʱ������
void MsgCache::CheckData( int timeout )
{
	share::Guard g( _mutex ) ;

	if ( _queue.size() == 0 ) {
		return ;
	}

	time_t ntime = time( NULL ) - timeout ;

	_msg_data *t = NULL ;
	_msg_data *p = _queue.begin() ;
	while( p != NULL ) {
		t = p ;
		p = p->_next ;

		if ( t->ent > ntime )
			break ;

		// ��ճ�ʱ����
		if ( ! t->key.empty() ) {
			_index.erase( t->key ) ;
		}
		if ( t->buf != NULL ) {
			delete [] t->buf ;
		}
		delete _queue.erase( t ) ;
	}
}

void MsgCache::ClearAll( void )
{
	share::Guard g( _mutex ) ;

	int size = 0 ;
	_msg_data *p = _queue.move(size) ;
	if ( size == 0 ) {
		return ;
	}

	while( p != NULL ) {
		p = p->_next ;
		if ( p->_pre->buf ) {
			delete [] p->_pre->buf ;
		}
		delete p->_pre;
	}

	_index.clear() ;
}

