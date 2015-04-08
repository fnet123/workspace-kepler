/*
 * servicecaller.cpp
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */

#include "srvcaller.h"
#include <xmlparser.h>
#include <comlog.h>
#include "BaseTools.h"
#include "murhash.h"
#include "comutil.h"
#include "tools.h"

#ifndef _WIN32
#include <strings.h>
#define stricmp strcasecmp
#endif

namespace TruckSrv{

#define HASH_SEED    0xffff

string fitodecstr( double fintger )
{
	string dest;
	char buf[16] = { 0 };
	sprintf( buf, "%5.2f", ( double ) fintger );
	dest += buf;
	return dest;
}

CSrvCaller::CSrvCaller( ) :
		_msgqueue( this ), _inited( false )
{
	_packfactory = new CPackFactory( &_unpacker ) ;

	_srv_table[SEND_TEAMMEDIA_REQ] = & CSrvCaller::Proc_SEND_TEAMMEDIA_REQ;
	_srv_table[SEND_MEDIADATA_REQ] = & CSrvCaller::Proc_SEND_MEDIADATA_REQ;
	_srv_table[INFO_PRIMCAR_REQ] = & CSrvCaller::Proc_INFO_PRIMCAR_REQ;
	_srv_table[SET_PRIMCAR_REQ] = & CSrvCaller::Proc_SET_PRIMCAR_REQ;
	_srv_table[INVITE_NUMBER_REQ] = & CSrvCaller::Proc_INVITE_NUMBER_REQ;
	_srv_table[ADD_CARTEAM_REQ] = & CSrvCaller::Proc_ADD_CARTEAM_REQ;
	_srv_table[GET_FRIENDLIST_REQ] = & CSrvCaller::Proc_GET_FRIENDLIST_REQ;
	_srv_table[INVITE_FRIEND_REQ] = & CSrvCaller::Proc_INVITE_FRIEND_REQ;
	_srv_table[ADD_FRIEND_REQ] = & CSrvCaller::Proc_ADD_FRIENDS_REQ;
	_srv_table[QUERY_FRIENDS_REQ] = & CSrvCaller::Proc_QUERY_FRIENDS_REQ;
	_srv_table[DRIVER_LOGOUT_REQ] = & CSrvCaller::Proc_DRIVER_LOGINOUT_REQ;
	_srv_table[DRIVER_LOGIN_REQ] = & CSrvCaller::Proc_DRIVER_LOGIN_REQ;
	_srv_table[QUERY_CARDATA_REQ] = & CSrvCaller::Proc_QUERY_CARDATA_REQ;
	_srv_table[UPLOAD_DATAINFO_REQ] = & CSrvCaller::Proc_UPLOAD_DATAINFO_REQ;
	_srv_table[SEND_SCHEDULE_RSP] = & CSrvCaller::Proc_SEND_SCHEDULE_RSP;
	//_srv_table[TERMINAL_COMMON_RSP]  = &CSrvCaller::Proc_TERMINAL_COMMON_RSP;
	_srv_table[RESULT_SCHEDULE_REQ] = & CSrvCaller::Proc_RESULT_SCHEDULE_REQ;
	_srv_table[QUERY_SCHEDULE_REQ] = & CSrvCaller::Proc_QUERY_SCHEDULE_REQ;
	_srv_table[UPLOAD_SCHEDULE_REQ] = & CSrvCaller::Proc_UPLOAD_SCHEDULE_REQ;
	_srv_table[STATE_SCHEDULE_REQ] = & CSrvCaller::Proc_STATE_SCHEDULE_REQ;
	_srv_table[ALARM_SCHEDULE_REQ] = & CSrvCaller::Proc_ALARM_SCHEDULE_REQ;
	_srv_table[SUBSCRIBE_REQ] = & CSrvCaller::Proc_SCHEDULE_REQ;
	_srv_table[UP_REPORTERROR_REQ] = & CSrvCaller::Proc_ERROR_SCHEDULE_REQ;
	_srv_table[UPLOAD_DATAINFO_REQ] = & CSrvCaller::Proc_AUTO_DATA_SCHEDULE_REQ;
	_srv_table[UP_MSGDATA_REQ] = & CSrvCaller::Proc_MSG_DATA_SCHEDULE_REQ;
	_srv_table[QUERY_INFO_REQ] = & CSrvCaller::Proc_QUERY_INFO_REQ;
	_srv_table[UP_CARDATA_INFO_REQ] = & CSrvCaller::Proc_CARDATA_INFO_REQ;
	_srv_table[UP_QUERY_ORDER_FORM_INFO_REQ] = & CSrvCaller::Proc_QUERY_ORDER_FORM_INFO_REQ;
	_srv_table[UP_ORDER_FORM_INFO_REQ] = & CSrvCaller::Proc_ORDER_FROM_INFO_REQ;
	_srv_table[UP_TRANSPORT_FORM_INFO_REQ] = & CSrvCaller::Proc_TRANSPORT_ORDER_FROM_INFO_REQ;
}

CSrvCaller::~CSrvCaller( )
{
	Stop();

	if ( _packfactory != NULL ) {
		delete _packfactory ;
		_packfactory = NULL ;
	}
}

// ��ʼ��
bool CSrvCaller::Init( IPlugin *pEnv , const char *url, int sendthread , int recvthread , int queuesize )
{
	_pEnv = pEnv;

	if ( ! _thread.init( 1, NULL, this ) ) {
		OUT_ERROR( NULL, 0, NULL, "init CSrvCaller queue thread failed" );
		return false;
	}
	_inited = true;

	// ��¼����hession��URL
	_callUrl.SetString( url );

	_httpcaller.SetReponse( this );

	return _httpcaller.Init( sendthread, recvthread, queuesize );
}

// ����
bool CSrvCaller::Start( void )
{
	_thread.start();

	return _httpcaller.Start();
}

// ֹͣ
void CSrvCaller::Stop( void )
{
	if ( ! _inited ) return;
	_inited = false;
	_monitor.notifyEnd();
	_thread.stop();

	_httpcaller.Stop();
}

// ��������
bool CSrvCaller::getSendTeamMediaReq( unsigned int fd, unsigned int cmd, CSendTeamMediaReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "ownid", uitodecstr( msg->_ownid ).c_str() );
	item.SetVal( "teamid", uitodecstr( msg->_teamid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getSendTeamMediaReq", item );
}
// ��������
bool CSrvCaller::getSendMediaDataReq( unsigned int fd, unsigned int cmd, CSendMediaDataReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "ownid", uitodecstr( msg->_ownid ).c_str() );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getSendMediaDataReq", item );
}
// ����ͷ������Ϣ
bool CSrvCaller::getInfoPriMcarReq( unsigned int fd, unsigned int cmd, CInfoPriMcarReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();

	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );
	item.SetVal( "name", ( const char * ) msg->_name );
	item.SetVal( "type", ustodecstr( msg->_type ).c_str() );
	item.SetVal( "weight", ustodecstr( msg->_weight ).c_str() );
	item.SetVal( "carnum", ( const char * ) msg->_carnum );
	item.SetVal( "dest", ( const char * ) msg->_dest );
	item.SetVal( "speed", ustodecstr( msg->_speed ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getInfoPriMcarReq", item );
}
// ���ñ���Ϊͷ��
bool CSrvCaller::getSetPriMcarReq( unsigned int fd, unsigned int cmd, CSetPriMcarReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();

	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );
	item.SetVal( "teamid", uitodecstr( msg->_teamid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getSetPriMcarReq", item );
}
// ���ӳ�Ա����
bool CSrvCaller::getInviteNumberReq( unsigned int fd, unsigned int cmd, CInviteNumberReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "teamid", uitodecstr( msg->_teamid ).c_str() );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getInviteNumberReq", item );
}
// ��������
bool CSrvCaller::getAddCarTeamReq( unsigned int fd, unsigned int cmd, CAddCarTeamReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "id", uitodecstr( msg->_id ).c_str() );
	item.SetVal( "teamname", ( const char * ) msg->_teamname );
	item.SetVal( "teamnum", ustodecstr( msg->_teamnum ).c_str() );
	item.SetVal( "teamdesc", ( const char* ) msg->_teamdesc );
	item.SetVal( "teamtype", chartodecstr( msg->_teamtype ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getAddCarTeamReq", item );
}
// ��ȡ�����б�
bool CSrvCaller::getGetFriendlistReq( unsigned int fd, unsigned int cmd, CGetFriendListReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "id", uitodecstr( msg->_id ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getGetFriendlistReq", item );
}
// ���복��
bool CSrvCaller::getInviteFriendReq( unsigned int fd, unsigned int cmd, CInviteFriendReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "ownid", uitodecstr( msg->_ownid ).c_str() );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getInviteFriendReq", item );
}

// ��ӳ���
bool CSrvCaller::getAddFriendsReq( unsigned int fd, unsigned int cmd, CAddFriendsReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "ownid", uitodecstr( msg->_ownid ).c_str() );
	item.SetVal( "userid", uitodecstr( msg->_userid ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getAddFriendsReq", item );
}
// ���Ҹ�������
bool CSrvCaller::getQueryFriendsReq( unsigned int fd, unsigned int cmd, CQueryFriendsReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "id", uitodecstr( msg->_id ).c_str() );
	item.SetVal( "Lon", uitodecstr( msg->_Lon ).c_str() );
	item.SetVal( "Lat", uitodecstr( msg->_Lat ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getQueryFriendsReq", item );
}
// ˾��ע��
bool CSrvCaller::getDriverLoginOutReq( unsigned int fd, unsigned int cmd, CDriverLoginOutReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "identify", ( const char * ) msg->_identify );
	item.SetVal( "driverid", ( const char * ) msg->_driverid );
	item.SetVal( "carid", ( const char * ) msg->_carid );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getDriverLoginOutReq", item );
}
// �û������֤
bool CSrvCaller::getDriverLoginReq( unsigned int fd, unsigned int cmd, CDriverLoginReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "identify", ( const char * ) msg->_identify );
	item.SetVal( "driverid", ( const char * ) msg->_driverid );
	item.SetVal( "phonenum", ( const char * ) msg->_phonenum );
	item.SetVal( "simnum", ( const char * ) msg->_simnum );
	item.SetVal( "carid", ( const char * ) msg->_carid );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getDriverLoginReq", item );
}

// ��ѯ������Ϣ
bool CSrvCaller::getQueryCarDataReq( unsigned int fd, unsigned int cmd, CQueryCarDataReq *msg, const char *id )
{
	char szkey[10240] = { 0 };
	sprintf( szkey, "%u_%s_%s_%s", msg->_header._type, uitodecstr( msg->_srcarea ).c_str(),
			uitodecstr( msg->_destarea ).c_str(), ( char* ) msg->_time );
	unsigned int nkey = mur_mur_hash2( szkey, strlen( szkey ), HASH_SEED );

	CQueryCarDataRsp rsp( msg->_header._seq );

	rsp._num = _resultmgr.GetResult< CCarDataInfo >( nkey, msg->_offset, msg->_count, rsp._vec );

	if ( rsp._num > 0 ) {
		DeliverPacket( fd, cmd, & rsp );
		return true;
	}

	unsigned int seq = _httpcaller.GetSequeue();
	// ��Ӷ����д���
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "darea", uitodecstr( msg->_destarea ).c_str() );
	item.SetVal( "sarea", uitodecstr( msg->_srcarea ).c_str() );
	item.SetVal( "time", bcd2utc( ( char* ) msg->_time ).c_str() );
	item.SetVal( "offset", chartodecstr( msg->_offset ).c_str() );
	item.SetVal( "count", chartodecstr( msg->_count ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getQueryCarDataList", item );
}

// �����·����ȵ���Ӧ�������
bool CSrvCaller::getResultScheduleReq( unsigned int fd, unsigned int cmd, CResultScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "scheduleid", safestring( ( const char* ) msg->_scheduleid, sizeof ( msg->_scheduleid ) ).c_str() );
	item.SetVal( "result", ( msg->_result == 0 ) ? "0" : "1" );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putSendScheduleAct", item );
}

// SEND_ SCHEDULE _RSP
// �����·����ȵ���Ӧ�������
bool CSrvCaller::putSendScheduleRsp( unsigned int fd, unsigned int cmd, CSendScheduleRsp *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "result", ( msg->_result == 0 ) ? "0" : "1" );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putSendSchedule", item );
}

//�ն�ͨ��Ӧ��
bool CSrvCaller::putTerminalCommonRsp( unsigned int fd, unsigned int cmd, CTerminalCommonRsp *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "result", ( msg->_result == 0 ) ? "0" : "1" );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putTerminalCommResponse", item );
}

// ��ѯ��ǰ����ĵ��ȵ�
bool CSrvCaller::getQueryScheduleReq( unsigned int fd, unsigned int cmd, CQueryScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "num", chartodecstr( msg->_num ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getQuerySchedule", item );
}
// �ϴ����ȵ�����Ϣ
bool CSrvCaller::putUploadScheduleReq( unsigned int fd, unsigned int cmd, CUploadScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "scheduleid", safestring( ( const char* ) msg->_scheduleid, sizeof ( msg->_scheduleid ) ).c_str() ); //  String	���ȵ���
	item.SetVal( "matchstate", chartodecstr( msg->_matchstate ).c_str() ); //  UINT8	ƥ��״̬(0�ɹ���1ʧ��)
	item.SetVal( "hangid", safestring( ( const char* ) msg->_hangid, sizeof ( msg->_hangid ) ).c_str() ); //	String	�ҳ�ID
	item.SetVal( "hangnum", safestring( ( const char* ) msg->_hangnum, sizeof ( msg->_hangnum ) ).c_str() ); //	String	�ҳ����ƺ�
	item.SetVal( "mtime", bcd2utc( ( char* ) msg->_mtime ).c_str() ); //  String	ƥ��ʱ��

	item.SetVal( "alam", uitodecstr( msg->_info._alam ).c_str() ); //	4	UINT32	������־λ
	item.SetVal( "state", uitodecstr( msg->_info._state ).c_str() ); //	4	UINT32	״̬λ
	item.SetVal( "lon", uitodecstr( msg->_info._lon ).c_str() ); //  4	UINT32	����
	item.SetVal( "lat", uitodecstr( msg->_info._lat ).c_str() ); //	4	UINT32	γ��
	item.SetVal( "height", uitodecstr( msg->_info._height ).c_str() ); //	2	UINT16	�߳�
	item.SetVal( "speed", uitodecstr( msg->_info._speed ).c_str() ); //	2	UINT16	�ٶ�
	item.SetVal( "direction", uitodecstr( msg->_info._direction ).c_str() ); //	2	UINT16	����
	item.SetVal( "time", bcd2utc( ( char* ) msg->_info._time ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putUploadSchedule", item );
}
// �ϱ���������״̬
bool CSrvCaller::putStateScheduleReq( unsigned int fd, unsigned int cmd, CStateScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "scheduleid", safestring( ( const char* ) msg->_scheduleid, sizeof ( msg->_scheduleid ) ).c_str() ); //  String	���ȵ���
	item.SetVal( "action", chartodecstr( msg->_action ).c_str() ); //  UINT8	����
	item.SetVal( "hangid", safestring( ( const char* ) msg->_hangid, sizeof ( msg->_hangid ) ).c_str() ); //	String	�ҳ�ID
	item.SetVal( "hangnum", safestring( ( const char* ) msg->_hangnum, sizeof ( msg->_hangnum ) ).c_str() ); //	String	�ҳ����ƺ�

	item.SetVal( "alam", uitodecstr( msg->_info._alam ).c_str() ); //	4	UINT32	������־λ
	item.SetVal( "state", uitodecstr( msg->_info._state ).c_str() ); //	4	UINT32	״̬λ
	item.SetVal( "lon", uitodecstr( msg->_info._lon ).c_str() ); //  4	UINT32	����
	item.SetVal( "lat", uitodecstr( msg->_info._lat ).c_str() ); //	4	UINT32	γ��
	item.SetVal( "height", uitodecstr( msg->_info._height ).c_str() ); //	2	UINT16	�߳�
	item.SetVal( "speed", uitodecstr( msg->_info._speed ).c_str() ); //	2	UINT16	�ٶ�
	item.SetVal( "direction", uitodecstr( msg->_info._direction ).c_str() ); //	2	UINT16	����
	item.SetVal( "time", bcd2utc( ( char* ) msg->_info._time ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putStateSchedule", item );
}

// �Զ��ϱ������Ϣ
bool CSrvCaller::putAutoDataScheduleReq( unsigned int fd, unsigned int cmd, CAutoDataScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid",id);
	//item.SetVal("sid", (const char *) msg->_sid);
	item.SetVal( "state", chartodecstr( msg->_state ).c_str());

	if ( msg->_state == 2 ) {
		item.SetVal( "space", chartodecstr( msg->_space ).c_str() );
		item.SetVal( "weight", uitodecstr( msg->_weight ).c_str() );
	}
	else{
		item.SetVal("space", "0");
		item.SetVal("weight","0");
	}

	item.SetVal("stime", bcd2utc( ( char* ) msg->_stime ).c_str() ); //����ʱ��
	item.SetVal("srcarea", uitodecstr( msg->_srcarea ).c_str() ); //������
	item.SetVal("destarea", uitodecstr( msg->_destarea ).c_str() ); //Ŀ�ĵ�

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putAutoDataScheduleReq", item);
}
// �ϴ���������
bool CSrvCaller::putErrorScheduleReq( unsigned int fd, unsigned int cmd, CErrorScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "code", uitodecstr( msg->_code ).c_str() ); //1 ������
	item.SetVal( "_desc", ( const char * ) msg->_desc );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putErrorScheduleReq", item );
}
// ���Ĺ���
bool CSrvCaller::putScheduleReq( unsigned int fd, unsigned int cmd, CSubscrbeReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	string s_data = "";

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "cmd", chartodecstr( ( unsigned char ) msg->_cmd ).c_str() ); //1  UINT8 ��������
	item.SetVal( "num", chartodecstr( ( unsigned char ) msg->_num ).c_str() ); //1  UINT8 ���ĸ���

	int num = msg->_num;

	string s_ctype = "";

	for ( int i = 0 ; i < num ; i ++ ) {
		if ( i == num - 1 )
			s_ctype += ustodecstr( msg->_vec[i]->_ctype );
		else
			s_ctype += ustodecstr( msg->_vec[i]->_ctype ) + ",";
	}

	item.SetVal( "ctype", s_ctype.c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putScheduleReq", item );
}
//������Ϣ��ѯ
bool CSrvCaller::getQueryInfoReq( unsigned int fd, unsigned int cmd, CQueryInfoReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();

	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "ctype", chartodecstr( msg->_ctype ).c_str() ); //1 UNINT8 ��������

	if ( msg->_ctype == CQueryInfoReq::Area ) {
		item.SetVal( "area", uitodecstr( msg->_area_id ).c_str() );
	}

	if ( msg->_ctype == CQueryInfoReq::Roea ) {
		item.SetVal( "road", uitodecstr( msg->_road_id ).c_str() );
	}

	if ( msg->_ctype == CQueryInfoReq::Real_Time_Weather ) {
		item.SetVal( "lat", uitodecstr( msg->_lat ).c_str() );
		item.SetVal( "lon", uitodecstr( msg->_lon ).c_str() );
	}

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getQueryInfoReq", item );
}
// �ն�͸��
bool CSrvCaller::getMsgDataScheduleReq( unsigned int fd, unsigned int cmd, CUpMsgDataScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "code", chartodecstr( msg->_code ).c_str() ); //1 UNINT8 ͸������
	item.SetVal( "data", ( const char * ) msg->_data ); // ͸������

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getMsgDataSchedule", item );
}
//�ϴ������Ϣ
bool CSrvCaller::getCarDataInfoReq( unsigned int fd, unsigned int cmd, CUpCarDataInfoReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "sid", safestring( ( const char* ) msg->_sid, sizeof ( msg->_sid ) ).c_str() ); //String �������
	item.SetVal( "status", chartodecstr( msg->_status ).c_str() ); //1 UNINT8 ״̬

