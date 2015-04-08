/**********************************************
 * pasclient.h
 *
 *  Created on: 2011-07-28
 *      Author: humingqing
 *    Comments: ����ƽ̨�ԽӴ���
 *********************************************/

#ifndef __PASCLIENT_H__
#define __PASCLIENT_H__

#include "interface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseClient.h>
#include "usermgr.h"
#include <ProtoParse.h>
#include <Session.h>
#include "servicecaller.h"
#include "filecacheex.h"
#include "postquery.h"
#include "statinfo.h"

#define PAS_USER_TAG 	"PASCLIENT"
#define PAS_TAG_LEN     9

class PasClient :
	public BaseClient , public IPasClient , public IOHandler , public IUserNotify
{
	struct SmsNotify {
		time_t btime;
		time_t ltime;
		int count;
	};
public:
	PasClient( CServiceCaller &srvCaller ) ;
	virtual ~PasClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ
	virtual bool Start( void ) ;
	// ֹͣ
	virtual void Stop( void ) ;
	// ��PAS������
	virtual void HandleClientData( const char *code, const char *data, int len ) ;
	// �������ŵ�����ʡƽ̨DOWN������
	virtual void HandlePasDownData( const int access, const char *data, int len ) ;
	// ��PAS������ͨ��������
	virtual void HandlePasUpData( const int access, const char *data, int len ) ;
	// ���MACID��SEQID��ӳ���ϵ
	virtual void AddMacId2SeqId( unsigned short msgid, const char *macid, const char *seqid ) ;
	// ͨ��MACID����Ϣ����ȡ�ö�Ӧ����
	virtual bool GetMacId2SeqId( unsigned short msgid, const char *macid, char *seqid ) ;
	// �ر�����·����������
	virtual void Close( int accesscode ) ;
	// ���µ�ǰ����״̬
	virtual void UpdateSlaveConn( int accesscode, int state ) ;
	// ֱ�ӶϿ���Ӧʡ�����Ӵ���
	virtual void Enable( int areacode , int flag ) ;

protected:
	// �����ⲿ����
	virtual int  HandleQueue( const char *id, void *buf, int len , int msgid = 0 ) ;

	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();

	// ֪ͨ�û�״̬�仯
	virtual void NotifyUser( const _UserInfo &info , int op ) ;

private:
	// ���͵�PAS����
	bool SendDataToUser( const string &area_code, const char *data, int len);
	// ���ӷ���������
	bool ConnectServer(User &user, int time_out /*= 10*/);
	// ���������û�
	void HandleOfflineUsers( void ) ;
	// ���������û�
	void HandleOnlineUsers(int timeval) ;
	// ����5B���봦��
	bool Send5BCodeData( socket_t *sock, const char *data, int len  , bool bflush = false ) ;
	// ����ѭ���봦��
	bool SendCrcData( socket_t *sock, const char* data, int len) ;
	// �������ݰ�
	void HandleOnePacket( socket_t *sock, const char* data , int len ) ;
	// ȡ�õ�ǰ�û����������
	int  GetAreaCode( User &user ) ;
	// ��USERINFOת��ΪUser����
	void ConvertUser( const _UserInfo &info, User &user ) ;
	// ���ܴ�������
	bool EncryptData( unsigned char *data, unsigned int len , bool encode ) ;
	// ���Ͷ��š��ʼ�����
	void sendSmsNotify(const User &user);

private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// ���һ�η���ʱ��
	time_t				_last_handle_user_time ;
	// Э�����
	ProtoParse 			_proto_parse;
	// �û��б����
	CUserMgr  			_online_user;
	// ����IP��ַ
	string				_down_ip ;
	// ���ж˿�
	unsigned short      _down_port ;
	// MACID��SEQID��ӳ���
	CSessionMgr			_macid2seqid ;
	// ҵ���첽http�ص�
	CServiceCaller     &_srvCaller;
	// �ļ��������
	CFileCacheEx		_filecache ;
	// ƽ̨������ݹ���
	CPostQueryMgr		_postquerymgr ;
	// PCC����ͳ�Ʒ���
	CStatInfo			_statinfo ;
	// ƽ̨����ļ���·��
	std::string 		_postpath ;
	//
	map<string, SmsNotify> _smsNotifySet;
	//
	share::Mutex           _smsNotifyMtx;
	//
	string                 _smsNotifyUrl;
	//
	string                 _smsNotifyMail;
	//
	string                 _smsNotifyTitle;
};

#endif /* LISTCLIENT_H_ */
