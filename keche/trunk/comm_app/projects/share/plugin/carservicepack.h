/*
 * header.h
 *
 *  Created on: 2012-5-31
 *      Author: humingqing
 *   ����������������
 */

#ifndef __CAR_SERVICE_PACK_H__
#define __CAR_SERVICE_PACK_H__

#include <msgpack.h>
#include <packfactory.h>

#pragma pack(1)

namespace CarService {
	//�������
	#define TERMINAL_COMMON_RSP                   0x1000   //�ն�ͨ�ûظ�
	#define PLATFORM_COMMON_RSP                   0x8000   //ƽ̨ͨ�ûظ�

	#define UP_LOGIN_INFO_REQ                     0x1001   //�û���¼
	#define UP_LOGIN_INFO_RSP                     0x8001   //�û���¼��Ӧ

	#define UP_QUERY_BALLANCE_LIST_REQ            0x1002   //����������ѯ
	#define UP_QUERY_BALLANCE_LIST_RSP            0x8002   //����������ѯ��Ӧ

	#define UP_QUERY_STORE_LIST_REQ               0x1003   //��ѯ�ŵ�
	#define UP_QUERY_STORE_LIST_RSP               0x8003   //��ѯ�ŵ���Ӧ

	#define UP_QUERY_VIEW_STORE_INFO_REQ          0x1004   //��ѯ�ŵ�����
	#define UP_QUERY_VIEW_STORE_INFO_RSP          0x8004   //��ѯ�ŵ�������Ӧ

	#define UP_QUERY_DISCOUNT_LIST_REQ            0x1005   //�°汾��ѯ�Ż���Ϣ
	#define UP_QUERY_DISCOUNT_LIST_RSP            0x8005   //�°汾��ѯ�Ż���Ϣ��Ӧ

	#define UP_VIEW_DISCOUNT_INFO_REQ             0x1006   //�°汾��ѯ�Ż���Ϣ��ϸ�б�
	#define UP_VIEW_DISCOUNT_INFO_RSP             0x8006   //�°汾��ѯ�Ż���Ϣ��ϸ�б���Ӧ

	#define UP_QUERY_TRADE_LIST_REQ               0x1007   //��ʷ���׼�¼��ѯ
	#define UP_QUERY_TRADE_LIST_RSP               0x8007   //��ʷ���׼�¼��ѯ��Ӧ

	#define UP_QUERY_FAVORITE_LIST_REQ            0x1008   //��ѯ�ղ��б�
	#define UP_QUERY_FAVORITE_LIST_RSP            0x8008   //��ѯ�ղ��б���Ӧ

	#define UP_VIEW_FAVORITE_INFO_REQ             0x1009   //��ѯ�ղ��б�����
	#define UP_VIEW_FAVORITE_INFO_RSP             0x8009   //��ѯ�ղ��б�������Ӧ

	#define UP_ADD_FAVORITE_REQ                   0x100A   //����ղ�����

	#define UP_DEL_FAVORITE_REQ                   0x100B   //ɾ���ղ�����

	#define UP_GET_DESTINATION_REQ                0x100C   //��ȡĿ�ĵ�

	#define SEND_DETAIL_DISCOUNT_INFO_REQ         0x102B //�·��Ż���Ϣ

	#define UP_DISCOUNT_INFO_REQ                  0x1029 //��ѯ�����Ż���Ϣ
	#define UP_DISCOUNT_INFO_RSP                  0x8029 //��ѯ�����Ż���Ϣ��Ӧ

	#define UP_DETAIL_DISCOUNT_INFO_REQ           0x102A //��ѯ��������Ż���Ϣ
	#define UP_DETAIL_DISCOUNT_INFO_RSP           0x802A //��ѯ��������Ż���Ϣ��Ӧ

	#define UP_UNION_BUSINESS_INFO_REQ            0x102C //��ѯ�����̼���Ϣ
	#define UP_UNION_BUSINESS_INFO_RSP            0x802C //��ѯ�����̼���Ϣ��Ӧ

	#define UP_DETAIL_UNION_BUSINESS_INFO_REQ     0x102D //��ѯ����������̼���Ϣ
	#define UP_DETAIL_UNION_BUSINESS_INFO_RSP     0x802D //��ѯ����������̼���Ӧ

	#define SEND_DESTINATION_INFO_REQ             0x100D //�·�Ŀ�ĵ�

	/*������*/
	class CCOMMON_REQ
	{
	public:
		CCOMMON_REQ( )
		{
			memset( _usercode, 0, sizeof ( _usercode ) );
			memset( _verifyCode, 0, sizeof ( _verifyCode ) );
		}
		virtual ~CCOMMON_REQ(){};

	public:
		unsigned char _usercode[32]; //�û�����
		unsigned char _verifyCode[30]; //�����֤ʶ����
	};

	//�û���¼
	class CLoginInfoReq : public IPacket //0x1001 �û���¼
	{
	public:
		CLoginInfoReq( )
		{
			_header._type = UP_LOGIN_INFO_REQ;
		}