	if ( msg->_status == 1 ) {
		item.SetVal( "price", uitodecstr( msg->_price ).c_str() );
	} else {
		item.SetVal( "price", "0" );
	}

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getCarDataInfoReq", item );
}

//������ϸ��ѯ
bool CSrvCaller::getQueryOrderFormInfoReq( unsigned int fd, unsigned int cmd, CQueryOrderFromInfoReq *msg,
		const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "sid", safestring( ( const char* ) msg->_sid, sizeof ( msg->_sid ) ).c_str() ); //String �˵���
	item.SetVal( "order_form_sid",
			safestring( ( const char* ) msg->_order_form_sid, sizeof ( msg->_order_form_sid ) ).c_str() ); //String ������

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getQueryOrderFormInfoReq", item );
}
//�ϴ����˶�����
bool CSrvCaller::getUpOrderFormInfoReq( unsigned int fd, unsigned int cmd, CUpOrderFromInfoReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );

	item.SetVal( "sid", safestring( ( const char* ) msg->_sid, sizeof ( msg->_sid ) ).c_str() ); //String �������
	item.SetVal( "order_form_id", safestring( ( const char* ) msg->_order_form, sizeof ( msg->_order_form ) ).c_str() ); //String ��������
	item.SetVal( "action", chartodecstr( msg->_action ).c_str() ); //UINT8  ���ȵ�����
	item.SetVal( "status", chartodecstr( msg->_status ).c_str() ); //״̬

	item.SetVal( "lon", uitodecstr( msg->_lon ).c_str() ); //����
	item.SetVal( "lat", uitodecstr( msg->_lat ).c_str() ); //γ��

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getOrderFormInfoReq", item );
}
//�ϴ��˵���
bool CSrvCaller::getUpTransportFormInfoReq( unsigned int fd, unsigned int cmd, CTransportFormInfoReq *msg,
		const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;
	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );

	item.SetVal( "sid", safestring( ( const char* ) msg->_sid, sizeof ( msg->_sid ) ).c_str() ); //String �������
	item.SetVal( "action", chartodecstr( msg->_action ).c_str() ); //UINT8  ���ȵ�����
	item.SetVal( "status", chartodecstr( msg->_status ).c_str() ); //״̬

	item.SetVal( "lon", uitodecstr( msg->_lon ).c_str() ); //����
	item.SetVal( "lat", uitodecstr( msg->_lat ).c_str() ); //γ��

	return ProcessMsg( msg->_header._type, seq, "TruckService", "getTransportOrderFormInfoReq", item );
}
// �ϱ��澯״̬
bool CSrvCaller::putAlarmScheduleReq( unsigned int fd, unsigned int cmd, CAlarmScheduleReq *msg, const char *id )
{
	unsigned int seq = _httpcaller.GetSequeue();
	_msgqueue.AddObj( seq, fd, cmd, id, msg );

	CKeyValue item;

	item.SetVal( "seq", uitodecstr( msg->_header._seq ).c_str() );
	item.SetVal( "macid", id );
	item.SetVal( "scheduleid", safestring( ( const char* ) msg->_scheduleid, sizeof ( msg->_scheduleid ) ).c_str() ); //  String	���ȵ���
	item.SetVal( "hangid", safestring( ( const char* ) msg->_hangid, sizeof ( msg->_hangid ) ).c_str() ); //	String	�ҳ�ID
	item.SetVal( "hangnum", safestring( ( const char* ) msg->_hangnum, sizeof ( msg->_hangnum ) ).c_str() ); //	String	�ҳ����ƺ�
	item.SetVal( "alarmtype", uitodecstr( msg->_alarmtype ).c_str() ); //  ��������

	item.SetVal( "alam", uitodecstr( msg->_info._alam ).c_str() ); //	4	UINT32	������־λ
	item.SetVal( "state", uitodecstr( msg->_info._state ).c_str() ); //	4	UINT32	״̬λ
	item.SetVal( "lon", uitodecstr( msg->_info._lon ).c_str() ); //  4	UINT32	����
	item.SetVal( "lat", uitodecstr( msg->_info._lat ).c_str() ); //	4	UINT32	γ��
	item.SetVal( "height", uitodecstr( msg->_info._height ).c_str() ); //	2	UINT16	�߳�
	item.SetVal( "speed", uitodecstr( msg->_info._speed ).c_str() ); //	2	UINT16	�ٶ�
	item.SetVal( "direction", uitodecstr( msg->_info._direction ).c_str() ); //	2	UINT16	����
	item.SetVal( "time", bcd2utc( ( char* ) msg->_info._time ).c_str() );

	return ProcessMsg( msg->_header._type, seq, "TruckService", "putAlarmSchedule", item );
}

