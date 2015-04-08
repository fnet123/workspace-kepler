/**********************************************
 * GBProtoParse.h
 *
 *  Created on: 2011-3-7
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/

#ifndef __GBPROTOPARSE_H_
#define __GBPROTOPARSE_H_

#pragma pack(1)

#include <string.h>
#include <arpa/inet.h>

//typedef struct _MsgType //��Ϣ��Ϣ��
//{
//	unsigned short retain:4;
//	unsigned char is_long_msg:1;
//	unsigned char rsa:1;
//	unsigned short msg_len:10;
//	//Ĭ��ֵȫ��Ϊ0�����ʱֻ��Ҫ����msg_len���ɡ�
//	_MsgType()
//	{
//		rsa = 0;
//		is_long_msg = 0;
//		retain = 0;
//		msg_len = 0;
//	}
//}MsgType;
typedef struct _Begin
{
	char _begin;
	_Begin()
	{
		_begin = 0x7e;
	}
}Begin;

typedef struct _End
{
	char _end;
	_End()
	{
		_end = 0x7e;
	}
}End;

typedef struct _MsgType //��Ϣ��Ϣ��
{
	unsigned char retain:2;
	unsigned char is_long_msg:1;
	unsigned char rsa:3;
	//unsigned char res:2;
	unsigned short msg_len:10;

	//Ĭ��ֵȫ��Ϊ0�����ʱֻ��Ҫ����msg_len���ɡ�
	_MsgType()
	{
		rsa = 0;
		is_long_msg = 0;
		retain = 0;
		msg_len = 0;
	}
}MsgType;

typedef struct _MsgPart//��Ϣ����װ��
{
	unsigned short msgsum;
	unsigned short msgorder;//��1��ʼ
}MsgPart;

typedef struct _GBheader //������Ϣͷ
{
	Begin begin;
	unsigned short msgid;
	MsgType msgtype;
	char car_id[6];
	unsigned short seq;
//	msgPart msgpart;
	_GBheader()
	{
		msgid = 0;
		memset(car_id,0,6);
		seq = 0;
	}
}GBheader;

typedef struct _GBFooter
{
	unsigned char check_sum ;
	End  ender ;
}GBFooter;

typedef struct _GBheader_test //������Ϣͷ
{
	Begin begin;
	unsigned short msgid;
	unsigned short msgtype;
	char car_id[6];
	unsigned short seq;
//	msgPart msgpart;
	_GBheader_test()
	{
		msgid = 0;
		memset(car_id,0,6);
		seq = 0;
	}
}GBheader_test;

typedef struct _CommonResp
{
	unsigned short resp_seq;
	unsigned short resp_msg_id;
	unsigned char  result;
	_CommonResp()
	{
		resp_seq = 0;
		resp_msg_id = 0;
		result = 0;
	}
}CommonResp;


typedef struct _TermCommonResp
{
	GBheader   header;
	CommonResp resp;
	GBFooter   footer ;

	_TermCommonResp()
	{
		header.msgid = 0x0001;
		header.msgtype.msg_len = sizeof(struct _TermCommonResp);
	}

}TermCommonResp;

typedef struct _PlatFormCommonResp
{
	GBheader header;
	CommonResp resp;
	unsigned char check_sum;
	End end;
	_PlatFormCommonResp()
	{
		//header.msgid = 0x0180;
		//header.msgtype.msg_len = (sizeof(resp));
	}

}PlatFormCommonResp;


typedef struct _TermRegisterHeader //ע��
{
	GBheader header;
	unsigned short province_id;
	unsigned short city_id;
	unsigned char corp_id[5];
	unsigned char termtype[20];  // ������������Ϊ20
	unsigned char termid[7];
	unsigned char carcolor;
	_TermRegisterHeader()
	{
		header.msgid = 0x0100;
		province_id = 0;
		city_id = 0;
		carcolor = 0;
	}
}TermRegisterHeader;


//�ն�ע��ظ�
typedef struct _TermRegisterRespHeader
{
	GBheader header;
	unsigned short resp_seq;
	unsigned char result;
	//��Ȩ�룻
	//unsigned char check_num;
	_TermRegisterRespHeader()
	{
		//�ն�ע��Ӧ��
		//header.msgid = 0x8100;
	}

}TermRegisterRespHeader;


//�ظ�Ϊƽ̨ͨ�ûظ�
typedef struct _TermAuthenticationHeader
{
	GBheader header;
	_TermAuthenticationHeader()
	{
		header.msgid = 0x0102;
	}
	//���滹����ϼ�Ȩ���check_sum;

}TermAuthenticationHeader;



//�澯��־λ ��Ӧ������Ϣ
typedef struct _TermAlarm
{
	unsigned char  bit31:1;
	unsigned char  bit30:1;
	unsigned char  bit29:1;
	unsigned char  bit28:1;
	unsigned char  bit27:1;
	unsigned char  bit26:1;
	unsigned char  bit25:1;
	unsigned char  bit24:1;
	unsigned char  bit23:1;
	unsigned char  bit22:1;
	unsigned char  bit21:1;
	unsigned char  bit20:1;
	unsigned char  bit19:1;
	unsigned char  bit18:1;
	unsigned char  bit17:1;
	unsigned char  bit16:1;
	unsigned char  bit15:1;
	unsigned char  bit14:1;
	unsigned char  bit13:1;
	unsigned char  bit12:1;
	unsigned char  bit11:1;
	unsigned char  bit10:1;
	unsigned char  bit9:1;
	unsigned char  bit8:1;
	unsigned char  bit7:1;
	unsigned char  bit6:1;
	unsigned char  bit5:1;
	unsigned char  bit4:1;
	unsigned char  bit3:1;
	unsigned char  bit2:1;
	unsigned char  bit1:1;
	unsigned char  bit0:1;
}TermAlarm;


typedef struct _CarState
{
	unsigned char  bit31:1;
	unsigned char  bit30:1;
	unsigned char  bit29:1;
	unsigned char  bit28:1;
	unsigned char  bit27:1;
	unsigned char  bit26:1;
	unsigned char  bit25:1;
	unsigned char  bit24:1;
	unsigned char  bit23:1;
	unsigned char  bit22:1;
	unsigned char  bit21:1;
	unsigned char  bit20:1;
	unsigned char  bit19:1;
	unsigned char  bit18:1;
	unsigned char  bit17:1;
	unsigned char  bit16:1;
	unsigned char  bit15:1;
	unsigned char  bit14:1;
	unsigned char  bit13:1;
	unsigned char  bit12:1; //12	0�����Ž���     1�����ż���
	unsigned char  bit11:1; //11	0��������·����     1��������·�Ͽ�
	unsigned char  bit10:1; // 10	0��������·����     1��������·�Ͽ�
	unsigned char  bit9:1; //9	0���ճ�         1���س�
	unsigned char  bit8:1; //8	0��ACC��      1��ACC��
	unsigned char  bit7:1;
	unsigned char  bit6:1; //����
	unsigned char  bit5:1; //����
	unsigned char  bit4:1; //0��δԤԼ     1��ԤԼ(����)
	unsigned char  bit3:1; //0����Ӫ״̬     1��ͣ��״
	unsigned char  bit2:1; //��������
	unsigned char  bit1:1; //�ϱ�γ��
	unsigned char  bit0:1;//Gps��λ δ��λ
}CarState;

typedef struct _VechileControl
{
	GBheader header;
	unsigned char a7:1;
	unsigned char a6:1;
	unsigned char a5:1;
	unsigned char a4:1;
	unsigned char a3:1;
	unsigned char a2:1;
	unsigned char a1:1;
	unsigned char a0:1;
	_VechileControl()
	{
		header.msgid = 0x8500;
		a0 = a1 = a2 = a3 = a4 = a5 = a6 =a7 = 0;
	}
}VechileControl;

typedef struct _GpsInfo
{
	TermAlarm alarm;
	CarState state;

	unsigned int latitude;
	unsigned int longitude;

	unsigned short heigth;
	unsigned short speed;
	unsigned short direction;
	char date_time[6];

	_GpsInfo()
	{
		memset(&alarm,0,sizeof(TermAlarm));
		memset(&state,0,sizeof(CarState));
		memset(date_time,0,6);
		longitude = 0;
		latitude = 0;
		heigth = 0;
		speed = 0;
		direction = 0;
	}
}GpsInfo;
//8.3.1��λ����Ϣ�㱨
//�ն˸��ݲ����趨�����Է���λ����Ϣ�㱨(0x0200)��Ϣ��ƽ̨�ظ�ƽ̨ͨ��Ӧ��(0x8001)��Ϣ��
//���ݲ������ƣ��ն����жϵ���������ʱ�ɷ���λ����Ϣ�㱨��Ϣ��
typedef struct _TermLocationHeader
{
	GBheader header;
	GpsInfo gpsinfo;

	//������Ϣ�������У�Ҳ����û��
	_TermLocationHeader()
	{
		header.msgid = 0x0200;
	}
}TermLocationHeader;




//8.3.2��λ����Ϣ��ѯ
//ƽ̨ͨ������λ����Ϣ��ѯ(0x8201)��Ϣ��ѯָ�������ն˵�ʱλ����Ϣ���ն˻ظ�λ����Ϣ��ѯӦ��(0x0201)��Ϣ��
typedef struct _PlatformRequestLocation
{
	GBheader header;
	unsigned char check_num;
	End end;
	_PlatformRequestLocation()
	{
		header.msgtype.msg_len = 0;
		header.msgid = 0x8201;
	}

}PlatformRequestLocation;

//�ɼ���ʻԱ�����Ϣ����8.8.1
//�ն˲ɼ�˾�������Ϣ����(0x8903)�ϴ�ƽ̨����ʶ��ƽ̨�ظ��ɹ������Ϣ(0x8001)��
typedef struct _PilotLocationHeader
{
	GBheader header;
	_PilotLocationHeader()
	{
		header.msgid = 0x8903;
	}
}PilotLocationHeader;

//�����ն˲���8.2.3
//ƽ̨ͨ�����������ն˲���(0x8103)��Ϣ�����ն˲������ն˻ظ��ն�ͨ��Ӧ��(0x0001)��Ϣ��
typedef struct _SetGetTermParamHeader
{
	GBheader header;
	_SetGetTermParamHeader()
	{
		header.msgid = 0x8103;
	}
}SetGetTermParamHeader;

//��ѯ�ն˲���8.2.3
//ƽ̨ͨ�����Ͳ�ѯ�ն˲���(0x8104)��Ϣ��ѯ�ն˲������ն˻ظ���ѯ�ն˲���Ӧ��(0x0104)��Ϣ��
typedef struct _QueryTermParamHeader
{
	GBheader header;
	_QueryTermParamHeader()
	{
		header.msgid = 0x8104;
	}

}QueryTermParamHeader;

struct PlatformTraceBody
{
	unsigned short timeval;
	unsigned int   period;
};

typedef struct _PlatformTraceLocation
{
	GBheader header;
	PlatformTraceBody body ;
	GBFooter footer ;
	_PlatformTraceLocation()
	{
		header.msgid = 0x8202;
	}
}PlatformTraceLocation;

typedef struct _PeripheryInfoInquiry
{
	GBheader header;
	unsigned char info_type;
}PeripheryInfoInquiry;

typedef struct _TermLocationRespHeader
{
	GBheader header;
	unsigned short reqseq;
	GpsInfo  gpsinfo;
	//������Ϣ�������У�Ҳ����û��
	_TermLocationRespHeader()
	{
		header.msgid = 0x0201;
	}
}TermLocationRespHeader;


/*
 *
ƽ̨ͨ������λ�ø��ٿ���(0x8202)��Ϣ����/ֹͣλ�ø��٣�λ�ø���Ҫ���ն�ֹ֮ͣǰ�����ڻ㱨������Ϣָ��ʱ�������л㱨���ն˻ظ��ն�ͨ��Ӧ��(0x0001)��Ϣ��
*/

