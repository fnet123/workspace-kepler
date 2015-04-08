/**********************************************
 * OnlineUser.h
 *
 *  Created on: 2011-6-8
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *    	�Ż������㷨���޸�ԭ��MAP������MAP�ı����ٶȱ����ͱȽ�����
 *    	�������Ч���ǱȽϿ��
 *********************************************/

#ifndef ONLINEUSER_H_
#define ONLINEUSER_H_

#include <string>
#include <iostream>
#include <stdarg.h>
#include <map>
#include <time.h>
#include <Mutex.h>
#include <vector>
#include <SocketHandle.h>
#include <TQueue.h>

using namespace std;

#pragma pack(1)

#define AlwaysReConn 0
#define ReConnTimes  1

class ConnectInfo
{
public:
	unsigned char keep_alive;
	time_t last_reconnect_time;
	unsigned short reconnect_times;  //ʣ�������Ĵ�����
	int timeval; //������ʱ������

	ConnectInfo()
	{
		reset();
	}

	void reset()
	{
		keep_alive = ReConnTimes;
		last_reconnect_time = 0;
		timeval = 0;
		reconnect_times = 1;
	}
};

class User
{
public:
	enum UserState{OFF_LINE,WAITING_RESP,ON_LINE,DISABLED};
	enum SocketType{TcpServer,TcpClient,TcpConnClient,UdpServer,UdpClient};

	string 	_user_name ;
	string 	_user_pwd ;
	string 	_user_id;
	string 	_user_type;
	unsigned int _access_code;
	UserState _user_state;

	socket_t  *_fd;
	SocketType _socket_type;

	string _ip;
	unsigned short _port;

	time_t _login_time;
	time_t _last_active_time;

	ConnectInfo _connect_info;
	unsigned int _msg_seq;

	void *_ext_ptr;  // �û����ݶ�����չָ��

	// �û�����˫������ָ��
	User *_next , *_pre ;

	User()
	{
		_access_code = 0;
		_user_state = OFF_LINE;
		_fd   = NULL;
		_socket_type = TcpClient;
		_port = 0;
		_login_time = _last_active_time  = 0;
		_msg_seq = 0;
		_connect_info.keep_alive = AlwaysReConn;
		_connect_info.timeval  = 30;
		_next = _pre = NULL ;
		_ext_ptr = NULL ;
	}

	void reset()
	{
		_user_name.clear();
		_user_id.clear();
		_user_type.clear();
		_user_state = OFF_LINE;
		_fd = NULL;
		_socket_type = TcpClient;
		_port = 0;
		_ip.clear();
		_login_time = _last_active_time = 0;
		_msg_seq = 0;
		_connect_info.reset();
		_ext_ptr = NULL ;
	}

	//����ʹ��
	void show( const char *who = NULL )
	{
		if ( who ) {
			cout << "--------------" << who << "-----------------" << endl ;
		} else {
			cout << "--------------UserInfo---------------" << endl;
		}
		cout << "_user_name:" << _user_name << endl;
		cout << "_user_id:" << _user_id << endl;

		if (_user_state == OFF_LINE)
			cout << "_user_state:OFF_LINE" << endl;
		else if (_user_state == ON_LINE)
			cout << "_user_state:ON_LINE" << endl;
		else if (_user_state == WAITING_RESP)
			cout << "_user_state:WAITING_RESP" << endl;
		else if ( _user_state == DISABLED  )
			cout << "_user_state:DISABLED" << endl;


		cout << "_access_code:" << _access_code << endl;
		cout << "_fd:" << _fd << endl;
		if(_socket_type == TcpClient)
			cout<<"_socket_type:TcpClient"<<endl;

		else if(_socket_type == TcpConnClient)
			cout<<"_socket_type:TcpConnClient"<<endl;

		cout<<"_ip:"<<_ip<<endl;
		cout << "_port:" << _port << endl;
		cout << "_time(0)" << time(0) << endl;
		cout << "_last_active_time:" << _last_active_time << endl;
		cout << "_connect_info.keep_alive" << _connect_info.keep_alive << endl;
		cout << "_connect_info.timeval" << _connect_info.timeval << endl;
	}