//  ��Ӹ�������Ĵ��������������Ϊһ�������Ӧһ������ , %d
bool CSrvCaller::ProcessMsg( unsigned int msgid, unsigned int seq, const char *service, const char *method,
		CKeyValue &item )
{
	if ( msgid == 0 ) return false;

	char key[256] = { 0 };
	sprintf( key, "%d", seq );
	// ���ӳ�����
	_seq2msgid.AddSeqMsg( seq, msgid );

	CQString sXml;
	// ����XML����
	if ( ! CreateRequestXml( sXml, key, service, method, item ) ) {
		OUT_ERROR( NULL, 0, "ProcMsg", "create request xml failed, msg id %x, seq id %d", msgid, seq );
		ProcessError( seq, true );
		return false;
	}
	OUT_INFO( NULL, 0, "Caller", "Request msg id %x,seqid %d,service %s,method %s", msgid, seq, service, method );
	// ����XML����
	if ( ! _httpcaller.Request( seq, ( const char * ) _callUrl, sXml.GetBuffer(), sXml.GetLength() ) ) {
		OUT_ERROR( NULL, 0, "ProcMsg", "caller http request failed , msg id %x, seq id %d", msgid, seq );
		ProcessError( seq, true );
		return false;
	}
	return true;
}

// ���ر���ת��UTF-8
static bool locale2utf8( const char *szdata, const int nlen, CQString &out )
{
	int len = nlen * 4 + 1;
	char *buf = new char[len];
	memset( buf, 0, len );

	if ( g2u( ( char * ) szdata, nlen, buf, len ) == - 1 ) {
		OUT_ERROR( NULL, 0, "auth", "locale2utf8 query %s failed", szdata );
		delete[] buf;
		return false;
	}
	buf[len] = 0;
	out.SetString( buf );
	delete[] buf;

	return true;
}
// ����XML���ݵĴ���
bool CSrvCaller::CreateRequestXml( CQString &sXml, const char *id, const char *service, const char *method,
		CKeyValue &item )
{
	CXmlBuilder XmlOb( "Request", "Param", "Item" );

	XmlOb.SetRootAttribute( "id", id );
	XmlOb.SetRootAttribute( "service", service );
	XmlOb.SetRootAttribute( "method", method );

	CQString stemp;
	// ȡ���Ƿ����������
	int count = item.GetSize();
	if ( count > 0 ) {
		for ( int i = 0 ; i < count ; ++ i ) {
			_KeyValue &vk = item.GetVal( i );
			// �������Ϊ�յĻ��Ͳ���Ӵ�����
			if ( vk.key.empty() || vk.val.empty() ) {
				continue;
			}
			// ת��UTF-8�ı��봦��
			if ( ! locale2utf8( vk.val.c_str(), vk.val.length(), stemp ) ) {
				// �������Ϊ���򲻴�
				continue;
			}
			// ��������
			XmlOb.SetItemAttribute( vk.key.c_str(), stemp.GetBuffer() );
		}
		XmlOb.InsertItem();
	}
	// ����XML����
	XmlOb.GetXmlText( sXml );
	// �����XML������
	OUT_PRINT( NULL, 0, "XML", "Request GBK XML:%s", sXml.GetBuffer() );

	return ( ! sXml.IsEmpty() );
}

