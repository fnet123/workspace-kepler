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

#ifndef __SYSTEMENV_H__
#define __SYSTEMENV_H__

#include "interface.h"

class PConvert ;
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
	// ȡ���û�����·��
	const char * GetUserPath( void ) { return NULL; }
	// ȡ��PAS����
	IPasClient * GetPasClient( void ) { return _pas_client; }
	// ȡ��MSG Client ����
	IMsgClient * GetMsgClient( void ) { return _msg_client; }
	// ȡ��MsgCache����
	IMsgCache  * GetMsgCache( void ) { return _msg_cache ; }
private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath, const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// ����Э�������
	IPasClient 		 * _pas_client ;
	// ʵ�ּ��ƽ̨ת��Э��
	IMsgClient		 * _msg_client ;
	// ���ݻ���
	IMsgCache		 * _msg_cache ;
	// Э��ת������
	PConvert		 * _convert ;
};

#endif
