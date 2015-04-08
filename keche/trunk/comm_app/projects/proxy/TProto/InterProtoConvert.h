/**********************************************
 * InterProtoConvert.h
 *
 *  Created on: 2010-7-17
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments: 2011-06-15 humingqing ��д�˶���
 *********************************************/

#ifndef INTERPROTOCONVERT_H_
#define INTERPROTOCONVERT_H_

#include "ProtoHeader.h"
#include "ProtoParse.h"
#include <vector>
#include <tools.h>
#include <map>
#include <Mutex.h>

#define DEFAULT_SEQ 		"000000_0000000000_0"
#define DEFAULT_MAC_ID 		"0_00000000"
#define MAS_ACCESS_CODE 	"00000000"

#define IMG_JPG   			0x01
#define IMG_GIF   			0x02
#define IMG_TIFF  			0x03

// ���г�ʱ���������MAP���У���Ҫ����Ҫ����ʱ����Ҫ�����Ƿ�Ϊ�պϵ�ϵͳ��
// ������������϶������Ϊ�պ�
class CReqMap
{
	struct _SEQ_DATA
	{
		string _seq ;
		time_t _now ;
	};
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
		map<string,_SEQ_DATA*>::iterator it = _map_seq.find(key) ;
		if ( it != _map_seq.end() ){
			_SEQ_DATA *p = it->second ;
			RemoveIndex( p->_seq, p->_now ) ;
			p->_now = time(NULL) ;
			p->_seq = val ;
			AddIndex( p->_seq, p->_now ) ;
		}else{
			_SEQ_DATA *p = new _SEQ_DATA;
			p->_now = time(NULL) ;
			p->_seq = val ;
			_map_seq.insert( pair<string,_SEQ_DATA*>( key, p ) ) ;
			AddIndex( p->_seq, p->_now ) ;
		}
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
		map<string,_SEQ_DATA*>::iterator itx;
		multimap<time_t,string>::iterator it ,itmp;
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
		map<string, _SEQ_DATA*>::iterator it = _map_seq.find(key);
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
		multimap<time_t,string>::iterator it = _map_index.find(now) ;
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
		map<string, _SEQ_DATA*>::iterator it ;
		for ( it = _map_seq.begin(); it != _map_seq.end(); ++ it ){
			delete it->second ;
		}
		_map_seq.clear() ;
		_map_index.clear() ;
		_map_mutex.unlock() ;
	}

private:
	share::Mutex 		 	 _map_mutex ;
	map<string,_SEQ_DATA*> 	 _map_seq ;    // MAPֵ
	multimap<time_t,string>  _map_index ;  // ʱ������
	unsigned  int			 _seq_count ;  // ����ֵ������
	bool 					 _bindex ;     // �Ƿ���Ҫ����
};

class InterProtoConvert
{
public:
	InterProtoConvert() ;
	virtual ~InterProtoConvert() ;
	// �����ڲ���GPSͷ��
	bool build_inter_header(string &destheader, const string &header, const string &seq, const string &mac_id,
			const string &command, const string &com_access_code );
	// �����ڲ���GPS����
	bool build_inter_proto( string &dest, const string &header, const string &seq, const string &mac_id,
			const string &command, const string &com_access_code, const string &command_value );
	// ���ڲ�������ת����GPS���ʹ���
	bool build_gps_info( string &dest, GnssData *gps_data ) ;
	// ��GPS����ת��GNSS
	bool convert_gps_info( const string &dest, GnssData &gps ) ;
	// ����yootu������
	bool build_yutoo_gps(string &dest, const string &mac_id, GnssData* gps_data);

	// ȡ�ó�����Ψһ��ʶ
	string get_mac_id( const char *device_id, unsigned char device_color ) ;

