/*
 * redispool.cpp
 *
 *  Created on: 2012-4-18
 *      Author: think
 */

#include <hiredis.h>
#include "redispool.h"
#include <assert.h>
#include <comlog.h>

#define FREE_REDIS(p)  if ( p != NULL ) { redisFree( p) ; p = NULL ; }

RedisObj::RedisObj()
{
	_ctmaster  = NULL ;
	_ctslaver  = NULL ;
	_lastcheck = time(NULL) ;
	_password  = "";
	_number   = 0;
}

RedisObj::~RedisObj()
{
	FreeObj() ;
}

void RedisObj::auth(const char *password)
{
	if(password == NULL) {
		return;
	}
	_password = password;
}

void RedisObj::select(int number)
{
	if(number < 0) {
		return;
	}
	_number = number;
}

bool RedisObj::stauts()
{
	if(_ctmaster != NULL && _ctmaster->err != 0) {
		return false;
	}

	if(_ctslaver != NULL && _ctslaver->err != 0) {
		return false;
	}

	return true;
}

// ��ʼ������
bool RedisObj::InitObj( const char *masterip, unsigned short masterport, const char *slaverip, unsigned short slaverport )
{
	if ( masterip == NULL || masterport == 0 ) {
		return false ;
	}

	int connsuccess = 0 ;
	// ������������
	if((_ctmaster = Connect((char*)masterip, masterport)) == NULL) {
		return false;
	}

	++connsuccess;

	// �Ӷ�������
	if ( slaverip != NULL && slaverport > 0 ) {
		if((_ctslaver = Connect( (char*)slaverip, slaverport )) == NULL) {
			redisFree(_ctmaster);
			_ctmaster = NULL;
			return false;
		}

		++connsuccess;
	}
	OUT_PRINT( NULL, 0, "Redis", "master: [%s:%d] fd %d , slaver: [%s:%d] fd %d, connect success count %d",
			masterip, masterport, ( ( _ctmaster) ? _ctmaster->fd : -1 ) , slaverip, slaverport,
			( ( _ctslaver ) ? _ctslaver->fd : -1 ) , connsuccess ) ;
	// �Ƿ��п�������
	return ( connsuccess > 0 ) ;
}

// ȡ�����ݶ���
bool RedisObj::GetValue( const char *key , std::string &val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "GET %s", key ) ;

	return GetString( scmd, val ) ;
}

// ���û��������ָ����ֵ
bool RedisObj::SetValue( const char *key, const char *val )
{
	string scmd = "SET " ;
	scmd += key ;
	scmd += " " ;
	scmd += val ;

	return Execute( scmd.c_str() ) ;
}

// �Ƴ���������е�ֵ
bool RedisObj::Remove( const char *key )
{
	char scmd[1024] = {0};
	sprintf( scmd, "DEL %s", key ) ;
	return Execute( scmd ) ;
}

// ȡ�û�������������
int RedisObj::GetList( const char *key, VecString &vec )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "LRANGE %s 0 -1", key ) ;

	int count = GetArray( scmd, vec ) ;
	// ��ӡȡ������ֵ
	// OUT_PRINT( NULL, 0, "Redis", "get key %s count %d", key, count ) ;
	return count ;
}

// ���û����������ֵ
int RedisObj::SetList( const char *key, VecString &vec )
{
	std::string val ;

	int len = vec.size() ;
	for ( int i = 0; i < len; ++ i ) {
		if ( ! val.empty() ) {
			val += " " ;
		}
		val += vec[i] ;
	}
	if ( val.empty() ) return -1 ;

	std::string scmd = "LPUSH " ;
	scmd += key ;
	scmd += " " + val ;

	if ( ! Execute( scmd.c_str() ) )
		return -1 ;

	return len;
}

// �Ӷ����е���ֵ
bool RedisObj::PopValue( const char *key , std::string &val )
{
	char scmd[1024] = {0};
	sprintf( scmd, "LPOP %s", key ) ;
	return GetString( scmd, val ) ;
}

// �����ݷŵ�����β��
bool RedisObj::PushValue( const char *key, const char *val )
{
	string scmd = "LPUSH " ;
	scmd += key ;
	scmd += " " ;
	scmd += val ;
	return Execute( scmd.c_str() ) ;
}

// ģ��ȡ������KEY��ֵ
int RedisObj::GetKeys( const char *key, VecString &vec )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "KEYS %s", key ) ;

	int count = GetArray( scmd, vec ) ;
	// ��ӡȡ������ֵ
	// OUT_PRINT( NULL, 0, "Redis", "get keys %s count %d", key, count ) ;

	return count ;
}