typedef struct _PlatformLocationControl
{
	GBheader header;
	unsigned int time_interval;
	unsigned char check_num;

	_PlatformLocationControl()
	{
		header.msgid = 0x8202;
		//��ʼ����ʱ��Ĭ��30s�ϴ�һ�Ρ�
		time_interval = 30;
	}
}PlatformLocationControl;


/*
ƽ̨ͨ�������ı���Ϣ�·�(0x8300)��Ϣ��ָ����ʽ֪ͨ��ʻԱ���ն˻ظ��ն�ͨ��Ӧ��(0x0001)��Ϣ��
 */

//λ	��־
//0	1������
//1	����
//2	1���ն���ʾ����ʾ
//3	1���ն�TTS����
//4	1���������ʾ
//5��7	����

typedef struct _TxtState
{
	unsigned char a7:1;
	unsigned char a6:1;
	unsigned char a5:1;
	unsigned char a4:1;
	unsigned char a3:1;
	unsigned char a2:1;
	unsigned char a1:1;
	unsigned char a0:1;

	_TxtState()
	{
		a0 = a1 = a2 = a3 = a4 = a5 = a6 = a7 = 1;
	}
}TxtState;

//�ظ�ƽ̨ͨ��Ӧ��
typedef struct _PlatformTxtMsgHeader
{
	GBheader header;
	TxtState state;
	_PlatformTxtMsgHeader()
	{
		header.msgid = 0x8300;
	}
}PlatformTxtMsgHeader;