	// �·���������Ӧ����Ϣ
	bool build_ctrl_msg_text_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const int msgid, const unsigned char result ) ;

	// �����������Ӧ����Ϣ
	bool build_ctrl_msg_monitor_vehicle_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const unsigned char result ) ;

	// ��������Ӧ����Ϣ
	bool build_ctrl_msg_take_photo_ack( string &dest, string &in_seq, const  unsigned int data_type, const string &mac_id,
			const string &access_code, const unsigned char photo_rsp_flag, const char * gnss_data, const unsigned char lens_id,
			const unsigned int photo_len, const unsigned char size_type, const unsigned char type, const char * photo , const int data_len, const char *szpath ) ;

	// �ϱ�������ʻ��¼Ӧ����Ϣ
	bool build_ctrl_msg_take_travel_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, unsigned char command_type,const char *data ) ;

	// ����Ӧ��������ƽ̨Ӧ����Ϣ
	bool build_ctrl_msg_emergency_monitoring_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id ,
			const string &access_code, const unsigned char result ) ;

	// ����ƽ̨���Ӧ��
	bool build_platform_msg_post_query_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &com_access_code, UpPlatformMsgpostqueryData * pMsg, const char *data , const int len ) ;
	// �·�ƽ̨�䱨��Ӧ����ϢPLATFORMMSG
	bool build_platform_msg_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const int msgid ) ;
	// ����������̬��ϢӦ����Ϣ UP_BASE_MSG_VEHICLE_ADDED_ACK
	bool build_base_msg_vehicle_added_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id ,
				const string &access_code , const char *data ) ;

	// ��������Ӧ����ϢUP_WARN_MSG_URGE_TODO_ACK
	bool build_warn_msg_urge_todo_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code , const unsigned char result ) ;
	// �ϱ�������Ϣ��Ϣ
	bool build_warn_msg_adpt_info( string &dest, const string &in_seq, const string &mac_id, const string &access_code, const unsigned char warn_src,
			const unsigned short warn_type , const unsigned long long warn_time , const char *data ) ;

	// �����ϱ�������������Ϣ��Ϣ
	bool build_warn_msg_adpt_todo_info( string &dest, const string &seq, const string &mac_id,
			const string &access_code, const unsigned int info_id, const unsigned char result);

	// ע�ᳵ����Ϣ "����
	// ƽ̨Ψһ����|�����ն˳���Ψһ����|�����ն��ͺ�|�����ն˱��|�����ն�SIM���绰����"
	bool build_exg_msg_register( string &dest, const string &seq , const string &mac_id , const string &access_code, const string &platform_id,
			const string &producer_id, const string &terminal_model_type, const string &terminal_id, const string &terminal_simcode ) ;

	// ������������λ��Ϣ����Ӧ����Ϣ
	bool build_exg_msg_arcossarea_startup_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &device_id ) ;

	// ������������λ��Ϣ����Ӧ����Ϣ
	bool build_exg_msg_arcossarea_end_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &device_id ) ;

	// ����������λ��Ϣ����Ӧ����Ϣ
	bool build_exg_msg_return_startup_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id ,
			const string &access_code ) ;
	// ����������λ��ϢЧ��Ӧ����Ϣ
	bool build_exg_msg_return_end_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id ,
				const string &access_code ) ;

	// ���뽻��ָ��������λ��Ϣ������Ϣ
	bool build_exg_msg_apply_for_monitor_startup( string &dest, const string &seq, const string &mac_id, const string &access_code,
			const unsigned long long start, const unsigned long long end ) ;

	// ȡ������ָ��������λ��Ϣ������Ϣ
	bool build_exg_msg_apply_for_monitor_end( string &dest, const string &seq, const string &mac_id, const string &access_code ) ;

	// ����������λ��Ϣ������Ϣ
	bool build_exg_msg_apply_hisgnssdata_req( string &dest, const string &seq, const string &mac_id, const string &access_code,
			const unsigned long long start, const unsigned long long end ) ;

	// �ϱ�˾�����ʶ����ϢӦ����Ϣ
	bool build_exg_msg_report_driver_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &driver_name ,  const string &driver_id , const string &licence , const string &org_name ) ;

	//	�����ϱ���ʻԱ�����Ϣ��Ϣ
	bool build_exg_msg_report_driver_info(string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &driver_name ,  const string &driver_id , const string &licence , const string &org_name );


	// �ϱ����������˵�Ӧ����Ϣ
	bool build_exg_msg_take_waybill_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const char *data, const int len ) ;

	//	�����ϱ����������˵���Ϣ
	bool build_exg_msg_report_waybill_info( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code , const char *data, const int len );

	// ����MAS�����ƽ̨���
	bool build_mas_platform_msg_post_query_req( string &dest, const string &seq, const string &mac_id,
			const string &access_code, DownPlatformMsgPostQueryBody * pBody, const char *data , const int len ) ;
	// �����·�ƽ̨�䱨��
	bool build_mas_platform_msg_info_req( string &dest, const string &seq, const string &mac_id,
				const string &access_code, DownPlatformMsgInfoReq * pReq, const char *data , const int len ) ;

	// ����MAS����������̬��Ϣ
	bool build_mas_exg_msg_car_info( string &dest, const string &seq, const string &mac_id, const string &access_code, const char *car_info ) ;

	// ����MAS����������λ��Ϣ����
	bool build_mas_exg_msg_return_startup( string &dest, const string &seq, const string &mac_id, const string &access_code, const unsigned char result ) ;

	// ����MAS����������λ��Ϣ����
	bool build_mas_exg_msg_return_end( string &dest, const string &seq, const string &mac_id, const string &access_code, const unsigned char result ) ;

	// ����MAS���뽻��ָ��������λ��Ϣ
	bool build_mas_exg_msg_apply_for_monitor_startup_ack( string &dest, string &seq, const uint16 data_type, const string &mac_id,
			const string &access_code, const uint8 result ) ;
	// ����MASȡ������ָ��������λ��
	bool build_mas_exg_msg_apply_for_monitor_end_ack( string &dest, string &seq, const uint16 data_type, const string &mac_id,
			const string &access_code , const uint8 result ) ;
	// ���Ͳ�����̬���ݴ���
	bool build_mas_down_base_msg_vehicle_added( string &dest, string &seq, const string &mac_id, const string &access_code ) ;

	// ����MASת������ƽ̨�������
	char * convert_mas_dplat( const string &seq, const string &mac_id, const string &company_id, const string &operator_key,
					const string &operate_value, int &len ) ;
	// ����MASת����D_MESG��Ϣ
	char * convert_mas_dmesg( const string &seq, const string &mac_id, const string &company_id, const string &operator_key,
			const string &operate_value, int &len ) ;

	// ת������MAS��ӦЭ������D_BASE
	char * convert_mas_dbase( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len ) ;

	//	ת�����ݣ�������mas��D_CTLM
	char * convert_mas_dctlm(const string &seq, const string &mac_id, const string &company_id,
			const string &operator_key, const string &operate_value, int &len );	

	// ת�����ݶ�ӦЭ������D_CTLM
	char * convert_dctlm( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len ) ;

	// ת�����ݶ�ӦЭ������D_BASE
	char * convert_dbase( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len ) ;

	// ת�����ݶ�ӦЭ������D_WARN
	char * convert_dwarn( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len ) ;

	// ת�����ݶ�ӦЭ������D_PLAT
	char * convert_dplat( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len ) ;

	// ת�����ݶ�ӦЭ������D_MESG
	char * convert_dmesg( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
				const string &operate_value, int &len ) ;

	// �ͷ�����
	void release( char *&data ) ;

	// ����ʱ��������
	void clear_timeout_sequeue( const unsigned int timeout ) ;

private:
	// ����ƽ̨�ڲ����
	const string build_platform_out_seq( const string &mac_id, unsigned short data_type ) ;

	// ���GPS��������
	void append_gps( string &dest, const string &data ) ;

	// ��������λ��Ϣ����Ӧ����Ϣ
	bool build_exg_msg_arcossarea_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &device_id , const int ack_type ) ;
	// ����ͨ�����ݸ�ʽ
	bool build_common_dctlm_data( string &dest, const string &header, string &in_seq, const string &mac_id, const unsigned int msg_type,
			const string &access_code, const string &command, const string &data ) ;

private:
	// �ڲ�ƽ̨���ⲿ���ж�Ӧת����ϵ
	CReqMap      _seq_map ;
};

#endif /* INTERPROTOCONVERT_H_ */
