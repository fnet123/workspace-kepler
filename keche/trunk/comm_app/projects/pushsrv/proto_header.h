/******************************************************
 *  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
 *   FileName: proto_header.h
 *     Author: liubo  2012-5-14
 *Description:
 *******************************************************/

#ifndef PROTOCOLSTRUCT_H_
#define PROTOCOLSTRUCT_H_

#define  MSG_RESULT_OK               0x00 //�ɹ�
#define  MSG_RESULT_ERR              0x01 //ʧ��
#define  MSG_VER                     0x01 //�汾
//�ͻ�������ָ��
#define  MSG_REQUEST_LOGIN                   0x0001 //��¼
#define  MSG_REQUEST_NOOP                    0x0002 //��·����
#define  MSG_REQUEST_SUBSCRIBE_INFO          0x0003 //��������
#define  MSG_REQUEST_LOCATION_INFO_SERVICE   0x0004 //λ���ϱ���Ϣ
#define  MSG_REQUEST_ONLINE                  0x0005
#define  MSG_REQUEST_LOCATION                0x0006
#define  MSG_REQUEST_TRACE                   0x0007
#define  MSG_REQUEST_TEXT                    0x0008
#define  MSG_REQUEST_MONITOR                 0x0009
#define  MSG_REQUEST_MEDIA_UP                0x000a
#define  MSG_REQUEST_GET_PHOTO               0x000b
#define  MSG_REQUEST_RECORD                  0x000c
#define  MSG_REQUEST_SET_PARAM               0x000d
#define  MSG_REQUEST_GET_PARAM               0x000e
#define  MSG_REQUEST_TERM_CTRL               0x000f
#define  MSG_REQUEST_SET_EVENT               0x0010
#define  MSG_REQUEST_EVENT_REPORT            0x0011
#define  MSG_REQUEST_QUESTION_ASK            0x0012
#define  MSG_REQUEST_QUESTION_ACK            0x0013
#define  MSG_REQUEST_INFO_MENU               0x0014
#define  MSG_REQUEST_INFO_RESP               0x0015
#define  MSG_REQUEST_INFO_SEND               0x0016
#define  MSG_REQUEST_PHONE_BOOK              0x0017
#define  MSG_REQUEST_CAR_CONTROL             0x0018
#define  MSG_REQUEST_CARCTRL_RESP            0x0019
#define  MSG_REQUEST_SET_CIRCLE              0x001a
#define  MSG_REQUEST_DEL_CIRCLE              0x001b
#define  MSG_REQUEST_SET_RECTANGLE           0x001c
#define  MSG_REQUEST_DEL_RECTANGLE           0x001d
#define  MSG_REQUEST_SET_POLYGON             0x001e
#define  MSG_REQUEST_DEL_POLYGON             0x001f
#define  MSG_REQUEST_SET_LINE                0x0020
#define  MSG_REQUEST_DEL_LINE                0x0021
#define  MSG_REQUEST_DRIVE_COLLECT           0x0022
#define  MSG_REQUEST_DRIVE_PARAM             0x0023
#define  MSG_REQUEST_LIST_REPORT             0x0024
#define  MSG_REQUEST_IDENTITY_COLLECT        0x0025
#define  MSG_REQUEST_MEDIA_EVENT             0x0026
#define  MSG_REQUEST_MEDIA_SEARCH            0x0027
#define  MSG_REQUEST_SINGLE_UPLOAD           0x0028
#define  MSG_REQUEST_MULTI_UPLOAD            0x0029
#define  MSG_REQUEST_TRANS_DELIVER           0x002a
#define  MSG_REQUEST_SUBSCRIBE_APPEND        0x0030
#define  MSG_REQUEST_SUBSCRIBE_ERASE         0x0031

//ʵʱ����--�ڲ�Э��
#define  LOCATION_INFO_SERVICE       "0"

enum RESP_MSG_TYPE
{

};

#pragma pack(1)

