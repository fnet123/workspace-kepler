/*
 * msgmgr.cpp
 *
 *  Created on: 2011-11-3
 *      Author: humingqing
 */

#include "nodemgr.h"
#include "netutil.h"
#include <comlog.h>
#include <tools.h>
#include <msgbuilder.h>
//////////////////////////////// ��Ĺ������ ////////////////////////////////////
CGroupMgr::CGroupMgr()
{
}

CGroupMgr::~CGroupMgr()
{
	share::Guard guard( _mutex ) ;
	if( _mapClient.empty() )
		return ;
	// �����ڴ�
	CMapClient::iterator  it ;
	for ( it = _mapClient.begin(); it != _mapClient.end(); ++ it ) {
		delete it->second ;
	}
	_mapClient.clear() ;
}

// ��ӵ���ǰ�Ŀͻ�������
void CGroupMgr::AddGroup( CFdClient *p )
{
	share::Guard guard( _mutex ) ;
	CGroupClient *pGroup = NULL ;
	CMapClient::iterator it = _mapClient.find( p->_group ) ;
	if ( it == _mapClient.end() ) {
		pGroup = new CGroupClient;
		_mapClient.insert( make_pair( p->_group, pGroup ) ) ;
	} else {
		pGroup = it->second ;
	}
	// ��Ӷ�Ӧ����
	pGroup->AddClient( p ) ;
}

// ��FD�Ƴ���Ӧ��Client
bool CGroupMgr::RemoveGroup( CFdClient *p )
{
	share::Guard guard( _mutex ) ;
	CMapClient::iterator it = _mapClient.find( p->_group ) ;
	if ( it == _mapClient.end() ) {
		return false ;
	}
	// ������ɾ��Ԫ��
	CGroupClient *pGroup = it->second ;
	if ( ! pGroup->RemoveClient( p ) ) {
		return false ;
	}
	// �����ǰ����û�Ϊ��ֱ��ȥ����ǰ��
	if ( pGroup->GetSize() == 0 ) {
		_mapClient.erase( it ) ;
		delete pGroup ;
	}
	return true ;
}

// ȡ�õ�ǰ�������
int CGroupMgr::GetGroup( uint16_t group , CVecClient &vec )
{
	share::Guard guard( _mutex ) ;
	CMapClient::iterator it = _mapClient.find( group ) ;
	if ( it == _mapClient.end() ) {
		return 0 ;
	}
	CGroupClient *pGroup = it->second ;
	return pGroup->GetClients( vec ) ;
}

// ȡ�õ�ǰ���������������
int CGroupMgr::GetGroupIDs( uint16_t group, CVecGroupIds &vec )
{
	share::Guard guard( _mutex ) ;
	if ( _mapClient.empty() )
		return 0 ;

	std::set<uint16_t>  setGroup ;
	std::set<uint16_t>::iterator itx ;

	CMapClient::iterator it ;
	for ( it = _mapClient.begin(); it != _mapClient.end(); ++ it ) {
		uint16_t ngroup = it->first ;
		if ( !( ngroup & group ) ) {
			continue ;
		}

		// �������Ƿ��Ѿ�������û�������
		itx = setGroup.find( ngroup ) ;
		if ( itx == setGroup.end() ) {
			vec.push_back( ngroup ) ;
			setGroup.insert( std::set<uint16_t>::value_type( ngroup ) ) ;
		}
	}
	return (int) vec.size() ;
}

/////////////////////////////// MSG������� /////////////////////////////////

CMsgManager::CMsgManager(ISystemEnv *pEnv , CMsgBuilder *builder )
{
	_pEnv    = pEnv ;
	_builder = builder ;
}

CMsgManager::~CMsgManager()
{

}

// ���MSG������
bool CMsgManager::AddMsgServer( CMsgServer *p )
{
	uint64_t id = netutil::strToAddr( p->_ip.c_str(), p->_port ) ;
	if ( id <= 0 ) return false;

	share::Guard guard( _msgmutex ) ;

	CMapMsgSrv::iterator it = _mapSrv.find( id ) ;
	if ( it != _mapSrv.end() ) {
		return false ;
	}
	// ���MSG������
	_mapSrv.insert( make_pair( id, p ) ) ;

	return true ;
}

// ��ӿͻ���
int CMsgManager::AddClient( uint64_t serverid , CFdClient *p )
{
	share::Guard guard( _msgmutex ) ;

	CMapMsgSrv::iterator it = _mapSrv.find( serverid ) ;
	if ( it == _mapSrv.end() ) {
		return 0 ;
	}

	CMsgServer *pMsg = it->second ;
	pMsg->AddClient( p ) ;

	// ���ﷵ����Ӹ����͸���
	return pMsg->GetGroupSize( p->_group ) ; // ���Ϊǰ�û����ͽ������Ҫ������Ӵ洢
}

