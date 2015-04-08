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

#include <map>
#include <string>

using std::map;
using std::string;

struct InnerMsg {
	string begin;
	string seqid;
	string macid;
	string order;
	map<string, string> param;
};

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
	//
	virtual void HandleData( const InnerMsg & msg ) = 0 ;
};

// ���������豸����Ϣת������
class IPhotoSvr {
public:
	virtual ~IPhotoSvr() {};

	virtual bool Init(ISystemEnv *pEnv) = 0;
	virtual bool Start(void) = 0;
	virtual void Stop(void) = 0;

	virtual void HandleData( const InnerMsg & msg ) = 0 ;
};

// ��������ָ��
class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��MSG�Ĳ���
	virtual IMsgClient *  GetMsgClient( void ) =  0;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
	//
	virtual IPhotoSvr *   GetPhotoSvr(void) = 0;
};

#endif
