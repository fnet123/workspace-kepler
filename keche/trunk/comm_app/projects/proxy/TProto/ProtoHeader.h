/**********************************************
 * ProtoParse.h
 *
 *  Created on: 2010-7-8
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *    	<2011-10-14 humingqing> �޸������ֽ����ת�����Դ�˺�С�˻�����֧��
 *********************************************/

#ifndef __PROTOHEADER_H__
#define __PROTOHEADER_H__

#include <string.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <tools.h>
#include <netorder.h>

#define COMPANY

#pragma pack(1)  //1�ֽڶ���

#define ntouv16( x )  htons( x )
#define ntouv32( x )  htonl( x )
#define ntouv64( x )  htonll( x )

#define UP_CONNECT_REQ				0x1001
#define UP_CONNECT_RSP				0x1002
#define UP_DISCONNECT_REQ			0x1003
#define UP_DISCONNECT_RSP			0x1004
#define UP_LINKTEST_REQ				0x1005
#define UP_LINKTEST_RSP				0x1006
#define UP_DISCONNECT_INFORM		0x1007
#define UP_CLOSELINK_INFORM			0x1008
#define DOWN_CONNECT_REQ			0x9001
#define DOWN_CONNECT_RSP			0x9002
#define DOWN_DISCONNECT_REQ			0x9003
#define DOWN_DISCONNECT_RSP			0x9004
#define DOWN_LINKTEST_REQ 			0x9005
#define DOWN_LINKTEST_RSP			0x9006
#define DOWN_DISCONNECT_INFORM		0x9007
#define DOWN_CLOSELINK_INFORM		0x9008
#define DOWN_TOTAL_RECV_BACK_MSG	0x9101
#define UP_EXG_MSG					0x1200
#define DOWN_EXG_MSG				0x9200
#define UP_PLATFORM_MSG				0x1300
#define DOWN_PLATFORM_MSG			0x9300
#define UP_WARN_MSG					0x1400
#define DOWN_WARN_MSG				0x9400
#define UP_CTRL_MSG					0x1500
#define DOWN_CTRL_MSG				0x9500
#define UP_BASE_MSG					0x1600
#define DOWN_BASE_MSG				0x9600

#define UP_EXG_MSG_REGISTER		    				0x1201
#define UP_EXG_MSG_REAL_LOCATION					0x1202
#define UP_EXG_MSG_HISTORY_LOCATION					0x1203
#define UP_EXG_MSG_RETURN_STARTUP_ACK				0x1205
#define UP_EXG_MSG_ARCOSSAREA_STARTUP_ACK			0x1205   // ��ǰ�������
#define UP_EXG_MSG_RETURN_END_ACK					0x1206
#define UP_EXG_MSG_ARCOSSAREA_END_ACK				0x1206   // ��ǰ�������
#define UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP		0x1207
#define UP_EXG_MSG_APPLY_FOR_MONITOR_END			0x1208
#define UP_EXG_MSG_APPLY_HISGNSSDATA_REQ			0x1209
#define UP_EXG_MSG_REPORT_DRIVER_INFO_ACK			0x120A
#define UP_EXG_MSG_TAKE_WAYBILL_ACK					0x120B
#define UP_EXG_MSG_REPORT_DRIVER_INFO				0x120C
#define UP_EXG_MSG_REPORT_EWAYBILL_INFO				0x120D
#define DOWN_EXG_MSG_CAR_LOCATION					0x9202
#define DOWN_EXG_MSG_HISTORY_ARCOSSAREA				0x9203
#define DOWN_EXG_MSG_CAR_INFO						0x9204
#define DOWN_EXG_MSG_RETURN_STARTUP					0x9205
#define DOWN_EXG_MSG_ARCOSSAREA_STARTUP				0x9205   // ��ǰ�������
#define DOWN_EXG_MSG_RETURN_END						0x9206
#define DOWN_EXG_MSG_ARCOSSAREA_END					0x9206   // ��ǰ�������
#define DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK	0x9207
#define DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK		0x9208
#define DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK			0x9209
#define DOWN_EXG_MSG_REPORT_DRIVER_INFO				0x920A
#define DOWN_EXG_MSG_TAKE_WAYBILL_REQ				0x920B
#define UP_PLATFORM_MSG_POST_QUERY_ACK				0x1301
#define UP_PLATFORM_MSG_INFO_ACK						0x1302
#define DOWN_PLATFORM_MSG_POST_QUERY_REQ			0x9301
#define DOWN_PLATFORM_MSG_INFO_REQ					0x9302
#define UP_WARN_MSG_URGE_TODO_ACK					0x1401
#define UP_WARN_MSG_ADPT_INFO						0x1402
#define UP_WARN_MSG_ADPT_TODO_INFO					0x1403
#define DOWN_WARN_MSG_URGE_TODO_REQ				0x9401
#define DOWN_WARN_MSG_INFORM_TIPS					0x9402
#define DOWN_WARN_MSG_EXG_INFORM					0x9403
#define UP_CTRL_MSG_MONITOR_VEHICLE_ACK				0x1501
#define UP_CTRL_MSG_TAKE_PHOTO_ACK					0x1502
#define UP_CTRL_MSG_TEXT_INFO_ACK					0x1503
#define UP_CTRL_MSG_TAKE_TRAVEL_ACK					0x1504
#define UP_CTRL_MSG_EMERGENCY_MONITORING_ACK		0x1505
#define DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ			0x9501
#define DOWN_CTRL_MSG_TAKE_PHOTO_REQ				0x9502
#define DOWN_CTRL_MSG_TEXT_INFO						0x9503
#define DOWN_CTRL_MSG_TAKE_TRAVEL_REQ				0x9504
#define DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ		0x9505
#define UP_BASE_MSG_VEHICLE_ADDED_ACK				0x1601
#define DOWN_BASE_MSG_VEHICLE_ADDED					0x9601

