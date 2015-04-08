/*
 * msgmgr.h
 *
 *  Created on: 2011-11-3
 *      Author: humingqing
 */

#ifndef __MSGMGR_H__
#define __MSGMGR_H__

#include "interface.h"
#include "usermgr.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <time.h>
#include <Mutex.h>
#include <stdint.h>
#include <TQueue.h>
using namespace std ;

#define MAX_TIME_OUT	180  // ���ʱʱ��
#define RESULT_SUCCESS  0    // ���سɹ�
#define RESULT_FAILED   1	 // ����ʧ��

class CMsgBuilder ;
class CFdClient ;
// ����Ŀͻ�
typedef vector<CFdClient *> CVecClient ;

// ��һ�ͻ�����
class CFdClient
{
public:
	CFdClient(){
		_fd    = NULL;
		_id    = 0 ;
		_ncar  = 0  ;
		_port  = 0  ;
		_ip    = "" ;
		_last  = time(0) ;
		_next  = _pre = NULL ;
		_group = 0 ;
	}
	virtual ~CFdClient(){}
	// ��ӵ�����MSG
	void AddMsgClient( CFdClient *p ) {
		share::Guard guard( _mutex ) ;
		_vecfd.push_back( p ) ;
	}
	//��ɾ����������MSG
	void DelMsgClient( CFdClient *p ) {
		share::Guard guard( _mutex ) ;
		if ( _vecfd.empty() )
			return ;
		CVecClient::iterator it ;
		for ( it = _vecfd.begin(); it != _vecfd.end(); ++ it ) {
			if ( *it != p )
				continue ;
			_vecfd.erase( it ) ;
			break ;
		}
	}
	// ȡ������MSG�Ķ���
	CVecClient & GetMsgClients( void ) {
		share::Guard guard( _mutex ) ;
		return _vecfd ;
	}
	// �������MSG������
	void ClearMsgClients( void ) {
		share::Guard guard( _mutex ) ;
		_vecfd.clear() ;
	}

	socket_t    *_fd ;     // ��Ӧ��FD
	uint32_t	 _id ;     // �ڵ�ID
	uint16_t     _group ;  // ������ǰ����ID
	uint32_t	 _ncar ;   // ��ǰ����ĳ�����
	string 	     _ip   ;   // IP��ַ
	uint16_t     _port ;   // �˿�
	time_t       _last ;   // ������ʱ��

	// ָ��ڵ�����ָ��
	CFdClient *_next, *_pre ;
	// ������MSG
	CVecClient   _vecfd ;
	// ����ͬ��������
	share::Mutex _mutex ;
};

// ��ID���б�
typedef std::vector<uint16_t>  CVecGroupIds ;

// �����ͷ���Ŀͻ���
class CGroupMgr
{
	// ͬһ���������пͻ�
	class CGroupClient
	{
	public:
		CGroupClient(){};
		~CGroupClient(){};
		// ��ӿͻ���
		void AddClient( CFdClient *p ) {
			_vecClient.push_back( p ) ;
		}

		// �Ƴ��ͻ���
		bool RemoveClient( CFdClient *p ) {
			if ( _vecClient.empty() ) {
				return false ;
			}
			// ����ɾ����Ӧ����
			CVecClient::iterator it ;
			for ( it = _vecClient.begin(); it != _vecClient.end(); ++ it )
			{
				if ( p != *it ) {
					continue ;
				}
				_vecClient.erase(it) ;
				return true ;
			}
			return false ;
		}

		// ȡ�õ�ǰ������ͻ���
		int  GetSize( void ) {return _vecClient.size();}
		// ȡ�õ�ǰ���������пͻ���
		int  GetClients( CVecClient &vec ) {
			if ( _vecClient.empty() ) {
				return 0 ;
			}
			int size = (int) _vecClient.size() ;
			for ( int i = 0; i < size; ++ i ) {
				vec.push_back( _vecClient[i] ) ;
			}
			return size ;
		}

	private:
		// ��ǰ��������ͻ�
		CVecClient _vecClient;
	};

	// ����ͻ�������
	typedef map<uint16_t, CGroupClient *>  CMapClient ;
public:
	CGroupMgr() ;
	~CGroupMgr() ;

	// ��ӵ���ǰ�Ŀͻ�������
	void AddGroup( CFdClient *p ) ;
	// ��FD�Ƴ���Ӧ��Client
	bool RemoveGroup( CFdClient *p ) ;
	// ȡ�õ�ǰ�����
	int  GetSize( void ) { return _mapClient.size(); }
	// ȡ�õ�ǰ�������
	int  GetGroup( uint16_t group, CVecClient &vec )  ;
	// ȡ�õ�ǰ���������������
	int  GetGroupIDs( uint16_t group, CVecGroupIds &vec ) ;

private:
	// �ͻ��˷������
	CMapClient   _mapClient ;
	// ����ͬ��������
	share::Mutex _mutex ;
};

