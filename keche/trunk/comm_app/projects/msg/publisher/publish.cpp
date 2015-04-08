/*
 * publish.cpp
 *
 *  Created on: 2012-4-16
 *      Author: humingqing
 *
 */

#include "publish.h"
#include <comlog.h>
#include <tools.h>

Publisher::Publisher()
{
	_nthread    = 1 ;
	_pusermgr   = NULL ;
	_pmsgserver = NULL ;
	_pubqueue   = new CDataQueue<_pubData>(102400) ;
	_queuethread= new CQueueThread( _pubqueue, this ) ;
}

Publisher::~Publisher()
{
	Stop() ;

	if ( _queuethread != NULL ) {
		delete _queuethread ;
		_queuethread = NULL ;
	}

	if ( _pubqueue != NULL ) {
		delete _pubqueue ;
		_pubqueue = NULL ;
	}
}

// ��ʼ����������
bool Publisher::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	int nvalue = 0 ;
	if ( pEnv->GetInteger( "publish_thread", nvalue ) ) {
		_nthread = nvalue ;
	}
	_pusermgr   = pEnv->GetUserMgr() ;
	_pmsgserver = pEnv->GetMsgClientServer() ;

	return true ;
}

// �������������߳�
bool Publisher::Start( void )
{
	if ( ! _queuethread->Init( _nthread ) ) {
		OUT_ERROR( NULL, 0, "Publish", "init publish thread failed" ) ;
		return false ;
	}
	OUT_INFO( NULL, 0, "Publish", "start publish thread success" ) ;

	return true ;
}

// ֹͣ���������߳�
bool Publisher::Stop( void )
{
	_queuethread->Stop() ;
	return true ;
}

// ��ʼ��������
bool Publisher::Publish( InterData &data, unsigned int cmd , User &user )
{
	_pubData *p = new _pubData ;
	if ( p == NULL )
		return false ;

	p->_cmd  	 = cmd ;
	p->_user 	 = user ;
	p->_data 	 = data ;

	if ( ! _queuethread->Push( p ) ) {
		delete p ;
		return false ;
	}
	return true ;
}

// �������ƽ̨�Ĳ���
static bool splitsubscribe( const std::string &param,  vector<string> &vec )
{
	size_t pos = param.find("{") ;
	if ( pos == string::npos ) {
		return false ;
	}

	size_t end = param.find("}", pos ) ;
	if ( end == string::npos || end < pos + 1 ) {
		return false ;
	}
	return splitvector( param.substr( pos+1, end-pos-1 ), vec, ",", 0 ) ;
}

// �������ݶ���
bool Publisher::OnDemand( unsigned int cmd , unsigned int group, const char *szval, User &user )
{
	if ( szval == NULL )
		return false ;

	vector<string> vec ;
	if ( ! splitsubscribe( szval, vec) )
		return false ;

	int i = 0 , count = 0 ;
	switch( cmd )
	{
	case OP_SUBSCRIBE_DMD:
	case OP_SUBSCRIBE_ADD:
		{
			if ( cmd == OP_SUBSCRIBE_DMD )
				_submgr.Del( user._access_code ) ;

			int len = 0 ;
			if ( group == TYPE_SUBSCRIBE_MACID ) {
				len = vec.size() ;
				// ��Ҫ��������
				for ( i = 0; i < len; ++ i ) {
					if ( _submgr.Add( user._access_code, vec[i].c_str() ) ) {
						++ count ;
					}
				}
			} else {
				// ��Ҫ�ӻ����в���
				std::set<string> kset ;
				vector<string> vecval ;

				len = vec.size() ;
				// ���ض��ĵ������������
				for( i = 0; i < len; ++ i ) {
					LoadSubscribe( vec[i].c_str(), vecval , kset ) ;
				}

				len = vecval.size() ;
				for ( i = 0; i< len ; ++ i ) {
					string &s = vecval[i] ;
					if ( strncmp( s.c_str(), "JMP:", 4 ) == 0 ) {
						continue ;
					}
					if ( _submgr.Add( user._access_code, s.c_str() ) ) {
						++ count ;
					}
				}
			}
		}
		break ;
	case OP_SUBSCRIBE_UMD:
		{
			if ( group == TYPE_SUBSCRIBE_MACID ) {
				count = vec.size() ;
				// ��Ҫ��������
				for ( i = 0; i < count; ++ i ) {
					_submgr.Remove( user._access_code, vec[i].c_str() ) ;
				}
			} else {
				// ��Ҫ�ӻ����в���
				vector<string> vecval ;
				std::set<string> kset ;
				for( i = 0; i < (int)vec.size(); ++ i ) {
					LoadSubscribe( vec[i].c_str(), vecval , kset ) ;
				}

				count = vecval.size() ;
				for ( i = 0; i< count; ++ i ) {
					string &s = vecval[i] ;
					if ( strncmp( s.c_str(), "JMP:", 4 ) == 0 ) {
						continue ;
					}
					_submgr.Remove( user._access_code, s.c_str() ) ;
				}
			}
		}
		break ;
	}
	OUT_PRINT( NULL, 0, "Publish", "%s %s user: %s, macid count: %d", ( (OP_SUBSCRIBE_UMD == cmd ) ? "Remove" : "Load" ) ,
			( (group == TYPE_SUBSCRIBE_MACID ) ? "macid": "group") , user._user_id.c_str(), count ) ;
	return true ;
}

