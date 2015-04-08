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
#include <imsgcache.h>

#define SEND_ALL   "SEND_ALL"

class ISystemEnv ;

// ʵ�ֱ�׼����Э�����
class IPasClient
{
public:
	virtual ~IPasClient() {} ;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv ) = 0 ;
	// ��ʼ�߳�
	virtual bool Start( void ) = 0 ;
	// ֹͣ����
	virtual void Stop( void ) = 0 ;
	// �ͻ����ڷ׷�����
	virtual void HandleData( const char *data, const int len ) = 0 ;
};

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
	// ��MSG�ϴ���Ϣ
	virtual bool HandleUpMsgData( const char *code, const char *data, int len )  = 0 ;
	// ͨ��HTTP������ȡͼƬ
	virtual void LoadUrlPic( unsigned int seq, const char *path ) = 0 ;
};

class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��PAS�Ķ���
	virtual IPasClient * GetPasClient( void ) = 0 ;
	// ȡ��MSG Client����
	virtual IMsgClient * GetMsgClient( void ) =  0 ;
	// ȡ��MsgCache����
	virtual IMsgCache  * GetMsgCache( void ) = 0 ;
};

#endif
