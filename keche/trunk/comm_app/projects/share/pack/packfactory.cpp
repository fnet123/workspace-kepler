/*
 * packfactory.cpp
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 */

#include "packfactory.h"
#include <comlog.h>
#include <Mutex.h>

// ���в�����
class CSequeueGen
{
public:
	CSequeueGen():_seq(0) {};
	~CSequeueGen(){};
	// ȡ�����
	unsigned int GetSequeue( void ) {
		share::Guard guard( _mutex ) ;
		return ++ _seq ;
	}

private:
	// ͬ��������
	share::Mutex  _mutex ;
	// ���ж���
	unsigned int  _seq ;
};

CPackFactory::CPackFactory( IUnPackMgr *packmgr) : _packmgr(packmgr)
{
	_seqgen = new CSequeueGen;
}

CPackFactory::~CPackFactory()
{
	if ( _seqgen != NULL ) {
		delete _seqgen ;
		_seqgen = NULL ;
	}
}

// �������
IPacket * CPackFactory::UnPack( const char *data, int len )
{
	if ( len < (int)sizeof(CMsgHeader) ) {
		OUT_ERROR( NULL, 0, "Pack", "recv data length %d too short", len ) ;
		return NULL ;
	}

	CPacker pack( data, len ) ;
	unsigned short msg_ver  = pack.readShort() ;
	if ( msg_ver != MSG_VERSION ) {
		OUT_ERROR( NULL, 0, "Pack", "recv data msg version error, msg version %d" , msg_ver ) ;
		OUT_HEX( NULL , 0, "Pack", data, len ) ;
		return NULL ;
	}

	unsigned short msg_type = pack.readShort() ;
	pack.seekRead(0) ;  // ����λ�ù���

	IPacket *msg = _packmgr->UnPack( msg_type, pack ) ;
	// �������
	if ( msg == NULL ) {
		OUT_ERROR( NULL, 0, "Pack", "unpack data length:%d" , len ) ;
		OUT_HEX( NULL, 0, "Pack", data, len ) ;
	}

	return msg;

}

// �������
void CPackFactory::Pack( IPacket *packet , CPacker &pack )
{
	// ���û���к�ֱ���Լ�����
	if ( packet->_header._seq == 0 ) {
		packet->_header._seq = _seqgen->GetSequeue() ;
	}
	// �����Ϣ����
	packet->Pack( &pack ) ;
}

// ȡ�����
unsigned int CPackFactory::GetSequeue( void )
{
	return _seqgen->GetSequeue() ;
}
