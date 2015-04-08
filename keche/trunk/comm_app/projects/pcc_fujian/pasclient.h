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
#include <protocol.h>
#include "pconvert.h"

class PasClient : public BaseClient , public IPasClient
{
	// �ְ�����
	class CPackSpliter : public IPackSpliter
	{
	public:
		CPackSpliter() {}
		virtual ~CPackSpliter() {}
		// �ְ�����
		virtual struct packet * get_kfifo_packet( DataBuffer *fifo ) ;
		// �ͷ����ݰ�
		virtual void free_kfifo_packet( struct packet *packet ) {
			free_packet( packet ) ;
		}
	};

public:
	PasClient( PConvert *convert ) ;
	virtual ~PasClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ
	virtual bool Start( void ) ;
	// ֹͣ
	virtual void Stop( void ) ;
	// ��PAS������
	virtual void HandleData( const char *data, int len ) ;

	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();

private:
	// ����ָ�봦��
	ISystemEnv  *		_pEnv ;
	// ���ݷְ�����
	CPackSpliter        _packspliter;
	// ���һ��������ʱ��
	time_t				_last_noop ;
	// Э��ת������
	PConvert		   *_convert ;
};

#endif /* LISTCLIENT_H_ */
