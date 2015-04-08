/*
 * inodeface.h
 *
 *  Created on: 2011-11-10
 *      Author: humingqing
 */

#ifndef __INODEFACE_H__
#define __INODEFACE_H__

#include <list>
#include <databuffer.h>
#include <SocketHandle.h>
// FD���б�
typedef std::list<socket_t*> ListFd ;
// ����һ����Ϣ����
struct MsgData
{
	unsigned short cmd ;
	unsigned int   seq ;
	DataBuffer     buf ;
};


#define MSG_COMPLETE   0   // ��Ӧ�������
#define MSG_TIMEOUT    1   // ��Ӧ���ݳ�ʱ

// ����֪ͨ����
class IMsgNotify
{
public:
	virtual ~IMsgNotify() {} ;
	// ������ɾ��֪ͨ���ݶ���
	virtual void NotifyMsgData( socket_t *sock , MsgData *p , ListFd &fd_list, unsigned int op ) = 0 ;
};

// MSG�ڴ�������
class IAllocMsg
{
public:
	// MSG �ڴ濪�ٺͻ��ն���
	virtual ~IAllocMsg() {}
	// �����ڴ�
	virtual MsgData * AllocMsg() = 0 ;
	// �����ڴ�
	virtual void FreeMsg( MsgData *p ) = 0 ;
};

// �ȴ����ж���Ľӿ�
class IWaitGroup
{
public:
	virtual ~IWaitGroup() {} ;
	// ��ʼ��
	virtual bool Init( void ) = 0 ;
	// ��ʼ�߳�
	virtual bool Start( void ) = 0  ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
	// ����������
	virtual bool AddGroup( socket_t *sock, unsigned int seq, MsgData *msg ) = 0 ;
	// ��ӵ���Ӧ��������
	virtual bool AddQueue( unsigned int seq, socket_t *sock ) = 0 ;
	// ɾ����Ӧ��ֵ
	virtual bool DelQueue( unsigned int seq, socket_t *sock , bool bclear ) = 0 ;
	// ȡ��û����Ӧ��FD
	virtual int  GetCount( unsigned int seq ) = 0 ;
	// ȡ�ö�Ӧ��FD��LISTֵ
	virtual bool GetList( unsigned int seq, ListFd &fds ) = 0 ;
	// �������ݻ��ն���
	virtual void SetNotify( IMsgNotify *pNotify ) = 0 ;
};

#endif /* INODEFACE_H_ */
