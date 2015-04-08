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

	if ( buf == NULL || len <= 0 ) {
		return false ;
	}

	time_t now = time(NULL) ;

	_msg_data *msg = new _msg_data;
	msg->len = len ;
	msg->buf = new char[ len+1 ] ;
	assert( msg->buf != NULL ) ;
	msg->ent = now ;
	memset( msg->buf, 0,  len+1 ) ;
	memcpy( msg->buf, buf, len ) ;

	_map_data.insert( std::pair<std::string,_msg_data*>( key, msg ) ) ;
	_map_index.insert( std::pair<time_t,std::string>( now, key ) ) ;

	return true ;
}

// ȡ������
char * MsgCache::GetData( const char *key, int &len )
{
	share::Guard g( _mutex ) ;

	if ( _map_data.empty() ) {
		return NULL ;
	}

	MapMsgData::iterator it = _map_data.find( key ) ;
	if ( it == _map_data.end() ) {
		return NULL ;
	}

	_msg_data *msg = it->second ;
	_map_data.erase( it ) ;

	MapMsgIndex::iterator itx = _map_index.find( msg->ent ) ;
	while ( itx != _map_index.end() ) {
		if ( itx->second != key ) {
			++ itx ;
			continue ;
		}
		_map_index.erase( itx ) ;
		break ;
	}

	char *ptr = msg->buf ;
	len = msg->len ;
	delete msg ;

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

// �������
bool MsgCache::RemoveData( const char *key )
{
	MapMsgData::iterator it = _map_data.find( key ) ;
	if ( it == _map_data.end() ) {
		return false ;
	}

	_msg_data *msg = it->second ;
	delete [] msg->buf ;
	_map_data.erase( it ) ;
	delete msg ;

	return true ;
}

// ����ʱ������
void MsgCache::CheckData( int timeout )
{
	share::Guard g( _mutex ) ;

	if ( _map_index.empty() ) {
		return ;
	}

	time_t ntime = time( NULL ) - timeout ;

	MapMsgIndex::iterator it, itx;
	for ( it = _map_index.begin(); it != _map_index.end() ; ) {
		if ( it->first < ntime ) {
			itx = it ;
			++ it ;
			// ��������
			_map_index.erase( itx ) ;
			// ��ճ�ʱ����
			RemoveData( itx->second.c_str() ) ;
		}
		break ;
	}
}

void MsgCache::ClearAll( void )
{
	share::Guard g( _mutex ) ;
	if ( _map_data.empty() ) {
		return ;
	}

	MapMsgData::iterator it ;
	for ( it = _map_data.begin(); it != _map_data.end(); ++ it ){
		_msg_data *msg = it->second ;
		delete [] msg->buf ;
		delete msg ;
	}
	_map_data.clear() ;
	_map_index.clear() ;
}