// ����MSG
CMsgServer * CMsgManager::FindMsgNode( CFdClient *pmsg , bool erase )
{
	CMsgServer *pMsg = NULL ;

	uint64_t id = netutil::strToAddr( pmsg->_ip.c_str(), pmsg->_port ) ;

	CMapMsgSrv::iterator it = _mapSrv.find( id ) ;
	if ( it == _mapSrv.end() ) {
		return NULL ;
	}

	// ȡ�õ�ǰMSG
	pMsg = it->second ;
	if ( erase ) {
		// ���Ƴ�����֪ͨ
		_mapSrv.erase( it ) ;
	}

	return pMsg ;
}

bool CMsgManager::RemoveMsgNode( CFdClient *pmsg )
{
	share::Guard guard( _msgmutex ) ;
	CMsgServer *pMsg = FindMsgNode( pmsg, true ) ;
	if ( pMsg == NULL ){
		return false ;
	}

	CVecClient vec ;
	// ��ȡ��ǰ��������ǰ�û�
	int size = pMsg->GetClients( vec ) ;
	if ( size > 0 ) {

		AddrInfo addr ;
		safe_memncpy((char*) addr.ip, pmsg->_ip.c_str() , sizeof(addr.ip) ) ;
		addr.port  =  pmsg->_port ;
		// ����֪ͨ��Ϣ
		MsgData *pdata = _builder->BuildMsgChgReq( NODE_MSG_DEL, &addr, 1 ) ;
		if ( ! _pEnv->GetWaitGroup()->AddGroup( pmsg->_fd, pdata->seq, pdata ) ){
			OUT_ERROR( NULL, 0, "MsgManger", "add group seq id %u failed" , pdata->seq ) ;
		}

		DataBuffer buf ;
		_builder->BuildMsgBuffer( buf, pdata ) ;

		// ����MSG�ı����Ϣ
		CFdClient *pClient = NULL ;
		for ( int i = 0; i < size; ++ i ) {
			// �����͵���Ϣ
			pClient = vec[i] ;
			// �첽��������ӵ����к���
			_pEnv->GetWaitGroup()->AddQueue( pdata->seq, pClient->_fd ) ;
			// ����MSG DOWN��֪ͨ������ǰ�û�ת������
			if ( ! _pEnv->GetNodeSrv()->HandleData( pClient->_fd, buf.getBuffer(), buf.getLength() )  ){
				// �������ʧ����ֱ��ɾ��
				_pEnv->GetWaitGroup()->DelQueue( pdata->seq, pClient->_fd , false ) ;
			}
			// FD_CONN_PIPE,
			pClient->DelMsgClient( pmsg ) ;
		}
	}
	OUT_INFO( NULL, 0, "NodeMgr" , "remove msg node fd %d, ip %s, port %d , client size %d" ,
			pmsg->_fd, pmsg->_ip.c_str(), pmsg->_port , size ) ;
	return true ;
}

// �Ƴ���Ӧ�Ŀͻ��˷�Ϊ����ǰ�û���MSG�Լ��洢
bool CMsgManager::RemoveClient( CFdClient *p )
{
	if ( p->_group & FD_NODE_MSG ) {
		// ���ΪMSG�ڵ�
		return RemoveMsgNode( p ) ;
	}

	share::Guard guard( _msgmutex ) ;
	CVecClient &vec = p->GetMsgClients();
	if ( vec.empty() ) {
		return false ;
	}

	for ( int i = 0 ; i < (int) vec.size(); ++ i ) {
		// ȡ�õ�ǰMSG
		CMsgServer *pMsg = FindMsgNode( vec[i], false ) ;
		if ( pMsg == NULL )
			continue ;
		// ����ֱ���Ƴ���ǰMSG�Ķ���
		pMsg->RemoveClient( p ) ;
	}
	return true ;
}

// �׷�����ӵ��û���
bool CMsgManager::DispatherUsers( socket_t *sock, UserInfo *p , int count )
{
	if ( count == 0 )
		return false ;

	// ������Ϣ
	MsgData *msg = _builder->BuildUserNotifyReq( p, count , 0 ) ;

	DataBuffer buf ;
	_builder->BuildMsgBuffer( buf, msg ) ;

	// ��ӵ��ȴ�������
	_pEnv->GetWaitGroup()->AddGroup( sock, msg->seq, msg ) ;
	// ��������ӵ��û��б��MSG
	return _pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ;
}

