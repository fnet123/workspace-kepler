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

#include <qstring.h>
#include <icontext.h>
#include <icache.h>

class ISystemEnv ;
#define TIMEOUT_LIVE			180

class IClient
{
public:
	virtual ~IClient() {}
	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0 ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
	// ���·�������
	virtual void HandleUpData( const char *data, const int len ) = 0 ;
};

class IServer
{
public:
	virtual ~IServer() {} ;

	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
	// ����was�·�������
	virtual void HandleDownData( const char *userid, const char *data, int len , unsigned int seq = 0 , bool send = true ) = 0 ;
	// ȡ�����߳�����
	virtual int  GetOnlineSize( void ) = 0 ;
	// ����TTS��������
	virtual void SendTTSMessage( const char *userid, const char *msg, int len ) = 0 ;
};

class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;

	virtual unsigned short GetSequeue( const char *key , int len = 1 ) = 0 ;
	// �����û�����
	virtual void ResetSequeue( const char *key ) = 0  ;
	// ȡ��Msg�������
	virtual IClient    * GetMsgClient( void ) = 0 ;
	// ȡ��CAS����
	virtual IServer    * GetClientServer( void ) = 0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
#ifdef _GPS_STAT
	// ���ﴦ��GPS������ͳ�Ƽ�����
	virtual void AddGpsCount( unsigned int count ) = 0 ;
	virtual void SetGpsState( bool enable ) = 0 ;
	virtual bool GetGpsState( void ) = 0 ;
	virtual int  GetGpsCount( void ) = 0 ;
#endif
};

#endif