		~CLoginInfoReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _phone, sizeof ( _phone ) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _phone, sizeof ( _phone ) );
			return true;
		}
	public:
		unsigned char _phone[32]; //�ֻ�����
	};
	/*�û���¼��Ӧ*/
	class CLoginInfoRsp : public CCOMMON_REQ , public IPacket//0x8001 �û���¼��Ӧ
	{
	public:
		CLoginInfoRsp( uint32_t seq = 0 )
		{
			_header._type = UP_LOGIN_INFO_RSP;
			_header._seq = seq;
		}

		~CLoginInfoRsp( )
		{
		}
		;

		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
		}
	};

	//����������ѯ
	class CQuery_Ballance_List_Req : public CCOMMON_REQ , public IPacket  //0x1002
	{
	public:
		CQuery_Ballance_List_Req( )
		{
			_header._type = UP_QUERY_BALLANCE_LIST_REQ;
		}

		~CQuery_Ballance_List_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );
			return true;
		}
	};

	//����������ѯ��Ӧ
	class CQuery_Ballance_List_Rsp : public IPacket // 0x8002
	{
	public:
		CQuery_Ballance_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_BALLANCE_LIST_RSP;
			_header._seq = seq;
		}
		~CQuery_Ballance_List_Rsp( )
		{
		}
		;

		void Body( CPacker *pack )
		{
			pack->writeBytes( _car_num, sizeof ( _car_num ) );
			pack->writeBytes( _vehicle_num, sizeof ( _vehicle_num ) );
			pack->writeInt( _balance );
			pack->writeByte( _status );

		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _car_num, sizeof ( _car_num ) );
			pack->readBytes( _vehicle_num, sizeof ( _vehicle_num ) );

			_balance = pack->readInt();
			_status = pack->readByte();
			return true;
		}
	public:
		uint8_t _car_num[20];  //��������
		uint8_t _vehicle_num[12];  //���ƺ�
		uint32_t _balance;  //���������
		uint8_t _status;  //������״̬
	};
	//��ѯ�ŵ�
	class CQuery_Store_List_Req : public CCOMMON_REQ , public IPacket //0x1003
	{
	public:
		CQuery_Store_List_Req( )
		{
			_header._type = UP_QUERY_STORE_LIST_REQ;
		}
		~CQuery_Store_List_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
			pack->writeByte( _scope );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			_lon = pack->readInt();
			_lat = pack->readInt();
			_scope = pack->readByte();

			return true;
		}
	public:
		uint32_t _lon; //����
		uint32_t _lat; //γ��
		uint8_t _scope; //��ѯ��Χ(0:15��1:30��2:50��255:Ԥ��)��λ:����
	};

	/*�ŵ��б�*/
	class CStore_List : public share::Ref
	{
	public:
		CStore_List( )
		{
			memset( _storeCode, 0, sizeof ( _storeCode ) );
			memset( _phone, 0, sizeof ( _phone ) );

			_type = _lon = _lat = 0;
		}
		~CStore_List( )
		{
		}
		;
		void Pack( CPacker *pack )
		{
			pack->writeByte( _type );
			pack->writeBytes( _storeCode, sizeof ( _storeCode ) );
			pack->writeString( _storeName );
			pack->writeString( _address );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}
		bool UnPack( CPacker *pack )
		{
			_type = pack->readByte();
			pack->readBytes( _storeCode, sizeof ( _storeCode ) );
			pack->readString( _storeName );
			pack->readString( _address );
			pack->readBytes( _phone, sizeof ( _phone ) );
			_lon = pack->readInt();
			_lat = pack->readInt();
			return true;
		}
	public:
		uint8_t _type; //1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�� 0.ȫ��
		uint8_t _storeCode[20]; //�ŵ����
		CQString _storeName; //�ŵ�����
		CQString _address; //��ϸ��ַ
		uint8_t _phone[20]; //�ŵ�绰
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};

	class CQuery_Store_List_Rsp : public IPacket // 0x8003 //��ѯ�ŵ���Ӧ
	{
		typedef std::vector< CStore_List * > CStore_List_VEC;
	public:
		CQuery_Store_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_STORE_LIST_RSP;
			_header._seq = seq;
		}
		~CQuery_Store_List_Rsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CStore_List *info = new CStore_List;
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
		unsigned char _num; //��ѯ�ŵ����
		CStore_List_VEC _vec;
	};
	//��ѯ�ŵ�����
	class CView_Store_Info_Req : public CCOMMON_REQ , public IPacket //0x1004
	{
	public:
		CView_Store_Info_Req( )
		{
			_header._type = UP_QUERY_VIEW_STORE_INFO_REQ;
		}
		~CView_Store_Info_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->writeBytes( _storecode, sizeof ( _storecode ) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->readBytes( _storecode, sizeof ( _storecode ) );
			return true;
		}
	public:
		uint8_t _storecode[20]; //�ŵ����

	};

	//��ѯ�ŵ�������Ӧ
	class CView_Store_List_Rsp : public IPacket // 0x8004 //��ѯ�ŵ�������Ӧ
	{
	public:
		CView_Store_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_VIEW_STORE_INFO_RSP;
			_header._seq = seq;
		}
		~CView_Store_List_Rsp( )
		{
		}
		;

		void Body( CPacker *pack )
		{
			pack->writeString( _allianceName );
			pack->writeByte( _type );
			pack->writeBytes( _storeCode, sizeof ( _storeCode ) );
			pack->writeString( _storeName );
			pack->writeString( _address );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeString( _desc );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readString( _allianceName );
			_type = pack->readByte();
			pack->readBytes( _storeCode, sizeof ( _storeCode ) );
			pack->readString( _storeName );
			pack->readString( _address );
			pack->readBytes( _phone, sizeof ( _phone ) );
			pack->readString( _desc );
			_lon = pack->readInt();
			_lat = pack->readInt();
			return true;
		}
	public:
		CQString _allianceName; //�̼�����
		uint8_t _type; //1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�� 0.ȫ��(��λ����)
		uint8_t _storeCode[20]; //�ŵ����
		CQString _storeName; //�ŵ�����
		CQString _address; //��ϸ��ַ
		uint8_t _phone[20]; //�绰
		CQString _desc; //�ŵ�����
		uint32_t _lon; //�ŵ꾭��
		uint32_t _lat; //�ŵ�γ��
	};

	//�°汾��ѯ�Ż���Ϣ
	class CQuery_Discount_List_Req : public CCOMMON_REQ , public IPacket //0x1005
	{
	public:
		CQuery_Discount_List_Req( )
		{
			_header._type = UP_QUERY_DISCOUNT_LIST_REQ;
		}
		~CQuery_Discount_List_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->writeInt( _lon );
			pack->writeInt( _lat );
			pack->writeByte( _type );
			pack->writeInt( _offset );
			pack->writeByte( _count );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			_lon = pack->readInt();
			_lat = pack->readInt();
			_type = pack->readByte();
			_offset = pack->readInt();
			_count = pack->readByte();

			return true;
		}
	public:
		uint32_t _lon; //��������
		uint32_t _lat; //����γ��
		uint8_t _type; //����
		uint8_t _scope; //��ѯ��Χ
		uint32_t _offset; //��ȡ��ʼλ�ã�Ĭ�ϴ�0��ʼ
		uint8_t _count; //һҳ��¼����
	};
	//�°汾�Ż���Ϣ�б�
	class CDiscount_List_Info : public share::Ref
	{
	public:
		CDiscount_List_Info( )
		{
			memset( _discountCode, 0, sizeof ( _discountCode ) );
		}
		~CDiscount_List_Info( )
		{
		}
		;

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _discountCode, sizeof ( _discountCode ) );
			pack->writeString( _title );
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _discountCode, sizeof ( _discountCode ) );
			pack->readString( _title );

			return true;
		}
	public:
		uint8_t _discountCode[20]; //�Ż���Ϣ����
		CQString _title; //����
	};
	/*�Ż��ŵ��б�*/
	class CDiscount_Store_List_Info : public share::Ref
	{
		typedef std::vector< CDiscount_List_Info * > CDiscount_List_Info_VEC;
	public:
		CDiscount_Store_List_Info( )
		{
			memset( _storeCode, 0, sizeof ( _storeCode ) );
			memset( _phone, 0, sizeof ( _phone ) );

			_lon = _lat = _discount_num = 0;
		}
		~CDiscount_Store_List_Info( )
		{
			Clear();
		}

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _storeCode, sizeof ( _storeCode ) );
			pack->writeString( _storeName );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeString( _address );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
			pack->writeByte( _discount_num );

			if ( _discount_num == 0 ) return;

			for ( int i = 0 ; i < ( int ) _discount_num ; ++ i ) {
				_vec_discount[i]->Pack( pack );
			}
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _storeCode, sizeof ( _storeCode ) );
			pack->readString( _storeName );
			pack->readBytes( _phone, sizeof ( _phone ) );
			pack->readString( _address );

			_lon = pack->readInt();
			_lat = pack->readInt();
			_discount_num = pack->readByte();

			if ( _discount_num == 0 ) return false;

			for ( int i = 0 ; i < _discount_num ; ++ i ) {
				CDiscount_List_Info *info = new CDiscount_List_Info;
				if ( ! info->UnPack( pack ) ) {
					delete info;
					continue;
				}
				info->AddRef();
				_vec_discount.push_back( info );
			}
			return true;
		}
	private:
		void Clear( void )
		{
			if ( _vec_discount.empty() ) return;
			for ( int i = 0 ; i < ( int ) _vec_discount.size() ; ++ i ) {
				_vec_discount[i]->Release();
			}
			_vec_discount.clear();
		}
	public:
		uint8_t _storeCode[20]; //�ŵ����
		CQString _storeName; //�ŵ�����
		uint8_t _phone[20]; //�ŵ�绰
		CQString _address; //��ϸ��ַ
		uint32_t _lon; //����
		uint32_t _lat; //γ��
		uint8_t _discount_num; //�Ż�����

		CDiscount_List_Info_VEC _vec_discount;
	};
	//�°汾��ѯ�Ż���Ϣ�б���Ӧ
	class CQuery_Discount_List_Rsp : public IPacket // 0x8005
	{
		typedef std::vector< CDiscount_Store_List_Info * > CDiscount_Store_List_Info_VEC;
	public:
		CQuery_Discount_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_DISCOUNT_LIST_RSP;
			_header._seq = seq;
		}
		~CQuery_Discount_List_Rsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CDiscount_Store_List_Info *info = new CDiscount_Store_List_Info;
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
		uint8_t _num; //�Ż���Ϣ����
		CDiscount_Store_List_Info_VEC _vec;
	};

	//�°汾��ѯ�Ż���Ϣ��ϸ�б�
	class CView_Discount_Info_Req : public CCOMMON_REQ , public IPacket //0x1006
	{
	public:
		CView_Discount_Info_Req( )
		{
			_header._type = UP_VIEW_DISCOUNT_INFO_REQ;
		}
		~CView_Discount_Info_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->writeBytes( _discountCode, sizeof(_discountCode) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->readBytes( _discountCode, sizeof(_discountCode) );
			return true;
		}
	public:
		 uint8_t  _discountCode[20];//�Ż���Ϣ����
	};

	//�°汾��ѯ�Ż���Ϣ��ϸ�б���Ӧ
	class CView_Discount_Info_Rsp: public IPacket // 0x8006
	{
	public:
		CView_Discount_Info_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_VIEW_DISCOUNT_INFO_RSP;
			_header._seq  = seq;
		}
		~CView_Discount_Info_Rsp( ){};

		void Body( CPacker *pack )
		{
			pack->writeString(_title);
			pack->writeString(_content);

			pack->writeBytes(_beginTime,sizeof(_beginTime));
			pack->writeBytes(_endTime,sizeof(_endTime));
		}
		bool UnBody( CPacker *pack )
		{
			pack->readString(_title);
			pack->readString(_content);

			pack->readBytes(_beginTime,sizeof(_beginTime));
			pack->readBytes(_endTime,sizeof(_endTime));

			return true;
		}
	public:
		  CQString  _title;//����
		  CQString  _content;//����
		  uint8_t   _beginTime[6];//��Ч��ʼʱ��
		  uint8_t   _endTime[6];//��Ч����ʱ��
	};
	//��ʷ���׼�¼��ѯ
	class CQuery_Trade_List_Req : public CCOMMON_REQ , public IPacket //0x1007
	{
	public:
		CQuery_Trade_List_Req( )
		{
			_header._type = UP_QUERY_TRADE_LIST_REQ;
		}
		~CQuery_Trade_List_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->writeByte( _tradeType );

			pack->writeBytes( _beginTime, sizeof ( _beginTime ) );
			pack->writeBytes( _endTime, sizeof ( _endTime ) );

			pack->writeInt( _offset );
			pack->writeByte( _count );

		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			_tradeType = pack->readByte();

			pack->readBytes( _beginTime, sizeof ( _beginTime ) );
			pack->readBytes( _endTime, sizeof ( _endTime ) );

			_offset = pack->readInt();
			_count = pack->readInt();
			return true;
		}
	public:
		uint8_t _tradeType; //�������ͣ�0��ȫ����1����ֵ��2�����ѣ�
		uint8_t _beginTime[6]; //��Ч��ʼʱ��
		uint8_t _endTime[6]; //��Ч��ֹʱ��
		uint32_t _offset; //��ȡ��ʼλ�ã�Ĭ�ϴ�0��ʼ
		uint8_t _count; //һҳ��¼����
	};

	//���׼�¼�б�
	class CTrade_List : public share::Ref
	{
	public:
		CTrade_List( )
		{
			memset( _card_num, 0, sizeof ( _card_num ) );
			memset( _tradeTime, 0, sizeof ( _tradeTime ) );

			_type = _money = _tradeState = 0;
		}
		~CTrade_List( )
		{
		}
		;
		void Pack( CPacker *pack )
		{
			pack->writeBytes( _card_num, sizeof ( _card_num ) );
			pack->writeString( _allianceName );
			pack->writeString( _storeName );
			pack->writeByte( _type );
			pack->writeString( _productName );

			pack->writeInt( _money );
			pack->writeBytes( _tradeTime, sizeof ( _tradeTime ) );
			pack->writeByte( _tradeState );
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _card_num, sizeof ( _card_num ) );
			pack->readString( _allianceName );
			pack->readString( _storeName );
			_type = pack->readByte();
			pack->readString( _productName );

			_money = pack->readInt();

			pack->readBytes( _tradeTime, sizeof ( _tradeTime ) );
			_tradeState = pack->readByte();
			return true;
		}
	public:
		uint8_t _card_num[20]; //��������
		CQString _allianceName; //�̻�����
		CQString _storeName; //�ŵ�����
		uint8_t _type; //��������
		CQString _productName; //��Ʒ����
		uint32_t _money; //���׽��
		uint8_t _tradeTime[6]; //����ʱ�䣨yy-MM-dd hh:mm:ss��
		uint8_t _tradeState; //����״̬
	};

	class CQuery_Trade_List_Rsp : public IPacket // 0x8007 //��ʷ���׼�¼��ѯ��Ӧ
	{
		typedef std::vector< CTrade_List * > CTrade_List_VEC;
	public:
		CQuery_Trade_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_TRADE_LIST_RSP;
			_header._seq = seq;
		}
		~CQuery_Trade_List_Rsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CTrade_List *info = new CTrade_List;
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
		unsigned char _num; //��ѯ�������
		CTrade_List_VEC _vec;
	};
	//��ѯ�ղ��б�
	class CQuery_Favorite_List_Req : public CCOMMON_REQ , public IPacket //0x1008
	{
	public:
		CQuery_Favorite_List_Req( )
		{
			_header._type = UP_QUERY_FAVORITE_LIST_REQ;
		}
		~CQuery_Favorite_List_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->writeByte( _type );
			pack->writeByte( _offset );
			pack->writeByte( _count );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			_type = pack->readByte();
			_offset = pack->readByte();
			_count = pack->readByte();
			return true;
		}
	public:
		uint8_t _type; //�ղ����ͣ�1���ŵꣻ2���Ż���Ϣ��
		uint8_t _offset; //��ȡ��ʼλ�ã�Ĭ�ϴ�0��ʼ
		uint8_t _count; //һҳ��¼����
	};

	//�ղ���Ϣ�б�
	class CFavorite_List : public share::Ref
	{
	public:
		CFavorite_List( )
		{
			memset( _id, 0, sizeof ( _id ) );
			memset( _discountCode, 0, sizeof ( _discountCode ) );

			_type = _isDiscount = _discountState = 0;
		}
		~CFavorite_List( )
		{
		}
		;
		void Pack( CPacker *pack )
		{
			pack->writeBytes( _id, sizeof ( _id ) );
			pack->writeString( _storeCode );
			pack->writeBytes( _discountCode, sizeof ( _discountCode ) );
			pack->writeString( _favoriteName );
			pack->writeByte( _type );

			pack->writeByte( _isDiscount );
			pack->writeByte( _discountState );
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _id, sizeof ( _id ) );
			pack->readString( _storeCode );
			pack->readBytes( _discountCode, sizeof ( _discountCode ) );
			pack->readString( _favoriteName );
			_type = pack->readByte();
			_isDiscount = pack->readByte();
			_discountState = pack->readByte();
			return true;
		}
	public:
		uint8_t _id[32]; //�ղ�id
		CQString _storeCode; //�ŵ����
		uint8_t _discountCode[32]; //�Ż���Ϣ����
		CQString _favoriteName; //�ղ����ƣ��ŵ����ơ��Ż���Ϣ���⣩
		uint8_t _type; //�ղ����ͣ�1���ŵꣻ2���Ż���Ϣ
		uint8_t _isDiscount; //�Ƿ����Żݣ����ղ�����Ϊ1ʱ
		uint8_t _discountState; //�Ż���Ϣ״̬��0��������1���ѹ��ڣ�2����ʧЧ��
	};

	class CQuery_Favorite_List_Rsp : public IPacket // 0x8008 //��ѯ�ղ��б���Ӧ
	{
	public:
		typedef std::vector< CFavorite_List * > CFavorite_List_VEC;
	public:
		CQuery_Favorite_List_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_QUERY_FAVORITE_LIST_RSP;
			_header._seq = seq;
		}
		~CQuery_Favorite_List_Rsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CFavorite_List *info = new CFavorite_List;
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
		uint8_t _num; //��ѯ�������
		CFavorite_List_VEC _vec;
	};

	//��ѯ�ղ��б�����
	class CView_Favorite_Info_Req : public CCOMMON_REQ , public IPacket //0x1009
	{
	public:
		CView_Favorite_Info_Req( )
		{
			_header._type = UP_VIEW_FAVORITE_INFO_REQ;
		}
		~CView_Favorite_Info_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
			pack->writeString( _storeCode );
			pack->writeBytes( _discountCode, sizeof ( _discountCode ) );
			pack->writeByte( _type );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->readString( _storeCode );
			pack->readBytes( _discountCode, sizeof ( _discountCode ) );

			_type = pack->readByte();
			return true;
		}
	public:
		CQString _storeCode;
		uint8_t _discountCode[32]; //�Ż���Ϣ����
		uint8_t _type; //�ղ�����
	};

	//�Ż���Ϣ�б�(��)
	class CDiscount_List_New : public share::Ref
	{
	public:
		CDiscount_List_New( )
		{
			memset( _discountCode, 0, sizeof ( _discountCode ) );
			memset( _begin_time, 0, sizeof ( _begin_time ) );
			memset( _end_time, 0, sizeof ( _end_time ) );

		}
		~CDiscount_List_New( )
		{
		}
		;
		void Pack( CPacker *pack )
		{
			pack->writeBytes( _discountCode, sizeof ( _discountCode ) );
			pack->writeString( _title );
			pack->writeString( _content );

			pack->writeBytes( _begin_time, sizeof ( _begin_time ) );
			pack->writeBytes( _end_time, sizeof ( _end_time ) );
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _discountCode, sizeof ( _discountCode ) );
			pack->readString( _title );
			pack->readString( _content );

			pack->readBytes( _begin_time, sizeof ( _begin_time ) );
			pack->readBytes( _end_time, sizeof ( _end_time ) );

			return true;
		}
	public:
		uint8_t _discountCode[32]; //�Ż���Ϣ����
		CQString _title; //����
		CQString _content; //����
		uint8_t _begin_time[6]; //��Чʱ��(yyyy-MM-dd)(BCD)
		uint8_t _end_time[6]; //ʧЧʱ��(yyyy-MM-dd)(BCD)
	};

	class CView_Favorite_Info_Rsp : public IPacket // 0x8009 //��ѯ�ղ��б�������Ӧ
	{
	public:
		typedef std::vector< CDiscount_List_New * > CDiscount_List_New_VEC;
	public:
		CView_Favorite_Info_Rsp( uint32_t seq = 0 )
		{
			_header._type = UP_VIEW_FAVORITE_INFO_RSP;
			_header._seq = seq;
		}
		~CView_Favorite_Info_Rsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			pack->readString( _allianceName );
			_allianceType = pack->readInt();
			pack->readBytes( _storeCode, sizeof ( _storeCode ) );
			pack->readString( _storeName );
			pack->readBytes( _phone, sizeof ( _phone ) );
			pack->readString( _address );
			pack->readString( _desc );

			_lon = pack->readInt();
			_lat = pack->readInt();

			_num = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CDiscount_List_New *info = new CDiscount_List_New;
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
			pack->writeString( _allianceName );
			pack->writeInt( _allianceType );
			pack->writeBytes( _storeCode, sizeof ( _storeCode ) );
			pack->writeString( _storeName );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeString( _address );
			pack->writeString( _desc );

			pack->writeInt( _lon );
			pack->writeInt( _lat );

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
		CQString _allianceName; //�̼�����
		uint32_t _allianceType; //�̻�����1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�ޣ���λ����)
		uint8_t _storeCode[20]; //�ŵ����
		CQString _storeName; //�ŵ�����
		uint8_t _phone[20]; //�ŵ�绰
		CQString _address; //��ϸ��ַ
		CQString _desc; //�ŵ�����
		uint32_t _lon; //�ŵ꾭��
		uint32_t _lat; //�ŵ�γ��
		uint8_t _num; //��ѯ�������
		CDiscount_List_New_VEC _vec;
	};
	//����ղ�
	class CAdd_Favorite_Req : public CCOMMON_REQ , public IPacket //0x100A
	{
	public:
		CAdd_Favorite_Req( )
		{
			_header._type = UP_ADD_FAVORITE_REQ;
		}
		~CAdd_Favorite_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->writeBytes( _fauoriteCode, sizeof ( _fauoriteCode ) );
			pack->writeByte( _type );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->readBytes( _fauoriteCode, sizeof ( _fauoriteCode ) );
			_type = pack->readByte();
			return true;
		}
	public:
		uint8_t _fauoriteCode[32]; //�ŵ����(���Ż���Ϣ����)
		uint8_t _type; //�ղ����ͣ�1���ŵꣻ2���Ż���Ϣ��
	};

	//ɾ���ղ�
	class CDel_Favorite_Req : public CCOMMON_REQ , public IPacket //0x100B
	{
	public:
		CDel_Favorite_Req( )
		{
			_header._type = UP_DEL_FAVORITE_REQ;
		}
		~CDel_Favorite_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->writeBytes( _id, sizeof ( _id ) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			pack->readBytes( _id, sizeof ( _id ) );
			return true;
		}
	public:
		uint8_t _id[32]; //�ղ�id
	};
	//��ȡĿ�ĵ�
	class CGet_Destination_Req : public CCOMMON_REQ , public IPacket //0x100C
	{
	public:
		CGet_Destination_Req( )
		{
			_header._type = UP_GET_DESTINATION_REQ;
		}
		~CGet_Destination_Req( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _usercode, sizeof ( _usercode ) );
			pack->writeBytes( _verifyCode, sizeof ( _verifyCode ) );
		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _usercode, sizeof ( _usercode ) );
			pack->readBytes( _verifyCode, sizeof ( _verifyCode ) );

			return true;
		}
	};

	//�������
	class CQueryDiscountInfoReq : public IPacket //0x1029 ��ѯ�����Ż���Ϣ
	{
	public:
		CQueryDiscountInfoReq( )
		{
			_header._type = UP_DISCOUNT_INFO_REQ;
		}
		~CQueryDiscountInfoReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeByte( _type );
			pack->writeByte( _range );
			pack->writeByte( _sort_type );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
			pack->writeByte( _offset );
			pack->writeByte( _count );
		}
		bool UnBody( CPacker *pack )
		{
			_type = pack->readByte();
			_range = pack->readByte();
			_sort_type = pack->readByte();

			_lon = pack->readInt();
			_lat = pack->readInt();

			_offset = pack->readByte();
			_count = pack->readByte();

			return true;
		}
	public:
		uint8_t _type; //�Ż�����(0.���� 1.ά�� 2.��Ԯ 3.ʳ�� 4.����)
		uint8_t _range; //��ѯ��Χ(0.15 1.30 2. 50)��λ:����
		uint8_t _sort_type; //��������
		uint32_t _lon; //����
		uint32_t _lat; //γ��;
		uint8_t _offset; //��ȡ��ʼλ��
		uint8_t _count; //��¼����
	};

	//�Ż���Ϣ�б�
	class CDiscount_List : public share::Ref
	{
	public:
		CDiscount_List( )
		{
		}
		;
		~CDiscount_List( )
		{
		}
		;

		void Pack( CPacker *pack )
		{
			pack->writeBytes( _discount_num, sizeof ( _discount_num ) );
			pack->writeByte( _type );
			pack->writeString( _title );
			pack->writeBytes( _time, sizeof ( _time ) );
		}
		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _discount_num, sizeof ( _discount_num ) );
			_type = pack->readByte();
			pack->readString( _title );
			pack->readBytes( _time, sizeof ( _time ) );
			return true;
		}
	public:
		uint8_t _discount_num[20]; //�Żݱ��
		uint8_t _type;  //�Ż�����(1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�ޡ�0.ȫ��)
		CQString _title; //����
		uint8_t _time[6]; //��Чʱ��(BCD)
	};

	//�·��Ż���Ϣ
	class  CSend_Detail_DiscountInfoReq:public IPacket //0x102B �·��Ż���Ϣ
	{
			typedef std::vector<CDiscount_List *> CDiscount_List_VEC;
	public:
			CSend_Detail_DiscountInfoReq() {
					_header._type = SEND_DETAIL_DISCOUNT_INFO_REQ;
			 }
			 ~CSend_Detail_DiscountInfoReq() {
				  Clear();
			  }
			 bool UnBody(CPacker *pack) {
				_num    = pack->readByte();
				_action = pack->readByte();

				if (_num == 0)
				   return false;
				for (int i = 0; i < (int) _num; ++i) {
				   CDiscount_List *info = new CDiscount_List;
				   if (!info->UnPack(pack)) {
						delete info;
						continue;
					}
				   info->AddRef();
				   _vec.push_back(info);
				}
					_num = _vec.size();
					return true;
				}

				void Body(CPacker *pack) {
				   pack->writeByte(_num);
				   pack->writeByte(_action);

				   if (_num == 0)
						return;
				   for (int i = 0; i < (int) _num; ++i) {
						_vec[i]->Pack(pack);
				   }
				}
	private:
			 void Clear(void) {
				if (_vec.empty())
				   return;
				for (int i = 0; i < (int) _vec.size(); ++i) {
						_vec[i]->Release();
				}
				_vec.clear();
			 }
	public:
			  uint8_t    _num;//��ѯ����ĸ���
			  uint8_t    _action;//���� (0.��ѯ 1.����)

			  CDiscount_List_VEC _vec;//�Ż���Ϣ�б�
	};

	class CQueryDiscountInfoRsp : public IPacket //0x8029 ��ѯ�����Ż���Ϣ��Ӧ
	{
		typedef std::vector< CDiscount_List * > CDiscount_List_VEC;
	public:
		CQueryDiscountInfoRsp( uint32_t seq = 0 )
		{

			_header._type = UP_DISCOUNT_INFO_RSP;
			_header._seq = seq;
		}
		~CQueryDiscountInfoRsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			_action = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CDiscount_List *info = new CDiscount_List;
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
			pack->writeByte( _action );

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
		uint8_t _num; //��ѯ����ĸ���
		uint8_t _action; //���� (0.��ѯ 1.����)

		CDiscount_List_VEC _vec; //�Ż���Ϣ�б�
	};

	class CQueryDetailiscountInfoReq : public IPacket //0x102A ��ѯ��������Ż���Ϣ
	{
	public:
		CQueryDetailiscountInfoReq( )
		{

			_header._type = UP_DETAIL_DISCOUNT_INFO_REQ;
		}

		~CQueryDetailiscountInfoReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _discount_num, sizeof ( _discount_num ) );
		}

		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _discount_num, sizeof ( _discount_num ) );
			return true;
		}
	public:
		uint8_t _discount_num[20]; //�Żݱ��
	};

	class CQueryDetailiscountInfoRsp : public IPacket //0x802A ��ѯ��������Ż���Ϣ
	{
	public:
		CQueryDetailiscountInfoRsp( uint32_t seq = 0 )
		{
			_header._type = UP_DETAIL_DISCOUNT_INFO_RSP;
			_header._seq = seq;

			memset( & _phone, 0, sizeof ( _phone ) );
		}
		~CQueryDetailiscountInfoRsp( )
		{

		}
		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _discount_num, sizeof ( _discount_num ) );
			pack->readBytes( _business_num, sizeof ( _business_num ) );

			pack->readString( _title );
			pack->readBytes( _time, sizeof ( _time ) );
			pack->readString( _name );
			pack->readString( _address );
			pack->readBytes( _phone, sizeof ( _phone ) );

			_service_level = pack->readByte();
			pack->readString( _detail );
			_lon = pack->readInt();
			_lat = pack->readInt();

			return true;
		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _discount_num, sizeof ( _discount_num ) );
			pack->writeBytes( _business_num, sizeof ( _business_num ) );

			pack->writeString( _title );
			pack->writeBytes( _time, sizeof ( _time ) );
			pack->writeString( _name );
			pack->writeString( _address );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeByte( _service_level );
			pack->writeString( _detail );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}
	public:
		uint8_t _discount_num[20]; //�Żݱ��
		uint8_t _business_num[20]; //�ŵ���
		CQString _title; //����
		uint8_t _time[6];
		; //��Ч����(BCD)
		CQString _name; //�̻�����
		CQString _address; //�̻���ַ
		uint8_t _phone[6]; //�绰
		uint8_t _service_level; //����ȼ�
		CQString _detail; //����
		uint32_t _lon; //����
		uint32_t _lat; //γ��

	};

	class CQueryUnionBusinessInfoReq : public IPacket //0x102C ��ѯ�����̼���Ϣ
	{
	public:
		CQueryUnionBusinessInfoReq( )
		{

			_header._type = UP_UNION_BUSINESS_INFO_REQ;
		}

		~CQueryUnionBusinessInfoReq( )
		{

		}

		void Body( CPacker *pack )
		{
			pack->writeByte( _type );
			pack->writeByte( _range );
			pack->writeByte( _service_level );
			pack->writeByte( _sort_type );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
			pack->writeByte( _offset );
			pack->writeByte( _count );
		}

		bool UnBody( CPacker *pack )
		{

			_type = pack->readByte();
			_range = pack->readByte();

			_service_level = pack->readByte();
			_sort_type = pack->readByte();

			_lon = pack->readInt();
			_lat = pack->readInt();

			_offset = pack->readByte();
			_count = pack->readByte();

			return true;
		}
	public:
		uint8_t _type; //�Ż�����(1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�ޡ�0.ȫ��
		uint8_t _range; //��ѯ��Χ(0.15 1.30 2. 50)��λ:����
		uint8_t _service_level; //����ȼ�
		uint8_t _sort_type; //��������
		uint32_t _lon; //����
		uint32_t _lat; //γ��
		uint8_t _offset; //��ȡ��ʼλ�ã�Ĭ�ϴ�0��ʼ
		uint8_t _count; //��¼����
	};

	//�̼���Ϣ�б�
	class CBusiness_List : public share::Ref
	{
	public:
		CBusiness_List( )
		{
		}
		;
		~CBusiness_List( )
		{
		}
		;
	public:
		void Pack( CPacker *pack )
		{
			pack->writeBytes( _business_num, sizeof ( _business_num ) );
			pack->writeString( _name );
			pack->writeByte( _type );
			pack->writeByte( _service_level );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}

		bool UnPack( CPacker *pack )
		{
			pack->readBytes( _business_num, sizeof ( _business_num ) );
			pack->readString( _name );
			_type = pack->readByte();
			_service_level = pack->readByte();
			_lon = pack->readInt();
			_lat = pack->readInt();
			return true;
		}
	public:
		uint8_t _business_num[20]; //�ŵ���
		CQString _name; //�ŵ�����
		uint8_t _type; //�Ż�����(1.ά�ޡ�2.��Ԯ��3.���͡�4.������5.������6.ס�ޡ�0.ȫ��)
		uint8_t _service_level; //����ȼ�
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};

	class CQueryUnionBusinessInfoRsp : public IPacket //0x802C ��ѯ�����̼���Ϣ��Ӧ
	{
		typedef std::vector< CBusiness_List * > CBusiness_List_VEC;
	public:
		CQueryUnionBusinessInfoRsp( uint32_t seq = 0 )
		{
			_header._type = UP_UNION_BUSINESS_INFO_RSP;
			_header._seq = seq;
		}

		~CQueryUnionBusinessInfoRsp( )
		{
			Clear();
		}
		bool UnBody( CPacker *pack )
		{
			_num = pack->readByte();
			_action = pack->readByte();

			if ( _num == 0 ) return false;
			for ( int i = 0 ; i < ( int ) _num ; ++ i ) {
				CBusiness_List *info = new CBusiness_List;
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
			pack->writeByte( _action );

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
		uint8_t _num; //��ѯ����ĸ���;
		uint8_t _action; //����(0.��ѯ 1.����)
		CBusiness_List_VEC _vec;
	};

	class CQueryDetailUnionBusinessInfoReq : public IPacket //0x102D  ��ѯ���������̼���Ϣ
	{
	public:
		CQueryDetailUnionBusinessInfoReq( )
		{

			_header._type = UP_DETAIL_UNION_BUSINESS_INFO_REQ;
		}

		~CQueryDetailUnionBusinessInfoReq( )
		{

		}
		void Body( CPacker *pack )
		{
			pack->writeBytes( _business_num, sizeof ( _business_num ) );
		}

		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _business_num, sizeof ( _business_num ) );
			return true;
		}
	public:
		uint8_t _business_num[20]; //�̼ұ��
	};

	class CQueryDetailUnionBusinessInfoRsp : public IPacket //0x802D ��ѯ���������̼���Ϣ��Ӧ
	{
	public:
		CQueryDetailUnionBusinessInfoRsp( uint32_t seq = 0 )
		{
			_header._type = UP_DETAIL_UNION_BUSINESS_INFO_RSP;
			_header._seq = seq;

			memset( & _phone, 0, sizeof ( _phone ) );
		}
		~CQueryDetailUnionBusinessInfoRsp( )
		{

		}

		void Body( CPacker *pack )
		{
			pack->writeBytes( _business_num, sizeof ( _business_num ) );
			pack->writeString( _name );
			pack->writeByte( _service_level );
			pack->writeString( _address );
			pack->writeBytes( _phone, sizeof ( _phone ) );
			pack->writeString( _detail );
			pack->writeInt( _lon );
			pack->writeInt( _lat );
		}

		bool UnBody( CPacker *pack )
		{
			pack->readBytes( _business_num, sizeof ( _business_num ) );
			pack->readString( _name );
			_service_level = pack->readByte();
			pack->readString( _address );
			pack->readBytes( _phone, sizeof ( _phone ) );
			pack->readString( _detail );
			_lon = pack->readInt();
			_lat = pack->readInt();
			return true;
		}
	public:
		uint8_t _business_num[20]; //�̼ұ��
		CQString _name; //�ŵ�����
		uint8_t _service_level; //����ȼ�
		CQString _address; //��ַ
		uint8_t _phone[6]; //�绰
		CQString _detail; //����
		uint32_t _lon; //����
		uint32_t _lat; //γ��
	};
	//�·�Ŀ�ĵ�
	class  CSend_DestinationInfoReq:public IPacket //0x100D �·�Ŀ�ĵ�
	{
	public:
			CSend_DestinationInfoReq(){
				_header._type = SEND_DESTINATION_INFO_REQ;
			}
			~CSend_DestinationInfoReq(){}

			void Body( CPacker *pack )
			{
				pack->writeBytes(_destName,sizeof(_destName));
				pack->writeString(_address);
				pack->writeBytes(_phone,sizeof(_phone));
				pack->writeInt(_lon);
				pack->writeInt(_lat);
				pack->writeByte(_state);
			}

			bool UnBody( CPacker *pack )
			{
				pack->readBytes(_destName,sizeof(_destName));
				pack->readString(_address);
				pack->readBytes(_phone,sizeof(_phone));
				_lon = pack->readInt();
				_lat = pack->readInt();
				_state = pack->readByte();

				return true;
			}
	public:
			uint8_t   _destName[20];//Ŀ�ĵ�����
			CQString  _address;//��ϸ��ַ
			uint8_t   _phone[20];//��ϵ�绰
			uint32_t  _lon;//����
			uint32_t  _lat;//γ��
			uint8_t   _state;//�Ƿ�ɹ���0���ɹ���1ʧ�ܣ�
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

	class CCarServiceUnPackMgr : public IUnPackMgr
	{
	public:
		  CCarServiceUnPackMgr(){}
		 ~CCarServiceUnPackMgr(){}

		// ʵ�����ݽ���ӿڷ���
		IPacket * UnPack( unsigned short msgtype, CPacker &pack )
		{
			IPacket *msg = NULL ;
			switch( msgtype )
				{
				case UP_DISCOUNT_INFO_REQ://��ѯ�����Ż���Ϣ
					msg = UnPacket<CQueryDiscountInfoReq>(pack,"CQueryDiscountInfoReq");
					break;
				case UP_DETAIL_DISCOUNT_INFO_REQ: //��ѯ��������Ż���Ϣ
					msg = UnPacket<CQueryDetailiscountInfoReq>(pack,"CQueryDetailiscountInfoReq");
					break;
				case UP_UNION_BUSINESS_INFO_REQ: //��ѯ�����̼���Ϣ
					msg = UnPacket<CQueryUnionBusinessInfoReq>(pack,"CQueryUnionBusinessInfoReq");
					break;
				case UP_DETAIL_UNION_BUSINESS_INFO_REQ: //��ѯ���˾����̼���Ϣ
					msg = UnPacket<CQueryDetailUnionBusinessInfoReq>(pack,"CQueryDetailUnionBusinessInfoReq");
					break;
				case UP_LOGIN_INFO_REQ://0x1001 �û���¼
					msg = UnPacket<CLoginInfoReq>(pack,"CLoginInfoReq");
					break;
				case UP_QUERY_BALLANCE_LIST_REQ://0x1002 ����������ѯ
					msg = UnPacket<CQuery_Ballance_List_Req>(pack,"CQuery_Ballance_List_Req");
					break;
				case UP_QUERY_STORE_LIST_REQ://0x1003 ��ѯ�ŵ�
					msg = UnPacket<CQuery_Store_List_Req>(pack,"CQuery_Store_List_Req");
					break;
				case  UP_QUERY_VIEW_STORE_INFO_REQ://0x1004 ��ѯ�ŵ�����
					msg = UnPacket<CView_Store_Info_Req>(pack,"CView_Store_Info_Req");
					break;
				case  UP_QUERY_DISCOUNT_LIST_REQ://0x1005 �°汾��ѯ�Ż���Ϣ
					msg = UnPacket<CQuery_Discount_List_Req>(pack,"CQuery_Discount_List_Req");
					break;
				case  UP_VIEW_DISCOUNT_INFO_REQ://0x1006  �°汾��ѯ�Ż���Ϣ��ϸ�б�
					msg = UnPacket<CView_Discount_Info_Req>(pack,"CView_Discount_Info_Req");
					break;
				case UP_QUERY_TRADE_LIST_REQ://0x1007 ��ʷ���׼�¼��ѯ
					msg = UnPacket<CQuery_Trade_List_Req>(pack,"CQuery_Trade_List_Req");
					break;
				case UP_QUERY_FAVORITE_LIST_REQ://0x1008 ��ѯ�ղ��б�
					msg = UnPacket<CQuery_Favorite_List_Req>(pack,"CQuery_Favorite_List_Req");
					break;
				case UP_VIEW_FAVORITE_INFO_REQ://0x1009 ��ѯ�ղ��б�����
					msg = UnPacket<CView_Favorite_Info_Req>(pack,"CView_Favorite_Info_Req");
					break;
				case UP_ADD_FAVORITE_REQ://0x100A ����ղ�����
					msg = UnPacket<CAdd_Favorite_Req>(pack,"CAdd_Favorite_Req");
					break;
				case UP_DEL_FAVORITE_REQ ://0x100B ɾ���ղ�����
					msg = UnPacket<CDel_Favorite_Req>(pack,"CDel_Favorite_Req");
					break;
				case UP_GET_DESTINATION_REQ://0x100C ��ȡĿ�ĵ�
					msg = UnPacket<CGet_Destination_Req>(pack,"CGet_Destination_Req");
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
}

#pragma pack()

#endif
