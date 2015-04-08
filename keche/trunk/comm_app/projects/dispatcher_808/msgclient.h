/**********************************************
 * msgclient.h
 *
 *  Created on: 2011-07-28
 *    Author:   humingqing
 *    Comments: ʵ������Ϣ��������ͨ���Լ�����ת��
 *********************************************/

#ifndef __MsgCLIENT_H__
#define __MsgCLIENT_H__

#include "interface.h"
#include <BaseClient.h>
#include <time.h>
#include <interpacker.h>
#include <set>
#include <string>
using namespace std ;

class MsgClient : public BaseClient , public IMsgClient
{

public:
	MsgClient( void ) ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	
public:
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½��Ϣ����
	virtual int  build_login_msg(User &user, char *buf, int buf_len);
	//
	virtual bool HandleMsgData(const char *data, int len);

protected:
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;

private:
	// ����ָ��
	ISystemEnv  *	_pEnv ;
	// �ڲ�Э��ְ�����
	CInterSpliter   _packspliter;
};

#endif /* LISTCLIENT_H_ */
