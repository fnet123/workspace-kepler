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

#include <std.h>
#include "interface.h"
#include <DataPool.h>
#ifdef _GPS_STAT
#include <gpsstat.h>
#endif

class CCConfig ;
class CSystemEnv : public ISystemEnv
{
	// �����û�����
	class CSequeueGen
	{
	public:
		CSequeueGen() {}
		~CSequeueGen() {}

		void ResetSequeue( const string &seq ) {
			share::Guard g( _mutex ) ;
			map<string,unsigned short>::iterator it = _map_seq.find(seq) ;
			if ( it != _map_seq.end() ) {
				it->second = 0 ;
			}
		}

		unsigned short  GetSequeue( const string &seq , int len ) {
			share::Guard g( _mutex ) ;

			unsigned short seq_id = 0 ;

			map<string,unsigned short>::iterator it = _map_seq.find(seq) ;
			if ( it == _map_seq.end() ) {
				seq_id = seq_id + len ;
				_map_seq.insert( pair<string,unsigned short>( seq, seq_id ) ) ;
			} else {
				seq_id = it->second ;
				seq_id = seq_id + len ;
				if ( seq_id >= 0xffff ) {
					seq_id = len ;
				}
				it->second = seq_id ;
			}
			return seq_id ;
		}

	private:
		share::Mutex  			    _mutex ;
		map<string,unsigned short>  _map_seq ;
	};
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *userfile,const char *logname);

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
	// ȡ���û�����
	unsigned short  GetSequeue( const char *key , int len = 1 )  { return _seq_gen.GetSequeue(key, len);  }
	// �����û�����
	void ResetSequeue( const char *key ) { _seq_gen.ResetSequeue(key); }

	// ȡ��Msg�������
	IClient * GetMsgClient( void ) { return _msg_client; }
	// ȡ��WAS����
	IServer * GetClientServer( void ) { return _was_server; }
	// ȡ��RedisCache
	IRedisCache * GetRedisCache( void ) { return _rediscache; }

#ifdef _GPS_STAT
	// ���ﴦ��GPS������ͳ�Ƽ�����
	void AddGpsCount( unsigned int count )  { _gpsstat.Add(count); 			}
	void SetGpsState( bool enable ) 		{ _gpsstat.SetEnable(enable) ;  }
	bool GetGpsState( void ) 				{ return _gpsstat.Enable(); 	}
	int  GetGpsCount( void )				{ return _gpsstat.Size(); 		}
#endif

private:
	// ��ʼ����־ϵͳ
	bool InitLog(const char *logpath,const char *logname);

private:
	// ���ݻ���ض���
	CacheDataPool	   _cache_pool ;
	// �����ļ���
	CCConfig		  *_config ;
	// �û��ļ�·��
	std::string 	   _user_file_path ;
	// ��Ϣ���Ŀͻ���
	IClient		      *_msg_client ;
	// WAS�������Ϳͻ���
	IServer       	 * _was_server ;
	//
	IRedisCache      * _rediscache ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// �û�����������
	CSequeueGen		   _seq_gen ;

#ifdef _GPS_STAT
	// ͳ��λ�������ϱ�
	CGpsStat 		   _gpsstat ;
#endif
};

#endif
