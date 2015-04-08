/*
 * servicecaller.h
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */

#ifndef __SERVICECALLER_H_
#define __SERVICECALLER_H_

#include <map>
#include <iplugin.h>
#include <vector>
#include <string>
#include <Mutex.h>
#include "httpcaller.h"
using namespace std ;

#include "truckpack.h"
#include "msgqueue.h"
#include "resultmgr.h"

#include <Thread.h>

namespace TruckSrv{
	// ҵ�����ģ�鸺��ʵ��HTTP����
	class CSrvCaller:
		public ICallResponse , public IMsgCaller, public share::Runnable
	{
		// ����KEY VALUE
		struct _KeyValue{
			string key ;
			string val ;
		};

		// KEY VALUE����
		class CKeyValue
		{
		public:
			CKeyValue() {}
			~CKeyValue(){}
			// ȡ��Ԫ�صĸ���
			int GetSize() { return (int)_vec.size(); }
			// ȡ��KEY VALUE��ֵ
			_KeyValue &GetVal(int index) { return _vec[index]; }
			// ����ֵ����
			void SetVal( const char *key, const char *val ) {
				// ����NULL���������coredump
				if ( key == NULL || val == NULL )
					return ;
				_KeyValue vk ;
				vk.key = key ;
				vk.val = val ;
				_vec.push_back( vk ) ;
			}
		private:
			// ������ݵ�����
			vector<_KeyValue> _vec ;
		};

		// ������ϢID��Ӧ����IDӳ��
		class CSeq2Msg
		{
			// ǰһ��Ϊ����ID����Ӧ����ϢID
			typedef map<unsigned int , unsigned int>  MapSeq2Msg;
		public:
			CSeq2Msg() {};
			~CSeq2Msg(){};

			// �����Ŷ�Ӧ��ϵ
			void AddSeqMsg( unsigned int seq, unsigned int msgid ) {
				share::Guard guard( _mutex ) ;
				_seq2msgid[seq] = msgid ;
			}

			// ȡ����Ŷ�Ӧ��Ϣӳ���ϵ
			bool GetSeqMsg( unsigned int seq , unsigned int &msgid ) {
				share::Guard guard( _mutex ) ;

				MapSeq2Msg::iterator it = _seq2msgid.find( seq ) ;
				if ( it == _seq2msgid.end() ) {
					return false ;
				}
				msgid = it->second ;
				_seq2msgid.erase( it ) ;

				return true ;
			}

			// �Ƴ�ӳ��
			void RemoveSeq( unsigned int seq ) {
				share::Guard guard( _mutex ) ;

				MapSeq2Msg::iterator it = _seq2msgid.find( seq ) ;
				if ( it == _seq2msgid.end() ) {
					return;
				}
				_seq2msgid.erase( it ) ;
			}

		private:
			// �������кŶ�Ӧ��Ϣ��������
			MapSeq2Msg				_seq2msgid ;
			// �������Ĳ���
			share::Mutex			_mutex ;
		};

		typedef bool (CSrvCaller::*ServiceFun)( unsigned int seq, const char *xml ) ;
		typedef map<unsigned int , ServiceFun>  ServiceTable;

	public:
		CSrvCaller() ;
		~CSrvCaller() ;
		// ��ʼ��
		bool Init( IPlugin *pEnv , const char *url, int sendthread , int recvthread , int queuesize ) ;
		// ����
		bool Start( void ) ;
		// ֹͣ
		void Stop( void );
		// ������Ϣ��ѯ
		bool getQueryInfoReq(unsigned int fd, unsigned int cmd, CQueryInfoReq *msg, const char *id);
		// ��������
		bool getSendTeamMediaReq(unsigned int fd, unsigned int cmd, CSendTeamMediaReq *msg, const char *id);
		// ��������
		bool getSendMediaDataReq(unsigned int fd, unsigned int cmd, CSendMediaDataReq *msg, const char *id);
		// ����ͷ������Ϣ
		bool getInfoPriMcarReq(unsigned int fd, unsigned int cmd, CInfoPriMcarReq  *msg, const char *id);
		//���ñ���Ϊͷ��
		bool getSetPriMcarReq(unsigned int fd, unsigned int cmd, CSetPriMcarReq *msg, const char *id);
		// ���ӳ�Ա����
		bool getInviteNumberReq(unsigned int fd, unsigned int cmd, CInviteNumberReq *msg, const char *id);
		// ��������
		bool getAddCarTeamReq(unsigned int fd, unsigned int cmd, CAddCarTeamReq *msg, const char *id);
		// ��ȡ�����б�
		bool getGetFriendlistReq(unsigned int fd, unsigned int cmd, CGetFriendListReq *msg, const char *id);
		// ���복��
		bool getInviteFriendReq(unsigned int fd, unsigned int cmd, CInviteFriendReq *msg, const char *id);
		// ��Ӻ���
		bool getAddFriendsReq(unsigned int fd, unsigned int cmd, CAddFriendsReq *msg, const char *id);
		// ���Ҹ�������
		bool getQueryFriendsReq(unsigned int fd, unsigned int cmd, CQueryFriendsReq *msg, const char *id);
		// ˾�����ע��
		bool getDriverLoginOutReq(unsigned int fd, unsigned int cmd, CDriverLoginOutReq *msg, const char *id);
		// ˾��ע���¼
		bool getDriverLoginReq(unsigned int fd, unsigned int cmd, CDriverLoginReq *msg, const char *id);
		// ȡ��ע����Ϣ������ֻ�����Ȩʱ����
		bool getQueryCarDataReq( unsigned int fd, unsigned int cmd , CQueryCarDataReq *msg , const char *id ) ;
		// �����·����ȵ���Ӧ�������
		bool getResultScheduleReq( unsigned int fd, unsigned int cmd, CResultScheduleReq *msg , const char *id ) ;
		// �����·����ȵ��Զ�Ӧ��
		bool putSendScheduleRsp( unsigned int fd, unsigned int cmd, CSendScheduleRsp *msg , const char *id );
		//�ն�ͨ��Ӧ��
		bool putTerminalCommonRsp(unsigned int fd, unsigned int cmd, CTerminalCommonRsp *msg , const char *id);
		// ��ѯ��ǰ����ĵ��ȵ�
		bool getQueryScheduleReq( unsigned int fd, unsigned int cmd, CQueryScheduleReq *msg , const char *id ) ;
		// �ϴ����ȵ�����Ϣ
		bool putUploadScheduleReq( unsigned int fd, unsigned int cmd, CUploadScheduleReq *msg , const char *id ) ;
		// �ϱ���������״̬
		bool putStateScheduleReq( unsigned int fd, unsigned int cmd, CStateScheduleReq *msg , const char *id ) ;
		// �ϱ��澯״̬
		bool putAlarmScheduleReq( unsigned int fd, unsigned int cmd, CAlarmScheduleReq *msg , const char *id ) ;
		// ��������
		bool putScheduleReq(unsigned int fd, unsigned int cmd, CSubscrbeReq *msg, const char *id);
		// �ϴ���������
		bool putErrorScheduleReq(unsigned int fd, unsigned int cmd,CErrorScheduleReq *msg , const char *id );
		// �Զ��ϱ������Ϣ
		bool putAutoDataScheduleReq(unsigned int fd, unsigned int cmd,CAutoDataScheduleReq *msg , const char *id );
		// �ն�͸��
		bool getMsgDataScheduleReq(unsigned int fd, unsigned int cmd,CUpMsgDataScheduleReq *msg , const char *id );
		// �ϴ������Ϣ
		bool getCarDataInfoReq(unsigned int fd, unsigned int cmd, CUpCarDataInfoReq *msg, const char *id);
		//�ϴ��˵���
		bool getUpTransportFormInfoReq(unsigned int fd, unsigned int cmd, CTransportFormInfoReq *msg, const char *id);
		// ������ϸ��ѯ
		bool getQueryOrderFormInfoReq(unsigned int fd, unsigned int cmd, CQueryOrderFromInfoReq *msg, const char *id);
		// �ϴ����˶�����
		bool getUpOrderFormInfoReq(unsigned int fd, unsigned int cmd, CUpOrderFromInfoReq *msg, const char *id);
		// ����HttpCaller�ص�
		void ProcessResp( unsigned int seqid, const char *xml, const int len , const int err ) ;
		// ��������ʱ������
		void OnTimeOut( unsigned int seq, unsigned int fd, unsigned int cmd, const char *id, IPacket *msg )  ;
		// ��ⳬʱ����
		void CheckTimeOut( int timeout ) { _resultmgr.Check(timeout) ;  }

	public:
		// �߳�ִ�ж���
		void run( void *param ) ;
	private:
		// ���Ϊһ��ͳһ���÷�ʽ����
		bool ProcessMsg( unsigned int msgid, unsigned int seq , const char *service, const char *method , CKeyValue &item ) ;
		// ��������XML����
		bool CreateRequestXml( CQString &sXml, const char *id, const char *service, const char *method , CKeyValue &item ) ;

	protected:
		//�ն�ͨ��Ӧ���Զ���Ӧ
		bool Proc_TERMINAL_COMMON_RSP( unsigned int seqid, const char *xml );
		//�ϴ��˵�״̬
		bool Proc_TRANSPORT_ORDER_FROM_INFO_REQ(unsigned int seqid, const char *xml);
		//�ϴ����˶���״̬
		bool Proc_ORDER_FROM_INFO_REQ(unsigned int seqid, const char *xml);
		//�ϴ������Ϣ
		bool Proc_CARDATA_INFO_REQ(unsigned int seqid, const char *xml);
		// ������ϸ��ѯ
		bool Proc_QUERY_ORDER_FORM_INFO_REQ(unsigned int seqid, const char *xml);
		// ������Ϣ��ѯ
		bool Proc_QUERY_INFO_REQ( unsigned int seqid, const char *xml);
		// ��������
		bool Proc_SEND_TEAMMEDIA_REQ(unsigned int seqid, const char *xml);
		// ��������
		bool Proc_SEND_MEDIADATA_REQ(unsigned int seqid, const char *xml);
		// ����ͷ������Ϣ
		bool Proc_INFO_PRIMCAR_REQ(unsigned int seqid, const char *xml);
		// ���ñ���Ϊͷ��
		bool Proc_SET_PRIMCAR_REQ(unsigned int seqid, const char *xml);
		// ���ӳ�Ա����
		bool Proc_INVITE_NUMBER_REQ(unsigned int seqid, const char *xml);
		// ��������
		bool Proc_ADD_CARTEAM_REQ(unsigned int seqid, const char *xml);
		// ��ȡ�����б�
		bool Proc_GET_FRIENDLIST_REQ(unsigned int seqid, const char *xml);
		//�������
		bool Proc_INVITE_FRIEND_REQ(unsigned int seqid, const char *xml);
		// ��Ӻ���
		bool Proc_ADD_FRIENDS_REQ(unsigned int seqid, const char *xml);
		// ���Ҹ�������
		bool Proc_QUERY_FRIENDS_REQ(unsigned int seqid, const char *xml);
		// ˾�����ע��
		bool Proc_DRIVER_LOGINOUT_REQ(unsigned int seqid, const char *xml);
		// ˾����ݵ�½��֤
		bool Proc_DRIVER_LOGIN_REQ(unsigned int seqid, const char *xml);
		// �����Ӧ��XML������
		bool Proc_QUERY_CARDATA_REQ( unsigned int seqid, const char *xml ) ;
		// ���������ϱ�
		bool Proc_UPLOAD_DATAINFO_REQ( unsigned int seqid, const char *xml ) ;
		// �����·����ȵ���Ӧ
		bool Proc_RESULT_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �·����ȵ��Զ�Ӧ���ϱ�
		bool Proc_SEND_SCHEDULE_RSP( unsigned int seqid, const char *xml ) ;
		// ��ѯ���ȵ�
		bool Proc_QUERY_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �ϴ����ȵ�
		bool Proc_UPLOAD_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �ϱ����ȵ�״̬
		bool Proc_STATE_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �ϱ�����״̬
		bool Proc_ALARM_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// ������Ӧ
		bool Proc_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �ϴ�����������Ӧ
		bool Proc_ERROR_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �Զ��ϱ������Ϣ��Ӧ
		bool Proc_AUTO_DATA_SCHEDULE_REQ( unsigned int seqid, const char *xml ) ;
		// �ն�͸��������Ӧ
		bool Proc_MSG_DATA_SCHEDULE_REQ( unsigned int seqid, const char *xml );
		// ��������
		void DeliverPacket( unsigned int fd, unsigned int cmd, IPacket *msg);
	private:
		// �����������
		void ProcessError( unsigned int seq , bool remove);
	private:
		// ����������
		IPlugin			   		*_pEnv ;
		// ����XML���õ�HTTP����
		CHttpCaller				_httpcaller ;
		// ����
		ServiceTable			_srv_table ;
		// ���������Ϣ��Ӧ��ϵ
		CSeq2Msg				_seq2msgid;
		// ���÷���URL��ַ
		CQString 				_callUrl;
		// ���ݵȴ�����
		CMsgQueue				_msgqueue;
		// ������������
		CResultMgr			    _resultmgr;
		// �߳�ִ�ж���
		share::ThreadManager 	_thread ;
		// �źŵȴ�����
		share::Monitor		 	_monitor;
		// �Ƿ��ʼ��
		bool 				 	_inited ;
		// ���ݽ������
		CTruckUnPackMgr 		_unpacker ;
		// �������
		CPackFactory *			_packfactory;
	};
};

#endif /* SERVICECALLER_H_ */
