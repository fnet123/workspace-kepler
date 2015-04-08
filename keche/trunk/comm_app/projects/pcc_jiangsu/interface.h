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

#define SEND_ALL   "SEND_ALL"

class ISystemEnv ;
// ��Ϣ�������
class IMsgCache
{
public:
	virtual ~IMsgCache() {} ;
	// �������
	virtual bool AddData( const char *key, const char *buf, const int len ) = 0 ;
	// ȡ������
	virtual char *GetData( const char *key, int &len ) = 0  ;
	// �ͷ�����
	virtual void FreeData( char *data ) = 0 ;
	// ����ʱ������
	virtual void CheckData( int timeout ) = 0 ;
	// �Ƴ�����
	virtual bool Remove( const char *key ) = 0 ;
};

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
	virtual bool HandleData( const char *data, const int len ) = 0 ;
	// ȡ��PAS��Ӧ����ID
	virtual const char * GetSrvId( void )  = 0 ;
	// ��ǰ�û��Ƿ�����
	virtual bool IsOnline( void ) = 0 ;
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
	virtual void HandleUpMsgData( const char *code, const char *data, int len )  = 0 ;
	// ȡ���ֻ���
	virtual bool GetCarMacId( const char *key, char *macid ) = 0 ;
};

// ʵ��Э��ת������
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
};

// PCC������־����
#define  PCCMSG_LOG( logger, ip, port, userid, key, fmt, ... )   \
	logger->print_net_msg( 1 , __FILE__, __LINE__, __FUNCTION__, key , ip, port, userid, fmt ,  ## __VA_ARGS__ )

class CCLog ;
class ISystemEnv
{
public:
	virtual ~ISystemEnv() {} ;
	// ��ʼ��ϵͳ
	virtual bool Init( const char *file , const char *logpath, const char *logname ) = 0 ;

	// ��ʼϵͳ
	virtual bool Start( void ) = 0 ;

	// ֹͣϵͳ
	virtual void Stop( void ) = 0 ;

	// ȡ������ֵ
	virtual bool GetInteger( const char *key , int &value ) = 0 ;
	// ȡ���ַ���ֵ
	virtual bool GetString( const char *key , char *value ) = 0 ;
	// ȡ��Cache��KEYֵ
	virtual void GetCacheKey( const char *macid , unsigned short data_type , char *buf ) = 0 ;
	// ȡ��PAS�Ķ���
	virtual IPasClient * GetPasClient( void ) = 0 ;
	// ȡ��MSG Client����
	virtual IMsgClient * GetMsgClient( void ) =  0 ;
	// ȡ��MsgCache����
	virtual IMsgCache  * GetMsgCache( void ) = 0 ;
	// ȡ�ö�Ӧ��LOG����
	virtual CCLog *      GetLogger() = 0 ;
};

#endif
