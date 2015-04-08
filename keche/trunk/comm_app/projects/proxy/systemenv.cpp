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
#include "passerver.h"
#include "transmit.h"
#include <tools.h>

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         = NULL ;
	_pas_server     = new PasServer;
	_mas_server	    = new Transmit ;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _pas_server != NULL ){
		delete _pas_server ;
		_pas_server = NULL ;
	}

	if ( _mas_server != NULL ) {
		delete _mas_server ;
		_mas_server = NULL ;
	}

	if ( _config != NULL )
	{
		delete _config ;
		_config = NULL ;
	}
}

bool CSystemEnv::InitLog( const char * logpath )
{
	char szbuf[512] = {0} ;
	sprintf( szbuf, "mkdir -p %s", logpath ) ;
	system( szbuf );

	sprintf( szbuf, "%s/proxy.log" , logpath ) ;
	CHGLOG( szbuf ) ;

	int log_num = 20;
	if ( ! GetInteger("log_num" , log_num ) )
	{
		printf( "get log number falied\n" ) ;
		log_num = 20 ;
	}

	int log_size = 16 ;
	if ( ! GetInteger("log_size" , log_size) )
	{
		printf( "get log size failed\n" ) ;
		log_size = 16 ;
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

bool CSystemEnv::Init( const char *file , const char *logpath , const char *runpath )
{
	_runpath = runpath ;

	_config = new CCConfig( file ) ;
	if ( _config == NULL ) {
		printf( "CSystemEnv::Init load config file %s failed\n", file ) ;
		return false ;
	}

	InitLog( logpath ) ;

	// ��ʼ������ϵͳ
	if ( ! _mas_server->Init(this) ) {
		printf( "start monitor system failed\n" ) ;
		return false ;
	}

	if ( ! _pas_server->Init( this ) ){
		printf( "CSystemEnv::Init pas server failed , %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}

	return true ;
}

bool CSystemEnv::Start( void )
{
	// �������ֻ�����ͨ������
	if ( ! _mas_server->Start() ) {
		printf( "CSystemEnv::Start monitor server failed\n" ) ;
		return false ;
	}

	if ( ! _pas_server->Start() ){
		printf( "CSystemEnv::Start pas server failed, %s:%d\n" , __FILE__, __LINE__ ) ;
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
	_mas_server->Stop() ;
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