// ȡ�����е�MSG�б�
bool CMsgManager::GetMsgList( CVecClient &vec )
{
	share::Guard guard( _msgmutex ) ;

	if ( _mapSrv.empty() )
		return false ;

	CMsgServer *p = NULL ;
	CMapMsgSrv::iterator it ;
	for ( it = _mapSrv.begin(); it != _mapSrv.end(); ++ it ) {
		p = it->second ;
		// ȡ�����п��õ�MSG
		if ( ! p->IsOnline() ) {
			continue ;
		}
		vec.push_back( p ) ;
	}
	return ( ! vec.empty() ) ;
}

//////////////////// ���Client�Ĺ���  /////////////////////////////
CFdClientMgr::CFdClientMgr(ISystemEnv *pEnv, CMsgBuilder *pBuilder )
{
	_pEnv 	   = pEnv ;
	_pBuilder  = pBuilder ;
	_pUserMgr  = new CUserMgr( pEnv->GetUserPath() );  // ȡ�õ�ǰ�û����ݱ���λ��
	// ����MSG��Ĺ���
	_pGroupMgr = new CGroupMgr;
	_pMsgMgr   = new CMsgManager( pEnv, pBuilder ) ;
}

CFdClientMgr::~CFdClientMgr()
{
	if ( _pGroupMgr != NULL ) {
		delete _pGroupMgr ;
		_pGroupMgr = NULL ;
	}
	if ( _pMsgMgr != NULL ) {
		delete _pMsgMgr ;
		_pMsgMgr = NULL ;
	}
	if ( _pUserMgr != NULL ) {
		delete _pUserMgr ;
		_pUserMgr = NULL ;
	}
}

// ��½������ƽ̨
bool CFdClientMgr::LoginClient( socket_t *sock, unsigned int seq, NodeLoginReq *req )
{
	// �Ƚ�ԭ�������ӿͻ���ɾ������Ҫ�ظ�����
	RemoveFdClient( sock ) ;

	// �����ʽ�������
	unsigned char result = RESULT_SUCCESS ;
	unsigned short group = ntohs( req->group ) ;
	unsigned int id = ntohl( req->id ) ;

	// ����FdClient
	CFdClient *p = NULL ;
	//ToDo: check login id
	if ( id == 0 || !(group & 0xf000) ) {
		result = RESULT_FAILED ;
	} else {
		// ���ΪMSG�ķ�����
		if ( group & FD_NODE_MSG ) {
			p = new CMsgServer ;
		} else { // ����Ϊ�ͻ���
			p = new CFdClient ;
		}
		p->_fd	   = sock ;
		p->_id     = id ;
		p->_group  = group ;
		p->_ip 	   = ( strlen((char*)req->addr.ip) == 0 ) ? "0.0.0.0" : (char*)req->addr.ip ;
		p->_port   = ntohs( req->addr.port ) ;
		sock->_ptr = p ;

		// ��ӹ��������
		if ( ! AddClient( p ) ) {
			delete p ;
			result = RESULT_FAILED ;
		}
	}

	DataBuffer buf ;
	// ����Ӧ���Ӧ
	_pBuilder->BuildLoginResp( buf, seq, result ) ;

	// ��ӳɹ�����
	if ( result == RESULT_SUCCESS &&
			_pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ) {
		if ( group & FD_NODE_MSG ) {
			// ����MSG��״̬Ϊ�ȴ���Ӧ
			((CMsgServer *)p)->SetWaitUser() ;
			// ���ΪMSG��Ҫ�׷��û��б�
			if ( ! DispatchUser( sock ) ) {
				// ����MSG��״̬Ϊ����״̬
				((CMsgServer *)p)->SetOnline() ;
				// ���û�û����ݾ�ֱ�ӷ�����
				return true ;
			}
			// ��Ҫ����MSG�����ӵ�֪ͨ���������ɹ�Ӧ���ٽ�һ������
		}
		// ��¼��־
		OUT_INFO( sock->_szIp, sock->_port, "NodeMgr" , "add %s node ip %s port %d , fd %d, id %d" ,
						(group&FD_NODE_MSG) ? "msg" : "client",  (char*)req->addr.ip, p->_port , sock->_fd , id ) ;

		return true ;
	} else {
		OUT_ERROR( sock->_szIp, sock->_port, "NodeMgr", "add %s node failed, ip %s , fd %d, id %d, result %d" ,
				(group&FD_NODE_MSG) ? "msg" : "client", (char*)req->addr.ip, sock->_fd , id , result ) ;
	}

	return false;
}

