/**********************************************
 * pasclient.h
 *
 *  Created on: 2014-06-30
 *      Author: ycq
 *********************************************/

#ifndef _PASCLIENT_H_
#define _PASCLIENT_H_ 1

#include "interface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseClient.h>
#include <protocol.h>

class PasClient : public BaseClient , public IPasClient
{
	// �ְ�����
	struct CPackSpliter: public IPackSpliter {
		// �ְ�����
		virtual struct packet * get_kfifo_packet(DataBuffer *fifo);
		// �ͷ����ݰ�
		virtual void free_kfifo_packet(struct packet *packet) {
			free_packet(packet);
		}
	};

public:
	PasClient();
	virtual ~PasClient();

	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv);
	// ��ʼ
	virtual bool Start(void);
	// ֹͣ
	virtual void Stop(void);
	// ��PAS������
	virtual bool HandleData(const char *data, int len);

	virtual void on_data_arrived(socket_t *sock, const void* data, int len);
	virtual void on_dis_connection(socket_t *sock);

	virtual void TimeWork();
	virtual void NoopWork();

	virtual int build_login_msg(User &user, char *buf, int buf_len);
private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// ���ݷְ�����
	CPackSpliter        _packspliter;
	// ��ܿͻ����߳���
	int                _threadnum;
};

#endif//_PASCLIENT_H_
