/**************************************************************************************
 * ClientAccessServer.h
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments: 2011-07-24
 *     humingqing ������⣬�������ݴ���Ӧ������ʱʵʱ����ʽ��
 *     ÿһ�������ϵ����ⶼӦ��׼ȷ��Ӧ�������Դͷ����,����Ӧ�ý��������ڻ����У�
 *     �����������������������һֱ�����ͻᵼ�����ݲ��ϵĻ�ѹ����������ձ�Ȼ�ᵼ��ϵͳ����
 *     ���������Ӧ����ô����,�����޸�ͨ�������߳����������ݵĴ���
 **************************************************************************************/

#ifndef __CLIENTACCESSSERVER_H__
#define __CLIENTACCESSSERVER_H__

#include "interface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseServer.h>
#include <list>
#include <protocol.h>
#include <Session.h>
#include "msguser.h"
#include <statflux.h>
#include <interpacker.h>

#ifndef BUF_LEN
#define BUF_LEN 		4096
#endif

class ClientAccessServer: public BaseServer ,
				public IMsgClientServer , public ISessionNotify
{
public:
	ClientAccessServer() ;
	~ClientAccessServer( void ) ;

	//////////////////////////ICasClientServer//////////////////////////////
	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	////////////////////////////////////////////////////////////

	virtual void on_data_arrived( socket_t *sock, const void *data, int len);
	virtual void on_dis_connection( socket_t *sock);
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	virtual bool HasLogin(const string &user_id);

	// ���û����ߺ����ߵ�֪ͨ
	virtual void NotifyChange( const char *key, const char *val , const int op ) ;
	// ȡ���ڳ�������ѹ��
	virtual int  GetOnlineSize( void ) ;
	// ��������û��б�
	virtual void AddNodeUser( const char *user, const char *pwd ) ;
	// ��������
	virtual bool Deliver( socket_t *sock, const char *data, int len )  ;
	// ��������
	virtual bool DeliverEx( const char *userid, const char *data, int len ) ;
	// �ر�����
	virtual void Close( socket_t *sock ) ;
private:
	// �����ڲ�����
	void HandleInterData( socket_t *sock, const char *data, int len, bool flag );

private:
	// �������ݴ���
	bool SendDataToUser(const char *data, int len , const string &userid);
	// ����û��Ƿ��½
	int  check_user_info(const string &user_name,const string &user_password);
	// ���¼��û�����
	bool ReloadUserInfo();
	// ���͵������û�
	bool SendAllUser( const char *data, int len ) ;
	// �����û�����
	bool SendUserData( User &user, const char *data, int len ) ;
	// �׷����ݴ���
	void DispatchData( User &user, unsigned int cmd, InterData &data ) ;

private:
	// ��������ָ��
	ISystemEnv			  *_pEnv ;
	// ��ǰ�������û�
	IGroupUserMgr 		  *_pusermgr;
	// MSG��ǰ���õ��û�
	CMsgUser			   _msg_user ;
	// �����߳���
	unsigned  int		   _thread_num ;
	// �û�����·��
	std::string 		   _user_file ;
	// ���ݷְ�����
	CInterSpliterEx		   _pack_spliter ;
	// Session����
	CSessionMgr		   	   _session ;
	// �ڵ�ŵ�ID
	unsigned int 		   _nodeid ;
	// ��������ͳ��
	CStatFlux			   _recvstat ;
	// ���ʱʱ��
	unsigned int 		   _max_timeout ;
	// λ���ϱ�ͳ��
	CStatFlux			   _reportstat ;
	// �Ƿ����洢����
	bool 				   _enbale_save ;
	// �Ƿ����������
	bool 				   _enable_plugin ;
};

#endif /* CLIENTACCESSSERVER_H_ */
