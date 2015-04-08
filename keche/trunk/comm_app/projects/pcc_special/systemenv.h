/**********************************************
 * systemenv.h
 *
 *  Created on: 2011-07-24
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments: ���������࣬��Ҫ������Ҫ�����Ķ��󣬶�������󽻻�֮����н磬
 *    �������κ���������֮��Ľ���������ͨ�������������ʵ��ֱ�ӽ�����ʹ������֮��͸������
 *    Ҳʹ�ýṹ����������ÿһ������ʵ�֣�������ʵ��Init Start Stop����������Ҫʵ��ϵͳ
 *    ֮���ͳһ�淶������
 *********************************************/

#ifndef __SYSTEMENV_H__
#define __SYSTEMENV_H__

#include <Mutex.h>
#include "interface.h"
#include "servicecaller.h"

class CUserLoader ;
class CCConfig ;
class CSystemEnv : public ISystemEnv
{
	class CSequeue
	{
	public:
		CSequeue():_seq_id(0) {}
		~CSequeue() {}

		// ȡ������
		unsigned int get_next_seq( void ) {
			share::Guard g( _mutex ) ;
			if ( _seq_id >= 0xffffffff ) {
				_seq_id = 0 ;
			}
			return ++ _seq_id ;
		}

	private:
		// ����������
		share::Mutex  _mutex ;
		// ����ID��
		unsigned int  _seq_id ;
	};
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath, const char *userfile, const char *logname ) ;

	// ��ʼϵͳ
	bool Start( void ) ;

	// ֹͣϵͳ
	void Stop( void ) ;

	// ȡ������ֵ
	bool GetInteger( const char *key , int &value ) ;
	// ȡ���ַ���ֵ
	bool GetString( const char *key , char *value ) ;
	// ȡ�û�������
	void GetCacheKey( unsigned int seq, char *key ) ;
	// ȡ�����
	unsigned int GetSequeue( void ) { return _seq_gen.get_next_seq(); };
	// ��������
	bool SetNotify( const char *tag, IUserNotify *notify );
	// �����û�����
	bool LoadUserData( void ) ;
	// ȡ�ü�����Կ
	bool GetUserKey( int accesscode, int &M1, int &IA1, int &IC1 ) ;
	// ����Ự����
	void ClearSession( const char *key ) ;

	// ʹ��macid��ȡ��Ҫת���ļ��ƽ̨
	bool getChannels(const string &macid, set<string> &channels);
	// ��ȡ����macid������msg����
	bool getSubscribe(list<string> &macids);
	// ʹ�ó��ƺ����ȡmacid
	bool getMacid(const string &plate, string &macid);

	// ȡ��PAS����
	IPasClient * GetPasClient( void ) { return _pas_client; }
	// ȡ��MSG Client ����
	IMsgClient * GetMsgClient( void ) { return _msg_client; }
	// ȡ��MsgCache����
	IMsgCache  * GetMsgCache( void ) { return _msg_cache ; }
	// ȡ��PCC������
	IPccServer * GetPccServer( void ){ return _pcc_server; }
	// ȡ��RedisCache
	IRedisCache * GetRedisCache( void ) { return _rediscache; }

	virtual const char * GetUserPath( void ) { return ""; }
private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath, const char *logname ) ;
	// ��ʼ���û�����
	bool InitUser( void ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// ����Э�������
	IPasClient 		 * _pas_client ;
	// ʵ�ּ��ƽ̨ת��Э��
	IMsgClient		 * _msg_client ;
	// ���ݻ���
	IMsgCache		 * _msg_cache ;
	// PCC����˿�
	IPccServer		 * _pcc_server ;
	// ��Ų�����
	CSequeue		   _seq_gen ;
	// �û����ݼ��ض���
	CUserLoader		 * _userloader ;
	// �û������ļ�·��
	std::string 	   _user_file ;
	// �����ļ�·��
	std::string        _dmddir;
	// ҵ���첽http�ص�
	CServiceCaller     _srvCaller ;
	// RedisCache
	IRedisCache      * _rediscache ;
};

#endif