// ֪ͨ���MSG�Ĳ���
void CFdClientMgr::NotifyAddMsg( CFdClient *p )
{
	CVecClient vec ;
	_pGroupMgr->GetGroup( FD_NODE_STORE, vec ) ;
	_pGroupMgr->GetGroup( FD_NODE_WEB  , vec ) ;

	// ���û�нڵ�ֱ�ӷ��سɹ�
	if ( vec.empty() ) return ;

	// ȡ�ù����Ĵ洢��WEB�ڵ�
	AddrInfo addr ;
	safe_memncpy( (char*)addr.ip, p->_ip.c_str(), sizeof(addr.ip) ) ;
	addr.port = p->_port ;

	DataBuffer dbuf ;
	MsgData *msg = _pBuilder->BuildMsgChgReq( NODE_MSG_ADD, &addr, 1 ) ;
	_pBuilder->BuildMsgBuffer( dbuf, msg ) ;

	// ת��Ϊ��������ID��
	uint64_t msgid = netutil::strToAddr( p->_ip.c_str(), p->_port ) ;

	// ��ӵ��ȴ�������
	_pEnv->GetWaitGroup()->AddGroup( p->_fd, msg->seq, msg ) ;
	// Ⱥ��֪ͨ���й����Ľڵ�
	for ( int i = 0; i < (int)vec.size(); ++ i ) {
		CFdClient *pClient = vec[i] ;
		_pEnv->GetWaitGroup()->AddQueue( msg->seq, pClient->_fd ) ;
		if ( ! _pEnv->GetNodeSrv()->HandleData( pClient->_fd, dbuf.getBuffer(), dbuf.getLength() ) ){
			// ����ʧ�ܴӶ����л���
			_pEnv->GetWaitGroup()->DelQueue( msg->seq, pClient->_fd, false ) ;
		} else {
			// ֪ͨ���ڵ�ʱ��Ҫ�����MSG��Ӧ��ϵ
			_pMsgMgr->AddClient( msgid, pClient ) ;
		}
	}
	OUT_PRINT( NULL, 0, "NodeMgr", "handle NODE_MSG_ADD message , ip %s port %d" , p->_ip.c_str() , p->_port ) ;
	OUT_HEX( NULL, 0, "NodeMgr" , dbuf.getBuffer() , dbuf.getLength() ) ;
}

// ע����½ƽ̨
bool CFdClientMgr::LogoutClient( socket_t *sock, unsigned int seq )
{
	unsigned char result = RESULT_SUCCESS ;
	if ( ! RemoveFdClient( sock ) ) {
		result = RESULT_FAILED ;
	}

	DataBuffer buf ;
	// ������½��Ӧ
	_pBuilder->BuildLogoutResp( buf, seq, result ) ;

	return ( result == RESULT_SUCCESS &&
			_pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ) ;
}

// ��·����
bool CFdClientMgr::LinkTest( socket_t *sock, unsigned int seq , NodeLinkTestReq *req )
{
	// ��ȡ�õ�ǰFD����
	CFdClient *p = GetFdClient( sock ) ;
	if ( p == NULL )  {
		OUT_ERROR( sock->_szIp, sock->_port, "FdClientMgr" , "LinkTest fd %d", sock->_fd ) ;
		return false ;
	}

	p->_last = time(NULL) ;
	p->_ncar = ntohl( req->num ) ;

	DataBuffer buf ;
	_pBuilder->BuildLinkTestResp( buf, seq ) ;

	// ����������Ӧ
	return ( _pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ) ;
}

