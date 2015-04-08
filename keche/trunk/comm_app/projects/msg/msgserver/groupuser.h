/*
 * groupuser.h
 *
 *  Created on: 2011-11-2
 *      Author: humingqing
 *       ����ʵ��ʹ�õ���ĸ��Ҳ�����Ǽ򵥷���linux����ϵͳ�û�Ȩ�޹�����ƣ����һϵ���û����з��鴦��
 *       ����ͬһ����û����������ܺ�ΪMSG����Ϣ�ܺͣ�������Ҫ����Դ洢�����ȵķ���������ʹ���㷨Ϊ�򵥵����MAC_ID����HASH���䷽ʽ���������ݵľ��⴦��
 *       ���ж���ͬһ���û�������ܴ��ڶ��������������ڷ׷�����ͬһ�û������ϸ��ؾ��⴦���������Ҫ���ü򵥵���ת��������
 *       �������Ϊһ���༶��״�ṹ������Ϊ�û�������Է�Ϊ�飬�����������Ϊ����û����������û�����ȴ�����Ƕ�����ӣ�һ������������������ṹ
 */

#ifndef __GROUPUSER_H__
#define __GROUPUSER_H__

#include <interface.h>
#include <assert.h>
#include <murhash.h>
#include <TQueue.h>

// ��λ���зֲ�����
#define MAX_TYPE_CMD    4			 // ����������
// ��������ö��
static unsigned int _gcmd[] = { PIPE_SEND_CMD, UWEB_SEND_CMD, SAVE_SEND_CMD, SEND_SEND_CMD };
// ����һ��Hash������
#define MUR_HASH_SEED  0xfffff
class CGroupUserMgr : public IGroupUserMgr
{
	// �û���չ������
	struct UserEx
	{
		User _user;	  // �û���������
		int _user_cmd;  // �û���������
	};

	struct _ConnList;
	// ���������û�����
	struct _UserList
	{
		UserEx _user;   	   // ��ǰ�û�ֵ
		_UserList *_next, *_pre;  // ˫��ָ��
		_ConnList *_conn;         // ָ������������
	};

	struct _ConnHeader;
	// ��������ӳ�Ա����
	struct _ConnList
	{
		_UserList *_user;		 // �û�ָ��
		_ConnList *_next, *_pre; // ˫��ָ�뷽��ɾ��
		_ConnHeader *_header;		 // ָ������ͷ��
	};

	struct _HeaderList;
	// ���Ӷ�������ͷ
	struct _ConnHeader
	{
		unsigned int _size;    // ��ǰ���Ӹ���
		_ConnList * _pcur;    // ��ǰʹ������
		std::string _group;   // �������������
		std::string _userid;  // ������������û�
		_ConnList * _header;  // �����Աͷָ��
		_ConnList * _tail;  // �����Ա��βָ��
		_HeaderList * _lsthead; // ����ͷָ��
	};

	// ����ͷ����
	struct _HeaderList
	{
		_ConnHeader *_header;       // ͷ��ָ��
		_HeaderList *_next, *_pre; // ͷβָ�봦��
	};

	// �û�ID����
	typedef std::map< std::string, _UserList* >   CMapIdIndex;
	// ��ǰ�û�������
	typedef std::map< std::string, _ConnHeader* > CMapConnGroup;

	// �û���Ӧ�����ӹ���
	class CUserConnMgr
	{
		// �û���Ľṹ��
		typedef vector< _HeaderList* > VecGroup;
		// �û��������ṹ
		struct _GroupList
		{
			_GroupList( )
			{
				_size = 0;
				_data = NULL;
				_next = NULL;
				_pre = NULL;
			}
			int _size;
			std::string _group;   // �������������
			VecGroup _vec;	 // ���Ա����
			_HeaderList *_data;	 // Ĭ��Ϊ�յ��ڵ����
			_GroupList *_next;	 // ��һ���ڵ�ָ��
			_GroupList *_pre;	 // ǰһ���ڵ�ָ��
		};
	public:
		CUserConnMgr( ) {} ;
		~CUserConnMgr( ) { Clear(); };

