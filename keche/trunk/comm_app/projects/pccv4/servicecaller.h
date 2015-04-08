/*
 * servicecaller.h
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */

#ifndef __SERVICECALLER_H_
#define __SERVICECALLER_H_

#include "interface.h"
#include <map>
#include <vector>
#include <string>
#include <Mutex.h>
#include <httpcaller.h>
#include <Session.h>

using namespace std ;

// ҵ�����ģ�鸺��ʵ��HTTP����
class CServiceCaller: public ICallResponse
{
	// ����KEY VALUE
	struct _KeyValue{
		string key ;
		string val ;
	};

	// KEY VALUE����
	class CKeyValue
	{
	public:
		CKeyValue() {}
		~CKeyValue(){}
		// ȡ��Ԫ�صĸ���
		int GetSize() { return (int)_vec.size(); }
		// ȡ��KEY VALUE��ֵ
		_KeyValue &GetVal(int index) { return _vec[index]; }
		// ����ֵ����
		void SetVal( const char *key, const char *val ) {
			_KeyValue vk ;
			vk.key = key ;
			vk.val = val ;
			_vec.push_back( vk ) ;
		}
	private:
		// ������ݵ�����
		vector<_KeyValue> _vec ;
	};

	// ������ϢID��Ӧ����IDӳ��
	class CSeq2Msg
	{
		// ǰһ��Ϊ����ID����Ӧ����ϢID
		typedef map<unsigned int , unsigned int>  MapSeq2Msg;
	public:
		CSeq2Msg() {};
		~CSeq2Msg(){};

		// �����Ŷ�Ӧ��ϵ
		void AddSeqMsg( unsigned int seq, unsigned int msgid ) {
			share::Guard guard( _mutex ) ;
			_seq2msgid[seq] = msgid ;
		}

		// ȡ����Ŷ�Ӧ��Ϣӳ���ϵ
		bool GetSeqMsg( unsigned int seq , unsigned int &msgid ) {
			share::Guard guard( _mutex ) ;

			MapSeq2Msg::iterator it = _seq2msgid.find( seq ) ;
			if ( it == _seq2msgid.end() ) {
				return false ;
			}
			msgid = it->second ;
			_seq2msgid.erase( it ) ;

			return true ;
		}

		// �Ƴ�ӳ��
		void RemoveSeq( unsigned int seq ) {
			share::Guard guard( _mutex ) ;

			MapSeq2Msg::iterator it = _seq2msgid.find( seq ) ;
			if ( it == _seq2msgid.end() ) {
				return;
			}
			_seq2msgid.erase( it ) ;
		}

	private:
		// �������кŶ�Ӧ��Ϣ��������
		MapSeq2Msg				_seq2msgid ;
		// �������Ĳ���
		share::Mutex			_mutex ;
	};

	// ��Ҫ������Ķ�����
	class CSeq2Key
	{
		typedef map<unsigned int,string> MapSeq2Key ;
	public:
		CSeq2Key(){}
		~CSeq2Key(){}

		// �����Ŷ�Ӧ��ϵ
		void AddSeqKey( unsigned int seq, const string &key ) {
			share::Guard guard( _mutex ) ;
			_seq2key[seq] = key ;
		}

		// ȡ����Ŷ�Ӧ��Ϣӳ���ϵ
		bool GetSeqKey( unsigned int seq , string &key ) {
			share::Guard guard( _mutex ) ;

			MapSeq2Key::iterator it = _seq2key.find( seq ) ;
			if ( it == _seq2key.end() ) {
				return false ;
			}
			key = it->second ;
			_seq2key.erase( it ) ;

			return true ;
		}

	private:
		MapSeq2Key    _seq2key ;
		share::Mutex  _mutex ;
	};

	// �������HTTP���ôλ���
	class CSeqMacRef
	{
		typedef map<unsigned int,string> MapSeq2Mac ;
		typedef map<string,time_t>		 MapMac2Ref ;
	public:
		CSeqMacRef():_timeout(120){};
		~CSeqMacRef(){};

		// �������
		bool Add( const char *macid, unsigned int seq ) {
			share::Guard guard( _mutex ) ;
			// ���ʱ��С���򲻴���
			if ( _timeout < 0 ) return true ;
			// ����HTTP���ô������ʱ��
			MapMac2Ref::iterator it = _mac2ref.find( macid ) ;
			if ( it != _mac2ref.end() ) {
				// �������120���ڲ������ظ�����
				if ( time(NULL) - it->second  < _timeout ) {
					return false ;
				}
				// ����ʱ�򲻹�������������
				_mac2ref.erase( it ) ;
			}
			// �����������
			_seq2mac.insert( make_pair(seq, macid) ) ;

			return true ;
		}

