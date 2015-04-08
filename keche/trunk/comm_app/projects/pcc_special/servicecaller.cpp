/*
 * servicecaller.cpp
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */


#include "servicecaller.h"
#include <xmlparser.h>
#include <comlog.h>
#include <ProtoHeader.h>
#include <tools.h>
#include "pccutil.h"
#include <string.h>

#ifndef _WIN32
#include <strings.h>
#define stricmp strcasecmp
#endif

CServiceCaller::CServiceCaller():
	_datacache(true), _istester(0), _livetime(0)
{
	_srv_table[UP_EXG_MSG_REGISTER]							= &CServiceCaller::Proc_UP_EXG_MSG_REGISTER ;
	_srv_table[UP_EXG_MSG_REAL_LOCATION] 					= &CServiceCaller::Proc_UP_EXG_MSG_REAL_LOCATION ;
	_srv_table[UP_EXG_MSG_REPORT_DRIVER_INFO]				= &CServiceCaller::Proc_UP_EXG_MSG_REPORT_DRIVER_INFO ;
	_srv_table[UP_EXG_MSG_REPORT_EWAYBILL_INFO]		    	= &CServiceCaller::Proc_UP_EXG_MSG_REPORT_EWAYBILL_INFO ;
	_srv_table[UP_CTRL_MSG_MONITOR_VEHICLE_ACK]				= &CServiceCaller::Proc_UP_CTRL_MSG_MONITOR_VEHICLE_ACK ;
	_srv_table[UP_CTRL_MSG_TEXT_INFO_ACK]					= &CServiceCaller::Proc_UP_CTRL_MSG_TEXT_INFO_ACK ;
	_srv_table[UP_CTRL_MSG_TAKE_TRAVEL_ACK]					= &CServiceCaller::Proc_UP_CTRL_MSG_TAKE_TRAVEL_ACK ;
	_srv_table[UP_CTRL_MSG_EMERGENCY_MONITORING_ACK]    	= &CServiceCaller::Proc_UP_CTRL_MSG_EMERGENCY_MONITORING_ACK ;
	_srv_table[DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ]			= &CServiceCaller::Proc_DOWN_CTRL_MSG ;
	_srv_table[DOWN_CTRL_MSG_TAKE_PHOTO_REQ]				= &CServiceCaller::Proc_DOWN_CTRL_MSG_TAKE_PHOTO_REQ ;  // �������⴦��
	_srv_table[DOWN_CTRL_MSG_TEXT_INFO]						= &CServiceCaller::Proc_DOWN_CTRL_MSG ;
	_srv_table[DOWN_CTRL_MSG_TAKE_TRAVEL_REQ]				= &CServiceCaller::Proc_DOWN_CTRL_MSG ;
	_srv_table[DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ]  	= &CServiceCaller::Proc_DOWN_CTRL_MSG ;
	_srv_table[DOWN_PLATFORM_MSG_POST_QUERY_REQ]			= &CServiceCaller::Proc_DOWN_PLATFORM_MSG ;
	_srv_table[DOWN_PLATFORM_MSG_INFO_REQ]					= &CServiceCaller::Proc_DOWN_PLATFORM_MSG_INFO_REQ ;
	_srv_table[DOWN_WARN_MSG_URGE_TODO_REQ]					= &CServiceCaller::Proc_DOWN_WARN_MSG_URGE_TODO_REQ ;  // ���������Զ�Ӧ��
	_srv_table[DOWN_WARN_MSG_INFORM_TIPS]					= &CServiceCaller::Proc_DOWN_WARN_MSG ;
	_srv_table[DOWN_WARN_MSG_EXG_INFORM]					= &CServiceCaller::Proc_DOWN_WARN_MSG ;
	_srv_table[DOWN_EXG_MSG_CAR_LOCATION]					= &CServiceCaller::Proc_DOWN_EXG_MSG_CAR_LOCATION ;
	_srv_table[DOWN_EXG_MSG_HISTORY_ARCOSSAREA]				= &CServiceCaller::Proc_DOWN_EXG_MSG_HISTORY_ARCOSSAREA ;
	_srv_table[DOWN_EXG_MSG_CAR_INFO]						= &CServiceCaller::Proc_DOWN_EXG_MSG_CAR_INFO ;
	_srv_table[DOWN_EXG_MSG_RETURN_STARTUP]					= &CServiceCaller::Proc_DOWN_EXG_MSG_RETURN_STARTUP ;
	_srv_table[DOWN_EXG_MSG_RETURN_END]						= &CServiceCaller::Proc_DOWN_EXG_MSG_RETURN_END ;
	_srv_table[DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK]  = &CServiceCaller::Proc_DOWN_EXG_MSG_ACK ;
	_srv_table[DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK]  	= &CServiceCaller::Proc_DOWN_EXG_MSG_ACK ;
	_srv_table[DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK]			= &CServiceCaller::Proc_DOWN_EXG_MSG_ACK ;
	_srv_table[DOWN_EXG_MSG_REPORT_DRIVER_INFO]				= &CServiceCaller::Proc_DOWN_EXG_MSG_REPORT_DRIVER_INFO ;
	_srv_table[DOWN_EXG_MSG_TAKE_WAYBILL_REQ]				= &CServiceCaller::Proc_DOWN_EXG_MSG_TAKE_WAYBILL_REQ ;
	_srv_table[DOWN_BASE_MSG_VEHICLE_ADDED]					= &CServiceCaller::Proc_DOWN_BASE_MSG_VEHICLE_ADDED ;
}

CServiceCaller::~CServiceCaller()
{
	Stop() ;
}

// ��ʼ��
bool CServiceCaller::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char buf[1024] = {0} ;
	if ( ! pEnv->GetString( "http_call_url" , buf ) ) {
		OUT_ERROR( NULL, 0, "auth" , "get hessian url failed" ) ;
		return false ;
	}
	// ȡ��HTTP�������ʱ��
	int nvalue = 0 ;
	if ( pEnv->GetInteger( "http_error_time", nvalue ) ) {
		_macref.SetTimeOut( nvalue ) ;
	}
	// ȡ��HTTP�Ļ�����ʱ��
	if ( pEnv->GetInteger( "http_cache_time", nvalue ) ) {
		_livetime = nvalue ;
	}
	// �Ƿ�Ϊ����״̬
	if ( _pEnv->GetInteger( "pcc_is_tester", nvalue ) ) {
		_istester = (unsigned int)nvalue ;
	}

	int send_thread = 10 ;
	// �����߳�
	if ( pEnv->GetInteger( "http_send_thread" , nvalue ) ) {
		send_thread = nvalue ;
	}

	int recv_thread = 10 ;
	// �����߳�
	if ( pEnv->GetInteger( "http_recv_thread" , nvalue ) ) {
		recv_thread = nvalue ;
	}

	int queue_size = 1000 ;
	// HTTP�������Ķ��г���
	if ( pEnv->GetInteger( "http_queue_size" , nvalue ) ) {
		queue_size = nvalue ;
	}
	// ��¼����hession��URL
	_callUrl.SetString(buf);

	_httpcaller.SetReponse( this ) ;

	return _httpcaller.Init( send_thread, recv_thread, queue_size ) ;
}