//9.24���绰�ز�(0x8400)��Ϣ��
//0	��־	BYTE	0:��ͨͨ�� 1:����
//1	�绰����	STRING	�Ϊ20�ֽ�
typedef struct _PlatformTelRequestHeader
{
	GBheader header;
	unsigned char state; //0:��ͨͨ�� 1:����
//	unsigned char check_sum;

	_PlatformTelRequestHeader()
	{
		//header.msgid = 0x8400;
	}

}PlatformTelRequestHeader;


struct TakePhotoBody
{
	unsigned char  camera_id;  // >0
	unsigned short photo_num; //0��ʾֹͣ���㣻0xFFFF��ʾ¼��������ʾ��������
	unsigned short time_interval; //��������ʱ��ʱ����
	unsigned char  is_save; //1�����棻0��ʵʱ�ϴ�
	unsigned char  sense; //�ֱ���
	unsigned char  photo_quality; //1��10��1���
	unsigned char  liangdu;
	unsigned char  duibidu;
	unsigned char  baohedu;
	unsigned char  sedu;
	// unsigned int   seq;

	TakePhotoBody()
	{
		camera_id 	  = 1;
		photo_num 	  = 1;
		is_save 	  = 0;
		sense 		  = 1;
		photo_quality = 1;
		liangdu 	  = 50;
		duibidu 	  = 50;
		baohedu 	  = 50;
		sedu 		  = 50;
		//seq           = 1;
	}
};

