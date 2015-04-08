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
#include <TQueue.h>
#include <Util.h>
#include <interpacker.h>

#include "Logistics.h"

using namespace std ;

#define THREAD_SEND    0x0100
#define TCP_MODE	   0x0001
#define UDP_MODE	   0x0002
#define OFF_LINE       0x0001
#define ON_LINE		   0x0002

class CVechileMgr : public BaseClient, public IVechileClient
{
	// ģ�⳵����Ϣ
	struct _stVechile
	{
		socket_t   *fd_ ;		   // TCP��FD
		socket_t   *ufd_ ;		   // UDP��FD
		int    gps_pos_ ;      // GPS���ݵ�λ��
		char   car_id_[6] ;    // BCD������Ϣ
		short  seq_id_ ;       // ���ID
		char   termid_[7] ;    // �ն�ID
		char   termtype[21] ;  // �ն�����
		char   carnum_[30] ;   // ����
		char   phone[12] ;     // �ֻ���
		char   carcolor ;      // ������ɫ
		short  proid ;		   // ʡ��ID
		short  cityid ;		   // ����ID
		int64_t last_gps_ ;     // ���һ���ϱ�λ�õ�ʱ��
		int64_t last_access_ ;  // ���һ�η��ʵ�ʱ��
		int64_t last_conn_ ;    // ���һ�ε�½��ʱ��
		int     gps_count_ ;    // ����GPS���ݴ���
		int64_t last_pic_  ;    // ���һ�η���PIC
		int64_t last_alam_ ;     // ���һ�θ澯
		int64_t last_candata_ ;  // ���һ���ϴ�CAN����
		int64_t lgs_time_ ;		// ���һ�η���ʱ��
		int	   car_state_ ;     // ����״̬
		_stVechile *_next ;     // ����ָ��
		_stVechile *_pre ;      // ǰ��ָ��
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
	virtual void NoopWork() ;
	// �����߳�
	virtual void SendWork() ;
private:
	// ���ļ��м����û�����
	bool LoadUserFromPath( const char *path ) ;
	// ����һ�����ݰ�
	void HandleOnePacket( socket_t *sock , const char *data, int len ) ;
	// ���������û�
	void LoadAllUser( void ) ;
	// ������߳���
	int64_t CheckOnlineUser( int ncount ) ;
	// ������߳���
	bool CheckOfflineUser( void ) ;
	// ��½������
	bool LoginServer(  _stVechile *pVechile ) ;
	// ����5B����
	bool Send5BData( socket_t *sock , const char *data, const int len ) ;
	// ������Ϣͷ��
	void BuildHeader( GBheader &header, unsigned short msgid, unsigned short len , _stVechile *p ) ;
	// ����λ����Ϣ
	bool SendLocationPos( _stVechile *p , int ncount ) ;
	// ����ͼƬ����
	bool SendLocationPic( _stVechile *p , int ncount ) ;
	/*����͸������*/
	bool SendTransparentMsg( _stVechile *p , int ncount,unsigned short wType);
	// ȡ���û��б�
	int  GetUser( list<_stVechile*> &lst , int state ) ;
	// ����ʱ��λ�ȡ�ٶ�
	int get_car_speed();

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
	int64_t 		  	  _time_span ;
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
	C808Spliter		  	  _pack_spliter ;
	// �Ƿ��ʼ��
	bool				  _vechile_inited ;
	// ��������ģʽ
	unsigned int 		  _connect_mode ;
	// ���������
	share::Mutex		  _carmutex ;
	// ���г����б�
	TQueue<_stVechile>    _car_queue ;
	// ͳ�Ʋ�������
	CBench				  _bench ;
	// �����û�ʱ����
	int64_t		 		  _deluser_span ;
	// һ�ε��ߵ��û�����
	unsigned int 		  _deluser_num ;
	// ��ǰ�ѵ��ߵ��û���
	unsigned int		  _deluser_count ;
	// �澯ʱ����
	int64_t 		  	  _alam_time ;
	// �ϴ�ͼƬʱ����
	int64_t 		  	  _upload_time ;
	// �ϴ�CAN����ʱ��
	int64_t		  		  _candata_time ;
	// ���ƺŵ�һ����ĸ
	unsigned char		  _simfirst_char ;
	// ���һ��ɾ���û���ʱ��
	int64_t			  	  _last_deluser ;
	// �ϴ���ͼƬ��URL
	string 				  _pic_url ;
	/*����*/
	CLogistics        	 *_logistics;
	//��������ģ������·��
	string            	  _logistics_path;
	// ��Ų�����
	share::Mutex          _seq_mutex ;
	// �û��ļ�·��
	string 				  _sim_car_user ;
	// ͼƬ�б���
	share::Monitor		   _picmonitor;
	// �·�ͼƬ�б�
	std::list<_stVechile*> _piclist ;

	map<int,int>          _car_speed;
};

#endif