//static uint32_t msg_seq = 0;

class Begin
{
public:
	char _begin;
	Begin()
	{
		_begin = '[';
	}
};

class End
{
public:
	char _end;
	End()
	{
		_end = ']';
	}
};

class Header
{
public:
	Begin begin;
	uint32_t msg_len;
	uint32_t msg_seq;
	uint16_t msg_type;
	uint32_t access_code;
	unsigned char version_flag[3];
	unsigned char encrypt_flag;
	uint32_t encrypt_key;

	Header()
	{
		version_flag[0] = 0x02;
		version_flag[1] = 0x01;
		version_flag[2] = 0x0f;
		encrypt_flag = 0;
		encrypt_key = 0;
		msg_seq = msg_seq++;
	}
};

class Footer
{
public:
	uint16_t crc_code;
	End end;
	Footer(){
		crc_code = 0 ;
	}
};

class UpConnectReq
{
public:
	Header header;
	uint32_t user_id;
	unsigned char password[8];
	unsigned char down_link_ip[32];
	uint16_t down_link_port;
	uint16_t crc_code;
	End end;
	UpConnectReq()
	{
		header.msg_len = ntouv32(sizeof(UpConnectReq));
		header.msg_type = ntouv16(UP_CONNECT_REQ);
		header.msg_seq = 0;
		memset(password, 0, sizeof(password));
		memset(down_link_ip, 0, sizeof(down_link_ip));
		down_link_port = 0;
	}
};

class UpConnectRsp
{
public:
	Header header;
	unsigned char result;
	uint32_t verify_code;
	uint16_t crc_code;
	End end;
};

class UpDisconnectReq
{
public:
	Header header;
	uint32_t user_id;
	unsigned char password[8];
	uint16_t crc_code;
	End end;

	UpDisconnectReq()
	{
		header.msg_type = ntouv16(UP_DISCONNECT_REQ);
		header.msg_len = ntouv32(sizeof(UpDisconnectReq));
		memset( password, 0, sizeof(password) ) ;
	}
};

class UpDisconnectRsp
{
public:
	Header header;
	uint16_t crc_code;
	End end;

	UpDisconnectRsp()
	{
		header.msg_type = ntouv16(UP_DISCONNECT_RSP);
		header.msg_len = ntouv32(sizeof(UpDisconnectRsp));
	}

};

class UpLinkTestReq
{
public:
	Header header;
	uint16_t crc_code;
	End end;
	UpLinkTestReq()
	{
		header.msg_type = ntouv16(UP_LINKTEST_REQ);
		header.msg_len = ntouv32(sizeof(UpLinkTestReq));
	}
};

class UpLinkTestRsp
{
public:
	Header header;
	uint16_t crc_code;
	End end;

	UpLinkTestRsp()
	{
		header.msg_type = ntouv16(UP_LINKTEST_RSP);
		header.msg_len = ntouv32(sizeof(UpLinkTestReq));
	}
};

class UpDisconnectInform
{
public:
	Header header;
	unsigned char error_code;
	uint16_t crc_code;
	End end;
};

