/*
 * header.h
 *
 *  Created on: 2012-5-31
 *      Author: humingqing
 *  ����˦��Э���������
 */

#ifndef __TRUCKPACK_H__
#define __TRUCKPACK_H__

#include <msgpack.h>
#include <packfactory.h>

#pragma pack(1)

namespace TruckSrv{
	// ������˦��
	#define DRIVER_LOGIN_REQ  		0x1001   // ˾����ݵ�½
	#define DRIVER_LOGIN_RSP  		0x8001
	#define DRIVER_LOGOUT_REQ		0x1002   // ˾�����ע��
	#define DRIVER_LOGOUT_RSP		0x8002	 // ˾�������Ӧ
	#define QUERY_FRIENDS_REQ		0x1010	 // ���Ҹ����ĺ���
	#define QUERY_FRIENDS_RSP		0x8010   // ���Һ�����Ӧ
	#define ADD_FRIEND_REQ			0x1011   // ��Ӻ���
	#define ADD_FRIEND_RSP          0x8011   // ��Ӻ�����Ӧ
	#define INVITE_FRIEND_REQ 		0x1012	 // �������
	#define INVITE_FRIEND_RSP       0x8012   // ���������Ӧ
	#define GET_FRIENDLIST_REQ		0x1013   // ��ȡ�����б�
	#define GET_FRIENDLIST_RSP      0x8013   // ��ȡ�����б���Ӧ

	#define ADD_CARTEAM_REQ         0x1014   //��������
	#define ADD_CARTEAM_RSP         0x8014   //����������Ӧ

	#define INVITE_NUMBER_REQ       0x1015   //���ӳ�Ա����
	#define INVITE_NUMBER_RSP       0x8015   //���ӳ�Ա������Ӧ

	#define SET_PRIMCAR_REQ         0x1016    //���ñ���Ϊͷ��
	#define SET_PRIMCAR_RSP         0x8016    //���ñ���Ϊͷ����Ӧ

	#define INFO_PRIMCAR_REQ        0x1017    //����ͷ������Ϣ
	#define INFO_PRIMCAR_RSP        0x8017    //����ͷ������Ϣ��Ӧ

	#define SEND_MEDIADATA_REQ      0x1018    //��������
	#define SEND_MEDIADATA_RSP      0x8018    //����������Ӧ

	#define SEND_TEAMMEDIA_REQ      0x1019    //��������
	#define SEND_TEAMMEDIA_RSP      0x8019    //����������Ӧ

	#define QUERY_CARDATA_REQ		0x1020   // ����Զ���ѯ
	#define QUERY_CARDATA_RSP		0x8020   // ����Զ���ѯ��Ӧ
	#define UPLOAD_DATAINFO_REQ	    0x1022   // ���״̬�ϱ�
	#define UPLOAD_DATAINFO_RSP		0x8022

	#define SEND_TEXT_MSG_REQ       0x1030    // �ı���Ϣ�·�
	#define SEND_TEXT_MSG_RSP       0x8030    // �ı���Ϣ�·���Ӧ

	// ˦�ҵĶ���
	#define SEND_SCHEDULE_REQ 		0x1040   // �·����ȵ�����
	#define SEND_SCHEDULE_RSP 		0x8040	 // �·����ȵ���Ӧ
	#define RESULT_SCHEDULE_REQ 	0x1045	 // �ϱ����ȵ��·����
	#define RESULT_SCHEDULE_RSP	 	0x8045   // ��Ӧ�ϱ����Ӧ��
	#define QUERY_SCHEDULE_REQ 		0x1041	 // ��ѯ���ȵ�����
	#define QUERY_SCHEDULE_RSP		0x8041	 // ��ѯ���ȵ���Ӧ
	#define UPLOAD_SCHEDULE_REQ 	0x1042	 // �ϴ����ȵ�
	#define UPLOAD_SCHEDULE_RSP		0x8042	 // �ϴ����ȵ���Ӧ
	#define STATE_SCHEDULE_REQ 		0x1043	 // �ϱ����ȵ�״̬
	#define STATE_SCHEDULE_RSP		0x8043	 // �ϱ����ȵ���Ӧ
	#define ALARM_SCHEDULE_REQ 		0x1044	 // ���Ҹ澯
	#define ALARM_SCHEDULE_RSP		0x8044	 //

	#define SUBSCRIBE_REQ           0x1050   // ���Ĺ���
	#define SUBSCRIBE_RSP	        0x8050   // ���Ĺ���ظ�

	#define UP_MSGDATA_REQ          0x1060   // �ն�͸��
	#define UP_MSGDATA_RSP          0x8060
	#define UP_REPORTERROR_REQ 		0x1070	 //�ϴ���������
	#define UP_REPORTERROR_RSP 		0x8070

	#define QUERY_INFO_REQ          0x1090   //��ѯ������Ϣ�ϱ�
	#define QUERY_INFO_RSP          0x8090   //��ѯ������Ϣ�ظ�

	/*���˶��������ӵ�Э��*/
	#define TERMINAL_COMMON_RSP                   0x1000   //�ն�ͨ�ûظ�
	#define PLATFORM_COMMON_RSP                   0x8000   //ƽ̨ͨ�ûظ�

	#define UP_CARDATA_INFO_REQ                   0x1023   //�ϴ������Ϣ
	#define UP_CARDATA_INFO_CONFIRM_REQ           0x1024   //����ɽ�״̬ȷ��

	#define UP_QUERY_ORDER_FORM_INFO_REQ          0x1026   //������ϸ��ѯ
	#define UP_QUERY_ORDER_FORM_INFO_RSP          0x8026   //������ϸ��ѯ��Ӧ
	#define UP_ORDER_FORM_INFO_REQ                0x1027   //�ϴ����˶�����
	#define UP_TRANSPORT_FORM_INFO_REQ            0x1028   //�ϴ��˵�״̬

	#define SEND_CARDATA_INFO_REQ	       		  0x1021 //�·������Ϣ
	#define SEND_CARDATA_INFO_CONFIRM_REQ  		  0x1024 //����ɽ�״̬ȷ��
	#define SEND_ORDER_FORM_REQ            		  0x102E //������ϸ����
	#define SEND_TRANSPORT_ORDER_FORM_REQ  		  0x1025 //�·��˵���Ϣ

	// �����б�
	class CFriendDataInfo : public share::Ref
	{
	public:
		CFriendDataInfo( )
		{
		}
		;
		~CFriendDataInfo( )
		{
		}
		;
		bool UnPack( CPacker *pack )
		{
			_avatar = pack->readInt();
			_userid = pack->readInt();
			pack->readBytes( _username, sizeof ( _username ) );
			pack->readString( _dest );
			_Type = pack->readShort();
			_bulk = pack->readShort();
			_weight = pack->readShort();
			_model = pack->readByte();

			if (pack->readString( _org ) ==0)
				return false;
			if (pack->readString( _desc )==0)
				return false;

			return true;
		}
		void Pack( CPacker *pack )
		{
			pack->writeInt32( _avatar );
			pack->writeInt32( _userid );
			pack->writeBytes( _username, sizeof ( _username ) );
			pack->writeInt16( _Type );
			pack->writeInt16( _bulk );
			pack->writeInt16( _weight );
			pack->writeByte( _model );
			pack->writeString( _org );
			pack->writeString( _desc );
		}
	public:
		uint32_t _avatar; //˾��ͷ��
		uint32_t _userid; //˾��ID
		uint8_t _username[12]; //˾������
		CQString _dest; //����Ŀ�ĵ�
		uint16_t _Type; //��������
		uint16_t _bulk; //�������
		uint16_t _weight; //����������
		uint8_t _model; //���߿�(���)
		CQString _org; //��������
		CQString _desc; //��ע
	};