// ����
bool CServiceCaller::Start( void )
{
	return _httpcaller.Start() ;
}

// ֹͣ
void CServiceCaller::Stop( void )
{
	return _httpcaller.Stop() ;
}

// ������
void CServiceCaller::RemoveCache( const char *key )
{
	if ( key == NULL ) {
		// ���ΪNULL����������������
		_datacache.RecycleAll() ;
		return ;
	}
	// ���ĳһ��ֵ�Ļ��洦��
	_datacache.RemoveSession( key ) ;

	OUT_PRINT( NULL, 0, key, "Remove Cache data" ) ;
}

// ����ǳ�ʱ�Ļ�������
void CServiceCaller::CheckTimeOut( void )
{
	if ( _livetime <= 0 )
		return ;

	// ������ݳ�ʱ
	_datacache.CheckTimeOut( _livetime ) ;

	OUT_PRINT( NULL, 0, "SrvCaller", "check data cache time out %d", _livetime ) ;
}

// ȡ��ע����Ϣ������ֻ�����Ȩʱ����
bool CServiceCaller::getRegVehicleInfo( unsigned int msgid, unsigned int seq, const char *phoneNum, const char *terminaltype )
{
	_seq2key2.AddSeqKey( seq, phoneNum ) ;

	CKeyValue item ;
	item.SetVal( "phoneNum"    , phoneNum ) ;
	item.SetVal( "terminaltype", terminaltype ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getRegVehicleInfo", item ) ;
}

// ͨ�ò�ѯ��ͨ�����ƺźͳ�����ɫȡ���ֻ�����ome��Ķ�Ӧ��ϵ
bool CServiceCaller::getTernimalByVehicleByType( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor )
{
	char key[256]= {0};
	sprintf( key, "%s_%s", vehicleno, vehicleColor ) ;

	_seq2key2.AddSeqKey( seq, key ) ;

	string val ;
	if ( _datacache.GetSession(key,val,false) ) {
		// ���ӳ�����
		_seq2msgid.AddSeqMsg( seq, msgid ) ;
		// ������ڻ�����ֱ�Ӵ���
		ProcessResp( seq, val.c_str(), val.length(), HTTP_CALL_SUCCESS ) ;

		return true ;
	}

	// ��һ�δ����¼���ô����⴦��һ�����ó���Ͷ��ڴ˳���һ��ʱ��ͣ��HTTP�ĵ���
	if ( ! _macref.Add( key, seq ) ) {
		OUT_ERROR( NULL, 0, "Caller", "macid %s call before error, seq %d", key, seq ) ;
		ProcessError( seq, false ) ;
		return false ;
	}

	CKeyValue item ;
	item.SetVal( "vehicleno"   , vehicleno ) ;
	item.SetVal( "vehicleColor", vehicleColor ) ;

	_seq2key.AddSeqKey( seq, key ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getTernimalByVehicleByType" , item ) ;
}

bool CServiceCaller::getTernimalByVehicleByTypeEx( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor , const char *text )
{
	if ( text == NULL )
		return false ;

	char key[256]= {0};
	_pEnv->GetCacheKey( seq, key ) ;

	// ���浱ǰ�����ڲ�Э��
	_innerdata.AddSession( key, text ) ;

	// ����������
	if ( ! getTernimalByVehicleByType( msgid, seq, vehicleno, vehicleColor ) ) {
		ProcessError( seq, false ) ;
		return false ;
	}

	return true ;
}

// ͨ�ò�ѯ��ͨ���ֻ��ź�oem��ȡ�ö�Ӧ�����Լ�ʡ���ϵ
bool CServiceCaller::getBaseVehicleInfo( unsigned int msgid, unsigned int seq, const char *phone  , const char *ome )
{
	char key[256]= {0};
	sprintf( key, "%s_%s", ome, phone ) ;

	_seq2key2.AddSeqKey( seq, key ) ;

	string val ;
	if ( _datacache.GetSession(key,val,false) ) {
		// ���ӳ�����
		_seq2msgid.AddSeqMsg( seq, msgid ) ;
		// ������ڻ�����ֱ�Ӵ���
		ProcessResp( seq, val.c_str(), val.length(), HTTP_CALL_SUCCESS ) ;

		return true ;
	}

	// ��һ�δ����¼���ô����⴦��һ�����ó���Ͷ��ڴ˳���һ��ʱ��ͣ��HTTP�ĵ���
	if ( ! _macref.Add( key, seq ) ) {
		OUT_ERROR( NULL, 0, "Caller", "macid %s call before error, seq %d", key, seq ) ;
		ProcessError( seq, false ) ;
		return false ;
	}

	CKeyValue item ;
	item.SetVal( "phoneNum"    , phone ) ;
	item.SetVal( "terminaltype", ome ) ;

	_seq2key.AddSeqKey( seq, key ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getBaseVehicleInfo" , item ) ;
}

// ��ȡ�ü�ʻԱʶ����Ϣ
bool CServiceCaller::getDriverOfVehicleByType( unsigned int msgid, unsigned int seq, const char *vehicleno , const char *vehicleColor )
{
	CKeyValue item ;
	item.SetVal( "vehicleno"   , vehicleno ) ;
	item.SetVal( "vehicleColor", vehicleColor ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getDriverOfVehicleByType", item ) ;
}

// ��ȡ�õ����˵�����
bool CServiceCaller::getEticketByVehicle( unsigned int msgid, unsigned int seq, const char *vehicleno , const char *vehicleColor )
{
	CKeyValue item ;
	item.SetVal( "vehicleno"   , vehicleno ) ;
	item.SetVal( "vehicleColor", vehicleColor ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getEticketByVehicle", item ) ;
}

// ����������
bool CServiceCaller::getDetailOfVehicleInfo( unsigned int msgid, unsigned int seq, const char *vehicleno, const char *vehicleColor )
{
	CKeyValue item ;
	item.SetVal( "vehicleno"   , vehicleno ) ;
	item.SetVal( "vehicleColor", vehicleColor ) ;

	return ProcessMsg( msgid, seq, "vehicleInforService", "getDetailOfVehicleInfo", item ) ;
}

// ����ƽ̨��ڵ���Ϣ
bool CServiceCaller::addForMsgPost( unsigned int msgid, unsigned int seq, const char *messageContent , const char * messageId ,
		const char *objectId, const char *objectType , const char *areaId )
{
	CKeyValue item ;

	item.SetVal( "messageContent" , messageContent ) ;
	item.SetVal( "messageId" 	  , messageId ) ;
	item.SetVal( "objectId"		  , objectId ) ;
	item.SetVal( "objectType"	  , objectType ) ;
	item.SetVal( "areaId"		  , areaId ) ;

	return ProcessMsg( msgid, seq, "thPlatInfosRmi", "addForMsgPost" , item ) ;
}

// �����·�ƽ̨�ı���
bool CServiceCaller::addForMsgInfo(unsigned int msgid, unsigned int seq, const char *messageContent , const char * messageId ,
			const char *objectId, const char *objectType , const char *areaId )
{
	CKeyValue item ;

	item.SetVal( "messageContent" , messageContent ) ;
	item.SetVal( "messageId" 	  , messageId ) ;
	item.SetVal( "objectId"		  , objectId ) ;
	item.SetVal( "objectType"	  , objectType ) ;
	item.SetVal( "areaId"		  , areaId ) ;

	return ProcessMsg( msgid, seq, "thPlatInfosRmi", "addForMsgInfo" , item ) ;
}

// ����������
bool CServiceCaller::addMsgUrgeTodo( unsigned int msgid, unsigned int seq, const char *supervisionEndUtc , const char *supervisionId ,
			const char * supervisionLevel , const char * supervisor , const char *supervisorEmail , const char *supervisorTel ,
			const char * vehicleColor, const char *vehicleNo , const char *wanSrc , const char *wanType , const char *warUtc )
{
	CKeyValue item ;

	item.SetVal( "supervisionEndUtc" , supervisionEndUtc ) ;
	item.SetVal( "supervisionId"	 , supervisionId ) ;
	item.SetVal( "supervisionLevel"  , supervisionLevel ) ;
	item.SetVal( "supervisor"		 , supervisor ) ;
	item.SetVal( "supervisorEmail"	 , supervisorEmail ) ;
	item.SetVal( "supervisorTel" 	 , supervisorTel ) ;
	item.SetVal( "vehicleColor"      , vehicleColor ) ;
	item.SetVal( "vehicleNo"         , vehicleNo ) ;
	item.SetVal( "wanSrc" 			 , wanSrc ) ;
	item.SetVal( "wanType" 			 , wanType ) ;
	item.SetVal( "warUtc" 			 , warUtc ) ;

	return ProcessMsg( msgid, seq, "thAlarmTodoRmi", "add" , item ) ;
}

// ������Ԥ��
bool CServiceCaller::addMsgInformTips( unsigned int msgid, unsigned int seq, const char *alarmDescr, const char *alarmFrom,
			const char *alarmTime, const char *alarmType , const char *vehicleColor , const char *vehicleNo )
{
	CKeyValue item ;

	item.SetVal( "alarmDescr"	, alarmDescr ) ;
	item.SetVal( "alarmFrom" 	, alarmFrom  ) ;
	item.SetVal( "alarmTime" 	, alarmTime  ) ;
	item.SetVal( "alarmType" 	, alarmType  ) ;
	item.SetVal( "vehicleColor" , vehicleColor ) ;
	item.SetVal( "vehicleNo"	, vehicleNo) ;

	return ProcessMsg( msgid, seq, "thVehicleEarlywarningRmi", "add" , item ) ;
}

// �������ӵ�״̬
bool CServiceCaller::updateConnectState( unsigned int msgid, unsigned int seq, int areaId , int linkType , int status )
{
	CKeyValue item ;

	char szareaId[128] = {0} ;
	sprintf( szareaId, "%d", areaId ) ;

	char szutc[128] = {0};
	sprintf( szutc, "%llu", (long long)time(NULL) ) ;

	char szType[12] = {0};
	sprintf( szType, "%d", linkType ) ;

	char szStatus[12] = {0};
	sprintf( szStatus, "%d", status ) ;

	// areaId="" utc="" linkType="" status="0:���ӣ�-1�Ͽ�"/
	item.SetVal( "areaId"	, szareaId  ) ;
	item.SetVal( "utc" 		, szutc     ) ;
	item.SetVal( "linkType" , szType    ) ;
	item.SetVal( "status" 	, szStatus  ) ;

	return ProcessMsg( msgid, seq, "thLinkStatusServiceRmi", "add" , item ) ;
}

// ToDo: ��Ӹ�������Ĵ��������������Ϊһ�������Ӧһ������ , USERID_UTC_%d
bool CServiceCaller::ProcessMsg( unsigned int msgid, unsigned int seq , const char *service, const char *method , CKeyValue &item )
{
	if ( msgid == 0 )return false ;

	char key[256] = {0};
	_pEnv->GetCacheKey( seq, key ) ;
	// ���ӳ�����
	_seq2msgid.AddSeqMsg( seq, msgid ) ;

	CQString sXml ;
	// ����XML����
	if ( ! CreateRequestXml( sXml, key, service, method, item ) ) {
		OUT_ERROR( NULL, 0, "ProcMsg" , "create request xml failed, msg id %x, seq id %d" , msgid, seq ) ;
		ProcessError( seq, true ) ;
		return false ;
	}
	OUT_INFO( NULL, 0, "Caller", "Request msg id %x,seqid %d,service %s,method %s" , msgid, seq, service, method ) ;
	// ����XML����
	if ( ! _httpcaller.Request( seq, (const char *)_callUrl, sXml.GetBuffer(), sXml.GetLength() ) ) {
		OUT_ERROR( NULL, 0, "ProcMsg" , "caller http request failed , msg id %x, seq id %d" , msgid, seq ) ;
		ProcessError( seq, true ) ;
		return false ;
	}

	return true ;
}

// ���ر���ת��UTF-8
static bool locale2utf8( const char *szdata, const int nlen , CQString &out )
{
	int   len = nlen*4 + 1 ;
	char *buf = new char[ len ] ;
	memset( buf, 0 , len ) ;

	if( g2u( (char *)szdata , nlen , buf, len ) == -1 ){
		OUT_ERROR( NULL, 0, "auth" , "locale2utf8 query %s failed" , szdata ) ;
		delete [] buf ;
		return false ;
	}
	buf[len] = 0 ;
	out.SetString( buf ) ;
	delete [] buf ;

	return true ;
}

// ����XML���ݵĴ���
bool CServiceCaller::CreateRequestXml( CQString &sXml, const char *id, const char *service, const char *method , CKeyValue &item )
{
	CXmlBuilder XmlOb("Request","Param","Item");

	XmlOb.SetRootAttribute( "id", id ) ;
	XmlOb.SetRootAttribute( "service", service );
	XmlOb.SetRootAttribute( "method", method );

	CQString stemp ;
	// ȡ���Ƿ����������
	int count = item.GetSize() ;
	if ( count > 0 ) {
		for ( int i = 0; i < count; ++ i ) {
			_KeyValue &vk = item.GetVal( i ) ;
			// �������Ϊ�յĻ��Ͳ���Ӵ�����
			if ( vk.key.empty() || vk.val.empty() ) {
				continue ;
			}
			// ת��UTF-8�ı��봦��
			if ( ! locale2utf8( vk.val.c_str() , vk.val.length() , stemp ) ){
				// �������Ϊ���򲻴�
				continue ;
			}
			// ��������
			XmlOb.SetItemAttribute( vk.key.c_str(), stemp.GetBuffer() ) ;
		}
		XmlOb.InsertItem();
	}

	// ����XML����
	XmlOb.GetXmlText(sXml);
	// �����XML������
	OUT_PRINT( NULL, 0, "XML" , "Request GBK XML:%s" , sXml.GetBuffer() ) ;

	return ( !sXml.IsEmpty() ) ;
}

// �����������
void CServiceCaller::ProcessError( unsigned int seqid , bool remove )
{
	// ����ص�TABLE��SEQ��ӦMAP
	if ( remove )
		_seq2msgid.RemoveSeq( seqid ) ;

	string skey ;
	// ������Ҫ��������ݱ�־�������ֵ����������ʧ���һ����������
	if ( _seq2key.GetSeqKey( seqid , skey ) ) {
		// ��Ӵ�������ݻ���
		_datacache.AddSession( skey, "error response" ) ;
	}
	_seq2key2.GetSeqKey( seqid, skey ) ;

	char key[256]= {0};
	_pEnv->GetCacheKey( seqid , key ) ;
	// �����ڲ�Э�黺������
	_innerdata.RemoveSession( key ) ;
	// �������ݻ���
	_pEnv->GetMsgCache()->Remove( key ) ;

	// ��Ҫ������õ�ǰ����ʱ��
	_macref.Dec( seqid, true ) ;
}

// ����HttpCaller�ص�
void CServiceCaller::ProcessResp( unsigned int seqid, const char *xml, const int len , const int err )
{
	// ������Ӧ��XML�������
	if ( xml == NULL || err != HTTP_CALL_SUCCESS || len == 0 ) {
		OUT_ERROR( NULL, 0, "Caller" , "Process seqid %u, error %d" , seqid, err ) ;
		ProcessError( seqid , true ) ;
		return ;
	}

	// ����XML����Ӧ���
	const char *ptr = strstr( xml , "Response" ) ;
	// ������ݸ�ʽ����ȷ
	if ( ptr == NULL || strstr( xml , "Result") == NULL || strstr( xml, "Item" ) == NULL ) {
		OUT_ERROR( NULL, 0, "Caller" , "ProcessResponse seqid %d , data error, xml %s", seqid , xml ) ;
		ProcessError( seqid , true ) ;
		return ;
	}

	// ȡ�ö�Ӧ���ID
	unsigned int msgid = 0;
	if ( ! _seq2msgid.GetSeqMsg( seqid , msgid ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "ProcessResponse seqid %d , msg id empty", seqid ) ;
		ProcessError( seqid , false ) ;
		return;
	}

	ServiceTable::iterator it = _srv_table.find( msgid ) ;
	// ������Ϣ��ӦID�ص�����
	if ( it == _srv_table.end() ) {
		OUT_ERROR( NULL, 0, "Caller" , "ProcessResponse seqid %d , call method empty", seqid ) ;
		ProcessError( seqid , false ) ;
		return ;
	}

	string key ;
	_seq2key2.GetSeqKey( seqid, key ) ;

	// ����XML������,��������ֻ����복�ƺ�֮��Ķ�Ӧ��ϵ��־����ʾ
	OUT_PRINT( NULL, 0, key.c_str(), "xml:%s" , (const char*)(ptr-1) ) ;
	// ���ô�����
	if ( ! (this->*(it->second)) ( seqid , xml ) ) {
		// ������ݲ���ȷ
		ProcessError( seqid , false ) ;
		return ;
	}
	// �������Ƶ���¼
	_macref.Dec( seqid, false ) ;

	// ������Ҫ���������,������ȷ�����Ҫ��CACHE���Ƴ�
	// ���ȡ��KEYֵ��Ϊ������Ҫ��������
	if ( _seq2key.GetSeqKey( seqid , key ) ) {
		// ��ӻ�����
		_datacache.AddSession( key, xml ) ;
	}
}

// ����ע�������
bool CServiceCaller::Proc_UP_EXG_MSG_REGISTER( unsigned int seq, const char *xml )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "UP_EXG_MSG_REGISTER Parser xml Error, xml: %s" , xml ) ;
		return false;
	}

	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_REGISTER failed, seq id %d" ,seq ) ;
		return false;
	}

	if ( len <= 0 || len < (int)sizeof(UpExgMsgRegister) ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_REGISTER failed, seq id %d" ,seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false;
	}

	/** <Item vehicleno="�.00001" phoneNum="15000000001" terminalid="0000001" vehicleColor="2" cityid="6401" terminaltype="15000000001"/> */
	UpExgMsgRegister *req = ( UpExgMsgRegister *) msg ;

	unsigned char car_color = parser.GetInteger( "Response::Result::Item:vehicleColor" , 0 ) ;
	const char *  car_num   = parser.GetString( "Response::Result::Item:vehicleno", 0 ) ;
	const char *  city_id   = parser.GetString( "Response::Result::Item:cityid" , 0 ) ;

	if ( car_num == NULL || city_id == NULL ) {
		OUT_ERROR( NULL, 0 , NULL, "UP_EXG_MSG_REGISTER get car num failed, seq id %d" , seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false;
	}

	req->exg_msg_header.vehicle_color = car_color;
	safe_memncpy( req->exg_msg_header.vehicle_no, car_num , sizeof(req->exg_msg_header.vehicle_no) ) ;

	const char *ptr = NULL ;

	// �����������ע������
	ptr = parser.GetString( "Response::Result::Item:manufacturerid" , 0 ) ;
	if ( ptr != NULL ) {
		safe_memncpy( req->producer_id, ptr, sizeof(req->producer_id) ) ;
	}

	ptr = parser.GetString( "Response::Result::Item:terminalid" , 0 ) ;
	if ( ptr != NULL ) {
		safe_memncpy( req->terminal_id, ptr, sizeof(req->terminal_id) ) ;
	}

	ptr = parser.GetString( "Response::Result::Item:terminaltype" , 0 ) ;
	if ( ptr != NULL ) {
		safe_memncpy( req->terminal_model_type, ptr, sizeof(req->terminal_model_type) ) ;
	}

	ptr = parser.GetString( "Response::Result::Item:phoneNum" , 0 ) ;
	if ( ptr != NULL ) {
		// ����Ӻ���ǰ������ǰ�治�㲹��
		reverse_copy( req->terminal_simcode, sizeof(req->terminal_simcode), ptr, 0 ) ;
	}

	// ���ݶ�Ӧʡ���͸���Ӧʡ����
	_pEnv->GetPasClient()->HandleClientData( city_id , msg, len ) ;//����PAS

	OUT_SEND( NULL, 0, city_id, "UP_EXG_MSG_REGISTER:%s, car color:%d", car_num , car_color );

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	return true ;
}

bool CServiceCaller::Proc_UP_EXG_MSG_REAL_LOCATION( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpExgMsgRealLocation>( seq, xml , "UP_EXG_MSG_REAL_LOCATION") ;
}

// �����ϱ���ʻԱ���ʶ��
bool CServiceCaller::Proc_UP_EXG_MSG_REPORT_DRIVER_INFO( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpExgMsgReportDriverInfo>( seq, xml , "UP_EXG_MSG_REPORT_DRIVER_INFO") ;
}

// �����ϱ������˵�
bool CServiceCaller::Proc_UP_EXG_MSG_REPORT_EWAYBILL_INFO( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpExgMsgReportEwaybillInfo>( seq, xml , "UP_EXG_MSG_REPORT_EWAYBILL_INFO") ;
}

// ������չ��Ϣģ��
template<typename T>
bool CServiceCaller::ProcUpMsg( unsigned int seq, const char *xml , const char *name )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "%s Parser xml Error, xml: %s" , name, xml ) ;
		return false ;
	}

	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "%s failed, seq id %d" , name, seq ) ;
		return false ;
	}

	if ( len <= 0 || len < (int)sizeof(T) ) {
		OUT_ERROR( NULL, 0, NULL, "%s failed, seq id %d" , name, seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	/** <Reponse><Result><Item vehicleno="" vehicleColor=""/></Result></Reponse> */
	// T *req = ( T *) msg ;

	unsigned char car_color = parser.GetInteger( "Response::Result::Item:vehicleColor" , 0 ) ;
	const char *  car_num   = parser.GetString( "Response::Result::Item:vehicleno", 0 ) ;
	const char *  city_id   = parser.GetString( "Response::Result::Item:cityid" , 0 ) ;

	// ycq 2013-11-14 waifujinjing
	city_id = "999999";

	if ( car_num == NULL || city_id == NULL ) {
		OUT_ERROR( NULL, 0 , NULL, "%s get car num failed, seq id %d, xml %s" , name, seq , xml ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	BaseMsgHeader *msgheader = (BaseMsgHeader*) (msg + sizeof(Header));
	msgheader->vehicle_color = car_color;
	safe_memncpy( msgheader->vehicle_no, car_num , sizeof(msgheader->vehicle_no) ) ;

	// ���ݶ�Ӧʡ���͸���Ӧʡ����
	_pEnv->GetPasClient()->HandleClientData( city_id , msg, len ) ;//����PAS

	OUT_SEND( NULL, 0, city_id , "%s:%s, color:%d", name, car_num , car_color );

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	return true ;
}

// �����¼�����
bool CServiceCaller::Proc_UP_CTRL_MSG_MONITOR_VEHICLE_ACK( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpCtrlMsgMonitorVehicleAck>( seq, xml , "UP_CTRL_MSG_MONITOR_VEHICLE_ACK" ) ;
}

// �����ı��·�Ӧ��
bool CServiceCaller::Proc_UP_CTRL_MSG_TEXT_INFO_ACK( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpCtrlMsgTextInfoAck>( seq, xml , "UP_CTRL_MSG_TEXT_INFO_ACK" ) ;
}

// ������ʻ��¼������
bool CServiceCaller::Proc_UP_CTRL_MSG_TAKE_TRAVEL_ACK( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpCtrlMsgTaketravel>( seq, xml , "UP_CTRL_MSG_TAKE_TRAVEL_ACK" ) ;
}

// ��������������
bool CServiceCaller::Proc_UP_CTRL_MSG_EMERGENCY_MONITORING_ACK( unsigned int seq, const char *xml )
{
	return ProcUpMsg<UpCtrlMsgEmergencyMonitoringAck>( seq, xml, "UP_CTRL_MSG_EMERGENCY_MONITORING_ACK" ) ;
}

// �����ڲ�Э������
static const string buildsenddata( const string &cmd, const string &szseq, const string &macid, const string &inner )
{
	string sdata = cmd ;
	sdata += " " ;
	sdata += szseq ;
	sdata += " " ;
	sdata += macid ;
	sdata += inner ;
	return sdata ;
}

// �����·�������������
bool CServiceCaller::Proc_DOWN_CTRL_MSG( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_CTRL_MSG error" ) ;
		return false ;
	}

	string senddata = buildsenddata("CAITS" , key, macid, inner ) ;

	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , senddata.c_str(), senddata.length() ) ;

	return true ;
}

// ����������⴦��
bool CServiceCaller::Proc_DOWN_CTRL_MSG_TAKE_PHOTO_REQ( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_CTRL_MSG_TAKE_PHOTO_REQ error" ) ;
		return false ;
	}

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "DOWN_CTRL_MSG_TAKE_PHOTO_REQ failed, seq id %d" ,seq ) ;
		return false ;
	}

	char szKey[512] = {0};
	// �����ĵļ�ֵ��ת��ome_phone_msgid��KEY��ŵ�CACHE�������첽��HTTP��ȡͼƬ����
	sprintf( szKey, "%s_%d" , macid, UP_CTRL_MSG_TAKE_PHOTO_ACK ) ;
	// ���´������
	_pEnv->GetMsgCache()->AddData( szKey, msg , len ) ;
	// �ͷ�ԭ������
	_pEnv->GetMsgCache()->FreeData( msg ) ;

	string senddata = buildsenddata("CAITS" , key, macid, inner ) ;

	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , senddata.c_str(), senddata.length() ) ;

	return true ;
}

