/******************************************************
 *  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
 *   FileName: oraclesave.cpp
 *     Author: liubo  2012-10-24 
 *Description:
 *******************************************************/

#include "oraclesave.h"
#include "./proto_save.h"
#include "idatapool.h"
#include "mapconvert.h"
#include "tools.h"

OracleSave::OracleSave()
{
	// TODO Auto-generated constructor stub
}

OracleSave::~OracleSave()
{
	// TODO Auto-generated destructor stub
}

string itostring(int i)
{
	char buffer[16] = { 0 };
	sprintf(buffer, "%d", i);
	return buffer;
}

string lltostring(long long i)
{
	char buffer[16] = { 0 };
	sprintf(buffer, "%lld", i);
	return buffer;
}

string get_user_from_seq(string &seq)
{
	vector<string> vec_seq;
	splitvector(seq, vec_seq, "_", 0);
	return vec_seq[0];
}

/**********************************************************************
 OP_ID	NUMBER(15,0)	Yes	1	�û����	1
 VEHICLE_NO	VARCHAR2(40 BYTE)	Yes	2	���ƺ�	��A63753
 CO_SUTC	NUMBER(15,0)	Yes	3	����ʱ��utc	1321946554065
 CO_TYPE	VARCHAR2(30 BYTE)	Yes	4	ָ������Call	D_CTLM
 CO_FROM	NUMBER(2,0)	Yes	5	ָ����Դ��0��ƽ̨ 1���ƽ̨��	0
 CO_SEQ	VARCHAR2(100 BYTE)	No	6	���� ҵ�����к�	1_1321946554_63
 CO_CHANNEL	VARCHAR2(20 BYTE)	Yes	7	ͨѶ��ʽ	0
 CO_PARM	VARCHAR2(1000 BYTE)	Yes	8	ָ���������ֵ����ʽ	TYPE:10,RETRY:1,VALUE:1|1|1|0|640*480|100|100|100|100|100
 CO_COMMAND	VARCHAR2(1000 BYTE)	Yes	9	ԭʼָ���ַ���	CAITS 1_1321946554_63 4C54_15313563753 0 D_CTLM {TYPE:10,RETRY:1,VALUE:1|1|1|0|640*480|100|100|100|100|100}
 CO_STATUS	NUMBER(2,0)	Yes	10	����״̬-1�ȴ���Ӧ  0:�ɹ�	0
 CR_RESULT	VARCHAR2(1000 BYTE)	Yes	11	ָ���Ӧ�������	RET:0
 CR_TIME	NUMBER(15,0)	Yes	12	��Ӧʱ��utc(s)	1321946554610
 CO_OEMCODE	VARCHAR2(20 BYTE)	Yes	13	�ն����ʹ���	4C54
 CO_SENDTIMES	NUMBER(2,0)	Yes	14	�ѷ��ʹ���	1
 CO_TRYTIMES	NUMBER(2,0)	Yes	15	���Դ���	1
 CO_SUBTYPE	VARCHAR2(20 BYTE)	Yes	16	ָ��������	10
 CREATE_BY	VARCHAR2(20 BYTE)	Yes	17	������	1
 CREATE_TIME	NUMBER(15,0)	Yes	18	����ʱ��	1321946554065
 VID	NUMBER(15,0)	Yes	19	�������	104
 CO_TEXT	VARCHAR2(500 BYTE)	Yes	20	ָ��ҳ����ʾ����	ץ��ָ��
 AUTO_ID	NUMBER(15,0)	No	21	��������	5855
 UPDATE_BY	VARCHAR2(20 BYTE)	Yes	22	�����û�	null
 UPDATE_TIME	NUMBER(15,0)	Yes	23	����ʱ��	null
 AREA_ID	VARCHAR2(32 BYTE)	Yes	24	ʡ�����	null

���Խ���� insert into TH_VEHICLE_COMMAND(AUTO_ID,CO_CHANNEL,CO_COMMAND,CO_FROM,CO_OEMCODE,CO_PARM,CO_SENDTIMES,CO_SEQ,CO_STATUS,CO_SUBTYPE,CO_TRYTIMES,CO_TYPE,CREATE_BY,CREATE_TIME,OP_ID,VEHICLE_NO,VID)values(seq_auto_id.nextval,'CAITS','',0,'','TYPE:24,RETRY:0,VALUE:-1}',1,'10206_1352190205_45224',0,'',1,'0','10206',1352190440,10206,'��N41799',529), result 0,Execute,oracledb.cpp:95
 **********************************************************************/

bool OracleSave::oracle_down_command(InterProto *inter_proto)
{
    // printf("into OracleSave::oracle_down_command \n");
	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	string command; //ԭʼָ���ַ���

	sql_obj->AddInteger("OP_ID",
			atoi(get_user_from_seq(inter_proto->meta_data->_seqid).c_str()));
	sql_obj->AddString("VEHICLE_NO", inter_proto->vehicle_info.vechile);
	sql_obj->AddLongLong("CO_SUTC", time(0) * 1000);
	sql_obj->AddString("CO_TYPE", inter_proto->meta_data->_cmtype);
	sql_obj->AddInteger("CO_FROM", 0);
	sql_obj->AddString("CO_SEQ", inter_proto->meta_data->_seqid);
	sql_obj->AddString("CO_CHANNEL", inter_proto->meta_data->_transtype);
	sql_obj->AddString("CO_PARM", inter_proto->meta_data->_packdata.substr(1,inter_proto->meta_data->_packdata.length() - 1));
    sql_obj->AddVar("AUTO_ID", "seq_auto_id.nextval");
	sql_obj->AddString("CO_COMMAND", command);
	sql_obj->AddInteger("CO_STATUS", 0); //���ڵȴ���Ӧ״̬��
//	sql_obj->AddString("CR_RESULT", command);
//	sql_obj->AddString("CR_TIME", command); //	NUMBER(15,0)	Yes	12	��Ӧʱ��utc(s)	1321946554610
	sql_obj->AddString("CO_OEMCODE", inter_proto->vehicle_info.oemcode);
	sql_obj->AddInteger("CO_SENDTIMES", 1);
	sql_obj->AddInteger("CO_TRYTIMES", 1);
	sql_obj->AddString("CO_SUBTYPE", inter_proto->kvmap["type"]);
	sql_obj->AddString("CREATE_BY",
    get_user_from_seq(inter_proto->meta_data->_seqid));
	sql_obj->AddInteger("CREATE_TIME", time(0));
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
//	TODO sql_obj->AddString("CO_TEXT", command);

//	sql_obj->AddString("UPDATE_BY", command);
//	sql_obj->AddInteger("UPDATE_TIME", command); //��ʱ�����ָ��ظ���ʱ����д
//	sql_obj->AddString("AREA_ID	", command);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_COMMAND, sql_obj);

	return true;
}

