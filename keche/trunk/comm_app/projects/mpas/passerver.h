/**********************************************
 * PasServer.h
 *
 *  Created on: 2011-08-04
 *      Author: huminqing
 *       Email:qshuihu@gmail.com
 *    Comments: ʹ�÷ְ��ӿ�ʵ���κ�����Э��ְ�����
 *********************************************/

#ifndef _PASSERVER_H_
#define _PASSERVER_H_

#include <map>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <BaseServer.h>
#include "interface.h"
#include <OnlineUser.h>
#include <protocol.h>
#include <time.h>
#include <Session.h>
#include "pconvert.h"

#define BUF_LEN  			1024
#define USER_TIMEOUT		60*3
#define USER_WAITTIME		60

class PasServer : public BaseServer, public IPasServer
{
	class CKeyMgr
	{
		struct _Key
		{
			unsigned int M1  ;  	 // ������Կ1  M1_IA1_IC1
			unsigned int IA1 ;  	 // ������Կ2
			unsigned int IC1 ;  	 // ������Կ3
		};
		typedef std::map<unsigned int,_Key>  CMapKey ;
	public:
		CKeyMgr() ;
		~CKeyMgr() ;
		// ��ӵ�������Կ������
		void AddEncryptKey( unsigned int access, const char *key ) ;
		// ȡ�ü�����Կ
		bool GetEncryptKey( unsigned int access, unsigned int &M1 , unsigned int &IA1 , unsigned int &IC1 ) ;

	private:
		// ��ǰ��¼���ݸ���
		int 		  _size ;
		// ������ܵ�KEY
		CMapKey   	  _keys ;
		// ͬ������������
		share::Mutex  _mutex ;
	};
public:
	PasServer( PConvert *convert );
	~PasServer();

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ�߳�
	virtual bool Start( void ) ;
	// ֹͣ����
	virtual void Stop( void ) ;
	// �ͻ����ڷ׷�����
	virtual void HandlePasDown( const char *code , const char *data, const int len ) ;

protected:
	// �������ݰ�
	void HandleOnePacket( socket_t *sock,  const char *data, int data_len );

	virtual void on_data_arrived( socket_t *sock, const void* data, int len );
	virtual void on_dis_connection( socket_t *sock );
	virtual void on_new_connection( socket_t *sock, const char* ip, int port );

private:
	bool ConnectServer( User &user, unsigned int timeout );

	virtual void TimeWork() ;
	virtual void NoopWork() ;

	// ���ݼӽ���
	bool EncryptData( unsigned char *data, unsigned int len , bool encode ) ;
	// ����ָ���Ľ������û�
	bool SendDataToPasUser( const string &access_code ,const char *data, int len ) ;
	// �����û�
	void HandleOnlineUsers(int timeval);
	// �����û�
	void HandleOfflineUsers();
	// ȡ�ó��ƺ��ֻ���֮���Ӧ��ϵ
	bool GetMacIdByVechicle( const char *vechicle, unsigned char color, string &macid ) ;
	// ����û��Ƿ�Ϸ�
	int  CheckLogin( unsigned int accesscode, const char *username=NULL, const char *password=NULL ) ;
	// ����5B���������
	bool Send5BCodeData( socket_t *sock, const char *data, int len ) ;
	// ����ѭ��
	void ResetCrcCode( char *data, int len ) ;
	// ���ͼ�������
	bool SendCrcData( socket_t *sock, const char* data, int len) ;

private:
	time_t				_last_handle_user_time ;
	// �����û�����
	OnlineUser 			_online_user;
	// �����߳�
	unsigned int 		_thread_num ;
	// ����������IP
	std::string 		_ip ;
	// �˿ڴ���
	unsigned short      _port ;
	// ��������ָ��
	ISystemEnv		   *_pEnv ;
	// ������
	unsigned int 	    _verify_code ;
	// ���ƺŵ��ֻ��ŵ�ת����ϵ
	CSessionMgr		    _carnum2phone ;
	// Э��ת������
	PConvert		   *_convert ;
	// ͼƬ����·��
	string 				_rootpath ;
	// ���������Կ
	CKeyMgr				_keymgr ;
};

#endif


