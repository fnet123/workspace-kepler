/*
 * packfactory.h
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 */

#ifndef __PACKFACTORY_H__
#define __PACKFACTORY_H__

#include "msgpack.h"

// ͨ�ý�������ģ����
template <typename T>
IPacket * UnPacket( CPacker &pack, const char *name )
{
	T *req = new T;
	if ( ! req->UnPack( &pack ) ) {
		delete req ;
		return NULL ;
	}
	req->AddRef() ;
	return req ;
}

// ����Э�������
class IUnPackMgr
{
public:
	virtual ~IUnPackMgr() {} ;
	// ʵ�����ݽ���ӿڷ���
	virtual IPacket * UnPack( unsigned short msgtype, CPacker &pack ) = 0 ;
};

// ���в�����
class CSequeueGen;
class CPackFactory
{
public:
	// ��ʼ��������ʱ��Ҫ�����Ӧ��ʵ�ֽ��
	CPackFactory( IUnPackMgr *packmgr ) ;
	// ����������
	~CPackFactory() ;
	// �������
	IPacket * UnPack( const char *data, int len ) ;
	// �������
	void Pack( IPacket *packet , CPacker &pack ) ;
	// ȡ�����
	unsigned int GetSequeue( void ) ;

private:
	// ���ݽ������
	IUnPackMgr 	*_packmgr ;
	// ��Ų�������
	CSequeueGen *_seqgen ;
};


#endif /* PACKFACTORY_H_ */
