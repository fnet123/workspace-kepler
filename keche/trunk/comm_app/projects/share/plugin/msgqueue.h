/*
 * msgqueue.h
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 *  ��Ϣ���д���
 */

#ifndef __MSGQUEUE_H__
#define __MSGQUEUE_H__

#include <map>
#include <string>
#include <Thread.h>
#include <Monitor.h>
#include <time.h>
#include "msgpack.h"
#include <TQueue.h>

// ��Ϣ�ص�����
class IMsgCaller
{
public:
	virtual ~IMsgCaller(){}
	// ��Ϣ��ʱ����
	virtual void OnTimeOut( unsigned int seq, unsigned int fd, unsigned int cmd, const char *id, IPacket *msg ) = 0 ;
};

// ��Ϣ����
class CMsgQueue
{
public:
	struct _MsgObj
	{
		unsigned int _fd ;
		unsigned int _cmd;
		unsigned int _seq;
		std::string	 _id;
		IPacket     *_msg;
		time_t		 _now;
		_MsgObj     *_next;
		_MsgObj     *_pre ;
	};

	typedef std::map<unsigned int,_MsgObj *>  CMapObj ;
public:
	CMsgQueue( IMsgCaller *caller ) ;
	~CMsgQueue() ;
	// ��Ӷ���
	bool AddObj( unsigned int seq, unsigned int fd, unsigned int cmd, const char *id, IPacket *msg ) ;
	// ɾ������
	bool Remove( unsigned int seq ) ;
	// ȡ�ö�Ӧ����
	_MsgObj * GetObj( unsigned int seq ) ;
	// �ͷŶ���
	void  FreeObj( _MsgObj *obj ) ;
	//  ��ⳬʱ�Ķ���
	void Check( int timeout ) ;

private:
	// �Ƴ���Ӧ��ֵ
	void RemoveValue( unsigned int seq , bool callback ) ;
	// �������
	void Clear( void ) ;

private:
	// ͬ��������
	share::Mutex 		 _mutex ;
	// �ȴ�������Ӧ����
	CMapObj	 			 _index ;
	// ʱ����������
	TQueue<_MsgObj> 	 _queue ;
	// ��Ϣ�ص�����
	IMsgCaller		   * _caller ;
};


#endif /* MSGQUEUE_H_ */
