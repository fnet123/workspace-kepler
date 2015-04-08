#ifndef _SF_TCP_SERVER_
#define _SF_TCP_SERVER_ 1

#include "sfpacker.h"

#include "interface.h"
#include <BaseServer.h>
#include <OnlineUser.h>

#include "../tools/tqueue.h"

class TcpServer : public BaseServer, public ITcpServer
{
	// ����ָ�봦��
	ISystemEnv  *		_pEnv;
	// �����߳���
	unsigned int 		_thread_num;
	// ����û����ʱ��
	unsigned int 		_max_timeout;
	// ���ݷְ�����
	CSFSpliter          _spliter;
	// �����û�����
	OnlineUser          _online_user;
	// ת����Ŀ¼��ַ
	vector<string>      _dstAddrs;
public:
	TcpServer(ISystemEnv *pEnv);
	~TcpServer();

	virtual bool Init(int port, const vector<string> &addrs);
	// �����ڵ�ͻ�
	virtual bool Start( void );
	// ����STOP����
	virtual void Stop( void );

	virtual void on_data_arrived(socket_t *sock, const void* data, int len);
	virtual void on_dis_connection(socket_t *sock);
	virtual void on_new_connection(socket_t *sock, const char* ip, int port);

	virtual void TimeWork();
	virtual void NoopWork();

	// ɾ������ͨ��
	virtual bool ChkChannel(const string &userid);
	// ��ַ������ύ��Ϣ
	virtual bool HandleData(const string &userid, const void *data, int len);
private:
	// �������״̬
	void HandleOfflineUsers();
};

#endif//_SF_TCP_SERVER_
