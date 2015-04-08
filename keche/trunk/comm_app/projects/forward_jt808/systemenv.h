/**********************************************
 * systemenv.h
 *
 *  Created on: 2014-8-19
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
	const char * GetUserPath( void )  { return _user_file_path.c_str(); }
	//
	IWasClient * GetWasClient(void) { return _wasclient;	}
	//
	IWasServer * GetWasServer(void) { return _wasserver; }
	//
	IRedisCache * GetRedisCache(void) { return _rediscache; }
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
	// jt808�ͻ��˶���
	IWasClient       * _wasclient ;
	// jt808����˶���
	IWasServer 		 * _wasserver ;
	// redis�ͻ��˶���
	IRedisCache      * _rediscache ;
};

#endif