// MSG������
class CMsgServer : public CFdClient
{
	// ����MSG״̬
	enum MSG_STATE{ MSG_OFFLINE = 0, MSG_WAITUSER , MSG_ONLINE };
public:
	CMsgServer() { _state = MSG_OFFLINE; };
	~CMsgServer(){};

	// ����µĿͻ�����
	void AddClient( CFdClient *p ) {
		p->AddMsgClient( this ) ;

		share::Guard guard( _mutex ) ;
		_vecfd.push_back( p ) ;
	}
	// �Ƴ���Ӧ��Client
	bool RemoveClient( CFdClient *p ) {
		_mutex.lock() ;
		if ( _vecfd.empty() ) {
			_mutex.unlock() ;
			return false ;
		}

		// ����ɾ����Ӧ����
		CVecClient::iterator it ;
		for ( it = _vecfd.begin(); it != _vecfd.end(); ++ it ) {
			if ( p != *it ) {
				continue ;
			}
			_vecfd.erase(it) ;
			_mutex.unlock() ;
			// ɾ��Ӧ��MSG��ϵ
			p->DelMsgClient( this ) ;

			return true ;
		}
		_mutex.unlock() ;

		return false ;
	}
	// �Ƿ�Ϊ��
	int  GetSize( void ) {
		share::Guard guard( _mutex ) ;
		return _vecfd.size();
	}
	// ȡ�õ�ǰ����MSG����Ŀͻ�������
	int  GetClients( uint16_t group, CVecClient &vec ) {
		share::Guard guard( _mutex ) ;
		if ( _vecfd.empty() ) {
			return 0 ;
		}
		// ȡ�õ�ǰMSG������Ŀͻ����Ӹ���
		CVecClient::iterator it ;
		for( it = _vecfd.begin(); it != _vecfd.end(); ++ it ) {
			CFdClient *p = (*it) ;
			if ( p->_group != group ) {
				continue ;
			}
			vec.push_back( p ) ;
		}
		return (int) vec.size() ;
	}

	// ȡ�õ�ǰ���ǰ�û�����
	int GetGroupSize( uint16_t group ) {
		share::Guard guard( _mutex ) ;
		if ( _vecfd.empty() )
			return 0 ;

		int count = 0 ;
		CVecClient::iterator it ;
		for ( it = _vecfd.begin(); it != _vecfd.end(); ++ it ) {
			CFdClient *p = ( *it ) ;
			if ( p->_group != group ) {
				continue ;
			}
			++ count ;
		}
		return count ;
	}

	// ȡ������MSG����Ŀͻ�����
	int GetClients( CVecClient &vec ) {
		share::Guard guard( _mutex ) ;
		if ( _vecfd.empty() ) {
			return 0 ;
		}
		vec = _vecfd ;
		return (int)vec.size() ;
	}
	// ȡ��MSG��״̬
	bool IsOnline(){ return ( _state == MSG_ONLINE ) ; }
	// �Ƿ�ȴ��û�Ӧ��״̬
	bool IsWaitUser() { return ( _state == MSG_WAITUSER ); }
	// ����MSG��״̬
	void SetOnline( void )   { _state = MSG_ONLINE ; }
	// ����MSGΪ�ȴ��û�״̬
	void SetWaitUser( void ) { _state = MSG_WAITUSER ; }

private:
	// MSG��������״̬
	MSG_STATE 	 _state ;
};

// MSG�������
class CMsgManager
{
	// ������������б�
	typedef map<uint64_t, CMsgServer*>  CMapMsgSrv ;
public:
	CMsgManager(ISystemEnv *pEnv, CMsgBuilder *builder ) ;
	~CMsgManager() ;

	// ���MSG������
	bool AddMsgServer( CMsgServer *p ) ;
	// ��ӿͻ���
	int  AddClient( uint64_t serverid , CFdClient *p ) ;
	// �Ƴ���Ӧ�Ŀͻ��˷�Ϊ����ǰ�û���MSG�Լ��洢
	bool RemoveClient( CFdClient *p ) ;
	// �׷�����ӵ��û���
	bool DispatherUsers( socket_t *sock , UserInfo *p , int count ) ;
	// ȡ�����е�MSG�б�
	bool GetMsgList( CVecClient &vec ) ;

private:
	// �Ƴ���ΪMSG�ڵ�
	bool RemoveMsgNode( CFdClient *p ) ;
	// ����MSG
	CMsgServer * FindMsgNode( CFdClient *pmsg , bool erase ) ;

private:
	// ��Ӧ��MSG���������б�
	CMapMsgSrv    _mapSrv ;
	// ��������ָ��
	ISystemEnv   *_pEnv ;
	// ��Ϣ��������
	CMsgBuilder	 *_builder ;
	// MSG������
	share::Mutex  _msgmutex ;
};