		// ����û����Ӵ���
		bool AddUserConn( _UserList * p )
		{
			string group;
			string userid = p->_user._user._user_id;
			// ���Ϊ������������
			if ( ! p->_user._user._user_name.empty() ) {
				group = p->_user._user._user_type + p->_user._user._user_name;
			}
			// �������������Ĵ���
			size_t pos = userid.find( ":" );
			if ( pos != string::npos ) {
				userid = userid.substr( 0, pos );
			}

			_ConnHeader *header = NULL;
			CMapConnGroup::iterator it = _mapUserConn.find( userid );
			if ( it == _mapUserConn.end() ) {
				header = new _ConnHeader;
				header->_group = group;
				header->_userid = userid;
				header->_pcur = NULL;
				header->_size = 0;
				header->_tail = header->_header = NULL;
				// ��ӵ��û�������
				_mapUserConn.insert( make_pair( userid, header ) );

				// ����û�����������
				AddHeader( header, p->_user._user_cmd );
			} else {
				header = it->second;
				// ����鲻һ���û�����ͬ�Ͳ�������½��
				if ( header->_group != group ) {
					return false;
				}
			}

			_ConnList *conn = new _ConnList;
			conn->_user = p;
			conn->_header = header;
			p->_conn = conn;
			conn->_next = NULL;
			conn->_pre = NULL;
			header->_size = header->_size + 1;

			// ���Ϊ�յ����
			if ( header->_header == NULL && header->_tail == NULL ) {
				header->_pcur = conn;
				header->_header = header->_tail = conn;
			} else { // �����Ϊ��ͷ�ڵ�
				conn->_pre = header->_tail;
				header->_tail->_next = conn;
				conn->_next = NULL;
				header->_tail = conn;
			}

			return true;
		}

		// ɾ����ǰ���Ӵ���
		void DelUserConn( _UserList *p )
		{
			if ( p == NULL )
				return;

			_ConnList *conn = p->_conn;
			_ConnHeader *header = conn->_header;

			unsigned int cmd = conn->_user->_user._user_cmd;

			// ���ֻ��һ��Ԫ�ص����
			if ( conn == header->_header && conn == header->_tail ) {
				header->_header = header->_tail = NULL;
			} else if ( conn == header->_header ) {
				header->_header = conn->_next;
				header->_header->_pre = NULL;
			} else if ( conn == header->_tail ) {
				header->_tail = conn->_pre;
				header->_tail->_next = NULL;
			} else {
				conn->_pre->_next = conn->_next;
				conn->_next->_pre = conn->_pre;
			}
			header->_size = header->_size - 1;
			header->_pcur = header->_header;  // �Ͽ�����ֱ��ָ��ͷ�����ӷ���
			// ɾ�����ӹ���
			delete conn;

			// ���������Ϊ���ˣ�������û���������
			if ( header->_header == NULL && header->_size == 0 ) {
				// �Ƴ���ǰ�û��Ự
				RemoveConn( header->_userid );
				// ɾ���������
				_HeaderList *phead = header->_lsthead ;
				if( phead != NULL ) {
					DelGroup( phead , cmd );
					delete _headqueue.erase( phead ) ;
				}
				// �Ƴ���Ķ�Ӧ��ϵ
				delete header;
			}
		}

		// ȡ�ÿ��÷����û�
		int GetSendUsers( vector< User > &vec, unsigned int hash, unsigned int cmd )
		{
			// û�п��õ��û�
			if ( _headqueue.size() == 0 ) {
				return 0;
			}

			int count = 0;
			for ( int i = 0 ; i < MAX_TYPE_CMD ; ++ i ) {
				if ( ! ( cmd & _gcmd[i] ) )
					continue;
				if ( _vecgroup[i].size() == 0 )
					continue;

				_GroupList *p = _vecgroup[i].begin() ;
				while ( p != NULL ) {
					if ( GetSendUsers( p, vec, hash, cmd ) ) {
						++ count;
					}
					p = p->_next;
				}
			}
			return count;
		}

		// �����û�IDȡ���û�������ΪͬӦ�õ�����
		bool GetUserByUserId( const string &userid, User &user )
		{
			share::Guard guard( _mutex );

			if ( _mapUserConn.empty() )
				return false;

			CMapConnGroup::iterator it = _mapUserConn.find( userid );
			if ( it == _mapUserConn.end() )
				return false;

			_ConnHeader *header = it->second;
			if ( header->_size == 0 || header->_header == NULL )
				return false;

			if ( header->_pcur == NULL )
				header->_pcur = header->_header;
			_ConnList *conn = header->_pcur;
			//  ������Ӹ�������1���Ĵ���
			if ( header->_size > 1 ) {
				// ���Ϊβ������ֱ��ָ��ͷ������
				if ( header->_pcur == header->_tail ) {
					header->_pcur = header->_header;
				} else {
					header->_pcur = header->_pcur->_next;
				}
			}
			// ȡ���û���������
			user = conn->_user->_user._user;

			return true;
		}