	// �û�����ֵ����
	void copy(User &user)
	{
		_user_name   = user._user_name ;
		_user_pwd    = user._user_pwd ;
		_user_id     = user._user_id ;
		_user_type   = user._user_type ;
		_access_code = user._access_code ;
		_user_state  = user._user_state ;
		_fd          = user._fd ;
		_socket_type = user._socket_type ;
		_ip 		 = user._ip ;
		_port		 = user._port ;
		_login_time  = user._login_time ;
		_last_active_time = user._last_active_time ;
		_connect_info = user._connect_info ;
		_msg_seq      = user._msg_seq ;
		_ext_ptr      = user._ext_ptr ;
	}
};

class OnlineUser
{
typedef std::map<string , User*>   MapUser;
public:
	OnlineUser(){};
	virtual ~OnlineUser(){};

	//0 success; -1,���û����Ѿ�����,�Ҳ������Ƿ����ߡ�
	bool AddUser( const string &user_id, User &user )
	{
		_mutex.lock() ;
		MapUser::iterator it = _idmap.find(user_id) ;
		if ( it != _idmap.end() ) {
			_mutex.unlock() ;
			return false ;
		}

		User *p = new User;
		p->copy(user) ;

		Add( p ) ;

		_idmap.insert( std::make_pair( user_id, p ) ) ;
		if ( user._fd != NULL ) {
			user._fd->_ptr = p ;
		}
		_mutex.unlock() ;

		return true ;
	}

	User GetUserBySocket( socket_t *sock )
	{
		User user ;
		if ( sock == NULL ) return user;

		_mutex.lock() ;
		if ( sock->_ptr == NULL ) {
			// ����ֵ
			_mutex.unlock() ;
			return user ;
		}

		User *p = (User *) sock->_ptr ;
		if ( p->_fd != sock ) {
			sock->_ptr = NULL ;
			_mutex.unlock() ;
			return user ;
		}
		user.copy( *p ) ;

		_mutex.unlock() ;

		return user;
	}

	User GetUserByUserId(const string &user_id)
	{
		User user ;
		_mutex.lock() ;

		MapUser::iterator iter = _idmap.find(user_id);
		if (iter == _idmap.end()) {
			// ����ֵ
			_mutex.unlock() ;
			return user ;
		}
		user.copy(*iter->second) ;
		_mutex.unlock() ;

		return user;
	}

	// ɾ���û����ر�ɾ�����û�����
	User DeleteUser( socket_t *sock )
	{
		User user ;
		if( sock == NULL ) return user;

		_mutex.lock() ;
		if ( sock->_ptr == NULL ) {
			// ����ֵ
			_mutex.unlock() ;
			return user ;
		}

		User *p = (User *) sock->_ptr ;
		if ( sock != p->_fd ) {
			sock->_ptr = NULL ;
			_mutex.unlock() ;
			return user ;
		}

		user.copy(*p );
		// ��Ҫ���������Ӷ�Ӧһ����������
		Remove( p ) ;

		_mutex.unlock() ;

		return user ;
	}

	User DeleteUser(const string &user_id)
	{
		User user ;
		_mutex.lock() ;

		MapUser::iterator iter = _idmap.find(user_id);
		if (iter == _idmap.end()) {
			// ����ֵ
			_mutex.unlock() ;
			return user ;
		}

		User *p = iter->second ;
		user.copy( *p ) ;
		Remove( p ) ;

		_mutex.unlock() ;

		return user ;
	}

	// �Ƿ�����
	bool IsOnline(const string &user_id)
	{
		_mutex.lock() ;
		MapUser::iterator iter = _idmap.find(user_id);
		if (iter != _idmap.end()) {
			// ����ֵ
			_mutex.unlock() ;
			return true ;
		}
		_mutex.unlock() ;

		return false ;
	}

