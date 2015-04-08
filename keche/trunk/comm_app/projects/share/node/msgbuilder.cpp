/*
 * nodebuilder.cpp
 *
 *  Created on: 2011-11-8
 *      Author: humingqing
 */
#include "msgbuilder.h"
#include <tools.h>

CMsgBuilder::CMsgBuilder(IAllocMsg *pAlloc): _pAlloc( pAlloc )
{
	_seqid = 0 ;
}

CMsgBuilder::~CMsgBuilder()
{

}


// ������½����
MsgData *CMsgBuilder::BuildLoginReq( unsigned int id, unsigned short group, AddrInfo &info )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_CONNECT_REQ ;
	msg->seq = GetSequeue() ;

	NodeLoginReq body ;
	body.id        = htonl( id ) ;
	body.group     = htons( group ) ;
	body.addr.port = htons( info.port ) ;
	safe_memncpy( body.addr.ip , info.ip, sizeof(info.ip) ) ;
	msg->buf.writeBlock( &body, sizeof(body) ) ;

	return msg ;
}

// �����˳���½����
MsgData *CMsgBuilder::BuildLogoutReq( void )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_DISCONN_REQ ;
	msg->seq = GetSequeue() ;
	return msg ;
}

// ���ض�Ӧ�Ĵ������߳�����
MsgData *CMsgBuilder::BuildLinkTestReq( unsigned int num )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_LINKTEST_REQ ;
	msg->seq = GetSequeue() ;
	msg->buf.writeInt32( num ) ;
	return msg ;
}

// ���ͻ�ȡ�û���������
MsgData *CMsgBuilder::BuildUserNameReq( void )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_USERNAME_REQ ;
	msg->seq = GetSequeue() ;
	return msg ;
}

// ��ȡ���õ�MSG���б�
MsgData *CMsgBuilder::BuildGetmsgReq( void )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_GETMSG_REQ ;
	msg->seq = GetSequeue() ;
	return msg ;
}

// ����MSG��������Ϣ
MsgData* CMsgBuilder::BuildMsgChgReq( unsigned char op, AddrInfo *p, int count )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_MSGCHG_REQ ;
	msg->seq = GetSequeue() ;

	NodeMsgChg body ;
	body.op   = op ;
	body.num  = htons( count ) ;
	msg->buf.writeBlock( &body, sizeof(body) ) ;

	for ( int i = 0; i < count; ++ i ) {
		msg->buf.writeBlock( p[i].ip,  sizeof(p[i].ip) ) ;
		msg->buf.writeInt16( p[i].port ) ;
	}

	return msg ;
}

// �����û��б�֪ͨ��Ϣ
MsgData * CMsgBuilder::BuildUserNotifyReq( UserInfo *p, int count, unsigned int seq )
{
	// ͳһ�����ڴ�
	MsgData *msg = _pAlloc->AllocMsg() ;
	msg->cmd = NODE_USERNOTIFY_REQ ;
	msg->seq = (seq == 0 ) ? GetSequeue() : seq ;

	NodeUserNotify body ;
	body.num  = htons( count ) ;
	msg->buf.writeBlock( &body, sizeof(body) ) ;

	for ( int i = 0; i < count; ++ i ) {
		msg->buf.writeBlock( p[i].user,  sizeof(p[i].user) ) ;
		msg->buf.writeBlock( p[i].pwd ,  sizeof(p[i].pwd ) ) ;
	}
	return msg ;
}

// ������½�ɹ���Ӧ
void CMsgBuilder::BuildLoginResp( DataBuffer &buf, unsigned int seq, unsigned char result )
{
	MsgData msg ;
	msg.cmd = NODE_CONNECT_RSP ;
	msg.seq = seq ;
	NodeLoginRsp rsp ;
	rsp.result = result ;
	msg.buf.writeBlock( &rsp, sizeof(rsp) ) ;

	BuildMsgBuffer( buf, &msg ) ;
}