// ��������ط�����
#define MAX_RESEND_TIME   5

// ���Client�Ĺ���
class CFdClientMgr
{
	// �ط����������ü�������
	class CSeqRef{
		typedef map<unsigned int, unsigned int> CMapRef ;
	public:
		CSeqRef(){}
		~CSeqRef(){}
		// ��ӷ��ʹ�������
		unsigned int AddRef( unsigned int seq ) {
			share::Guard guard( _mutex ) ;
			CMapRef::iterator it = _ref.find( seq ) ;

			unsigned int count = 1 ;
			if ( it == _ref.end() ) {
				_ref.insert( make_pair( seq, count ) ) ;
			} else {
				count = it->second + 1 ;
				it->second = count ;
			}
			return count ;
		}

		// ɾ������������
		void DelRef( unsigned int seq ) {
			share::Guard guard( _mutex ) ;
			CMapRef::iterator it = _ref.find( seq ) ;
			if ( it == _ref.end() )
				return ;
			_ref.erase( it ) ;
		}

	private:
		CMapRef  	 _ref ;
		share::Mutex _mutex ;
	};
public:
	CFdClientMgr( ISystemEnv *pEnv, CMsgBuilder *pBuilder ) ;
	~CFdClientMgr() ;
	// ��½������ƽ̨
	bool LoginClient( socket_t *sock, unsigned int seq, NodeLoginReq *req ) ;
	// ע����½ƽ̨
	bool LogoutClient( socket_t *sock, unsigned int seq ) ;
	// ��·����
	bool LinkTest( socket_t *sock, unsigned int seq , NodeLinkTestReq *req ) ;
	// ��������û���
	bool UserName( socket_t *sock, unsigned int seq ) ;
	// �������MSG�б�
	bool GetMsgList( socket_t *sock, unsigned int seq ) ;
	// ���������û����Ľ��
	bool ProcUserName( socket_t *sock, unsigned int seq , MsgData *p ) ; // ���������������û��������
	// ���²����û��б�����
	bool DispatchUser( socket_t *sock ) ;
	// ���²���δ�յ�������
	bool ResendMsg( socket_t *sock, MsgData *p , ListFd &fds ) ;
	// ��⵱ǰFD�Ƿ����
	bool CheckFdClient( socket_t *sock ) ;
	// �Ƴ��ͻ���
	bool RemoveFdClient( socket_t *sock ) ;
private:
	// ��ӿͻ���
	bool AddClient( CFdClient *p ) ;
	// ����FDȡ�ö�ӦCLIENT
	CFdClient * GetFdClient( socket_t *sock ) ;
	// �Ƴ��ͻ���
	bool RemoveClient( CFdClient *p ) ;
	// ֪ͨ���MSG����
	void NotifyAddMsg( CFdClient *p ) ;

private:
	// ������
	share::RWMutex   	_rwmutex ;
	// ���Ӷ������
	TQueue<CFdClient>   _fdqueue;
	// ��Ĺ���
	CGroupMgr 		   *_pGroupMgr ;
	// MSG����������
	CMsgManager		   *_pMsgMgr ;
	// ��Ϣ��������
	CMsgBuilder		   *_pBuilder ;
	// �û��������
	CUserMgr		   *_pUserMgr ;
	// �ؼ��������ô���
	CSeqRef				_seqref ;
	// ��������ָ��
	ISystemEnv 	       *_pEnv ;
};

// �ڵ�������������н�����
class CNodeMgr : public IMsgNotify
{
public:
	CNodeMgr(ISystemEnv *pEnv) ;
	~CNodeMgr() ;
	// ������������
	void Process( socket_t *sock, const char *data, int len ) ;
	// �����¼�
	void Close( socket_t *sock ) ;
	// �������ص�����
	void NotifyMsgData( socket_t *sock, MsgData *p , ListFd &fds, unsigned int op ) ;

private:
	// �������ݷ�����
	ISystemEnv *   _pEnv ;
	// ���ӹ���
	CFdClientMgr * _clientmgr ;
	// ��Ϣ��������
	CMsgBuilder	 * _builder ;
};

#endif /* MSGLIST_H_ */