class UpCloselinkInform
{
public:
	Header header;
	unsigned char reason_code;
	uint16_t crc_code;
	End end;
};

class DownConnectReq
{
public:
	Header header;
	uint32_t verify_code;
	uint16_t crc_code;
	End end;
	DownConnectReq()
	{
		header.msg_type = ntouv16(DOWN_CONNECT_REQ);
		header.msg_len = ntouv32(sizeof(DownConnectReq));
	}
};

class DownConnectRsp
{
public:
	Header header;
	unsigned char result;
	uint16_t crc_code;
	End end;

	DownConnectRsp()
	{
		header.msg_type = ntouv16(DOWN_CONNECT_RSP);
		header.msg_len = ntouv32(sizeof(DownConnectRsp));
	}
};

class DownDisconnectReq
{
public:
	Header header;
	uint32_t verify_code;
	uint16_t crc_code;
	End end;
};

class DownDisconnectRsp
{
public:
	Header header;
	uint16_t crc_code;
	End end;
};

class DownLinkTestReq
{
public:
	Header header;
	uint16_t crc_code;
	End end;

	DownLinkTestReq()
	{
		header.msg_type = ntouv16(DOWN_LINKTEST_REQ);
		header.msg_len = ntouv32(sizeof(DownLinkTestReq));
	}
};

class DownLinkTestRsp
{
public:
	Header header;
	uint16_t crc_code;
	End end;

	DownLinkTestRsp()
	{
		header.msg_type = ntouv16(DOWN_LINKTEST_RSP);
		header.msg_len = ntouv32(sizeof(DownLinkTestRsp));
	}

};

class DownDisconnectInform
{
public:
	Header header;
	unsigned char error_code;
	uint16_t crc_code;
	End end;
};

class DownCloselinkInform
{
public:
	Header header;
	unsigned char reason_code;
	uint16_t crc_code;
	End end;
};

class DownTotalRecvBackMsg
{
public:
	Header header;
	uint32_t DynamicInfoTotal;
	uint64_t start_time;
	uint64_t end_time;
	uint16_t crc_code;
	End end;

	DownTotalRecvBackMsg()
	{
		header.msg_type = ntouv16(DOWN_TOTAL_RECV_BACK_MSG);
		header.msg_len = ntouv32(sizeof(DownTotalRecvBackMsg));
	}
};

/*��̬ҵ������*/
class ExgMsgHeader
{
public:
	char vehicle_no[21];
	unsigned char vehicle_color;
	uint16_t data_type;
	uint32_t data_length;
	ExgMsgHeader()
	{
		memset( vehicle_no, 0, sizeof(vehicle_no) ) ;
	}
};

class GnssData
{
public:
	unsigned char encrypt;
	unsigned char date[4];
	unsigned char time[3];
	uint32_t lon;
	uint32_t lat;
	uint16_t vec1; 		//�ϴ��������ٶ�
	uint16_t vec2; 		//��ʻ�ٶȼ�¼
	uint32_t vec3; 		//������ǰ���
	uint16_t direction; // ����
	uint16_t altitude;  // ���θ߶�
	uint32_t state;	    // ����״̬
	uint32_t alarm;     // ����״̬
};

// �ϴ�����ע����Ϣ��Ϣ
class UpExgMsgRegister
{
public:
	Header 		  header ;
	ExgMsgHeader  exg_msg_header ;
	char platform_id[11] ;
	char producer_id[11] ;
	char terminal_model_type[20] ;
	char terminal_id[7] ;
	char terminal_simcode[12] ;
	uint16_t crc_code ;
	End end ;
	UpExgMsgRegister()
	{
		memset( platform_id, 0, sizeof(platform_id) ) ;
		memset( producer_id, 0, sizeof(producer_id) ) ;
		memset( terminal_model_type, 0, sizeof(terminal_model_type) ) ;
		memset( terminal_id, 0, sizeof(terminal_id) ) ;
		memset( terminal_simcode, 0, sizeof(terminal_simcode) ) ;
	}
};

class UpExgMsgRealLocation
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	GnssData gnss_data;
	uint16_t crc_code;
	End end;
	UpExgMsgRealLocation()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_REAL_LOCATION);
	}
};

class UpExgMsgHistoryHeader
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
};

class UpExgMsgHistoryLocation
{
public:
	UpExgMsgHistoryHeader header;
	unsigned char gnss_cnt;
};