	private:
		// ���õķ��͵��û�����
		bool GetSendUsers( _GroupList *pcmd, vector< User > &vec, unsigned int hash, unsigned int cmd )
		{
			share::Guard guard( _mutex );

			_HeaderList *p = NULL;
			// �������ռ
			if ( pcmd->_size == 0 ) {
				p = pcmd->_data;  // ֱ��ȡ�ڵ�ֵ
			} else {
				if ( pcmd->_size == 1 ) {  // ���Ϊ��ĵ�����
					p = pcmd->_vec[0];
				} else { // ����Ϊ��Ķ�Ӧ�ý���
					p = pcmd->_vec[hash % pcmd->_size];
				}
			}

			_ConnHeader * header = p->_header;
			if ( header->_size == 0 || header->_header == NULL )
				return false;

			if ( header->_pcur == NULL )
				header->_pcur = header->_header;
			_ConnList *conn = header->_pcur;
			// ����û���ǰΪ��ֱ�ӷ���
			if ( conn->_user == NULL )
				return false;

			// ���Ϊ�������û���ֱ�ӷ�����
			if ( conn->_user->_user._user._user_state != User::ON_LINE )
				return false;

			// �������������1�Ž���ѭ������
			if ( header->_size > 1 ) {
				if ( header->_pcur == header->_tail ) {
					header->_pcur = header->_header;
				} else {
					header->_pcur = header->_pcur->_next;
				}
			}
			// ��ӿɷ��͵Ķ�����
			vec.push_back( conn->_user->_user._user );
			// ����ɹ����ؽ��
			return true;
		}

		// �Ƴ��û����Ӵ���
		void RemoveConn( const string &userid )
		{
			CMapConnGroup::iterator it = _mapUserConn.find( userid );
			if ( it == _mapUserConn.end() )
				return;
			_mapUserConn.erase( it );
		}

		// �������
		void AddHeader( _ConnHeader *p, unsigned int cmd )
		{
			_HeaderList *temp = new _HeaderList;
			temp->_header = p;
			temp->_next = NULL;
			temp->_pre = NULL;
			p->_lsthead = temp;

			_headqueue.push( temp ) ;
			// �����Ķ༶������
			AddGroup( temp, cmd );
		}

		// �Ƴ�ͷ������
		void RemoveHeader( _ConnHeader *header )
		{
			if ( header->_header != NULL ) {
				_ConnList *pre, *p = header->_header;
				while ( p != NULL ) {
					pre = p;
					p = p->_next;
					delete pre;
				}
				header->_header = header->_tail = NULL;
			}
			delete header;
		}

		// ������������
		void Clear( void )
		{
			int size = 0 ;
			_HeaderList *p = _headqueue.move( size ) ;
			if ( size == 0 )
				return;

			_HeaderList *pre ;
			while ( p != NULL ) {
				pre = p;
				p = p->_next;
				RemoveHeader( pre->_header );
				delete pre;
			}

			_mapUserConn.clear();
		}

		// ����������
		_GroupList * FindGroup( TQueue<_GroupList> &cmd, const string &group )
		{
			// ���Ϊ�ջ���û����Ĺ�ϵ
			if ( cmd.size() == 0 || group.empty() ) {
				return NULL;
			}

			_GroupList *p = cmd.begin() ;
			// ����������
			while ( p != NULL ) {
				if ( p->_group == group ) {
					return p;
				}
				p = p->_next;
			}

			return NULL;
		}

		// ��ӵ�����������
		void AddGroup( _HeaderList *p, unsigned int cmd )
		{
			// ��ӵ���Ӧ����Ĺ�ϵ��
			for ( int i = 0 ; i < MAX_TYPE_CMD ; ++ i ) {
				if ( ! ( cmd & _gcmd[i] ) )
					continue;

				string &group = p->_header->_group;
				// ����������
				_GroupList *pcmd = FindGroup( _vecgroup[i], group );
				if ( pcmd == NULL ) { // ��������ڻ��ߵ���
					pcmd = new _GroupList;
					pcmd->_group = group;
					pcmd->_next = NULL;
					_vecgroup[i].push( pcmd ) ;
				}
				// �����Ϊ�վ��ǵ��ڵ�ģʽ
				if ( group.empty() )
					pcmd->_data = p;
				else { // ���Ϊ����ķ�ʽ����
					pcmd->_vec.push_back( p );
					++ pcmd->_size;
				}
			}
		}

