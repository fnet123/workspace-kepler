/*
 * datapool.cpp
 *
 *  Created on: 2012-5-25
 *      Author: humingqing
 */

#include "dbpool.h"
#include <comlog.h>
#include "dbfacemgr.h"
#include "mongodb.h"
#include "oracledb.h"

////////////////////////////////// ������������ȡ����  //////////////////////////////////
// �������ݶ���ȡ�ö�Ӧ������
CSqlObj* IDataPool::GetSqlObj(DB_TYPE type)
{
	CSqlObj *obj = NULL ;
	switch( type ) {
	case IDataPool::mongo:
		obj = new MongoDBSqlObj;
		break ;
	case IDataPool::oracle:
		obj = new OracleSqlObj;
		break ;
	case IDataPool::mysql:
		break ;
	case IDataPool::redis:
		break ;
	}
	return obj ;
}

/**
// ȡ�ý�����Ķ���
CSqlResult *IDataPool::GetSqlResult(DB_TYPE type)
{
	CSqlResult *rs = NULL ;
	switch( type ) {
	case IDataPool::mongo:
		break ;
	case IDataPool::oracle:
		rs = new OracleResult;
		break ;
	case IDataPool::mysql:
		break ;
	case IDataPool::redis:
		break ;
	}
	return rs ;
}*/

///////////////////////////////////// CSqlWhere����  /////////////////////////////////////////
// ���WHERE����
void CSqlWhere::AddWhere( const string &key, const string &val , TOPER op , TWHERE rel )
{
	_WhereVal where ;
	where._rel  = rel ;
	where._skey = key ;
	where._sval = val ;
	where._op   = op ;
	_wlst.push_back( where ) ;
}

// ȡ�ù�ϵ����
bool CSqlWhere::GetWhereList( CWhereList &w ) const
{
	if ( _wlst.empty() )
		return false ;

	w = _wlst ;
	return true ;
}

// ���WEHERE��LIST
void CSqlWhere::ClearWhereList()
{
	if(!_wlst.empty()){
		_wlst.clear();
	}
}

// ��SQLWHERE�������л�
void CSqlWhere::Seralize( DataBuffer &buf )
{
	int nsize = _wlst.size();
	// д��WHERE��������
	buf.writeInt16( nsize );
	if ( nsize > 0 ) {
		CWhereList::iterator it;
		// �������е��������
		for ( it = _wlst.begin(); it != _wlst.end() ; ++ it ) {
			_WhereVal &w = * it;

			buf.writeInt8( ( unsigned char ) w._rel );

			buf.writeInt16( w._skey.length() );
			if ( w._skey.length() > 0 ){
				buf.writeBlock( w._skey.c_str(), w._skey.length() );
			}
			buf.writeInt8( ( unsigned char ) w._op );

			buf.writeInt16( w._sval.length() );
			if ( w._sval.length() > 0 ){
				buf.writeBlock( w._sval.c_str(), w._sval.length() );
			}
		}
	}
}

// �����ݶ������л�
void CSqlWhere::UnSeralize( const char *ptr, int len )
{
	CPacker pack( ptr, len ) ;

	int nsize  = pack.readShort() ;

	if ( nsize > 0 ) {
		char szbuf[1024] = {0} ;
		unsigned short nlen = 0 ;
		// ��ȡ����������
		for ( int i = 0; i < nsize; ++ i ) {
			_WhereVal val ;
			val._rel = (TWHERE) pack.readByte() ;

			nlen = pack.readShort() ;
			if ( ! pack.readBytes( szbuf, nlen ) )
				break ;
			val._skey.assign( szbuf , nlen ) ;

			val._op = (TOPER) pack.readByte() ;

			nlen = pack.readShort() ;
			if ( ! pack.readBytes( szbuf, nlen ) )
				break ;
			val._sval.assign( szbuf, nlen ) ;

			_wlst.push_back( val ) ;
		}
	}
}

///////////////////////////////// CSqlUnit /////////////////////////////////////////