// ��������û���
bool CFdClientMgr::UserName( socket_t *sock, unsigned int seq )
{
	// ��ȡ�õ�ǰFD����
	CFdClient *pFd = GetFdClient( sock ) ;
	if ( pFd == NULL )  {
		OUT_ERROR( sock->_szIp, sock->_port, "FdClientMgr" , "UserName fd %d", sock->_fd ) ;
		return false ;
	}
	pFd->_last = time(NULL) ;

	CVecClient vec ;
	// ȡ�õ�ǰ��MSG����û���
	if ( ! _pMsgMgr->GetMsgList( vec ) ) {
		// ���û��MSG�Ͳ��ܻ�ȡ�û�����
		OUT_WARNING( NULL, 0, "FdClientMgr" , "UserName Msg List empty" ) ;
		// return false ;
	}

	char user[13] = {0};
	char pwd[9]   = {0} ;

	char szid[1024] = {0} ;
	sprintf( szid, "%u_%u_%llu", pFd->_id, pFd->_group, netutil::strToAddr( sock->_szIp, pFd->_port ) ) ;
	bool exist =  _pUserMgr->GetUser( szid , user, pwd ) ;

	UserInfo info ;
	safe_memncpy( info.user ,user , sizeof(info.user) ) ;
	safe_memncpy( info.pwd  ,pwd  , sizeof(info.pwd ) ) ;

	DataBuffer buf ;
	MsgData *p = _pBuilder->BuildUserNotifyReq( &info, 1, seq ) ;
	_pBuilder->BuildMsgBuffer( buf, p ) ;

	_pEnv->GetWaitGroup()->AddGroup( sock, seq, p ) ;

	// ���Ϊ�������û���Ҫ֪ͨ����MSG����,���û��MSG��ֱ�ӷ����û���
	if ( ! exist && ! vec.empty() ){
		for ( int i = 0; i < (int) vec.size(); ++ i ) {
			CFdClient *pClient = vec[i] ;
			// �첽��������ӵ����к���
			_pEnv->GetWaitGroup()->AddQueue( p->seq, pClient->_fd ) ;
			// printf( "add seq id %d fd %d\n" , p->seq, pClient->_fd ) ;
			// ���Ϊ����ӵ��û���Ҫ�㲥���д��ڵ�MSG
			if ( ! _pEnv->GetNodeSrv()->HandleData( pClient->_fd, buf.getBuffer(), buf.getLength() )  ){
				// �������ʧ����ֱ��ɾ��
				_pEnv->GetWaitGroup()->DelQueue( p->seq, pClient->_fd , false ) ;
			}
		}
	} else {  // �����ֱ�ӷ��ش��ڵ��û���
		// ����Ѵ��ֱ�ӵȴ���ȥ���ˣ��ɶ��лص�����֪ͨ�ڵ�
		_pEnv->GetWaitGroup()->DelQueue( seq, sock , true ) ;
	}
	return true ;
}

