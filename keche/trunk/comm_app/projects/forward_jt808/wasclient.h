#ifndef _WAS_CLIENT_
#define _WAS_CLIENT_ 1

#include "protocol.h"

#include <interface.h>
#include <BaseClient.h>
#include <OnlineUser.h>
#include <interpacker.h>

class WasClient : public BaseClient, public IWasClient
{
	typedef struct HostInfo {
		string ip;
		uint16_t port;
	} HostInfo;

	typedef struct TermInfo {
		string sim;
		string color;
		string plate;
		string termMfrs;
		string termType;
		string termID;
	} TermInfo;
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
	virtual bool HandleData(const string &sim, const uint8_t *ptr, size_t len);
	// ����ģ���ն�
	virtual bool AddTerminal(const string &sim);
	// ɾ��ģ���ն�
	virtual bool DelTerminal(const string &sim);
private:
	void HandleOnlineUsers();
	void HandleOfflineUsers();
	void registerTerm(); //ģ���ն�ע��

	// �����ն�ע����Ϣ
	bool buildTerm0100(const TermInfo &term, vector<uint8_t> &msg);
	// �����ն˼�Ȩ��Ϣ
	bool buildTerm0102(const string &userid, vector<uint8_t> &msg);
	// ����ת��Ŀ��ǰ�û���Ϣ
	void loadHost();
	// ����808����
	bool Send7ECodeData(socket_t *sock, const uint8_t *ptr, size_t len);
	// ��BCD����ȡ���ֻ���
	bool bcd2sim(uint8_t *bcd, string &sim);
	// ����jt808У����
	uint8_t get_check_sum(uint8_t *buf, size_t len);

	void sendDataToUser(const User &user, const map<string, vector<unsigned char> > &msgs);

	// ���ģ���ն�
	bool addActiveUser(const string &userid, const string &macid);
private:
	// ����ָ�봦��
	ISystemEnv  *		 _pEnv ;
	// �����߳���
	unsigned int 		 _thread_num ;
	// ���ݷְ�����
	C808Spliter          _spliter808 ;
	// �����û�����
	OnlineUser           _online_user;
	// ���ӿ���ʱ�䣬��
	unsigned int       _max_timeout;
	// ������ǰ�û������ļ�
	string                _hostfile;
	// ������ǰ�û����ò���
	map<string, HostInfo> _hostInfo;
};

#endif//_WAS_CLIENT_
