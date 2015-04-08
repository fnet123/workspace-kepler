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

#include <map>
#include "interface.h"
#ifdef _GPS_STAT
#include <gpsstat.h>
#endif

class CCConfig ;
class CSystemEnv : public ISystemEnv
{
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *userfile  , const char *logname ) ;

	// ��ʼϵͳ
	bool Start( void ) ;

	// ֹͣϵͳ
	void Stop( void ) ;

	// ȡ������ֵ
	bool GetInteger( const char *key , int &value ) ;
	// ȡ���ַ���ֵ
	bool GetString( const char *key , char *value ) ;
	// ȡ���û�����λ��
	const char * GetUserPath( void )  { return _user_file_path.c_str(); }
	// ȡ��Msg����
	IMsgClientServer * GetMsgClientServer( void ){ return _msg_server; } ;
	// ȡ���û��������
	IGroupUserMgr * GetUserMgr( void ) { return _usermgr ; }
	// ȡ�÷�������
	IPublisher  * GetPublisher( void ) { return _publisher; }
	// ȡ��RedisCache
	IRedisCache * GetRedisCache( void ) { return _rediscache; }
	// ȡ�ö�Ӧ�������Ķ���
	IMsgHandler * GetMsgHandler( unsigned int id ) { return _msghandler[id]; }
	// ȡ������ͬ��msgclient
	IMsgClient  * GetMsgClient( void ) { return _msgclient; }

#ifdef _GPS_STAT
	// ���ﴦ��GPS������ͳ�Ƽ�����
	void AddGpsCount( unsigned int count )  { _gpsstat.Add(count); 			}
	void SetGpsState( bool enable ) 		{ _gpsstat.SetEnable(enable) ;  }
	bool GetGpsState( void ) 				{ return _gpsstat.Enable(); 	}
	int  GetGpsCount( void )				{ return _gpsstat.Size(); 		}
#endif

private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath , const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �û��ļ�·��
	std::string 	   _user_file_path ;
	// Msg�������Ϳͻ���
	IMsgClientServer * _msg_server ;
	// �ڵ����ͻ���
	INodeClient		 * _node_client ;
	// �û��������
	IGroupUserMgr    * _usermgr ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// ��������ģ��
	IPublisher		 * _publisher ;
	// RedisCache
	IRedisCache      * _rediscache ;
#ifdef _GPS_STAT
	// ͳ��λ�������ϱ�
	CGpsStat 		   _gpsstat ;
#endif
	// ����������
	IMsgHandler      * _msghandler[MAX_MSGHANDLE] ;
	// ͬ�����ķ�����
	IMsgClient       * _msgclient ;
};

#endif