// ����ƽ̨��
bool CServiceCaller::Proc_DOWN_PLATFORM_MSG( unsigned int seq, const char *xml )
{
	// ���ﲻ�账���κ�
	OUT_SEND( NULL, 0, NULL, "DOWN_PLATFORM_MSG , seq id %d" , seq );
	return true ;
}

// ����ƽ̨�䱨���Զ�Ӧ����
bool CServiceCaller::Proc_DOWN_PLATFORM_MSG_INFO_REQ( unsigned int seq, const char *xml )
{
	// ���������Զ�Ӧ����
	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int len = 0 ;
	char *buf = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( buf == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "Proc_DOWN_PLATFORM_MSG_INFO_REQ get message seq %d cache error" , seq ) ;
		return false ;
	}

	// ���ȡ�û�������ݳ���С��ƽ̨��Ӧ��
	if ( len < (int)sizeof(UpPlatFormMsgInfoAck) ) {
		OUT_ERROR( NULL, 0, NULL, "Proc_DOWN_PLATFORM_MSG_INFO_REQ length %d, less than need length %d" , len , sizeof(UpPlatFormMsgInfoAck) ) ;
		_pEnv->GetMsgCache()->FreeData( buf ) ;
		return false ;
	}

	UpPlatFormMsgInfoAck *ack = (UpPlatFormMsgInfoAck *) buf ;
	int accesscode = ntouv32( ack->header.access_code ) ;

	// �Զ���Ӧ����
	_pEnv->GetPasClient()->HandlePasUpData( accesscode, buf, len ) ;

	OUT_PRINT( NULL, 0, NULL, "UP_PLATFORM_MSG_INFO_ACK, accesscode: %d" , accesscode ) ;

	_pEnv->GetMsgCache()->FreeData( buf ) ;

	return true ;
}