/*******************************
 //U_REPT 0x0800��ý���¼���Ϣ�ϴ�{TYPE:39,120:��ý������ID,121:��ý������,122:��ý���ʽ����,123:�¼������,124:ͨ��ID}

 PID	VARCHAR2(100 BYTE)	No	1	��Ψһ��ʶ	723e4df3-0790-4572-a458-cc2ab7178dee
 VID	VARCHAR2(32 BYTE)	No	2	������ʶ	233723
 MULTIMEDIA_TYPE	NUMBER(1,0)	Yes	3	��ý������ 0:ͼ��1����Ƶ��2����Ƶ	0
 MULTIMEDIA_FORMAT	NUMBER(1,0)	Yes	4	��ý���ʽ 0 JPEG 1: TIF; 2: MP3; 3: WAV; 4: WMV	0
 EVENT_NUM	NUMBER(1,0)	Yes	5	�¼�����0 ƽ̨�·� 1 ��ʱ���� 2 ���ٱ��� 3 ��ײ�෭��������	2
 CHANNEL_ID	NUMBER(2,0)	Yes	6	ͨ����ʶ	1

���Խ���� insert into TH_VEHICLE_MULTIMEDIA_EVENT(CHANNEL_ID,EVENT_NUM,MULTIMEDIA_FORMAT,MULTIMEDIA_TYPE,PID,VID)values(2,1,0,0,sys_guid(),'319')
 ******************************/

bool OracleSave::oracle_vehicle_multimedia_event(InterProto *inter_proto)
{
    // printf("into OracleSave::oracle_vehicle_multimedia_event \n");
    CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);

    //�Լ�����һ��guid�ĺ��������������ݿ��Լ����ɡ�
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddString("VID", itostring(inter_proto->vehicle_info.vid));
	sql_obj->AddInteger("MULTIMEDIA_TYPE",
			atoi(inter_proto->kvmap["121"].c_str()));
	sql_obj->AddInteger("MULTIMEDIA_FORMAT",
			atoi(inter_proto->kvmap["122"].c_str()));
	sql_obj->AddInteger("EVENT_NUM", atoi(inter_proto->kvmap["123"].c_str()));
	sql_obj->AddInteger("CHANNEL_ID", atoi(inter_proto->kvmap["124"].c_str()));

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_MULTIMEDIA_EVENT, sql_obj);

	return true;
}

/*
 MEDIA_ID	VARCHAR2(100 BYTE)	No	1	��ý���ţ���������	4e569d30-e920-4af0-9818-4b4764cd5644
 VID	NUMBER(15,0)	No	2	����ID	736
 DEVICE_NO	VARCHAR2(20 BYTE)	Yes	3	�ֻ�����	15290424768
 MTYPE_CODE	VARCHAR2(20 BYTE)	Yes	4	��ý������	0
 MFORMAT_CODE	VARCHAR2(20 BYTE)	Yes	5	��ý���ʽ	0
 EVENT_TYPE	VARCHAR2(20 BYTE)	Yes	6	�¼������ �μ���ý���¼�������	7
 UTC	NUMBER(15,0)	Yes	7	��ý���ϴ�ʱ��UTC	1.35003E+12
 MEDIA_URI	VARCHAR2(200 BYTE)	Yes	8	��ý��URL	2012/10/12/20121012164618-E001_15290424768-2537-7-1-0-0.jpeg
 LENS_NO	VARCHAR2(10 BYTE)	Yes	9	ͨ����	1
 FILE_SIZE	NUMBER(10,0)	Yes	10	��ý���ļ���С�ֽ�	NULL
 DIMENSION	VARCHAR2(20 BYTE)	Yes	11	ͼƬ�ߴ���(1:320x240, 2:640x480, 3:800x600, 4:1024x768)	NULL
 FILE_TYPE	VARCHAR2(20 BYTE)	Yes	12	�ļ����� 1:jpg;2:gif;3:tiff;4:����	NULL
 SAMPLE_RATE	NUMBER(5,0)	Yes	13	��Ƶ����Ƶ��(��Ƶ���ý����Ϣ��Ҫ)	NULL
 LAT	NUMBER	Yes	14	γ�ȣ���λ��ʮ���֮һ�ȣ�	16533939
 LON	NUMBER	Yes	15	���ȣ���λ��ʮ���֮һ�ȣ�	65969722
 MAPLON	NUMBER	Yes	16	��ͼƫ�ƺ�GPS����	65972445
 MAPLAT	NUMBER	Yes	17	��ͼƫ�ƺ�GPSγ��	16531874
 ELEVATION	NUMBER(10,0)	Yes	18	���θ߶ȣ���λ���ף�	249
 DIRECTION	NUMBER(10,0)	Yes	19	���򣨵�λ���ȣ�	238
 GPS_SPEED	NUMBER(10,0)	Yes	20	�ٶ�(��λ����/Сʱ)	0
 STATUS_CODE	VARCHAR2(200 BYTE)	Yes	21	״̬��Ϣ, ��ֵ�ö��ŷָ�	3
 ALARM_CODE	VARCHAR2(200 BYTE)	Yes	22	������Ϣ����ֵ�ö��ŷָ�	NULL
 SYSUTC	NUMBER(15,0)	Yes	23	���ʱ��utc	NULL
 IS_OVERLOAD	NUMBER(2,0)	Yes	24	�Ƿ���(0 �� 1 ��)	0
 EVENT_STATUS	NUMBER(2,0)	Yes	25	�¼�״̬��0 �ɹ�1 ʧ�� 2ִ���У�	NULL
 ENABLE_FLAG	VARCHAR2(2 BYTE)	Yes	26	��Ч��� 1:��Ч 0:��Ч Ĭ��Ϊ1	1
 SEQ	VARCHAR2(100 BYTE)	Yes	27	SEQָ��Ψһ�ı�ʶ��	NULL
 SEND_USER	NUMBER(10,0)	Yes	28	������ID	NULL
 EVENTID	VARCHAR2(10 BYTE)	Yes	29	0��ƽ̨�·�ָ�1����ʱ������2�����ٱ���������3����ײ�෭����������4���ſ����գ�5���Ź����գ�6�������ɿ���أ�ʱ�ٴӣ�20���ﳬ��20����	NULL
 MEMO	VARCHAR2(500 BYTE)	Yes	30	��ע	NULL
 MULT_MEDIA_ID	VARCHAR2(100 BYTE)	Yes	31	��ý������ID���Ϲ��¼�)	2537

 //U_REPT 0x0801��ý�������ϴ�{TYPE:3,120:��ý������ID,121:��ý������,122:��ý���ʽ����,123:�¼������,124:ͨ��ID ,
 * 125:url��ַ,1:����,2:γ��,3:�ٶ�,4:ʱ��,5:����,6:����,20:������־,8:����λ����Ϣ
insert into TH_VEHICLE_MEDIA(ALARM_CODE,DEVICE_NO,DIRECTION,ELEVATION,ENABLE_FLAG,EVENT_TYPE,GPS_SPEED,LAT,LENS_NO,LON,MEDIA_ID,MEDIA_URI,MFORMAT_CODE,MTYPE_CODE,VID)values('','15294614042',0,0,'1','4',0,0,'4',0,'939','','0','0',140150)
 */