/*
0	ͼ��/��ƵID	DWORD	>0
4	����ͷID	BYTE	0xFF��ʾ��Ƶ�ļ�
5	�����ܰ���	DWORD
9	��ID	DWORD	��1��ʼ
13	λ��ͼ��/��Ƶ/��Ƶ���ݰ�
 */
typedef struct _PhotoUploadHeader
{
	GBheader header;
	unsigned int photo_id;
	unsigned char camera_id;
	unsigned int total_num;
	unsigned int current_num;
}PhotoUploadHeader;

typedef struct _PhotoRespHeader
{
	GBheader header;
	unsigned int  photo_id;
	unsigned char retry_package_num;
	unsigned char check_num;
	End end;
}PhotoRespHeader;

// �ְ����ݴ���
typedef struct _MediaPart
{
	unsigned short total ;
	unsigned short seq ;
}MediaPart;

// ��ý�������ϴ�
typedef struct _MediaUploadBody
{
	unsigned int id ;
	unsigned char type ;
	unsigned char mtype ;
	unsigned char event ;
	unsigned char chanel ;
	GpsInfo       gps ;
}MediaUploadBody;

struct MediaUploadBodyEx
{
	unsigned int id ;
	unsigned char type;
	unsigned char mtype;
	unsigned char event;
	unsigned char chanel;
	unsigned char bcd[6];
	GpsInfo       gps ;
};

