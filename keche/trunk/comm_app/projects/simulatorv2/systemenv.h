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
#include "Logistics.h"

class CCConfig ;
class CSystemEnv : public ISystemEnv
{
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *userfile ) ;

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
	// ȡ��Msg����
	IVechileClient * GetVechileClient( void ){ return _vechile_client; } ;

private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �û��ļ�·��
	std::string 	   _user_file_path ;
	// Msg�������Ϳͻ���
	IVechileClient   * _vechile_client ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
};

#endif