// ɾ��������ĳ��Ԫ��
bool RedisObj::LRem( const char *key , const char *val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "LREM %s 0 %s", key , val ) ;

	return Execute( scmd ) ;
}

// ʹ��Hash SET������������
bool RedisObj::HSet( const char *areaid, const char *key, const char *val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "HSET %s %s %s", areaid, key, val ) ;

	return Execute( scmd ) ;
}

// ȡ��Hash SET��������
bool RedisObj::HGet( const char *areaid, const char *key, std::string &val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "HGET %s %s", areaid, key ) ;

	return GetString( scmd, val ) ;
}

// ȡ��Hash SET�е����м���
int  RedisObj::HKeys( const char *areaid,  std::vector<std::string> &vec )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "HKEYS %s", areaid ) ;

	return GetArray( scmd, vec ) ;
}

// ɾ��HASH�е�ĳֵ
bool RedisObj::HDel( const char *areaid, const char *key )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "HDEL %s %s", areaid, key ) ;

	return Execute( scmd ) ;
}

// ����Set���е�Ԫ��
bool RedisObj::SAdd( const char *key, const char *val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "SADD %s %s", key , val ) ;

	return Execute( scmd ) ;
}

// ɾ��SET���е�ĳ��Ԫ��
bool RedisObj::SRem( const char *key , const char *val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "SREM %s %s", key, val ) ;

	return Execute( scmd ) ;
}

// ����SET����ĳ��Ԫ��
bool RedisObj::SPop( const char *key, std::string &val )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "SPOP %s", key ) ;

	return GetString( scmd, val ) ;
}

// ȡ�õ�ǰֵSET�������г�Ա
int  RedisObj::SMembers( const char *key, std::vector<std::string> &vec )
{
	char scmd[1024] = {0} ;
	sprintf( scmd, "SMEMBERS %s", key ) ;

	return GetArray( scmd, vec ) ;
}

// �ͷ����ж���
void RedisObj::FreeObj( void )
{
	OUT_PRINT( NULL, 0, "Redis",  "Free master %d and slaver %d \n",
			( ( _ctmaster) ? _ctmaster->fd : -1 ) , ( ( _ctslaver ) ? _ctslaver->fd : -1 ) ) ;
	// ���������
	FREE_REDIS( _ctmaster ) ;
	// �ӻ������
	FREE_REDIS( _ctslaver ) ;
}

// ���������·
bool RedisObj::Ping( void )
{
	if ( _ctmaster == NULL && _ctslaver == NULL )
		return false ;

	time_t now = time(NULL) ;
	// ����Ƿ���30���ʱ����
	if ( now - _lastcheck < 60 ) {
		return true ;
	}
	_lastcheck = now ;

	string rspstr;

	// �����������
	if ( _ctmaster ) {
		/* Set a key */
		redisReply *reply = (redisReply*)redisCommand( _ctmaster ,"PING" );
		if ( reply == NULL ) {
			OUT_ERROR( NULL, 0, "Redis" , "Ping master fd %d failed, error[%d]: %s", _ctmaster->fd , _ctmaster->err, _ctmaster->errstr ) ;
			return false ;
		}

		rspstr = "";
		if(reply->str != NULL && reply->len > 0) {
			rspstr.assign(reply->str, reply->len);
		}
		freeReplyObject(reply);

		if(rspstr != "PONG") {
			OUT_ERROR( NULL, 0, "Redis",  "ping reply error: %s\n", rspstr.c_str() ) ;
			return false;
		}
	}

	// ���ӷ�����
	if ( _ctslaver ) {
		/* Set a key */
		redisReply *reply = (redisReply*)redisCommand( _ctslaver ,"PING" );
		if ( reply == NULL ) {
			OUT_ERROR( NULL, 0, "Redis" , "Ping slaver fd %d failed, error[%d]: %s", _ctslaver->fd, _ctslaver->err, _ctslaver->errstr ) ;
			return false ;
		}

		rspstr = "";
		if(reply->str != NULL && reply->len > 0) {
			rspstr.assign(reply->str, reply->len);
		}
		freeReplyObject(reply);

		if(rspstr != "PONG") {
			OUT_ERROR( NULL, 0, "Redis",  "ping reply error: %s\n", rspstr.c_str() ) ;
			return false;
		}
	}

	return true ;
}