// ��������
bool CServiceCaller::Proc_DOWN_WARN_MSG( unsigned int seq, const char *xml )
{
	OUT_SEND( NULL, 0, NULL, "DOWN_WARN_MSG , seq id %d", seq ) ;
	return true ;
}

// �����������Զ�Ӧ��
bool CServiceCaller::Proc_DOWN_WARN_MSG_URGE_TODO_REQ( unsigned int seq, const char *xml )
{
	// ���������Զ�Ӧ����
	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int len = 0 ;
	char *buf = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( buf == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "Proc_DOWN_WARN_MSG_URGE_TODO_REQ get message seq %d cache error" , seq ) ;
		return false ;
	}
	// У�鳤���Ƿ���ȷ
	if ( len < (int)sizeof(UpWarnMsgUrgeTodoAck) ) {
		OUT_ERROR( NULL, 0, NULL, "Proc_DOWN_WARN_MSG_URGE_TODO_REQ  data length %d, less than need length: %d" , len, sizeof(UpWarnMsgUrgeTodoAck) ) ;
		_pEnv->GetMsgCache()->FreeData( buf ) ;
		return false ;
	}

	UpWarnMsgUrgeTodoAck *resp = ( UpWarnMsgUrgeTodoAck *) buf ;
	int access_code = ntouv32( resp->header.access_code ) ;
	// ���ݶ�Ӧʡ���͸���Ӧʡ����
	_pEnv->GetPasClient()->HandlePasUpData( access_code , buf, len ) ;//����PAS
	// �Զ�Ӧ����������
	OUT_SEND( NULL, 0, NULL, "UP_WARN_MSG_URGE_TODO_ACK:%s, color:%d, accesscode:%d",
			resp->warn_msg_header.vehicle_no , resp->warn_msg_header.vehicle_color, access_code );

	_pEnv->GetMsgCache()->FreeData( buf ) ;

	return true;
}