// �������MSG�б�
bool CFdClientMgr::GetMsgList( socket_t *sock, unsigned int seq )
{
	// ��ȡ�õ�ǰFD����
	CFdClient *p = GetFdClient( sock ) ;
	if ( p == NULL )  {
		OUT_ERROR( sock->_szIp, sock->_port, "FdClientMgr" , "GetMsgList fd %d", sock->_fd ) ;
		return false ;
	}
	p->_last = time(NULL) ;

	// ����������Ͳ���Ҫ����
	if ( !(p->_group & 0x7000) ) {
		OUT_ERROR( sock->_szIp, sock->_port, "FdClientMgr" , "GetMsgList fd %d group %d error" , sock->_fd, p->_group  ) ;
		return false ;
	}

	CVecClient vecMsg ;
	// ȡ������MSG
	if ( ! _pMsgMgr->GetMsgList(vecMsg) ) {
		OUT_ERROR( sock->_szIp, sock->_port, "CFdClientMgr" , "Get msg list empty fd %d" , sock->_fd ) ;
		return false ;
	}

	// �Ƴ�ԭ���е�MSG�Ĺ�ϵ
	_pMsgMgr->RemoveClient( p ) ;
	// ���ԭ�е�MSG�����б�
	p->ClearMsgClients() ;

	// �������BUF
	DataBuffer buf ;
	// ���Ϊ��һ�Ĵ洢����WEB�Ĵ�����Ҫ�������MSG�б�
	if ( p->_group ==  FD_NODE_STORE || p->_group == FD_NODE_WEB ) {
		// ȡ��MSG�ĸ���
		int size = (int) vecMsg.size() ;
		AddrInfo *addrs = new AddrInfo[size] ;
		for ( int i = 0; i < size; ++ i ) {
			CFdClient * tmp = vecMsg[i] ;
			safe_memncpy( (char*)addrs[i].ip, tmp->_ip.c_str() , sizeof(addrs[i].ip) ) ;
			addrs[i].port = tmp->_port ;
			// ��ӵ���Ӧ�Ŀͻ�������
			_pMsgMgr->AddClient( netutil::strToAddr( tmp->_ip.c_str(), tmp->_port ) , p ) ;
		}
		// ����MSG�ķ��������б�
		_pBuilder->BuildGetMsgResp( buf, seq, addrs, size ) ;

		delete [] addrs ;

	} else if ( p->_group & FD_NODE_PIPE ) { // ����ܵ���Ϊǰ�û�ѡһ�����õ�MSG����������д洢��
		// ȡ�õ�ǰ���ǰ�û�
		CVecClient vecgroup ;
		// ȡ�õ�ǰ���Ԫ��
		_pGroupMgr->GetGroup( p->_group, vecgroup ) ;

		CVecGroupIds ids ;
		_pGroupMgr->GetGroupIDs( FD_NODE_STORE, ids ) ;

		int min_store = 1024 , min_nostore = 1024 ;
		int index_store = 0, index_nostore = 0 ;
		int size = (int) vecMsg.size() ;
		for ( int i = 0 ; i < size; ++ i ) {
			CMsgServer *tmp = ( CMsgServer *) vecMsg[i] ;
			// ����ֲ�ǰ�û��ļ��㷨
			int nstore = 0 ;
			if ( ! ids.empty() ) {
				for ( int k = 0; k < (int)ids.size(); ++ k ) {
					nstore += tmp->GetGroupSize( ids[k] ) ;
				}
			}
			// ȡ�õ�ǰǰ�û�����
			int npipe = tmp->GetGroupSize( p->_group ) ;
			if ( nstore > 0 ) {  // �д洢��С����
				// ȡ�����пͻ�����
				if ( npipe < min_store ) {
					min_store   = npipe ;
					index_store = i ;
				}
			} else { // �޴洢����С����
				if ( npipe < min_nostore ) {
					min_nostore   = npipe ;
					index_nostore = i ;
				}
			}
		}

		// �����Ϊ�д洢��û�洢��������������д洢�Ĵ���
		int index = ( ! ids.empty() ) ? index_store : index_nostore ;

		// ȡ��FdClient����
		CFdClient *pFd =  vecMsg[index] ;

		AddrInfo addr ;
		safe_memncpy( addr.ip, pFd->_ip.c_str(), sizeof(addr.ip) ) ;
		addr.port = pFd->_port ;
		//  ȡ�ö�Ӧ����Ӧ
		_pBuilder->BuildGetMsgResp( buf, seq, &addr, 1 ) ;
		// ��ӵ���Ӧ�Ŀͻ�������
		_pMsgMgr->AddClient( netutil::strToAddr( pFd->_ip.c_str(), pFd->_port ) , p ) ;

	} else { // ����Ϊ����Ĵ洢�����ȷ�������ӵ�MSG���ٴη��为����Խ��ص�MSG
		float max = 0.0f ;
		int index = 0 , size = (int) vecMsg.size() ;
		// ����Ȩ����һ��Ȩ�����Ĵ���
		for ( int i = 0 ; i < size; ++ i ) {
			CMsgServer *tmp = (CMsgServer*)vecMsg[i] ;
			int  nstore = tmp->GetGroupSize( p->_group ) ;
			float value = ( nstore == 0 ) ? 1.0f : (  (float)tmp->_ncar / (float)(nstore * 10000000 ) ) ;
			if ( value > max ) {
				max   = value ;
				index = i ;
			}
		}

		// ȡ��FdClient����
		CFdClient *pFd =  vecMsg[index] ;

		AddrInfo addr ;
		safe_memncpy( addr.ip, pFd->_ip.c_str(), sizeof(addr.ip) ) ;
		addr.port = pFd->_port ;
		//  ȡ�ö�Ӧ����Ӧ
		_pBuilder->BuildGetMsgResp( buf, seq, &addr, 1 ) ;
		// ��ӵ���Ӧ�Ŀͻ�������
		_pMsgMgr->AddClient( netutil::strToAddr( pFd->_ip.c_str(), pFd->_port ) , p ) ;
	}

	// ���ط����Ƿ�ɹ�
	if ( ! _pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ) {
		// ����ʧ����Ҫ�������MSG�����ϵ
		_pMsgMgr->RemoveClient( p ) ;
		return false ;
	}
	return true ;
}

// ���������û����Ľ��
bool CFdClientMgr::ProcUserName( socket_t *sock, unsigned int seq , MsgData *p )  // ���������������û��������
{
	if ( p->buf.getLength() < (int)sizeof(UserInfo) ) {
		// ������ز���ȷֱ�ӷ���
		OUT_ERROR( NULL, 0, "FdClientMgr", "ProcUserName data length error" ) ;
		return false ;
	}

	// ��ȡ�õ�ǰFD����
	CFdClient *pFd = GetFdClient( sock ) ;
	if ( pFd == NULL )  {
		OUT_ERROR( sock->_szIp, sock->_port, "FdClientMgr" , "GetFdClient failed fd %d" , sock->_fd ) ;
		return false ;
	}
	pFd->_last = time(NULL) ;

	// ���ΪMSG��ֱ�ӷ�����
	if ( pFd->_group == FD_NODE_MSG ){
		// ����תΪMSG����
		CMsgServer *pmsg = (CMsgServer*) pFd ;
		// ���MSG�Ƿ�Ϊ�ȴ��׷�����״̬
		if ( pmsg->IsWaitUser() ) {
			pmsg->SetOnline() ;   // ��MSG��Ϊ����״̬
			NotifyAddMsg( pFd ) ; // ֪ͨ���е��ڵ���������ش�MSG
		}
		return true ;
	}

	// �û�������
	UserInfo info ;
	// �ӻ���ȡ�÷�����û���
	p->buf.fetchBlock( sizeof(NodeUserNotify), &info, sizeof(UserInfo) ) ;

	// ���������û�������Ӧ
	DataBuffer buf ;
	_pBuilder->BuildUserNameResp( buf, p->seq, &info, 1 ) ;

	// ���ض�Ӧ����
	return ( _pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ) ;
}