bool OracleSave::oracle_vehicle_media(InterProto *inter_proto)
{
    // printf("into OracleSave::oracle_vehicle_media \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);

	sql_obj->AddString("MEDIA_ID", inter_proto->kvmap["120"]);
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("DEVICE_NO", lltostring(inter_proto->phone_number));
	sql_obj->AddString("MTYPE_CODE", inter_proto->kvmap["121"]);
	sql_obj->AddString("MFORMAT_CODE", inter_proto->kvmap["122"]);
	sql_obj->AddString("EVENT_TYPE", inter_proto->kvmap["123"]);
	sql_obj->AddLongLong("UTC", time(0) * 1000);
	sql_obj->AddString("MEDIA_URI", inter_proto->kvmap["125"]);
	sql_obj->AddString("LENS_NO", inter_proto->kvmap["124"]);
//  ���������ڴ��ϵ������ж�û��
//    sql_obj->AddInteger("FILE_SIZE", );
//    sql_obj->AddString("DIMENSION", );
//    sql_obj->AddString("FILE_TYPE", );
//    sql_obj->AddInteger("SAMPLE_RATE", );
	sql_obj->AddInteger("LAT", atoi(inter_proto->kvmap["2"].c_str()));
	sql_obj->AddInteger("LON", atoi(inter_proto->kvmap["1"].c_str()));
//    sql_obj->AddInteger("MAPLON", );
//    sql_obj->AddInteger("MAPLAT", );
	sql_obj->AddInteger("ELEVATION", atoi(inter_proto->kvmap["6"].c_str()));
	sql_obj->AddInteger("DIRECTION", atoi(inter_proto->kvmap["5"].c_str()));
	sql_obj->AddInteger("GPS_SPEED", atoi(inter_proto->kvmap["3"].c_str()));
//    sql_obj->AddString("STATUS_CODE", );
	sql_obj->AddString("ALARM_CODE", inter_proto->kvmap["20"]);
	sql_obj->AddLongLong("SYSUTC", time(0) * 1000);
//    sql_obj->AddInteger("IS_OVERLOAD", );
//    sql_obj->AddInteger("EVENT_STATUS", );
	sql_obj->AddString("ENABLE_FLAG", "1");

//    sql_obj->AddString("SEQ", );
//    sql_obj->AddInteger("SEND_USER", );
//    sql_obj->AddString("EVENTID", );
//    sql_obj->AddString("MEMO", );
//TODO  ��ý��ID����λ�õ�    sql_obj->AddString("MULT_MEDIA_ID", );

	int x = atoi(inter_proto->kvmap["1"].c_str());
	int y = atoi(inter_proto->kvmap["2"].c_str());

	MapConvert map_convert;
	Point *point = map_convert.getEncryPoint(x / 600000.000000, y / 600000.000000);
	if (point != NULL)
	{
		unsigned int map_lon = (unsigned int) (point->getX() * 600000);
		unsigned int map_lat = (unsigned int) (point->getY() * 600000);
		sql_obj->AddInteger("MAPLON", map_lon);
		sql_obj->AddInteger("MAPLAT", map_lat);
	}

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_MEDIA, sql_obj);

	return true;
}

/*
 PID	VARCHAR2(64 BYTE)	No	1	����	958eeb24-5c04-4b8c-bad4-397cfe9aaafb
 VID	NUMBER(15,0)	Yes	2	����ID	211262
 TYPE	VARCHAR2(16 BYTE)	Yes	3	��Ϣ����	1
 UTC	NUMBER(15,0)	Yes	4	�ϴ�ʱ��	1.35061E+12
 STATUS	NUMBER(1,0)	Yes	5	״̬	0
 U_REPT 0x0303��Ϣ�㲥/ȡ��{TYPE:33,83:��Ϣ����|*}(0:ȡ��,1:�㲥)
 */
bool OracleSave::oracle_vehicle_infoplay(InterProto *inter_proto)
{
    // printf("into OracleSave::oracle_vehicle_infoplay \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("DEVICE_NO", lltostring(inter_proto->phone_number));

	char type[16] = {0};
	int state = 0;
	sscanf(inter_proto->kvmap["83"].c_str(), "%s|%d", type, &state);
	sql_obj->AddString("TYPE", type);
	sql_obj->AddLongLong("UTC", time(0) * 1000);
	sql_obj->AddInteger("STATUS", state); //״̬Ĭ�ϳ�0

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_INFOPLAY, sql_obj);

	return true;
}