// �Ӷ��Ĺ�ϵ
void Publisher::LoadSubscribe( const char *key, std::vector<std::string> &vec, std::set<std::string> &kset )
{
	if ( key == NULL )  return ;

	std::set<std::string>::iterator it = kset.find( key ) ;
	if ( it != kset.end() ) {
		return ;
	}
	kset.insert( std::set<std::string>::value_type(key) ) ;

	int size  = vec.size() ;
	int count = _pEnv->GetRedisCache()->GetList( key , vec ) ;
	if ( count == 0 ) return ;

	int len = vec.size() ;
	// ��������ָ��
	for ( int i = size; i < len; ++ i ) {
		string &s = vec[i] ;
		if ( s.empty() )
			continue ;
		// ����ʹ����תָ�����������ݣ�Ҳ������������Զ�������
		if ( strncmp( s.c_str(), "JMP:", 4 ) == 0 ) {
			LoadSubscribe( s.substr(4).c_str(), vec, kset ) ;
		}
	}
}

// �߳�ִ�ж��󷽷�
void Publisher::HandleQueue( void *packet )
{
	Deliver( ( _pubData *) packet ) ;
}

// �׷�����
bool Publisher::Deliver( _pubData *p )
{
	// ����HASH����
	unsigned int hash = _pusermgr->GetHash( p->_data._macid.c_str(), p->_data._macid.length() ) ;
	// �����ڲ�Э��
	string data = p->_data._transtype + " " + p->_data._seqid + " " + p->_data._macid + " " + p->_data._cmtype + " "
			+ p->_data._command + " " + p->_data._packdata  + " \r\n" ;

	// ToDo: �Ż����������̫Ƶ����������ÿһ��GPS���ݶ���Ҫ��ѯ������̫ռ��CPU��Դ��Ҫ�Ż�����
	vector<User> vec_user ;
	if ( !_pusermgr->GetSendGroup( vec_user, hash , p->_cmd ) ) { // ���û���û���ֱ�ӷ�����
		// OUT_ERROR( NULL, 0, NULL, "user not online: %s" , data.c_str() ) ;
		return false ;
	}

	// ȡ���û�������
	int size = (int) vec_user.size();
	// ת�����ݴ���
	for(int i = 0; i < size; ++i ) {
		User &user = vec_user[i] ;
		// ������Ϊ�Լ��µķ����û�����Ҫת��
		if ( p->_user._access_code == user._access_code ) {
			continue ;
		}
		// ����û��鲻Ϊ��ʱ
		if ( ! p->_user._user_name.empty() ) {
			// �����ǰ�·����ݵ��û����뵱ǰ��Ҫ���͵�����ͬ�Ͳ�������
			if ( p->_user._user_type == user._user_type && p->_user._user_name == user._user_name )
				continue ;
		}
		// ���Ϊ�Ƕ��ĵ����ݾ�ֱ�ӷ�����,Ϊ�����û��Ž������ݹ���
		if ( ! _submgr.Check( user._access_code, p->_data._macid.c_str() , user._msg_seq & MSG_USER_DEMAND ) ) {
			continue ;
		}

		if ( ! _pmsgserver->Deliver( user._fd, data.c_str() , data.length() ) ) {
			OUT_ERROR( user._ip.c_str(), user._port, user._user_id.c_str(), "Send Data failed, close fd %d", user._fd ) ;
			_pmsgserver->Close( user._fd ) ;
		}
	}

	return true;
}