// ����������λ��Ϣ������Ӧ����Ϣ
class UpExgMsgArcossareaStartupAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
	UpExgMsgArcossareaStartupAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_RETURN_STARTUP_ACK);
	}
};
// ����������λ��Ϣ������Ӧ����Ϣ
class UpExgMsgReturnStartupAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
	UpExgMsgReturnStartupAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_RETURN_STARTUP_ACK);
	}
};

// ����������λ��Ϣ������Ӧ����Ϣ
class UpExgMsgArcossareaEndAck//4.5.3.1.5 ������������λ��Ϣ����Ӧ����Ϣ
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
	UpExgMsgArcossareaEndAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_RETURN_END_ACK);
	}
};

// �·���������������Ϣ
class DownExgMsgArcossareaend//4.5.3.2.6 ������������λ��Ϣ����������Ϣ
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char reason_char;
	uint16_t crc_code;
	End end;
	DownExgMsgArcossareaend()
	{
		header.msg_type = ntouv16(DOWN_EXG_MSG);
		exg_msg_header.data_type = ntouv16(DOWN_EXG_MSG_ARCOSSAREA_END);
	}
};

// ����������λ��Ϣ������Ӧ����Ϣ
class UpExgMsgReturnEndAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
	UpExgMsgReturnEndAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_RETURN_END_ACK);
	}
};

// �·������˵�����
class DownExgMsgTaskWaybillReq//4.5.3.2.11 �ϱ����������˵�������Ϣ
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	End end;
	DownExgMsgTaskWaybillReq()
	{
		header.msg_type = ntouv16(DOWN_EXG_MSG);
		exg_msg_header.data_type = ntouv16(DOWN_EXG_MSG_TAKE_WAYBILL_REQ);
	}
};
class UpExgMsgTaskWaybillAck//4.5.3.1.10 �ϱ����������˵�Ӧ����ϢUP_EXG_MSG_TAKE_WAYBILL_ACK
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint32_t ewaybill_length;
	//--EWAYBILL_INFO
	//End end;
	UpExgMsgTaskWaybillAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_TAKE_WAYBILL_ACK);
	}
};

//���복������ָ��������λ��Ϣ������Ϣ
class UpExgMsgApplyForMonitorStartup
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint64_t start_time;
	uint64_t end_time;
	uint16_t crc_code;
	End end;
	UpExgMsgApplyForMonitorStartup()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP);
	}
};

// ȡ��������ָ��������λ��Ϣ������Ϣ
class UpExgMsgApplyForMonitorEnd
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
	UpExgMsgApplyForMonitorEnd()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_APPLY_FOR_MONITOR_END);
	}
};

// �ϱ���ʻԱ���ʶ����Ϣ
class UpExgMsgreportdriverInfoAckHeader
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
};

// ����������λ��Ϣ������Ϣ
class UpExgApplyHisGnssDataReq
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint64_t start_time;
	uint64_t end_time;
	uint16_t crc_code;
	End end;
};

//	�����ϱ���ʻԱ�����Ϣ��Ϣ
class UpExgMsgReportDriverInfo
{
public:
	Header header ;
	ExgMsgHeader exg_msg_header;
	char driver_name[16];
	char driver_id[20];
	char licence[40];
	char org_name[200];
	uint16_t crc_code ;
	End end ;
	UpExgMsgReportDriverInfo()
	{
		memset( driver_name, 0, sizeof(driver_name) ) ;
		memset( driver_id  , 0, sizeof(driver_id) ) ;
		memset( licence    , 0, sizeof(licence) ) ;
		memset( org_name   , 0, sizeof(org_name) ) ;	
	}
};

// �ϱ�˾�����ʶ����ϢӦ����Ϣ
class UpExgMsgReportDriverInfoAck
{
public:
	Header header ;
	ExgMsgHeader exg_msg_header;
	char driver_name[16];
	char driver_id[20];
	char licence[40];
	char org_name[200];
	uint16_t crc_code ;
	End end ;
	UpExgMsgReportDriverInfoAck()
	{
		memset( driver_name, 0, sizeof(driver_name) ) ;
		memset( driver_id  , 0, sizeof(driver_id) ) ;
		memset( licence    , 0, sizeof(licence) ) ;
		memset( org_name   , 0, sizeof(org_name) ) ;
	}
};

// �ϱ������˵�Ӧ�����Ϣ
class UpExgMsgTakewaybillAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
};