/*
 PID	VARCHAR2(64 BYTE)	No	1	����	1418babe-052e-46a0-898d-a403790e5ff2
 VID	NUMBER(15,0)	Yes	2	����ID	141141
 CONTENT	VARCHAR2(1024 BYTE)	Yes	3	�����˵�����	�����˵����ݣ�1111
 UTC	NUMBER(15,0)	Yes	4	�ϴ�ʱ��	1335520953619
 */

//U_REPT 0x0701�����˵��ϱ�{TYPE:35,87:BASE64����}
bool OracleSave::oracle_vehicle_eticket(InterProto *inter_proto)
{
    // printf("into OracleSave::oracle_vehicle_eticket \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);

	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("CONTENT", inter_proto->kvmap["87"]);
	sql_obj->AddLongLong("UTC", time(0) * 1000);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_ETICKET, sql_obj);

	return true;
}

/******************************************************
 PID	VARCHAR2(100 BYTE)	No	1	��Ψһ��ʶ	8ac0d1a3-90c7-4dad-8f75-ca5c4d60e077
 VID	VARCHAR2(32 BYTE)	No	2	������ʶ	140128
 DRIVER_NAME	VARCHAR2(64 BYTE)	Yes	3	��ʻԱ����	�׺��
 DRIVER_NO	VARCHAR2(64 BYTE)	Yes	4	��ʻԱ���֤����	NULL
 DRIVER_CERTIFICATE	VARCHAR2(128 BYTE)	Yes	5	��ʻԱ��ҵ�ʸ�֤	141002727380
 CERTIFICATE_AGENCY	VARCHAR2(256 BYTE)	Yes	6	��֤����	�ٷ��г�����������
 UTC	NUMBER(15,0)	Yes	7	�ϴ�ʱ��	1331606508968
 STATUS	NUMBER(1,0)	Yes	8	��ʻԱ���ʶ��״̬��0ʶ��ɹ���-1ʶ��ʧ��	1
 UP_STATUS	NUMBER(1,0)	Yes	9	�ϱ�״̬1.���� 2.����	NULL
 ******************************************************/
//U_REPT 0x0702��ʻԱ��ݲɼ��ϴ�{TYPE:8,RESLUT:���,110:��ʻԱ����,111:��ʻԱ����,112:��ҵ�ʸ�֤����,113:��֤��������}
bool OracleSave::oracle_vehicle_driver(InterProto *inter_proto)
{
    // printf("OracleSave::oracle_vehicle_driver \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("DRIVER_NAME", inter_proto->kvmap["110"]);
	sql_obj->AddString("DRIVER_NO", inter_proto->kvmap["111"]);
	sql_obj->AddString("DRIVER_CERTIFICATE", inter_proto->kvmap["112"]);
	sql_obj->AddString("CERTIFICATE_AGENCY", inter_proto->kvmap["113"]);
	sql_obj->AddLongLong("UTC", time(0) * 1000);
	sql_obj->AddInteger("STATUS", 0); //Ĭ��ʶ��ɹ�

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_DRIVER, sql_obj);

    return true;
}

/*
 PID	VARCHAR2(64 BYTE)	No	1	����	2bbb1d4f-a17d-41d0-a153-056e22a68c29
 VID	NUMBER(15,0)	Yes	2	����ID	10238
 CONTENT	VARCHAR2(3500 BYTE)	Yes	3	��ʻ��¼������	###########(BASE64���������ݣ�
 UTC	NUMBER(15,0)	Yes	4	�ϴ�ʱ��	1332983923410
 ISPARSE	NUMBER(2,0)	Yes	5	?????1???0??	1
 PARSENUM	NUMBER(2,0)	Yes	6	��������,����3��δ�����ɹ����ٽ���	1
 CO_SEQ	VARCHAR2(100 BYTE)	Yes	7	seq	NULL

 ������ʻ��¼������(70-99)
 */
bool OracleSave::oracle_vehicle_recorder(InterProto *inter_proto)
{
    //printf("OracleSave::oracle_vehicle_recorder \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);

	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("CONTENT", inter_proto->kvmap["61"]);
	sql_obj->AddLongLong("UTC", time(0) * 1000);
	sql_obj->AddInteger("ISPARSE", 1); //TODO �Ƿ������Ĭ����1
	sql_obj->AddInteger("PARSENUM", 1);
	sql_obj->AddString("CO_SEQ", inter_proto->meta_data->_seqid);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_RECORDER, sql_obj);

    return true;
}

/*
 DMSG_ID	NUMBER(15,0)	No	1	Ϣ��ţ���������	43040
 VID	NUMBER(15,0)	No	2	����ID	478
 VEHICLE_NO	VARCHAR2(40 BYTE)	No	3	���ƺ���	��N41136
 DMSG_UTC	NUMBER(15,0)	No	4	��Ϣ������UTCʱ��	1326697140953
 DMSG_SRTIME	NUMBER(15,0)	Yes	5	����ʱ��	1326697140953
 DMSG_FLAG	NUMBER(15,0)	Yes	6	��Ϣ��־λ����808Э��	12
 DMSG_TYPE	VARCHAR2(20 BYTE)	Yes	7	�������:1:������Ϣ2:·����Ϣ�ȱ���	NULL
 SEND_FLAG	NUMBER(2,0)	Yes	8	���ͱ�־1�����ж���0�����ж���	0
 DMSG_CONTENT	VARCHAR2(300 BYTE)	Yes	9	���ж�������	��������ʻ������
 SEND_RESULT	NUMBER(2,0)	Yes	10	���ͽ����0. �ɹ� 1.�豸����ʧ�� 2. ����ʧ�� 3. �豸��֧�ִ˹��� 4. �豸������ 5. ��ʱ��	-1
 DMSG_STATUS	NUMBER(2,0)	Yes	11	�Ƿ��Ѷ� 1-δ�� 0-�Ѷ�	1
 SEQ	VARCHAR2(40 BYTE)	No	12	SEQָ��Ψһ�ı�ʶ��	10206_1326697140_229
 VEHICLE_COLOR	VARCHAR2(20 BYTE)	Yes	13	������ɫ	2
 UMSG_SRTIME	NUMBER(15,0)	Yes	14	����ʱ��	NULL
 UMSG_CONTENT	VARCHAR2(300 BYTE)	Yes	15	���ж�������	NULL
 DSEND_USER_ID	NUMBER(10,0)	Yes	16	���з�����	10206
 USEND_USER_ID	NUMBER(10,0)	Yes	17	���з�����	NULL

 */