		// ɾ����������
		void DelGroup( _HeaderList *p, unsigned int cmd )
		{
			// �������п����������
			for ( int i = 0 ; i < MAX_TYPE_CMD ; ++ i ) {
				if ( ! ( cmd & _gcmd[i] ) )
					continue;
				if ( _vecgroup[i].size() == 0 )
					continue;

				// ������ص���
				_GroupList *pcmd = FindGroup( _vecgroup[i], p->_header->_group );
				if ( pcmd == NULL ) {  // ���û����ĵ������
					pcmd = _vecgroup[i].begin() ;
					while ( pcmd != NULL ) {
						if ( pcmd->_data == p ) {
							delete _vecgroup[i].erase(pcmd) ;
							break;
						}
						pcmd = pcmd->_next;
					}
				} else {  // ������ҵ�������
					VecGroup::iterator it;
					for ( it = pcmd->_vec.begin(); it != pcmd->_vec.end() ; ++ it ) {
						if ( * it != p )
							continue;
						pcmd->_vec.erase( it );
						-- pcmd->_size;
						break;
					}
					// ���Ԫ�ظ���Ϊ������Ҫ����ڵ�ͷ
					if ( pcmd->_size == 0 ) {
						delete _vecgroup[i].erase( pcmd ) ;
					}
				}
			}
		}

	private:
		// �û����ӹ���
		CMapConnGroup 		_mapUserConn;
		// �û�ͷβָ��
		TQueue<_HeaderList> _headqueue;
		// ����߳�ͬʱ�������ָ��
		share::Mutex 		_mutex;
		// ��������ͷֲ�HASH�����
		TQueue<_GroupList>  _vecgroup[MAX_TYPE_CMD];
	};

public:
	// ��ʼ������
	CGroupUserMgr( ){
		_pusermgr = new CUserConnMgr;
	}
	// ���������ڴ�
	~CGroupUserMgr( ) {
		delete _pusermgr;
	}

	//0 success; -1,���û����Ѿ�����,�Ҳ������Ƿ����ߡ�
	bool AddUser( const std::string &user_id, const User &user )
	{
		share::RWGuard guard( _rwmutex, true );
		// �����û��Ƿ������
		if ( FindIdIndex( user_id, false ) != NULL )
			return false;

		_UserList * p = new _UserList;
		p->_user._user = user ;
		p->_user._user_cmd = GetCmdByUserType( user._user_type ); // ȡ�ö�Ӧ��������
		p->_next = p->_pre = NULL;

		// �����ӵ��û���ʧ�ܾ�ֱ�ӷ�����
		if ( ! _pusermgr->AddUserConn( p ) ) {
			delete p;
			return false;
		}
		_userqueue.push( p ) ;

		p->_user._user._fd->_ptr = p ;
		// �����������
		_mapIdIndex.insert( make_pair( user_id, p ) );

		return true;
	}

	// ȡ���û�ͨ��FD
	bool GetUserBySocket( socket_t *sock, User &user )
	{
		// ȡ���û�ͨ���û���fd
		share::RWGuard guard( _rwmutex, false );
		_UserList *p = FindFdIndex( sock, false );
		if ( p == NULL )
			return false;
		// �����û���Ϣ
		user = p->_user._user;
		return true;
	}

	// ͨ���û�ID��ȡ���û�
	bool GetUserByUserId( const std::string &user_id, User &user, bool bgroup = false )
	{
		// ȡ���û�ͨ���û�ID
		share::RWGuard guard( _rwmutex, false );
		// �����û�
		_UserList *p = FindIdIndex( user_id, false );
		if ( p == NULL ) {
			if ( ! bgroup || user_id.empty() )
				return false;

			// ������鴦����Ҫ����ǰ��
			size_t pos = user_id.find( ":" );
			if ( pos == string::npos ) {
				return false;
			}
			// ȡ�õ�ǰ�������û�
			string guserid = user_id.substr( 0, pos );
			// get user from user list
			return _pusermgr->GetUserByUserId( guserid, user );
		}
		// �������ŵ�ǰ�û�
		user = p->_user._user;
		return true;
	}