//	�����ϱ����������˵���Ϣ��Ϣ
class UpExgMsgReportEwaybillInfo
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint32_t ewaybill_length;	
};

// �������ݴ���
class DownExgMsgCarLocation
{
public:
	Header 		 header;
	ExgMsgHeader exg_msg_header;
	GnssData     gnss ;
	Footer  	 footer ;
};

// ������λ��Ϣ����������Ϣ
class DownExgMsgHistoryArcossareaHeader
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char cnt_num ;  // ��¼�켣����
};

// ����������̬��Ϣ
class DownExgMsgCarInfoHeader
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
};

// ����������λ��Ϣ����Ӧ����Ϣ
class DownExgMsgReturnStartUp
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char reason_code;
	uint16_t crc_code;
	End end;
};

// ����������λ��ϢЧ��Ӧ����Ϣ
class DownExgMsgReturnEnd
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char reason_code;
	uint16_t crc_code;
	End end;
};

// ����������λ��Ϣ����������Ϣ
class DownExgMsgApplyforMonitorstartUpAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char result;
	uint16_t crc_code;
	End end;
};

// ����������λ��Ϣ����������Ϣ
class DownExgMsgApplyforMonitorEndAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char result;
	uint16_t crc_code;
	End end;
};

// ����������λ��ϢӦ����Ϣ
class DownExgMsgApplyHisgnssdataAck
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char result;
	uint16_t crc_code;
	End end;
};

// �ϱ���ʻԱ���ʶ����Ϣ������Ϣ
class DownExgMsgReportDriverInfo
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
};

// �ϱ����������˵�������Ϣ
class DownExgMsgTakeWaybillReq
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	uint16_t crc_code;
	End end;
};

//ƽ̨���ͨ��
class UpPlatformMsg
{
public:
	uint16_t data_type;
	uint32_t data_length;
};

class UpPlatformMsgpostqueryAckHeader
{
public:
	Header header;
	UpPlatformMsg up_platform_msg;
};

// ƽ̨���Ӧ����Ϣ��
class UpPlatformMsgpostqueryData
{
public:
	unsigned char object_type;
	char object_id[12];
	uint32_t info_id ;  // ��ϢID����ID���·���ID��ͬ
	uint32_t msg_len ;  // ���ݳ���
};

class DownPlatformMsg
{
public:
	uint16_t data_type;
	uint32_t data_length;
};

//4.5.4.2.3 �·�ƽ̨�䱨��������Ϣ
class DownPlatformMsgInfoReq
{
public :
	Header header;
	DownPlatformMsg down_platform_msg;
	unsigned char object_type;
	char object_id[12];
	uint32_t info_id;
	uint32_t info_length;
	DownPlatformMsgInfoReq()
	{
		header.msg_type = ntouv16(DOWN_PLATFORM_MSG);
		down_platform_msg.data_type = ntouv16(DOWN_PLATFORM_MSG_INFO_REQ);
	}
};

// ƽ̨��ڵ��ڲ�����
class DownPlatformMsgPostQueryBody
{
public:
	unsigned char object_type;
	char object_id[12];
	uint32_t  info_id ; 	// ��ϢID
	uint32_t  info_length ; //���ݳ��ȣ����滹�о��������
	// INFO_CONTENT	INFO_LENGTH	Octet String	��Ϣ����
};

// ƽ̨��ڵ�����
class DownPlatformMsgPostQueryReq//4.5.4.2.2 ƽ̨���������Ϣ
{
public:
	Header header;
	DownPlatformMsg down_platform_msg;
	DownPlatformMsgPostQueryBody down_platform_body ;
	DownPlatformMsgPostQueryReq()
	{
		header.msg_type = ntouv16(DOWN_PLATFORM_MSG);
		down_platform_msg.data_type = ntouv16(DOWN_PLATFORM_MSG_POST_QUERY_REQ);
	}
};

//4.5.4.1.2 ƽ̨���Ӧ����Ϣ
class UpPlatformMsgPostQueryAck
{
public:
	Header header;
	UpPlatformMsg up_platform_msg;
	UpPlatformMsgpostqueryData up_platform_post;
	UpPlatformMsgPostQueryAck()
	{
		header.msg_type = ntouv16(UP_PLATFORM_MSG);
		up_platform_msg.data_type = ntouv16(UP_PLATFORM_MSG_POST_QUERY_ACK);
	}
};

//4.5.4.1.3 �·�ƽ̨�䱨��Ӧ����Ϣ
class UpPlatformMsgInfoAck
{
public:
	Header header;
	UpPlatformMsg up_platform_msg;
	uint32_t info_id;
	uint16_t crc_code;
	End end;

