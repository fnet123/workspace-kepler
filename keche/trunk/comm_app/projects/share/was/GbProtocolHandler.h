/**********************************************
 * GbProtocolHandler.h
 *
 *  Created on: 2011-09-22
 *      Author: humingqing
 *    Comments: ʵ���µ�808Э�鲿�ݴ���
 *********************************************/

#ifndef __GBPROTOCOLHANDLER_H_
#define __GBPROTOCOLHANDLER_H_

#include <std.h>
#include "GBProtoParse.h"
#include "BaseTools.h"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <Ref.h>
#include <databuffer.h>

#define   GB_PACK_SIZE             500

//�ж�ĳλ�ǲ���Ϊ1������Ǵ���0������Ϊ0�����޸�valueֵ
#define  IS_BIT(value,num)   (value &(0x1<<num))

//ĳλ��ֵΪ1���޸�valueֵ
#define  S_BIT(value,num)   (value |=(0x1<<num))

//ĳλ��ֵΪ0���޸�valueֵ
#define  C_BIT(value,num)   (value &=(~(0x1<<num)) )

using namespace std;

// ��ʻԱ�����Ϣ
struct DRIVER_INFO
{
	char drivername[256] ;
	char driverid[21] ;
	char driverorgid[41] ;
	char orgname[256] ;
};

class GbProtocolHandler : public share::Ref
{
	 /*
	 *   ���ַ���ת����ʮ�����ơ��� "FFFFFFFF" ת����  0xffffffff
	 */
	static int atohex(char *str);
	static GbProtocolHandler * _instance ;

public:
	static GbProtocolHandler * getInstance() ;
	static void FreeHandler( GbProtocolHandler *inst ) ;

public:
	GbProtocolHandler(){}

	virtual ~GbProtocolHandler(){} ;

	//��ӡͷ�������Ϣ
	// void HeaderDecode(GBheader *header, int len, int flag);

	PlatFormCommonResp BuildPlatFormCommonResp(const GBheader*reqheaderptr,
			unsigned short downreq,unsigned char result);
	//ת��Ϊ�ڲ��� λ�ð�
	string ConvertGpsInfo(GpsInfo*gps_info, const char *append_data, int append_data_len);

	//�����ò���ת��Ϊ�ڲ�Э��
	bool  ConvertGetPara(char *buf, int buf_len,string &data);

	// ��ȡ��ʻԱ�����Ϣ
	bool  GetDriverInfo( const char *buf, int len, DRIVER_INFO &info ) ;

	//�Ѽ�ʻԱ��Ϣת��Ϊ�ڲ�Э��
	string  ConvertDriverInfo(char *buf, int buf_len, unsigned char result );

	// ת����������Ч����
	string ConvertEngeer( EngneerData *p ) ;
	// ת����ʻ��Ϊ�¼�
	string ConvertEventGps( GpsInfo *gps ) ;

	// ������������
	bool  buildParamSet( DataBuffer *pbuf , map<string,string> &map, unsigned char &pnum ) ;

public:
	unsigned char get_check_sum( const char *buf, int len ) ;

	string get_bcd_time(char bcd[6]);
	//20110304
	string get_date();
	//050507
	string get_time();

private:
	// ����λ�������й����Զ�����Ϣ����
	bool getCommonExtend(const unsigned char *ptr, int len, map<string, string> &mp);
};

#endif /* GBPROTOCOLHANDLER_H_ */
