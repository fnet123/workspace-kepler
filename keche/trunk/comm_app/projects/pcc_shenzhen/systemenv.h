/**********************************************
 * systemenv.h
 *
 *  Created on: 2014-06-30
 *      Author: ycq
 *********************************************/

#ifndef _SYSTEMENV_H_
#define _SYSTEMENV_H_ 1

#include "interface.h"

class CCConfig ;
class CSystemEnv : public ISystemEnv {
public:
	CSystemEnv() ;
	~CSystemEnv() ;
	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *userfile  , const char *logname ) ;
	// ��ʼϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// ȡ������ֵ
	bool GetInteger( const char *key , int &value ) ;
	// ȡ���ַ���ֵ
	bool GetString( const char *key , char *value ) ;
	// ȡ���û�����·��
	const char * GetUserPath( void ) { return NULL; }
	// ȡ��PAS����
	IPasClient * GetPasClient( void ) { return _pas_client; }
	// ȡ��MSG Client ����
	IMsgClient * GetMsgClient( void ) { return _msg_client; }
	// ȡ��RedisCache
	IRedisCache * GetRedisCache( void ) { return _rediscache; }
private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath, const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// �ǹ�����ƽ̨�ͻ���
	IPasClient 		 * _pas_client ;
	// msg�ͻ��˶���ָ��
	IMsgClient		 * _msg_client ;
	// redis�ͻ��˶���
	IRedisCache       *_rediscache;
};
#endif//_SYSTEMENV_H_