typedef struct _EventReport//�¼�����
{
	GBheader header;
	unsigned int  id ;     // ��ý��ID
	unsigned char type ;  // ��ý������ 0 PIC, 1 AUDIO, 2 VIDEO
	unsigned char mtype ; // �ļ������� 0: JPEG , 1:TIF 2:MP3 3:WAV 4:WMV
	unsigned char event ; // �¼� 0 ��Ϣ���� , 1 ��ʱ���� , 2 ���ٱ��� , 3 ��ײ�෭
	unsigned char chanel ; //
	GBFooter footer ;
}EventReort;

// ��ͨ����չ�����ֶ�
struct EventReportEx
{
	unsigned int  id ;     // ��ý��ID
	unsigned char type ;   // ��ý������ 0 PIC, 1 AUDIO, 2 VIDEO
	unsigned char mtype ;  // �ļ������� 0: JPEG , 1:TIF 2:MP3 3:WAV 4:WMV
	unsigned char event ;  // �¼� 0 ��Ϣ���� , 1 ��ʱ���� , 2 ���ٱ��� , 3 ��ײ�෭
	unsigned char chanel ; //
	unsigned char bcd[6] ; // bcdʱ�䴦��
	unsigned int  seq;//�������
};

typedef struct _QuestionReplyAck//����Ӧ��
{
	GBheader header;
	unsigned short seq ;  // Ӧ����ˮ��
	unsigned char  id;	  // Ӧ��
	GBFooter footer ;

}QuestionReplyAsk;

typedef struct _InfoDemandCancleAck  // ��Ϣ�㲥/ȡ��
{
	GBheader header;
	unsigned char info_type;  // ��Ϣ����
	unsigned char info_mark;  // �㲥��ȡ����־
	GBFooter footer ;
}InfoDemandCancleAck;

typedef struct _ElectronicSingle
{
	GBheader header;
	unsigned short single_len;
}ElectronicSingle;

// ��������Ӧ��
struct CarControlAck
{
	GBheader header ;
	unsigned short seqid ;   // Ӧ����ˮ��
	GpsInfo gpsinfo;
	GBFooter  footer ;
};

// �����˵��ϱ�
struct EWayBillReportAckHeader
{
	GBheader header ;
	unsigned int length ;  // ���ݳ���
};

// ��ʻ��¼��
struct TachographBody
{
	unsigned short seq ;    // Ӧ����ˮ��
	unsigned char  cmd ;    // ��ʻ��¼��������
};

/*
��ʼ�ֽ�	�ֶ�	��������	˵��
0	Ӧ����ˮ��	WORD	��Ӧ������ͷͼ��/��Ƶ/��Ƶ�ϴ���Ϣ����ˮ��
2	ͼ��/��Ƶ/��ƵID	DWORD	>0
6	�ش���ID�б�		������125��޸��ֶ���������յ�ȫ�����ݰ�
 */
