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
#include <OnlineUser.h>
#include <time.h>
#include <Session.h>
#include <qstring.h>
#include <interpacker.h>
#include <filecache.h>

class CPicClient ;
class MsgClient :
	public BaseClient , public IMsgClient, public IOHandler
{
public:
	MsgClient() ;
	virtual ~MsgClient() ;
	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// �ϴ�����
	virtual void HandleData( const char *data, int len , bool pic ) ;

public:
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½��Ϣ����
	virtual int  build_login_msg(User &user, char *buf, int buf_len);
	// �ص������ļ�����������
	virtual int HandleQueue( const char *sid , void *buf, int len , int msgid = 0 ) ;

protected:
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	// ���������û�
	void HandleOfflineUsers( void ) ;
	// ���������û�
	void HandleOnlineUsers(int timeval) ;

private:
	// ����ָ��
	ISystemEnv  *		  _pEnv ;
	// �����û�����
	OnlineUser   		  _online_user;
	// �û�����
	CInterSpliter   	  _packspliter ;
	 // �ļ����ݻ���
	CFileCache	    	  _filecache ;
	// �Ƿ�������ͬ��
	bool 				  _enable ;
	// ͼƬ�������
	CPicClient			 *_picclient;
};

#endif /* LISTCLIENT_H_ */
