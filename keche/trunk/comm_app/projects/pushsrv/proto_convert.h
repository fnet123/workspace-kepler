/******************************************************
 *  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
 *   FileName: proto_convert.h
 *     Author: liubo  2012-5-14 
 *Description:
 *******************************************************/

#ifndef PROTO_CONVERT_H_
#define PROTO_CONVERT_H_

#include "proto_header.h"
#include "databuffer.h"
#include "proto_parse.h"

//#define  MSG_REQUEST_LOGIN                   0x0001 //��¼
//#define  MSG_REQUEST_NOOP                    0x0002 //��·����
//#define  MSG_REQUEST_SUBSCRIBE_INFO          0x0003 //��������
//#define  MSG_REQUEST_LOCATION_INFO_SERVICE   0x0004 //λ���ϱ���Ϣ
//#define  MSG_REQUEST_ONLINE                  0x0005
//#define  MSG_REQUEST_LOCATION                0x0006
//#define  MSG_REQUEST_TRACE                   0x0007
//#define  MSG_REQUEST_TEXT                    0x0008
//#define  MSG_REQUEST_MONITOR                 0x0009
//#define  MSG_REQUEST_MEDIA_UP                0x000a
//#define  MSG_REQUEST_GET_PHOTO               0x000b
//#define  MSG_REQUEST_RECORD                  0x000c

class ConvertProto
{
public:
	typedef struct _NewProtoOut
	{
		string mac_id;
		DataBuffer data_buffer;
		MSG_TYPE msg_type;
	} NewProtoOut;

	typedef struct _InterProtoOut
	{
        string mac_id;
        string seq;
        string msg;
	} InterProtoOut;

	typedef bool (ConvertProto::*NewConvertFun)(NewProto *, InterProtoOut *);

	ConvertProto()
	{
		_msg_seq = 0;
	}

	~ConvertProto(){}

	void init();

	bool InterProto2NewProto(InterProto *inter_proto, NewProtoOut *out);

	bool NewProto2InterProto(NewProto *new_proto, InterProtoOut *out);

	bool BuildNewProto(unsigned short msg_type,  unsigned short msg_len,
			const char *msg_data, DataBuffer *data_buffer);

	bool BuildNewNoop(DataBuffer *data_buffer);

	bool BuildNewCommResp(unsigned short msg_type, unsigned int seq,  unsigned char ret, DataBuffer *data_buffer);

private:

	NewConvertFun GetNewConvertFun(unsigned short msg_type);


	//����
	bool NewConvertReqGpsInfo(NewProto *new_proto, InterProtoOut *out);

	//����
	bool NewConvertMonitor(NewProto *new_proto, InterProtoOut *out);

	//λ�ò�ѯ
	bool NewConvertTrace(NewProto *new_proto, InterProtoOut *out);

	//�·�����
	bool NewConvertText(NewProto *new_proto, InterProtoOut *out);

	//��ȡ����
	bool NewConvertGetParam(NewProto *new_proto, InterProtoOut *out);

	//���ò���
	bool NewConvertSetParam(NewProto *new_proto, InterProtoOut *out);

	//�ն˿���
	bool NewConvertTermControl(NewProto *new_proto, InterProtoOut *out);

	//�¼�����
	bool NewConvertSetEvent(NewProto *new_proto, InterProtoOut *out);

	//�����·�
	bool NewConvertQuestionAsk(NewProto *new_proto, InterProtoOut *out);

	//��Ϣ�㲥�˵�����
	bool NewConvertInfoMenu(NewProto *new_proto, InterProtoOut *out);

	//��Ϣ�����·�
	bool NewConvertInfoSend(NewProto *new_proto, InterProtoOut *out);

	//���õ绰��
	bool NewConvertPhoneBook(NewProto *new_proto, InterProtoOut *out);

	//��������
	bool NewConvertCarControl(NewProto *new_proto, InterProtoOut *out);

	//����Բ������
	bool NewConvertSetCircle(NewProto *new_proto, InterProtoOut *out);

	//ɾ��Բ������
	bool NewConvertDelCircle(NewProto *new_proto, InterProtoOut *out);

	//���þ�������
	bool NewConvertSetRectangle(NewProto *new_proto, InterProtoOut *out);

	//ɾ����������
	bool NewConvertDelRectangle(NewProto *new_proto, InterProtoOut *out);

	//���ö��������
	bool NewConvertSetPolygon(NewProto *new_proto, InterProtoOut *out);

	//ɾ�����������
	bool NewConvertDelPolygon(NewProto *new_proto, InterProtoOut *out);

	//����·��
	bool NewConvertSetLine(NewProto *new_proto, InterProtoOut *out);

	//ɾ��·��
	bool NewConvertDelLine(NewProto *new_proto, InterProtoOut *out);

	//��ʻ��¼���ݲɼ�����
	bool NewConvertDriveCollect(NewProto *new_proto, InterProtoOut *out);

	//��ʻ��¼�����´�����
	bool NewConvertDriveParam(NewProto *new_proto, InterProtoOut *out);

	//��ý�����ݼ���
	bool NewConvertMediaSearch(NewProto *new_proto, InterProtoOut *out);

	//ָ����ý�������ϴ�
	bool NewConvertSingleUpload(NewProto *new_proto, InterProtoOut *out);

	//�洢��ý�������ϴ�
	bool NewConvertMultiUpload(NewProto *new_proto, InterProtoOut *out);

	//����
	bool NewConvertGetPhoto(NewProto *new_proto, InterProtoOut *out);

	//¼��
	bool NewConvertRecord(NewProto *new_proto, InterProtoOut *out);

	//ͨ�ûظ�
	bool InterConvertResp(InterProto *inter_proto, NewProtoOut *out);

	//�ڲ�λ����Ϣ��ת��
	bool InterConvertLocation(InterProto *inter_proto, NewProtoOut *out);

	//������֪ͨ
	bool InterConvertQuery(InterProto *inter_proto, NewProtoOut *out);

	//��ý���ϴ�
	bool InterConvertMedia(InterProto *inter_proto, NewProtoOut *out);

	//��ѯ�ն˲���Ӧ��
	bool InterConvertGetParam(InterProto *inter_proto, NewProtoOut *out);

	//�¼�����
	bool InterConvertEventReport(InterProto *inter_proto, NewProtoOut *out);

	//����Ӧ��
	bool InterConvertQuestionAck(InterProto *inter_proto, NewProtoOut *out);

	//��Ϣ�㲥/ȡ��
	bool InterConvertInfoResp(InterProto *inter_proto, NewProtoOut *out);

	//�����˵��ϱ�
	bool InterConvertListReport(InterProto *inter_proto, NewProtoOut *out);

	//��ʻԱ��ݲɼ�
	bool InterConvertIdentityCollect(InterProto *inter_proto, NewProtoOut *out);

	//��ý���¼��ϴ�
	bool InterConvertMediaEvent(InterProto *inter_proto, NewProtoOut *out);

	//�洢��ý�����ݼ���Ӧ��
	bool InterConvertSearchResp(InterProto *inter_proto, NewProtoOut *out);

	//��������͸��
	bool InterConvertTransDeliver(InterProto *inter_proto, NewProtoOut *out);

	unsigned int StringToInteger(string &s_value, int nType);

	unsigned int get_msg_seq()
	{
		return _msg_seq++;
	}

	bool parseItem(const string &text, vector<string> &res, char lc, char rc);

private:
    unsigned _msg_seq;

    CNewProtoParse _new_parse;
    CInterProtoParse _inter_parse;


    map<unsigned short, NewConvertFun> _new_convert_table;
};

#endif /* PROTO_CONVERT_H_ */