CSqlUnit::CSqlUnit( IDataPool::DB_TYPE type, CSqlData *data , unsigned int groupid )
{
	_dbtype  = type ;
	_groupid = groupid ;
	_sqldata = data ;
}

CSqlUnit::CSqlUnit( const char *ptr, int len )
{
	UnSeralize( ptr, len ) ;
}

CSqlUnit::~CSqlUnit()
{
	if ( _sqldata != NULL ){
		delete _sqldata ;
		_sqldata = NULL ;
	}
}

// ���л����ݶ���
void CSqlUnit::Seralize( DataBuffer &buf )
{
	buf.writeInt8( (unsigned char)_dbtype ) ;
	buf.writeInt32( _groupid ) ;
	buf.writeInt8( (unsigned char) _sqldata->oper ) ;

	buf.writeInt16( _sqldata->table.length() ) ;
	buf.writeBlock( _sqldata->table.c_str(), _sqldata->table.length() ) ;

	if ( _sqldata->oper == IDataPool::dothing )
		return ;

	DataBuffer tmp ;
	_sqldata->sql_obj->Seralize( tmp ) ;
	buf.writeInt32( tmp.getLength() ) ;
	buf.writeBlock( tmp.getBuffer(), tmp.getLength() ) ;

	tmp.resetBuf() ;
	if ( _sqldata->oper == IDataPool::update ) {
		_sqldata->sql_where->Seralize( tmp ) ;
		buf.writeInt32( tmp.getLength() ) ;
		buf.writeBlock( tmp.getBuffer(), tmp.getLength() ) ;
	}
}

// ���������ݶ���
void CSqlUnit::UnSeralize( const char *ptr, int len )
{
	CPacker pack( ptr, len ) ;

	int offset  = 0 ;
	_dbtype  = ( IDataPool::DB_TYPE ) pack.readByte() ;
	offset += sizeof(char) ;

	_groupid = pack.readInt() ;
	offset += sizeof(int) ;

	_sqldata = new CSqlData;
	_sqldata->oper = (IDataPool::DB_OPRE) pack.readByte() ;
	offset += sizeof(char) ;

	char szbuf[1024] = {0} ;
	unsigned short nlen = pack.readShort() ;
	offset += sizeof(short) ;

	if ( nlen > 0 ) {
		pack.readBytes( szbuf, nlen ) ;
	}
	_sqldata->table.assign( szbuf , nlen) ;
	offset += nlen ;

	_sqldata->sql_obj = IDataPool::GetSqlObj( _dbtype ) ;

	unsigned int dlen = pack.readInt() ;
	offset += sizeof(int) ;
	if ( dlen > 0 ) {
		_sqldata->sql_obj->UnSeralize( ptr + offset , dlen ) ;
	}
	offset += dlen ;
	pack.seekRead( offset ) ;  // ��λ��ȡ�������

	// ���Ϊ���²�������WEHERE����
	if ( _sqldata->oper == IDataPool::update ) {
		dlen = pack.readInt() ;
		offset += sizeof(int) ;
		if ( dlen > 0 ) {
			_sqldata->sql_where = new CSqlWhere ;
			_sqldata->sql_where->UnSeralize( ptr+offset, dlen ) ;
		}
	}
}

///////////////////////////////// ���ݿ����ӳع������   ///////////////////////////////////////
CDbPool::CDbPool():_inited(false)
{

}

CDbPool::~CDbPool()
{
	Stop() ;
	Clear() ;
}

// ��ʼ�����ݿ����Ӷ���
bool CDbPool::Init( void )
{
	if ( ! _thread.init( 1, NULL, this ) ) {
		return false ;
	}
	_inited = true ;
	return true ;
}

// �������ݿ����Ӷ���
bool CDbPool::Start( void )
{
	if ( ! _inited )
		return false ;

	_thread.start() ;
	return true ;
}

// ֹͣ�������Ӷ���
bool CDbPool::Stop( void )
{
	if ( ! _inited )
		return false;

	_inited = false ;
	_monitor.notifyEnd() ;
	_thread.stop() ;
	return true ;
}