typedef struct _PhotoUploadResp
{
	GBheader header;
	unsigned int resp_seq;
	unsigned int photo_id;
	//�ش��б���ʱΪ�գ������ش�
	unsigned char check_num;

	_PhotoUploadResp()
	{
		header.msgid = 0x8800;
		header.msgtype.msg_len = sizeof(struct _PhotoUploadResp);
	}
}PhotoUploadResp;

// ��ѯ�ն�Ӧ��
struct QueryTermParamAckHeader
{
	GBheader header  ;
	unsigned short respseq ;
	unsigned char  nparam ;
};

// �洢��ý�����ݼ���Ӧ����Ϣ
struct DataMediaHeader
{
	GBheader header ;
	unsigned short ackseq;  // Ӧ����ˮ��
	unsigned short num ;    // ��ý������������
};
// ���ݽṹ��
struct DataMediaBody
{
	unsigned int   mid  ;   // ��ý��ID����ҳ����
	unsigned char  type ;   // ��ý������
	unsigned char  id ;		// ͨ��ID
	unsigned char  event ;  // �¼������
	GpsInfo	   	   gps ;	// λ���ϱ���Ϣ
};

// ���ݽṹ��,��ͨ�����ر���
struct DataMediaBodyEx
{
	unsigned int   mid  ;   // ��ý��ID����ҳ����
	unsigned char  type ;   // ��ý������
	unsigned char  id ;		// ͨ��ID
	unsigned char  event ;  // �¼������
	unsigned char  bcd[6];  // bcdʱ��
	GpsInfo	   	   gps ;	// λ���ϱ���Ϣ
};

//¼����ʼ����
struct VoiceRecordBody
{
	unsigned char  command;
	unsigned short recordtime;
	unsigned char  saveflag;
	unsigned char  samplerates;
};

////////////////////// ����Χ��  ////////////////////////
// ��������ͷ��
struct AreaSetHeader
{
	unsigned char op  ;
	unsigned char total ;
};

// ��������ͷ
struct AreaHeader
{
	unsigned int    areaid ;  // ����ID
	unsigned short  attr ;	  // �ֶ���������
};

// ��γ�Ƚṹ��
struct LatLonPoint
{
	unsigned int lat ;  // γ��
	unsigned int lon ;  // ����
};

// Բ������
struct BoundAreaBody
{
 	LatLonPoint		local ;   // λ��
 	unsigned int    raduis ;   // �뾶
};

// ������������
struct RangleAreaBody
{
	LatLonPoint    left_top ;       // ���϶�
	LatLonPoint    right_bottom ;   // ���¶�
};

// ���������
struct PolygonArea
{
	AreaHeader	area ;			// ��������ͷ
};

// ����ʱ��
struct AreaTime
{
	char starttime[6] ;  // ��ʼʱ��
	char endtime[6] ;    // ����ʱ��
};

// �ٶ�����
struct AreaSpeed
{
	unsigned short speed ;  // �ٶ�
	unsigned char  nlast ;  // ���ٳ���ʱ��
};

////////////// ������· ////////////////////////
struct RoadHeader
{
	unsigned int roadid ;  // ��·ID
	unsigned short attr ;  // ����
};

struct BendPoint   // �յ�����
{
	unsigned int  bendid ;  // �յ�ID
	unsigned int  segid ;   // ·��ID
	LatLonPoint   postion ; // λ��
	unsigned char width ;   // ·��
	unsigned char battr ;   // ·������
};

struct Threshold  // ��ֵ
{
	unsigned short more ;  // ·����ʻ������ֵ
	unsigned short less ;  // ·����ʻ������ֵ
};

//////////////////// �¼����� ////////////////////////
struct EventHeader
{
	unsigned char type ;  // �¼�����
	unsigned char total ; // �¼�����
};

// �¼����ݵĴ���
struct EventContentBody
{
	unsigned char eventid ;  // �¼�ID
	unsigned char length ;	 // �¼����ݳ���
};

///////////////////////��Ϣ�㲥�˵����� /////////////////////////
struct DemandHeader  // �㲥����
{
	unsigned char type ;  // ��������
	unsigned char total ; // ����
};