//////////////////////////////////////////// �������ݶ���  ///////////////////////////////////////////////////////
Publisher::SubscribeMgr::SubscribeMgr()
{

}

Publisher::SubscribeMgr::~SubscribeMgr()
{
	Clear() ;
}

// ��Ӷ��ĳ�����Ϣ
bool Publisher::SubscribeMgr::Add( unsigned int ncode, const char *macid )
{
	share::RWGuard guard( _rwmutex, true ) ;

	MapSubscribe::iterator it = _mapSuber.find( ncode ) ;

	_macList *p = NULL ;
	if ( it == _mapSuber.end() ) {
		p = new _macList;
		_mapSuber.insert( make_pair( ncode, p ) ) ;
	} else {
		p = it->second ;
	}
	map<string,int>::iterator itx = p->_mkmap.find( macid ) ;
	if ( itx != p->_mkmap.end() ) {
		itx->second = itx->second + 1 ;  // ��Ӷ��ĵ����ü���
		return true ;
	}
	p->_mkmap.insert( make_pair(macid , 1 ) ) ;
	return true ;
}

// �Ƿ�ɾ��
bool Publisher::SubscribeMgr::Find( unsigned int ncode, const char *macid , bool erase )
{
	MapSubscribe::iterator it = _mapSuber.find( ncode ) ;
	if ( it == _mapSuber.end() )
		return false ;

	_macList *p = it->second ;
	map<string,int>::iterator itx = p->_mkmap.find( macid ) ;
	if ( itx == p->_mkmap.end() )
		return false ;

	if ( erase ) {
		// �������ü���
		itx->second = itx->second - 1 ;
		// �������ü���Ϊ��ʱ����������
		if ( itx->second <= 0 ) {
			p->_mkmap.erase( itx ) ;
			if ( p->_mkmap.empty() ) {
				_mapSuber.erase( it ) ;
				delete p ;
			}
		}
	}
	return true ;
}

// ɾ�����ĳ�����Ϣ
void Publisher::SubscribeMgr::Remove( unsigned int ncode, const char *macid )
{
	share::RWGuard guard( _rwmutex, true ) ;
	// �Ƴ�����
	Find( ncode, macid, true ) ;
}

// ɾ����ǰ��������Ϣ
void Publisher::SubscribeMgr::Del( unsigned int ncode )
{
	share::RWGuard guard( _rwmutex, true ) ;

	MapSubscribe::iterator it = _mapSuber.find( ncode ) ;
	if ( it == _mapSuber.end() )
		return ;

	_macList *p = it->second ;
	_mapSuber.erase( it ) ;
	delete p ;
}

// ����Ƿ��ĳɹ�
bool Publisher::SubscribeMgr::Check( unsigned int ncode, const char *macid , bool check )
{
	if ( ! check ) return true ;
	share::RWGuard guard( _rwmutex, false ) ;
	return Find( ncode, macid, false ) ;
}

void Publisher::SubscribeMgr::Clear( void )
{
	share::RWGuard guard( _rwmutex, true ) ;
	// ���Ϊ��ֱ��ɾ��
	if ( _mapSuber.empty() ) {
		return ;
	}

	MapSubscribe::iterator it ;
	for ( it = _mapSuber.begin(); it != _mapSuber.end(); ++ it ) {
		delete it->second ;
	}
	_mapSuber.clear() ;
}