	// ��������
	class CSendTeamMediaReq : public IPacket
	{
	public:
		CSendTeamMediaReq( uint32_t seq = 0 )
		{
			_header._type = SEND_TEAMMEDIA_REQ;
			_header._seq = seq;
		}
		;
		~CSendTeamMediaReq( )
		{
		}
		;
		bool UnBody( CPacker *pack )
		{
			_ownid  = pack->readInt();
			_teamid = pack->readInt();

			if (pack->readString( _voice ) ==0)
			   return false;

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _teamid );
			pack->writeString( _voice );
		}
	public:
		uint32_t _ownid; //˾��ID
		uint32_t _teamid; //����ID
		CQString _voice; //��������

	};

	// ����������Ӧ
	class CSendTeamMediaRsp : public IPacket
	{
	public:
		CSendTeamMediaRsp( uint32_t seq = 0 )
		{
			_header._type = SEND_TEAMMEDIA_RSP;
			_header._seq = seq;
		}
		;
		~CSendTeamMediaRsp( )
		{
		}
		;
		bool UnBody( CPacker *pack )
		{
			_teamid = pack->readInt();
			_state = pack->readByte();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _teamid );
			pack->writeByte( _state );
		}
	public:
		uint32_t _teamid; //����ID
		uint8_t _state; //�������ݷ���״̬
	};
	// ��������
	class CSendMediaDataReq : public IPacket
	{
	public:
		CSendMediaDataReq( )
		{
			_header._type = SEND_MEDIADATA_REQ;
		}
		;
		~CSendMediaDataReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_ownid = pack->readInt();
			_userid = pack->readInt();

			if (pack->readString( _voice) == 0)
				return false;

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _userid );
			pack->writeString( _voice );
		}
	public:
		uint32_t _ownid; //˾��ID
		uint32_t _userid; //����ID
		CQString _voice; //��������
	};
	// ����������Ӧ
	class CSendMediaDataRsp : public IPacket
	{
	public:
		CSendMediaDataRsp( uint32_t seq = 0 )
		{
			_header._type = SEND_MEDIADATA_RSP;
			_header._seq = seq;
		}
		;
		~CSendMediaDataRsp( )
		{
		}
		;
		bool UnBody( CPacker *pack )
		{
			_ownid = pack->readInt();
			_userid = pack->readInt();

			if (pack->readString( _voice ) == 0)
				return false;

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _userid );
			pack->writeString( _voice );
		}
	public:
		uint32_t _ownid; //˾��ID
		uint32_t _userid; //����ID
		CQString _voice; //��������
	};
	// ����ͷ������Ϣ
	class CInfoPriMcarReq : public IPacket
	{
	public:
		CInfoPriMcarReq( )
		{
			_header._type = SET_PRIMCAR_REQ;
		}
		;
		~CInfoPriMcarReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_userid = pack->readInt();

			if (pack->readString( _name ) == 0)
				return false;

			_type   = pack->readShort();
			_weight = pack->readShort();

			if (pack->readString( _carnum ) ==0)
				 return false;
			if (pack->readString( _dest ) == 0)
				 return false;

			_speed = pack->readShort();

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _userid );
			pack->writeInt16( _type );
			pack->writeInt16( _weight );
			pack->writeString( _name );
			pack->writeString( _carnum );
			pack->writeString( _dest );
			pack->writeInt16( _speed );
		}
	public:
		uint32_t _userid; //˾�����ID
		CQString _name; //˾������
		uint16_t _type; //�����ͺ�
		uint16_t _weight; // ����������
		CQString _carnum; //���ƺ�
		CQString _dest; //����Ŀ�ĵ�
		uint16_t _speed; //�ٶ�
	};

	// ����ͷ������Ϣ��Ӧ
	class CInfoPriMcarRsp : public IPacket
	{
	public:
		CInfoPriMcarRsp( uint32_t seq = 0 )
		{
			_header._type = INFO_PRIMCAR_REQ;
			_header._seq = seq;
		}
		;
		~CInfoPriMcarRsp( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_state = pack->readByte();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeByte( _state );
		}
	public:
		uint8_t _state; //����״̬(0 �ɹ���1ʧ��)
	};
	// ���ñ���Ϊͷ��
	class CSetPriMcarReq : public IPacket
	{
	public:
		CSetPriMcarReq( )
		{
			_header._type = SET_PRIMCAR_REQ;
		}
		;
		~CSetPriMcarReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_teamid = pack->readInt();
			_userid = pack->readInt();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _teamid );
			pack->writeInt32( _userid );
		}
	public:
		uint32_t _teamid; //����ID
		uint32_t _userid; //����ID
	};

	// ���ñ���Ϊͷ����Ӧ
	class CSetPriMcarRsp : public IPacket
	{
	public:
		CSetPriMcarRsp( uint32_t seq = 0 )
		{
			_header._type = ADD_CARTEAM_RSP;
			_header._seq = seq;
		}
		;
		~CSetPriMcarRsp( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_userid = pack->readInt();
			_state = pack->readByte();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _userid );
			pack->writeByte( _state );
		}
	public:
		uint32_t _userid; //����ID
		uint8_t _state; //ͷ������״̬(0 �ɹ���1ʧ��)
	};
	// ���ӳ�Ա����
	class CInviteNumberReq : public IPacket
	{
	public:
		CInviteNumberReq( )
		{
			_header._type = INVITE_NUMBER_REQ;
		}
		;
		~CInviteNumberReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_teamid = pack->readInt();
			_userid = pack->readInt();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _teamid );
			pack->writeInt32( _userid );
		}
	public:
		uint32_t _teamid; //����ID
		uint32_t _userid; //����ID
	};

	// ���ӳ�Ա������Ӧ
	class CInviteNumberRsp : public IPacket
	{
	public:
		CInviteNumberRsp( uint32_t seq = 0 )
		{
			_header._type = ADD_CARTEAM_RSP;
			_header._seq = seq;
		}
		;
		~CInviteNumberRsp( )
		{
		}
		;
		bool UnBody( CPacker *pack )
		{
			_userid = pack->readInt();
			_teamid = pack->readInt();
			_state = pack->readByte();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _userid );
			pack->writeInt32( _teamid );
			pack->writeByte( _state );
		}
	public:
		uint32_t _teamid; //����ID
		uint32_t _userid; //����ID
		uint8_t _state; //����״̬(0 �ɹ���1�ܾ���2 ������)
	};
	// ��������
	class CAddCarTeamReq : public IPacket
	{
	public:
		CAddCarTeamReq( )
		{
			_header._type = ADD_CARTEAM_REQ;
		}
		;
		~CAddCarTeamReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_id = pack->readInt();
			if (pack->readString( _teamname ) == 0)
				return false;

			_teamnum = pack->readShort();
			pack->readString( _teamdesc );
			_teamtype = pack->readByte();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _id );
			pack->writeString( _teamname );
			pack->writeInt16( _teamnum );
			pack->writeString( _teamdesc );
			pack->writeByte( _teamtype );
		}
	public:
		uint32_t _id; //˾�����ID��
		CQString _teamname; //��������
		uint16_t _teamnum; //���������޶�
		CQString _teamdesc; //����˵��
		uint8_t _teamtype; //����ģʽ
	};
	// ����������Ӧ
	class CAddCarTeamRsp : public IPacket
	{
	public:
		CAddCarTeamRsp( uint32_t seq = 0 )
		{
			_header._type = ADD_CARTEAM_RSP;
			_header._seq = seq;
		}
		;
		~CAddCarTeamRsp( )
		{
		}
		;
		bool UnBody( CPacker *pack )
		{
			_userid = pack->readInt();
			_teamid = pack->readInt();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _userid );
			pack->writeInt32( _teamid );
		}
	public:
		uint32_t _userid; //˾�����ID
		uint32_t _teamid; //���Ӵ���״̬(0 ʧ�ܣ�����Ϊ����ID)
	};
	// ��ȡ�����б�
	class CGetFriendListReq : public IPacket
	{
	public:
		CGetFriendListReq( )
		{
			_header._type = GET_FRIENDLIST_REQ;
		}
		;
		~CGetFriendListReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_id = pack->readInt();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _id );
		}
	public:
		uint32_t _id; //˾�����ID��
	};
	// ��ȡ������Ӧ
	class CGetFriendListRsp : public IPacket
	{
		typedef std::vector< CFriendDataInfo* > CFriendDataInfoVec;
	public:
		CGetFriendListRsp( uint32_t seq = 0 )
		{
			_header._type = GET_FRIENDLIST_RSP;
			_header._seq = seq;
		}
		;
		~CGetFriendListRsp( )
		{
			Clear();
		}
		;

		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CFriendDataInfo *info = new CFriendDataInfo;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			_num = _vec.size();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
			if ( _num == 0 ) return;

			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

	private:
		void Clear( void )
		{
			if ( _vec.empty() ) return;
			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}
	public:
		uint16_t _num; //���Ѹ���(û���򷵻�0)
		CFriendDataInfoVec _vec;
	};
	// ���복��
	class CInviteFriendReq : public IPacket
	{
	public:
		CInviteFriendReq( )
		{
			_header._type = INVITE_FRIEND_REQ;
		}
		;
		~CInviteFriendReq( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			_ownid = pack->readInt();
			_userid = pack->readInt();

			return true;
		}

		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _userid );
		}
	public:
		uint32_t _ownid; //˾�����ID��
		uint32_t _userid; //�������ID��
	};

	// ���복�ѻ�Ӧ
	class CInviteFriendRsp : public IPacket
	{
		typedef std::vector< CFriendDataInfo* > CFriendDataInfoVec;
	public:
		CInviteFriendRsp( uint32_t seq = 0 )
		{
			_header._type = ADD_FRIEND_RSP;
			_header._seq = seq;
		}
		;
		~CInviteFriendRsp( )
		{

		}
		;
		bool UnBody( CPacker *pack )
		{
			_ownid = pack->readInt();
			_userid = pack->readInt();
			_state = pack->readByte();
			return true;
		}
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _userid );
			pack->writeByte( _state );
		}
	public:
		uint32_t _ownid; //˾�����ID��
		uint32_t _userid; //�������ID��
		uint8_t _state; //����״̬(0 �ɹ���1�ܾ���2 ������)
	};
	// ��ӳ���
	class CAddFriendsReq : public IPacket
	{
	public:
		CAddFriendsReq( )
		{
			_header._type = ADD_FRIEND_REQ;
		}
		;
		~CAddFriendsReq( )
		{
		}
		;
	// ���
		bool UnBody( CPacker *pack )
		{
			_ownid = pack->readInt();
			_userid = pack->readInt();
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _ownid );
			pack->writeInt32( _userid );
		}
	public:
		uint32_t _ownid; //˾�����ID��
		uint32_t _userid; //�������ID��
	};
	// ��ӳ�����Ӧ
	class CAddFriendsRsp : public IPacket
	{
	public:
		CAddFriendsRsp( uint32_t seq = 0 )
		{
			_header._type = ADD_FRIEND_RSP;
			_header._seq = seq;
		}
		;
		~CAddFriendsRsp( )
		{

		}
		;
		bool UnBody( CPacker *pack )
		{
			_state = pack->readByte();
			_userid = pack->readInt();
			return true;
		}
		void Body( CPacker *pack )
		{
			pack->writeByte( _state );
			pack->writeInt32( _userid );
		}
	public:
		uint8_t _state; //����״̬(0 ���ܣ�1�ܾ���2������)
		uint32_t _userid; //����ID��
	};

	// ���Ҹ�������
	class CQueryFriendsReq : public IPacket
	{
	public:
		CQueryFriendsReq( )
		{
			_header._type = QUERY_FRIENDS_REQ;
		}
		;
		~CQueryFriendsReq( )
		{
		}
		;
		// ���
		bool UnBody( CPacker *pack )
		{
			_id = pack->readInt();
			_Lon = pack->readInt();
			_Lat = pack->readInt();

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeInt32( _id );
			pack->writeInt32( _Lon );
			pack->writeInt32( _Lat );
		}
	public:
		uint32_t _id; //�ն��û�Ψһ��ʶ��
		uint32_t _Lon; //GPS�ľ���
		uint32_t _Lat; //GPS��γ��
	};

	// ���ز��Ҹ�������
	class CQueryFriendsRsp : public IPacket
	{
		typedef std::vector< CFriendDataInfo* > CFriendDataInfoVec;
	public:
		CQueryFriendsRsp( uint32_t seq = 0 )
		{
			_header._type = QUERY_FRIENDS_RSP;
			_header._seq = seq;
		}
		;
		~CQueryFriendsRsp( )
		{
			Clear();
		}
		;
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CFriendDataInfo *info = new CFriendDataInfo;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			_num = _vec.size();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
			if ( _num == 0 ) return;

			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

	private:
		void Clear( void )
		{
			if ( _vec.empty() ) return;
			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}
	public:
		uint16_t _num; //���ҵ��ĳ��Ѹ��������û����Ϊ
		CFriendDataInfoVec _vec;

	};
	// ˾�����ע��
	class CDriverLoginOutReq : public IPacket
	{
	public:
		CDriverLoginOutReq( )
		{
			_header._type = DRIVER_LOGOUT_REQ;
		}
		;
		~CDriverLoginOutReq( )
		{
		}
		;
		// ���
		bool UnBody( CPacker *pack )
		{
			if ( pack->readString( _identify ) == 0 ) return false;
			if ( pack->readString( _driverid ) == 0 ) return false;
			if ( pack->readString( _carid ) == 0 ) return false;
			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeString( _identify );
			pack->writeString( _driverid );
			pack->writeString( _carid );
		}
	public:
		CQString _identify; //	Variant	   ���֤����
		CQString _driverid; //	Variant	   ��������
		CQString _carid; //	   Variant	   �������
	};
	// ����˾����ע����Ϣ xfm
	class CDriverLoginOutRsp : public IPacket
	{
	public:
		CDriverLoginOutRsp( uint32_t seq = 0 )
		{
			_header._type = DRIVER_LOGOUT_RSP;
			_header._seq = seq;
		}
		;
		~CDriverLoginOutRsp( )
		{

		}
		;
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _time, sizeof ( _time ) );
			_state = pack->readByte();
			pack->readBytes( _name, sizeof ( _name ) );
			return true;
		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _time, sizeof ( _time ) );
			pack->writeByte( _state );
			pack->writeBytes( _name, sizeof ( _name ) );
		}
	public:
		uint8_t _time[6]; //ע��ʱ��(BCD,YY-mm-dd HH-ii-ss)
		uint8_t _state; //ע��״̬
		uint8_t _name[12]; //˾������
	};

	// ˾����ݵ�½��֤
	class CDriverLoginReq : public IPacket
	{
	public:
		CDriverLoginReq( )
		{
			_header._type = DRIVER_LOGIN_REQ;
		}
		;
		~CDriverLoginReq( )
		{
		}
		;
		// ���
		bool UnBody( CPacker *pack )
		{
			if ( pack->readString( _identify ) == 0 ) return false;
			if ( pack->readString( _driverid ) == 0 ) return false;
			if ( pack->readString( _phonenum ) == 0 ) return false;
			if ( pack->readString( _simnum ) == 0 ) return false;
			if ( pack->readString( _carid ) == 0 ) return false;

			return true;
		}
		// ���������
		void Body( CPacker *pack )
		{
			pack->writeString( _identify );
			pack->writeString( _driverid );
			pack->writeString( _phonenum );
			pack->writeString( _simnum );
			pack->writeString( _carid );
		}
	public:
		CQString _identify; //	Variant	   ���֤����
		CQString _driverid; //	Variant	    ��������
		CQString _phonenum; //	Variant	    ˾���ֻ�����(BCD�ֻ���)
		CQString _simnum; //	Variant	  SIM������
		CQString _carid; //	Variant  �������
	};
	// �����ѯ
	class CQueryCarDataReq : public IPacket
	{
	public:
		CQueryCarDataReq( )
		{
			_header._type = QUERY_CARDATA_REQ;
		}
		;
		~CQueryCarDataReq( )
		{
		}
		;

		// ���
		bool UnBody( CPacker *pack )
		{

			_destarea = pack->readInt();
			_srcarea = pack->readInt();

			if ( pack->readBytes( _time, 6 ) == 0 ) return false;

			_offset = pack->readByte();
			_count = pack->readByte();

			return true;
		}

		// ���������
		void Body( CPacker *pack )
		{

			pack->writeInt32( _destarea );
			pack->writeInt32( _srcarea );

			pack->writeBytes( _time, 6 );
			pack->writeByte( _offset );
			pack->writeByte( _count );
		}

	public:
		uint32_t _destarea; // Ŀ�ĵ�
		uint32_t _srcarea; //   ������
		uint8_t _time[6]; //	String	���ʱ��
		uint8_t _offset; //	1	UINT8	��ȡ��ʼλ�ã�Ĭ�ϴ�0��ʼ
		uint8_t _count; //  1	UINT8	��¼����
	};

	// ֱ��ʹ�ö������ת������ dtxi
	class CCarDataInfo : public share::Ref
	{
	public:
		CCarDataInfo( )
		{
		}
		;
		~CCarDataInfo( )
		{
		}
		;

		bool UnPack( CPacker *pack )
		{
			if ( pack->readBytes( _sid, 20 ) == 0 ) return false;
			if ( pack->readBytes( _time, 6 ) == 0 ) return false;

			_sarea = pack->readInt();
			_darea = pack->readInt();

			pack->readString(_stype);
			_model = pack->readInt();
			_bulk = pack->readInt();
			_nnum = pack->readShort();

			pack->readBytes( _contact, sizeof ( _contact ) );
			if ( pack->readBytes( _phone, 6 ) == 0 ) return false;
			pack->readString( _info );

			return true;
		}

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _sid, 20 );
			pack->writeBytes( _time, 6 );

			pack->writeInt( _sarea );
			pack->writeInt( _darea );

			pack->writeString( _stype );
			pack->writeInt( _model );
			pack->writeInt( _bulk );
			pack->writeShort( _nnum );
			pack->writeBytes( _contact, sizeof ( _contact ) );

			pack->writeBytes( _phone, 6 );
			pack->writeString( _info );
		}

	public:
		uint8_t _sid[20]; //  String	������
		uint8_t _time[6]; //  String	���ʱ��
		uint32_t _sarea; //	 UINT32  	���������
		uint32_t _darea; //	 UINT32	           ���Ŀ�ĵ�
		CQString _stype; //	 Variant    ��������
		uint32_t _model; //   UINT32	������
		uint32_t _bulk; //    UINT32	�������
		uint16_t _nnum; //    UINT16	����Ҫ��O
		uint8_t _contact[12]; //
		uint8_t _phone[6]; //  String	�����绰(BCD��ǰ�治������)
		CQString _info; //  Variant 	��ע�û�˵��
	};

	// ����˾������֤��Ϣ xfm
	class CDriverLoginRsp : public IPacket
	{
	public:
		CDriverLoginRsp( uint32_t seq = 0 )
		{
			_header._type = DRIVER_LOGIN_RSP;
			_header._seq = seq;
		}
		;
		~CDriverLoginRsp( )
		{

		}
		;
		bool UnBody( CPacker *pack )
		{

			pack->readBytes( _time, sizeof ( _time ) );

			_userid = pack->readInt();
			if ( pack->readString( _pic ) == 0 ) return false;

			pack->readBytes( _name, sizeof ( _name ) );

			pack->readBytes( _simnum, sizeof ( _simnum ) );

			_grade = pack->readShort();
			_score = pack->readInt();

			pack->readBytes( _carnum, sizeof ( _carnum ) );

			return true;
		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _time, sizeof ( _time ) );
			pack->writeInt32( _userid );
			pack->writeString( _pic );
			pack->writeBytes( _name, sizeof ( _name ) );
			pack->writeBytes( _simnum, sizeof ( _simnum ) );
			pack->writeInt16( _grade );
			pack->writeInt32( _score );
			pack->writeBytes( _carnum, sizeof ( _carnum ) );
		}
	public:
		uint8_t _time[6]; //��֤ʱ��(BCDʱ��)
		uint32_t _userid; //��֤״̬����(�����û���ID)
		CQString _pic; //˾����Ƭ
		uint8_t _name[12]; //˾������
		uint8_t _simnum[18]; //SIM������
		uint16_t _grade; //��Ա�ȼ�
		uint32_t _score; //��Ա����
		uint8_t _carnum[12]; //���ƺ�
	};
	// ��ѯ�����Ϣ����Ӧ
	class CQueryCarDataRsp : public IPacket
	{
		typedef std::vector< CCarDataInfo* > CCarDataInfoVec;
	public:
		CQueryCarDataRsp( uint32_t seq = 0 ) :
				_num( 0 )
		{
			_header._type = QUERY_CARDATA_RSP;
			_header._seq = seq;
		}
		;
		~CQueryCarDataRsp( )
		{
			Clear();
		}
		;

		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CCarDataInfo *info = new CCarDataInfo;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			_num = _vec.size();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
			if ( _num == 0 ) return;

			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

	private:
		void Clear( void )
		{
			if ( _vec.empty() ) return;
			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}

	public:
		// ��ѯ���ص���Ϣ����
		uint8_t _num;
		// ������ݵĶ���
		CCarDataInfoVec _vec;
	};

	// ˦�ҵĶ���
	// GPS������Ϣ�ṹ����
	class GpsInfo : public share::Ref
	{
	public:
		GpsInfo( )
		{
		}
		;
		~GpsInfo( )
		{
		}
		;

		void Pack( CPacker *pack )
		{
			pack->writeInt( _alam );
			pack->writeInt( _state );
			pack->writeInt( _lat );
			pack->writeInt( _lon );
			pack->writeShort( _height );
			pack->writeShort( _speed );
			pack->writeShort( _direction );
			pack->writeBytes( _time, 6 );
		}

		bool UnPack( CPacker *pack )
		{

			_alam = pack->readInt();
			_state = pack->readInt();
			_lat = pack->readInt();
			_lon = pack->readInt();
			_height = pack->readShort();
			_speed = pack->readShort();
			_direction = pack->readShort();

			if ( pack->readBytes( _time, 6 ) == 0 ) return false;

			return true;
		}

	public:

		uint32_t _alam; //	4	UINT32	������־λ
		uint32_t _state; //	4	UINT32	״̬λ
		uint32_t _lat; //	4	UINT32	γ��
		uint32_t _lon; //  4	UINT32	����
		uint16_t _height; //	2	UINT16	�߳�
		uint16_t _speed; //	2	UINT16	�ٶ�
		uint16_t _direction; //	2	UINT16	����
		uint8_t _time[6]; //	String	YY-MM-DD-hh-mm-ss(BCDʱ��)

	};

	// ���ȵ�������Ϣ
	class CScheduleInfo : public share::Ref
	{
	public:
		CScheduleInfo( )
		{
			memset( _carnum, 0, sizeof ( _carnum ) );
		}
		~CScheduleInfo( )
		{
		}

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _scheduleid, 18 );
			pack->writeString( _sarea );
			pack->writeBytes( _srcid, 18 );
			pack->writeBytes( _start, 6 );
			pack->writeBytes( _carnum, 12 );
			pack->writeBytes( _hangid, 17 );
			pack->writeString( _darea );
			pack->writeBytes( _destid, 18 );
			pack->writeBytes( _atime, 6 );
			pack->writeBytes( _stime, 6 );
		}

		bool UnPack( CPacker *pack )
		{
			if ( pack->readBytes( _scheduleid, 18 ) == 0 ) return false;
			pack->readString( _sarea );

			if ( pack->readBytes( _srcid, 18 ) == 0 ) return false;
			if ( pack->readBytes( _start, 6 ) == 0 ) return false;
			if ( pack->readBytes( _carnum, 12 ) == 0 ) return false;
			if ( pack->readBytes( _hangid, 17 ) == 0 ) return false;
			pack->readString( _darea );

			if ( pack->readBytes( _destid, 18 ) == 0 ) return false;
			if ( pack->readBytes( _atime, 6 ) == 0 ) return false;
			if ( pack->readBytes( _stime, 6 ) == 0 ) return false;
			return true;
		}

	public:
		uint8_t _scheduleid[18]; // String	���ȵ���
		CQString _sarea; // Variant	������
		uint8_t _srcid[18]; // String	������λ
		uint8_t _start[6]; // String	����ʱ��
		uint8_t _carnum[12]; // String	�ҳ����ƺ�
		uint8_t _hangid[17]; // String	�ҳ�ID
		CQString _darea; // Variant	Ŀ�ĵ�
		uint8_t _destid[18]; // String	Ŀ�ĳ�λ
		uint8_t _atime[6]; // String	����ʱ��
		uint8_t _stime[6]; // String	����ʱ��
	};

	// #define SEND_SCHEDULE_REQ 		0x1040   // �·����ȵ�����
	class CSendScheduleReq : public IPacket
	{
	public:
		CSendScheduleReq( )
		{
			_header._type = SEND_SCHEDULE_REQ;
		}
		~CSendScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			_info.Pack( pack );
		}

		bool UnBody( CPacker *pack )
		{
			return _info.UnPack( pack );
		}

	public:
		CScheduleInfo _info; // ���ȵ���Ϣ
	};

	// #define SEND_SCHEDULE_RSP 		0x8040	 // �·����ȵ���Ӧ
	class CSendScheduleRsp : public IPacket
	{
	public:
		CSendScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = SEND_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CSendScheduleRsp( )
		{
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

	public:
		uint8_t _result; //1	UINT8 0�ɹ���1ʧ��
	};

	// �ϱ����ȵ��·���Ӧ�Ľ��  RESULT_SCHEDULE_REQ  0x1045
	class CResultScheduleReq : public IPacket
	{
	public:
		CResultScheduleReq( )
		{
			_header._type = RESULT_SCHEDULE_REQ;
		}
		~CResultScheduleReq( )
		{
		}

		bool UnBody( CPacker *pack )
		{
			if ( pack->readBytes( _scheduleid, 18 ) == 0 ) return false;
			_result = pack->readByte();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _scheduleid, 18 );
			pack->writeByte( _result );
		}

	public:
		uint8_t _scheduleid[18]; // String	���ȵ���
		uint8_t _result; //1	UINT8	���ܾܾ���0 ���ܣ�1�ܾ���
	};

	// #define RESULT_SCHEDULE_RSP 		0x8045	 // �ϱ��·����ȵ���ӦӦ��
	class CResultScheduleRsp : public IPacket
	{
	public:
		CResultScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = RESULT_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CResultScheduleRsp( )
		{
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

	public:
		uint8_t _result; //1	UINT8	0�ɹ���1ʧ��
	};

	//#define QUERY_SCHEDULE_REQ 		0x1041	 // ��ѯ���ȵ�����
	class CQueryScheduleReq : public IPacket
	{
	public:
		CQueryScheduleReq( )
		{
			_header._type = QUERY_SCHEDULE_REQ;
		}
		~CQueryScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
		}

		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			return true;
		}

	public:
		uint8_t _num; // ��ѯ���ص��ȵ�����
	};

	//#define QUERY_SCHEDULE_RSP		0x8041	 // ��ѯ���ȵ���Ӧ
	class CQueryScheduleRsp : public IPacket
	{
		typedef std::vector< CScheduleInfo* > ScheduleVec;
	public:
		CQueryScheduleRsp( uint8_t seq = 0 )
		{
			_header._type = QUERY_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CQueryScheduleRsp( )
		{
			Clear();
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
			// �������������
			for ( int i = 0 ; i < _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			for ( int i = 0 ; i < _num ; ++ i ) {
				CScheduleInfo *info = new CScheduleInfo;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			return true;
		}

	private:
		// �������ж�������
		void Clear( void )
		{
			if ( _vec.empty() ) return;

			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}

	public:
		uint8_t _num;
		ScheduleVec _vec;
	};

	// #define UPLOAD_SCHEDULE_REQ 	0x1042	 // �ϴ����ȵ�
	class CUploadScheduleReq : public IPacket
	{
	public:
		CUploadScheduleReq( )
		{
			_header._type = UPLOAD_SCHEDULE_REQ;
		}
		~CUploadScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _scheduleid, 18 );
			pack->writeByte( _matchstate );
			pack->writeBytes( _hangid, 17 );
			pack->writeBytes( _hangnum, 12 );
			pack->writeBytes( _mtime, 6 );
			_info.Pack( pack );
		}

		bool UnBody( CPacker *pack )
		{
			if ( pack->readBytes( _scheduleid, 18 ) == 0 ) return false;

			_matchstate = pack->readByte();

			if ( pack->readBytes( _hangid, 17 ) == 0 ) return false;
			if ( pack->readBytes( _hangnum, 12 ) == 0 ) return false;
			if ( pack->readBytes( _mtime, 6 ) == 0 ) return false;
			return _info.UnPack( pack );
		}

	public:
		uint8_t _scheduleid[18]; //  String	���ȵ���
		uint8_t _matchstate; //  UINT8	ƥ��״̬(0�ɹ���1ʧ��)
		uint8_t _hangid[17]; //	String	�ҳ�ID
		uint8_t _hangnum[12]; //	String	�ҳ����ƺ�
		uint8_t _mtime[6]; //  String	ƥ��ʱ��
		GpsInfo _info; // GPS
	};

	// #define UPLOAD_SCHEDULE_RSP		0x8042	 // �ϴ����ȵ���Ӧ
	class CUploadScheduleRsp : public IPacket
	{
	public:
		CUploadScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = UPLOAD_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CUploadScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

	public:
		uint8_t _result; // �ɹ����
	};

	// #define STATE_SCHEDULE_REQ 		0x1043	 // �ϱ����ȵ�״̬
	class CStateScheduleReq : public IPacket
	{
	public:
		CStateScheduleReq( )
		{
			_header._type = STATE_SCHEDULE_REQ;
		}
		~CStateScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _scheduleid, 18 );
			pack->writeByte( _action );
			pack->writeBytes( _hangid, 17 );
			pack->writeBytes( _hangnum, 12 );
			_info.Pack( pack );
		}

		bool UnBody( CPacker *pack )
		{
			if ( pack->readBytes( _scheduleid, 18 ) == 0 ) return false;
			_action = pack->readByte();
			if ( pack->readBytes( _hangid, 17 ) == 0 ) return false;
			if ( pack->readBytes( _hangnum, 12 ) == 0 ) return false;
			return _info.UnPack( pack );
		}

	public:
		uint8_t _scheduleid[18]; // String	���ȵ���
		uint8_t _action; // UINT8	���ȶ����� 0������1���
		uint8_t _hangid[17]; // String	�ҳ�ID
		uint8_t _hangnum[12]; // String	�ҳ����ƺ�
		GpsInfo _info; // GPSλ����Ϣ������2.5.6
	};

	//#define STATE_SCHEDULE_RSP		0x8043	 // �ϱ����ȵ���Ӧ
	class CStateScheduleRsp : public IPacket
	{
	public:
		CStateScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = STATE_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CStateScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

	public:
		uint8_t _result; // �ϱ����ȵ����
	};

	// #define ALARM_SCHEDULE_REQ 		0x1044	 // ���Ҹ澯
	class CAlarmScheduleReq : public IPacket
	{
	public:
		CAlarmScheduleReq( )
		{
			_header._type = ALARM_SCHEDULE_REQ;
		}

		~CAlarmScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _scheduleid, 18 );
			pack->writeBytes( _hangid, 17 );
			pack->writeBytes( _hangnum, 12 );
			pack->writeInt( _alarmtype );
			_info.Pack( pack );
		}

		bool UnBody( CPacker *pack )
		{
			if ( pack->readBytes( _scheduleid, 18 ) == 0 ) return false;
			if ( pack->readBytes( _hangid, 17 ) == 0 ) return false;
			if ( pack->readBytes( _hangnum, 12 ) == 0 ) return false;
			_alarmtype = pack->readInt();

			return _info.UnPack( pack );
		}

	public:
		uint8_t _scheduleid[18]; //String	���ȵ���
		uint8_t _hangid[17]; //String	�ҳ�ID
		uint8_t _hangnum[12]; //String	�ҳ����ƺ�
		uint32_t _alarmtype; //UINT32	�ݶ��尴λ���㣬��0λΪ�������룩
		GpsInfo _info; //GPSλ����Ϣ������2.5.6
	};

	//#define ALARM_SCHEDULE_RSP		0x8044	 //
	class CAlarmScheduleRsp : public IPacket
	{
	public:
		CAlarmScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = ALARM_SCHEDULE_RSP;
			_header._seq = seq;
		}
		~CAlarmScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

	public:
		uint8_t _result; // ������
	};

	//�����б� xfm
	class CSubscribeList : public share::Ref
	{
	public:
		CSubscribeList( )
		{

		}
		~CSubscribeList( )
		{

		}
		void Pack( CPacker *pack )
		{
			pack->writeInt16( _ctype );
		}

		bool UnPack( CPacker *pack )
		{
			_ctype = pack->readShort();
			return true;
		}
	public:
		uint16_t _ctype; //��������

	};
	class CSubscrbeReq : public IPacket //0x1050 ���Ĺ���
	{
	public:
		typedef std::vector< CSubscribeList* > SubscribeListVec;

		CSubscrbeReq( )
		{
			_header._type = SUBSCRIBE_REQ;
		}
		~CSubscrbeReq( )
		{
			Clear();
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _cmd );
			pack->writeByte( _num );
			// �������������
			for ( int i = 0 ; i < _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

		bool UnBody( CPacker *pack )
		{
			_cmd = pack->readByte();
			_num = pack->readByte();

			for ( int i = 0 ; i < _num ; ++ i ) {
				CSubscribeList *info = new CSubscribeList;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			return true;
		}
	private:
		// �������ж�������
		void Clear( void )
		{
			if ( _vec.empty() ) return;
			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}
	public:
		uint8_t _cmd; //��������
		uint8_t _num; //���ĸ���
		SubscribeListVec _vec;
	};
	//���Ĺ���ظ�
	class CSubscrbeRsp : public IPacket //0x8050
	{
	public:
		typedef std::vector< CSubscribeList* > SubscribeListVec;

		CSubscrbeRsp( uint32_t seq = 0 )
		{
			_header._type = SUBSCRIBE_RSP;
			_header._seq = seq;
		}
		~CSubscrbeRsp( )
		{
			Clear();
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _num );
			// �������������
			for ( int i = 0 ; i < _num ; ++ i ) {
				_vec[i]->Pack( pack );
			}
		}

		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			for ( int i = 0 ; i < _num ; ++ i ) {
				CSubscribeList *info = new CSubscribeList;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec.push_back( info );
			}
			return true;
		}
	private:
		// �������ж�������
		void Clear( void )
		{
			if ( _vec.empty() ) return;

			for ( int i = 0 ; i < ( int ) _vec.size() ; ++ i ) {
				_vec[i]->Release();
			}
			_vec.clear();
		}
	public:
		uint8_t _num; // ������
		SubscribeListVec _vec;
	};

	//������Ϣ��ѯ
	class CQueryInfoReq : public IPacket //0x1090
	{
	public:
		typedef enum
		{
			Area = 0,
			Roea,
			Bad_Weather,
			Real_Time_Weather,
			Maintenance_Reminders,
			Health_Care_Reminder,
			Annual_Reminder,
			Operation_To_Remind,
			Integrity_Aler_Reminder,
			Illegal_To_Remind,
			Notice_Notice,
			Accident_Black_Spots
		} SUBSCRIBE_TYPE;

		CQueryInfoReq( )
		{
			_header._type = QUERY_INFO_REQ;
		}
		~CQueryInfoReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeByte( _ctype );
			if ( _ctype == Area ) pack->writeInt32( _area_id );
			if ( _ctype == Roea ) pack->writeInt32( _road_id );
			if ( _ctype == Real_Time_Weather ) {
				pack->writeInt32( _lon );
				pack->writeInt32( _lat );
			}
		}
		bool UnBody( CPacker *pack )
		{
			_ctype = pack->readByte();
			if ( _ctype == Area ) {
				_area_id = pack->readInt();
			}
			if ( _ctype == Roea ) {
				_road_id = pack->readInt();
			}
			if ( _ctype == Real_Time_Weather ) {
				_lon = pack->readInt();
				_lat = pack->readInt();
			}
			return true;
		}
	public:
		uint8_t _ctype; //��������
		uint32_t _area_id; //��������id
		uint32_t _road_id; //·��id
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};

	//������Ϣ�ظ�
	class CQueryInfoRsp : public IPacket //0x8090
	{
	public:
		CQueryInfoRsp( uint32_t seq = 0 )
		{
			_header._type = QUERY_INFO_RSP;
			_header._seq = seq;
		}
		~CQueryInfoRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _ctype );
			if ( _ctype == CQueryInfoReq::Area ) pack->writeInt32( _area_id );
			if ( _ctype == CQueryInfoReq::Roea ) pack->writeInt32( _road_id );

			pack->writeString( _data );
		}

		bool UnBody( CPacker *pack )
		{
			_ctype = pack->readByte();

			if ( _ctype == CQueryInfoReq::Area ) {
				_area_id = pack->readInt();
			}

			if ( _ctype == CQueryInfoReq::Roea ) {
				_road_id = pack->readInt();
			}
			pack->readString( _data );
			return true;
		}
	public:

		uint8_t _ctype; //��������
		uint32_t _area_id; //��������id
		uint32_t _road_id; //·��id
		CQString _data; //	Variant	����
	};

	class CErrorScheduleReq : public IPacket //0x1070
	{
	public:
		CErrorScheduleReq( )
		{
			_header._type = UP_REPORTERROR_REQ;
		}
		~CErrorScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeInt32( _code );
			pack->writeString( _desc);
		}

		bool UnBody( CPacker *pack )
		{
			_code = pack->readInt();

			if (pack->readString(_desc) == 0)
				return false;

			return true;
		}

	public:
		uint32_t _code; // ����ԭ��
		CQString _desc; //��������
	};

	class CErrorScheduleRsp : public IPacket //0x8070
	{
	public:
		CErrorScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = UP_REPORTERROR_RSP;
			_header._seq = seq;
		}
		~CErrorScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{

			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{

			_result = pack->readByte();
			return true;
		}
	public:
		uint8_t _result; // ������
	};

	class CAutoDataScheduleReq : public IPacket //0x1022
	{
	public:
		CAutoDataScheduleReq( )
		{
			_header._type = UPLOAD_DATAINFO_REQ;
		}
		~CAutoDataScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			//pack->writeString(_sid);
			pack->writeByte( _state );

			if ( _state == 2 ) { //����
				pack->writeByte( _space );
				pack->writeShort( _weight );
			}

			pack->writeBytes( _stime, 6 );
			pack->writeInt( _srcarea );
			pack->writeInt( _destarea );
		}
		bool UnBody( CPacker *pack )
		{

			//pack->readString(_sid);
			_state = pack->readByte();

			if ( _state == 2 ) { //����
				_space = pack->readByte();
				_weight = pack->readShort();
			}

			pack->readBytes( _stime, 6 );

			_srcarea = pack->readInt();
			_destarea = pack->readInt();

			return true;
		}
	public:
		//CQString   _sid; // ������
		uint8_t _state; //״̬
		uint8_t _space; //���ؿռ�
		uint16_t _weight; //������
		uint8_t _stime[6]; //String	����ʱ��
		uint32_t _srcarea; //������
		uint32_t _destarea; //Ŀ�ĵ�
	};

	class CAutoDataScheduleRsp : public IPacket //0x8022
	{
	public:
		CAutoDataScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = UPLOAD_DATAINFO_RSP;
			_header._seq = seq;
		}

		~CAutoDataScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}
	public:
		uint8_t _result; // ������
	};

	class CUpMsgDataScheduleReq : public IPacket // 0x1060 �ն�͸��
	{
	public:
		CUpMsgDataScheduleReq( )
		{
			_header._type = UP_MSGDATA_REQ;
		}

		~CUpMsgDataScheduleReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _code );
			pack->writeString( _data );
		}

		bool UnBody( CPacker *pack )
		{
			_code = pack->readByte();
			pack->readString( _data );
			return true;
		}
	public:
		uint8_t _code; //͸������
		CQString _data; //͸������

	};

	class CUpMsgDataScheduleRsp : public IPacket // 0x8060 �ն�͸���ظ�
	{
	public:
		CUpMsgDataScheduleRsp( uint32_t seq = 0 )
		{
			_header._type = UP_MSGDATA_RSP;
			_header._seq = seq;
		}

		~CUpMsgDataScheduleRsp( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _code );
			pack->writeString( _data );
		}

		bool UnBody( CPacker *pack )
		{

			_code = pack->readByte();
			pack->readString( _data );

			return true;
		}
	public:
		uint8_t _code; //͸������
		CQString _data; //͸������
	};

	class CUpCarDataInfoReq : public IPacket //0x1023   //�ϴ������Ϣ
	{
	public:
		CUpCarDataInfoReq( )
		{
			_header._type = UP_CARDATA_INFO_REQ;
		}

		~CUpCarDataInfoReq( )
		{
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _sid, sizeof ( _sid ) );
			pack->writeByte( _status );

			if ( _status == 0 ) {
				pack->writeInt( _price );
			}
		}
		bool UnBody( CPacker *pack )
		{

			pack->readBytes( _sid, sizeof ( _sid ) );

			_status = pack->readByte();

			if ( _status == 1 ) {
				_price = pack->readInt();
			}
			return true;
		}
	public:
		uint8_t _sid[20]; // String	�������
		uint8_t _status; //״̬(0.�ѱ��� 1.�ܾ� 2.ȡ��)
		uint32_t _price; //����(����0.01 Ԫ),���״̬Ϊ0���и��ֶ�
	};

	class CSendTextMsgReq : public IPacket // 0x1030 �ı���Ϣ�·�
	{
	public:
		CSendTextMsgReq( )
		{
			_header._type = SEND_TEXT_MSG_REQ;
		}
		~CSendTextMsgReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeByte( _type );
			pack->writeString( _content );
			pack->writeBytes( _time, sizeof ( _time ) );
		}

		bool UnBody( CPacker *pack )
		{
			_type = pack->readByte();
			pack->readString( _content );
			pack->readBytes( _time, sizeof ( _time ) );
			return true;
		}

	public:
		unsigned char _type; //��������
		CQString _content; //��������
		unsigned char _time[6]; //����ʱ��
	};

	class CSendTextMsgRsp : public IPacket // 0x8030 �ı���Ϣ�ظ�
	{
	public:
		CSendTextMsgRsp( uint32_t seq = 0 )
		{
			_header._type = SEND_TEXT_MSG_RSP;
			_header._seq = seq;
		}
		~CSendTextMsgRsp( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeByte( _result );
		}

		bool UnBody( CPacker *pack )
		{
			_result = pack->readByte();
			return true;
		}

	public:
		uint8_t _result; // ������
	};

	//�·������Ϣ
	class CSendCarDataInfoMsgReq:public IPacket //0x1021 �·������Ϣ
	{
	public:
			CSendCarDataInfoMsgReq(){
				_header._type = SEND_CARDATA_INFO_REQ;
			}

			~CSendCarDataInfoMsgReq()
			{

			}

			void Body( CPacker *pack )
			{
			   pack->writeBytes(_sid,sizeof(_sid));
			   pack->writeString(_stype);
			   pack->writeShort(_num);
			   pack->writeInt(_model);
			   pack->writeInt(_bulk);
			   pack->writeInt(_sarea);
			   pack->writeInt(_darea);
			   pack->writeBytes(_submit_time,sizeof(_submit_time));
			   pack->writeBytes(_arrival_time,sizeof(_arrival_time));
			   pack->writeInt(_price);
			   pack->writeBytes(_send_time,sizeof(_send_time));
			   pack->writeBytes(_end_time,sizeof(_end_time));
			}

			bool UnBody( CPacker *pack )
			{
			   pack->readBytes(_sid,sizeof(_sid));
			   pack->readString(_stype);
			  _num   = pack->readShort();
			  _model = pack->readInt();
			  _bulk  = pack->readInt();

			  _sarea = pack->readInt();
			  _darea = pack->readInt();

			  pack->readBytes(_submit_time,sizeof(_submit_time));
			  pack->readBytes(_arrival_time,sizeof(_arrival_time));

			  _price = pack->readInt();

			  pack->readBytes(_send_time,sizeof(_send_time));
			  pack->readBytes(_end_time,sizeof(_end_time));

			  return true;
			}
	public:
			uint8_t   _sid[20];//�������
			CQString  _stype;//��������
			uint16_t  _num;//����
			uint32_t  _model;//������(��)
			uint32_t  _bulk;//�����(������ )
			uint32_t  _sarea;//������
			uint32_t  _darea;//Ŀ�ĵ�
			uint8_t   _submit_time[6];//���ʱ��(BCD)
			uint8_t   _arrival_time[6];//����ʱ��(BCD)
			uint32_t  _price;//����(���� 0.01Ԫ)
			uint8_t   _send_time[6];//����ʱ��(BCD)
			uint8_t   _end_time[6];//��Ч��ֹʱ��(BCD)
	};
	//����ɽ�״̬ȷ��
	class CCarData_Info_Confirm_Req:public IPacket //0x1024 ����ɽ�״̬ȷ��
	{
	public:
			CCarData_Info_Confirm_Req(){
				_header._type = SEND_CARDATA_INFO_CONFIRM_REQ;
			}
			~CCarData_Info_Confirm_Req()
			{

			}
			void Body( CPacker *pack )
			{
				pack->writeBytes(_sid,sizeof(_sid));
				pack->writeByte(_status);
			}
			bool UnBody( CPacker *pack )
			{
				pack->readBytes(_sid,sizeof(_sid));
				_status = pack->readByte();

				return true;
			}
	public:
			uint8_t   _sid[20];//�������
			uint8_t   _status;//״̬
	};

	/*�����б� dtxi*/
	class COrderList : public share::Ref
	{
	public:
			COrderList() {

			}

			~COrderList() {

			}

			bool UnPack(CPacker *pack) {

			  pack->readBytes(_sid,sizeof(_sid));
			  pack->readBytes(_dsid,sizeof(_dsid));

			  pack->readString(_stype);
			  _num   = pack->readShort();
			  _model = pack->readInt();
			  _bulk  = pack->readInt();

			  pack->readString(_company);
			  pack->readBytes(_s_contact,sizeof(_s_contact));
			  pack->readBytes(_sphone,sizeof(_sphone));
			  pack->readBytes(_s_submit_time,sizeof(_s_submit_time));
			  pack->readString(_s_address);

			  _s_lon = pack->readInt();
			  _s_lat = pack->readInt();

			  pack->readBytes(_r_contact,sizeof(_r_contact));
			  pack->readBytes(_rphone,sizeof(_rphone));
			  pack->readBytes(_s_arrival_time,sizeof(_s_arrival_time));

			  pack->readString(_r_address);

			  _r_lon  = pack->readInt();
			  _r_lat  = pack->readInt();
			  _status = pack->readByte();

			  return true;
			}

			void Pack(CPacker *pack) {
			  pack->writeBytes(_sid,sizeof(_sid));
			  pack->writeBytes(_dsid,sizeof(_dsid));
			  pack->writeString(_stype);
			  pack->writeShort(_num);
			  pack->writeInt(_model);
			  pack->writeInt(_bulk);

			  pack->writeString(_company);
			  pack->writeBytes(_s_contact,sizeof(_s_contact));
			  pack->writeBytes(_sphone,sizeof(_sphone));
			  pack->writeBytes(_s_submit_time,sizeof(_s_submit_time));
			  pack->writeString(_s_address);
			  pack->writeInt(_s_lon);
			  pack->writeInt(_s_lat);

			  pack->writeBytes(_r_contact,sizeof(_r_contact));
			  pack->writeBytes(_rphone,sizeof(_rphone));
			  pack->writeBytes(_s_arrival_time,sizeof(_s_arrival_time));

			  pack->writeString(_r_address);

			  pack->writeInt(_r_lon);
			  pack->writeInt(_r_lat);
			  pack->writeByte(_status);

			}
	public:
			  uint8_t   _sid[20];//String ½����ˮ��
			  uint8_t   _dsid[20];//������
			  CQString  _stype;//��������
			  uint16_t  _num;//����
			  uint32_t  _model;//��������(��)
			  uint32_t  _bulk;//�������(������)
			  CQString  _company;//������λ
			  uint8_t   _s_contact[12];//�����ϵ��
			  uint8_t   _sphone[6];//����ֻ���
			  uint8_t   _s_submit_time[6];//���ʱ��
			  CQString  _s_address;//�����ַ
			  uint32_t  _s_lon;//�������
			  uint32_t  _s_lat;//���γ��
			  uint8_t   _r_contact[12];//�ջ���ϵ��
			  uint8_t   _rphone[6];//�ջ��ֻ���
			  uint8_t   _s_arrival_time[6];//Ҫ�󵽴�ʱ��
			  CQString  _r_address;//�ջ���ַ
			  uint32_t  _r_lon;//�ջ�����
			  uint32_t  _r_lat;//�ջ�γ��
			  uint8_t   _status;//����״̬(0.����� 1.������)
	};

	//������ϸ��ѯ�б�
	class CQueryOrderFormList : public share::Ref
	{
	public:
		CQueryOrderFormList( )
		{
		}
		;
		~CQueryOrderFormList( )
		{
		}
		;

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _sid, sizeof ( _sid ) );
			pack->writeBytes( _order_form_sid, sizeof ( _order_form_sid ) );
		}

		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _sid, sizeof ( _sid ) );
			pack->readBytes( _order_form_sid, sizeof ( _order_form_sid ) );
			return true;
		}
	public:
		unsigned char _sid[20]; //String �˵���
		unsigned char _order_form_sid[20]; //������
	};

	class CQueryOrderFromInfoReq : public IPacket // 1026 ������ϸ��ѯ
	{

	public:
		CQueryOrderFromInfoReq( )
		{
			_header._type = UP_QUERY_ORDER_FORM_INFO_REQ;
		}

		~CQueryOrderFromInfoReq( )
		{

		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _sid, sizeof ( _sid ) );
			pack->writeBytes( _order_form_sid, sizeof ( _order_form_sid ) );
		}

		bool UnBody( CPacker *pack )
		{

			pack->readBytes( _sid, sizeof ( _sid ) );
			pack->readBytes( _order_form_sid, sizeof ( _order_form_sid ) );
			return true;
		}
	public:
		uint8_t _sid[20]; //String �˵���
		uint8_t _order_form_sid[20]; //������
	};

	class CQueryOrderFromInfoRsp : public IPacket //8026 ������ϸ��ѯ��Ӧ
	{
	public:
		CQueryOrderFromInfoRsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_ORDER_FORM_INFO_RSP;
			_header._seq = seq;
		}

		~CQueryOrderFromInfoRsp( )
		{
			if ( lpOrderList != NULL ) {
				delete lpOrderList;
				lpOrderList = NULL;
			}
		}

		void Body( CPacker *pack )
		{
			lpOrderList->Pack( pack );
		}

		bool UnBody( CPacker *pack )
		{
			lpOrderList = new COrderList;
			if ( ! lpOrderList->UnPack( pack ) ) {
				delete lpOrderList;
				return false;
			}
			return true;
		}
	public:
		COrderList *lpOrderList;
	};

	class CUpOrderFromInfoReq : public IPacket //0x1027   �ϴ����˶���״̬
	{

	public:
		CUpOrderFromInfoReq( )
		{

			_header._type = UP_ORDER_FORM_INFO_REQ;

		}
		~CUpOrderFromInfoReq( )
		{

		}

		void Body( CPacker *pack )
		{

			pack->writeBytes( _sid, sizeof ( _sid ) );
			pack->writeBytes( _order_form, sizeof ( _order_form ) );
			pack->writeByte( _action );
			pack->writeByte( _status );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}

		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _sid, sizeof ( _sid ) );
			pack->readBytes( _order_form, sizeof ( _order_form ) );

			_action = pack->readByte();
			_status = pack->readByte();
			_lon = pack->readInt();
			_lat = pack->readInt();

			return true;
		}
	public:
		uint8_t  _sid[20]; //String ½����ˮ��
		uint8_t  _order_form[20]; //String ��������
		uint8_t  _action; //���ȵ�����(0.��� 1.���� 2.���ǩ�� 3.�쳣ǩ�� 4.�쳣)
		uint8_t  _status; //״̬(0.���� 1.�쳣)
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};

	class CTransportFormInfoReq : public IPacket //0x1028   �ϴ��˵�״̬
	{
	public:
		CTransportFormInfoReq( )
		{
			_header._type = UP_TRANSPORT_FORM_INFO_REQ;

		}
		~CTransportFormInfoReq( )
		{

		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _sid, sizeof ( _sid ) );
			pack->writeByte( _action );
			pack->writeByte( _status );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _sid, sizeof ( _sid ) );

			_action = pack->readByte();
			_status = pack->readByte();
			_lon = pack->readInt();
			_lat = pack->readInt();

			return true;
		}
	public:
		uint8_t  _sid[20]; //String ½����ˮ��
		uint8_t  _action; //���ȵ�����(0.��� 1.���� 2.���ǩ�� 3.�쳣ǩ�� 4.�쳣)
		uint8_t  _status; //״̬(0.���� 1.�쳣)
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};

	class CSend_Order_Form_Info_Req:public IPacket //0x102E ������ϸ����
	{

	public:
		CSend_Order_Form_Info_Req(){
			_header._type = SEND_ORDER_FORM_REQ;
			lpOrderList   = NULL;
		}

		~CSend_Order_Form_Info_Req(){

		  if (lpOrderList != NULL){
			  delete lpOrderList;
			  lpOrderList = NULL;
		  }
		}
		void Body(CPacker *pack) {
			lpOrderList->Pack(pack);
		}

		bool UnBody(CPacker *pack) {
			lpOrderList = new COrderList;
			if (!lpOrderList->UnPack(pack)) {
			  delete lpOrderList;
			  return false;
			}
			return true;
		}

	public:
		 COrderList *lpOrderList;
	};

	//�·��˵���Ϣ
	class CSend_Transport_Order_Form_Info_Req:public IPacket  //0x1025 �·��˵���Ϣ
	{
		   typedef std::vector<COrderList*> COrderListVec;
	public:
			CSend_Transport_Order_Form_Info_Req(){
				_header._type = SEND_TRANSPORT_ORDER_FORM_REQ;
			}

			~CSend_Transport_Order_Form_Info_Req(){
				Clear();
			}
			void Body(CPacker *pack) {

				pack->writeBytes(_sid,sizeof(_sid));
				pack->writeInt(_sarea);
				pack->writeInt(_darea);
				pack->writeByte(_status);
				pack->writeBytes(_send_time,sizeof(_send_time));
				pack->writeByte(_num);

				// �������������
				for (int i = 0; i < _num; ++i) {
				   _vec[i]->Pack(pack);
				}
			}
			bool UnBody(CPacker *pack) {

				pack->readBytes(_sid,sizeof(_sid));

				_sarea  = pack->readInt();
				_darea  = pack->readInt();
				_status = pack->readByte();
				pack->readBytes(_send_time,sizeof(_send_time));
				_num    = pack->readByte();

				for (int i = 0; i < _num; ++i) {
					COrderList *info = new COrderList;
				   if (!info->UnPack(pack)) {
					  delete info;
					  continue;
				   }
				   info->AddRef();
				   _vec.push_back(info);
				}
				return  true;
			}
	private:
		// �������ж�������
		void Clear(void) {
			if (_vec.empty())
				return;
			for (int i = 0; i < (int) _vec.size(); ++i) {
				_vec[i]->Release();
			}
			_vec.clear();
		}
	public:
			uint8_t    _sid[20];//�˵���
			uint32_t   _sarea;//������
			uint32_t   _darea;//Ŀ�ĵ�
			uint8_t    _status;//�˵�״̬(0.����� 1.������)
			uint8_t    _send_time[6];//����ʱ��
			uint8_t    _num;//������

			COrderListVec _vec;
	};

	class CPlatFormCommonRsp : public IPacket //8000   ƽ̨���ն˵�ͨ��Ӧ��
	{
	public:
		CPlatFormCommonRsp( uint32_t seq = 0 )
		{
			_header._type = PLATFORM_COMMON_RSP;
			_header._seq  = seq;
		}
		~CPlatFormCommonRsp( )
		{

		}
		bool UnBody( CPacker *pack )
		{
			_type = pack->readShort();
			_result = pack->readByte();
			return true;
		}
		void Body( CPacker *pack )
		{
			pack->writeShort( _type );
			pack->writeByte( _result );
		}
	public:
		uint16_t _type; //��������
		uint8_t _result; //���
	};
	class CTerminalCommonRsp : public IPacket //0x1000 �ն�ͨ��Ӧ��
	{
	public:
		CTerminalCommonRsp( uint32_t seq = 0 )
		{

			_header._type = TERMINAL_COMMON_RSP;
			_header._seq  = seq;
		}
		~CTerminalCommonRsp( )
		{

		}
		bool UnBody( CPacker *pack )
		{
			_type = pack->readShort();
			_result = pack->readByte();
			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeShort( _type );
			pack->writeByte( _result );
		}
	public:
		uint16_t _type; //��������
		uint8_t _result; //���
	};

	class CTruckUnPackMgr : public IUnPackMgr
	{
	public:
		CTruckUnPackMgr(){}
		~CTruckUnPackMgr(){}

		// ʵ�����ݽ���ӿڷ���
		IPacket * UnPack( unsigned short msgtype, CPacker &pack )
		{
			IPacket *msg = NULL ;
			switch( msgtype )
			{
			case SEND_TEXT_MSG_REQ:   // �ı���Ϣ�·�
				msg = UnPacket<CSendTextMsgReq>( pack, "CSendTextMsgReq" ) ;
				break;
			case SEND_TEXT_MSG_RSP:   // �ı���Ϣ��Ӧ
				msg = UnPacket<CSendTextMsgRsp>( pack, "CSendTextMsgRsp" ) ;
				break;
			case QUERY_CARDATA_REQ:   // ��ѯ�����Ϣ
				msg = UnPacket<CQueryCarDataReq>( pack, "CQueryCarDataReq" ) ;
				break ;
			case QUERY_CARDATA_RSP:
				msg = UnPacket<CQueryCarDataRsp>( pack, "CQueryCarDataRsp" ) ;
				break ;
			case UPLOAD_DATAINFO_REQ: // �Զ��ϱ������Ϣ
				msg = UnPacket<CAutoDataScheduleReq>(pack,"CAutoDataScheduleReq");
				break ;
			case UPLOAD_DATAINFO_RSP:// 0x8022
				msg = UnPacket<CAutoDataScheduleRsp>(pack,"CAutoDataScheduleRsp");
				break;
			case SUBSCRIBE_REQ: //0x1050    ���Ĺ���
				msg = UnPacket<CSubscrbeReq>(pack,"CSubscrbeReq");
				break ;
			case SUBSCRIBE_RSP:// 0x8050
				msg = UnPacket<CSubscrbeRsp>(pack,"CSubscrbeRsp");
				break;
			case QUERY_INFO_REQ : //0x1090    ������Ϣ��ѯ
				msg = UnPacket<CQueryInfoReq>(pack,"CQueryInfoReq");
				break ;
			case QUERY_INFO_RSP:// 0x8090
				msg = UnPacket<CQueryInfoRsp>(pack,"CQueryInfoRsp");
				break;
			case UP_REPORTERROR_REQ:  // �ϴ���������
				msg = UnPacket<CErrorScheduleReq>(pack,"CErrorScheduleReq");
				break;
			case UP_REPORTERROR_RSP: //0x8070
				msg = UnPacket<CErrorScheduleRsp>(pack,"CErrorScheduleRsp");
				break;
			// ���沿��Ϊ˦��ҵ��Ĵ���
			case SEND_SCHEDULE_REQ:  // 0x1040   // �·����ȵ�����
				msg = UnPacket<CSendScheduleReq>( pack, "CSendScheduleReq" ) ;
				break ;
			case SEND_SCHEDULE_RSP:	 // 0x8040	 // �·����ȵ���Ӧ
				msg = UnPacket<CSendScheduleRsp>( pack, "CSendScheduleRsp" ) ;
				break ;
			case RESULT_SCHEDULE_REQ: // 0x1045 // �����ϱ��·���������Ӧ��
				msg = UnPacket<CResultScheduleReq>( pack, "CResultScheduleReq" ) ;
				break ;
			case RESULT_SCHEDULE_RSP: // 0x8045 // �����ϱ��·����ȵ�����Ӧ
				msg = UnPacket<CResultScheduleRsp>( pack, "CResultScheduleRsp" ) ;
				break ;
			case QUERY_SCHEDULE_REQ:	// 0x1041	 // ��ѯ���ȵ�����
				msg = UnPacket<CQueryScheduleReq>( pack, "CQueryScheduleReq" ) ;
				break ;
			case QUERY_SCHEDULE_RSP:	// 0x8041	 // ��ѯ���ȵ���Ӧ
				msg = UnPacket<CQueryScheduleRsp>( pack, "CQueryScheduleRsp" ) ;
				break ;
			case UPLOAD_SCHEDULE_REQ: 	// 0x1042	 // �ϴ����ȵ�
				msg = UnPacket<CUploadScheduleReq>( pack, "CUploadScheduleReq" ) ;
				break ;
			case UPLOAD_SCHEDULE_RSP:	// 0x8042	 // �ϴ����ȵ���Ӧ
				msg = UnPacket<CUploadScheduleRsp>( pack, "CUploadScheduleRsp" ) ;
				break ;
			case STATE_SCHEDULE_REQ: 	//	0x1043	 // �ϱ����ȵ�״̬
				msg = UnPacket<CStateScheduleReq>( pack, "CStateScheduleReq" ) ;
				break ;
			case STATE_SCHEDULE_RSP:	//	0x8043	 // �ϱ����ȵ���Ӧ
				msg = UnPacket<CStateScheduleRsp>( pack, "CStateScheduleRsp" ) ;
				break ;
			case ALARM_SCHEDULE_REQ: 	//	0x1044	 // ���Ҹ澯
				msg = UnPacket<CAlarmScheduleReq>(pack, "CAlarmScheduleReq");
				break ;
			case ALARM_SCHEDULE_RSP:	//	0x8044	 //
				msg = UnPacket<CAlarmScheduleRsp>(pack, "CAlarmScheduleRsp");
				break ;
			case UP_MSGDATA_REQ: //0x1060 ͸��
				msg = UnPacket<CUpMsgDataScheduleReq >( pack, "CUpMsgDataScheduleReq");
				break;
			case UP_MSGDATA_RSP:// 0x8060
				msg = UnPacket<CUpMsgDataScheduleRsp >( pack, "CUpMsgDataScheduleRsp");
				break;
			case SEND_TEAMMEDIA_REQ://0x1019     ��������
				msg = UnPacket<CSendTeamMediaReq>(pack,"CSendTeamMediaReq");
				break;
			case SEND_TEAMMEDIA_RSP://0x8019  ����������Ӧ
				msg = UnPacket<CSendTeamMediaRsp>(pack,"CSendTeamMediaRsp");
				break;
			case SEND_MEDIADATA_REQ: //0x1018    ��������
				msg = UnPacket<CSendMediaDataReq>(pack,"CSendMediaDataReq");
				break;
			case SEND_MEDIADATA_RSP: //0x8018 ����������Ӧ
				msg = UnPacket<CSendMediaDataRsp>(pack,"CSendMediaDataRsp");
				break;
			case INFO_PRIMCAR_REQ: //0x1017 ����ͷ����Ϣ
				msg = UnPacket<CInfoPriMcarReq>(pack,"CInfoPriMcarReq");
				break;
			case INFO_PRIMCAR_RSP://0x8017 ����ͷ����Ϣ��Ӧ
				msg = UnPacket<CInfoPriMcarRsp>(pack,"CInfoPriMcarRsp");
				break;
			case SET_PRIMCAR_REQ://0x1016 ���ñ���Ϊͷ��
				msg = UnPacket<CSetPriMcarReq>(pack,"CSetPriMcarReq");
				break;
			case SET_PRIMCAR_RSP://0x8016 ���ñ���Ϊͷ����Ӧ
				msg = UnPacket<CSetPriMcarRsp>(pack,"CSetPriMcarRsp");
				break;
			case INVITE_NUMBER_REQ://0x1015 ���ӳ�Ա����
				msg = UnPacket<CInviteNumberReq>(pack,"CInviteNumberReq");
				break;
			case INVITE_NUMBER_RSP://0x8015 ���ӳ�Ա������Ӧ
				msg = UnPacket<CInviteNumberRsp>(pack,"CInviteNumberRsp");
				break;
			case ADD_CARTEAM_REQ: //0x1014 ��������
				msg = UnPacket<CAddCarTeamReq>(pack,"CAddCarTeamReq");
				break;
			case ADD_CARTEAM_RSP://0x8014  ����������Ӧ
				msg = UnPacket<CAddCarTeamRsp>(pack,"CAddCarTeamRsp");
				break;
			case GET_FRIENDLIST_REQ://��ȡ�����б�
				msg = UnPacket<CGetFriendListReq>(pack,"CGetFriendListReq");
				break;
			case GET_FRIENDLIST_RSP://��ȡ�����б���Ӧ
				msg = UnPacket<CGetFriendListRsp>(pack,"CGetFriendListRsp");
				break;
			case INVITE_FRIEND_REQ://���복��
				msg = UnPacket<CInviteFriendReq>(pack,"CInviteFriendReq");
				break;
			case INVITE_FRIEND_RSP://���복����Ӧ
				msg = UnPacket<CInviteFriendRsp>(pack,"CInviteFriendRsp");
				break;
			case ADD_FRIEND_REQ://��Ӻ���
				msg = UnPacket<CAddFriendsReq>(pack,"CAddFriendsReq");
				break;
			case ADD_FRIEND_RSP://��Ӻ�����Ӧ
				msg = UnPacket<CAddFriendsRsp>(pack,"CAddFriendsRsp");
				break;
			case QUERY_FRIENDS_REQ://���Ҹ����ĺ���
				msg = UnPacket<CQueryFriendsReq>(pack,"CQueryFriendsReq");
				break;
			case QUERY_FRIENDS_RSP://���Ҹ����ĺ�����Ӧ
				msg = UnPacket<CQueryFriendsRsp>(pack,"CQueryFriendsRsp");
				break;
			case DRIVER_LOGOUT_REQ://˾��ע��
				msg = UnPacket<CDriverLoginOutReq>(pack,"CDriverLoginOutReq");
				break;
			case DRIVER_LOGOUT_RSP://˾��ע����Ӧ
				msg = UnPacket<CDriverLoginOutRsp>(pack,"CDriverLoginOutRsp");
				break;
			case DRIVER_LOGIN_REQ://˾����¼��֤
				msg = UnPacket<CDriverLoginReq>(pack,"CDriverLoginReq");
				break;
			case DRIVER_LOGIN_RSP://˾����¼��֤��Ӧ
				msg = UnPacket<CDriverLoginRsp>(pack,"CDriverLoginRsp");
				break;
			case UP_CARDATA_INFO_REQ://�ϴ������Ϣ
				msg = UnPacket<CUpCarDataInfoReq>(pack,"CUpCarDataInfoReq");
				break;
			case UP_QUERY_ORDER_FORM_INFO_REQ://������ϸ��ѯ
				msg = UnPacket<CQueryOrderFromInfoReq>(pack,"CQueryOrderFromInfoReq");
				break;
			case UP_ORDER_FORM_INFO_REQ://�ϴ����˶���״̬
				msg = UnPacket<CUpOrderFromInfoReq>(pack,"CUpOrderFromInfoReq");
				break;
			case UP_TRANSPORT_FORM_INFO_REQ://�ϴ��˵�״̬
				msg = UnPacket<CTransportFormInfoReq>(pack,"CTransportFormInfoReq");
				break;
			case TERMINAL_COMMON_RSP://�ն�ͨ��Ӧ��
				msg = UnPacket<CTerminalCommonRsp>(pack,"CTerminalCommonRsp");
				break;

			default:
				break ;
			}
			return msg;
		}
	};
};

#pragma pack()

#endif /* HEADER_H_ */
