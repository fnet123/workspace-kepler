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

#define MSG_MSGPROC  	0    // ���ݴ������
#define MSG_PLUGIN    	1	 // ����������

// ������ʱֻ������������
#define MAX_MSGHANDLE   2    // ��ʱ�����Ĵ���������

class ISystemEnv ;
// �ڲ�Э�������
class InterData
{
public:
	std::string  _transtype ;  // ��������
	std::string  _seqid ;	  // ���к�
	std::string  _macid ;	  // MACID
	std::string  _cmtype ;     // ͨ������
	std::string  _command ;	  // ����ģʽ
	std::string  _packdata ;   // ��������
};

#define OP_SUBSCRIBE_DMD			0    // ��������
#define OP_SUBSCRIBE_ADD			1    // ��������
#define OP_SUBSCRIBE_UMD			2    // ȡ������

#define TYPE_SUBSCRIBE_MACID     	0   // ͨ��MACID����
#define TYPE_SUBSCRIBE_GROUP     	1   // ͨ����������
// ���ݷ�������ӿ�
class IPublisher
{
public:
	virtual ~IPublisher() {} ;
	// ��ʼ����������
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// �������������߳�
	virtual bool Start( void ) = 0 ;
	// ֹͣ���������߳�
	virtual bool Stop( void ) = 0 ;
	// ��ʼ��������
	virtual bool Publish( InterData &data, unsigned int cmd , User &user ) = 0 ;
	// �������ݶ���
	virtual bool OnDemand( unsigned int cmd , unsigned int group, const char *szval, User &user ) = 0 ;
};

// ���ݴ������
class IMsgHandler
{
public:
	virtual ~IMsgHandler() {} ;
	// ��ʼ��
	virtual bool Init( ISystemEnv * pEnv ) = 0 ;
	// ��������
	virtual bool Start( void ) = 0 ;
	// ֹͣ����
	virtual bool Stop( void ) = 0 ;
	// ��������
	virtual bool Process( InterData &data , User &user ) = 0 ;
};

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

// ͬ�������ĵ�MSG�Ŀͻ��˴�������
class IMsgClient
{
public:
	virtual ~IMsgClient() {} ;
	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// ��ʼ����
	virtual bool Start( void ) = 0 ;
	// ֹͣ����
	virtual void Stop() = 0 ;
	// �ϴ�����
	virtual void HandleData( const char *data, int len , bool pic ) = 0 ;
};

// ��Ϣ����������
class IMsgClientServer
{
public:
	virtual ~IMsgClientServer() {} ;

	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
	// ȡ���ڳ�������ѹ��
	virtual int  GetOnlineSize( void ) = 0 ;
	// ��������û��б�
	virtual void AddNodeUser( const char *user, const char *pwd ) = 0 ;
	// ��������
	virtual bool Deliver( socket_t *sock, const char *data, int len ) = 0 ;
	// ͨ���û�ID����������
	virtual bool DeliverEx( const char *userid, const char *data, int len ) = 0 ;
	// �ر�����
	virtual void Close( socket_t *sock ) = 0 ;
};

/**
��¼����USAVE���洢����	���У������ڲ�Э��ָ��
��¼����UWEB�����ƽ̨	���У�������ָ��
��¼����UANLY����������	���У�D_REPT(λ����Ϣ)��U_REPT(����λ����Ϣ��
*/

#define COMPANY_TYPE 	"PIPE"
#define WEB_TYPE 		"WEB"
#define STORAGE_TYPE 	"SAVE"
#define PROXY_TYPE		"PROXY"
#define SEND_TYPE		"SEND"  // �·�����
#define MSG_DATA_DM     "DM"  // ���ݶ��ķ�ʽ

#define PIPE_SEND_CMD   	0x00000001   // ���͵�PIPE
#define UWEB_SEND_CMD   	0x00000002   // ���͵�WEB
#define SAVE_SEND_CMD   	0x00000004   // ���͵�SAVE
#define SEND_SEND_CMD		0x00000008   // ���͵����ͷ���

#define MSG_USER_ENCODE     0x0001		 // �Ƿ�Ϊ����
#define MSG_USER_DEMAND     0x0002  	 // �Ƿ�Ϊ����

// �û��������
class IGroupUserMgr
{
public:
	virtual ~IGroupUserMgr() {} ;
	//0 success; -1,���û����Ѿ�����,�Ҳ������Ƿ����ߡ�
	virtual bool AddUser(const std::string &user_id,const User &user) = 0;
	// ȡ���û�ͨ��FD
	virtual bool GetUserBySocket( socket_t *sock , User &user) = 0 ;
	// ͨ���û�ID��ȡ���û�
	virtual bool GetUserByUserId( const std::string &user_id, User &user ,  bool bgroup = false ) = 0 ;
	// ɾ���û�ͨ��FD
	virtual void DeleteUser( socket_t *sock ) = 0 ;
	// ɾ���û�
	virtual void DeleteUser(const std::string &user_id) = 0 ;
	// ȡ�������û�
	virtual bool GetOnlineUsers( std::vector<User> &vec ) = 0 ;
	// �����û���״̬
	virtual bool SetUser(const std::string &user_id,User &user) = 0 ;
	// ����Ƿ���Ҫ���͵�����
	virtual bool GetSendGroup( std::vector<User> &vec, unsigned int hash , unsigned int cmd ) = 0 ;
	// ȡ��Hashֵ
	virtual unsigned int GetHash( const char *key , int len ) = 0 ;
	// ��ǰ����������
	virtual int GetSize( void ) = 0 ;
};

class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��CAS����
	virtual IMsgClientServer * GetMsgClientServer( void ) = 0 ;
	// ȡ���û��������
	virtual IGroupUserMgr * GetUserMgr( void ) = 0 ;
	// ȡ�÷����������
	virtual IPublisher *GetPublisher( void ) = 0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;
	// ȡ�ö�Ӧ�������Ķ���
	virtual IMsgHandler * GetMsgHandler( unsigned int id ) = 0 ;
	// ȡ������ͬ��msgclient
	virtual IMsgClient  * GetMsgClient( void ) =  0 ;
#ifdef _GPS_STAT
	// ���ﴦ��GPS������ͳ�Ƽ�����
	virtual void AddGpsCount( unsigned int count ) = 0 ;
	virtual void SetGpsState( bool enable ) = 0 ;
	virtual bool GetGpsState( void ) = 0 ;
	virtual int  GetGpsCount( void ) = 0 ;
#endif
};

#endif
