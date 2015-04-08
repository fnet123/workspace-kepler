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

#include <icache.h>
#include <icontext.h>

class ISystemEnv ;

class ISynServer
{
public:
	virtual ~ISynServer() {} ;
	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// ��ʼ����������
	virtual bool Start( void ) = 0 ;
	// STOP����
	virtual void Stop( void ) = 0 ;
};

class ISystemEnv: public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��ͼƬ������
	virtual ISynServer* GetSynServer( void ) = 0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
};

#endif
