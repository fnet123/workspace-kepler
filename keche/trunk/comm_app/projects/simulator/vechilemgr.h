/**
 * Author: humingqing
 * date:   2011-09-19
 * memo:  ����ģ�����������
 */
#ifndef __VECHILEMGR_H__
#define __VECHILEMGR_H__

#include "interface.h"
#include "bench.h"
#include "simutil.h"
#include <string>
#include <map>
#include <list>
#include <vector>
#include <BaseClient.h>
#include <protocol.h>
#include <GBProtoParse.h>
#include <Thread.h>

#include "Logistics.h"

using namespace std ;

#define THREAD_SEND    0x0100
#define TCP_MODE	   0x0001
#define UDP_MODE	   0x0002
#define OFF_LINE       0x0001
#define ON_LINE		   0x0002

class CVechileMgr : public BaseClient, public IVechileClient
{
	class CQueue
	{
	public:
		CQueue() {}
		~CQueue(){}

		void AddSock( socket_t *sock )
		{
			share::Guard guard(_mutex) ;
			{
				_lstSock.push_back( sock ) ;
			}
		}
		socket_t * PopSock( void )
		{
			share::Guard guard( _mutex ) ;
			{
				if ( _lstSock.empty() ) {
					return NULL ;
				}
				socket_t *sock = _lstSock.front() ;
				_lstSock.pop_front() ;
				return  sock ;
			}
		}
		bool IsEmpty( void ) {
			share::Guard guard( _mutex ) ;
			return _lstSock.empty() ;
		}

	private:
		share::Mutex  		  _mutex ;
		list<socket_t *>  	  _lstSock ;
	};

	class CWait
	{
		typedef multimap<string, int>  WaitMap;
	public:
		CWait(){}
		~CWait(){}

		void Add( const char *phone ) {
			_mpWait.insert( make_pair(phone,0) ) ;
			printf( "add phone %s", phone ) ;
		}

		bool Pop( const char *phone ) {
			WaitMap::iterator it = _mpWait.find( phone ) ;
			if ( it == _mpWait.end() ) {
				return false ;
			}
			printf( "pop phone %s", phone ) ;
			_mpWait.erase( it ) ;
			return true ;
		}

	private:
		share::Mutex  	_mutex ;
		WaitMap  		_mpWait ;
	};

	// �ְ�����
	class CPackSpliter : public IPackSpliter
	{
	public:
		CPackSpliter() {}
		virtual ~CPackSpliter() {}
		// �ְ�����
		virtual struct packet * get_kfifo_packet( DataBuffer *fifo ) ;
		// �ͷ����ݰ�
		virtual void free_kfifo_packet( struct packet *packet ) {
			free_packet( packet ) ;
		}
	};

	// ģ�⳵����Ϣ
	struct _stVechile
	{
		socket_t   *fd_ ;		   // TCP��FD
		socket_t   *ufd_ ;		   // UDP��FD
		int    gps_pos_ ;      // GPS���ݵ�λ��
		char   car_id_[6] ;    // BCD������Ϣ
		short  seq_id_ ;       // ���ID
		char   termid_[7] ;    // �ն�ID
		char   carnum_[30] ;   // ����
		char   phone[12] ;     // �ֻ���
		time_t last_gps_ ;     // ���һ���ϱ�λ�õ�ʱ��
		time_t last_access_ ;  // ���һ�η��ʵ�ʱ��
		time_t last_conn_ ;    // ���һ�ε�½��ʱ��
		int    gps_count_ ;    // ����GPS���ݴ���
		time_t last_pic_  ;    // ���һ�η���PIC
		time_t last_alam_ ;     // ���һ�θ澯
		time_t last_candata_ ;  // ���һ���ϴ�CAN����
		time_t lgs_time_ ;		// ���һ�η���ʱ��
		int	   car_state_ ;     // ����״̬
	};

public:
	CVechileMgr() ;
	~CVechileMgr() ;