enum MSG_TYPE
{
	gps_info, alert, resp
};

const char HeaderTag = 0x5b;
const char EndTag = 0x5d;

typedef struct _tagProtoHeader
{
	unsigned char tag;  // ���ͷ����ʶ 0x5b
	unsigned short len; // ���ݳ��ȣ�����������ͷ��β
	unsigned char flag; // �Ƿ���ܱ�ʶ
	unsigned char type; // Э������

	_tagProtoHeader()
	{
		tag = HeaderTag;
		len = 0;
		flag = 0;
		type = 0x01; //�µ���Ϣ����Ĭ��Ϊ��0x01;
	}
} PROTO_HEADER;

/*������Ϣͷ*/
typedef struct _COMM_HEADER
{
	unsigned char msg_version; //��Ϣ�汾��
	unsigned short msg_type; //��Ϣ����
	unsigned short msg_len; //��Ϣ����
	unsigned int msg_seq; //��Ϣ���
} COMM_HEADER;

typedef struct _STRING {
	unsigned char len;
	char msg[0];
} STRING;

typedef struct _STRING4 {
	unsigned int len;
	char msg[0];
} STRING4;

//��¼-------------------------------------------------------------------------------
/*Request ��¼��Ϣ*/
typedef struct _LOGIN_DATA
{
    char compony_code[32];
    char user_name[16];
} LOGIN_DATA;


typedef struct _RESP_LOGIN
{
    unsigned char result;
}RESP_LOGIN;

/*Response ��¼�ظ�*/
typedef struct _COMM_RESPONSE
{
	unsigned int resp_seq;
	unsigned char result; //�û��ظ�
} COMM_RESPONSE;

//����-------------------------------------------------------------------------------
typedef struct _MAC_LIST
{
	char oem_code[6]; //oemcode;
	char phone[6]; //�����ֻ���(BCD)
} MAC_LIST;

/*Request ������Ϣ*/
typedef struct _SUBSCRIBE_DATA
{
	unsigned short subs_num; //���ĳ����б�
	MAC_LIST mac_list;
} SUBSCRIBE_DATA;

typedef struct _CAR_ONLINE
{
	MAC_LIST mac_list;
    unsigned char online;
} CAR_ONLINE;

typedef struct LOCATION_QUERY {
	MAC_LIST mac_list;
} LOCATION_QUERY;

//λ����Ϣ-------------------------------------------------------------------------------
typedef struct SPEED_ALARM {
	unsigned char type;
	unsigned int name;
} SPEED_ALARM;

typedef struct AREA_ALARM {
	unsigned char type;
	unsigned int name;
	unsigned char flag;
} AREA_ALARM;

typedef struct ROAD_ALARM {
	unsigned int name;
	unsigned short time;
	unsigned char flag;
} ROAD_ALARM;

/*Response λ����Ϣ����*/
typedef struct _LOCATION_INFO
{
	MAC_LIST mac_list;
	char alarm_id[32];
    unsigned char alarm_flag;
	unsigned char dwAlerm; //������־λ
	unsigned int dwState; //״̬��־λ
	unsigned int dwLon; //����
	unsigned int dwLat; //γ��
	unsigned short wHight; //�߳�
	unsigned short wSpeed; //�ٶ�
	unsigned short wDirection; //����
	unsigned char byTime[6]; //BCD ʱ��
	unsigned char ext[128];
} LOCATION_INFO;

typedef struct _LOCATION_RESP
{
    unsigned int msg_seq;
    LOCATION_INFO location_info;
}LOCATION_RESP;

//����

//λ�ø���
typedef struct _LOCATION_TRACE
{
	MAC_LIST mac_list;
    unsigned short interval;
    unsigned int valid_time;
}LOCATION_TRACE;

typedef struct _DOWN_TEXT
{
	MAC_LIST mac_list;
    unsigned char flag;
    char text[0];                //����ʱȡ���ַ
}DOWN_TEXT;

