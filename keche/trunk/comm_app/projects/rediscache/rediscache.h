/*
 * rediscache.h
 *
 *  Created on: 2012-4-18
 *      Author: humingqing
 */

#ifndef __REDISCACHE_H__
#define __REDISCACHE_H__

#include <icontext.h>
#include <icache.h>
#include <string>
#include <vector>

class RedisPool;
class RedisCache :
	public IRedisCache
{
public:
	RedisCache() ;
	~RedisCache() ;
	// ��ʼ���������
	bool Init( IContext *pEnv ) ;
	// ��ʼ���������
	bool Start( void ) ;
	// ֹͣ���������
	bool Stop( void ) ;
	// ȡ��RedisObj��������
	IRedisObj *GetObj( void ) ;
	// �Ż�ȡ��RedisObj����
	void PutObj( IRedisObj *obj ) ;

	//=============== ��Բ���Ƶ�ʲ��ߵĻ������  ========================
	// �ӻ��������ȡ�ýӶ���ֵ
	bool GetValue( const char *key , std::string &val ) ;
	// ���û��������ָ����ֵ
	bool SetValue( const char *key, const char *val ) ;
	// �Ƴ���������е�ֵ
	bool Remove( const char *key ) ;
	// ȡ�û�������������
	int GetList( const char *key, std::vector<std::string> &vec ) ;
	// ���û����������ֵ
	int SetList( const char *key, std::vector<std::string> &vec ) ;
	// �Ӷ����е���ֵ
	bool PopValue( const char *key , std::string &val ) ;
	// �����ݷŵ�����β��
	bool PushValue( const char *key, const char *val ) ;
	// ģ��ȡ��KEY��ֵ
	int  GetKeys( const char *key, std::vector<std::string> &vec ) ;
	// ɾ��������ĳ��Ԫ��
	bool LRem( const char *key , const char *val ) ;
	// ʹ��Hash SET������������
	bool HSet( const char *areaid, const char *key, const char *val ) ;
	// ȡ��Hash SET��������
	bool HGet( const char *areaid, const char *key, std::string &val ) ;
	// ȡ��Hash SET�е����м���
	int  HKeys( const char *areaid,  std::vector<std::string> &vec ) ;
	// ɾ��HASH�е�ĳֵ
	bool HDel( const char *areaid, const char *key ) ;
	// ����Set���е�Ԫ��
	bool SAdd( const char *key, const char *val ) ;
	// ɾ��SET���е�ĳ��Ԫ��
	bool SRem( const char *key , const char *val ) ;
	// ����SET����ĳ��Ԫ��
	bool SPop( const char *key, std::string &val ) ;
	// ȡ�õ�ǰֵSET�������г�Ա
	int  SMembers( const char *key, std::vector<std::string> &vec ) ;

private:
	// ��������
	RedisPool  *_pool ;
};


#endif /* REDISCACHE_H_ */
