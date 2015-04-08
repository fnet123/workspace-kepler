/**********************************************
 * systemenv.h
 *
 *  Created on: 2011-07-24
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments: ���������࣬��Ҫ������Ҫ�����Ķ��󣬶�������󽻻�֮����н磬
 *    �������κ���������֮��Ľ���������ͨ�������������ʵ��ֱ�ӽ�����ʹ������֮��͸������
 *    Ҳʹ�ýṹ����������ÿһ������ʵ�֣�������ʵ��Init Start Stop����������Ҫʵ��ϵͳ
 *    ֮���ͳһ�淶������
 *********************************************/

#include <comlog.h>
#include <cConfig.h>
#include <tools.h>
#include "systemenv.h"
#include "truck.h"
#include "packfactory.h"

using namespace TruckSrv;

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         = NULL ;
	_waytester		= NULL ;
	_waytester      = new TruckSrv::CTruck;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _waytester != NULL ){
		delete _waytester ;
		_waytester = NULL ;
	}

	if ( _config != NULL ){
		delete _config ;
		_config = NULL ;
	}
}

bool CSystemEnv::InitLog( const char * logpath  , const char *logname )
{
	char szbuf[512] = {0} ;
	sprintf( szbuf, "mkdir -p %s", logpath ) ;
	system( szbuf );

	sprintf( szbuf, "%s/%s" , logpath , logname ) ;
	CHGLOG( szbuf ) ;

	int log_num = 20;
	if ( ! GetInteger("log_num" , log_num ) ){
		printf( "get log number falied\n" ) ;
		log_num = 0 ;
	}

	int log_size = 20 ;
	if ( ! GetInteger("log_size" , log_size) ){
		printf( "get log size failed\n" ) ;
		log_size = 20 ;
	}
	// ȡ����־����
	int log_level = 3 ;
	if ( ! GetInteger("log_level" , log_level) ) {
		log_level = 3 ;
	}
	// ������־����
	SETLOGLEVEL(log_level) ;
	CHGLOGSIZE(log_size);
	CHGLOGNUM(log_num);

	return true ;
}

bool CSystemEnv::Init( const char *file , const char *logpath , const char *logname )
{
	_config = new CCConfig( file ) ;
	if ( _config == NULL ) {
		printf( "CSystemEnv::Init load config file %s failed\n", file ) ;
		return false ;
	}

	InitLog( logpath , logname ) ;

	return _waytester->Init( this , "http://127.0.0.1:8080" , 1, 1, 1000 ) ;
}

bool CSystemEnv::Start( void )
{
	_initialed = true ;

	return _waytester->Start() ;
}

void CSystemEnv::Stop( void )
{
	if ( ! _initialed )
		return ;

	_initialed = false ;

	_waytester->Stop() ;
}

// ȡ������ֵ
bool CSystemEnv::GetInteger( const char *key , int &value )
{
	char buf[1024] = {0} ;
	if ( _config->fGetValue("COMMON" , key, buf ) == -1 ){
		return false ;
	}
	value = atoi( buf ) ;
	return true ;
}

// ȡ���ַ���ֵ
bool CSystemEnv::GetString( const char *key , char *value )
{
	char buf[512] = {0} ;
	if ( _config->fGetValue("COMMON", key , buf ) == -1 ){
		return false ;
	}
	return getenvpath( buf , value );
}

// �ص���������
void CSystemEnv::OnDeliver( unsigned int fd, const char *data, int len , unsigned int cmd )
{
	printf( "ondeliver fd %d, cmd %04x, len %d\n", fd, cmd, len ) ;

	CTruckUnPackMgr packer ;
	CPackFactory  factory(&packer) ;
	IPacket *msg = factory.UnPack( data, len ) ;
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, "OnDeliver" , "fd %d, on deliver data length %d, cmd %04x", fd, len , cmd ) ;
		return ;
	}
	CAutoRelease autoGuard( msg ) ;

	switch( msg->_header._type )
	{
	case QUERY_CARDATA_RSP:
		{
			CQueryCarDataRsp *rsp = (CQueryCarDataRsp *) msg ;

			printf( "QUERY_CARDATA_RSP return num %d;", rsp->_num ) ;
			for ( int i = 0; i < rsp->_num; ++ i ) {
				CCarDataInfo *info = rsp->_vec[i] ;
				printf( "car data: %d , data: %s;", i , (char*)info->_sid ) ;
			}
			printf( "\n" ) ;
		}
		break ;
	case QUERY_SCHEDULE_RSP:
		{
			CQueryScheduleRsp *rsp = (CQueryScheduleRsp*) msg;

			printf( "QUERY_SCHEDULE_RSP return num %d;", rsp->_num);

			for ( int i = 0; i < rsp->_num; ++ i ) {
				CScheduleInfo *info = rsp->_vec[i];
				printf( "QUERY_SCHEDULE_RSP schedule: %d, id %s;" , i, (char*)info->_scheduleid ) ;
			}

			printf( "\n" ) ;
		}
		break ;
	case UPLOAD_SCHEDULE_RSP:
		{
			CUploadScheduleRsp *rsp = (CUploadScheduleRsp *) msg ;
			printf( "UPLOAD_SCHEDULE_RSP result %d\n", rsp->_result ) ;
		}
		break ;
	case STATE_SCHEDULE_RSP:
		{
			CStateScheduleRsp *rsp = (CStateScheduleRsp *) msg ;
			printf( "STATE_SCHEDULE_RSP result %d\n", rsp->_result ) ;
		}
		break ;
	case ALARM_SCHEDULE_RSP:
		{
			CAlarmScheduleRsp *rsp = (CAlarmScheduleRsp *) msg ;
			printf( "ALARM_SCHEDULE_RSP result %d\n", rsp->_result ) ;
		}
		break ;
	case RESULT_SCHEDULE_RSP:
		{
			CResultScheduleRsp *rsp = ( CResultScheduleRsp *) msg;
			printf( "RESULT_SCHEDULE_RSP result %d\n", rsp->_result ) ;
		}
		break ;
	}
}