	bool Init( ISystemEnv *pEnv ) ;
	bool Start( void ) ;
	void Stop( void ) ;

public:
	// ���ݵ���
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ�����
	virtual void on_dis_connection( socket_t *sock );
	// �����ӵ���
	virtual void on_new_connection( socket_t *sock , const char* ip, int port){};
	// �����̶߳���
	virtual void run( void *param ) ;

protected:
	// ��ʱ�߳�
	virtual void TimeWork();
	// �����߳�
	virtual void NoopWork(){};
	// �����߳�
	virtual void SendWork() ;
private:
	// ����һ�����ݰ�
	void HandleOnePacket( socket_t *sock , const char *data, int len ) ;
	// �Ƴ������û�
	void RemoveOnlineUser( socket_t *sock ) ;
	// ��������û�
	void AddOnlineUser( _stVechile *pVechile ) ;
	// ��������û�
	void AddOfflineUser( _stVechile *pVechile ) ;
	// �Ƴ������û�
	void RemoveOfflineUser( _stVechile *pVechile ) ;
	// �Ƴ�ָ����FD
	void RemoveOfflineUser( socket_t *sock ) ;
	// ���������û�
	void LoadAllUser( void ) ;
	// ������߳���
	bool CheckOnlineUser( int ncount ) ;
	// ������߳���
	bool CheckOfflineUser( void ) ;
	// ��½������
	bool LoginServer(  _stVechile *pVechile ) ;
	// ����5B����
	bool Send5BData( socket_t *sock , const char *data, const int len ) ;
	// ������Ϣͷ��
	void BuildHeader( GBheader &header, unsigned short msgid, unsigned short len , _stVechile *p ) ;
	// ȡ�����߳���
	bool GetOnlineVechile( socket_t *sock, _stVechile *&p ) ;
	// ����λ����Ϣ
	bool SendLocationPos( _stVechile *p , int ncount ) ;
	// ����ͼƬ����
	bool SendLocationPic( _stVechile *p , int ncount ) ;
	/*����͸������*/
	bool SendTransparentMsg( _stVechile *p , int ncount,unsigned short wType);

private:
	// ��������ָ��
	ISystemEnv			* _pEnv ;
	// ������IP
	string 				  _server_ip ;
	// �������˿�
	unsigned short 		  _server_port ;
	// �̳߳ض���
	share::ThreadManager  _send_thread ;
	// �߳�����
	unsigned int 		  _thread_num ;
	// ����λ�ð�ʱ����
	unsigned int 		  _time_span ;
	// ģ�⳵�ĸ���
	unsigned int 		  _vechile_num ;
	// ����ǰ׺
	unsigned int 		  _phone_numpre ;
	// ��ʼλ��
	unsigned int 		  _start_pos ;
	// ����GPS����
	string				  _gps_filepath ;
	// GPS����
	vector<Point> 		  _gps_vec ;
	// �ְ�
	CPackSpliter		  _pack_spliter ;
	// �Ƿ��ʼ��
	bool				  _vechile_inited ;
	// ��������ģʽ
	unsigned int 		  _connect_mode ;
	// ���峵������
	typedef map<socket_t*,_stVechile*>  MapVechile ;
	// ���ߵĳ���
	MapVechile 			  _mapOnline ;

	typedef list<_stVechile*>     ListVechile ;
	// ����͵ĳ���
	ListVechile			  _listOnline ;
	// ���ߵĳ���
	ListVechile			  _listOffline ;
	// ����������
	share::Mutex		  _mutex_online ;
	// ����������
	share::Mutex		  _mutex_offline ;
	// ���������
	share::Mutex		  _mutex_vechile ;
	// ͳ�Ʋ�������
	CBench				  _bench ;
	// �����û�ʱ����
	unsigned int		  _deluser_span ;
	// һ�ε��ߵ��û�����
	unsigned int 		  _deluser_num ;
	// ��ǰ�ѵ��ߵ��û���
	unsigned int		  _deluser_count ;
	// �澯ʱ����
	unsigned int 		  _alam_time ;
	// �ϴ�ͼƬʱ����
	unsigned int 		  _upload_time ;
	// �ϴ�CAN����ʱ��
	unsigned int 		  _candata_time ;
	// ���ƺŵ�һ����ĸ
	unsigned char		  _simfirst_char ;
	// ������SOCK����
	CQueue				  _queue_sock ;
	// ���һ��ɾ���û���ʱ��
	time_t				  _last_deluser ;
	// �ϴ���ͼƬ��URL
	string 				  _pic_url ;
	// ����ͼƬ�ϴ�
	CWait				  _wait_pic ;

	/*����*/
	CLogistics        	 *_logistics;
	//��������ģ������·��
	string            	  _logistics_path;
};

#endif
