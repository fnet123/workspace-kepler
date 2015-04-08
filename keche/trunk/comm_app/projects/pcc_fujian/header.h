/*
 * header.h
 *
 *  Created on: 2012-8-13
 *      Author: humingqing
 */

#ifndef __HEADER_H__
#define __HEADER_H__

#define PACK_TAG   "&&"

#pragma pack(1)

struct _Header
{
	unsigned char  tag[2] ;  	// && ��������&����ʾһ�����ĵĿ�ʼ
	unsigned short flag ;    	// 0λ��ʾ��Ϣ����RSA����,1λ��ʾ��Ϣ���еľ�γ�Ⱦ������ܣ�����λ����
	unsigned short mid ; 	 	// ��ʶ�����豸�ĳ��ң����޶�����������0x0
	unsigned char  termid[8] ;  // ���ڱ�ʾ�ն˺ţ���ѹ��BCD���ʾ��ͨ���ն���SIM�ű�ʾ��λ�������0x0
	unsigned int   result ; 	// ��ƽ̨������Ӧ���ݣ���������Ӧ����һһ��Ӧ
	unsigned int   seq ;  		// ������˳���0��ʼѭ���ۼӣ����û��Զ�����ţ����м�����һһ��Ӧ
	unsigned int   len ;    	// ������ĳ��ȣ���λ��ǰ
};

struct _Gps
{
	unsigned char  time[6] ; 	// GPS��BCD��ʱ��
	unsigned int   state ; 		// ״̬
	unsigned int   lon ;   		// ����
	unsigned int   lat ;	  	// γ��
	unsigned short speed ; 		// �ٶ�
	unsigned char  direction ;  // ����
	unsigned short height ;  	// ����
	unsigned short distance ;  	// ��ʻ����
	unsigned int   mile ;  	 	// �����
};

struct _CallReq
{
	unsigned short cmd ;  // 0x0102
};

struct _CallRsp
{
	unsigned short cmd ;  // 0x0182
	_Gps 		   gps ;
};

struct _PhotoReq
{
	unsigned short cmd ; 		// 0x0301
	unsigned char  cameraid ;  	// ����ͷID	1Byte	���ڶ�·���ƣ���·ʱ�� 1����·��λ������Ӧ����ͷ
	unsigned short num ; 		// ����	1Word	0��ʾ�������
	unsigned short time; 		// ���	1Word	��λ����
	unsigned char  quality;    // ͼ������	1Byte	0~5, ����Խ��ͼƬ����Խ�ã�������ļ�ҲԽ��
	unsigned char  brightness; // ͼ������	1Byte	0~255
	unsigned char  contrast;   // �Աȶ�	1Byte	0~127
	unsigned char  saturation; // ���Ͷ�	1Byte	0~127
	unsigned char  chroma;     // ɫ��	1Byte	0~255
};

struct _PhotoRsp
{
	unsigned short cmd ;  	  // 0x0381
	unsigned char  cameraid ; // ����ͷID(1)
	_Gps		   gps ;      // GPS����(29)
	unsigned short data_len;  // ͼ�����ݴ�С(2)+ͼ������
};

struct _ScheduleReq
{
	unsigned short cmd ;  // 0x0401
	// ���������ı���Ϣ
};

struct _ScheduleRsp
{
	unsigned short cmd ;  // 0x0481  ������Ӧ
};

struct _UploadGps
{
	unsigned short cmd ;  // 0x0181 GPSλ���ϴ�
	_Gps		   gps ;  // λ��������Ϣ
};

#pragma pack()


#endif /* HEADER_H_ */