	// ȡ�������û��б�
	int GetAllUsers( vector<User> &vec )
	{
		int size = 0 ;

		_mutex.lock() ;

		size = _queue.size() ;
		if ( size == 0 ) {
			_mutex.unlock() ;
			return size ;
		}

		User *p = _queue.begin() ;
		while ( p != NULL ) {
			vec.push_back( *p ) ;
			p = _queue.next( p ) ;
		}

		_mutex.unlock() ;

		return size ;
	}

	vector<User> GetOnlineUsers()
	{
		vector<User> vec_user;

		_mutex.lock() ;

		if ( _queue.size() == 0 ) {
			_mutex.unlock() ;
			return vec_user ;
		}

		User *tmp, *p = _queue.begin() ;
		while( p != NULL ) {
			tmp = p ;
			p   = p->_next ;

			if ( tmp->_user_state == User::ON_LINE && tmp->_fd != NULL ){
				vec_user.push_back(*tmp) ;
			}
		}

		_mutex.unlock() ;

		return vec_user;
	}

	//���Ѿ����б���ɾ���ˡ��������¾�������Щ���ߵ�user�����ڵĴ�����1���ر�socket ��2����Ҫ������Ҫ���������еĹر��¼�����Timer�߳��д���
	vector<User> GetOfflineUsers()
	{
		vector<User> vec_user;

		_mutex.lock() ;
		if ( _queue.size() == 0 ) {
			_mutex.unlock() ;
			return vec_user ;
		}

		User *tmp, *p = _queue.begin() ;
		while( p != NULL ) {
			tmp = p ;
			p   = p->_next ;

			if ( ( tmp->_user_state == User::OFF_LINE || tmp->_fd == NULL ) && tmp->_user_state != User::DISABLED ){
				vec_user.push_back(*tmp) ;
				Remove( tmp ) ;
			}
		}

		_mutex.unlock() ;

		return vec_user;
	}

	//�õ����ߵ�users,���а�����ʱ�ġ�
	vector<User> GetOfflineUsers( int timeout )
	{
		vector<User> vec_user;

		_mutex.lock() ;
		if ( _queue.size() == 0 ) {
			_mutex.unlock() ;
			return vec_user ;
		}

		time_t now = time(0) ;

		User *tmp, *p = _queue.begin() ;
		while( p != NULL ) {
			tmp = p ;
			p   = p->_next ;

			if ( ( tmp->_user_state == User::OFF_LINE || tmp->_fd == NULL
					|| now - tmp->_last_active_time  > timeout ) && tmp->_user_state != User::DISABLED  ){
				vec_user.push_back(*tmp) ;
				Remove( tmp ) ;
			}
		}

		_mutex.unlock() ;

		return vec_user;
	}

	bool SetUser( const string &user_id,User &user )
	{
		_mutex.lock() ;
		MapUser::iterator iter = _idmap.find(user_id);
		if (iter == _idmap.end()) {
			_mutex.unlock() ;
			return false ;
		}
		// ����ֵ
		User *p = iter->second ;
		p->copy(user) ;
		if ( p->_fd != NULL ) {
			p->_fd->_ptr = p ;
		}
		_mutex.unlock() ;

		return true;
	}

private:
	// ��ӽڵ�
	void Add( User *p ) { _queue.push( p ) ; }
	// �Ƴ��û��ڵ�
	void Remove( User *p )
	{
		_idmap.erase( p->_user_id ) ;
		if ( p->_fd != NULL ) {
			p->_fd->_ptr = NULL ;
		}
		delete _queue.erase( p ) ;
	}
	// ��������û�����
	void Clear( void )
	{
		_idmap.clear() ;
	}

protected:  // ���㴦����չ
	// ���������
	share::Mutex   _mutex ;
	// ͷβָ��
	TQueue<User>   _queue ;
	// �û�ID���û���ѯ����
	MapUser		   _idmap ;
};

#pragma pack()

#endif /* ONLINEUSER_H_ */