bool OracleSave::oracle_vehicle_dispatch_msg(InterProto *inter_proto)
{
    // printf("OracleSave::oracle_vehicle_dispatch_msg \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("DMSG_ID", "SEQ_DISPATCH_ID.nextval");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddString("VEHICLE_NO", inter_proto->vehicle_info.vechile);
	sql_obj->AddLongLong("DMSG_UTC", time(0) * 1000);
	sql_obj->AddLongLong("DMSG_SRTIME", time(0) * 1000);
	if (inter_proto->meta_data->_cmtype == "D_SNDM") //���ж���
	{
		sql_obj->AddInteger("SEND_FALG", 0);
	}
	else
	{
		sql_obj->AddInteger("SEND_FALG", 1);
	}

	sql_obj->AddString("DMSG_CONTENT", inter_proto->kvmap["2"]);

	sql_obj->AddInteger("SEND_RESULT", 0);
	sql_obj->AddInteger("DMSG_STATUS", 1);
	sql_obj->AddString("SEQ", inter_proto->meta_data->_seqid);
	sql_obj->AddInteger("VEHICLE_COLOR", inter_proto->vehicle_info.corlor);
//  UMSG_SRTIME	NUMBER(15,0)	Yes	14	����ʱ��	NULL
//	UMSG_CONTENT	VARCHAR2(300 BYTE)	Yes	15	���ж�������	NULL
	string user_id = get_user_from_seq(inter_proto->meta_data->_seqid);
	sql_obj->AddString("DSEND_USER_ID", user_id);
//	DSEND_USER_ID	NUMBER(10,0)	Yes	16	���з�����	10206
//	USEND_USER_ID	NUMBER(10,0)	Yes	17	���з�����	NULL

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_DISPATCH_MSG, sql_obj);

	return true;
}

/*
 AUTO_ID	NUMBER(15,0)	No	1	???????	8751945
 MODIFY_DATE	NUMBER(15,0)	No	2	???? utc??	1331005758260
 TID	NUMBER(15,0)	No	3	????	10338
 T_MAC	VARCHAR2(40 BYTE)	Yes	4	?????	11C1056
 PARAM_TYPE	VARCHAR2(100 BYTE)	Yes	5	???? ?????????	130
 PARAM_VALUE	VARCHAR2(20 BYTE)	Yes	6	???	14400
 SET_RESULT	NUMBER(2,0)	Yes	7	?????-1???? 0 ?? 1:??????  2:????	0
 CREATE_BY	NUMBER(15,0)	Yes	8	???	118460
 CREATE_TIME	NUMBER(15,0)	Yes	9	????	1331005758260
 UPDATE_BY	NUMBER(15,0)	Yes	10	???	118460
 UPDATE_TIME	NUMBER(15,0)	Yes	11	????	1331005774276
 SEQ	VARCHAR2(100 BYTE)	Yes	12	SEQ????????	118460_1331005758_461
 PARENT_CODE	VARCHAR2(20 BYTE)	Yes	13	????????	gaojing

 "CAITS 153916_1352187820_1002 E001_15255554455 0 D_GETP {TYPE:0}",
 "CAITR 153916_1352187817_1001 E001_15255554455 0 D_GETP {TYPE:0,RET:0,"
 "0:M2M.YUTONG.COM,1:7709,10:13559963589,100:30,101:3,102:30,103:3,104:30,"
 "105:3,106:ZZYTBJ.HA,107:WAP,108:WAP,109:58.83.210.8,110:7709,111:0,112:0,"
 "113:30,114:1800,115:10,116:10,117:500,118:1000,119:1000,120:100,121:45,"
 "122:13559963588,123:13559963500,124:13559963544,125:1,126:600,127:18000,"
 "128:100,129:10,130:14400,131:57600,132:1200,133:0,134:0,135:0,136:5,137:70,"
 "138:50,139:70,140:70,141:,142:0,143:1,144:0,145:0,146:7,147:0,15:13559963533,"
 "180:0,181:0,187:0,3:ZZYTBJ.HA,300:100,301:1800,302:0,303:8,"
 "304:400,305:1|3|5|7|9|11|13|15|17|19|21|23|25|27|29|31|33|35|37|39|41|43,"
 "306:1|3|5|7|9|11|13|15|17|19|21|23|25|27|29|31,307:1|3|5|7|9|11|13|16|18|20|22|23|25|27|29|1|3|5|7|9|11|13|16|18|20|22|23|25|27|29,"
 "308:2,31:0,310:,4:WAP,41:��C55554,42:2,5:WAP,7:30,9:13999655668} \r\n",

 */
bool OracleSave::oracle_terminal_history_param(InterProto *inter_proto)
{
	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	string user_id = get_user_from_seq(inter_proto->meta_data->_seqid);
    sql_obj->AddInteger("AUTO_ID", 0);
	sql_obj->AddLongLong("MODIFY_DATE", time(0) * 1000);
	sql_obj->AddInteger("TID", inter_proto->vehicle_info.vid);
//    sql_obj->AddInteger("T_MAC", 0);

//  ÿһ����������һ��,�ܸ���
//  sql_obj->AddString("PARAM_TYPE", 0);
//  sql_obj->AddString("PARAM_VALUE", 0);
	sql_obj->AddInteger("SET_RESULT", 0);
	sql_obj->AddInteger("CREATE_BY", atoi(user_id.c_str()));
	sql_obj->AddInteger("CREATE_TIME", time(0) * 1000);
	sql_obj->AddInteger("UPDATE_BY", atoi(user_id.c_str()));
	sql_obj->AddInteger("UPDATE_TIME", time(0) * 1000);
	sql_obj->AddString("SEQ", inter_proto->meta_data->_seqid);
//    sql_obj->AddString("PARENT_CODE", user_id);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TB_TERMINAL_HISTORY_PARAM, sql_obj);

	return true;
}

