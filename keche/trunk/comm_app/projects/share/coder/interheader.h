/*
 * interheader.h
 *
 *  Created on: 2012-4-26
 *      Author: think
 *  ������Ҫ��ʵ��ͨ�õ�Э�������ԾͶ�����ͨ��Э��ͷ������typeΪЭ�������Ҫ���ֲ�ͬ��ʽ��Э���
 */

#ifndef __INTERHEADER_H__
#define __INTERHEADER_H__

#pragma pack(1)

struct interheader
{
	unsigned char  tag ;   // ���ͷ����ʶ 0x5b
	unsigned short len ;   // ���ݳ��ȣ�����������ͷ��β
	unsigned char  flag ;  // �Ƿ���ܱ�ʶ
	unsigned char  type ;  // Э������

	interheader()
	{
		tag = 0x5b ;
	}
};

struct interfooter
{
	unsigned char footer ;  // ���β��ʶ 0x5d

	interfooter()
	{
		footer = 0x5d ;
	}
};

////////////////////////////////// �ļ�����  ///////////////////////////////////////
/**
 *  ������Ҫʵ��һ�������ļ����������Ҫʵ�����Ʊ����ļ��洢��ʽ�Ĺ���
 *  ʵ��ͳһ��open��write��read��close�ĸ��ӿ���ʵ���ļ��洢��д����
 */
// ʹ�������ַ��İ汾�Ź���
#define BIG_VER_0    		0x01   		// 0�ַ�ֵ
#define BIG_VER_1	 		0x01    	// 1�ַ�ֵ
#define BIG_ERR_SUCCESS   	0x00		// �ɹ�
#define BIG_ERR_FAILED		0x01		// ʧ��

// �ļ��������������
#define BIG_OPEN_REQ  		0x0001  	// ��½����
#define BIG_OPEN_RSP  		0x8001  	// ��½��Ӧ
#define BIG_WRITE_REQ  		0x0002  	// д�ļ�����
#define BIG_WRITE_RSP  		0x8002  	// д�ļ��ɹ�Ӧ��
#define BIG_READ_REQ		0x0003  	// ���ļ�����
#define BIG_READ_RSP   		0x8003  	// ���ļ�Ӧ��
#define BIG_CLOSE_REQ  		0x0004 		// �˳���½
#define BIG_CLOSE_RSP  		0x8004 		// �˳���½�ɹ�Ӧ��

// ����ͨ��ͷ��
struct bigheader
{
	unsigned char  	 ver[2] ;  // �汾�Ź���
	unsigned int   	 seq ;	   // ͳһ�������
	unsigned short   cmd ;     // ���������
	unsigned int     len ;	   // ���ݳ���

	bigheader()
	{
		ver[0] = BIG_VER_0 ;
		ver[1] = BIG_VER_1 ;
		seq    = 0 ;
		cmd    = 0 ;
		len    = 0 ;
	}
};

// ��½����
struct bigloginreq
{
	unsigned char user[20] ;
	unsigned char pwd[20] ;
};

// ��½��Ӧ
struct bigloginrsp
{
	unsigned char result ;
};

// д��������
struct bigwritereq
{
	unsigned char path[256] ;
	unsigned int  data_len ;
	// ����������
};

// д������Ӧ
struct bigwritersp
{
	unsigned char result ;
};

// ����������
struct bigreadreq
{
	unsigned char path[256] ;
};

// ��������Ӧ
struct bigreadrsp
{
	unsigned char result ;
	unsigned int  data_len ;
	// ��Ӧ��������
};

#pragma pack()

#endif /* INTERHEADER_H_ */
