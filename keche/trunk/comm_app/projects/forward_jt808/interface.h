/**********************************************
 * interface.h
 *
 *  Created on: 2014-8-19
 *      Author: ycq
 *********************************************/
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <icontext.h>
#include <icache.h>

class ISystemEnv ;

#include <string>
using std::string;

// ����808�ͻ���
class IWasClient {
public:
	virtual ~IWasClient() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0;
	// ֹͣ
	virtual void Stop( void ) = 0;
	// �������ݴ�����
	virtual bool HandleData(const string &sim, const uint8_t *ptr, size_t len) = 0;
	// ����ģ���ն�
	virtual bool AddTerminal(const string &sim) = 0;
	// ɾ��ģ���ն�
	virtual bool DelTerminal(const string &sim) = 0;
};

// ����808�����
class IWasServer {
public:
	virtual ~IWasServer() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0;
	// ֹͣ
	virtual void Stop( void ) = 0;
	// �������ݴ�����
	virtual bool HandleData(const string &sim, const uint8_t *ptr, size_t len) = 0;
	// �ж��ն��Ƿ�����
	virtual bool ChkTerminal(const string &sim) = 0;
};

// ��������ָ��
class ISystemEnv: public IContext {
public:
	virtual ~ISystemEnv() {}

	virtual IWasClient* GetWasClient(void) = 0;
	virtual IWasServer* GetWasServer(void) = 0;
	virtual IRedisCache* GetRedisCache(void) = 0;
};

#endif