	// ɾ���û�ͨ��FD
	void DeleteUser( socket_t *sock )
	{
		// ɾ����Ĺ�ϵ
		share::RWGuard guard( _rwmutex, true );
		_UserList *p = FindFdIndex( sock, true );
		if ( p == NULL )
			return;

		FindIdIndex( p->_user._user._user_id, true );
		RemoveUserList( p );
	}

	// ɾ���û�
	void DeleteUser( const std::string &user_id )
	{
		// ɾ�����ϵ
		share::RWGuard guard( _rwmutex, true );
		_UserList *p = FindIdIndex( user_id, true );
		if ( p == NULL )
			return;

		FindFdIndex( p->_user._user._fd, true );
		RemoveUserList( p );
	}

	// ȡ�������û�
	bool GetOnlineUsers( std::vector< User > &vec )
	{
		// ȡ�������û�
		share::RWGuard guard( _rwmutex, false );
		if (  _userqueue.size() == 0 )
			return false;

		_UserList *p = _userqueue.begin() ;
		while ( p != NULL ) {
			// �������е�ǰ�û�
			if ( p->_user._user._user_state == User::ON_LINE
					&& p->_user._user._fd != NULL ) {
				vec.push_back( p->_user._user );
			}
			p = p->_next;
		}

		return true;
	}

	// �����û���״̬
	bool SetUser( const std::string &user_id, User &user )
	{
		share::RWGuard guard( _rwmutex, true );
		_UserList *p = FindIdIndex( user_id, false );
		if ( p == NULL )
			return false;
		p->_user._user = user;
		p->_user._user_cmd = GetCmdByUserType( user._user_type );
		return true;
	}

	// ����Ƿ���Ҫ���͵�����
	bool GetSendGroup( std::vector< User > &vec, unsigned int hash, unsigned int cmd )
	{
		share::RWGuard guard( _rwmutex, false );
		// ȡ�ÿ��Է������ݵ��û�
		return ( _pusermgr->GetSendUsers( vec, hash, cmd ) > 0 );
	}

	// ȡ��Hashֵ
	unsigned int GetHash( const char *key, int len )
	{
		// ����Hash����
		return mur_mur_hash2( ( void* ) key, len, MUR_HASH_SEED );
	}

	// ��ǰ����������
	int GetSize( void )
	{
		share::RWGuard guard( _rwmutex, false );
		return _userqueue.size() ;
	}

private:
	// �Ƴ��û�����
	void RemoveUserList( _UserList * p )
	{
		// ���û��������Ƴ�
		p = _userqueue.erase( p ) ;
		// �Ƴ��û�
		_pusermgr->DelUserConn( p );

		//
		if ( p->_user._user._fd != NULL ) {
			p->_user._user._fd->_ptr = NULL ;
		}

		delete p;
	}

	// �Ƴ�FD����
	_UserList * FindFdIndex( socket_t *sock, bool erase )
	{
		if ( sock->_ptr == NULL )
			return NULL ;

		_UserList *p = (_UserList *) sock->_ptr ;
		if ( erase )
			sock->_ptr = NULL ;
		if ( p->_user._user._fd != sock )
			return NULL ;

		return p;
	}

	// �Ƴ��û�ID������
	_UserList * FindIdIndex( const string &userid, bool erase )
	{
		CMapIdIndex::iterator it = _mapIdIndex.find( userid );
		if ( it == _mapIdIndex.end() )
			return NULL;
		_UserList *p = it->second;
		if ( erase )
			_mapIdIndex.erase( it );
		return p;
	}

	// ͨ���û�������ȡ�ö�Ӧ����
	unsigned int GetCmdByUserType( const string &msg_type )
	{
		if ( msg_type == COMPANY_TYPE ) {
			return PIPE_SEND_CMD;
		} else if ( msg_type == WEB_TYPE ) {
			return UWEB_SEND_CMD;
		} else if ( msg_type == STORAGE_TYPE ) {
			return SAVE_SEND_CMD;
		} else if ( msg_type == SEND_TYPE ) {
			return SEND_SEND_CMD;
		}
		return 0x00000000;
	}

private:
	share::RWMutex    _rwmutex;
	TQueue<_UserList> _userqueue;
	CMapIdIndex 	  _mapIdIndex; // �û�ID������
	CUserConnMgr *    _pusermgr;   // �û����ݹ���
};

#endif /* GROUPUSER_H_ */
