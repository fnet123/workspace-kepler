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
#include <InterProtoConvert.h>
#include "systemenv.h"
#include "pasclient.h"
#include "msgclient.h"
#include "msgcache.h"
#include "pccserver.h"
#include "userloader.h"
#include "rediscache.h"

CSystemEnv::CSystemEnv():_initialed(false)
{
	_config         = NULL ;
	_rediscache     = new RedisCache ;
	_userloader     = new CUserLoader ;
	_msg_cache      = new MsgCache() ;
	_pas_client     = new PasClient(_srvCaller);
	_msg_client     = new MsgClient(_srvCaller);
	_pcc_server		= new CPccServer;
}

CSystemEnv::~CSystemEnv()
{
	Stop() ;

	if ( _pcc_server != NULL ) {
		delete _pcc_server ;
		_pcc_server = NULL ;
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

	if ( _userloader != NULL ) {
		delete _userloader ;
		_userloader = NULL ;
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

bool CSystemEnv::InitLog( const char * logpath, const char *logname )
{
	char szbuf[512] = {0} ;
	sprintf( szbuf, "mkdir -p %s", logpath ) ;
	system( szbuf );

	sprintf( szbuf, "%s/%s" , logpath, logname ) ;
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

bool CSystemEnv::InitUser( void )
{
	char temp[512] = {0};
	if ( ! GetString( "user_filepath" , temp ) ) {
		printf( "load user file failed\n" ) ;
		return false ;
	}
	char buf[1024] = {0} ;
	getenvpath( temp, buf );

	_user_file = buf ;

	// ���ػ��������б�·��
	if ( ! GetString( "base_dmddir" ,temp ) ) {
		printf( "load base_dmddir failed\n" ) ;
		return false ;
	}
	getenvpath( temp, buf );
	_dmddir = buf;

	return true ;
}

bool CSystemEnv::Init( const char *file , const char *logpath , const char *userfile, const char *logname )
{
	_config = new CCConfig( file ) ;
	if ( _config == NULL ) {
		printf( "CSystemEnv::Init load config file %s failed\n", file ) ;
		return false ;
	}
	// ��ʼ����־
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

	// ��ʼ�û�
	if ( ! InitUser() ) {
		printf( "CSystemEnv::Init pcc user file failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}

	//srvCaller ��ʼ��
	if ( ! _srvCaller.Init( this ) ){
		printf( "CSystemEnv::Init srv caller failed , %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}
	// PCC����
	if ( ! _pcc_server->Init( this ) ) {
		printf( "CSystemEnv::Init pcc server failed, %s:%d\n" , __FILE__ , __LINE__ ) ;
		return false ;
	}

	// MSG�ͻ���
	if ( ! _msg_client->Init( this ) ) {
		printf( "CSystemEnv::Init msg client failed , %s:%d\n", __FILE__ , __LINE__ ) ;
		return false ;
	}

	// PAS�ͻ���
	if ( ! _pas_client->Init( this ) ){
		printf( "CSystemEnv::Init pas client failed , %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}

	//MSG��PAS�������õ���
	if( !_userloader->Init(this) ) {
		printf( "CSystemEnv::Init userloader failed , %s:%d\n", __FILE__, __LINE__ ) ;
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

	if ( ! _srvCaller.Start() ) {
		printf( "CSystemEnv::Start srv caller failed, %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}
	if ( ! _pcc_server->Start() ) {
		printf( "CSystemEnv::Start pcc server failed, %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}

	if ( ! _pas_client->Start() ){
		printf( "CSystemEnv::Start pas client failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}
	if ( ! _msg_client->Start() ){
		printf( "CSystemEnv::Start msg client failed , %s:%d\n", __FILE__, __LINE__ ) ;
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

	_msg_client->Stop() ;
	_pas_client->Stop() ;
	_pcc_server->Stop() ;
	_srvCaller.Stop() ;
	_rediscache->Stop() ;
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

// ȡ�û�������
void CSystemEnv::GetCacheKey( unsigned int seq, char *key )
{
	// MAC����Ϣ�������ֶ�Ӧ���������
	sprintf( key , "%s%u" , SEQ_HEAD, seq ) ;
}

// ��������
bool CSystemEnv::SetNotify( const char *tag, IUserNotify *notify )
{
	return _userloader->SetNotify( tag, notify ) ;
}

// �����û�����
bool CSystemEnv::LoadUserData( void )
{
	return _userloader->LoadUser( _user_file.c_str(), _dmddir.c_str() ) ;
}

// ���ؼ�����Կ
bool CSystemEnv::GetUserKey( int accesscode, int &M1, int &IA1, int &IC1 )
{
	return _userloader->GetUserKey( accesscode, M1, IA1, IC1 ) ;
}

// ����Ự����
void CSystemEnv::ClearSession( const char *key )
{
	_srvCaller.RemoveCache( key ) ;
}

bool CSystemEnv::getChannels(const string &macid, set<string> &channels)
{
	return _userloader->getChannels(macid, channels);
}

bool CSystemEnv::getSubscribe(list<string> &macids)
{
	return _userloader->getSubscribe(macids);
}

bool CSystemEnv::getMacid(const string &plate, string &macid)
{
	return _userloader->getMacid(plate, macid);
}
