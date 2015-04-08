/*
 * userloader.h
 *
 *  Created on: 2011-12-29
 *      Author: Administrator
 */

#ifndef __USERLOADER_H__
#define __USERLOADER_H__

#include <string>
#include <map>
#include <Mutex.h>
#include "interface.h"

// �û��ļ����ع������
class CUserLoader
{
	// ���ƽ̨�ӽ�����Կ����
	struct _EncryptKey
	{
		int _M1 ;   // M1��Կ
		int _IA1 ;  // IA1��Կ
		int _IC1 ;	// IC1��Կ
	};

	// �����û���Կ
	class CUserKey
	{
		typedef std::map<int, _EncryptKey>  CMapKey ;
	public:
		CUserKey() {}
		~CUserKey() {}

		// ����û���Կ
		void AddKey( int accesscode , int M1, int IA1, int IC1 ){
			share::Guard guard( _mutex ) ;
			CMapKey::iterator it = _mapkey.find( accesscode ) ;
			if ( it == _mapkey.end() ) {
				_EncryptKey key ;
				key._M1  = M1  ;
				key._IA1 = IA1 ;
				key._IC1 = IC1 ;
				_mapkey.insert( make_pair(accesscode, key) ) ;
			} else {
				it->second._M1  = M1  ;
				it->second._IA1 = IA1 ;
				it->second._IC1 = IC1 ;
			}
			//printf( "Add M1: %d, IA1: %d, IC1: %d\n" , M1, IA1, IC1 ) ;
		}

		// ɾ��KEY��ֵ
		void DelKey( int accesscode ) {
			share::Guard guard( _mutex ) ;
			CMapKey::iterator it = _mapkey.find( accesscode ) ;
			if ( it == _mapkey.end() ) {
				return ;
			}
			_mapkey.erase( it ) ;
		}

		// ȡ��KEY������
		bool GetKey( int accesscode, int &M1, int &IA1, int &IC1 ) {
			share::Guard guard( _mutex ) ;
			CMapKey::iterator it = _mapkey.find( accesscode ) ;
			if ( it == _mapkey.end() ) {
				return false ;
			}

			M1  = it->second._M1 ;
			IA1 = it->second._IA1 ;
			IC1 = it->second._IC1 ;

			return true ;
		}

	private:
		// ��Կ��Ӧ��MAP����
		CMapKey      _mapkey ;
		share::Mutex _mutex ;
	};

	typedef std::map<std::string, std::set<string> > Macid2Channel;

	typedef std::map<std::string,IUserNotify::_UserInfo>  CUserMap ;
	class CUserMgr
	{
	public:
		CUserMgr() :_notify(NULL){ }
		~CUserMgr() {}
		// ����û�����
		void Add( CUserMap &users ) ;
		// ����֪ͨ�ص�����
		void SetNotify( IUserNotify *notify ) ;
	private:
		// �û��������
		CUserMap  	 _users ;
		// �û����֪ͨ����
		IUserNotify *_notify ;
	};

	typedef std::map<std::string,CUserMap>    CGroupUsers ;
	typedef std::map<std::string,CUserMgr> 	  CGroupMap ;
public:
	CUserLoader() {};
	~CUserLoader(){};

	bool Init( ISystemEnv *pEnv );

	// ��������
	bool SetNotify( const char *tag, IUserNotify *notify ) ;
	// �����û�����
	bool LoadUser( const char *file, const char *path ) ;
	// ȡ�õ�ǰ�����������
	bool GetUserKey( int accesscode, int &M1, int &IA1, int &IC1 ) {
		// ������Կ����
		return _userkey.GetKey( accesscode, M1, IA1, IC1 ) ;
	}

	// ʹ��macid��ȡ��Ҫת���ļ��ƽ̨
	bool getChannels(const string &macid, set<string> &channels);
	// ��ȡ����macid������msg����
	bool getSubscribe(list<string> &macids);
private:
	// �����ļ�����
	bool LoadFile( const char *file, const char *path, CGroupUsers &users ) ;

	// ���ض�������
	bool loadSubscribe(const string &code, const string &path, Macid2Channel &macid2Channel);
private:
	// ��������ָ��
	ISystemEnv		   *_pEnv ;
	// �û���Կ�Ĺ�����
	CUserKey	   _userkey ;
	// �����û���ʶ���鴦��
	CGroupMap      _groupuser ;
	// �����������
	share::Mutex   _mutex ;

	// ӳ����д��
	share::RWMutex  _rwMutex;
	// ʹ��macid����ת����ͨ�������ƺ���
	Macid2Channel  _macid2channel;
};


#endif /* USERLOADER_H_ */
