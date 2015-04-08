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

#include <icontext.h>
#include <icache.h>

class ISystemEnv ;

#include <set>
using std::set;
#include <string>
using std::string;

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
	// �������ݴ�����
	virtual bool HandleMsgData(const char *data, int len) = 0;
};

// �������ͷ���
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
	virtual bool HandleMsgData(const string userid, const char *data, int len) = 0;
};

// �û�����
class IUserMgr {
public:
	virtual ~IUserMgr() {};

	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0;
	// ��ʼ
	virtual bool Start(void) = 0;
	// ֹͣ
	virtual void Stop(void) = 0;

	// ��ѯ�ַ�·��
	virtual set<string> getRoute(const string &macid) = 0;
	// ��ѯͨ����Ϣ
	virtual string getCorpInfo(const string &corpid) = 0;
	// ȷ�Ϸַ�·��
	virtual bool chkRoute(const string &userid) = 0;
	// ��ȡ����·��
	virtual set<string> getAllRoute() = 0;
};

// ��������ָ��
class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��MSG�Ĳ���
	virtual IMsgClient *  GetMsgClient( void ) =  0;
	//
	virtual IWasClient *  GetWasClient( void ) =  0;
	//
	virtual IUserMgr   *  GetUserMgr(void) = 0;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
};

#endif