typedef struct _PHONE_MONITOR
{
	MAC_LIST mac_list;
    unsigned char flag;
    char phone[20];
}PHONE_MONITOR;

typedef struct _UP_MEDIA
{
	MAC_LIST mac_list;
    unsigned char type;
    unsigned char format;
    unsigned char event;
    char url[512]; //����ʱȡ��ַ
}UP_MEDIA;

typedef struct _REQUEST_PHOTO
{
	MAC_LIST mac_list;
    unsigned char channel_id;
    unsigned short command;
    unsigned short interval;
    unsigned char save_flag;
    unsigned char sens;
    unsigned char quality;

    unsigned char brightness; //����
    unsigned char contrast;   //�Աȶ�
    unsigned char saturation; //���Ͷ�
    unsigned char chroma;     //ɫ��
}REQUEST_PHOTO;

//¼��ָ��
typedef struct _RECORD
{
	MAC_LIST mac_list;
    unsigned char command;     //0��ֹͣ¼����0x01����ʼ¼����
    unsigned short time;       //��λΪ�루s����0��ʾһֱ¼��
    unsigned char save_flag;   //0��ʵʱ�ϴ���1������
    unsigned char sampling_rate;
}RECORD;

//����ѡ��
typedef struct _PARAM_ITEM {
	unsigned int type;
	unsigned char size;
	char date[0];
} PARAM_ITEM;

//���ò���
typedef struct _SET_PARAM {
	MAC_LIST macid;
	unsigned char num;
} SET_PARAM;

//��ȡ����
typedef struct _GET_PARAM {
	MAC_LIST macid;
} GET_PARAM;

typedef struct _GET_PARAM_RESP {
	MAC_LIST macid;
	unsigned short seq;
	unsigned char num;
} GET_PARAM_RESP;

typedef struct _TERM_CONTROL {
	MAC_LIST macid;
	unsigned char cmd;
	char msg[0];
} TERM_CONTROL;

typedef struct _EVENT_ITEM {
	unsigned char id;
	unsigned char len;
	char buf[0];
} EVENT_ITEM;

typedef struct _SET_EVENT {
	MAC_LIST macid;
	unsigned char cmd;
	unsigned char num;
} SET_EVENT;

typedef struct _EVENT_REPORT {
	MAC_LIST macid;
	unsigned char id;
} EVENT_REPORT;

typedef struct _QUESTION_ITEM {
	unsigned char len;
	char msg[0];
} QUESTION_ITEM;

typedef struct _ANSWER_ITME {
	unsigned char id;
	unsigned short len;
	char msg[0];
} ANSWER_ITME;

typedef struct _QUESTION_ASK {
	MAC_LIST macid;
	unsigned char attr;
} QUESTION_ASK;

typedef struct _QUESTION_ACK {
	MAC_LIST macid;
	unsigned short seq;
	unsigned char res;
} QUESTION_ACK;

typedef struct _MENU_ITEM {
	unsigned char type;
	unsigned short len;
	char msg[0];
} MENU_ITEM;

typedef struct _INFO_MENU {
	MAC_LIST macid;
	unsigned char cmd;
	unsigned char num;
} INFO_MENU;

typedef struct _INOF_RESP {
	MAC_LIST macid;
	unsigned char type;
	unsigned char flag;
} INFO_RESP;

typedef struct _INFO_SEND {
	MAC_LIST macid;
	unsigned char type;
	unsigned short len;
	char msg[0];
} INFO_SEND;

typedef struct _PHONE_BOOK {
	MAC_LIST macid;
	unsigned char cmd;
	unsigned char num;
} PHONE_BOOK;

typedef struct _CAR_CONTROL {
	MAC_LIST macid;
	unsigned char flag;
} CAR_CONTROL;

typedef struct _TIME_SEGMENT {
	unsigned char bt[6];
	unsigned char et[6];
} TIME_SEGMENT;

