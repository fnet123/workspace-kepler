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

#include <map>
#include "iplugin.h"

class CCConfig ;
class CSystemEnv : public IPlugin
{
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *logname ) ;
	// ��ʼϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// ȡ������ֵ
	bool GetInteger( const char *key , int &value ) ;
	// ȡ���ַ���ֵ
	bool GetString( const char *key , char *value ) ;
	// �ص���������
	void OnDeliver( unsigned int fd, const char *data, int len , unsigned int cmd ) ;
	// ȡ��ͨ��
	IPlugWay * GetPlugWay( void ) { return _waytester; }

private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath , const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// ����ͨ������
	IPlugWay		  *_waytester;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
};

#endif
