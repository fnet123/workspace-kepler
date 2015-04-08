/*
 * icache.h
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  ���洦��
 */

#ifndef __ICACHE_H_
#define __ICACHE_H_

#include <vector>
#include <string>

class IContext ;

// ����ԽӴ���ӿ�
class IRedisObj
{
public:
	virtual ~IRedisObj() {}
	// �ӻ��������ȡ�ýӶ���ֵ
	virtual bool GetValue( const char *key , std::string &val ) = 0 ;
	// ���û��������ָ����ֵ
	virtual bool SetValue( const char *key, const char *val ) = 0 ;
	// �Ƴ���������е�ֵ
	virtual bool Remove( const char *key ) = 0 ;
	// ȡ�û�������������
	virtual int  GetList( const char *key, std::vector<std::string> &vec ) = 0 ;
	// ���û����������ֵ
	virtual int  SetList( const char *key, std::vector<std::string> &vec ) = 0 ;
	// �Ӷ����е���ֵ
	virtual bool PopValue( const char *key , std::string &val ) = 0 ;
	// �����ݷŵ�����β��
	virtual bool PushValue( const char *key, const char *val ) = 0 ;
	// ģ��ȡ��KEY��ֵ
	virtual int  GetKeys( const char *key, std::vector<std::string> &vec ) = 0 ;
	// ɾ��������ĳ��Ԫ��
	virtual bool LRem( const char *key , const char *val ) = 0 ;
	// ʹ��Hash SET������������
	virtual bool HSet( const char *areaid, const char *key, const char *val ) =  0 ;
	// ȡ��Hash SET��������
	virtual bool HGet( const char *areaid, const char *key, std::string &val ) = 0 ;
	// ȡ��Hash SET�е����м���
	virtual int  HKeys( const char *areaid,  std::vector<std::string> &vec ) = 0 ;
	// ɾ��HASH�е�ĳֵ
	virtual bool HDel( const char *areaid, const char *key ) = 0 ;
	// ����Set���е�Ԫ��
	virtual bool SAdd( const char *key, const char *val ) = 0 ;
	// ɾ��SET���е�ĳ��Ԫ��
	virtual bool SRem( const char *key , const char *val ) =  0 ;
	// ����SET����ĳ��Ԫ��
	virtual bool SPop( const char *key, std::string &val ) = 0 ;
	// ȡ�õ�ǰֵSET�������г�Ա
	virtual int  SMembers( const char *key, std::vector<std::string> &vec ) = 0 ;
};

// �������ӿڶ���
class IRedisCache : public IRedisObj
{
public:
	virtual ~IRedisCache() {} ;
	// ��ʼ���������
	virtual bool Init( IContext *pEnv ) = 0 ;
	// ��ʼ���������
	virtual bool Start( void ) = 0 ;
	// ֹͣ���������
	virtual bool Stop( void ) = 0 ;
	// ȡ��RedisObj��������
	virtual IRedisObj *GetObj( void ) =  0 ;
	// �Ż�ȡ��RedisObj����
	virtual void PutObj( IRedisObj *obj ) = 0 ;
};

#endif /* ICACHE_H_ */
