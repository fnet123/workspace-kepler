/**********************************************
 * PasServer.h
 *
 *  Created on: 2010-10-11
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments: 2011-07-24 humingqing
 *    �޸ģ�ͳһ���������󲿼��У���Ҫȥ��������У�ʵ���ɽ����߳��������ݴ�������ʡ������Ҫ���м价�ڣ����Ч�ʣ�
 *    Ҳ��һ��ϵͳ���ʱ���迼�ǣ�һ����ϵͳ�����ֲ����ֹ���ʱӦ���ܼ�ʱ�ķ�����Դͷ��Դͷ�������ô�ʩ�������һ����׳��ϵͳ��ơ�
 *********************************************/

#ifndef __PASSERVER_H_
#define __PASSERVER_H_

#include "interface.h"
#include <NetHandle.h>
#include "ProtoParse.h"
#include <OnlineUser.h>
#include <crc16.h>
#include <BaseServer.h>
#include "filecacheex.h"
#include "corpinfohandle.h"
#include "logincheck.h"
#include "datastat.h"

class PasServer :
	public BaseServer, public IPasServer,public IOHandler
{
public:
	PasServer();
	~PasServer();
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv ) ;
	// ��ʼ�߳�
	virtual bool Start( void ) ;
	// ֹͣ����
	virtual void Stop( void ) ;
	// �ͻ����ڷ׷�����
	virtual void HandleClientData( const char *data, const int len ) ;
	// ���յ�����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ�����
	virtual void on_dis_connection( socket_t *sock );
	// �������ݵĻص�����
	virtual int HandleQueue( const char *id , void *buf, int len , int msgid = 0 ) ;

private:
	// �������ݰ�
	void HandleOnePacket( socket_t *sock,  const char *data, int data_len );
	// ���ݽ����뷢�Ͷ�Ӧ���û�
	bool SendDataToPasUser( unsigned int accesscode, const char *data, int len );

	bool ConnectServer(User &user, unsigned int timeout);

	virtual void TimeWork() ;
	virtual void NoopWork() ;
	// ���ܻ��������
	bool EncryptData( unsigned char *data, unsigned int len , bool encode ) ;
	// ���������5Bת�������
	bool Send5BCodeData( socket_t *sock, const char *data, int len ) ;
	// �������´���ѭ���������
	bool SendCrcData( socket_t *sock, const char* data, int len) ;

	void HandleOnlineUsers(int timeval);
	void HandleOfflineUsers();
	// ����CRC��CODE
	void  ResetCrcCode( char *data, int len ) ;

private:
	// ��������ָ��
	ISystemEnv		   * _pEnv ;
	// Э�����
	ProtoParse 			 _proto_parse;
	// �����û�����
	OnlineUser 			 _online_user;
	// �����ʵ�ʱ��
	time_t 				 _last_check;
	// ������
	unsigned int 		 _verify_code;
	// �̸߳���
	unsigned int 		 _thread_num ;
	// ��������
	int 				 _max_send_num ;
	// �ļ����ݻ���
	CFileCacheEx		 _filecache;
	// ��ų���������Լ�ͳ��ָ��Ķ���
	CorpInfoHandle 		 _corp_info_handler;
	// �����û���Ϣ�Ķ���
	LoginCheck 			*_login_check;
	// ��������ͳ��
	CDataStat			 _datastat ;
	// ������ͳ��
	CStat				 _allstat ;
};

#endif