/****************************************************
 PID	VARCHAR2(32 BYTE)	No	1	UUID	2897
 CLASS_LINE_ID	NUMBER(15,0)	Yes	2	????(TB_CLASS_LINE)??	NULL
 VID	NUMBER(15,0)	Yes	3	????ID	11845
 SEND_COMMAND_STATUS	NUMBER	Yes	4	????????	-1
 SEQ	VARCHAR2(32 BYTE)	Yes	5	????SEQ	NULL
 CREATE_TIME	NUMBER(15,0)	Yes	6	????	NULL
 UPDATE_TIME	NUMBER(15,0)	Yes	7	????	NULL
 LINE_STATUS	NUMBER	Yes	8	???????????1??2??3??	NULL
 JUDGMENT	NUMBER(2,0)	Yes	9	�ж�����:0ƽ̨�ж� 1�����ж�	NULL
 USETYPE	VARCHAR2(50 BYTE)	Yes	10	ҵ������,1-��ʱ,2-����,3-�������ж�,4-���������ն�,5-�������ж�,6-���������ն� 7��������ƽ̨8��������ƽ̨	NULL
 LINE_BEGINTIME	NUMBER(15,0)	Yes	11	������Ч��ʼʱ��㣬���賿0:00�ķ�����	NULL
 LINE_ENDTIME	NUMBER(15,0)	Yes	12	������Ч����ʱ��㣬���賿0:00�ķ�����������	NULL
 PERIOD_BEGINTIME	VARCHAR2(8 BYTE)	Yes	13	��ʼʱ�����ڣ�hh:mm:ss��	NULL
 PERIOD_ENDTIME	VARCHAR2(8 BYTE)	Yes	14	����ʱ�����ڣ�hh:mm:ss��	NULL
 ��·Χ��

 155	"�������ͣ�ȡֵ��Χ��0|1|2|3����
0����ʾɾ��ȫ����·
1��ɾ��ָ��ID����·
2��������·
3��׷����·
4���޸���·"
156	"{TYPE:15,156:[��·����1][��·����2][��·����N]}
·�������ṹ��
[
1:·��ID��
2:·�����ԣ�
3:��ʼʱ�䣬
4:����ʱ�䣬
5:·�������ṹ��
��1=�յ�ID|2=·��ID|3=�յ�γ��|4=�յ㾫��|5=·�ο��|6=·������|7=·����ʻ������ֵ|8=·����ʻ���̷�ֵ|
9=·������ٶ�|10=·�γ��ٳ���ʱ�䣩��·��2����·��3����·��4��
]"
	·������  			·������
	B0	1������ʱ��		B0	1����ʻʱ��
	B1	����		B1	1������
	B2	1����·�߱�������ʻԱ		B2	0����γ��1����γ
	B3	1����·�߱�����ƽ̨		B3	0��������1������
	B4	1����·�߱�������ʻԱ		B4-B31	����
	B5	1����·�߱�����ƽ̨
	B6-B31	����
 *****************************************************/

bool OracleSave::oracle_line_vehicle(InterProto *inter_proto)
{
    // printf("OracleSave::oracle_line_vehicle \n");

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("CLASS_LINE_ID", 0);
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddInteger("SEND_COMMAND_STATUS", 0);
	sql_obj->AddString("SEQ", inter_proto->meta_data->_seqid);
	sql_obj->AddInteger("CREATE_TIME", time(0) * 1000);
	sql_obj->AddInteger("UPDATE_TIME", time(0) * 1000);
	sql_obj->AddInteger("LINE_STATUS", 0);
	sql_obj->AddInteger("JUDGMENT", 0);
	sql_obj->AddString("USETYPE", 0);
	//������ʱ����
	sql_obj->AddInteger("LINE_BEGINTIME", 0);
	sql_obj->AddInteger("LINE_ENDTIME", 0);
	sql_obj->AddString("PERIOD_BEGINTIME", 0);
	sql_obj->AddString("PERIOD_ENDTIME", 0);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TR_LINE_VEHICLE, sql_obj);

	return true;
}

/**
 ID	NUMBER(15,0)	No	1	???	8840186
 AREA_ID	NUMBER(15,0)	No	2	??ID	8840117
 VID	NUMBER(15,0)	No	3	??ID	140169
 UTC	NUMBER(15,0)	Yes	4	????utc	NULL
 AREA_BEGINTIME	NUMBER(15,0)	Yes	5	?????????????0:00????	1332340597000
 AREA_ENDTIME	NUMBER(15,0)	Yes	6	?????????????0:00???????	1395412599000
 AREA_USETYPE	VARCHAR2(50 BYTE)	Yes	7	????,1-??,2-??,3-??????,4-???????,5-??????,6-????????????????	2
 AREA_UPDATETIME	NUMBER(15,0)	Yes	8	????utc	1332340633396
 AREA_ENABLE	NUMBER(2,0)	Yes	9	????1-?? 0-???	0
 AREA_DECIDE	NUMBER(1,0)	Yes	10	1???? 2????	1
 SEQ	VARCHAR2(50 BYTE)	No	11	?????	test
 AREA_STATUS	NUMBER(1,0)	Yes	12	??1??2??3??	3
 SEND_STATUS	NUMBER(1,0)	Yes	13	"????
 -1????
 0:??"	-1
 SEND_UTC	NUMBER(15,0)	Yes	14	????	1332340633396
 RECEIVE_UTC	NUMBER(15,0)	Yes	15	????	NULL
 PERIOD_BEGINTIME	VARCHAR2(8 BYTE)	Yes	16	��ʼʱ�����ڣ�hh:mm:ss��	NULL
 PERIOD_ENDTIME	VARCHAR2(8 BYTE)	Yes	17	����ʱ�����ڣ�hh:mm:ss��	NULL

 ����Χ��
 */