// ����HttpCaller�ص�
void CSrvCaller::ProcessResp( unsigned int seqid, const char *xml, const int len, const int err )
{
	// ������Ӧ��XML�������
	if ( xml == NULL || err != HTTP_CALL_SUCCESS || len == 0 ) {
		OUT_ERROR( NULL, 0, "Caller", "Process seqid %u, error %d, %s:%d", seqid, err, __FILE__, __LINE__ );
		ProcessError( seqid, true );
		return;
	}

	// ������ݸ�ʽ����ȷ
	if ( strstr( xml, "Response" ) == NULL || strstr( xml, "Result" ) == NULL || strstr( xml, "Item" ) == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "ProcessResponse seqid %d , data error, xml %s", seqid, xml );
		ProcessError( seqid, true );
		return;
	}

	// ȡ�ö�Ӧ���ID
	unsigned int msgid = 0;
	if ( ! _seq2msgid.GetSeqMsg( seqid, msgid ) ) {
		OUT_ERROR( NULL, 0, "Caller", "ProcessResponse seqid %d , msg id empty", seqid );
		ProcessError( seqid, false );
		return;
	}

	ServiceTable::iterator it = _srv_table.find( msgid );
	// ������Ϣ��ӦID�ص�����
	if ( it == _srv_table.end() ) {
		OUT_ERROR( NULL, 0, "Caller", "ProcessResponse seqid %d , call method empty", seqid );
		ProcessError( seqid, false );
		return;
	}

	// ����XML������
	OUT_PRINT( NULL, 0, NULL, "xml:%s", xml );
	// ���ô�����
	if ( ! ( this->* ( it->second ) )( seqid, xml ) ) {
		// ������ݲ���ȷ
		ProcessError( seqid, false );
		return;
	}
}