typedef struct _TIME_LIMIT {
	unsigned short max;
	unsigned short min;
} TIME_LIMIT;

typedef struct _SPEED_LIMIT {
	unsigned short ts;
    unsigned char ht;
} SPEED_LIMIT;

typedef struct _DEL_GRAPHID {
	MAC_LIST macid;
	unsigned char num;
    unsigned int areaid[0];
} DEL_GRAPHID;

typedef struct _CIRCLE_ITEM {
	unsigned int areaid;
	unsigned short areaattr;
    unsigned int lat;
    unsigned int lon;
    unsigned int rad;
} CIRCLE_ITEM;

typedef struct _SET_CIRCLE {
	MAC_LIST macid;
	unsigned char cmd;
	unsigned char num;
} SET_CIRCLE;

typedef struct _RECTANGLE_ITEM {
	unsigned int areaid;
	unsigned short areaattr;
    unsigned int leftlat;
    unsigned int leftlon;
    unsigned int rightlat;
    unsigned int rightlon;
} RECTANGLE_ITEM;

typedef struct _SET_RECTANGLE {
	MAC_LIST macid;
	unsigned char cmd;
	unsigned char num;
} SET_RECTANGLE;

typedef struct _POLYGON_ITEM {
    unsigned int lat;
    unsigned int lon;
} POLYGON_ITEM;

typedef struct _SET_POLYGON {
	MAC_LIST macid;
	unsigned int areaid;
	unsigned short areaattr;
} SET_POLYGON;

typedef struct _LINE_ITEM {
	unsigned int pointid;
	unsigned int roadid;
    unsigned int lat;
    unsigned int lon;
    unsigned char width;
    unsigned char attr;
} LINE_ITEM;

typedef struct _SET_LINE {
	MAC_LIST macid;
	unsigned int areaid;
	unsigned short attr;
} SET_LINE;

typedef struct _DRIVE_COLLECT {
	MAC_LIST macid;
	unsigned char cmd;
} DRIVE_COLLECT;

typedef struct _DRIVE_PARAM {
	MAC_LIST macid;
	unsigned char cmd;
	char data[0];
} DRIVE_PARAM;

typedef struct _MEDIA_EVENT {
	MAC_LIST macid;
	unsigned int id;
	unsigned char type;
	unsigned char code;
	unsigned char event;
	unsigned char channel;
} MEDIA_EVENT;

typedef struct _MEDIA_SEARCH {
	MAC_LIST macid;
	unsigned char mediatype;
	unsigned char channleid;
	unsigned char eventtype;
	unsigned char begintime[6];
	unsigned char endtime[6];
} MEDIA_SEARCH;

typedef struct _SEARCH_ITEM {
	unsigned int id;
	unsigned char type;
	unsigned char chnnel;
	unsigned char event;
	unsigned int alerm;
	unsigned int state;
	unsigned int lat;
	unsigned int lon;
	unsigned short hight;
	unsigned short speed;
	unsigned short direction;
	unsigned char time[6];
} SEARCH_ITEM;

typedef struct _SEARCH_RESP {
	MAC_LIST macid;
	unsigned short seq;
	unsigned short num;
	SEARCH_ITEM item[0];
} SEARCH_RESP;

typedef struct _SINGLE_UPLOAD {
	MAC_LIST macid;
	unsigned int id;
	unsigned char flag;
} SINGLE_UPLOAD;

typedef struct _MULTI_UPLOAD {
	MAC_LIST macid;
	unsigned char mediatype;
	unsigned char channleid;
	unsigned char eventtype;
	unsigned char begintime[6];
	unsigned char endtime[6];
	unsigned char flag;
} MULTI_UPLOAD;

typedef struct _TRANS_DELIVER {
	MAC_LIST macid;
	unsigned char type;
	STRING data;
} TRANS_DELIVER;

#pragma pack()

#endif /* PROTOCOLSTRUCT_H_ */
