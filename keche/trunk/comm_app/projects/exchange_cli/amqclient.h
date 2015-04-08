/**********************************************
 * amqclient.h
 *
 *  Created on: 2014-05-19
 *    Author:   ycq
 *********************************************/

#ifndef _AMQCLIENT_H_
#define _AMQCLIENT_H_ 1

#include <Thread.h>
#include "interface.h"

#include <string>
using std::string;

class AmqClient : public IAmqClient,  public share::Runnable
{

public:
	AmqClient( void ) ;
	virtual ~AmqClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();

protected:
	virtual void run(void *param);
private:
	// ����ָ��
	ISystemEnv  *_pEnv ;
	// mq��������ַ
	string      _brokerUrl;
	// mq��������
	string      _topicFile;
	// ����mq���ݹ����߳�
	share::ThreadManager  	_recv_thread ;
};

#endif//_AMQCLIENT_H_