struct DemandContentBody
{
	unsigned char  mtype ;  // �˵�����
	unsigned short length ; // ����
};

///////////////////////���õ绰��////////////////////////////
struct PhoneHeader
{
	unsigned char type ;  // ��������
	unsigned char total ; // ��ϵ������
};

//////////////////////// ��Ϣ����  ///////////////////////////
struct MsgServiceBody
{
	unsigned char type ;  // ��Ϣ����
	unsigned short len ;  // ��Ϣ����
};

//////////////////////// �����·� ///////////////////////////
struct QuestAskBody // ���ʵ�����
{
	unsigned char flag ;   // �����·���־λ
	unsigned char len  ;   // ����ĳ���
};

// ��ѡ�Ĵ�
struct QuestAnswerBody
{
	unsigned char  aflag ;  // ��־��
	unsigned short alen ;   // �𰸳���
};

///////////////////////// ��ý�����ݴ��� //////////////////////////////
struct MediaDataBody  // ��ý�����ݼ���
{
	unsigned char type ;    // ����
	unsigned char channel ; // ͨ��
	unsigned char event ;  // ����
	char starttime[6] ;     // ��ʼʱ��
	char endtime[6] ;       // ����ʱ��
	MediaDataBody() {
		type    = 0 ;
		channel = 1 ;
		event   = 0 ;
	}
};

struct MediaDataUploadBody // ��ý�������ϴ�
{
	MediaDataBody body ;
	unsigned char flag ;  // �Ƿ�ɾ����־
};

// ��ͨ��ʷ�����ϴ�
struct HistoryDataUploadBody
{
	unsigned short seqid ;  // ��Ӧ����ʷ���ݲ�ѯ�������ˮ��
	unsigned char  type  ;	// ��ʷ��������
	char starttime[6] ;     // ��ʷ������ʼʱ��
	char endtime[6] ;		// ��ʷ���ݽ���ʱ��
	unsigned char  num ;    // ��ʷ������ĸ���
};

// ��ͨ�ϴ���ʷ����Ӧ��
struct HistoryDataUploadAck
{
	GBheader header ;
	unsigned short seqid ;  // ��Ӧ���
	unsigned char  num ;	// �ش�����
};

// ��ͨ��������Ч���ݸ�ʽ����
struct EngneerData
{
	char time[6] ;  // ʱ��
	unsigned short speed ;   // ת��
	unsigned char  torque ;  // Ť��
	unsigned char  position ; // ����̤��λ��
};

// ��ͨ��ʷ���ݲ�ѯ
struct QueryHistoryDataBody
{
	unsigned char type ; // ����
	char starttime[6] ;  // ��ʼʱ��
	char endtime[6] ;    // ����ʱ��
	unsigned short num ; // ����
};

// ��ͨ��ʻ��Ϊ�¼�
struct DriverActionEvent
{
	GBheader header ;
	unsigned char type ; // ��Ϊ�¼�ID
	GpsInfo       startgps ; // ��ʼλ��
	GpsInfo 	  endgps ; // ����λ��
	GBFooter      footer ; //
};

typedef struct tagInfoVersion
{
	unsigned char id;//��ϢID
	unsigned char len;//��Ϣ����
	char          buf[0];//��Ϣ����
}VERSION_IINFO;

struct ProgramVersionEvent
{
	GBheader header;
	unsigned char count;//��Ϣ����
};

struct IdleSpeed //����Э��
{
	GBheader header;
    unsigned char byStatus;//״̬
    char byKey[64];//��Կ
    unsigned short wVehicleTurnSpeed;//����ת��
};

struct SpeedAdjustReport {
	unsigned char btime[6];
	unsigned char etime[6];
	unsigned short gpsSpeed;
	unsigned short vssSpeed;
	unsigned short recommend;
};

#pragma pack()
#endif /* GBPROTOPARSE_H_ */

