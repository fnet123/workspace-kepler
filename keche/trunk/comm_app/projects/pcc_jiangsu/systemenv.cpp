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
#include "systemenv.h"
#include "pasclient.h"
#include "msgclient.h"
#include "msgcache.h"
#include "passerver.h"
#include <tools.h>
#include "statinfo.h"

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         = NULL ;
	_pStatInfo      = new CStatInfo ;
	_pErrLog        = new CCLog ;
	_msg_cache      = new MsgCache() ;
	_pas_client     = new PasClient(_pStatInfo) ;
	_msg_client     = new MsgClient(_pStatInfo) ;
	_pas_server     = new PasServer(_pStatInfo) ;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _pas_server != NULL ) {
		delete _pas_server ;
		_pas_server = NULL ;
	}
	if ( _msg_client != NULL ) {
		delete _msg_client ;
		_msg_client = NULL ;
	}
	if ( _pas_client != NULL ){
		delete _pas_client ;
		_pas_client = NULL ;
	}

	if ( _msg_cache != NULL ) {
		delete _msg_cache ;
		_msg_cache = NULL ;
	}
	if ( _pErrLog != NULL ) {
		delete _pErrLog ;
		_pErrLog = NULL ;
	}
	if ( _pStatInfo != NULL ) {
		delete _pStatInfo ;
		_pStatInfo = NULL ;
	}
	if ( _config != NULL )
	{
		delete _config ;
		_config = NULL ;
	}
}

bool CSystemEnv::InitLog( const char * logpath, const char *logname )
{
	char szbuf[512] = {0} ;
	sprintf( szbuf, "mkdir -p %s", logpath ) ;
	system( szbuf );

	sprintf( szbuf, "%s/%s" , logpath, logname ) ;
	CHGLOG( szbuf ) ;

	sprintf( szbuf, "%s/pcc_jiangsu_error.log", logpath ) ;
	_pErrLog->set_log_file( szbuf ) ;

	int log_num = 20;
	if ( ! GetInteger("log_num" , log_num ) )
	{
		printf( "get log number falied\n" ) ;
		log_num = 20 ;
	}
	_pErrLog->set_log_num( log_num ) ;

	int log_size = 16 ;
	if ( ! GetInteger("log_size" , log_size) )
	{
		printf( "get log size failed\n" ) ;
		log_size = 16 ;
	}
	_pErrLog->set_log_num( log_size ) ;

	// ȡ����־����
	int log_level = 3 ;
	if ( ! GetInteger("log_level" , log_level) ) {
		log_level = 3 ;
	}
	_pErrLog->set_log_level( log_level ) ;

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

	char temp[256] = {0} ;
	// ��������ļ������˹�����־Ŀ¼
	if ( GetString( "log_dir", temp ) ) {
		InitLog( temp, logname ) ;
	} else {
		InitLog( logpath , logname ) ;
	}

	if ( ! _msg_client->Init( this ) ) {
		printf( "CSystemEnv::Init msg client failed , %s:%d\n", __FILE__ , __LINE__ ) ;
		return false ;
	}

	if ( ! _pas_client->Init( this ) ){
		printf( "CSystemEnv::Init pas client failed , %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}

	if ( ! _pas_server->Init(this) ) {
		printf( "CSystemEnv::Init pas server failed" ) ;
		return false ;
	}

	return true ;
}

bool CSystemEnv::Start( void )
{
	if ( ! _pas_client->Start() ){
		printf( "CSystemEnv::Start pas client failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}
	if ( ! _msg_client->Start() ){
		printf( "CSystemEnv::Start msg client failed , %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}
	if ( ! _pas_server->Start() ) {
		printf( "CSystemEnv::Start pas server failed" ) ;
		return false ;
	}
	_initialed = true ;
	return true ;
}

void CSystemEnv::Stop( void )
{
	if ( ! _initialed )
		return ;

	_initialed = false ;

	_pas_server->Stop() ;
	_msg_client->Stop() ;
	_pas_client->Stop() ;
}

// ȡ������ֵ
bool CSystemEnv::GetInteger( const char *key , int &value )
{
	char buf[1024] = {0} ;
	if ( _config->fGetValue("COMMON" , key, buf ) == -1 )
	{
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

void CSystemEnv::GetCacheKey( const char *macid , unsigned short data_type , char *buf )
{
	// ������Ӧ��������������,0x8000
	if ( ! ( data_type & 0x8000 ) ) {
		data_type |= 0x8000 ;
	}
	// MAC����Ϣ�������ֶ�Ӧ���������
	sprintf( buf, "%s_%d" , macid, data_type ) ;
}