// ����λ���ϱ�����Ϣ
bool CServiceCaller::Proc_DOWN_EXG_MSG_CAR_LOCATION( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_CAR_LOCATION error" ) ;
		return false ;
	}

	string senddata = buildsenddata("CAITS" , "0_0", macid, inner ) ;

	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , senddata.c_str(), senddata.length() ) ;

	return true ;
}

// ������λ��Ϣ����
bool CServiceCaller::Proc_DOWN_EXG_MSG_HISTORY_ARCOSSAREA( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_HISTORY_ARCOSSAREA error" ) ;
		return false;
	}

	vector<string> vec ;
	if ( ! splitvector( inner, vec, "|", 0 ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_HISTORY_ARCOSSAREA get inner gps failed" ) ;
		return false;
	}

	char head[512] = {0} ;
	sprintf( head, "CAITS 0_0 %s 4 U_REPT {TYPE:0," , macid ) ;

	// �������·�������
	for( int i = 0; i < (int) vec.size(); ++ i ) {
		string sdata = head ;
		sdata += vec[i] ;
		sdata += "} \r\n" ;

		// ����MACID��·�ɽ������ݷ���
		((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , sdata.c_str(), sdata.length() ) ;
	}
	return true ;
}

// ���������ľ�̬��Ϣ
bool CServiceCaller::Proc_DOWN_EXG_MSG_CAR_INFO( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_CAR_INFO error" ) ;
		return false ;
	}

	string sdata = buildsenddata( "CAITS" , key , macid, inner ) ;
	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , sdata.c_str(), sdata.length() ) ;

	return true ;
}

