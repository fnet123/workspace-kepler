/*
 * waitobjmgr.h
 *
 *  Created on: 2012-9-18
 *      Author: humingqing
 */

#ifndef __WAITOBJMGR_H__
#define __WAITOBJMGR_H__

#include <map>
#include <Monitor.h>
#include <databuffer.h>

struct _waitobj
{
	unsigned char  _result ;
	DataBuffer 	   _inbuf  ;
	DataBuffer 	  *_outbuf ;
	share::Monitor _monitor ;
};

class CWaitObjMgr
{
	class CSequeue
	{
	public:
		CSequeue():_seq(0){}
		~CSequeue(){}
		// ȡ�����
		unsigned int gensequeue( void ) {
			share::Guard guard( _mutex ) ;
			return ++ _seq ;
		}

	private:
		share::Mutex  _mutex ;
		unsigned int  _seq ;
	};
	typedef std::map<unsigned int, _waitobj*> CMapObj ;
public:
	CWaitObjMgr() ;
	~CWaitObjMgr() ;

	// ������Ŷ���
	_waitobj * AllocObj( unsigned int seq , DataBuffer *out = NULL ) ;
	// ���¶�������
	void ChangeObj( unsigned int seq, unsigned char result, void *data, int len ) ;
	// �Ƴ�����
	void RemoveObj( unsigned int seq ) ;
	// ����ȡ����Ŷ���
	unsigned int GenSequeue( void ) { return _genseq.gensequeue(); }
	// ֪ͨ���ж�����
	void NotfiyAll( void ) ;

private:
	// �������ж���
	void Clear( void ) ;

private:
	// ������ɶ���
	CSequeue  	  _genseq ;
	// ���ݶ���
	CMapObj   	  _mapobj ;
	// ���������
	share::Mutex  _mutex ;
};


#endif /* WAITOBJMGR_H_ */
