/*
 * usermgr.h
 *
 *  Created on: 2011-11-28
 *      Author: humingqing
 */

#ifndef __USERMGR_H__
#define __USERMGR_H__

#include <OnlineUser.h>

class CUserMgr
{
	typedef std::map<int,string>   CMapCode ;
public:
	CUserMgr(){}
	~CUserMgr(){}

	//0 success; -1,���û����Ѿ�����,�Ҳ������Ƿ����ߡ�
	bool AddUser(const string &user_id, User &user) {
		if ( ! _onlineuser.AddUser( user_id, user )  ) {
			// ���ʧ��ֱ�ӷ���
			return false ;
		}
		// ��ӵ��������Ӧ�û���
		AddMapCode( user_id, user ) ;

		return true ;
	}

	// ȡ���û�ͨ��FD
	User GetUserBySocket( socket_t *sock ) {
		// ȡ���û�ͨ���û���fd
		return _onlineuser.GetUserBySocket( sock ) ;
	}

	// ͨ���û�ID��ȡ���û�
	User GetUserByUserId(const string &user_id){
		// ȡ���û�ͨ���û�ID
		return _onlineuser.GetUserByUserId( user_id ) ;
	}

	// ͨ��������ȡ���û�
	User GetUserByAccessCode( int accesscode ){
		User user ;
		string key = GetMapUserId( accesscode ) ;
		if ( key.empty() ) {
			return user ;
		}
		return _onlineuser.GetUserByUserId( key ) ;
	}

	// ɾ���û�ͨ��FD
	void DeleteUser( socket_t *sock ){
		// ɾ��������Ĺ�ϵ
		User user = _onlineuser.DeleteUser( sock ) ;
		if ( user._user_id.empty() )
			return ;
		DelMapCode( user._access_code ) ;
	}

	// ɾ���û�
	void DeleteUser(const string &user_id){
		// ɾ���������ϵ
		User user =  _onlineuser.DeleteUser( user_id ) ;
		if ( user._user_id.empty() )
			return ;
		DelMapCode( user._access_code ) ;
	}

	// ȡ�������û�
	vector<User> GetOnlineUsers(){
		// ȡ�������û�
		return _onlineuser.GetOnlineUsers() ;
	}

	// ȡ�õ�ǰ�����û�
	vector<User> GetOfflineUsers(int timeout){
		vector<User> vec = _onlineuser.GetOfflineUsers(timeout) ;
		if ( vec.empty() ) {
			return vec;
		}

		for ( int i = 0; i < (int)vec.size(); ++ i ) {
			DelMapCode( vec[i]._access_code ) ;
		}
		return vec ;
	}

	// �����û���״̬
	bool SetUser(const string &user_id,User &user){
		if ( user._user_state == User::OFF_LINE ) {
			// ��Ҫ��ӽ�����Ĺ�ϵ
			AddMapCode( user_id, user ) ;
		}
		return _onlineuser.SetUser( user_id, user ) ;
	}
private:
	// ��ӽ�����
	void AddMapCode( const string &userid, const User &user ){
		share::Guard g( _mutex ) ;
		if ( user._access_code <= 0 )
			return ;

		CMapCode::iterator it = _mapcode.find( user._access_code ) ;
		if ( it == _mapcode.end() ) {
			_mapcode.insert( make_pair( user._access_code, userid ) ) ;
		} else {
			it->second = userid ;
		}
	}

	// ɾ��������
	void DelMapCode( const int accesscode ){
		share::Guard g( _mutex ) ;

		CMapCode::iterator it = _mapcode.find( accesscode ) ;
		if ( it == _mapcode.end() )
			return ;
		_mapcode.erase( it ) ;
	}

	// ���ݽ�����ȡ��KEYID
	const string GetMapUserId( const int accesscode ){
		share::Guard g( _mutex ) ;
		CMapCode::iterator it = _mapcode.find( accesscode ) ;
		if ( it == _mapcode.end() )
			return "";
		return it->second ;
	}

private:
	// �����û��б�
	OnlineUser   _onlineuser;
	// �������Ӧ��ϵ
	CMapCode   	 _mapcode ;
	// ��������
	share::Mutex _mutex ;
};

#endif /* USERMGR_H_ */
