/*
 * pccsession.cpp
 *
 *  Created on: 2011-03-02
 *      Author: humingqing
 */

#include "pccsession.h"
#include <tools.h>

CPccSession::CPccSession()
{
}

CPccSession::~CPccSession()
{
}

// ��������
bool CPccSession::Load( const char *file )
{
	// area_code:ome_phone:color_num
	int   len = 0 ;
	char *ptr = ReadFile( file, len ) ;
	if ( ptr == NULL || len == 0 ) {
		return false ;
	}

	for( int i = 0; i < len; ++ i ){
		if ( ptr[i] != '\r' )
			continue ;
		ptr[i] = '\n' ;
	}

	vector<string> vec ;
	if ( ! splitvector( ptr , vec, "\n", 0 ) ) {
		FreeBuffer( ptr ) ;
		return false ;
	}

	// �����û�����,  4C54_15001088478:WZ:1:A1:18:��E-Y8888
	for ( int i = 0; i < (int)vec.size(); ++ i ) {
		string &temp = vec[i] ;
		if ( temp.empty() )
			continue ;

		vector<string> vk ;
		if ( ! splitvector( temp, vk, ":" , 6 ) ){
			continue ;
		}

		_stCarInfo info ;
		info.macid       = vk[0] ;
		info.areacode    = vk[1] ;
		info.color       = vk[2] ;
		info.carmodel    = vk[3] ;
		info.vehicletype = vk[4] ;
		info.vehiclenum  = vk[5] ;

		_mgr.AddCarInfo( info ) ;
	}

	FreeBuffer( ptr ) ;
	return true ;
}

// ȡ������MAC����
bool CPccSession::GetCarMacData( string &s )
{
	return _mgr.GetAllMacId( s ) ;
}

// �����ֻ�MACȡ�ó���
bool CPccSession::GetCarInfo( const char *key, _stCarInfo &info )
{
	if ( !_mgr.GetCarInfo( key, info, true ) ) {
		// ����Ự�д�����ֱ�Ӵ���
		return false ;
	}
	return true ;
}

// ���ݳ���ȡ�ö�Ӧ���ֻ�MAC
bool CPccSession::GetCarMacId( const char *key, char *macid )
{
	_stCarInfo info ;
	if ( !_mgr.GetCarInfo( key, info, false ) ) {
		return false ;
	}
	// ȡ������
	sprintf( macid, "%s", info.macid.c_str() ) ;

	return true ;
}

CPccSession::CCarInfoMgr::CCarInfoMgr()
{
	_head = _tail = NULL ;
	_size = 0 ;
}

CPccSession::CCarInfoMgr::~CCarInfoMgr()
{
	ClearAll() ;
}

// ��ӳ�����Ϣ
bool CPccSession::CCarInfoMgr::AddCarInfo( _stCarInfo &info )
{
	share::Guard guard( _mutex ) ;

	_stCarList *p = new _stCarList ;
	p->info = info ;
	p->next = p->pre = NULL ;

	if ( _head == NULL ) {
		_tail = _head = p ;
		_size = 1 ;
	} else {
		_tail->next = p ;
		p->pre      = _tail ;
		_tail       = p ;
		_size       = _size + 1 ;
	}

	_stCarList *temp = NULL ;

	CMapCarInfo::iterator it ;
	it = _phone2car.find( info.macid ) ;
	if ( it != _phone2car.end() ) {
		temp = it->second ;
		_phone2car.erase( it ) ;
	}

	it = _vehice2car.find( info.vehiclenum ) ;
	if ( it != _vehice2car.end() ) {
		_vehice2car.erase( it ) ;
	}
	if ( temp ) RemoveNode( temp ) ;

	_phone2car.insert( make_pair( info.macid, p ) ) ;
	_vehice2car.insert( make_pair(info.vehiclenum, p ) ) ;

	return true ;
}

// ȡ�ó�����Ϣ
bool CPccSession::CCarInfoMgr::GetCarInfo( const string &key, _stCarInfo &info , bool byphone )
{
	share::Guard guard( _mutex ) ;

	CMapCarInfo::iterator it ;
	if ( byphone ) {
		it = _phone2car.find( key ) ;
		if ( it == _phone2car.end() )
			return false ;
		info = (it->second)->info ;
	} else {
		it = _vehice2car.find( key ) ;
		if ( it == _vehice2car.end() )
			return false ;
		info = (it->second)->info ;
	}

	return true ;
}

// �Ƴ�������Ϣ
void CPccSession::CCarInfoMgr::RemoveInfo( const string &key , bool byphone )
{
	share::Guard guard( _mutex ) ;

	_stCarList *temp = NULL ;
	CMapCarInfo::iterator it ;

	if ( byphone ) {
		it = _phone2car.find( key ) ;
		if ( it == _phone2car.end() ) {
			return ;
		}
		temp = it->second ;
		_phone2car.erase( it ) ;

		it = _vehice2car.find(  temp->info.vehiclenum ) ;
		if ( it != _vehice2car.end() ) {
			_vehice2car.erase( it ) ;
		}
	} else {
		it = _vehice2car.find( key ) ;
		if ( it == _vehice2car.end() ) {
			return ;
		}
		temp = it->second ;
		_vehice2car.erase( it ) ;

		it = _phone2car.find( temp->info.macid ) ;
		if ( it != _phone2car.end() ) {
			_phone2car.erase( it ) ;
		}
	}
	RemoveNode( temp ) ;
}

// �Ƴ������
void CPccSession::CCarInfoMgr::RemoveNode( _stCarList *p )
{
	if ( p == _head ) { // ���Ϊͷ�ڵ�
		_head = p->next   ;
		_head->pre = NULL ;
		if ( _head == NULL )
			_tail = NULL ;
	} else if ( _tail == p ){ // ���Ϊβ���
		_tail = p->pre ;
		_tail->next = NULL ;
	} else { // ����м�ڵ�
		p->pre->next = p->next ;
		p->next->pre = p->pre  ;
	}
	_size = _size - 1 ;

	delete p ;
}

// ȡ������MACID
bool CPccSession::CCarInfoMgr::GetAllMacId( string &s )
{
	share::Guard guard( _mutex ) ;

	if ( _head == NULL )
		return false ;

	_stCarList *temp, *p = _head ;
	while( p != NULL ) {
		if ( ! s.empty() ){
			s += "," ;
		}
		s += p->info.macid ;
		p = p->next ;
	}
	return ( !s.empty() ) ;
}

// �����������
void CPccSession::CCarInfoMgr::ClearAll( void )
{
	share::Guard guard( _mutex ) ;

	if ( _head == NULL )
		return ;

	_stCarList *temp, *p = _head ;
	while( p != NULL ) {
		temp = p ;
		p = p->next ;
		delete temp ;
	}

	_head = _tail = NULL ;
	_size = 0 ;

	_phone2car.clear() ;
	_vehice2car.clear() ;
}



