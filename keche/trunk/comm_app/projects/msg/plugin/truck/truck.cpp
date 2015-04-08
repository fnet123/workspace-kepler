/*
 * truck.cpp
 *
 *  Created on: 2012-5-31
 *      Author: humingqing
 */

#include <stdio.h>
#include <truck.h>
#include <comlog.h>

namespace TruckSrv{
	// ʹ��
	CTruck::CTruck(): _pCaller(NULL)
	{
		_packfactory = new CPackFactory( &_unpacker ) ;
		_srvCaller = new CSrvCaller ;
	}

	CTruck::~CTruck()
	{
		Stop() ;

		if ( _srvCaller != NULL ){
			delete _srvCaller ;
			_srvCaller = NULL ;
		}
		if ( _packfactory != NULL ) {
			delete _packfactory ;
			_packfactory = NULL ;
		}
	}

	// ��Ҫ��ʼ������
	bool CTruck::Init( IPlugin *plug , const char *url, int sendthread , int recvthread , int queuesize )
	{
		_pCaller = plug ;

		if ( ! _srvCaller->Init( plug , url, sendthread, recvthread , queuesize ) ) {
			OUT_ERROR( NULL, 0, "CTruck", "init service caller http failed" ) ;
			return false ;
		}
		return true ;
	}

	// ��ʼ�����ͨ��
	bool CTruck::Start( void )
	{
		if ( ! _srvCaller->Start() ){
			OUT_ERROR( NULL, 0, "CTruck", "start service caller http failed" ) ;
			return false ;
		}
		return true ;
	}

	// ֹͣ���ͨ��
	bool CTruck::Stop( void )
	{
		_srvCaller->Stop() ;
		return true ;
	}