// ������÷���ʧ�ܣ�ֱ�ӻ�Ӧ�ɹ�
template<typename T>
bool CServiceCaller::ProcDownExgReturnMsg( unsigned int seq, const char *id )
{
	int   len = 0 ;
	char key[128] = {0};
	_pEnv->GetCacheKey( seq, key ) ;

	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "Send %s", id ) ;
		return false ;
	}

	T *ack = ( T * ) msg ;

	int accesscode = ntouv32( ack->header.access_code ) ;
	// �Զ�Ӧ����
	_pEnv->GetPasClient()->HandlePasUpData( accesscode, msg , len ) ;

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	OUT_SEND( NULL, 0, "Caller", "Send %s:%s, color:%d, accesscode:%d",
			id , ack->exg_msg_header.vehicle_no, ack->exg_msg_header.vehicle_color , accesscode ) ;

	return true ;
}

// ������������
bool CServiceCaller::Proc_DOWN_EXG_MSG_RETURN_STARTUP( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_RETURN_STARTUP error" ) ;
		return ProcDownExgReturnMsg<UpExgMsgReturnStartupAck>( seq, "UP_EXG_MSG_RETURN_STARTUP_ACK" ) ;
	}

	string sdata = buildsenddata( "CAITS" , key , macid, inner ) ;
	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , sdata.c_str(), sdata.length() ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	assert( msg != NULL ) ;
	UpExgMsgReturnStartupAck *ack = ( UpExgMsgReturnStartupAck * ) msg ;

	int accesscode = ntouv32( ack->header.access_code ) ;
	// �Զ�Ӧ����
	_pEnv->GetPasClient()->HandlePasUpData( accesscode, msg , len ) ;

	size_t pos = inner.find( "RETURNSTARTUP:" ) ;
	assert( pos != string::npos ) ;

	sdata = inner.substr( 0, pos ) ;
	sdata += "RETURNSTARTUP:} \r\n" ;

	string sends = buildsenddata( "CAITR" , key , macid, sdata ) ;

	// �����ڲ�Э��
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid, sends.c_str(), sends.length() ) ;

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	OUT_SEND( NULL, 0, macid, "Send UP_EXG_MSG_RETURN_STARTUP_ACK:%s, color:%d",
			ack->exg_msg_header.vehicle_no, ack->exg_msg_header.vehicle_color ) ;

	return true ;
}