		// �����Ƿ���ó���
		void Dec( unsigned int seq , bool err ){
			share::Guard guard( _mutex ) ;
			// ���ʱ��С���㲻����
			if ( _timeout < 0 ) return ;
			// ���ҵ�ǰ��Ŷ�ӦMAC
			MapSeq2Mac::iterator it = _seq2mac.find( seq ) ;
			if ( it == _seq2mac.end() )
				return ;

			// ������������账�����ʱ��������
			if ( err )
				_mac2ref.insert( make_pair( it->second, time(NULL) ) ) ;
			// �Ƴ�����
			_seq2mac.erase( it ) ;
		}
		// ����HTTP���ô���ʱ��
		void SetTimeOut( int timeout ) { _timeout = timeout ;}

	private:
		// ��¼SEQ��Ӧ��MAC
		MapSeq2Mac	 _seq2mac ;
		// ��¼MAC��Ӧ�����ô���
		MapMac2Ref	 _mac2ref ;
		// ����HTTP�������ʱ��
		int 		 _timeout ;
		// ͬ������������
		share::Mutex _mutex ;
	};

	typedef bool (CServiceCaller::*ServiceFun)( unsigned int seq, const char *xml ) ;
	typedef map<unsigned int , ServiceFun>  ServiceTable;

public:
	CServiceCaller() ;
	~CServiceCaller() ;
	// ��ʼ��
	bool Init( ISystemEnv *pEnv ) ;
	// ����
	bool Start( void ) ;
	// ֹͣ
	void Stop( void );
	// ������
	void RemoveCache( const char *key ) ;
	// ����ǳ�ʱ�Ļ�������
	void CheckTimeOut( void ) ;
	// ȡ��ע����Ϣ������ֻ�����Ȩʱ����
	bool getRegVehicleInfo( unsigned int msgid, unsigned int seq, const char *phoneNum, const char *terminaltype ) ;
	// ͨ�ò�ѯ��ͨ�����ƺźͳ�����ɫȡ���ֻ�����ome��Ķ�Ӧ��ϵ
	bool getTernimalByVehicleByType( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor ) ;
	// ͨ�ò�ѯ��ͨ�����ƺźͳ�����ɫȡ���ֻ�����ome��Ķ�Ӧ��ϵ,�·���Ϣ����
	bool getTernimalByVehicleByTypeEx( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor , const char *text ) ;
	// ͨ�ò�ѯ��ͨ���ֻ��ź�oem��ȡ�ö�Ӧ�����Լ�ʡ���ϵ
	bool getBaseVehicleInfo( unsigned int msgid, unsigned int seq, const char *phone  , const char *ome ) ;
	// ��ȡ�ü�ʻԱʶ����Ϣ
	bool getDriverOfVehicleByType( unsigned int msgid, unsigned int seq, const char *vehicleno , const char *vehicleColor ) ;
	// ��ȡ�õ����˵�����
	bool getEticketByVehicle( unsigned int msgid, unsigned int seq, const char *vehicleno , const char *vehicleColor ) ;
	// ��ȡ������̬��Ϣ������
	bool getDetailOfVehicleInfo( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor ) ;
	// �������ӵ�״̬
	bool updateConnectState( unsigned int msgid, unsigned int seq, int areaId , int linkType , int status ) ;

	// ����ƽ̨��ڵ���Ϣ
	bool addForMsgPost( unsigned int msgid, unsigned int seq, const char *messageContent , const char * messageId ,
			const char *objectId, const char *objectType , const char *areaId ) ;
	// �����·�ƽ̨�ı���
	bool addForMsgInfo(unsigned int msgid, unsigned int seq, const char *messageContent , const char * messageId ,
			const char *objectId, const char *objectType , const char *areaId ) ;
	// ����������
	bool addMsgUrgeTodo( unsigned int msgid, unsigned int seq, const char *supervisionEndUtc , const char *supervisionId ,
			const char * supervisionLevel , const char * supervisor , const char *supervisorEmail , const char *supervisorTel ,
			const char * vehicleColor, const char *vehicleNo , const char *wanSrc , const char *wanType , const char *warUtc ) ;
	// ������Ԥ��
	bool addMsgInformTips( unsigned int msgid, unsigned int seq, const char *alarmDescr, const char *alarmFrom,
			const char *alarmTime, const char *alarmType , const char *vehicleColor , const char *vehicleNo ) ;