// �����Ӧ��XML������
bool CSrvCaller::Proc_QUERY_CARDATA_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}
	// ToDo : cope result
	// <Item sid="4" time="201206060906"  sarea="beijing" darea="changsha" stype="0" model="0" bulk="0" num="1" phone="15001088478" info="test4"/>

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CQueryCarDataReq *req = ( CQueryCarDataReq * ) obj->_msg;

	char szkey[10240] = { 0 };
	sprintf( szkey, "%u_%s_%s_%s", req->_header._type, uitodecstr( req->_srcarea ).c_str(),
			uitodecstr( req->_destarea ).c_str(), ( char* ) req->_time );
	unsigned int nkey = mur_mur_hash2( szkey, strlen( szkey ), HASH_SEED );

	CQueryCarDataRsp rsp( req->_header._seq );

	int count = parser.GetCount( "Response::Result::Item:sid" );

    const char *value = NULL;
	if ( count > 0 ) {
		_resultmgr.ClearResult( nkey );

		for ( int i = 0 ; i < count ; ++ i ) {

			CCarDataInfo *info = new CCarDataInfo;
			value = parser.GetString( "Response::Result::Item:sid", i );
			if (value == NULL){
			   OUT_ERROR( NULL, 0, NULL, "get sid failed" );
			   return false;
			}
			safe_memncpy( ( char* ) info->_sid,value,sizeof ( info->_sid));

			value = parser.GetString( "Response::Result::Item:time", i );
			if (value == NULL){
				OUT_ERROR( NULL, 0, NULL, "get time failed" );
				return false;
			}

			utc2bcd(value, ( char* ) info->_time );
			info->_sarea = parser.GetInteger( "Response::Result::Item:sarea", i );
			info->_darea = parser.GetInteger( "Response::Result::Item:darea", i );

			value = parser.GetString( "Response::Result::Item:stype", i );
			if (value == NULL){
			   OUT_ERROR( NULL, 0, NULL, "get stype failed" );
			   return false;
			}
			info->_stype = value;
			info->_model = parser.GetInteger( "Response::Result::Item:model", i );
			info->_bulk  = parser.GetInteger( "Response::Result::Item:bulk", i );
			info->_nnum  = parser.GetInteger( "Response::Result::Item:num", i );

			safe_memncpy( ( char* ) info->_contact, parser.GetString( "Response::Result::Item:contact", i ),
					sizeof ( info->_contact ) );

			value = parser.GetString( "Response::Result::Item:phone", i );
			if (value == NULL){
				OUT_ERROR( NULL, 0, NULL, "get phone failed" );
				return false;
			}
			strtoBCD(value, ( char* ) info->_phone );

			value = parser.GetString( "Response::Result::Item:info", i );
			if (value == NULL){
				OUT_ERROR( NULL, 0, NULL, "get info failed" );
				return false;
		    }
			info->_info = value;

			_resultmgr.AddResult( nkey, info );
		}
	}
	rsp._num = _resultmgr.GetResult< CCarDataInfo >( nkey, req->_offset, req->_count, rsp._vec );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_QUERY_CARDATA_REQ dest area %s, src area %s",
			uitodecstr(req->_destarea).c_str(), uitodecstr(req->_srcarea).c_str() );

	_msgqueue.FreeObj( obj );

	return true;
}
// ��������
bool CSrvCaller::Proc_SEND_TEAMMEDIA_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CSendTeamMediaReq *req = ( CSendTeamMediaReq * ) obj->_msg;
	CSendTeamMediaRsp rsp( req->_header._seq );

	rsp._teamid = parser.GetInteger( "Response::Result::Item:teamid", 0 );
	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );
	//voice
	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_SEND_TEAMMEDIA_REQ name state %d", rsp._state );

	_msgqueue.FreeObj( obj );

	return true;
}
// ��������
bool CSrvCaller::Proc_SEND_MEDIADATA_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CSendMediaDataReq *req = ( CSendMediaDataReq * ) obj->_msg;
	CSendMediaDataRsp rsp( req->_header._seq );

	rsp._ownid = parser.GetInteger( "Response::Result::Item:ownid", 0 );
	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	//voice

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_SEND_MEDIADATA_REQ name userid %d", rsp._userid );

	_msgqueue.FreeObj( obj );

	return true;
}
// ����ͷ������Ϣ
bool CSrvCaller::Proc_INFO_PRIMCAR_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CInfoPriMcarReq *req = ( CInfoPriMcarReq * ) obj->_msg;
	CInfoPriMcarRsp rsp( req->_header._seq );

	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_INFO_PRIMCAR_REQ name state %d", rsp._state );

	_msgqueue.FreeObj( obj );

	return true;
}
// ���ñ���Ϊͷ��
bool CSrvCaller::Proc_SET_PRIMCAR_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CSetPriMcarReq *req = ( CSetPriMcarReq * ) obj->_msg;
	CSetPriMcarRsp rsp( req->_header._seq );

	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );
	OUT_INFO( NULL, 0, "Caller", "Proc_Proc_SET_PRIMCAR_REQ name userid %d", rsp._userid );

	_msgqueue.FreeObj( obj );

	return true;
}
//���ӳ�Ա����
bool CSrvCaller::Proc_INVITE_NUMBER_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CInviteNumberReq *req = ( CInviteNumberReq * ) obj->_msg;
	CInviteNumberRsp rsp( req->_header._seq );

	rsp._teamid = parser.GetInteger( "Response::Result::Item:teamid", 0 );
	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_INVITE_NUMBER_REQ name userid %d", rsp._userid );
	_msgqueue.FreeObj( obj );

	return true;
}
// ��������
bool CSrvCaller::Proc_ADD_CARTEAM_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CAddCarTeamReq *req = ( CAddCarTeamReq * ) obj->_msg;
	CAddCarTeamRsp rsp( req->_header._seq );

	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	rsp._teamid = parser.GetInteger( "Response::Result::Item:teamid", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_ADD_CARTEAM_REQ name userid %d", rsp._userid );

	_msgqueue.FreeObj( obj );

	return true;
}
// ��ȡ�����б�
bool CSrvCaller::Proc_GET_FRIENDLIST_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CGetFriendListReq *req = ( CGetFriendListReq * ) obj->_msg;
	CGetFriendListRsp rsp( req->_header._seq );

	int count = parser.GetCount( "Response::Result::Item:avatar" );

	if ( count > 0 ) {
		for ( int i = 0 ; i < count ; ++ i ) {

			CFriendDataInfo *info = new CFriendDataInfo;

			info->_avatar = parser.GetInteger( "Response::Result::Item:avatar", i);
			info->_userid = parser.GetInteger( "Response::Result::Item:userid", i);

			safe_memncpy(( char * ) info->_username, parser.GetString( "Response::Result::Item:username", i ),
					     sizeof(info->_username));

			info->_dest = parser.GetString( "Response::Result::Item:dest", i ), info->_Type = parser.GetInteger(
					"Response::Result::Item:Type", i );
			info->_bulk = parser.GetInteger( "Response::Result::Item:bulk", i );
			info->_weight = parser.GetInteger( "Response::Result::Item:weight", i );
			info->_model = parser.GetInteger( "Response::Result::Item:model", i );
			info->_org = parser.GetString( "Response::Result::Item:_org", i ), info->_desc = parser.GetString(
					"Response::Result::Item:_org", i ),

			info->AddRef();
			rsp._vec.push_back( info );
		}
	}

	rsp._num = count;
	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_GET_FRIENDLIST_REQ name num %d", rsp._num );

	_msgqueue.FreeObj( obj );

	return true;
}
// �������
bool CSrvCaller::Proc_INVITE_FRIEND_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CInviteFriendReq *req = ( CInviteFriendReq * ) obj->_msg;
	CInviteFriendRsp rsp( req->_header._seq );

	rsp._ownid = parser.GetInteger( "Response::Result::Item:ownid", 0 );
	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_INVITE_FRIEND_REQ name userid %d", rsp._userid );

	_msgqueue.FreeObj( obj );

	return true;
}
// ��Ӻ���
bool CSrvCaller::Proc_ADD_FRIENDS_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CAddFriendsReq *req = ( CAddFriendsReq * ) obj->_msg;
	CAddFriendsRsp rsp( req->_header._seq );

	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );
	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_ADD_FRIENDS_REQ name userid %d", rsp._userid );

	_msgqueue.FreeObj( obj );

	return true;
}
// ���Ҹ�������
bool CSrvCaller::Proc_QUERY_FRIENDS_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}
	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CQueryFriendsReq *req = ( CQueryFriendsReq * ) obj->_msg;
	CQueryFriendsRsp rsp( req->_header._seq );

	int count = parser.GetCount( "Response::Result::Item:avatar" );
	if ( count > 0 ) {
		for ( int i = 0 ; i < count ; ++ i ) {
			CFriendDataInfo *info = new CFriendDataInfo;
			info->_avatar = parser.GetInteger( "Response::Result::Item:avatar", i );
			info->_userid = parser.GetInteger( "Response::Result::Item:userid", i );
			safe_memncpy( ( char * ) info->_username, parser.GetString( "Response::Result::Item:username", i ),
					sizeof ( info->_username ) );

			info->_dest = parser.GetString( "Response::Result::Item:dest", i ), info->_Type = parser.GetInteger(
					"Response::Result::Item:Type", i );
			info->_bulk = parser.GetInteger( "Response::Result::Item:bulk", i );
			info->_weight = parser.GetInteger( "Response::Result::Item:weight", i );
			info->_model = parser.GetInteger( "Response::Result::Item:model", i );
			info->_org = parser.GetString( "Response::Result::Item:_org", i ), info->_desc = parser.GetString(
					"Response::Result::Item:_org", i ),

			info->AddRef();
			rsp._vec.push_back( info );
		}
	}
	rsp._num = count;

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_QUERY_FRIENDS_REQ name num %d", rsp._num );

	_msgqueue.FreeObj( obj );

	return true;
}
// ˾�����ע��
bool CSrvCaller::Proc_DRIVER_LOGINOUT_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CDriverLoginOutReq *req = ( CDriverLoginOutReq * ) obj->_msg;
	CDriverLoginOutRsp rsp( req->_header._seq );

	utc2bcd( parser.GetString( "Response::Result::Item:time", 0 ), ( char* ) rsp._time );

	rsp._state = parser.GetInteger( "Response::Result::Item:state", 0 );
	safe_memncpy( ( char* ) rsp._name, parser.GetString( "Response::Result::Item:name", 0 ), sizeof ( rsp._name ) );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_DRIVER_LOGINOUT_REQ name id %s", (const char*)rsp._name );
	_msgqueue.FreeObj( obj );
	return true;
}

