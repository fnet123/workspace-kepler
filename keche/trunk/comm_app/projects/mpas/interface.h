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

#include <inodeface.h>
#include <icontext.h>
#include <icache.h>

class ISystemEnv ;
// MSG���ݴ�����
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
	// ��MSG�·���Ϣ
	virtual bool Deliver( const char *data, int len )  = 0 ;
	// ����û�����
	virtual void AddUser( const char *ip, unsigned short port, const char *user, const char *pwd ) = 0 ;
	// ����ɾ������
	virtual void DelUser( const char *ip, unsigned short port ) = 0 ;
	// ����ֻ�MAC
	virtual void AddMac2Car( const char *macid, const char *vechile ) = 0 ;
};

// �ڵ�������
class INodeClient
{
public:
	virtual ~INodeClient() {} ;
	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// �����ڵ�ͻ�
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
};

// Pas�������ӿ�
class IPasServer
{
public:
	virtual ~IPasServer() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv ) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0 ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
	// �ͻ����ڷ׷�����
	virtual void HandlePasDown( const char *code , const char *data, const int len ) = 0 ;
};

// ��������ָ��
class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��MSG�Ĳ���
	virtual IMsgClient *  GetMsgClient( void ) =  0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
	// ȡ��PAS�Ĵ�����
	virtual IPasServer *  GetPasServer( void ) = 0 ;
};

#endif