	// ����͸��������
	bool CTruck::Process( unsigned int fd, const char *data, int len , unsigned int cmd , const char *id )
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
		case SEND_TEAMMEDIA_REQ:// ��������
			_srvCaller->getSendTeamMediaReq(fd, cmd, (CSendTeamMediaReq *)msg, id);
			 break;
		case SEND_MEDIADATA_REQ://��������
			_srvCaller->getSendMediaDataReq(fd, cmd, (CSendMediaDataReq *)msg, id);
			 break;
		case INFO_PRIMCAR_REQ://����ͷ������Ϣ
			_srvCaller->getInfoPriMcarReq(fd, cmd, (CInfoPriMcarReq *)msg, id);
			 break;
		case SET_PRIMCAR_REQ://���ñ���Ϊͷ��
			_srvCaller->getSetPriMcarReq(fd, cmd, (CSetPriMcarReq *)msg, id);
			 break;
		case INVITE_NUMBER_REQ://���ӳ�Ա����
			_srvCaller->getInviteNumberReq(fd, cmd, (CInviteNumberReq *)msg, id);
			break;
		case ADD_CARTEAM_REQ: //��������
			_srvCaller->getAddCarTeamReq(fd, cmd, (CAddCarTeamReq *)msg, id);
			 break;
		case GET_FRIENDLIST_REQ://��ȡ�����б�
			 _srvCaller->getGetFriendlistReq(fd, cmd, (CGetFriendListReq *)msg, id);
			 break;
		case INVITE_FRIEND_REQ://���복��
			_srvCaller->getInviteFriendReq(fd, cmd, (CInviteFriendReq *)msg, id);
			break;
		case ADD_FRIEND_REQ://��Ӻ���
			_srvCaller->getAddFriendsReq(fd, cmd, (CAddFriendsReq *)msg, id);
			break;
		case QUERY_FRIENDS_REQ://���Ҹ����ĺ���
			_srvCaller->getQueryFriendsReq(fd, cmd, (CQueryFriendsReq *) msg , id ) ;
			break;
		case DRIVER_LOGOUT_REQ://˾��ע��
			_srvCaller->getDriverLoginOutReq(fd, cmd, (CDriverLoginOutReq *) msg , id ) ;
			break;
		case DRIVER_LOGIN_REQ://˾����¼��֤
			_srvCaller->getDriverLoginReq(fd, cmd, ( CDriverLoginReq *) msg , id ) ;
			break;
		case QUERY_CARDATA_REQ:// ��ѯ�����Ϣ
			_srvCaller->getQueryCarDataReq(fd, cmd, ( CQueryCarDataReq *) msg , id ) ;
			break;
		case UPLOAD_DATAINFO_REQ:// ���״̬�ϱ�
			_srvCaller->putAutoDataScheduleReq(fd,cmd,(CAutoDataScheduleReq *)msg , id );
			break;
		case SUBSCRIBE_REQ: //���Ĺ���
			_srvCaller->putScheduleReq(fd , cmd , (CSubscrbeReq*)msg , id ) ;
			break;
		case QUERY_INFO_REQ://������Ϣ��ѯ
			_srvCaller->getQueryInfoReq(fd, cmd, (CQueryInfoReq *) msg , id);
			break;
		case UP_REPORTERROR_REQ:  	// �ϴ���������
			_srvCaller->putErrorScheduleReq(fd,cmd,(CErrorScheduleReq*)msg , id );
			break;
			// ���沿��Ϊ˦��ҵ��Ĵ���
		case SEND_SCHEDULE_RSP:  // �����·����ȵ��Զ�Ӧ����Ӧ
			_srvCaller->putSendScheduleRsp( fd, cmd, (CSendScheduleRsp*) msg, id ) ;
			break ;
		case RESULT_SCHEDULE_REQ:	 	// 0x1045  �·����ȵ���Ӧ���
			_srvCaller->getResultScheduleReq( fd , cmd , (CResultScheduleReq*) msg , id ) ;
			break ;
		case QUERY_SCHEDULE_REQ:	// 0x1041  ��ѯ���ȵ�����
			_srvCaller->getQueryScheduleReq( fd, cmd, (CQueryScheduleReq*) msg , id ) ;
			break ;
		case UPLOAD_SCHEDULE_REQ: 	// 0x1042 �ϴ����ȵ�
			_srvCaller->putUploadScheduleReq( fd, cmd, (CUploadScheduleReq*) msg , id ) ;
			break ;
		case STATE_SCHEDULE_REQ: 	// 0x1043  �ϱ����ȵ�״̬
			_srvCaller->putStateScheduleReq( fd, cmd, (CStateScheduleReq *) msg , id ) ;
			break ;
		case ALARM_SCHEDULE_REQ: 	// 0x1044  ���Ҹ澯
			_srvCaller->putAlarmScheduleReq( fd, cmd, (CAlarmScheduleReq *) msg , id ) ;
			break ;
		case UP_MSGDATA_REQ:        // 0x1060  ����͸��
			_srvCaller->getMsgDataScheduleReq( fd, cmd, (CUpMsgDataScheduleReq *) msg , id );
			break;
		//����ҵ��
		case UP_CARDATA_INFO_REQ:   //0x1023 �ϴ������Ϣ
			_srvCaller->getCarDataInfoReq( fd, cmd, (CUpCarDataInfoReq *) msg , id );
			break;
		case UP_QUERY_ORDER_FORM_INFO_REQ: //0x1026  ������ϸ��ѯ
			_srvCaller->getQueryOrderFormInfoReq(fd, cmd, (CQueryOrderFromInfoReq*) msg , id);
			break;
		case UP_ORDER_FORM_INFO_REQ: //0x1027 �ϴ����˶���״̬
			_srvCaller->getUpOrderFormInfoReq( fd, cmd, (CUpOrderFromInfoReq*)msg, id);
			break;
		case UP_TRANSPORT_FORM_INFO_REQ:// 0x1028 �ϴ��˵�״̬
			_srvCaller->getUpTransportFormInfoReq( fd, cmd, (CTransportFormInfoReq*)msg,id);
			break;
		case TERMINAL_COMMON_RSP://�ն�ͨ��Ӧ��
			_srvCaller->putTerminalCommonRsp( fd, cmd, (CTerminalCommonRsp*) msg , id);
			break;
		default:
			break ;
		}

		// �ͷ�����
		return true;
	}
} ;
//====================================== ��̬�⶯̬���غ��� ================================================
extern "C" IPlugWay* GetPlugObject( void )
{
	return new TruckSrv::CTruck ;
}
//----------------------------------------------------------------------------------
extern "C" void FreePlugObject( IPlugWay* p )
{
	if ( p != NULL )
		delete p ;
}
//----------------------------------------------------------------------------------