// �����˳���½��Ӧ
void CMsgBuilder::BuildLogoutResp( DataBuffer &buf, unsigned int seq , unsigned char result )
{
	MsgData msg ;
	msg.cmd = NODE_DISCONN_RSP ;
	msg.seq = seq ;
	NodeLogoutRsp rsp ;
	rsp.result = result ;
	msg.buf.writeBlock( &rsp, sizeof(rsp) ) ;

	BuildMsgBuffer( buf, &msg ) ;
}

// ����������Ӧ��
void CMsgBuilder::BuildLinkTestResp( DataBuffer &buf, unsigned int seq )
{
	MsgData msg ;
	msg.cmd = NODE_LINKTEST_RSP ;
	msg.seq = seq ;

	BuildMsgBuffer( buf, &msg ) ;
}

// ���������û�����Ӧ
void CMsgBuilder::BuildUserNameResp( DataBuffer &buf, unsigned int seq, UserInfo *p , int count )
{
	MsgData msg ;
	msg.cmd = NODE_USERNAME_RSP ;
	msg.seq = seq ;

	if ( count > 0 && p != NULL ) {
		NodeUserNameRsp rsp ;
		safe_memncpy( rsp.user.user, p->user, sizeof(p->user) ) ;
		safe_memncpy( rsp.user.pwd , p->pwd , sizeof(p->pwd ) ) ;
		msg.buf.writeBlock( &rsp, sizeof(rsp) ) ;
	}
	BuildMsgBuffer( buf, &msg ) ;
}

// ����ȡ�÷�����
void CMsgBuilder::BuildGetMsgResp( DataBuffer &buf, unsigned int seq, AddrInfo *p, int count )
{
	MsgData msg ;
	msg.cmd = NODE_GETMSG_RSP ;
	msg.seq = seq ;

	NodeGetMsgRsp rsp ;
	rsp.num = htons(count) ;
	msg.buf.writeBlock( &rsp, sizeof(rsp) ) ;

	if ( count > 0 && p != NULL ) {
		for ( int i = 0; i < count; ++ i ) {
			msg.buf.writeBlock( p[i].ip, sizeof(p[i].ip) ) ;
			msg.buf.writeInt16( p[i].port ) ;
		}
	}
	BuildMsgBuffer( buf, &msg ) ;
}

// �����û�֪ͨ��Ӧ
void CMsgBuilder::BuildUserNotifyResp( DataBuffer &buf, unsigned int seq , unsigned short success )
{
	MsgData msg;
	msg.cmd = NODE_USERNOTIFY_RSP ;
	msg.seq = seq ;
	msg.buf.writeInt16( success ) ;
	BuildMsgBuffer( buf, &msg ) ;
}

// ����MSG������Ӧ
void CMsgBuilder::BuildMsgChgResp( DataBuffer &buf, unsigned int seq, unsigned char result )
{
	MsgData msg;
	msg.cmd = NODE_MSGCHG_RSP ;
	msg.seq = seq ;
	msg.buf.writeInt8( result ) ;
	BuildMsgBuffer( buf, &msg ) ;
}

// ȡ�����
unsigned int CMsgBuilder::GetSequeue( void )
{
	share::Guard guard( _mutex ) ;
	{
		return ++ _seqid ;
	}
}

// �������͵���Ϣ
void CMsgBuilder::BuildMsgBuffer( DataBuffer &buf, MsgData *p )
{
	NodeHeader header ;
	safe_memncpy( (char*)header.tag, NODE_CTFO_TAG , sizeof(header.tag)) ;
	header.cmd = htons( p->cmd ) ;
	header.seq = htonl( p->seq ) ;
	header.len = htonl( p->buf.getLength() ) ;

	buf.writeBlock( &header, sizeof(header) ) ;
	if ( p->buf.getLength() > 0 ) {
		buf.writeBlock( p->buf.getBuffer(), p->buf.getLength() ) ;
	}
}

// �ͷ�����
void CMsgBuilder::FreeMsgData( MsgData *p )
{
	_pAlloc->FreeMsg( p ) ;
}