	UpPlatformMsgInfoAck()
	{
		header.msg_type = ntouv16(UP_PLATFORM_MSG);
		up_platform_msg.data_type = ntouv16(UP_PLATFORM_MSG_INFO_ACK);
	}
};

// ����������λ��Ϣ����������Ϣ
class DownExgMsgArcossareastartUp//DOWN_EXG_MSG_ARCOSSAREA_STARTUP
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	unsigned char reason_code;
	End end;
	DownExgMsgArcossareastartUp()
	{
		header.msg_type = ntouv16(DOWN_EXG_MSG);
		exg_msg_header.data_type = ntouv16(DOWN_EXG_MSG_ARCOSSAREA_STARTUP);
	}
};

// ����������λ��Ϣ����Ӧ����Ϣ
class UpExgMsgArcossareastartUpAck//UP_EXG_MSG_ARCOSSAREA_STARTUP_ACK
{
public:
	Header header;
	ExgMsgHeader exg_msg_header;
	End end;
	UpExgMsgArcossareastartUpAck()
	{
		header.msg_type = ntouv16(UP_EXG_MSG);
		exg_msg_header.data_type = ntouv16(UP_EXG_MSG_ARCOSSAREA_STARTUP_ACK);
	}
};

// �·�ƽ̨��Ϣͷ
class DownPlatformMsgInfoReqHeader
{
public:
	Header header;
	DownPlatformMsg down_platform_msg;
};


///���������Ϣ��
class WarnMsgHeader
{
public:
	char vehicle_no[21];
	unsigned char vehicle_color;
	uint16_t data_type;
	uint32_t data_length;
	WarnMsgHeader()
	{
		memset( vehicle_no, 0, sizeof(vehicle_no) ) ;
	}
};

// ��������Ӧ����Ϣ
class UpWarnMsgUrgeTodoAck//4.5.5.1.2 ��������Ӧ����Ϣ
{
public:
	Header header;
	WarnMsgHeader warn_msg_header;
	unsigned int  supervision_id ;  // ���������ID
	unsigned char result;
	uint16_t crc_code;
	End end;

	UpWarnMsgUrgeTodoAck()
	{
		header.msg_type = ntouv16(UP_WARN_MSG);
		warn_msg_header.data_type = ntouv16(UP_WARN_MSG_URGE_TODO_ACK);
	}
};

// �ϱ�������Ϣ��Ϣ
class UpWarnMsgAdptInfoHeader
{
public:
	Header header;
	WarnMsgHeader warn_msg_header;
};

// ��Ϣ������
class UpWarnMsgAdptInfoBody
{
public:
	unsigned char  warn_src ;
	uint16_t warn_type ;
	uint64_t warn_time ;
	uint32_t info_id ;
	uint32_t info_length ;
};

// ��������BODY����
class WarnMsgUrgeTodoBody
{
public:
	unsigned char warn_src ;
	uint16_t warn_type ;
	uint64_t warn_time ;
	uint32_t supervision_id ;
	uint64_t supervision_endtime ;
	unsigned char supervision_level ;
	char supervisor[16] ;
	char supervisor_tel[20] ;
	char supervisor_email[32] ;
	WarnMsgUrgeTodoBody()
	{
		memset( supervisor, 0 , sizeof(supervisor) ) ;
		memset( supervisor_tel , 0, sizeof(supervisor_tel) ) ;
		memset( supervisor_email, 0, sizeof(supervisor_email) ) ;
	}
};

// ��������
class DownWarnMsgUrgeTodoReq
{
public:
	Header header;
	WarnMsgHeader warn_msg_header;
	WarnMsgUrgeTodoBody  warn_msg_body;
	uint16_t crc_code;
	End end;
};

// ����Ԥ��������
class WarnMsgInformTipsBody
{
public:
	unsigned char warn_src ;
	uint16_t  warn_type ;
	uint64_t  warn_time ;
	uint32_t  warn_length ;
};

// ����Ԥ��
class DownWarnMsgInformTips
{
public:
	Header header;
	WarnMsgHeader warn_msg_header;
	WarnMsgInformTipsBody warn_msg_body ;
};

//	�����ϱ�����������������
class WarnMsgAdptTodoInfoBody
{
public:
	uint32_t	info_id;
	unsigned char result;
};

