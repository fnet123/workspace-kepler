/*
 * redispool.h
 *
 *  Created on: 2012-4-18
 *      Author: humingqing
 *
 *  Redis����������ӳأ���Ҫʵ�ֶ��ڴ������󲢷�ʱ������һ�ο��ٵ����ӿ��Ը��õ�ԭ��
 *  ���һ��ʱ����û��ʹ���ɶ�ʱ�̻߳��գ�����Redis��д������ͨ��������ʵ�ֶ�д�������������û�дӾ�ֱ�Ӵ��������ȡ
 */

#ifndef __REDISPOOL_H__
#define __REDISPOOL_H__

#include <icache.h>
#include <icontext.h>
#include <vector>
#include <string>
#include <Mutex.h>
#include <Thread.h>
#include <time.h>
#include <TQueue.h>

using std::string;

// ���ﶨ�������ʱ��Ϊ60��
#define OBJ_LIVE_TIME  120

struct redisContext ;
// Redis����
class RedisObj : public IRedisObj
{
	typedef std::vector<std::string> VecString ;
public:
	// �������ͷβָ��
	RedisObj *_next ;
	RedisObj *_pre  ;

public:
	RedisObj() ;
	~RedisObj() ;
	// ��ʼ������
	bool InitObj( const char *masterip, unsigned short masterport, const char *slaverip=NULL, unsigned short slaverport= 0) ;
	// ȡ�����ݶ���
	bool GetValue( const char *key , std::string &val ) ;
	// ���û��������ָ����ֵ
	bool SetValue( const char *key, const char *val ) ;
	// �Ƴ���������е�ֵ
	bool Remove( const char *key ) ;
	// ȡ�û�������������
	int GetList( const char *key, VecString &vec ) ;
	// ���û����������ֵ
	int SetList( const char *key, VecString &vec ) ;
	// �Ӷ����е���ֵ
	bool PopValue( const char *key , std::string &val ) ;
	// �����ݷŵ�����β��
	bool PushValue( const char *key, const char *val ) ;
	// ģ��ȡ������KEY��ֵ
	int  GetKeys( const char *key, VecString &vec ) ;
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
	// �ͷ����ж���
	void FreeObj( void ) ;
	// ����Ƿ���״̬
	bool Ping( void ) ;
	// ��������
	void auth(const char *password);
	// �������ݿ���
	void select(int number);
	// ������״̬
	bool stauts();

private:
	// ���ӷ�����
	redisContext * Connect( const char *ip, int port ) ;
	// ȡ���������͵�ֵ
	int  GetArray( const char *cmd, VecString &vec ) ;
	// ȡ���ַ����͵�ֵ
	bool GetString( const char *cmd, std::string &val ) ;
	// ִ���������͵�ֵ
	bool Execute( const char *cmd ) ;

private:
	// ���������
	redisContext *_ctmaster ;
	// �ӻ������
	redisContext *_ctslaver ;
	// ���һ�μ��ʱ��
	time_t		  _lastcheck;
	// ����
	string        _password;
	// ���ݿ���
	int           _number;
};

// ˫���л�����
#define BACK_POOL_SIZE  2
// Redis���ӳض���
class RedisPool : public share::Runnable
{
	typedef TQueue<RedisObj>   RedisObjList;
public:
	RedisPool() ;
	~RedisPool() ;

	// ��ʼ����������
	bool Init( IContext *pEnv ) ;
	// ��ʼ�߳�
	bool Start( void ) ;
	// ֹͣ����
	bool Stop( void ) ;

	// �����ӳ���ǩ������
	RedisObj * CheckOut( void ) ;
	// �������ӳض���
	void CheckIn(RedisObj *obj) ;

public:
	// �߳�ִ�ж���
	void run( void *param ) ;

private:
	// ������ַ�Ƿ�����
	bool PaserAddress( char *val ) ;
	// �������ж���
	void Clear( void ) ;

private:
	// �̹߳������
	share::ThreadManager  _threadpool;
	// ����ص�������
	share::Mutex		  _mutex ;
	// ����صĲ���
	RedisObjList		  _objpool[BACK_POOL_SIZE] ;
	// ��ǰʹ�óص����
	int 				  _index ;
	// ��ʼ������
	bool 				  _inited ;
	// �������IP��ַ
	std::string 		  _masterip ;
	// �����Ķ˿�
	unsigned short        _masterport ;
	// �ӻ����IP��ַ
	std::string			  _slaverip ;
	// �ӻ���Ķ˿�
	unsigned short   	  _slaverport ;
	// ����
	string                _password;
	// ���
	int                   _number;
};


#endif /* REDISPOOL_H_ */
