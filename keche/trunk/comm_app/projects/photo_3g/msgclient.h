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

#include "../tools/tqueue.h"

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
	virtual void HandleData( const InnerMsg & msg );

protected:
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	//
	bool parseInnerMsg(const string &str, InnerMsg &msg);
	//
	bool spellInnerMsg(const InnerMsg &msg, string &str);
	//
	string getInnerMsgArg(const InnerMsg &msg, const string &key);
	//
	bool get3gby2g(const string &sim2g, string &sim3g, string &oem);

private:
	// ����ָ��
	ISystemEnv  *	_pEnv ;
	// �ڲ�Э��ְ�����
	CInterSpliter   _packspliter;
	// �ֻ������ѯ�ӿ�
	string          _httpUrl;
	//
	int             _thread_num;
	//
	TimeQueue<string, int> _timeQueue;
};

#endif /* LISTCLIENT_H_ */