//	�����ϱ�������������Ϣ
class UpWarnMsgAdptTodoInfo
{
public:
	Header header;
	WarnMsgHeader warn_msg_header;
	WarnMsgAdptTodoInfoBody warn_msg_body;
	uint16_t crc_code;
	End end;
};

//class DownWarnMsgExgInform
//{
//public:
//	Header header;
//	uint16_t crc_code;
//	End end;
//};

class CtrlMsgHeader
{
public:
	char vehicle_no[21];
	unsigned char vehicle_color;
	uint16_t data_type;
	uint32_t data_length;
	CtrlMsgHeader()
	{
		memset( vehicle_no, 0, sizeof(vehicle_no) ) ;
	}
};

class DownCtrlMsgHeader
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
};

//4.5.6.1.2 �����������Ӧ����Ϣ
class UpCtrlMsgMonitorVehicleAck
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	unsigned char result;
	uint16_t crc_code;
	End end;
	UpCtrlMsgMonitorVehicleAck()
	{
		header.msg_type = ntouv16(UP_CTRL_MSG);
		ctrl_msg_header.data_type = ntouv16(UP_CTRL_MSG_MONITOR_VEHICLE_ACK);

	}
};

//4.5.6.1.4 �·���������Ӧ����Ϣ
class UpCtrlMsgTextInfoAck
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	uint32_t msg_id;
	unsigned char result;
	uint16_t crc_code;
	End end;
	UpCtrlMsgTextInfoAck()
	{
		header.msg_type = ntouv16(UP_CTRL_MSG);
		ctrl_msg_header.data_type = ntouv16(UP_CTRL_MSG_TEXT_INFO_ACK);
	}
};

//4.5.6.2.4 �·���������������Ϣ
class DownCtrlMsgTextInfo
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	uint32_t msg_sequence;
	unsigned char  msg_priority;
	uint32_t msg_length;
	DownCtrlMsgTextInfo()
	{
		header.msg_type = ntouv16(DOWN_CTRL_MSG);
		ctrl_msg_header.data_type = ntouv16(DOWN_CTRL_MSG_TEXT_INFO);
	}
};
// ��������Ӧ����Ϣ
class UpCtrlMsgTakePhotoBody
{
public:
	unsigned char rsp_flag ;
	GnssData gps  ;
	unsigned char lens_id ;
	unsigned int  photo_len ;
	unsigned char size_type ;
	unsigned char type ;
};

//�ϱ��г���¼�����ݡ�
class UpCtrlMsgTaketravel
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header ;
	unsigned char command_type;
	uint32_t travel_length ;
};

// ����Ӧ��������ƽ̨����Ϣ
class UpCtrlMsgEmergencyMonitoringAck
{
public:
	Header header ;
	CtrlMsgHeader ctrl_msg_header ;
	unsigned char result ;
	uint16_t crc_code ;
	End end ;
};

class DownCtrlMsgMonitorVehicleReq
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	char monitor_tel[20];
	uint16_t crc_code;
	End end;

	DownCtrlMsgMonitorVehicleReq()
	{
		memset( monitor_tel , 0 , sizeof(monitor_tel) ) ;
	}
};

// ����
class DownCtrlMsgTakePhotoReq//4.5.6.2.3 ��������������Ϣ
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	unsigned char lens_id;
	unsigned char size;
	uint16_t crc_code;
	End end;
};
class UpCtrlMsgTakePhotoAck
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	UpCtrlMsgTakePhotoBody ctrl_photo_body;
	UpCtrlMsgTakePhotoAck()
	{
		header.msg_type = ntouv16(UP_CTRL_MSG);
		ctrl_msg_header.data_type = ntouv16(UP_CTRL_MSG_TAKE_PHOTO_ACK);
		ctrl_photo_body.rsp_flag = 0x01;
		ctrl_photo_body.type = 0x01;
		ctrl_photo_body.size_type = 0x01;
	}
};
class DownCtrlMsgTextInfoHeader
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
	uint32_t msg_sequence;
	unsigned char msg_priority;
	uint32_t msg_len;
};

// �ϱ�������ʻ��¼������Ϣ
class DownCtrlMsgTaketravelReq
{
public:
	Header header;
	CtrlMsgHeader ctrl_msg_header;
//	uint64_t start_time;
//	uint64_t end_time;
	unsigned char command_type;
	uint16_t crc_code;
	End end;
};