void CDbPool::run( void *param )
{
	while( _inited ) {

		_mutex.lock() ;
		if ( _queue.size() > 0 ){
			ObjList *p = _queue.begin() ;
			while( p != NULL ) {
				// ����Ƿ�ʱ
				p->Check( 120 ) ;
				p = p->_next ;
			}
		}
		_mutex.unlock() ;

		share::Synchronized syn(_monitor);
		{
			_monitor.wait( 30 ) ;
		}
	}
}

IDBFace * CDbPool::CheckOut( const char *connstr )
// ǩ�����ݲ�������,�������Ӵ�������������
{
	unsigned int key = CDBFaceMgr::GetHash( connstr ) ;

	IDBFace *pface = NULL;
	{
		share::Guard guard(_mutex);
		CMapObj::iterator it = _index.find(key);
		if (it != _index.end()) {
			pface = it->second->GetObj();
			if (pface != NULL) {
				return pface;
			}
		}
	}
	// ȡ�����ݿ����Ӷ���
	pface = CDBFaceMgr::GetDBFace(connstr, key);

	// ǩ������
	return pface;
}

// ǩ�����ݲ�������
void CDbPool::CheckIn( IDBFace *obj )
{
	share::Guard guard( _mutex ) ;
	// ȡ�ö�Ӧ�����Ӵ���Hashֵ
	unsigned int key = obj->GetId() ;

	ObjList *p = NULL ;
	CMapObj::iterator it = _index.find( key ) ;
	if ( it == _index.end() ) {
		p = new ObjList ;
		_queue.push( p ) ;
		_index.insert( make_pair(key, p ) ) ;
	} else {
		p = it->second ;
	}
	p->AddObj( obj ) ;
}

// �ͷ����Ӷ���
void CDbPool::Remove(IDBFace *obj)
{
	if ( obj == NULL )
		return ;
	obj->Release() ;
}

// ������ж���
void CDbPool::Clear( void )
{
	share::Guard guard( _mutex ) ;

	int size = 0 ;
	ObjList *p = _queue.move(size) ;
	if ( size == 0 )
		return ;

	while( p != NULL ) {
		p = p->_next ;
		delete p->_pre ;
	}
}

//=========================== ObjList ====================================
// ������ݿ����
void CDbPool::ObjList::AddObj( IDBFace *pface )
{
	_DataObj *p = new _DataObj;
	p->_pFace   = pface ;
	p->_time    = time(NULL) ;
	p->_next    = p->_pre = NULL ;
	_queue.push( p ) ;
}

// ȡ�����ݿ����
IDBFace *CDbPool::ObjList::GetObj( void )
{
	if ( _queue.size() == 0 )
		return NULL ;

	_DataObj *p = _queue.begin() ;
	IDBFace *pface = p->_pFace ;
	Remove( p , false ) ;

	return pface ;
}

// ��ⳬʱ�Ķ���
void CDbPool::ObjList::Check( int timeout )
{
	if ( _queue.size() == 0 )
		return ;

	time_t now = time(NULL) - timeout ;
	// ����Ƿ�ʱ
	_DataObj *tmp,*p = _queue.begin() ;
	while( p != NULL ){
		tmp = p ;
		p   = p->_next ;
		if ( now < tmp->_time )
			break ;

		Remove( tmp , true ) ;
	}
}

// �Ƴ����ݶ���
void CDbPool::ObjList::Remove( _DataObj *p , bool release )
{
	p = _queue.erase( p ) ;
	// �Ƿ��ͷ�����
	if ( release ) {
		// �ͷŶ���
		p->_pFace->Release() ;
	}
	// ɾ��ָ��
	delete p ;
}

// ���ȫ������
void CDbPool::ObjList::Clear(void )
{
	int size = 0 ;
	_DataObj *p = _queue.move(size) ;
	if ( size == 0  )
		return ;

	// ������ж���
	_DataObj *tmp ;
	while( p != NULL ) {
		tmp = p ;
		p = p->_next ;
		if ( tmp != NULL ) {
			tmp->_pFace->Release() ;
		}
		delete tmp ;
	}
}