// ˾����ݵ�½��֤  xfm
bool CSrvCaller::Proc_DRIVER_LOGIN_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CDriverLoginReq *req = ( CDriverLoginReq * ) obj->_msg;
	CDriverLoginRsp rsp( req->_header._seq );

	utc2bcd( parser.GetString( "Response::Result::Item:time", 0 ), ( char* ) rsp._time );

	rsp._userid = parser.GetInteger( "Response::Result::Item:userid", 0 );
	rsp._pic = parser.GetString( "Response::Result::Item:pic", 0 );

	safe_memncpy( ( char* ) rsp._name, parser.GetString( "Response::Result::Item:name", 0 ), sizeof ( rsp._name ) );

	safe_memncpy( ( char* ) rsp._simnum, parser.GetString( "Response::Result::Item:simnum", 0 ),
			sizeof ( rsp._simnum ) );

	rsp._grade = parser.GetInteger( "Response::Result::Item:grade", 0 );
	rsp._score = parser.GetInteger( "Response::Result::Item:score", 0 );

	safe_memncpy( ( char* ) rsp._carnum, parser.GetString( "Response::Result::Item:carnum", 0 ),
			sizeof ( rsp._carnum ) );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );
	OUT_INFO( NULL, 0, "Caller", "Proc_DRIVER_LOGIN_REQ name id %s", (const char*)rsp._name );
	_msgqueue.FreeObj( obj );

	return true;
}
// ���������ϱ�
bool CSrvCaller::Proc_UPLOAD_DATAINFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;

	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CAutoDataScheduleReq *req = ( CAutoDataScheduleReq * ) obj->_msg;

	CPlatFormCommonRsp rsp( req->_header._seq ); //CAutoDataScheduleRsp

	rsp._type   = UPLOAD_DATAINFO_REQ;
	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_UPLOAD_DATAINFO_REQ state id %d", req->_state );

	_msgqueue.FreeObj( obj );
	return true;
}

// �·����ȵ��Զ�Ӧ���ϱ�
bool CSrvCaller::Proc_SEND_SCHEDULE_RSP( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CSendScheduleRsp *req = ( CSendScheduleRsp * ) obj->_msg;
	CSendScheduleRsp rsp( req->_header._seq );

	rsp._result = 0x00;

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );
	OUT_INFO( NULL, 0, "Caller", "Proc_SEND_SCHEDULE_RSP schedule id %d", (const char*)req->_result );

	_msgqueue.FreeObj( obj );
	return true;
}
// �����ϱ��·����ȵ����
bool CSrvCaller::Proc_RESULT_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CResultScheduleReq *req = ( CResultScheduleReq * ) obj->_msg;
	CResultScheduleRsp rsp( req->_header._seq );

	rsp._result = 0x00;

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_RESULT_SCHEDULE_REQ schedule id %s", (const char*)req->_scheduleid );

	_msgqueue.FreeObj( obj );
	return true;
}
//xfm
bool CSrvCaller::Proc_QUERY_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CQueryScheduleReq *req = ( CQueryScheduleReq * ) obj->_msg;
	CQueryScheduleRsp rsp( req->_header._seq );

	int count = parser.GetCount( "Response::Result::Item:scheduleid" );
	if ( count > 0 ) {
		for ( int i = 0 ; i < count ; ++ i ) {
			CScheduleInfo *info = new CScheduleInfo;
			safe_memncpy( ( char* ) info->_scheduleid, parser.GetString( "Response::Result::Item:scheduleid", i ),
					sizeof ( info->_scheduleid ) );
			info->_sarea = parser.GetString( "Response::Result::Item:sarea", i );
			safe_memncpy( ( char* ) info->_srcid, parser.GetString( "Response::Result::Item:srcid", i ),
					sizeof ( info->_srcid ) );
			utc2bcd( parser.GetString( "Response::Result::Item:start", i ), ( char* ) info->_start );
			safe_memncpy( ( char* ) info->_carnum, parser.GetString( "Response::Result::Item::carnum", i ),
					sizeof ( info->_carnum ) );
			safe_memncpy( ( char* ) info->_hangid, parser.GetString( "Response::Result::Item:hangid", i ),
					sizeof ( info->_hangid ) );
			info->_darea = parser.GetString( "Response::Result::Item:darea", i );
			safe_memncpy( ( char* ) info->_destid, parser.GetString( "Response::Result::Item:destid", i ),
					sizeof ( info->_destid ) );
			utc2bcd( parser.GetString( "Response::Result::Item:atime", i ), ( char* ) info->_atime );
			utc2bcd( parser.GetString( "Response::Result::Item:stime", i ), ( char* ) info->_stime );

			info->AddRef();
			rsp._vec.push_back( info );
		}
	}

	rsp._num = count;
	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_QUERY_SCHEDULE_REQ num %d", req->_num );

	_msgqueue.FreeObj( obj );

	return true;
}