	// ����HttpCaller�ص�
	void ProcessResp( unsigned int seqid, const char *xml, const int len , const int err ) ;

private:
	// ���Ϊһ��ͳһ���÷�ʽ����
	bool ProcessMsg( unsigned int msgid, unsigned int seq , const char *service, const char *method , CKeyValue &item ) ;
	// ��������XML����
	bool CreateRequestXml( CQString &sXml, const char *id, const char *service, const char *method , CKeyValue &item ) ;

protected:
	// ����ע�������
	bool Proc_UP_EXG_MSG_REGISTER( unsigned int seq, const char *xml ) ;
	// ����λ���ϱ�����
	bool Proc_UP_EXG_MSG_REAL_LOCATION( unsigned int seq, const char *xml ) ;
	// �����ϱ���ʻԱ���ʶ��
	bool Proc_UP_EXG_MSG_REPORT_DRIVER_INFO( unsigned int seq, const char *xml ) ;
	// �����ϱ������˵�
	bool Proc_UP_EXG_MSG_REPORT_EWAYBILL_INFO( unsigned int seq, const char *xml ) ;
	// �����¼�����
	bool Proc_UP_CTRL_MSG_MONITOR_VEHICLE_ACK( unsigned int seq, const char *xml ) ;
	// �����ı��·�Ӧ��
	bool Proc_UP_CTRL_MSG_TEXT_INFO_ACK( unsigned int seq, const char *xml ) ;
	// �����г���¼��
	bool Proc_UP_CTRL_MSG_TAKE_TRAVEL_ACK( unsigned int seq, const char *xml ) ;
	// ��������������
	bool Proc_UP_CTRL_MSG_EMERGENCY_MONITORING_ACK( unsigned int seq, const char *xml ) ;
	// �������������
	bool Proc_DOWN_CTRL_MSG( unsigned int seq, const char *xml ) ;
	// �������գ�����������⴦��
	bool Proc_DOWN_CTRL_MSG_TAKE_PHOTO_REQ( unsigned int seq, const char *xml ) ;
	// ����ƽ̨����Ϣ
	bool Proc_DOWN_PLATFORM_MSG( unsigned int seq, const char *xml ) ;
	// ����ƽ̨��ı����Զ�Ӧ��
	bool Proc_DOWN_PLATFORM_MSG_INFO_REQ( unsigned int seq, const char *xml ) ;
	// ��������Ϣ
	bool Proc_DOWN_WARN_MSG( unsigned int seq, const char *xml ) ;
	// �����������Զ�Ӧ��
	bool Proc_DOWN_WARN_MSG_URGE_TODO_REQ( unsigned int seq, const char *xml ) ;
	// ȡ�ó�����̬��Ϣ
	bool Proc_DOWN_BASE_MSG_VEHICLE_ADDED( unsigned int seq, const char *xml ) ;
	// ����λ���ϱ�����Ϣ
	bool Proc_DOWN_EXG_MSG_CAR_LOCATION( unsigned int seq, const char *xml ) ;
	// ������λ��Ϣ����
	bool Proc_DOWN_EXG_MSG_HISTORY_ARCOSSAREA( unsigned int seq, const char *xml ) ;
	// ���������ľ�̬��Ϣ
	bool Proc_DOWN_EXG_MSG_CAR_INFO( unsigned int seq, const char *xml ) ;
	// ������������
	bool Proc_DOWN_EXG_MSG_RETURN_STARTUP( unsigned int seq, const char *xml ) ;
	// ������������
	bool Proc_DOWN_EXG_MSG_RETURN_END( unsigned int seq, const char *xml ) ;
	// ��������Ӧ��
	bool Proc_DOWN_EXG_MSG_ACK( unsigned int seq, const char *xml ) ;
	// �ϱ���ʻԱ���ʶ��
	bool Proc_DOWN_EXG_MSG_REPORT_DRIVER_INFO( unsigned int seq, const char *xml ) ;
	// �ϱ������˵�����
	bool Proc_DOWN_EXG_MSG_TAKE_WAYBILL_REQ( unsigned int seq, const char *xml ) ;

private:
	// �Զ�����ʧ�ܵĿ�����
	template<typename T>
	bool ProcDownExgReturnMsg( unsigned int seq, const char *id ) ;
	// ������չ��Ϣģ��
	template<typename T>
	bool ProcUpMsg( unsigned int seq, const char *xml , const char *msg ) ;
	// �����ֻ���XML��ֵ
	bool ParsePhoneXml( unsigned int seq, const char *xml , char *key, char *macid,  string &inner ) ;
	// �����������
	void ProcessError( unsigned int seq , bool remove ) ;

private:
	// ����������
	ISystemEnv			   *_pEnv ;
	// ����XML���õ�HTTP����
	CHttpCaller				_httpcaller ;
	// ����
	ServiceTable			_srv_table ;
	// ���������Ϣ��Ӧ��ϵ
	CSeq2Msg				_seq2msgid;
	// ����ڲ�Э������
	CSessionMgr				_innerdata ;
	// ���ݻ������
	CSessionMgr 			_datacache ;
	// ��Ŷ�ӦKEY��ϵ
	CSeq2Key				_seq2key ;
	// ���������ǵ�����
	CSeqMacRef				_macref ;
	// ���÷���URL��ַ
	CQString 				_callUrl ;
	// �Ƿ�������
	unsigned int			_istester ;
	// ������ʱ��
	int 			  		_livetime ;
	// ��Ҫ���������Ų�
	CSeq2Key				_seq2key2;
};


#endif /* SERVICECALLER_H_ */