bool OracleSave::oracle_vehicle_area(InterProto *inter_proto)
{
	return true;

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddInteger("ID", 0);
	sql_obj->AddInteger("AREA_ID", 0);
	sql_obj->AddInteger("VID", 0);
	sql_obj->AddInteger("UTC", 0);
	sql_obj->AddInteger("AREA_BEGINTIME", 0);
	sql_obj->AddInteger("AREA_ENDTIME", 0);
	sql_obj->AddString("AREA_USETYPE", 0);
	sql_obj->AddInteger("AREA_UPDATETIME", 0);
	sql_obj->AddInteger("AREA_ENABLE", 0);
	sql_obj->AddInteger("AREA_DECIDE", 0);
	sql_obj->AddString("SEQ", 0);
	sql_obj->AddInteger("AREA_STATUS", 0);
	sql_obj->AddInteger("SEND_STATUS", 0);
	sql_obj->AddInteger("SEND_UTC", 0);
	sql_obj->AddInteger("RECEIVE_UTC", 0);
	sql_obj->AddString("PERIOD_BEGINTIME", 0);
	sql_obj->AddString("PERIOD_ENDTIME", 0);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TR_VEHICLE_AREA, sql_obj);


	return true;
}

/****
 TID	NUMBER(15,0)	No	1	??ID	10178
 PARAM_ID	NUMBER(15,0)	No	2	????ID	41
 T_MAC	VARCHAR2(40 BYTE)	Yes	3	?????	NULL
 PARAM_TYPE	VARCHAR2(20 BYTE)	Yes	4	???? ?????????	NULL
 PARAM_VALUE	VARCHAR2(100 BYTE)	Yes	5	???	��N20622
 CREATE_BY	NUMBER(15,0)	Yes	6	???	NULL
 CREATE_TIME	NUMBER(15,0)	Yes	7	????	NULL
 UPDATE_BY	NUMBER(15,0)	Yes	8	???	NULL
 UPDATE_TIME	NUMBER(15,0)	Yes	9	????	NULL
 SEQ	VARCHAR2(100 BYTE)	Yes	10	SEQ????????	NULL
 PARENT_CODE	VARCHAR2(20 BYTE)	Yes	11	????????	NULL
 */
bool OracleSave::oracle_terminal_param(InterProto *inter_proto)
{
	return true;

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddInteger("TID", 0);
	sql_obj->AddInteger("PARAM_ID", 0);
	sql_obj->AddString("T_MAC", 0);
	sql_obj->AddString("PARAM_TYPE", 0);
	sql_obj->AddString("PARAM_VALUE", 0);
	sql_obj->AddInteger("CREATE_BY", 0);
	sql_obj->AddInteger("CREATE_TIME", 0);
	sql_obj->AddInteger("UPDATE_BY", 0);
	sql_obj->AddInteger("UPDATE_TIME", 0);
	sql_obj->AddString("SEQ", 0);
	sql_obj->AddString("PARENT_CODE", 0);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TB_TERMINAL_PARAM, sql_obj);

	return true;
}

/****
 INFO_ID	NUMBER(15,0)	No	1	??	2137
 VID	NUMBER(15,0)	No	2	??VID	819
 VEHICLE_NO	VARCHAR2(40 BYTE)	No	3	???	��N07015
 OEM_CODE	VARCHAR2(20 BYTE)	No	4	???ID(?????????????)	E001
 HARDWARE_VERSION	VARCHAR2(40 BYTE)	Yes	5	?????	1.02
 OLD_HARDWARE_VERSION	VARCHAR2(40 BYTE)	Yes	6	??????	NULL
 FIRMWARE_VERSION	VARCHAR2(40 BYTE)	Yes	7	????	1.00.20120121.025800-patch
 OLD_FIRMWARE_VERSION	VARCHAR2(40 BYTE)	Yes	8	??????	NULL
 CONNECT_TIMES	NUMBER(5,0)	No	9	???????????	10
 URL_ADDRESS	VARCHAR2(100 BYTE)	Yes	10	URL??	192.168.111.106
 DIAL_NAME	VARCHAR2(40 BYTE)	Yes	11	?????	192.168.111.106
 DIAL_USER	VARCHAR2(40 BYTE)	Yes	12	??????	ftpuser
 DIAL_PASSWORD	VARCHAR2(40 BYTE)	Yes	13	????	ftpuser
 IP	VARCHAR2(40 BYTE)	Yes	14	IP??	192.168.111.106
 TCP_PORT	NUMBER(10,0)	Yes	15	TCP??	21
 UDP_PORT	NUMBER(10,0)	Yes	16	UDP??	21
 CREATE_BY	NUMBER(15,0)	No	17	???id	10024
 CREATE_NAME	VARCHAR2(40 BYTE)	No	18	?????	����
 CREATE_TIME	NUMBER(15,0)	No	19	????	1330306562468
 SEND_FLAG	NUMBER(2,0)	No	20	?????0-?????1-?????	0
 FINISH_TIME	NUMBER(15,0)	Yes	21	??????	NULL
 FINSIH_FLAG	NUMBER(2,0)	No	22	???????-1????0???1?? ?	-1
 COMMADDR	VARCHAR2(100 BYTE)	Yes	23	????	NULL
 ********/
bool OracleSave::oracle_terminal_updateinfo(InterProto *inter_proto)
{
    return true;

	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddInteger("INFO_ID", 0);
	sql_obj->AddInteger("VID", 0);
	sql_obj->AddString("VEHICLE_NO", 0);
	sql_obj->AddString("OEM_CODE", 0);
	sql_obj->AddString("HARDWARE_VERSION", 0);
	sql_obj->AddString("OLD_HARDWARE_VERSION", 0);
	sql_obj->AddString("FIRMWARE_VERSION", 0);
	sql_obj->AddString("OLD_FIRMWARE_VERSION", 0);
	sql_obj->AddInteger("CONNECT_TIMES", 0);
	sql_obj->AddString("URL_ADDRESS", 0);
	sql_obj->AddString("DIAL_NAME", 0);
	sql_obj->AddString("DIAL_USER", 0);
	sql_obj->AddString("DIAL_PASSWORD", 0);
	sql_obj->AddString("IP", 0);
	sql_obj->AddInteger("TCP_PORT", 0);
	sql_obj->AddInteger("UDP_PORT", 0);
	sql_obj->AddInteger("CREATE_BY", 0);
	sql_obj->AddString("CREATE_NAME", 0);
	sql_obj->AddInteger("CREATE_TIME", 0);
	sql_obj->AddInteger("SEND_FLAG", 0);
	sql_obj->AddInteger("FINISH_TIME", 0);
	sql_obj->AddInteger("FINSIH_FLAG", 0);
	sql_obj->AddString("COMMADDR", 0);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TB_TERMINAL_UPDATEINFO, sql_obj);

	return true;
}

