/*
 * truck.cpp
 *
 *  Created on: 2012-5-31
 *      Author: humingqing
 */

#include <stdio.h>
#include <truck.h>
#include <comlog.h>
#include "packfactory.h"

namespace CarService {

	CCarService::CCarService(): _pCaller(NULL)
	{
		_packfactory = new CPackFactory( &_unpacker ) ;
		_srvCaller   = new CSrvCaller ;
	}

	CCarService::~CCarService()
	{
		Stop() ;

		if ( _srvCaller != NULL ){
			delete _srvCaller ;
			_srvCaller = NULL ;
		}
	}

	// ��Ҫ��ʼ������
	bool CCarService::Init( IPlugin *plug , const char *url, int sendthread , int recvthread , int queuesize )
	{
		_pCaller = plug ;

		if ( ! _srvCaller->Init( plug , url, sendthread, recvthread , queuesize ) ) {
			OUT_ERROR( NULL, 0, "CCarService", "init service caller http failed" ) ;
			return false ;
		}
		return true ;
	}

	// ��ʼ�����ͨ��
	bool CCarService::Start( void )
	{
		if ( ! _srvCaller->Start() ){
			OUT_ERROR( NULL, 0, "CCarService", "start service caller http failed" ) ;
			return false ;
		}
		return true ;
	}

	// ֹͣ���ͨ��
	bool CCarService::Stop( void )
	{
		_srvCaller->Stop() ;
		return true ;
	}

	// ����͸��������
	bool CCarService::Process( unsigned int fd, const char *data, int len , unsigned int cmd , const char *id )
	{
		// printf( "recv data length: %d\n" , len ) ;
		IPacket *msg = _packfactory->UnPack( data, len ) ;
		// �����Ϣͷ��
		if ( msg == NULL) {
			OUT_ERROR( NULL, 0, id , "Truck process data length error, len %d" , len ) ;
			return false ;
		}

		// �Զ��ͷŶ���
		CAutoRelease autoRef(msg);

		// ������������
		unsigned short msg_type = msg->_header._type;
		// ����������
		OUT_RECV( NULL, 0, id, "Truck process recv fd %d, msg type %4x" , fd, msg_type ) ;
		// ������Ӧ��Э��
		switch( msg_type )
		{
		// ����ҵ��
		case UP_DISCOUNT_INFO_REQ:// 0x1029 ��ѯ�����Ż���Ϣ
			_srvCaller->getUpQueryDiscountInfoReq( fd, cmd, (CQueryDiscountInfoReq*) msg , id);
			break;
		case UP_DETAIL_DISCOUNT_INFO_REQ: //0x102A ��ѯ��������Ż���Ϣ
			_srvCaller->getUpQueryDetailDiscountInfoReq( fd, cmd, (CQueryDetailiscountInfoReq*) msg , id);
			break;
		case UP_UNION_BUSINESS_INFO_REQ: //0x102C ��ѯ�����̼���Ϣ
			_srvCaller->getUpQueryUnionBusinessInfoReq( fd, cmd, (CQueryUnionBusinessInfoReq*) msg , id);
			break;
		case UP_DETAIL_UNION_BUSINESS_INFO_REQ: //0x102D ��ѯ���˾����̼���Ϣ
			_srvCaller->getUpQueryDetailUnionBusinessInfoReq( fd, cmd, (CQueryDetailUnionBusinessInfoReq*) msg , id);
			break;
		case TERMINAL_COMMON_RSP://�ն�ͨ��Ӧ��
			_srvCaller->putTerminalCommonRsp( fd, cmd, (CTerminalCommonRsp*) msg , id);
			break;
		case UP_LOGIN_INFO_REQ://0x1001 �û���¼
			_srvCaller->getUpLoginInfoReq( fd, cmd, (CLoginInfoReq*) msg , id);
			break;
		case UP_QUERY_BALLANCE_LIST_REQ://0x1002 ����������ѯ
			_srvCaller->getUpQueryBallanceListReq( fd, cmd,(CQuery_Ballance_List_Req*)msg,id);
			break;
		case UP_QUERY_STORE_LIST_REQ://0x1003 ��ѯ�ŵ�
			_srvCaller->getUpQueryStoreListReq( fd, cmd,(CQuery_Store_List_Req*)msg,id);
			break;
		case UP_QUERY_VIEW_STORE_INFO_REQ://0x1004  ��ѯ�ŵ�����
			_srvCaller->getUpQueryViewStoreInfoReq( fd, cmd,(CView_Store_Info_Req*)msg,id);
			 break;
		case UP_QUERY_DISCOUNT_LIST_REQ://0x1005  �°汾��ѯ�Ż���Ϣ
			_srvCaller->getUpQueryDiscountListReq( fd, cmd,(CQuery_Discount_List_Req*)msg,id);
		   break;
		case UP_VIEW_DISCOUNT_INFO_REQ://0x1006  �°汾��ѯ�Ż���Ϣ��ϸ�б�
			_srvCaller->getUpViewDiscountInfoReq( fd, cmd,(CView_Discount_Info_Req*)msg,id);
		   break;
		case UP_QUERY_TRADE_LIST_REQ://0x1007 ��ʷ���׼�¼��ѯ
			_srvCaller->getUpQueryTradeListReq( fd, cmd,(CQuery_Trade_List_Req*)msg,id);
			break;
		case UP_QUERY_FAVORITE_LIST_REQ://0x1008 ��ѯ�ղ��б�
			_srvCaller->getUpQueryFavoriteListReq(fd, cmd,(CQuery_Favorite_List_Req*)msg,id);
			break;
		case UP_VIEW_FAVORITE_INFO_REQ://0x1009 ��ѯ�ղ��б�����
			_srvCaller->getUpViewFavoriteInfoReq(fd, cmd,(CView_Favorite_Info_Req*)msg,id);
			break;
		case UP_ADD_FAVORITE_REQ://0x100A ����ղ�����
			_srvCaller->getUpAddFavoriteReq(fd, cmd,(CAdd_Favorite_Req*)msg,id);
			break;
		case UP_DEL_FAVORITE_REQ ://0x100B ɾ���ղ�����
			_srvCaller->getUpDelFavoriteReq(fd, cmd,(CDel_Favorite_Req*)msg,id);
			break;
		case UP_GET_DESTINATION_REQ://0x100C ��ȡĿ�ĵ�
			_srvCaller->getUpGetDestinationReq(fd, cmd,(CGet_Destination_Req*)msg,id);
			break;
		default:
			break;
		}

		// �ͷ�����
		return true;
	}
} ;
//====================================== ��̬�⶯̬���غ��� ================================================
extern "C" IPlugWay* GetPlugObject( void )
{
	return new CarService::CCarService ;
}
//----------------------------------------------------------------------------------
extern "C" void FreePlugObject( IPlugWay* p )
{
	if ( p != NULL )
		delete p ;
}
//----------------------------------------------------------------------------------
