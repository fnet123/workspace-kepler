/*
 * authpack.h
 *
 *  Created on: 2012-6-1
 *      Author: humingqing
 *  Э�����ݽ�������
 */

#ifndef __AUTHMSGPACK_H__
#define __AUTHMSGPACK_H__

#include <packer.h>
#include <packfactory.h>

#define SERVER_LOGIN_REQ  		0x1001   // �����֤��½
#define SERVER_LOGIN_RSP  		0x8001
#define SERVER_LOOP_REQ			0x1002   // ��·ά����Ӧ
#define SERVER_LOOP_RSP			0x8002
#define SERVER_REGISTER_REQ     0x1003   // ����ע������
#define SERVER_REGISTER_RSP 	0x8003   // Ӧ��
#define SERVER_TERMAUTH_REQ		0x1004   // ��Ȩ����
#define SERVER_TERMAUTH_RSP		0x8004
#define SERVER_CKDRIVER_REQ		0x1005   // ˾�����ʶ��
#define SERVER_CKDRIVER_RSP		0x8005
#define SERVER_TERMLOGO_REQ		0x1006 	 // �ն�ע��
#define SERVER_TERMLOGO_RSP		0x8006

// ��½����
class CLoginReq: public IPacket
{
public:
	CLoginReq( uint32_t seq = 0 ) {
		_header._type = SERVER_LOGIN_REQ ;
		_header._seq  = seq ;
	};
	~CLoginReq(){};
	// ���������
	bool UnBody( CPacker *pack ) {
		if ( pack->readBytes( _uid, 32 ) == 0 )
			return false ;
		return true ;
	} ;

	void Body( CPacker *pack ) {
		pack->writeBytes( _uid, 32 ) ;
	}

public:
	// ��½�û�ID
	uint8_t   _uid[32] ;
};

// ��½��Ӧ
class CLoginRsp: public IPacket
{
public:
	CLoginRsp( uint32_t seq = 0 ){
		_header._type = SERVER_LOGIN_RSP ;
		_header._seq  = seq ;
		_result       = 0 ;
	}
	~CLoginRsp(){};
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
class CLoopReq: public IPacket
{
public:
	CLoopReq( uint32_t seq = 0 ){
		_header._type = SERVER_LOOP_REQ ;
		_header._seq  = seq ;
	}
	~CLoopReq(){};
	void Body( CPacker *pack ) {}
	bool UnBody( CPacker *pack ) { return true; }
};

// ��������
class CLoopRsp: public IPacket
{
public:
	CLoopRsp( uint32_t seq = 0 ){
		_header._type = SERVER_LOOP_RSP ;
		_header._seq  = seq ;
	}
	~CLoopRsp(){};
	void Body( CPacker *pack ) {}
	bool UnBody( CPacker *pack ) { return true; }
};

// �ն�ע������
class CRegisterReq: public IPacket
{
public:
	CRegisterReq( uint32_t seq = 0 ) {
		_header._type = SERVER_REGISTER_REQ ;
		_header._seq  = seq ;
	}
	~CRegisterReq() {} ;

	// ��������
	bool UnBody( CPacker *pack ) {
		pack->readString( _vehicleColor ) ;
		pack->readString( _vehicleno ) ;
		pack->readString( _phone ) ;
		pack->readString( _terminaltype ) ;
		pack->readString( _terminalid ) ;
		pack->readString( _manufacturerid ) ;
		pack->readString( _cityid ) ;
		return true ;
	}

	void Body( CPacker *pack ) {
		pack->writeString( _vehicleColor ) ;
		pack->writeString( _vehicleno ) ;
		pack->writeString( _phone ) ;
		pack->writeString( _terminaltype ) ;
		pack->writeString( _terminalid ) ;
		pack->writeString( _manufacturerid ) ;
		pack->writeString( _cityid ) ;
	}

public:
	CQString _vehicleColor ;  	// ������ɫ
	CQString _vehicleno ;		// ���ƺ�
	CQString _phone ;			// �ֻ���
	CQString _terminaltype ;	// �ն�����
	CQString _terminalid ;		// �ն�ID
	CQString _manufacturerid ;	// ������ID
	CQString _cityid ;			// ����ID
};

// �ն�ע����Ӧ
class CRegisterRsp: public IPacket
{
public:
	CRegisterRsp( uint32_t seq = 0 ) {
		_header._type = SERVER_REGISTER_RSP ;
		_header._seq  = seq ;
	}
	~CRegisterRsp() {}

	void Body( CPacker *pack ) {
		pack->writeByte( _result ) ;
		pack->writeString( _ome ) ;
		pack->writeString( _auth ) ;
	}
	bool UnBody( CPacker *pack ) {
		_result = pack->readByte() ;
		pack->readString( _ome ) ;
		pack->readString( _auth ) ;
		return true ;
	}
public:
	uint8_t	 	_result ;   // ע����
	CQString 	_ome ;		// �豸OME
	CQString 	_auth ;		// ��Ȩ��
};

// �ն��豸��Ȩ
class CTermAuthReq: public IPacket
{
public:
	CTermAuthReq( uint32_t seq = 0 ) {
		_header._type = SERVER_TERMAUTH_REQ ;
		_header._seq  = seq ;
	}
	~CTermAuthReq(){}