bool OracleSave::oracle_vehicle_media_idx(InterProto *inter_proto)
{
	return true;
}

/*
PID	VARCHAR2(64 BYTE)	No		1	��Ψһ��ʶ
VID	NUMBER(15,0)	No		2	������ʶ
UTC	NUMBER(15,0)	Yes		3	͸��ʱ��
CONTENT	VARCHAR2(1024 BYTE)	Yes	4	͸������
TYPE	NUMBER(1,0)	Yes	0	5	͸�����ͣ�0������͸����1������͸��
MSGTYPE	NUMBER(5,0)	Yes		6	��Ϣ����


 */
bool OracleSave::oracle_vehicle_bridge(InterProto *inter_proto)
{
	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddInteger("UTC", time(0) * 1000);
	sql_obj->AddInteger("TYPE", atoi(inter_proto->kvmap["91"].c_str()));
	sql_obj->AddString("CONTENT", inter_proto->kvmap["90"]);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_BRIDGE, sql_obj);

	return true;
}

/*
PID	VARCHAR2(64 BYTE)	No	        1	����
VID	NUMBER(15,0)	Yes		        2	������ʶ
UTC	NUMBER(15,0)	Yes		        3	�ϴ�ʱ��
CONTENT	VARCHAR2(1024 BYTE)	Yes		4           ѹ����Ϣ��
 "CAITS 0_0 E013_14784324206 0 U_REPT {TYPE:14,90:1, 92:Q00tMTBB, 93:8} \r\n",
 */
bool OracleSave::oracle_vehicle_compress(InterProto *inter_proto)
{
	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddVar("PID", "sys_guid()");
	sql_obj->AddInteger("VID", inter_proto->vehicle_info.vid);
	sql_obj->AddInteger("UTC", time(0) * 1000);
	sql_obj->AddString("CONTENT", inter_proto->kvmap["92"]);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::insert, TH_VEHICLE_BRIDGE, sql_obj);
	return true;
}

/*
 AUTO_ID	NUMBER(15,0)	No	1	��������������
 VID	NUMBER(15,0)	No	2	����ID
 DEVICE_NO	VARCHAR2(20 BYTE)	Yes	3	�ֻ�����
 STAFF_ID	NUMBER(15,0)	Yes	4	��ʻԱ���
 STAFF_NAME	VARCHAR2(40 BYTE)	Yes	5	��ʻԱ����
 ASK_UTC	NUMBER(15,0)	Yes	6	����ʱ��UTC
 TTYPE_CODE	VARCHAR2(20 BYTE)	Yes	7	�����־ �μ��ı���Ϣ����
 QUESTION_CONTENT	VARCHAR2(200 BYTE)	Yes	8	��������
 CANDIDATE_ANSWER	VARCHAR2(1000 BYTE)	Yes	9	��ѡ�𰸣����� ��ID1��������1����ID2��������2����ʽ�洢��
 REPLY_UTC	NUMBER(15,0)	Yes	10	�ش�ʱ��
 ANSWER_CONTENT	VARCHAR2(20 BYTE)	Yes	11	�ն˴�
 QUESTION_RESULT	NUMBER(2,0)	Yes	12	���ʽ����0 ��ȷ 1 ���� 2�ش��У�
 MAC_ID	VARCHAR2(10 BYTE)	Yes	13
 SEQ	VARCHAR2(64 BYTE)	Yes	14	ҵ��Ψһ��ʶ
 OP_ID	VARCHAR2(32 BYTE)	Yes	15	����Ա

 //�����·�
 "CAITS 10206_1352187406_24719 E001_15249677400 0 D_SNDM {TYPE:1,2:4+TB6rfWuavLvszh0NHE+qOsxPrS0bOsy9nQ0Mq7o6zH67z1y9nC/dDQo6E=,1:2} \r\n",

 //����Ӧ��
 "CAITS 10206_1352187406_24719 E001_15249677400 0 U_REPT {TYPE:32,82:id,84:seq} \r\n"
 UPDATE TH_QUESTION_ANSWER T SET T.REPLY_UTC = ?, T.ANSWER_CONTENT = ? WHERE T.SEQ = ?

 */
bool OracleSave::oracle_question_answer(InterProto *inter_proto)
{
	CSqlObj *sql_obj = IDataPool::GetSqlObj(IDataPool::oracle);
	sql_obj->AddInteger("REPLY_UTC", time(0) * 1000);
	sql_obj->AddString("ANSWER_CONTENT", inter_proto->kvmap["85"]);

	CSqlWhere *where = new CSqlWhere;
	where->AddWhere("SEQ", inter_proto->kvmap["84"]);

	_inter2save->add_pool(IDataPool::oracle, IDataPool::update, TH_QUESTION_ANSWER, sql_obj, where);


//	CSqlWhere *where = IDataPool::GetSqlObj()
    //�洢����ֻ�����²���
//    sql_obj->AddInteger("AUTO_ID", 0);
//    sql_obj->AddInteger("VID", inter_proto->vid);
//    sql_obj->AddString("DEVICE_NO", inter_proto->vehicle_number);
//    sql_obj->AddInteger("STAFF_ID", 0);
//    sql_obj->AddString("STAFF_NAME", 0);
//    sql_obj->AddInteger("ASK_UTC", 0);
//    sql_obj->AddString("TTYPE_CODE", 0);
//    sql_obj->AddString("QUESTION_CONTENT", 0);
//    sql_obj->AddString("CANDIDATE_ANSWER", 0);
//    sql_obj->AddInteger("REPLY_UTC", 0);
//    sql_obj->AddString("ANSWER_CONTENT", 0);
//    sql_obj->AddInteger("QUESTION_RESULT", 0);
//    sql_obj->AddString("MAC_ID", 0);
//    sql_obj->AddInteger("SEQ", 0);
//    sql_obj->AddString("OP_ID", 0);
	return true;
}