bool CSrvCaller::Proc_UPLOAD_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CUploadScheduleReq *req = ( CUploadScheduleReq * ) obj->_msg;
	CUploadScheduleRsp rsp( req->_header._seq );

	rsp._result = 0;
	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_UPLOAD_SCHEDULE_REQ schedule id %s", (const char*)req->_scheduleid );

	_msgqueue.FreeObj( obj );

	return true;
}

bool CSrvCaller::Proc_STATE_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CStateScheduleReq *req = ( CStateScheduleReq * ) obj->_msg;

	CStateScheduleRsp rsp( req->_header._seq );

	rsp._result = 0;

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_STATE_SCHEDULE_REQ schedule id %s", req->_scheduleid );

	_msgqueue.FreeObj( obj );

	return true;
}

bool CSrvCaller::Proc_ALARM_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CAlarmScheduleReq *req = ( CAlarmScheduleReq * ) obj->_msg;

	CAlarmScheduleRsp rsp( req->_header._seq );

	rsp._result = 0;

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_ALARM_SCHEDULE_REQ schedule id %s", req->_scheduleid );

	_msgqueue.FreeObj( obj );

	return true;
}

//���Ĺ���  xfm
bool CSrvCaller::Proc_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CSubscrbeReq *req = ( CSubscrbeReq * ) obj->_msg;
	CSubscrbeRsp rsp( req->_header._seq );

	int count = parser.GetCount( "Response::Result::Item:ctype" );

	if ( count > 0 ) {
		for ( int i = 0 ; i < count ; i ++ ) {
			CSubscribeList *info = new CSubscribeList;
			info->_ctype = parser.GetInteger( "Response::Result::Item:ctype", i );
			info->AddRef();
			rsp._vec.push_back( info );
		}
	}
	rsp._num = count;
	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_SCHEDULE_REQ schedule type %d", rsp._num );

	_msgqueue.FreeObj( obj );

	return true;
}
//������Ϣ��ѯ
bool CSrvCaller::Proc_QUERY_INFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CQueryInfoReq *req = ( CQueryInfoReq * ) obj->_msg;
	CQueryInfoRsp rsp( req->_header._seq );

	rsp._ctype = parser.GetInteger( "Response::Result::Item:ctype", 0 );

	if ( rsp._ctype == CQueryInfoReq::Area ) {
		rsp._area_id = parser.GetInteger( "Response::Result::Item:area", 0 );
	}

	if ( rsp._ctype == CQueryInfoReq::Roea ) {
		rsp._road_id = parser.GetInteger( "Response::Result::Item:road", 0 );
	}

	rsp._data = parser.GetString( "Response::Result::Item:data", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", ":Proc_QUERY_INFO_REQ type %d", rsp._ctype );

	_msgqueue.FreeObj( obj );

	return true;
}
//�ϴ������Ϣ
bool CSrvCaller::Proc_CARDATA_INFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;

	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CUpCarDataInfoReq *req = ( CUpCarDataInfoReq * ) obj->_msg;

	CPlatFormCommonRsp rsp( req->_header._seq );

	rsp._type = UP_CARDATA_INFO_REQ;
	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_CARDATA_INFO_REQ result id %d", rsp._result );

	_msgqueue.FreeObj( obj );

	return true;
}
//������ϸ��ѯ
bool CSrvCaller::Proc_QUERY_ORDER_FORM_INFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CQueryOrderFromInfoReq *req = ( CQueryOrderFromInfoReq * ) obj->_msg;
	CQueryOrderFromInfoRsp rsp(req->_header._seq);

	if (rsp.lpOrderList == NULL) {
		rsp.lpOrderList = new COrderList;
	}

	const char *value = NULL;

	value = parser.GetString("Response::Result::Item:ssid",0);
	if (value == NULL){
	   OUT_ERROR( NULL, 0, NULL, "get ssid failed" );
	   return false;
	}
	safe_memncpy((char*) rsp.lpOrderList->_sid,value,sizeof(rsp.lpOrderList->_sid));

	value = parser.GetString("Response::Result::Item:stype",0);
    if (value == NULL){
	   OUT_ERROR( NULL, 0, NULL, "get stype failed" );
	   return false;
	}
	rsp.lpOrderList->_stype = value;
	rsp.lpOrderList->_num = parser.GetInteger("Response::Result::Item:num",0);
	rsp.lpOrderList->_model = parser.GetInteger("Response::Result::Item:model",0);
	rsp.lpOrderList->_bulk = parser.GetInteger("Response::Result::Item:bulk",0);

	value = parser.GetString("Response::Result::Item:company",0);
	if (value == NULL){
	  OUT_ERROR( NULL, 0, NULL, "get company failed" );
	  return false;
	}

	rsp.lpOrderList->_company = value;

	value = parser.GetString("Response::Result::Item:s_contact",0);
	if (value == NULL){
	  OUT_ERROR( NULL, 0, NULL, "get s_contact failed" );
	  return false;
    }

	safe_memncpy((char*) rsp.lpOrderList->_s_contact,value,sizeof(rsp.lpOrderList->_s_contact));

	value = parser.GetString("Response::Result::Item:s_phone",0);
	if (value == NULL){
	   OUT_ERROR( NULL, 0, NULL, "get s_phone failed" );
	   return false;
	}
	strtoBCD(value,(char *)rsp.lpOrderList->_sphone);

	value = parser.GetString("Response::Result::Item:submit_time",0);
    if (value == NULL){
	  OUT_ERROR( NULL, 0, NULL, "get submit_time failed" );
	  return false;
    }
	utc2bcd(value,(char *)rsp.lpOrderList->_s_submit_time);


	value = parser.GetString("Response::Result::Item:s_address",0);
	if (value == NULL){
		  OUT_ERROR( NULL, 0, NULL, "get s_address failed" );
		  return false;
	}
	rsp.lpOrderList->_s_address = value;
	rsp.lpOrderList->_s_lon = parser.GetInteger("Response::Result::Item:s_lon",0);
	rsp.lpOrderList->_s_lat = parser.GetInteger("Response::Result::Item:s_lat",0);

	value = parser.GetString("Response::Result::Item:r_contact",0);
    if (value == NULL){
	  OUT_ERROR( NULL, 0, NULL, "get r_contact failed" );
	  return false;
	}

	safe_memncpy((char*) rsp.lpOrderList->_r_contact,value,sizeof(rsp.lpOrderList->_r_contact));

	value = parser.GetString("Response::Result::Item:r_phone",0);
	if (value == NULL){
      OUT_ERROR( NULL, 0, NULL, "get r_phone failed" );
	  return false;
    }
	strtoBCD(value,(char *)rsp.lpOrderList->_rphone);

	value = parser.GetString("Response::Result::Item:arrival_time",0);
    if (value == NULL){
	   OUT_ERROR( NULL, 0, NULL, "get arrival_time failed" );
	   return false;
	}
	utc2bcd(value,(char *)rsp.lpOrderList->_s_arrival_time);

	value = parser.GetString("Response::Result::Item:r_address",0);
	if (value == NULL){
	   OUT_ERROR( NULL, 0, NULL, "get r_address failed" );
	   return false;
    }
	rsp.lpOrderList->_r_address = value;

	rsp.lpOrderList->_r_lon = parser.GetInteger("Response::Result::Item:r_lon",0);
	rsp.lpOrderList->_r_lat = parser.GetInteger("Response::Result::Item:r_lat",0);

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_QUERY_ORDER_FORM_INFO_REQ num %d",rsp.lpOrderList->_num);

	_msgqueue.FreeObj( obj );
	return true;
}
//�ϴ����˶���״̬
bool CSrvCaller::Proc_ORDER_FROM_INFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;

	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CUpOrderFromInfoReq *req = ( CUpOrderFromInfoReq * ) obj->_msg;

	CPlatFormCommonRsp rsp( req->_header._seq );

	rsp._type = UP_ORDER_FORM_INFO_REQ;
	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_ORDER_FROM_INFO_REQ result id %d", rsp._result );

	_msgqueue.FreeObj( obj );
	return true;
}
//�ϴ��˵�״̬
bool CSrvCaller::Proc_TRANSPORT_ORDER_FROM_INFO_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;

	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%d", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CTransportFormInfoReq *req = ( CTransportFormInfoReq * ) obj->_msg;

	CPlatFormCommonRsp rsp( req->_header._seq );

	rsp._type = UP_TRANSPORT_FORM_INFO_REQ;
	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_TRANSPORT_ORDER_FROM_INFO_REQ result id %d", rsp._result );

	_msgqueue.FreeObj( obj );
	return true;
}

