/**********************************************
 * systemenv.h
 *
 *  Created on: 2014-05-23
 *      Author: ycq
 *********************************************/
#ifndef __SYSTEMENV_H__
#define __SYSTEMENV_H__

#include "interface.h"

class CCConfig ;
class CSystemEnv : public ISystemEnv
{
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
	// ȡ���û�����λ��
	const char * GetUserPath( void )  {return _user_file_path.c_str();}
	// ȡ��RedisCache
	IRedisCache * GetRedisCache(void) {return _rediscache;}
	//
	IProtParse * GetProtParse(void) { return _protparse;}
	//
	IExchServer * GetExchServer(void) {return _exchserver;}
	
private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath , const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �û��ļ�·��
	std::string 	   _user_file_path ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	//
	IProtParse        *_protparse;
	//
	IRedisCache       *_rediscache;
	//
	IExchServer       *_exchserver;
};

#endif