// ���²���δ�յ�������
bool CFdClientMgr::ResendMsg( socket_t *sock, MsgData *p , ListFd &fds )
{
	unsigned int count = _seqref.AddRef( p->seq ) ;
	// ����ط��δ�������ط��������Ͳ����ط���
	if ( count > MAX_RESEND_TIME ) {
		_seqref.DelRef( p->seq ) ;
		return false ;
	}

	// ��Ҫ���¿���һ������
	MsgData *temp = _pEnv->GetAllocMsg()->AllocMsg() ;
	temp->cmd     = p->cmd ;
	temp->seq	  = p->seq ;
	temp->buf.writeBlock( p->buf.getBuffer(), p->buf.getLength() ) ;

	DataBuffer buf ;
	_pBuilder->BuildMsgBuffer( buf, temp ) ;
	// ��ӵ��ȴ���������
	_pEnv->GetWaitGroup()->AddGroup( sock, temp->seq , temp ) ;

	// ���û��Ҫ���͵�FD��������FD����
	if ( fds.empty() ) {
		_pEnv->GetNodeSrv()->HandleData( sock, buf.getBuffer(), buf.getLength() ) ;
	} else {
		ListFd::iterator it ;
		// ���Ⱥ����Ϣ��ֱ��Ⱥ������
		for ( it = fds.begin(); it != fds.end(); ++ it ) {
			socket_t *nfd = (*it) ;
			_pEnv->GetWaitGroup()->AddQueue( temp->seq, nfd ) ;
			_pEnv->GetNodeSrv()->HandleData( nfd, buf.getBuffer(), buf.getLength() )  ;
		}
	}
	return true ;
}

// ���²����û��б�����
bool CFdClientMgr::DispatchUser( socket_t *sock )
{
	int count = _pUserMgr->GetSize() ;
	if ( count <= 0 )
		return false ;

	// ����ѳɹ�����û��账���û��б�
	UserInfo *pUsers = new UserInfo[count] ;
	_pUserMgr->GetUsers( pUsers, count ) ;
	// ���MSG��½�ɹ���Ҫ�׷��û���
	_pMsgMgr->DispatherUsers( sock, pUsers, count ) ;

	delete [] pUsers;

	return true ;
}

// �Ƴ��ͻ���
bool CFdClientMgr::RemoveFdClient( socket_t *sock )
{
	if ( sock == NULL || sock->_ptr == NULL )
		return false ;

	return RemoveClient( (CFdClient *)sock->_ptr ) ;
}

// ��⵱ǰFD�Ƿ����
bool CFdClientMgr::CheckFdClient( socket_t *sock )
{
	if ( sock == NULL )
		return false ;
	// ����Ƿ��Ѵ���
	return ( sock->_ptr != NULL ) ;
}

// ��ӿͻ���
bool CFdClientMgr::AddClient( CFdClient *p )
{
	share::RWGuard guard( _rwmutex , true ) ;
	{
		if ( p == NULL || p->_fd == NULL )
			return false ;

		// ��ӵ�������
		_fdqueue.push( p ) ;

		if ( p->_group & 0xf000 ) {
			// ���ΪMSG�����Ϊ������
			if ( p->_group== FD_NODE_MSG ) {
				_pMsgMgr->AddMsgServer( (CMsgServer*) p ) ;
			} else {
				// ��ӵ��������
				_pGroupMgr->AddGroup( p ) ;
			}
		}
		return true ;
	}
}

// �Ƴ��ͻ���
bool CFdClientMgr::RemoveClient( CFdClient *p )
{
	share::RWGuard guard( _rwmutex , true ) ;
	{
		// �ȴӶ������Ƴ�
		p = _fdqueue.erase( p ) ;
		if ( p->_fd != NULL ) {
			p->_fd->_ptr = NULL ;
		}
		if ( p->_group & 0xf000 ) {
			// ɾ����Ӧ����
			if ( p->_group != FD_NODE_MSG ) {
				_pGroupMgr->RemoveGroup( p ) ;
			}
			_pMsgMgr->RemoveClient( p ) ;
		}

		// ����ͷŶ���
		delete p ;

		return true ;
	}
}

