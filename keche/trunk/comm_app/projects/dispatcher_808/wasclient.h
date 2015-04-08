
#ifndef _WAS_CLIENT_
#define _WAS_CLIENT_ 1

#include "protocol.h"
#include "tqueue.h"
#include "utils.h"

#include <interface.h>
#include <BaseClient.h>
#include <OnlineUser.h>
#include <interpacker.h>

class WasClient : public BaseClient, public IWasClient
{
	struct Gb808WaitReply {
		string userid;
		time_t login;
		int number;
		vector<unsigned char> msg808;
	};

	struct InnerWaitReply {
		string userid;
		time_t login;
		Gb808SimpleMsg simpleMsg;
	};
public:
	WasClient();
	~WasClient();

	virtual bool Init( ISystemEnv *pEnv );
	// �����ڵ�ͻ�
	virtual bool Start( void );
	// ����STOP����
	virtual void Stop( void );

	virtual void on_data_arrived(socket_t *sock, const void* data, int len);
	virtual void on_dis_connection(socket_t *sock);
	virtual void on_new_connection(socket_t *sock, const char* ip, int port) {};
	virtual int build_login_msg( User &user, char *buf,int buf_len ) ;

	virtual void TimeWork();
	virtual void NoopWork();

	// ��ַ������ύ��Ϣ
	virtual bool HandleMsgData(const string userid, const char *data, int len);
private:
	void HandleOnlineUsers();
	void HandleOfflineUsers();
	void registerTerm(); //ģ���ն�ע��

	void sendDataToUser(const User &user, const map<string, vector<unsigned char> > &msgs);

	// ���ģ���ն�
	bool addActiveUser(const string &userid, const string &macid);
private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// �����߳���
	unsigned int 		_thread_num ;
	// ���ݷְ�����
	C808Spliter         _spliter808 ;
	// �����û�����
	OnlineUser          _online_user;
	// ��������������
	string              _corp_data_path;
	//
	Protocol            _protocol;
	//
	TimeQueue<string, Gb808WaitReply> _gb808WaitReply;
	//
	TimeQueue<string, MultiMediaData> _multiMediaData;
	//
	TimeQueue<string, InnerWaitReply> _innerWaitReply;
};

#endif//_WAS_CLIENT_
