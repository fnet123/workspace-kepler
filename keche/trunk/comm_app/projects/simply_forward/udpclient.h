#ifndef _SF_UDP_CLIENT_
#define _SF_UDP_CLIENT_ 1

#include "sfpacker.h"

#include "interface.h"
#include <BaseClient.h>
#include <OnlineUser.h>
#include <Mutex.h>

class UdpClient : public BaseClient , public IUdpClient
{
	// ����ָ��
	ISystemEnv  *	_pEnv ;
	// �����߳���
	unsigned int    _thread_num ;
	// ����û����ʱ��
	unsigned int    _max_timeout;
	// �ڲ�Э��ְ�����
	CSFSpliter      _spliter;
	// �����û�����
	OnlineUser      _online_user;
	//
	map<string, vector<char> > _cache;
	//
	share::Mutex    _mutex;
public:
	UdpClient( ISystemEnv *pEnv ) ;
	virtual ~UdpClient() ;

	// ��ʼ��
	virtual bool Init(void);
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();

	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½��Ϣ����
	virtual bool ConnectServer(User &user, unsigned int timeout);

	// ��������ͨ��
	virtual bool AddChannel(IUdpServer *svr, const string &userid, const char *ip, int port);
	// ɾ������ͨ��
	virtual bool DelChannel(const string &userid);
	// �ύ���ݴ���
	virtual bool HandleData(const string &userid, const void *data, int len);
private:
	// �������״̬
	void HandleOfflineUsers(void);
};

#endif//_SF_UDP_CLIENT_
