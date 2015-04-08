/**********************************************
 * pccutil.h
 *
 *  Created on: 2011-08-04
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments:
 *********************************************/

#ifndef __PCONVERT_H__
#define __PCONVERT_H__

#include <map>
#include <string>
#include <Mutex.h>
#include "ProtoHeader.h"
#include <Session.h>
#include <databuffer.h>
using namespace std ;

#ifdef PHONE_LEN
#undef PHONE_LEN
#endif
#define PHONE_LEN 11

// ���г�ʱ���������MAP���У���Ҫ����Ҫ����ʱ����Ҫ�����Ƿ�Ϊ�պϵ�ϵͳ��
// ������������϶������Ϊ�պ�
class CReqMap
{
	struct _SEQ_DATA
	{
		string _seq ;
		time_t _now ;
	};
	typedef multimap<string,_SEQ_DATA*>  CSeqMap ;
	typedef multimap<time_t,string>		 CSeqIndex ;
public:
	CReqMap(bool bIndex=false):_seq_count(0) {
		_bindex = bIndex ;
	} ;
	~CReqMap(){
		ClearData() ;
	} ;

	// ����޷�������������ֵ
	void AddNumReqMap( const unsigned int key, const string &val ){
		char buf[128] = {0} ;
		sprintf( buf, "%d", key ) ;
		AddReqMap( buf, val ) ;
	}

	// ����ַ�����ֵ
	void AddReqMap( const string &key, const string &val )
	{
#ifdef _XDEBUG
		printf( "Start add key %s value %s\n", key.c_str(), val.c_str() ) ;
#endif
		_map_mutex.lock() ;
		CSeqMap::iterator it = _map_seq.find(key) ;
		/**
		if ( it != _map_seq.end() ){
			_SEQ_DATA *p = it->second ;
			RemoveIndex( p->_seq, p->_now ) ;
			p->_now = time(NULL) ;
			p->_seq = val ;
			AddIndex( p->_seq, p->_now ) ;
		}else{
		*/
			_SEQ_DATA *p = new _SEQ_DATA;
			p->_now = time(NULL) ;
			p->_seq = val ;
			_map_seq.insert( pair<string,_SEQ_DATA*>( key, p ) ) ;
			AddIndex( p->_seq, p->_now ) ;
		/**}*/
		_map_mutex.unlock() ;
#ifdef _XDEBUG
		printf( "Add map key %s value %s\n", key.c_str(), val.c_str() ) ;
#endif
	}

	bool RemoveNumKey( const unsigned int key ) {
		char buf[128] = {0} ;
		sprintf( buf, "%d", key ) ;
		return RemoveKey( buf ) ;
	}

	bool RemoveKey( const string &key ){
		string val;
		return FindReqMap( key, val, true ) ;
	}

	void ClearKey( const unsigned int timeout ){
		// �������Ҫ�����������ڱպϴ�������Ҫ��ʱ����
		if ( ! _bindex ) return ;

		time_t now = time(NULL) ;
		_map_mutex.lock() ;
		if ( _map_index.empty() ){
			// ���Ϊ����ֱ�ӷ�����
			_map_mutex.unlock() ;
			return ;
		}

		// ����ʹ��ʱ����Ϊ����MAP�����Զ�����Ĺ����������ʱ�����Ȼ����ǰ�棬��ΪMAP�����Ƚ���
		CSeqMap::iterator itx;
		CSeqIndex::iterator it ,itmp;
		for ( it = _map_index.begin(); it != _map_index.end(); ){
			if ( now - it->first < timeout ){
				break ;
			}
			itmp = it ;
			++ it ;

			// �Ƴ���ʱ������
			itx = _map_seq.find( itmp->second ) ;
			if ( itx != _map_seq.end() ){
				delete itx->second ;
				_map_seq.erase( itx ) ;
			}

			_map_index.erase( itmp ) ;
		}
		_map_mutex.unlock() ;
	}

	bool FindNumReqMap( const unsigned int key, string &val , bool berase ){
		char buf[128] = {0} ;
		sprintf( buf, "%d", key ) ;
		return FindReqMap( buf , val , berase ) ;
	}

	bool FindReqMap( const string &key, string &val , bool berase )
	{
#ifdef _XDEBUG
		printf( "start find key %s\n", key.c_str() ) ;
#endif
		bool bFind = false ;
		_map_mutex.lock() ;
		if ( _map_seq.empty() ){
			_map_mutex.unlock() ;
			return false ;
		}
		CSeqMap::iterator it = _map_seq.find(key);
		if (it != _map_seq.end()){
			_SEQ_DATA *p = it->second ;
			val = p->_seq ;
			if ( berase ){
				// �Ƴ�����
				RemoveIndex( p->_seq, p->_now ) ;
				// �ƶ�ԭ����
				_map_seq.erase( it ) ;
				// �����ڴ�
				delete p ;
			}
			bFind = true;
		}
		_map_mutex.unlock() ;
#ifdef _XDEBUG
		printf( "find key %s val %s\n", key.c_str(), val.c_str() ) ;
#endif
		return bFind;
	}