// ������������
bool CServiceCaller::Proc_DOWN_EXG_MSG_RETURN_END( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_RETURN_END error" ) ;
		return ProcDownExgReturnMsg<UpExgMsgReturnEndAck>( seq, "UP_EXG_MSG_RETURN_END_ACK" ) ;
	}

	string sdata = buildsenddata( "CAITR" , key , macid, inner ) ;
	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , sdata.c_str(), sdata.length() ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	assert( msg != NULL ) ;
	UpExgMsgReturnEndAck *ack = ( UpExgMsgReturnEndAck * ) msg ;

	int accesscode = ntouv32( ack->header.access_code ) ;
	// �Զ�Ӧ����
	_pEnv->GetPasClient()->HandlePasUpData( accesscode, msg , len ) ;

	size_t pos = inner.find( "RETURNEND:" ) ;
	assert( pos != string::npos ) ;

	sdata = inner.substr( 0, pos ) ;
	sdata += "RETURNEND:} \r\n" ;

	string sends = buildsenddata( "CAITR" , key , macid, sdata ) ;

	// �����ڲ�Э��
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid, sends.c_str(), sends.length() ) ;

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	OUT_SEND( NULL, 0, macid, "Send UP_EXG_MSG_RETURN_END_ACK:%s, color:%d",
			ack->exg_msg_header.vehicle_no, ack->exg_msg_header.vehicle_color ) ;

	return true ;
}

// ����������Ӧ��
bool CServiceCaller::Proc_DOWN_EXG_MSG_ACK( unsigned int seq, const char *xml )
{
	// ����MACID
	char key[256]   = {0} ;
	char macid[128] = {0} ;

	string inner ;
	if ( ! ParsePhoneXml( seq, xml, key, macid, inner ) ) {
		OUT_ERROR( NULL, 0, "Caller" , "DOWN_EXG_MSG_ACK error" ) ;
		return false ;
	}

	string sdata = inner ;
	// �����������һ����ֱ�ӷ��͵������һ����Ҫ��������ڲ�Э������
	if ( strncmp( inner.c_str(), "CAIT", 4 ) != 0 ) {
		sdata = buildsenddata( "CAITR" , key , macid, inner ) ;
	} else {
		size_t pos = sdata.find( "MACID" ) ;
		if ( pos != string::npos ) {
			// ���´���MACID
			sdata.replace( pos, 5 , macid ) ;
		}
	}
	// ����MACID��·�ɽ������ݷ���
	((IMsgClient*)_pEnv->GetMsgClient())->HandleUpMsgData( macid , sdata.c_str(), sdata.length() ) ;

	return true ;
}

// �����ֻ���XML��ֵ
bool CServiceCaller::ParsePhoneXml( unsigned int seq, const char *xml , char *key, char *macid,  string &inner )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "Parser xml Error, xml: %s" , xml ) ;
		return false;
	}

	_pEnv->GetCacheKey( seq, key ) ;

	// ��Ҫ���ڲ�Э�鷢��MSG����
	if ( ! _innerdata.GetSession( key, inner ) ) {
		OUT_ERROR( NULL, 0, "GetSession", "Get Inner failed" ) ;
		return false ;
	}
	_innerdata.RemoveSession( key ) ;

	const char *  ome_code    = parser.GetString( "Response::Result::Item:terminaltype" , 0 ) ;
	const char *  phone_num   = parser.GetString( "Response::Result::Item:phoneNum", 0 ) ;

	if ( ome_code == NULL || phone_num == NULL ) {
		OUT_ERROR( NULL, 0, "Xml" , "parser xml result get phoneNum and terminal type failed" ) ;
		return false;
	}

	// ����MACID
	sprintf( macid, "%s_%s" , ome_code, phone_num ) ;

	return true ;
}