// ���ӷ�����
redisContext * RedisObj::Connect( const char *ip, int port )
{
	redisReply *reply;
	char cmd[1024];

	string rspstr;

	struct timeval tv = {1, 0};
	struct timeval timeout = { 0, 100000 }; // 0.1 seconds
	redisContext *c = redisConnectWithTimeout((char*)ip, port , timeout);
	if ( c->err ) {
		OUT_ERROR(ip, port, "Redis",  "Connection error: %s\n", c->errstr ) ;
		FREE_REDIS( c ) ;
		return NULL ;
	}

	redisSetTimeout(c,tv);

	if ( ! _password.empty()) {
		snprintf(cmd, 1024, "auth %s", _password.c_str());
		if ((reply = (redisReply*) redisCommand(c, cmd)) == NULL) {
			OUT_ERROR( ip, port, "Redis",  "query reply error: %s\n", c->errstr ) ;
			redisFree(c);
			return NULL;
		}

		rspstr = "";
		if(reply->str != NULL && reply->len > 0) {
			rspstr.assign(reply->str, reply->len);
		}
		freeReplyObject(reply);

		if(rspstr != "OK") {
			OUT_ERROR( ip, port, "Redis",  "query reply error: %s\n", rspstr.c_str() ) ;
			redisFree(c);
			return NULL;
		}
	}

	if (_number > 0) {
		snprintf(cmd, 1024, "select %d", _number);
		if ((reply = (redisReply*) redisCommand(c, cmd)) == NULL) {
			OUT_ERROR( ip, port, "Redis",  "query reply error: %s\n", c->errstr);
			redisFree(c);
			return NULL;
		}

		rspstr = "";
		if (reply->str != NULL && reply->len > 0) {
			rspstr.assign(reply->str, reply->len);
		}
		freeReplyObject(reply);

		if(rspstr != "OK") {
			OUT_ERROR( ip, port, "Redis",  "query reply error: %s\n", rspstr.c_str()) ;
			redisFree(c);
			return NULL;
		}
	}

	return c ;
}

// ȡ���������͵�ֵ
int  RedisObj::GetArray( const char *cmd, VecString &vec )
{
	redisContext *c   = ( _ctslaver ) ? _ctslaver : _ctmaster ;
	/* Get a key */
	redisReply *reply = (redisReply*)redisCommand( c, cmd ) ;
	if ( reply == NULL ){
		if ( c == _ctslaver ) {
			reply = (redisReply*)redisCommand( _ctmaster, cmd );
		}
		if ( reply == NULL ) {
			OUT_ERROR( NULL, 0, "Redis", "get key %s error[%d]:%s" , cmd , c->err, c->errstr ) ;
			return -1 ;
		}
	}

	int count = 0 ;
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (int j = 0; j < reply->elements; j++) {
			if ( reply->element[j]->str == NULL )
				continue ;
			vec.push_back(reply->element[j]->str);
		}
		count = reply->elements ;
	}
	freeReplyObject(reply);

	return count ;
}

// ȡ���ַ����͵�ֵ
bool RedisObj::GetString( const char *cmd, std::string &val )
{
	redisContext *c   = ( _ctslaver ) ? _ctslaver : _ctmaster ;
	/* Get a key */
	redisReply *reply = (redisReply*)redisCommand(c, cmd );
	if ( reply == NULL ){
		if ( c == _ctslaver ) {
			reply = (redisReply*)redisCommand( _ctmaster, cmd );
		}
		if ( reply == NULL ) {
			OUT_ERROR( NULL, 0, "Redis", "get key %s error[%d]:%s" , cmd , c->err, c->errstr ) ;
			return false ;
		}
	}
	if ( reply->str == NULL ){
		freeReplyObject(reply);
		return false ;
	}
	val = reply->str ;
	freeReplyObject(reply);

	return true ;
}

// ִ���������͵�ֵ
bool RedisObj::Execute( const char *cmd )
{
	redisContext *c = ( _ctmaster != NULL ) ? _ctmaster : _ctslaver ;
	/* Set a key */
	redisReply *reply = (redisReply*)redisCommand( c, cmd );
	if ( reply == NULL )
		return false ;

	//printf("SET: %s\n", reply->str);
	freeReplyObject(reply);

	return true ;
}

////////////////////////////////////////// RedisPool�صĹ��� ///////////////////////////////////////////////

RedisPool::RedisPool():_inited(false)
{
	_index = 0 ;
	// �����Ķ˿�
	_masterport = 0 ;
	// �ӻ���Ķ˿�
	_slaverport = 0 ;

	_password = "";
	_number = 0;
}

RedisPool::~RedisPool()
{
	Stop() ;
	Clear() ;
}