bool CSrvCaller::Proc_ERROR_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CErrorScheduleReq *req = ( CErrorScheduleReq * ) obj->_msg;

	CErrorScheduleRsp rsp( req->_header._seq );

	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_ERROR_SCHEDULE_REQ schedule type %d", req->_code );

	_msgqueue.FreeObj( obj );

	return true;
}

bool CSrvCaller::Proc_AUTO_DATA_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}
	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CAutoDataScheduleReq *req = ( CAutoDataScheduleReq * ) obj->_msg;
	CPlatFormCommonRsp rsp( req->_header._seq );

	rsp._type = UPLOAD_DATAINFO_REQ;
	rsp._result = parser.GetInteger( "Response::Result::Item:result", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );

	OUT_INFO( NULL, 0, "Caller", "Proc_AUTO_DATA_SCHEDULE_REQ schedule state %d", req->_state );

	_msgqueue.FreeObj( obj );

	return true;
}

bool CSrvCaller::Proc_MSG_DATA_SCHEDULE_REQ( unsigned int seqid, const char *xml )
{
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );

	if ( obj == NULL ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d is time out remove it", seqid );
		return false;
	}

	CXmlParser parser;
	// ����XML������
	if ( ! parser.LoadXml( xml ) ) {
		OUT_ERROR( NULL, 0, "Caller", "seqid %d parser xml error, xml:%s", seqid, xml );
		_msgqueue.FreeObj( obj );
		return false;
	}

	CUpMsgDataScheduleReq *req = ( CUpMsgDataScheduleReq * ) obj->_msg;
	CUpMsgDataScheduleRsp rsp( req->_header._seq );

	rsp._code = parser.GetInteger( "Response::Result::Item:code", 0 );
	rsp._data = parser.GetString( "Response::Result::Item:data", 0 );

	DeliverPacket( obj->_fd, obj->_cmd, & rsp );
	OUT_INFO( NULL, 0, "Caller", "Proc_MSG_DATA_SCHEDULE_REQ code %d", req->_code );

	_msgqueue.FreeObj( obj );

	return true;
}
// ��������
void CSrvCaller::DeliverPacket( unsigned int fd, unsigned int cmd, IPacket *msg )
{
	CPacker pack;
	_packfactory->Pack( msg, pack );

	_pEnv->OnDeliver( fd, pack.getBuffer(), pack.getLength(), cmd );
}

// ��������ʱ������
void CSrvCaller::OnTimeOut( unsigned int seq, unsigned int fd, unsigned int cmd, const char *id, IPacket *msg )
{
	// ToDo:  ��ʱ����
	OUT_INFO( NULL, 0, id, "seq id %d, fd %d, cmd %04x msg_type %04x time out", seq, fd, cmd, msg->_header._type );
}

// �߳�ִ�ж���
void CSrvCaller::run( void *param )
{
	while ( _inited ) {
		// ��ⳬʱ��Ϣ
		_msgqueue.Check( 120 );
		_resultmgr.Check( 300 );

		share::Synchronized syn( _monitor );
		{
			_monitor.wait( 30 );
		}
	}
}

// �����������
void CSrvCaller::ProcessError( unsigned int seqid, bool remove )
{
	// ����ص�TABLE��SEQ��ӦMAP
	if ( remove ) _seq2msgid.RemoveSeq( seqid );

	// ��Ӷ����д���
	CMsgQueue::_MsgObj *obj = _msgqueue.GetObj( seqid );
	if ( obj == NULL ) {
		return;
	}

	IPacket * req = obj->_msg;
	// ���������ݳ�ʱ���ߴ�������

	switch ( req->_header._type )
	{
	case QUERY_CARDATA_REQ: // ��ѯ�����Ϣ
		{
			CQueryCarDataRsp rsp( req->_header._seq );
			rsp._num = 0;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case SEND_SCHEDULE_RSP:
		{
			CSendScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case SUBSCRIBE_REQ: // ���Ĺ���
		{
			CSubscrbeRsp rsp( req->_header._seq );
			rsp._num = 0;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case UP_REPORTERROR_REQ: // �ϴ���������
		{
			CErrorScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case QUERY_SCHEDULE_REQ: // 0x1041	 // ��ѯ���ȵ�����
		{
			CQueryScheduleRsp rsp( req->_header._seq );
			rsp._num = 0;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case UPLOAD_SCHEDULE_REQ: // 0x1042	 // �ϴ����ȵ�
		{
			CUploadScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case STATE_SCHEDULE_REQ: //	0x1043	 // �ϱ����ȵ�״̬
		{
			CStateScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case ALARM_SCHEDULE_REQ: //	0x1044	 // ���Ҹ澯
		{
			CAlarmScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	case RESULT_SCHEDULE_REQ: // 0x1045  // �ϱ����ȵ���״̬
		{
			CResultScheduleRsp rsp( req->_header._seq );
			rsp._result = 0x01;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break ;
	case  UPLOAD_DATAINFO_REQ:
	case  UP_CARDATA_INFO_REQ:
	case  UP_CARDATA_INFO_CONFIRM_REQ:
	case  UP_QUERY_ORDER_FORM_INFO_REQ:
	case  UP_ORDER_FORM_INFO_REQ:
	case  UP_TRANSPORT_FORM_INFO_REQ:
		{
			CPlatFormCommonRsp rsp( req->_header._seq);
			rsp._result = 0x01;
			rsp._type   = req->_header._type;
			DeliverPacket( obj->_fd, obj->_cmd, & rsp );
		}
		break;
	default:
		break;
	}

	_msgqueue.FreeObj( obj );
}

} ;
