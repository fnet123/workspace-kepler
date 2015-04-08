/*
 * icontext.h
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  ������ֹͣ
 */

#ifndef __ICONTEXT_H__
#define __ICONTEXT_H__

class IContext
{
public:
	// ȡ��������ָ��
	virtual ~IContext(){}
	// ��ʼ��ϵͳ
	virtual bool Init( const char *file , const char *logpath , const char *userfile  , const char *logname ) = 0 ;
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
};


#endif /* ICONTEXT_H_ */
