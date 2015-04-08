/*
 * pushserver.h
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  �������ͷ���
 */

#ifndef __PUSHSERVER_H__
#define __PUSHSERVER_H__

#include <interface.h>
#include <BaseServer.h>
#include <interpacker.h>
#include "subscribe.h"
#include "proto_convert.h"
#include <set>
#include <vector>
#include <string>
//�û��������ӵ�ʱ����û�������ϴ�������Ϊ�ǳ�ʱ�ˡ�
#define SOCK_TIME_OUT  (1 * 30)

class PushServer :
	public BaseServer , public IPushServer
{
public:
	PushServer() ;
	~PushServer() ;
	//////////////////////////ICasClientServer//////////////////////////////
	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	////////////////////////////////////////////////////////////
	virtual void on_data_arrived( socket_t *sock, const void *data, int len);

	virtual void on_new_connection( socket_t *sock, const char* ip, int port);

	virtual void on_dis_connection( socket_t *sock );
	// ��MSGת������������
	virtual void HandleData( const char *data, int len ) ;

	virtual void TimeWork();

	virtual void NoopWork();

	// ��MSG �����������ݽ���ת��
	void DispathMsgData(const string &mac_id, const char *data, int len);

	void NotifyMsgData(const string &mac_id, const char *data, int len);

	void SendDataToUser(const string &user_id, char *data, int len);

private:
	// ���ͼ�������
	bool SendDataEx( socket_t *sock, const char *data, int len ) ;
	// ���ض��Ĺ�ϵ
	void LoadSubscribe( const char *key, std::vector<std::string> &vec, std::set<std::string> &kset ) ;

private:
	// ��������ָ��
	ISystemEnv			  *_pEnv ;
	// �����߳���
	unsigned  int		   _thread_num ;
	// ���ݷְ�����
	CInterSpliter		   _pack_spliter ;

	//PushUserHandler        _push_user_handler;

	Subscribe              _subs_info;

	CInterProtoParse       _inter_parse;

	CNewProtoParse         _new_parse;

	ConvertProto           _proto_convert;
	// �����û�����
	OnlineUser   		   _online_user;
};


#endif /* PUSHSERVER_H_ */