	bool UnBody( CPacker *pack ) {
		if ( pack->readString( _phone ) == 0 )
			return false ;
		pack->readString( _auth ) ;
		return true ;
	}
	void Body( CPacker *pack ) {
		pack->writeString( _phone ) ;
		pack->writeString( _auth ) ;
	}
public:
	CQString _phone ;   // �ֻ���
	CQString _auth ;	// ��Ȩ��
};

// �ն˼�Ȩ��Ӧ
class CTermAuthRsp: public IPacket
{
public:
	CTermAuthRsp( uint32_t seq = 0 ){
		_header._type = SERVER_TERMAUTH_RSP ;
		_header._seq  = seq ;
	}
	~CTermAuthRsp(){}

	void Body( CPacker *pack ) {
		pack->writeByte( _result ) ;
		pack->writeString( _ome ) ;
	}
	bool UnBody( CPacker *pack ) {
		_result = pack->readByte() ;
		pack->readString( _ome ) ;
		return true ;
	}
public:
	uint8_t  _result ;  // ���
	CQString _ome ;	    // OEM��
};

// ˾�����ʶ��
class CChkDriverReq: public IPacket
{
public:
	CChkDriverReq( uint32_t seq = 0 ){
		_header._type = SERVER_CKDRIVER_REQ ;
		_header._seq  = seq ;
	}
	~CChkDriverReq(){}

	bool UnBody( CPacker *pack ) {
		if ( pack->readString( _phone ) == 0 )
			return false ;
		pack->readString( _driverNo ) ;
		pack->readString( _driverCertificate ) ;
		return true ;
	}

	void Body( CPacker *pack ) {
		pack->writeString( _phone ) ;
		pack->writeString( _driverNo ) ;
		pack->writeString( _driverCertificate ) ;
	}

public:
	CQString _phone ; 				// �ֻ���
	CQString _driverNo ;			// ˾��֤��
	CQString _driverCertificate ;	// ���֤��
};

// ˾�����ʶ����Ӧ
class CChkDriverRsp: public IPacket
{
public:
	CChkDriverRsp( uint32_t seq = 0 ) {
		_header._type = SERVER_CKDRIVER_RSP ;
		_header._seq  = seq ;
	}
	~CChkDriverRsp(){}

	void Body( CPacker *pack ) {
		pack->writeByte( _result ) ;
		pack->writeString( _msg ) ;
	}
	bool UnBody( CPacker *pack ) {
		_result = pack->readByte() ;
		pack->readString( _msg ) ;
		return true ;
	}
public:
	uint8_t  	_result ;   // ʶ����
	CQString 	_msg ;		// ���������Ϣ
};

// �ն��豸ע������
class CTermLogoReq: public IPacket
{
public:
	CTermLogoReq( uint32_t seq = 0 ){
		_header._type = SERVER_TERMLOGO_REQ ;
		_header._seq  = seq ;
	}
	~CTermLogoReq(){}

	bool UnBody( CPacker *pack ) {
		if ( pack->readString( _phone ) == 0 )
			return false ;
		return true ;
	}
	void Body( CPacker *pack ) {
		pack->writeString( _phone ) ;
	}
public:
	CQString _phone ;
};

// �ն��豸ע����Ӧ
class CTermLogoRsp: public IPacket
{
public:
	CTermLogoRsp( uint32_t seq = 0 ){
		_header._type = SERVER_TERMLOGO_RSP ;
		_header._seq  = seq ;
	}
	~CTermLogoRsp() {}

	void Body( CPacker *pack ) {
		pack->writeByte( _result ) ;
	}
	bool UnBody( CPacker *pack ) {
		_result = pack->readByte() ;
		return true ;
	}

public:
	uint8_t _result ;
};

class CAuthUnPackMgr : public IUnPackMgr
{
public:
	CAuthUnPackMgr(){}
	~CAuthUnPackMgr(){}

	// ʵ�����ݽ���ӿڷ���
	IPacket * UnPack( unsigned short msgtype, CPacker &pack )
	{
		IPacket *msg = NULL ;
		switch( msgtype )
		{
		case SERVER_LOGIN_REQ:
			msg = UnPacket<CLoginReq>( pack, "login" ) ;
			break ;
		case SERVER_LOGIN_RSP:
			msg = UnPacket<CLoginRsp>( pack, "login resp" ) ;
			break ;
		case SERVER_LOOP_REQ:
			msg = UnPacket<CLoopReq>( pack, "loop" ) ;
			break ;
		case SERVER_LOOP_RSP:
			msg = UnPacket<CLoopRsp>( pack, "loop resp" ) ;
			break ;
		case SERVER_REGISTER_REQ:
			msg = UnPacket<CRegisterReq>( pack, "term register" ) ;
			break ;
		case SERVER_REGISTER_RSP:
			msg = UnPacket<CRegisterRsp>( pack, "term register resp" );
			break ;
		case SERVER_TERMAUTH_REQ:
			msg = UnPacket<CTermAuthReq>( pack, "terminal auth" ) ;
			break ;
		case SERVER_TERMAUTH_RSP:
			msg = UnPacket<CTermAuthRsp>( pack, "terminal auth resp" ) ;
			break ;
		case SERVER_CKDRIVER_REQ:
			msg = UnPacket<CChkDriverReq>( pack, "check driver" ) ;
			break ;
		case SERVER_CKDRIVER_RSP:
			msg = UnPacket<CChkDriverRsp>( pack, "check driver resp" ) ;
			break ;
		case SERVER_TERMLOGO_REQ:
			msg = UnPacket<CTermLogoReq>( pack, "terminal logout" ) ;
			break ;
		case SERVER_TERMLOGO_RSP:
			msg = UnPacket<CTermLogoRsp>( pack, "terminal logout resp" ) ;
			break ;
		default:
			break ;
		}
		return msg ;
	}
};

#endif /* MSGPARSER_H_ */
