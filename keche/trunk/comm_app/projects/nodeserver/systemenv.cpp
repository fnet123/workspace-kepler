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
#include "nodesrv.h"
#include "waitgroup.h"
#include <tools.h>

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         = NULL ;
	_msg_server     = NULL ;
	_allocmsg		= new CAllocMsg ;
	_waitgroup	    = new CWaitGroup(_allocmsg) ;
	_msg_server     = new CNodeSrv ;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _msg_server != NULL ){
		delete _msg_server ;
		_msg_server = NULL ;
	}
	if ( _waitgroup != NULL ) {
		delete _waitgroup ;
		_waitgroup = NULL ;
	}
	if ( _config != NULL ){
		delete _config ;
		_config = NULL ;
	}
	if ( _allocmsg != NULL ) {
		delete _allocmsg ;
		_allocmsg = NULL ;
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

bool CSystemEnv::Init( const char *file , const char *logpath , const char *userfile , const char *logname )
{
	_user_file_path = userfile ;

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

	// ��ʼ���ȴ������߳�
	if ( ! _waitgroup->Init() ) {
		printf( "CSystemEnv::Init wait group failed\n" ) ;
		return false ;
	}

	// ��ʼ��MSG������
	if ( ! _msg_server->Init( this ) )
	{
		printf( "CSystemEnv::Init cas server init failed\n" ) ;
		return false ;
	}

	return true ;
}

bool CSystemEnv::Start( void )
{
	if ( ! _waitgroup->Start() ) {
		printf( "CSystemEnv::Start wait group thread failed\n" ) ;
		return false ;
	}
	if ( ! _msg_server->Start() ){
		printf( "CSystemEnv::Start start cas server failed\n" ) ;
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
	_msg_server->Stop() ;
	_waitgroup->Stop() ;
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