// ��ʼ����������
bool RedisPool::Init( IContext *pEnv )
{
	char buf[1024] = {0};
	if ( ! pEnv->GetString( "rediscluster" , buf ) ) {
		OUT_ERROR( NULL, 0, "RedisPool", "get redis cache master ip port failed" ) ;
		return false ;
	}
	// ������ַʧ��
	if ( ! PaserAddress(buf) ) {
		OUT_ERROR( NULL, 0, "RedisPool", "paser address %s failed", buf ) ;
		return false ;
	}

	if ( pEnv->GetString( "redispassword" , buf ) ) {
		_password = buf;
	}

	int value;
	if ( pEnv->GetInteger( "redisnumber" , value ) ) {
		_number = value;
	}

	// �����ʼ���߳�ʧ��
	if (! _threadpool.init(1, this, this) ) {
		OUT_ERROR( NULL, 0, "RedisPool", "init thead pool failed" ) ;
		return false ;
	}
	_inited = true ;
	return true ;
}

// ��ʼ�߳�
bool RedisPool::Start( void )
{
	_threadpool.start() ;
	return true ;
}

// ֹͣ����
bool RedisPool::Stop( void )
{
	if ( !_inited )
		return false;

	_inited = false ;
	_threadpool.stop() ;
	return true ;
}

// �����ӳ���ǩ������
RedisObj * RedisPool::CheckOut( void )
{
	RedisObj *p = NULL ;

	// ͬ��������������ʱ���BUG
	_mutex.lock() ;
	if ( _objpool[_index].size() > 0 ) {
		p = _objpool[_index].begin() ;
		_objpool[_index].erase( p ) ;
		_mutex.unlock() ;
		return p ;
	}
	_mutex.unlock() ;

	p = new RedisObj;
	p->auth(_password.c_str());
	p->select(_number);
	if ( ! p->InitObj( _masterip.c_str(), _masterport, _slaverip.c_str(), _slaverport ) ) {
		delete p ;
		OUT_ERROR( NULL, 0, "RedisPool", "init obj master: %s:%d, slaver: %s:%d failed",
				_masterip.c_str(), _masterport, _slaverip.c_str(), _slaverport ) ;
		return NULL ;
	}
	return p ;
}

// �������ӳض���
void RedisPool::CheckIn(RedisObj *obj)
{
	if( ! obj->stauts()) { //�������ʧЧֱ��ɾ��
		delete obj;
		return;
	}

	// �������ݶ���������߳����й�ϵ
	_mutex.lock() ;
	_objpool[_index].push( obj ) ;
	_mutex.unlock() ;
}

// �߳�ִ�ж���
void RedisPool::run( void *param )
{
	// ������Ҫ�������Ƿ�ʱ
	while( _inited ) {
		// ��ǰʹ������
		int curindex = _index ;

		// �л�������
		_mutex.lock() ;
		_index = ( _index + 1 ) % BACK_POOL_SIZE ;
		_mutex.unlock() ;

		if ( _objpool[curindex].size() == 0 ) {
			sleep(10) ;
			continue ;
		}

		// time_t now = time(NULL) ;

		RedisObj *t = NULL ;
		RedisObj *p = _objpool[curindex].begin() ;
		while( p != NULL ) {
			t = p ;
			p = p->_next ;

			// ���PING������оͼ���
			if ( t->Ping() ) {
				continue ;
			}
			delete _objpool[curindex].erase(t) ;
		}

		sleep(10) ;
	}
}


// ������ַ�Ƿ�����
bool RedisPool::PaserAddress( char *val )
{
	// 127.0.0.1:6379,127.0.0.1:6380
	char *ptr = NULL ;
	char *p = (char*) strstr( val, "," ) ;
	if ( p != NULL ) {
		*p  = 0 ;
		ptr = p + 1 ;
	}

	// ����IP�Ͷ˿�
	p = strstr( val, ":" ) ;
	if ( p == NULL )
		return false ;
	// ���ֻ����������
	_masterip.assign( val, p-val ) ;
	_masterport = atoi( p+1 ) ;

	// ����ӷ�����������
	if ( ptr != NULL ) {
		p = strstr( ptr, ":" ) ;
		if ( p != NULL ) {
			_slaverip.assign( ptr, p - ptr ) ;
			_slaverport = atoi( p+1 ) ;
		}
	}
	return true ;
}

// ������������
void RedisPool::Clear( void )
{
	share::Guard guard( _mutex ) ;

	for ( int i = 0; i < BACK_POOL_SIZE; ++ i ) {

		int size = 0 ;
		RedisObj *p = _objpool[i].move(size) ;
		if ( size == 0 )
			continue ;

		while( p != NULL ) {
			p = p->_next ;
			delete p->_pre ;
		}
	}
}
