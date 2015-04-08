
#ifndef _EXCH_CLIENT_
#define _EXCH_CLIENT_ 1

#include "exchspliter.h"
#include "tqueue.h"

#include <interface.h>
#include <BaseClient.h>
#include <tchdb.h>

class ExchClient : public BaseClient, public IExchClient
{
	struct DbValue {
		uint16_t cnt;    //�ش�����
		uint8_t dat[0];  //�ش�����
	};
public:
	ExchClient();
	~ExchClient();

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
	virtual bool HandleData(uint32_t seqid, const char *data, int len);
private:
	size_t enCode(const uint8_t *src, size_t len, uint8_t *dst);
	size_t deCode(const uint8_t *src, size_t len, uint8_t *dst);
	//bool popCacheData(uint32_t seqid, vector<uint8_t> res);
private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// �����߳���
	unsigned int 		_thread_num ;
	// ���ݷְ�����
	CExchSpliter         _spliter_exch;
	// �������ݳ�ʱ�������
	TimeQueue<uint32_t>  _exch_pack_tqueue;
	// �������ݻ������
	TCHDB               *_exch_pack_tchdb;
	// ���������ش�����
	int                  _exch_pack_retries;
	// �������ݳ�ʱͬ������
	share::Mutex         _exch_pack_mutex;
};

#endif//_EXCH_CLIENT_
