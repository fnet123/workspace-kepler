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
// ʵ��Э��ת������
class IMsgClient
{
public:
	virtual ~IMsgClient() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv ) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0 ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
};

class ISystemEnv
{
public:
	virtual ~ISystemEnv() {} ;
	// ��ʼ��ϵͳ
	virtual bool Init( const char *file , const char *logpath , const char *logname ) = 0 ;
	// ��ʼϵͳ
	virtual bool Start( void ) = 0 ;
	// ֹͣϵͳ
	virtual void Stop( void ) = 0 ;
	// ȡ������ֵ
	virtual bool GetInteger( const char *key , int &value ) = 0 ;
	// ȡ���ַ���ֵ
	virtual bool GetString( const char *key , char *value ) = 0 ;
};

#endif
