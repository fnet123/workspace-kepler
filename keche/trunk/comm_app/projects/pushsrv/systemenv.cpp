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
#include "msgclient.h"
#include "nodeclient.h"
#include "rediscache.h"
#include "pushserver.h"

CSystemEnv::CSystemEnv():_initialed(false)
{
	_rediscache     = new RedisCache ;
	_pushserver     = new PushServer ;
	_msgclient      = new MsgClient ;
	_node_client	= new CNodeClient;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;
	if ( _node_client != NULL ){
		delete _node_client ;
		_node_client = NULL ;
	}
	if ( _msgclient != NULL ) {
		delete _msgclient ;
		_msgclient = NULL ;
	}
	if ( _pushserver != NULL  ){
		delete _pushserver ;
		_pushserver = NULL ;
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

bool CSystemEnv::Init( const char *file , const char *logpath , const char *userfile , const char *logname )
{
	_config = new CCConfig( file ) ;

	_user_file_path = userfile;

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

	if ( ! _rediscache->Init(this) ) {
		printf( "CSystemEnv::Init redis cache failed\n" ) ;
		return false ;
	}
	// ��ʼ�����ͷ���
	if ( ! _pushserver->Init(this) ) {
		printf( "CSystemEnv::Init push server failed\n" ) ;
		return false ;
	}
	// ��ʼ��MSG�Ķ���
	if ( ! _msgclient->Init( this ) ) {
		printf( "CSystemEnv::Init msg client failed\n" ) ;
		return false ;
	}
	// ��ʼ����������
	if ( ! _node_client->Init(this) ) {
		printf( "CSystemEnv::Init node client failed\n" ) ;
		return false ;
	}
	return true ;
}

bool CSystemEnv::Start( void )
{
	if ( ! _rediscache->Start() ) {
		printf( "CSystemEnv::Start start redis cache failed\n" ) ;
		return false ;
	}
	_initialed = true ;

	// ��ʼ�����ͷ���
	if ( ! _pushserver->Start() ) {
		printf( "CSystemEnv::start push server failed\n" ) ;
		return false ;
	}
	// ����MSGClient����
	if ( ! _msgclient->Start() ) {
		printf( "CSystemEnv::Start msg client failed\n" ) ;
		return false ;
	}

	return _node_client->Start() ;
}

void CSystemEnv::Stop( void )
{
	if ( ! _initialed )
		return ;

	_initialed = false ;

	_node_client->Stop() ;
	_msgclient->Stop() ;
	_pushserver->Stop() ;
	_rediscache->Stop() ;
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
