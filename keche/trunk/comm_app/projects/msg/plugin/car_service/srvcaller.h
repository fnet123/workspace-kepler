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

using namespace std;

#include "carservicepack.h"
#include "msgqueue.h"
#include "resultmgr.h"

#include <Thread.h>

namespace CarService{
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
		//�ն�ͨ��Ӧ��
		bool putTerminalCommonRsp(unsigned int fd, unsigned int cmd, CTerminalCommonRsp *msg , const char *id);
		// ��ѯ�����Ż���Ϣ
		bool getUpQueryDiscountInfoReq(unsigned int fd, unsigned int cmd, CQueryDiscountInfoReq *msg, const char *id);
		// ��ѯ��������Ż���Ϣ
		bool getUpQueryDetailDiscountInfoReq(unsigned int fd, unsigned int cmd, CQueryDetailiscountInfoReq *msg, const char *id);
		// ��ѯ�����̼���Ϣ
		bool getUpQueryUnionBusinessInfoReq(unsigned int fd, unsigned int cmd, CQueryUnionBusinessInfoReq *msg, const char *id);
		// ��ѯ���������̼���Ϣ
		bool getUpQueryDetailUnionBusinessInfoReq(unsigned int fd, unsigned int cmd, CQueryDetailUnionBusinessInfoReq *msg, const char *id);
		//�û���¼
		bool getUpLoginInfoReq( unsigned int fd, unsigned int cmd,CLoginInfoReq *msg, const char *id );
		//����������ѯ
		bool getUpQueryBallanceListReq( unsigned int fd, unsigned int cmd,CQuery_Ballance_List_Req *msg, const char *id );
		//��ѯ�ŵ�
		bool getUpQueryStoreListReq(unsigned int fd, unsigned int cmd,CQuery_Store_List_Req *msg, const char *id);
		//��ѯ�ŵ�����
		bool getUpQueryViewStoreInfoReq(unsigned int fd, unsigned int cmd,CView_Store_Info_Req *msg, const char *id);
		//�°汾��ѯ�Ż���Ϣ
		bool getUpQueryDiscountListReq(unsigned int fd, unsigned int cmd,CQuery_Discount_List_Req *msg, const char *id);
		//�°汾��ѯ�Ż���Ϣ��ϸ�б�
		bool getUpViewDiscountInfoReq( unsigned int fd, unsigned int cmd,CView_Discount_Info_Req *msg, const char *id);
		//��ʷ���׼�¼��ѯ
		bool getUpQueryTradeListReq(unsigned int fd, unsigned int cmd,CQuery_Trade_List_Req *msg, const char *id);
		//��ѯ�ղ��б�
		bool getUpQueryFavoriteListReq(unsigned int fd, unsigned int cmd,CQuery_Favorite_List_Req *msg, const char *id);
		//��ѯ�ղ��б�����
		bool getUpViewFavoriteInfoReq(unsigned int fd, unsigned int cmd,CView_Favorite_Info_Req *msg, const char *id);
		//����ղ�����
		bool getUpAddFavoriteReq(unsigned int fd, unsigned int cmd,CAdd_Favorite_Req *msg, const char *id);
		//ɾ���ղ�����
		bool getUpDelFavoriteReq(unsigned int fd, unsigned int cmd,CDel_Favorite_Req *msg, const char *id);
		//��ȡĿ�ĵ�
		bool getUpGetDestinationReq(unsigned int fd, unsigned int cmd,CGet_Destination_Req *msg, const char *id);
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
		bool ProcessMsg( unsigned int msgid, unsigned int seq , const char *service, const char *method , CKeyValue &item);
		// ��������XML����
		bool CreateRequestXml( CQString &sXml, const char *id, const char *service, const char *method , CKeyValue &item);
	protected:
		//��ѯ�����Ż���Ϣ
		bool Proc_UPDISCOUNT_INFO_REQ( unsigned int seqid, const char *xml );
		//��ѯ��������Ż���Ϣ
		bool Proc_UPDETAIL_DISCOUNT_INFO_REQ( unsigned int seqid, const char *xml);
		//��ѯ�����̼���Ϣ
		bool Proc_UPUNION_BUSINESS_INFO_REQ( unsigned int seqid, const char *xml);
		//��ѯ���������̼���Ϣ
		bool Proc_UPDETAIL_UNION_BUSINESS_INFO_REQ( unsigned int seqid, const char *xml );
		//�û���¼
		bool Proc_UPLOGIN_INFO_REQ( unsigned int seqid, const char *xml );
		//����������ѯ
		bool Proc_UPQUERY_BALLANCE_LIST_REQ( unsigned int seqid, const char *xml );
		//��ѯ�ŵ�
		bool Proc_UPQUERY_STORE_LIST_REQ( unsigned int seqid, const char *xml );
		//��ѯ�ŵ�����
		bool Proc_UPQUERY_VIEW_STORE_INFO_REQ(unsigned int seqid, const char *xml );
		//�°汾��ѯ�Ż���Ϣ
		bool Proc_UPQUERY_DISCOUNT_LIST_REQ(unsigned int seqid, const char *xml );
		//�°汾��ѯ�Ż���Ϣ��ϸ�б�
		bool Proc_UPVIEW_DISCOUNT_INFO_REQ(unsigned int seqid, const char *xml );
		//��ʷ���׼�¼��ѯ
		bool Proc_UPQUERY_TRADE_LIST_REQ( unsigned int seqid, const char *xml );
		//��ѯ�ղ��б�
		bool Proc_UPQUERY_FAVORITE_LIST_REQ( unsigned int seqid, const char *xml );
		//��ѯ�ղ��б�����
		bool Proc_UPVIEW_FAVORITE_INFO_REQ( unsigned int seqid, const char *xml );
		//����ղ�����
		bool Proc_UPADD_FAVORITE_REQ( unsigned int seqid, const char *xml );
		//ɾ���ղ�����
		bool Proc_UPDEL_FAVORITE_REQ( unsigned int seqid, const char *xml );
		//��ȡĿ�ĵ�
		bool Proc_UPGET_DESTINATION_REQ( unsigned int seqid, const char *xml );
		// ��������
		void DeliverPacket( unsigned int fd, unsigned int cmd, IPacket *msg);
	private:
		// �����������
		void ProcessError( unsigned int seq , bool remove);
	private:
		// ����������
		IPlugin			   		*_pEnv;
		// ����XML���õ�HTTP����
		CHttpCaller				_httpcaller;
		// ����
		ServiceTable			_srv_table;
		// ���������Ϣ��Ӧ��ϵ
		CSeq2Msg				_seq2msgid;
		// ���÷���URL��ַ
		CQString 				_callUrl;
		// ���ݵȴ�����
		CMsgQueue				_msgqueue;
		// ������������
		CResultMgr			    _resultmgr;
		// �߳�ִ�ж���
		share::ThreadManager 	_thread;
		// �źŵȴ�����
		share::Monitor		 	_monitor;
		// �Ƿ��ʼ��
		bool 				 	_inited;
		// ���ݽ������
		CCarServiceUnPackMgr    _unpacker;
		// �������
		CPackFactory *			_packfactory;
	};
};
#endif /* SERVICECALLER_H_ */
