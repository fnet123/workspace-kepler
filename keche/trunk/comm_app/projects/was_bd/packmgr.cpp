/**
 * Author: humingqing
 * Date:   2011-10-14
 */
#include "packmgr.h"
#include <comlog.h>
#include <GBProtoParse.h>

CPackMgr::CPackMgr()
{
	_lastcheck = time(NULL) ;
}

CPackMgr::~CPackMgr()
{
	share::Guard g( _mutex ) ;
	{
		_mapPack.clear() ;
	}
}

// ��ӵ����ݰ�����
bool CPackMgr::AddPack( DataBuffer &pack, const char *carid, const int msgid,
		const int index, const int count, const int seq, const char *buf, int len )
{
	share::Guard g( _mutex ) ;
	{
		if ( count <= 0 || index <= 0 ) {
			OUT_ERROR( NULL, 0, "packmgr" , "msg id %x, add pack car id %s, count %d, index %d error" , msgid, carid, count, index ) ;
			return false ;
		}

		char key[1024] = {0} ;
		// ����ID�����ֻ����Լ���ϢID�ͷְ���
		sprintf( key, "%s-%d-%d-%d" , carid, msgid, count , (seq-index)+1 ) ;

		CMemFile *p = NULL ;

		CMapFile::iterator it = _mapPack.find( key ) ;
		if ( it == _mapPack.end() ) {
			p = new CMemFile(key) ;
			if ( p == NULL ) {
				OUT_ERROR( NULL, 0, "packmgr" , "msg id %x, add pack car id %s, count %d, index %d malloc data faild" , msgid, carid, count, index ) ;
				return false ;
			}
		} else {
			p = it->second ;
			// �Ƴ�����Ҫʱ������
			p = _queue.erase( p ) ;
		}

		// ����ɹ���������ݺϳ�һ�����ݰ���ֱ�Ӵ��ڴ�ɾ��
		if ( p->AddBuffer( pack, index, count, buf, len ) ) {
			// ��ӡ������Ϣ
			OUT_INFO( NULL, 0, "packmgr" , "msg id %x, build msg pack success", msgid ) ;
			// ֻ�в�Ϊһ�����ݳ���Ϣ�Ż�������
			if ( it != _mapPack.end() ) {
				// �Ӷ������Ƴ�
				_mapPack.erase( it ) ;
			}
			delete p ;
			return true ;
		}

		// ֻ�е�һ�����ʱ����ӵ�������
		if ( it == _mapPack.end() ) {
			_mapPack.insert( pair<string,CMemFile*>( key, p ) ) ;
		}
		// ���ʱ������
		_queue.push( p ) ;

		return false ;
	}
}

// ����ʱ�����ݰ�
void CPackMgr::CheckTimeOut( unsigned int timeout )
{
	share::Guard g( _mutex ) ;
	{
		if ( _queue.size() == 0 ){
			return ;
		}

		time_t now = time(NULL) ;
		if ( now - _lastcheck < LONG_PACK_CHECK ) {
			return ;
		}
		_lastcheck = now ;

		time_t pass = now - timeout ;
		CMemFile *t,*p = _queue.begin() ;
		while( p != NULL ) {
			t = p ;
			p = _queue.next( p ) ;

			if ( t->GetLastTime() > pass ) {
				break ;
			}
			RemovePack( t->GetId() ) ;
		}
	}
}

// �Ƴ����ݶ���
void CPackMgr::RemovePack( const string &key )
{
	CMapFile::iterator it = _mapPack.find( key ) ;
	if ( it == _mapPack.end() ) {
		return ;
	}
	CMemFile *p = it->second ;
	_mapPack.erase( it ) ;
	delete _queue.erase( p ) ;
}

CPackMgr::CMemFile::CMemFile( const char *id ):_cur( 0 )
{
	_last = time(NULL) ;
	_next = _pre = NULL ;
	_id   = id ;
}

CPackMgr::CMemFile::~CMemFile()
{
	if ( _vec.empty() ) {
		return ;
	}
	for ( size_t i = 0; i < _vec.size(); ++ i ) {
		delete _vec[i] ;
	}
	_vec.clear() ;
}

// ����ڴ�����
bool CPackMgr::CMemFile::AddBuffer( DataBuffer &pack, const int index, const int count, const char *buf, int len )
{
	// ���Ϊһ�����ݾͲ���Ҫ���������
	if ( count == 1 ) {
		pack.writeBlock( buf, len ) ;
		return true ;
	}

	// �����û�п������ݿռ�
	if ( _vec.empty() ) {
		for ( int i = 0; i < count; ++ i ) {
			_vec.push_back( new DataBuffer ) ;
		}
	}

	// �������ȷ��INDEX�Ͳ�������
	if ( index > (int) _vec.size() ) {
		return false ;
	}

	// ȡ�ô�����ݿ���ڴ�
	DataBuffer *p = _vec[index-1] ;
	assert( p != NULL ) ;
	if ( p->getLength() == 0 ) {
		++ _cur ;
	} else {
		p->resetBuf() ;
	}
	p->writeBlock( buf, len ) ;

	// ����������ݰ��ɹ�
	if ( _cur >= (unsigned int) count ) {
		// ȡ�÷ְ�����
		int size = _vec.size() ;
		// д���һ�����ݰ���ͷ��
		pack.writeBlock( p->getBuffer() , sizeof(GBheader) ) ;
		// ���¿�ʼ������������ݾ�ֻ������ͷ��������
		for ( int i = 0; i < size ; ++ i ) {
			p = _vec[i] ;
			if ( p->getLength() < (int)sizeof(GBheader) ){
				continue ;
			}
			unsigned short mlen = p->fetchInt16( 3 ) & 0x03FF;
			if ( mlen > 0 ) {
				pack.writeBlock( p->getBuffer() + sizeof(GBheader) + 4 , mlen ) ;
			}
		}
		// ���β������
		GBFooter footer ;
		pack.writeBlock( &footer, sizeof(footer) ) ;

		return true ;
	}

	// �������һ�η��ʵ�ʱ��
	_last = time(NULL) ;

	OUT_INFO( NULL, 0, "packmgr" , "save index file %d, current count %d total %d" , index, _cur, count ) ;

	return false ;
}