// �ϱ���ʻԱ���ʶ��
bool CServiceCaller::Proc_DOWN_EXG_MSG_REPORT_DRIVER_INFO( unsigned int seq, const char *xml )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "UP_EXG_MSG_REPORT_DRIVER_INFO Parser xml Error, xml: %s" , xml ) ;
		return false ;
	}

	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_REPORT_DRIVER_INFO failed, seq id %d" ,seq ) ;
		return false ;
	}

	if ( len <= 0 || len < (int)sizeof(UpExgMsgReportDriverInfo) ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_REPORT_DRIVER_INFO failed, seq id %d" ,seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	/** <Item vehicleno="" vehicleColor="" cityid="" driverName="" driverNo="" driverCertificate="" certificateAgency=""/> */
	const char *cityid  = parser.GetString( "Response::Result::Item:cityid" , 0 ) ;
	const char *car_num = parser.GetString( "Response::Result::Item:vehicleno" , 0 ) ;

	// ���ȡ�õ�ǰ����Ϊ����ֱ�ӷ���
	if ( cityid == NULL || car_num == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_REPORT_DRIVER_INFO get cityid and car_num null, seq %d, xml %s" , seq , xml ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	UpExgMsgReportDriverInfo *rsp = ( UpExgMsgReportDriverInfo *) msg ;

	safe_memncpy( rsp->exg_msg_header.vehicle_no, car_num, sizeof(rsp->exg_msg_header.vehicle_no) ) ;
	rsp->exg_msg_header.vehicle_color = parser.GetInteger( "Response::Result::Item:vehicleColor" , 0 ) ;
	safe_memncpy( rsp->driver_name, parser.GetString( "Response::Result::Item:driverName" , 0 ) , sizeof(rsp->driver_name) ) ;
	safe_memncpy( rsp->driver_id  , parser.GetString( "Response::Result::Item:driverNo", 0 )   , sizeof(rsp->driver_id) ) ;
	safe_memncpy( rsp->org_name   , parser.GetString( "Response::Result::Item:driverCertificate", 0 )  , sizeof(rsp->org_name) ) ;
	safe_memncpy( rsp->licence  , parser.GetString( "Response::Result::Item:certificateAgency", 0 )  , sizeof(rsp->licence) ) ;

	_pEnv->GetPasClient()->HandleClientData( cityid, msg, len ) ;

	OUT_SEND( NULL, 0, cityid, "UP_EXG_MSG_REPORT_DRIVER_INFO:%s, color:%d", car_num , rsp->exg_msg_header.vehicle_color );

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	return true ;
}

// �ϱ������˵�����
bool CServiceCaller::Proc_DOWN_EXG_MSG_TAKE_WAYBILL_REQ( unsigned int seq, const char *xml )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "UP_EXG_MSG_TAKE_WAYBILL_ACK Parser xml Error, xml: %s" , xml ) ;
		return false ;
	}

	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_TAKE_WAYBILL_ACK failed, seq id %d" ,seq ) ;
		return false ;
	}

	if ( len <= 0 || len < (int)sizeof(UpExgMsgReportEwaybillInfo) ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_TAKE_WAYBILL_ACK failed, seq id %d" ,seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	/** <Item vehicleno="" vehicleColor="" cityid="" eticketContent=""/> */
	const char *cityid  = parser.GetString( "Response::Result::Item:cityid" , 0 ) ;
	const char *car_num = parser.GetString( "Response::Result::Item:vehicleno" , 0 ) ;

	// ���ȡ�õ�ǰ����Ϊ����ֱ�ӷ���
	if ( cityid == NULL || car_num == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG_TAKE_WAYBILL_ACK get cityid and car_num null, seq %d, xml %s" , seq , xml ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	UpExgMsgReportEwaybillInfo *rsp = ( UpExgMsgReportEwaybillInfo *) msg ;

	safe_memncpy( rsp->exg_msg_header.vehicle_no, car_num , sizeof(rsp->exg_msg_header.vehicle_no) ) ;
	rsp->exg_msg_header.vehicle_color = parser.GetInteger( "Response::Result::Item:vehicleColor" , 0 ) ;

	CQString content = parser.GetString( "Response::Result::Item:eticketContent" , 0  ) ;
	rsp->exg_msg_header.data_length = ntouv32( sizeof(int) + content.GetLength() ) ;
	rsp->ewaybill_length 		    = ntouv32( content.GetLength() ) ;
	rsp->header.msg_len	 			= ntouv32( content.GetLength() + len + sizeof(Footer) ) ;

	DataBuffer buf ;
	buf.writeBlock( msg, len ) ;
	buf.writeBlock( content.GetBuffer(), content.GetLength() ) ;

	Footer footer ;
	buf.writeBlock( &footer, sizeof(footer) ) ;

	_pEnv->GetPasClient()->HandleClientData( cityid, buf.getBuffer(), buf.getLength() ) ;

	OUT_SEND( NULL, 0, cityid, "UP_EXG_MSG_TAKE_WAYBILL_ACK:%s, color:%d", car_num , rsp->exg_msg_header.vehicle_color);

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	return true ;
}

// ȡ�ó�����̬��Ϣ���Զ�������
bool CServiceCaller::Proc_DOWN_BASE_MSG_VEHICLE_ADDED( unsigned int seq, const char *xml )
{
	// XML�����������XML�ı�
	CXmlParser parser ;
	if ( ! parser.LoadXml( xml ) ){
		OUT_ERROR( NULL, 0, "XmlParser" , "DOWN_BASE_MSG_VEHICLE_ADDED Parser xml Error, xml: %s" , xml ) ;
		return false ;
	}

	char key[256] = {0} ;
	_pEnv->GetCacheKey( seq, key ) ;

	int   len = 0 ;
	char *msg = _pEnv->GetMsgCache()->GetData( key, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_BASE_MSG_VEHICLE_ADDED_ACK failed, seq id %d" ,seq ) ;
		return false ;
	}

	if ( len <= 0 || len < (int)sizeof(UpbaseMsgVehicleAddedAck) ) {
		OUT_ERROR( NULL, 0, NULL, "UP_BASE_MSG_VEHICLE_ADDED_ACK failed, seq id %d" ,seq ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	/** <Item vehicleno="" plateColorId="" vehicletypeId="" typeName="" transTypeDesc="" city="" corpId="" corpName="" linkPhone="" spId="" spName=""/> */
	const char *car_num = parser.GetString( "Response::Result::Item:vehicleno" , 0 ) ;
	if ( car_num == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "UP_BASE_MSG_VEHICLE_ADDED_ACK get cityid and car_num null, seq %d, xml %s" , seq , xml ) ;
		_pEnv->GetMsgCache()->FreeData( msg ) ;
		return false ;
	}

	// <Item vehicleno="��A8888" plateColorId="1" vehicletypeId="111" typeName="11" transTypeDesc="12" city="1000" corpId="12" corpName="test" linkPhone="15000000001" spId="11" spName="test"/>
	UpbaseMsgVehicleAddedAck *rsp = ( UpbaseMsgVehicleAddedAck *) msg ;
	// ������̬��������  "VIN","VEHICLE_COLOR","VEHICLE_TYPE","TRANS_TYPE","VEHICLE_NATIONALITY","OWERS_NAME"
	CQString sBaseData ;
	sBaseData.AppendBuffer( "VIN:=" ) ;
	sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:vehicleno" , 0 ) ) ;
	sBaseData.AppendBuffer( ";" ) ;
	sBaseData.AppendBuffer( "VEHICLE_COLOR:=" ) ;
	sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:plateColorId", 0 ) ) ;
	sBaseData.AppendBuffer( ";" ) ;

	if ( _istester & 0x04 ) {
		// VIN:=��A77889;VEHICLE_COLOR:=2;VEHICLE_TYPE:=�γ�;TRANS_TYPE:=010;VEHICLE_NATIONALITY:=640000;OWERS_ID:=;OWERS_NAME:=�н�����;OWERS_ORIG_ID:=;OWERS_ORIG_NAME:=;OWERS_TEL:=;
		sBaseData.AppendBuffer( "VEHICLE_TYPE:=�γ�;TRANS_TYPE:=010;VEHICLE_NATIONALITY:=640000;OWERS_ID:=;OWERS_NAME:=�н�����;OWERS_ORIG_ID:=1;OWERS_ORIG_NAME:=�н�����;OWERS_TEL:=13801088478;" ) ;
	} else {
		sBaseData.AppendBuffer( "VEHICLE_TYPE:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:typeName" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "TRANS_TYPE:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:transTypeDesc" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "VEHICLE_NATIONALITY:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:city" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "OWERS_ID:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:corpId" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "OWERS_NAME:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:corpName" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "OWERS_ORIG_ID:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:spId" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "OWERS_ORIG_NAME:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:spName" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
		sBaseData.AppendBuffer( "OWERS_TEL:=" ) ;
		sBaseData.AppendBuffer( parser.GetString( "Response::Result::Item:linkPhone" , 0 ) ) ;
		sBaseData.AppendBuffer( ";" ) ;
	}

	rsp->header.msg_len  		= ntouv32( sBaseData.GetLength() + len + sizeof(Footer) ) ;
	rsp->msg_header.data_length = ntouv32( sBaseData.GetLength() ) ;

	DataBuffer buf ;
	buf.writeBlock( msg , len ) ;
	buf.writeBlock( sBaseData.GetBuffer(), sBaseData.GetLength() ) ;

	Footer footer ;
	buf.writeBlock( &footer, sizeof(footer) ) ;

	int access_code = ntouv32( rsp->header.access_code ) ;
	// ���ݶ�Ӧʡ���͸���Ӧʡ����
	_pEnv->GetPasClient()->HandlePasUpData( access_code , buf.getBuffer(), buf.getLength() ) ;//����PAS

	OUT_SEND( NULL, 0, NULL, "UP_BASE_MSG_VEHICLE_ADDED_ACK:%s, color:%d, accesscode:%d",
			car_num , rsp->msg_header.vehicle_color, access_code );

	_pEnv->GetMsgCache()->FreeData( msg ) ;

	return true ;
}
