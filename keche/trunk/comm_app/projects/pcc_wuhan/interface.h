/**********************************************
 * interface.h
 *
 *  Created on: 2014-06-30
 *      Author: ycq
 *********************************************/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_ 1

#include <icache.h>
#include <icontext.h>

class ISystemEnv;

// �ǹ�����Э�����
class IPasClient {
public:
	virtual ~IPasClient() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv ) = 0 ;
	// ��ʼ�߳�
	virtual bool Start( void ) = 0 ;
	// ֹͣ����
	virtual void Stop( void ) = 0 ;
	// �ͻ����ڷ׷�����
	virtual bool HandleData( const char *data, const int len ) = 0 ;
};

// ʵ��Э��ת������
class IMsgClient {
public:
	virtual ~IMsgClient() {}
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0;
	// ��ʼ
	virtual bool Start(void) = 0;
	// ֹͣ
	virtual void Stop(void) = 0;
	// ��MSG�ϴ���Ϣ
	virtual bool HandleData(const char *data, int len) = 0;
};

class ISystemEnv: public IContext {
public:
	virtual ~ISystemEnv() {}
	// ȡ��PAS client����
	virtual IPasClient * GetPasClient(void) = 0;
	// ȡ��MSG client����
	virtual IMsgClient * GetMsgClient(void) = 0;
	// ȡ��Redis Cache����
	virtual IRedisCache * GetRedisCache(void) = 0;
};
#endif//_INTERFACE_H_
