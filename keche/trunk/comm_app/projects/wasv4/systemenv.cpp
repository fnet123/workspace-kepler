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
#include "wasserver.h"
#include "rediscache.h"
#include <tools.h>

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         	= NULL ;
	_msg_client         = NULL ;
	_was_server     	= NULL ;
	_msg_client     	= new MsgClient(_cache_pool ) ;
	_was_server     	= new ClientAccessServer(_cache_pool ) ;
	_rediscache         = new RedisCache ;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _msg_client != NULL ){
		delete _msg_client ;
		_msg_client = NULL ;
	}
	if ( _was_server != NULL ){
		delete _was_server ;
		_was_server = NULL ;
	}
	if ( _rediscache != NULL ) {
		delete _rediscache ;
		_rediscache = NULL ;
	}
	if ( _config != NULL ){
		delete _config ;
		_config = NULL ;
	}
}

bool CSystemEnv::InitLog(const char * logpath , const char *logname)
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

	int log_size = 16;
	if ( ! GetInteger("log_size" , log_size) ){
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
	CHGLOGSIZE(log_size) ;
	CHGLOGNUM(log_num) ;

	return true ;
}

bool CSystemEnv::Init(const char *file, const char *logpath,const char *userfile, const char *logname)

{
	_user_file_path = userfile;

	_config = new CCConfig(file);

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

	// ��ʼ������
	if ( ! _rediscache->Init(this) ) {
		printf( "CSystemEnv::Init redis cache failed\n" ) ;
		return false ;
	}

	// ��ʼ��CAS������
	if ( ! _was_server->Init( this ) )
	{
		printf( "CSystemEnv::Init was server init failed\n" ) ;
		return false ;
	}
	// ��ʼ��MSG�ͻ���
	if ( ! _msg_client->Init( this ) )
	{
		printf( "CSystemEnv::Init msg client init failed\n" ) ;
		return false ;
	}

	return true ;
}

bool CSystemEnv::Start( void )
{
	// �����������
	if( ! _rediscache->Start() ) {
		return false;
	}

	// ����ǰ�û�����
	if ( ! _was_server->Start() ){
		printf( "CSystemEnv::Start start was server failed\n" ) ;
		return false ;
	}

	// ����MSG��ģ��
	if ( ! _msg_client->Start() ) {
		printf( "CSystemEnv::Start start msg client failed\n" ) ;
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

	_was_server->Stop() ;
	_msg_client->Stop() ;
	_rediscache->Stop();
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
