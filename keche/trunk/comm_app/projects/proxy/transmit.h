/***********************************************************************
 ** Copyright (c)2009,����ǧ���Ƽ��������޹�˾
 ** All rights reserved.
 **
 ** File name  : CountryServer.h
 ** Author     : lizp (lizp.net@gmail.com)
 ** Date       : 2010-1-7 ���� 02:29:25
 ** Comments   : ����ȫ��ƽ̨��
 ** 2011-07-24 humingqing
 *    �޸ģ�ͳһ���������󲿼��У���Ҫȥ��������У�ʵ���ɽ����߳��������ݴ���
 ***********************************************************************/

#ifndef _TRANSMIT_SERVICE_H
#define _TRANSMIT_SERVICE_H

#include "interface.h"
#include "ProtoParse.h"
#include "OnlineUser.h"
#include "crc16.h"
#include "BaseTools.h"
#include <BaseServer.h>
#include "filecacheex.h"
#include "datastat.h"

class Transmit :
	public BaseServer , public IMasServer, public IOHandler
{
public:
	Transmit();
	~Transmit() ;

	virtual bool Init(ISystemEnv *pEnv ) ;
	virtual bool Start( void ) ;
	virtual void Stop( void ) ;
	// �׷��ϴ�MAS����
	virtual void HandleMasUpData( const char *data, int len ) ;
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	virtual void on_dis_connection( socket_t *sock );
	// �������ݵĻص�����
	virtual int HandleQueue( const char *uid , void *buf, int len , int msgid = 0 ) ;

protected:
	bool ConnectServer(User &user, unsigned int timeout = 5);

private:

	virtual void TimeWork() ;
	virtual void NoopWork() ;

	// �������´���ѭ���������
	bool SendCrcData( socket_t *sock, const char* data, int len ) ;
	// ���ܽ����㷨
	bool EncryptData( unsigned char *data, unsigned int len , bool encode ) ;
	// ����MAS������
	bool SendMasData( const char *data, int len ) ;

private:
	ProtoParse 		_proto_parse;

	User 			_client;
	User 			_server;  //�Է������������Ի���user��������listen��User

	int         	_verify_code;
	unsigned int 	_access_code;
	unsigned int 	_user_name;
	string 			_user_password;
	string 			_down_ip ;

	unsigned int 	_M1 ;
	unsigned int 	_IA1 ;
	unsigned int    _IC1 ;

	// �ļ��������
	CFileCacheEx	_filecache ;
	// ��������
	ISystemEnv     *_pEnv ;
	// �·�����ͳ��
	CStat			_recvstat ;
	// ������ͳ��
	CStat		    _sendstat ;
};

#endif

