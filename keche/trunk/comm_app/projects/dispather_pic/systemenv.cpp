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
#include "msgclient.h"
#include <tools.h>

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config       = NULL ;
	_msgclient     = new MsgClient;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _msgclient != NULL ) {
		delete _msgclient ;
		_msgclient = NULL ;
	}

	if ( _config != NULL )
	{
		delete _config ;
		_config = NULL ;
	}
}

bool CSystemEnv::InitLog( const char * logpath , const char *logname )
{
	char szbuf[512] = {0} ;
	sprintf( szbuf, "mkdir -p %s", logpath ) ;
	system( szbuf );

	sprintf( szbuf, "%s/%s" , logpath , logname ) ;
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

bool CSystemEnv::Init( const char *file , const char *logpath  , const char *logname )
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

	if ( ! _msgclient->Init( this ) ) {
		printf( "CSystemEnv::Init msg client failed , %s:%d\n", __FILE__ , __LINE__ ) ;
		return false ;
	}

	return true ;
}

bool CSystemEnv::Start( void )
{
	if ( ! _msgclient->Start() ){
		printf( "CSystemEnv::Start msg client failed, %s:%d\n" , __FILE__, __LINE__ ) ;
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

	_msgclient->Stop() ;
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