// ����Ӧ��������ƽ̨
class DownCtrlMsgEmergencyMonitoringReq
{
public:
	Header header ;
	CtrlMsgHeader ctrl_msg_header ;
	char authentication_code[10] ;
	char access_point_name[20] ;
	char username[49] ;
	char password[22] ;
	char server_ip[32] ;
	uint16_t  tcp_port ;
	uint16_t  udp_port ;
	uint64_t  end_time ;
	uint16_t  crc_code ;
	End end ;

	DownCtrlMsgEmergencyMonitoringReq()
	{
		memset( authentication_code, 0 , sizeof(authentication_code) ) ;
		memset( access_point_name  , 0 , sizeof(access_point_name) ) ;
		memset( username , 0, sizeof(username) ) ;
		memset( password , 0, sizeof(password) ) ;
		memset( server_ip, 0, sizeof(server_ip) ) ;
	}
};

class BaseMsgHeader
{
public:
	char vehicle_no[21];
	unsigned char vehicle_color;
	uint16_t data_type;
	uint32_t data_length;
	BaseMsgHeader()
	{
		memset( vehicle_no, 0, sizeof(vehicle_no) ) ;
	}
};

// ����������̬��Ϣ
class DownBaseMsgVehicleAdded
{
public:
	Header header;
	BaseMsgHeader  msg_header ;
	Footer footer ;

	DownBaseMsgVehicleAdded()
	{
		header.msg_len         = ntouv32( sizeof(DownBaseMsgVehicleAdded) ) ;
		header.msg_type  	   = ntouv16( DOWN_BASE_MSG ) ;
		msg_header.data_type   = ntouv16( DOWN_BASE_MSG_VEHICLE_ADDED ) ;
		msg_header.data_length = ntouv32( 0x00 ) ;
	}
};

// ����������̬��ϢӦ����Ϣ
class UpbaseMsgVehicleAddedAck
{
public:
	Header header;
	BaseMsgHeader msg_header;
	// ����Ϊ����������

	UpbaseMsgVehicleAddedAck()
	{
		header.msg_type   	   = ntouv16( UP_BASE_MSG ) ;
		msg_header.data_type   = ntouv16( UP_BASE_MSG_VEHICLE_ADDED_ACK ) ;
	}
};

#pragma pack()


typedef long long 		   sint64 ;
typedef int       		   sint32 ;
typedef short			   sint16 ;
typedef char			   sint8  ;

typedef unsigned long long uint64 ;
typedef unsigned int       uint32 ;
typedef unsigned short     uint16 ;
typedef unsigned char      uint8  ;

// ת��������Ƶ�ATOI
static sint64 atoiInt64 (const char *ident, sint64 base)
{
	sint64 number = 0;
	bool neg = false;

	// NULL string
	assert (ident != NULL);

	// empty string
	if (*ident == '\0') goto end;

	// + sign
	if (*ident == '+') ident++;

	// - sign
	if (*ident == '-') { neg = true; ident++; }

	while (*ident != '\0')
	{
		if (isdigit((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'0';
		}
		else if (base > 10 && islower((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'a'+10;
		}
		else if (base > 10 && isupper((unsigned char)*ident))
		{
			number *= base;
			number += (*ident)-'A'+10;
		}
		else
		{
			goto end;
		}
		ident++;
	}
end:
	if (neg) number = -number;
	return number;
}

// ת64λ��ATOI
static uint64 atoi64( const char *idest )
{
	return (uint64) atoiInt64( idest, 10 ) ;
}

/**
static void itoaInt64 (sint64 number, char *str, sint64 base)
{
	str[0] = '\0';
	char b[256];
	if(!number)
	{
		str[0] = '0';
		str[1] = '\0';
		return;
	}
	memset(b,'\0',255);
	memset(b,'0',64);
	sint32 n;
	sint64 x = number;
	if (x < 0) x = -x;
	char baseTable[] = "0123456789abcdefghijklmnopqrstuvwyz";
	for(n = 0; n < 64; n ++)
	{
		sint32 num = (sint32)(x % base);
		b[64 - n] = baseTable[num];
		if(!x)
		{
			int k;
			int j = 0;

			if (number < 0)
			{
				str[j++] = '-';
			}

			for(k = 64 - n + 1; k <= 64; k++)
			{
				str[j ++] = b[k];
			}
			str[j] = '\0';
			break;
		}
		x /= base;
	}
}

static void itoa64( sint64 number, char *str )
{
	return itoaInt64( number, str, 10 ) ;
}
*/

#endif /* PROTOPARSE_H_ */
