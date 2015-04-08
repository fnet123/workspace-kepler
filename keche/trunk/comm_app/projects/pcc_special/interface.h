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


#include <OnlineUser.h>
#include <icache.h>
#include <icontext.h>
#include <imsgcache.h>

#include <string>
using std::string;
#include <set>
using std::set;
#include <list>
using std::list;

#define SEQ_HEAD_LEN     11
#define SEQ_HEAD   		 "USERID_UTC_"
#define SEND_ALL   		 "SEND_ALL"
#define DATA_FILECACHE   0  // ���ݻ���
#define DATA_ARCOSSDAT	 1  // �������

#define CONN_MASTER         0    // ����·����
#define CONN_SLAVER         1    // ����·����
#define CONN_CONNECT      	0	 // ���Ӵ���
#define CONN_DISCONN    	-1   // �Ͽ�����

class ISystemEnv ;

#define USER_ADDED   1  // ������û�
#define USER_DELED   2  // ɾ���û�
#define USER_CHGED   3  // �޸��û�
// �����û��仯��֪ͨ����
class IUserNotify
{
public:
	// PASCLIENT:110:192.168.5.45:9880:701115:701115:701115:M1_IA1_IC1
	// MSGCLIENT:1:10.1.99.115:8880:user_name:user_password:A3
	struct _UserInfo{
		std::string tag  ;
		std::string code ;
		std::string ip   ;
		short 		port ;
		std::string user ;
		std::string pwd  ;
		std::string type ;
	};
	// �Ƚ��û�
	static bool Compare( _UserInfo &u1, _UserInfo &u2 ) {
		if ( u1.ip != u2.ip || u1.port != u2.port ||
				u1.user != u2.user || u1.pwd != u2.pwd || u1.type != u2.type ){
			return false ;
		}
		return true ;
	}
public:
	virtual ~IUserNotify() {} ;
	// ֪ͨ�û�״̬�仯
	virtual void NotifyUser( const _UserInfo &info , int op ) = 0  ;
};

#define PAS_SUBLINK_ERROR     0x01    // �������·�쳣
#define PAS_MAINLINK_LOGOUT   0x02    // ��������·����
#define PAS_USERLINK_ONLINE   0x04    // �����û����Ӷ���
#define PAS_MAINLINK_ERROR    0x08    // �����û�����·�쳣�����

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
	virtual void HandleClientData( const char *code, const char *data, const int len ) = 0 ;
	// �������ŵ�����ʡƽ̨DOWN������
	virtual void HandlePasDownData( const int access, const char *data, int len ) = 0 ;
	// ��PAS������ͨ��������
	virtual void HandlePasUpData( const int access, const char *data, int len ) = 0 ;
	// ���MACID��SEQID��ӳ���ϵ
	virtual void AddMacId2SeqId( unsigned short msgid, const char *macid, const char *seqid ) = 0 ;
	// ͨ��MACID����Ϣ����ȡ�ö�Ӧ����
	virtual bool GetMacId2SeqId( unsigned short msgid, const char *macid, char *seqid ) = 0 ;
	// �ر�����·����������
	virtual void Close( int accesscode ) = 0 ;
	// ���µ�ǰ����״̬
	virtual void UpdateSlaveConn( int accesscode, int state ) = 0 ;
	// ֱ�ӶϿ���Ӧʡ�����Ӵ����Ͽ�����·�������
	virtual void Enable( int areacode , int flag ) = 0 ;
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
	//�����û�����
	virtual void HandleUserData( const User &user, const char *data, int len ) = 0 ;
	// ��ͼƬ����
	virtual void LoadUrlPic( unsigned int seqid , const char *path ) = 0 ;
};

// PCC������
class IPccServer
{
public:
	virtual ~IPccServer() {} ;
	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	// ��ʼ�߳�
	virtual bool Start( void ) = 0 ;
	// ֹͣ����
	virtual void Stop( void ) = 0 ;
	// ����·��������Ͽ�������������
	virtual void Close( int accesscode , unsigned short msgid, int reason ) = 0 ;
	// ���µ�ǰPCC����ʡ��·��
	virtual void updateAreaids(const string &areaids) = 0;
};

class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ��������
	virtual bool SetNotify( const char *tag, IUserNotify *notify ) = 0 ;
	// �����û�����
	virtual bool LoadUserData( void ) = 0 ;
	// ȡ�ü�����Կ
	virtual bool GetUserKey( int accesscode, int &M1, int &IA1, int &IC1 ) = 0 ;
	// ȡ��Cache��KEYֵ
	virtual void GetCacheKey( unsigned int seq, char *key ) = 0 ;
	// ����Ự����
	virtual void ClearSession( const char *key ) = 0 ;
	// ȡ����Ŵ���
	virtual unsigned int GetSequeue( void ) = 0 ;
	// ȡ��PAS�Ķ���
	virtual IPasClient * GetPasClient( void ) = 0 ;
	// ȡ��MSG Client����
	virtual IMsgClient * GetMsgClient( void ) =  0 ;
	// ȡ��MsgCache����
	virtual IMsgCache  * GetMsgCache( void ) = 0 ;
	// ȡ��PCC������
	virtual IPccServer * GetPccServer( void ) = 0 ;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache( void ) = 0 ;

	// ʹ��macid��ȡ��Ҫת���ļ��ƽ̨
	virtual bool getChannels(const string &macid, set<string> &channels) = 0;
	// ��ȡ����macid������msg����
	virtual bool getSubscribe(list<string> &macids) = 0;
	// ʹ�ó��ƺ����ȡmacid
	virtual bool getMacid(const string &plate, string &macid) = 0;
};

#endif
