/*
 * msgpack.h
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 *  Э�����ݽ�������
 */

#ifndef __MSGPACK_H__
#define __MSGPACK_H__

#include <Ref.h>
#include <packer.h>

#pragma pack(1)

#define MSG_VERSION  0x0001              // ��Ϣ�汾��
// ��Ϣ����ͷ
class CMsgHeader
{
public:
	CMsgHeader( )
	{
		_ver = MSG_VERSION;
		_type = 0;
		_seq = _len = 0;
	}
	~CMsgHeader( ){}

	// �������
	bool UnPack( CPacker *pack )
	{
		if ( ( int ) sizeof(CMsgHeader) > pack->GetReadLen() )
			return false;

		_ver  = pack->readShort();
		_type = pack->readShort();
		_seq  = pack->readInt();
		_len  = pack->readInt();

		if ( ( int ) _len + ( int ) sizeof(CMsgHeader) > pack->GetReadLen() )
			return false;
		return true;
	}

	// �������
	void Pack( CPacker *pack )
	{
		pack->writeShort( _ver );
		pack->writeShort( _type );
		pack->writeInt( _seq );
		pack->writeInt( _len );
	}

public:
	// Э��İ汾��
	uint16_t _ver;
	// Э�������
	uint16_t _type;
	//��Э������
	uint32_t _seq;
	// ���ݳ���
	uint32_t _len;
};

// ���ݽ������ӿ�
class IPacket : public share::Ref
{
public:
	IPacket() {}
	// ����������
	virtual ~IPacket(){}
	// �����Ϣ��
	virtual void Body( CPacker *body ) = 0 ;
	// ���������
	virtual bool UnBody( CPacker *pack ) = 0 ;

public:
	// �������
	bool UnPack( CPacker *pack )
	{
		if ( ! _header.UnPack( pack ) )
			return false;

		return UnBody( pack );
	}

	// �������
	void Pack( CPacker *pack )
	{
		_header.Pack( pack );
		Body( pack );
		int len = pack->getLength() - ( int ) sizeof(CMsgHeader);
		pack->fillInt32( len, 8 );
	}

public:
	// ����������ͷ
	CMsgHeader _header;
};

#pragma pack()

// �Զ��ͷ����ö���
class CAutoRelease
{
public:
	CAutoRelease( share::Ref *ref )
	{
		_ref = ref;
	}
	~CAutoRelease( )
	{
		if ( _ref != NULL ) {
			_ref->Release();
			_ref = NULL;
		}
	}
private:
	share::Ref *_ref;
};

#endif /* MSGPARSER_H_ */
