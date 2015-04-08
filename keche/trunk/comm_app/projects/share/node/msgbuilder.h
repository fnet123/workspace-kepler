/*
 * msgbuilder.h
 *
 *  Created on: 2011-11-8
 *      Author: humingqing
 */

#ifndef __MSGBUILDER_H__
#define __MSGBUILDER_H__

#include <inodeface.h>
#include <nodeheader.h>
#include <Mutex.h>

// ��Ϣ��������
class CMsgBuilder
{
public:
	CMsgBuilder(IAllocMsg *pAlloc) ;
	~CMsgBuilder() ;

	// ������½����
	MsgData *BuildLoginReq( unsigned int id, unsigned short group, AddrInfo &info ) ;
	// �����˳���½����
	MsgData *BuildLogoutReq( void ) ;
	// ���ض�Ӧ�Ĵ������߳�����
	MsgData *BuildLinkTestReq( unsigned int num ) ;
	// ���ͻ�ȡ�û���������
	MsgData *BuildUserNameReq( void ) ;
	// ��ȡ���õ�MSG���б�
	MsgData *BuildGetmsgReq( void ) ;
	// �����û��б�֪ͨ��Ϣ
	MsgData *BuildUserNotifyReq( UserInfo *p, int count , unsigned int seq ) ;
	// ����MSG��������Ϣ
	MsgData *BuildMsgChgReq( unsigned char op, AddrInfo *p, int count ) ;

	// ������½�ɹ���Ӧ
	void BuildLoginResp( DataBuffer &buf, unsigned int seq, unsigned char result ) ;
	// �����˳���½��Ӧ
	void BuildLogoutResp( DataBuffer &buf, unsigned int seq , unsigned char result ) ;
	// ����������Ӧ��
	void BuildLinkTestResp( DataBuffer &buf, unsigned int seq ) ;
	// ���������û�����Ӧ
	void BuildUserNameResp( DataBuffer &buf, unsigned int seq, UserInfo *p , int count ) ;
	// ����ȡ�÷�����
	void BuildGetMsgResp( DataBuffer &buf, unsigned int seq, AddrInfo *p, int count ) ;
	// �����û�֪ͨ��Ӧ
	void BuildUserNotifyResp( DataBuffer &buf, unsigned int seq , unsigned short success ) ;
	// ����MSG������Ӧ
	void BuildMsgChgResp( DataBuffer &buf, unsigned int seq, unsigned char result ) ;
	// �������͵���Ϣ
	void BuildMsgBuffer( DataBuffer &buf, MsgData *p ) ;
	// �ͷ�����
	void FreeMsgData( MsgData *p ) ;

private:
	// ȡ�����
	unsigned int GetSequeue( void ) ;

private:
	// ��������
	IAllocMsg	 *_pAlloc ;
	// ������
	share::Mutex  _mutex ;
	// ���к�
	unsigned int  _seqid ;
};

#endif /* NODEBUILDER_H_ */