	unsigned int GetSequeue( void ) {
		// ��Ҫ�������л�����
		_map_mutex.lock() ;
		if (_seq_count > 1000 * 10000)
			_seq_count = 0;
		++ _seq_count ;
		_map_mutex.unlock() ;
		return _seq_count;
	}

private:
	void AddIndex( const string &key, const time_t now ){
		if( !_bindex ) return ;
		// ���ʱ������
		_map_index.insert(pair<time_t,string>( now, key ) ) ;
	}
	void RemoveIndex( const string &key, const time_t now ){
		if ( !_bindex )
			return ;
		CSeqIndex::iterator it = _map_index.find(now) ;
		if ( it == _map_index.end() ){
			return  ;
		}
		for ( ; it != _map_index.end(); ){
			if ( it->first != now ) {
				break ;
			}
			if ( it->second == key ){
				_map_index.erase( it ) ;
				break ;
			}
			++ it ;
		}
	}
	void ClearData( void ) {
		_map_mutex.lock() ;
		if ( _map_seq.empty() ){
			_map_mutex.unlock() ;
			return ;
		}
		CSeqMap::iterator it ;
		for ( it = _map_seq.begin(); it != _map_seq.end(); ++ it ){
			delete it->second ;
		}
		_map_seq.clear() ;
		_map_index.clear() ;
		_map_mutex.unlock() ;
	}

private:
	share::Mutex 	_map_mutex ;
	CSeqMap  		_map_seq ;    // MAPֵ
	CSeqIndex  	 	_map_index ;  // ʱ������
	unsigned  int	_seq_count ;  // ����ֵ������
	bool 			_bindex ;     // �Ƿ���Ҫ����
};

class PConvert
{
	class CSequeue
	{
	public:
		CSequeue():_seq_id(0) {}
		~CSequeue() {}

		// ȡ������
		unsigned int get_next_seq( void ) {
			share::Guard g( _mutex ) ;
			if ( _seq_id >= 0xffffffff ) {
				_seq_id = 0 ;
			}
			return ++ _seq_id ;
		}

	private:
		// ����������
		share::Mutex  _mutex ;
		// ����ID��
		unsigned int  _seq_id ;
	};
	typedef std::map<std::string,std::string>   MapString;
public:
	PConvert() ;
	~PConvert() ;
	// ת������ָ�����
	bool convert_ctrl( const string &seq, const string &macid, const string &line, const string &vechile,
			DataBuffer &dbuf , string &acode ) ;
	// �������ı���Ϣ
	bool convert_sndm( const string &seq, const string &macid, const string &line, const string &vechile,
			DataBuffer &dbuf, string &acode ) ;

	// �����ڲ�Э��
	bool BuildMonitorVehicleResp( const string &macid, UpCtrlMsgMonitorVehicleAck *moni , string &data ) ;
	// ������Ƭ�ϴ�
	bool BuildUpCtrlMsgTakePhotoAck( const string &macid, const string &path, const char *data, int len , string &sdata ) ;
	// ת���·��ı�Ӧ��
	bool BuildUpCtrlMsgTextInfoAck( const string &macid, UpCtrlMsgTextInfoAck *text, string &sdata ) ;
	// ����λ������
	bool BuildUpRealLocation( const string &macid, UpExgMsgRealLocation *upmsg, string &sdata ) ;
	// ������ʷ�����ϴ�
	bool BuildUpHistoryLocation( const string &macid, const char *data, int len , int num, string &sdata ) ;
	// ���MACID��������Ĺ�ϵ
	void AddMac2Access( const string &macid, const string &accessid ) { _macid2access.AddSession( macid, accessid ) ; }
	// ȡ���ֻ��ź�OME��
	bool get_phoneome( const string &macid, string &phone, string &ome ) ;
	// ͨ��������MAC��ȡ�ó��ƺź���ɫ
	bool get_carinfobymacid( const string &macid, unsigned char &carcolor, string &carnum ) ;
	// ȡ����Ŷ�Ӧ��ϵ
	unsigned int get_next_seq( void ) { return _seq_gen.get_next_seq(); }
	// ��ⳬʱ����
	void CheckTime( int timeout ) { _reqmap.ClearKey(timeout); }
public:
	// ��GnssDataת�ɼ������
	static void build_gps_info( string &dest, GnssData *gps_data ) ;

private:
	// ת��GPS����
	bool convert_gps_info( MapString &map, GnssData &gps ) ;
	// �����صĲ���
	bool parse_jkpt_value( const std::string &param, MapString &val ) ;
	// ȡ�ö�ӦMAP�ַ�ֵ
	bool get_map_string( MapString &map, const std::string &key , std::string &val ) ;
	// ȡ�ö�ӦMAP������ֵ
	bool get_map_integer( MapString &map, const std::string &key , int &val ) ;
	// ȡ��ͷ����
	bool get_map_header( const std::string &param, MapString &val, int &ntype ) ;
	// ������ݵ�MAP
	bool split2map( const std::string &s , MapString &val ) ;

private:
	// ����������
	CSequeue  	 _seq_gen ;
	// ���ش��ͼƬ·��
	string 		 _picdir ;
	// ����복�ƶ�Ӧ��ϵ
	CReqMap		 _reqmap ;
	// �ֻ��Ž�����
	CSessionMgr  _macid2access;
};

#endif
