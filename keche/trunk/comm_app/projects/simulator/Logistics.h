/**
 * Author: xifengming
 * date:   2012-11-17
 * memo:   ����ģ������
 */
#ifndef _LOGISTICS_H
#define _LOGISTICS_H

#include <tools.h>
#include <databuffer.h>
#include <vector>
#include <arpa/inet.h>
#include <GBProtoParse.h>

using namespace std;

#define UPLOAD_DATAINFO_REQ	          0x1022 //����Զ��ϱ�
#define UP_CARDATA_INFO_REQ           0x1023 //�ϴ������Ϣ
#define UP_ORDER_FORM_INFO_REQ        0x1027 //�ϴ����˶�����
#define UP_TRANSPORT_FORM_INFO_REQ    0x1028 //�ϴ��˵����ȵ�
#define TERMINAL_COMMON_RSP           0x1000 //�ն�ͨ�ûظ�
#define PLATFORM_COMMON_RSP           0x8000 //ƽ̨ͨ�ûظ�
#define SEND_CARDAT_INFO_REQ                 0x1021   //�·������Ϣ
#define SEND_TRANSPORT_ORDER_FORM_INFO_REQ   0x1025   //�·��˵���Ϣ
#define SEND_CARDATA_INFO_CONFIRM_REQ        0x1024   //����ɽ�״̬ȷ��(�·�)

#pragma pack(1)

struct TransHeader  // ����͸����808ͷ������
{
	GBheader header ;
	uint8_t  type;

	TransHeader() {
		type = 0x01 ;
	}
};

typedef struct tagMsgHeader
{
	unsigned short wMsgVer; //Э���ڲ��汾��
	unsigned short wMsgType; //���ĵ�����
	unsigned int dwMsgSeq; //�������
	unsigned int dwDataLen; //�����峤��
} MSG_HEADER;

/*���״̬�ϱ�*/
typedef struct tagAutoDataSchedule
{
	unsigned char byState; //״̬
	unsigned char bySpace; //���ؿռ�
	unsigned short w_weight; //������
	unsigned char byTime[6]; //����ʱ��
	unsigned int dwSarea; //������
	unsigned int dwDarea; //Ŀ�ĵ�
} AUTO_DATA_SCHEDULE;

/*�ϴ������Ϣ*/
typedef struct tagCarDataInfo
{
	unsigned char _sid[20]; //String  �������
	unsigned char _status; //״̬(0.�ѱ��� 1.�ܾ� 2.ȡ��)
	unsigned int _price; //����(����0.01 Ԫ),���״̬Ϊ0���и��ֶ�
} CAR_DATA_INFO;

/*�ϴ�����״̬*/
typedef struct tagOrderFormInfo
{
	unsigned char _sid[20]; //String ½����ˮ��
	unsigned char _order_form[20]; //String ��������
	unsigned char _action; //���ȵ�����(0.��� 1.ǩ��)
	unsigned char _status; //״̬(0.���� 1.�쳣)
	unsigned int _lon; //����
	unsigned int _lat; //γ��
} ORDER_FORM_INFO;

/*�ϴ��˵�״̬*/
typedef struct tagTransportFromInfo
{
	unsigned char _sid[20]; //String ½����ˮ��
	unsigned char _action; //���ȵ�����(0.��� 1.ǩ��)
	unsigned char _status; //״̬(0.���� 1.�쳣)
	unsigned int _lon; //����
	unsigned int _lat; //γ��
} TRANSPORT_FORM_INFO;

/*ƽ̨ͨ��Ӧ��*/
typedef struct tagPlatFormCommonRsp
{
	unsigned short wType; //��������
	unsigned char byResult; //���Ľ��
} RSP_PLATFORM_COMMON;
#pragma pack()

//------------------------------------------------------------------------
class CLogistics  //����
{
public:
	CLogistics( );
	~CLogistics( );

	bool LoadInitFile( const char *szName );
	int BuildTransportData( unsigned short wType, DataBuffer &pBuf );
	/*�����·����ݰ�*/
	int ParseTransparentMsgData( unsigned char *lpData, int nLen, DataBuffer &pRetuBuf );
private:
	void ParseParam( vector< string > &vec_param );
	int CreateRequestFrame( unsigned short wDataType, unsigned char *lpData, unsigned short nLen, unsigned int nSeq,
			DataBuffer &pBuf );
	/*0x1022 ��ʼ������ϱ�*/
	void Init_Auto_Data_Schedule( vector< string > &vec_param );
	/*0x1023 ��ʼ���ϴ������Ϣ*/
	void Init_Car_Data_Info( vector< string > &vec_param );
	/*0x1027 ��ʼ���ϴ�����״̬*/
	void Init_Order_Form_Info( vector< string > &vec_param );
	/*0x1028 ��ʼ���ϴ��˵�״̬*/
	void Init_Transport_Form_Info( vector< string > &vec_param );
	/*0x8000 ����ƽ̨ͨ��Ӧ��ظ�*/
	void Parse0x8000Frame( unsigned char *lpData );
	/*0x1021 �·������Ϣ*/
	int Parse0x1021Frame( unsigned char *lpSrcData, int msglen, unsigned int nSeq, DataBuffer &pRetBuf );
	//�·��˵���Ϣ
	int Parse0x1025Frame( unsigned char *lpSrcData, int msglen, unsigned int nSeq, DataBuffer &pRetuBuf );
	//����ɽ�״̬ȷ��
	int Parse0x1024Frame( unsigned char *lpSrcData, int msglen, unsigned int nSeq, DataBuffer &pRetuBuf );
	/*�ն�ͨ�ûظ�*/
	void CreateTerminalCommRspFrame( unsigned short wMsgType, unsigned int nSeq, DataBuffer &pBuf );

private:
	unsigned int seq;

	AUTO_DATA_SCHEDULE _auto_data_schedule;
	CAR_DATA_INFO _car_data_info;
	ORDER_FORM_INFO _order_form_info;
	TRANSPORT_FORM_INFO _transport_form_info;
};
#endif
