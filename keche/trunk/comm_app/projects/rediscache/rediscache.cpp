/*
 * rediscache.cpp
 *
 *  Created on: 2012-4-18
 *      Author: humingqing
 */

#include "rediscache.h"
#include "redispool.h"

RedisCache::RedisCache()
{
	_pool = new RedisPool;
}

RedisCache::~RedisCache()
{
	if ( _pool != NULL ) {
		delete _pool ;
		_pool = NULL ;
	}
}

// ��ʼ���������
bool RedisCache::Init( IContext *pEnv )
{
	return _pool->Init( pEnv ) ;
}

// ��ʼ���������
bool RedisCache::Start( void )
{
	return _pool->Start() ;
}

// ֹͣ���������
bool RedisCache::Stop( void )
{
	return _pool->Stop() ;
}

// ȡ��RedisObj��������
IRedisObj *RedisCache::GetObj( void )
{
	return _pool->CheckOut() ;
}

// �Ż�ȡ��RedisObj����
void RedisCache::PutObj( IRedisObj *obj )
{
	_pool->CheckIn( (RedisObj*)obj ) ;
}

// �ӻ��������ȡ�ýӶ���ֵ
bool RedisCache::GetValue( const char *key , std::string &val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->GetValue( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ���û��������ָ����ֵ
bool RedisCache::SetValue( const char *key, const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->SetValue( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// �Ƴ���������е�ֵ
bool RedisCache::Remove( const char *key )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->Remove( key ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ȡ�û�������������
int RedisCache::GetList( const char *key, std::vector<std::string> &vec )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	int count = obj->GetList( key, vec ) ;
	_pool->CheckIn( obj ) ;

	return count;
}

// ���û����������ֵ
int RedisCache::SetList( const char *key, std::vector<std::string> &vec )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return -1;
	}
	int count = obj->SetList( key, vec ) ;
	_pool->CheckIn( obj ) ;

	return count ;
}

// �Ӷ����е���ֵ
bool RedisCache::PopValue( const char *key , std::string &val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->PopValue( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// �����ݷŵ�����β��
bool RedisCache::PushValue( const char *key, const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->PushValue( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ģ��ȡ��KEY��ֵ
int RedisCache::GetKeys( const char *key, std::vector<std::string> &vec )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL )
		return -1 ;

	int count = obj->GetKeys( key, vec ) ;
	_pool->CheckIn( obj ) ;

	return count ;
}

// ɾ��������ĳ��Ԫ��
bool RedisCache::LRem( const char *key , const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->LRem( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ʹ��Hash SET������������
bool RedisCache::HSet( const char *areaid, const char *key, const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->HSet( areaid, key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ȡ��Hash SET��������
bool RedisCache::HGet( const char *areaid, const char *key, std::string &val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->HGet( areaid, key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ȡ��Hash SET�е����м���
int RedisCache::HKeys( const char *areaid,  std::vector<std::string> &vec )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL )
		return -1 ;

	int count = obj->HKeys( areaid, vec ) ;
	_pool->CheckIn( obj ) ;

	return count ;
}

// ɾ��HASH�е�ĳֵ
bool RedisCache::HDel( const char *areaid, const char *key )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->HDel( areaid, key ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ����Set���е�Ԫ��
bool RedisCache::SAdd( const char *key, const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->SAdd( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ɾ��SET���е�ĳ��Ԫ��
bool RedisCache::SRem( const char *key , const char *val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->SRem( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ����SET����ĳ��Ԫ��
bool RedisCache::SPop( const char *key, std::string &val )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL ) {
		return false ;
	}
	bool find = obj->SPop( key, val ) ;
	_pool->CheckIn( obj ) ;

	return find ;
}

// ȡ�õ�ǰֵSET�������г�Ա
int RedisCache::SMembers( const char *key, std::vector<std::string> &vec )
{
	RedisObj *obj = _pool->CheckOut() ;
	if ( obj == NULL )
		return -1 ;

	int count = obj->SMembers( key, vec ) ;
	_pool->CheckIn( obj ) ;

	return count ;
}

