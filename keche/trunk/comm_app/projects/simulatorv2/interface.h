/**********************************************
 * interface.h
 *
 *  Created on: 2011-07-24
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments: ��������ӿ��ඨ�壬��Ҫʵ��������֮�佻���Ľӿڶ���
 *********************************************/
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

class ISystemEnv ;
class IVechileClient
{
public:
	virtual ~IVechileClient() {} ;

	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
};

class ISystemEnv
{
public:
	virtual ~ISystemEnv() {} ;
	// ��ʼ��ϵͳ
	virtual bool Init( const char *file , const char *logpath , const char *userfile ) = 0 ;

	// ��ʼϵͳ
	virtual bool Start( void ) = 0 ;

	// ֹͣϵͳ
	virtual void Stop( void ) = 0 ;

	// ȡ������ֵ
	virtual bool GetInteger( const char *key , int &value ) = 0 ;
	// ȡ���ַ���ֵ
	virtual bool GetString( const char *key , char *value ) = 0 ;
	// ȡ���û�����·��
	virtual const char * GetUserPath( void ) = 0 ;
	// ȡ��CAS����
	virtual IVechileClient * GetVechileClient( void ) = 0 ;
};

#endif