CFdClient * CFdClientMgr::GetFdClient( socket_t *sock )
{
	share::RWGuard guard( _rwmutex, false ) ;
	{
		if ( sock == NULL || sock->_ptr == NULL )
			return NULL ;
		// ���ض�Ӧ��FD����
		return ( CFdClient *) sock->_ptr ;
	}
}

//////////////// �ڵ�������������н����� ////////////////
CNodeMgr::CNodeMgr(ISystemEnv *pEnv):_pEnv(pEnv)
{
	_builder    =  new CMsgBuilder(pEnv->GetAllocMsg());
	_clientmgr  =  new CFdClientMgr( pEnv , _builder ) ;
	// ���õȴ����еĻص�����
	_pEnv->GetWaitGroup()->SetNotify( this ) ;
}

CNodeMgr::~CNodeMgr()
{
	if ( _clientmgr != NULL ) {
		delete _clientmgr ;
		_clientmgr = NULL ;
	}
	if ( _builder != NULL ) {
		delete _builder ;
		_builder = NULL ;
	}
}

// ������������
void CNodeMgr::Process( socket_t *sock, const char *data, int len )
{
	if ( len < (int)sizeof(NodeHeader) ) {
		OUT_ERROR( sock->_szIp, sock->_port, "Node", "recv fd %d data len %d error" , sock->_fd, len ) ;
		return ;
	}

	NodeHeader *header = (NodeHeader *) (data) ;
	unsigned int mlen = ntohl( header->len ) ;
	// �������ݵ���ȷ��
	if ( (int)(mlen + sizeof(NodeHeader)) != len ) {
		OUT_ERROR( sock->_szIp, sock->_port, "Node", "recv fd %d data len %d error" , sock->_fd, len ) ;
		return ;
	}

	unsigned int   seq = ntohl( header->seq ) ;
	unsigned short cmd = ntohs( header->cmd ) ;
	switch( cmd ) {
	case NODE_CONNECT_REQ:
		_clientmgr->LoginClient( sock, seq, (NodeLoginReq*)(data+sizeof(NodeHeader))) ;
		break ;
	case NODE_DISCONN_REQ:
		_clientmgr->LogoutClient( sock, seq ) ;
		break ;
	case NODE_LINKTEST_REQ:
		_clientmgr->LinkTest( sock, seq, (NodeLinkTestReq*)(data+sizeof(NodeHeader))) ;
		break ;
	case NODE_USERNAME_REQ:
		_clientmgr->UserName( sock, seq ) ;
		break ;
	case NODE_GETMSG_REQ:
		_clientmgr->GetMsgList( sock, seq ) ;
		break ;
	case NODE_USERNOTIFY_RSP:
		// printf( "remove seq id %d fd %d\n" , seq, fd ) ;
	case NODE_MSGERROR_RSP:
	case NODE_MSGCHG_RSP:
		// �����첽��Ӧ����
		_pEnv->GetWaitGroup()->DelQueue( seq, sock, true ) ;
		break ;
	}
}

// ���洫��Ķ����¼�
void CNodeMgr::Close( socket_t *sock )
{
	// �Ƴ�����
	_clientmgr->RemoveFdClient( sock ) ;
}

// �������ص�����
void CNodeMgr::NotifyMsgData( socket_t *sock , MsgData *p , ListFd &fds, unsigned int op )
{
	// ���ݲ�ͬ����Ϣ��ͬ�Ĵ���
	switch( p->cmd ) {
	case NODE_USERNOTIFY_REQ:  // �׷��û���
		 // ���ΪMSG��½�����û��б���Ҫ���´�����
		if ( fds.empty() && op == MSG_TIMEOUT ) {
			_clientmgr->ResendMsg( sock , p, fds ) ; // ���²�������
		} else {  // ���������MSG�׷������û������Ӧ�÷����û���
			_clientmgr->ProcUserName( sock, p->seq , p ) ;
		}
		break ;
	case NODE_MSGERROR_REQ:
		break ;
	case NODE_MSGCHG_REQ:
		if ( op == MSG_TIMEOUT ) {
			// �ط�����
			_clientmgr->ResendMsg( sock, p, fds ) ;
		}
		break ;
	}
	OUT_INFO( sock->_szIp, sock->_port, "Notify" , "%s fd %d, cmd %04x , seq %d" ,
			(op==MSG_TIMEOUT) ? "timeout" : "success" , sock->_fd, p->cmd, p->seq ) ;
}
