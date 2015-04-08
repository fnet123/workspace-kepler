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
#include <OnlineUser.h>
#include <icontext.h>
#include <icache.h>

class ISystemEnv ;
// �ڵ�������
class INodeClient
{
public:
	virtual ~INodeClient() {} ;

	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
};

#define DEMAND_MACID   0     // ͨ��MACID����  OME_PHONE  4C54_132343423
#define DEMAND_GROUP   1	 // ͨ������ж���     group-> macid list

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
	// ��MSG�ϴ���Ϣ
	virtual void HandleMsgData( const char *macid, const char *data, int len )  = 0 ;
	// ����û�����
	virtual void AddUser( const char *ip, unsigned short port, const char *user, const char *pwd ) = 0 ;
	// ����ɾ������
	virtual void DelUser( const char *ip, unsigned short port ) = 0 ;
	// ��Ӷ��Ļ���
	virtual void AddDemand( const char *name, int type )  = 0 ;
	// ȡ��������
	virtual void DelDemand( const char *name, int type )  = 0 ;
};

// �������ͷ���
class IPushServer
{
public:
	virtual ~IPushServer() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0;
	// ֹͣ
	virtual void Stop( void ) = 0;
	// ��MSGת������������
	virtual void HandleData( const char *data, int len ) = 0 ;
};

// ��������ָ��
class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��MSG�Ĳ���
	virtual IMsgClient *  GetMsgClient( void ) =  0;
	// ȡ�����ͷ������
	virtual IPushServer * GetPushServer( void ) = 0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
};

#endif
