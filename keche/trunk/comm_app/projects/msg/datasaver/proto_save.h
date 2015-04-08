/******************************************************
 *  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
 *   FileName: proto_save.h
 *     Author: liubo  2012-6-5 
 *Description:
 *******************************************************/

#ifndef PROTO_SAVE_H_
#define PROTO_SAVE_H_

#include <interface.h>
#include "idatapool.h"
#include <map>
#include <string>
#include <vector>
#include "run_info.h"
#include "filecache.h"
#include "dbsave.h"

#define CAR_INFO_TIMEOUT (60 * 5)

using namespace std;

/* ����ص���󳤶� 100 ��������*/
#define MAX_DATA_POOL_NUM        (100 * 1000)

#define gps_info_table           "TH_VEHICLE_TRACK"
#define gps_info_dump            "TH_VEHICLE_DUMP"
#define alert_table              "TH_VEHICLE_ALARM"
#define off_online_table         "TH_VEHICLE_ONOFFLINE"
#define driver_action_table      "TH_VEHICLE_DRIVERACTION"
#define engine_load_table        "TH_VEHICLE_ENGINELOAD"
#define alert_log_table          "TH_VEHICLE_ALARM_LOG"
#define down_command_table       "TH_VEHICLE_COMMAND"
#define meadia_table             "TH_VEHICLE_MEDIA"

#define MSG_TERM_UP              0x0100  /* �ն������ϴ�         */
#define MSG_SET_TERM_PARAM       0x0200  /* �����ն˲���         */
#define MSG_GET_TERM_PARAM       0x0300  /* ��ȡ�ն˲���        */
#define MSG_CTRL_TERM            0x0300  /* �ն˿���                 */
#define MSG_SNDM_DOWN            0x0400  /* �����·�                 */
#define MSG_PLAT_REQ_TERM        0x0500  /* ƽ̨�����ն�����  */
#define MSG_TERM_REQ_PLAT        0x0600  /* �ն�����ƽ̨����  */
#define MSG_SUBSCRIBE            0x0700  /* ����ָ��                  */
#define MSG_DCALL                0x0800  /* ����ָ��ش�          */

class InterProto;
class VehicleInfo;

#define DBSTATE_ONLINE    1   // ���ݿ����
#define DBSTATE_OFFLINE   0   // ���ݿⲻ����״̬
#define DBSTATE_MODIFY    3   // �иĶ���
#define DBSTATE_DELETE    4   // ��Ҫɾ����

class Inter2SaveConvert : public IOHandler
{
#define MAX_DBTYPE  2  // �������ݿ�����
	// ���ݿ����ṹ��
	struct DataBaseObj
	{
        DataBaseObj(){}
		DataBaseObj(string v): _dbvalues(v), _dbface(NULL), _last(0),
		_state(DBSTATE_OFFLINE){}

        string _dbvalues ;     // ��redis������
        int _type;             // ���ݿ������
        int _groupid;          // ��ID
		unsigned int _state ;  // ״̬�Ƿ�Ϊ����
		IDBFace     *_dbface;  // ��ǰ��������
		time_t		 _last ;   // ���һ�η��ʵ�ʱ��
	};

public:
#define max_num 16
	class AreaAlert; 	//���򱨾�
    class GpsInfo;      //����GPS��Ϣ�ͱ�����Ϣ��һ�鴦��ģ����������ֶε�ʹ�õ�gps��Ϣ��

	typedef bool (DataBaseSave::*ConvertSqlFun)(InterProto *);

	Inter2SaveConvert() ;
	virtual ~Inter2SaveConvert();

	bool init(IDataPool *save_pool, ISystemEnv *pEnv);
	void stop() ;

	bool convert(InterData *data);

    //���ݿ�Ĳ������,�������ɵ�����һ���߳���ִ��
	void savedb();

	// �ص������ļ�����������
	int HandleQueue( const char *sid , void *buf, int len , int msgid = 0 ) ;

    // ����������ļ�������
    bool add_pool(IDataPool::DB_TYPE type, IDataPool::DB_OPRE oper, string table, CSqlObj *obj, CSqlWhere* where = NULL, int groupid = 0 );

	//���ɲ�������, num ��ʾ������Ŀ
    /**
	static GpsInfo create_test_gpsinfo(int num = 10000);
    static void create_test_msg(vector<string> &vec);
	*/
private:
	bool get_dbconfig(map<string, DataBaseObj> &map_dbobj);
    bool update_dbconfig();

	bool data2proto(InterData *data,  InterProto *inter_proto);
    bool get_vehicle_info(const char *car_id, VehicleInfo &car_info);

private:
    map<unsigned short, ConvertSqlFun> _map_mongo_fun;

    //Ҫ֧�Ŷ��̣߳�������������Ҫ������
	share::Mutex  	     		_mutex ;
    share::Monitor  	    	_car_monitor ;
    map<long long, VehicleInfo> _map_car_info;

    CFileCache					_filecache ;
    //��dataserver������п���
    IDataPool         		   *_save_pool;
    RunInfo 					_run_info;
	ISystemEnv 		  	       *_pEnv ;

    map<string, DataBaseObj>   _map_dbobj;
    // �Ƿ�ֹͣ
    bool 					   _bstop ;
    DataBaseSave               _db_save;
};

class VehicleInfo
{
public:
	//Ĭ������ӳ�ʱ
	time_t update_time;

	unsigned int corpid;
    unsigned int gid;
	unsigned char corlor;
	string vechile;
	string termid;
	string oemcode;
	unsigned vid;

	VehicleInfo()
    {
    	update_time = time(0);
    }
};

/* ��������ģ�鴦���ʱ�� �Ƚ���Ҫ������ȫ��ȡ�� */
class InterProto
{
public:
	unsigned short msg_type;
	VehicleInfo vehicle_info;
    long long phone_number;
	map<string, string> kvmap;
	InterData *meta_data;

	InterProto()
	{
        msg_type   = 0xffff;
        phone_number = 0;
	}
};

//���򱨾�
class Inter2SaveConvert::AreaAlert
{
public:
	  // λ�����ͣ��������·ID������  | ��������|������·ID|�������|��ʻʱ��
    char isvalid ; // 0��ʾ��Ч�� 1��ʾ��Ч
    unsigned char area_type;
    unsigned char area_line_id;
    unsigned char trigger;
    unsigned char drive_time;
    AreaAlert() : isvalid(0)
    {
    	area_type = area_line_id = trigger = drive_time = 0;
    }
};

/* ***********************************************
 * 1. ����GPS��Ϣ�ͱ�����Ϣ��һ�鴦��ģ����������ֶε�ʹ�õ�gps��Ϣ��
 * 2. �����ŵ������ԭ����������Ĳ��Դ������õ����Ķ��塣
 * ***********************************************/
class Inter2SaveConvert::GpsInfo
{
public:
	Inter2SaveConvert::AreaAlert area_alert;
	long long car_id;
    unsigned int vid;
    unsigned int company_id;
	unsigned int map_lon;
	unsigned int map_lat;
	time_t sys_time;
    int gps_time;
	unsigned int ssp;
	unsigned int sdir;
	unsigned int salt;
	unsigned int smil;
	unsigned int stow;
	string saa; //����������Ϣ
};


#endif /* PROTO2SAVE_H_ */
