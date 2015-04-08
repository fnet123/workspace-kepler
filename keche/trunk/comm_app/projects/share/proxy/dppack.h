/*
 * dppack.h
 *
 *  Created on: 2012-12-26
 *      Author: humingqing
 */

#ifndef __DPPACK_H_
#define __DPPACK_H_

#include <packer.h>
#include <packfactory.h>

#pragma pack(1)

// redis�д洢���ķ����б�key
#define CACHE_KEY 		"lbs.dp_msg_list"
#define ROLE_PIPE 		0x01  // �ܵ�
#define ROLE_SAVE 		0x02  // �洢

#define PHONE_LEN   	12  // �ֻ��ŵĳ���
#define USERNAME_LEN    32  // �û�������
#define PASSWORD_LEN    16  // ����ĳ���

#define CMD_LOGIN_REQ       0x0001  // ��½����
#define CMD_LOGIN_RSP       0x8001  // ��½��Ӧ
#define CMD_ACTIVE_REQ      0x0002  // ��������
#define CMD_ACTIVE_RSP      0x8002  // ������Ӧ
#define CMD_SUBSCRIBE       0x0003  // �����б�
#define CMD_TRANSFER        0x0004  // ����͸��

// ��½����
class CDpLoginReq: public IPacket
{
public:
	CDpLoginReq( uint32_t seq = 0 ) {
		_header._type = CMD_LOGIN_REQ ;
		_header._seq  = seq ;
	};
	~CDpLoginReq(){};
	// ���������
	bool UnBody( CPacker *pack ) {
		if ( pack->readBytes( _name, USERNAME_LEN ) <= 0 )
			return false ;
		if ( pack->readBytes( _pass, PASSWORD_LEN ) <= 0 )
			return false ;
		_type = pack->readByte() ;
		return true ;
	} ;

	void Body( CPacker *pack ) {
		pack->writeBytes( _name , USERNAME_LEN ) ;
		pack->writeBytes( _pass , PASSWORD_LEN ) ;
		pack->writeByte( _type ) ;
	}

public:
	uint8_t      _name[USERNAME_LEN+1];
	uint8_t      _pass[PASSWORD_LEN+1];
	uint8_t      _type;
};

// ��½��Ӧ
class CDpLoginRsp: public IPacket
{
public:
	CDpLoginRsp( uint32_t seq = 0 ){
		_header._type = CMD_LOGIN_RSP;
		_header._seq  = seq ;
		_result       = 0 ;
	}
	~CDpLoginRsp(){};
	// д���½�ɹ����Ӧ��
	void Body( CPacker *pack ) {
		pack->writeByte( _result ) ;
	}
	bool UnBody( CPacker *pack ) {
		_result = pack->readByte() ;
		return true ;
	}
public:
	uint8_t   _result ;
};

// ��������
class CDpActiveReq: public IPacket
{
public:
	CDpActiveReq( uint32_t seq= 0 ) {
		_header._type = CMD_ACTIVE_REQ;
		_header._seq  = seq ;
	}
	~CDpActiveReq() {}

	void Body( CPacker *pack ) {}
	bool UnBody( CPacker *pack ) {
		return true ;
	}
};

// ������Ӧ
class CDpActiveRsp: public IPacket
{
public:
	CDpActiveRsp( uint32_t seq= 0 ) {
		_header._type = CMD_ACTIVE_RSP;
		_header._seq  = seq ;
	}
	~CDpActiveRsp() {}

	void Body( CPacker *pack ) {}
	bool UnBody( CPacker *pack ) {
		return true ;
	}
};

// ����������
class CDpSubscribeReq: public IPacket
{
	typedef std::vector<CQString> SubList;
public:
	CDpSubscribeReq( uint32_t seq= 0 ) {
		_header._type = CMD_SUBSCRIBE ;
		_header._seq  = seq ;
		_size		  = 0 ;
	}
	~CDpSubscribeReq(){}

	void Body( CPacker *pack ) {
		_size = _vec.size() ;

		pack->writeShort( _size ) ;

		if ( _size == 0 )
			return ;
		// �����ĵĳ����ֻ���
		for ( int i = 0; i < _size; ++ i ){
			CQString &tmp = _vec[i] ;
			pack->writeFix(
					tmp.GetBuffer(),
					tmp.GetLength(),
					PHONE_LEN ) ;
		}
	}

	bool UnBody( CPacker *pack ) {
		_size = pack->readShort() ;
		if ( _size == 0 )
			return false ;

		for ( int i = 0; i < _size; ++ i ) {
			char phone[PHONE_LEN+1] = {0} ;
			pack->readBytes( phone, PHONE_LEN ) ;
			_vec.push_back( phone ) ;
		}

		return true ;
	}

public:
	// ���ݸ���
	uint16_t    _size ;
	// �����б�
	SubList 	_vec ;
};

// ����͸��������
class CDpTransferReq : public IPacket
{
public:
	CDpTransferReq( uint32_t seq= 0 ) {
		_header._type = CMD_TRANSFER ;
		_header._seq  = seq ;
		_transtype    = 0 ;
		memset( _phone, 0, sizeof(_phone) ) ;
	}
	~CDpTransferReq(){}

	void Body( CPacker *pack ) {
		pack->writeBytes( _phone, PHONE_LEN ) ;
		pack->writeInt8( _transtype ) ;
		pack->writeString( _data ) ;
	}

	bool UnBody( CPacker *pack ) {
		pack->readBytes( _phone, PHONE_LEN ) ;
		_transtype = pack->readInt8() ;
		pack->readString( _data ) ;
		return true ;
	}

public:
	uint8_t  _phone[PHONE_LEN+1] ;
	uint8_t  _transtype ;
	CQString _data ;
};

class CDPUnPackMgr : public IUnPackMgr
{
public:
	CDPUnPackMgr(){}
	~CDPUnPackMgr(){}

	// ʵ�����ݽ���ӿڷ���
	IPacket * UnPack( unsigned short msgtype, CPacker &pack )
	{
		IPacket *msg = NULL ;
		switch( msgtype )
		{
		case CMD_LOGIN_REQ:
			msg = UnPacket<CDpLoginReq>( pack, "login request" ) ;
			break ;
		case CMD_LOGIN_RSP:
			msg = UnPacket<CDpLoginRsp>( pack, "login response" ) ;
			break ;
		case CMD_ACTIVE_REQ:
			msg = UnPacket<CDpActiveReq>( pack, "active request" ) ;
			break ;
		case CMD_ACTIVE_RSP:
			msg = UnPacket<CDpActiveRsp>( pack, "active response" ) ;
			break ;
		case CMD_SUBSCRIBE:
			msg = UnPacket<CDpSubscribeReq>( pack, "subscribe request" ) ;
			break ;
		case CMD_TRANSFER:
			msg = UnPacket<CDpTransferReq>( pack, "transfer request" ) ;
			break ;
		default:
			break ;
		}
		return msg ;
	}
};

#pragma pack()

#endif /* DPPACK_H_ */
