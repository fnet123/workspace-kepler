/**********************************************
 * ProtoParse.h
 *
 *  Created on: 2010-7-8
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/

#ifndef __PROTOPARSE_H_
#define __PROTOPARSE_H_

#include "ProtoHeader.h"
#include "BaseTools.h"
#include <iostream>
using namespace std;

#define TRANS_ALL 1
#define TRANS_CAS 2
#define TRANS_PAS 3

#define FROM_CAS 0
#define FROM_PAS 1

const char *get_type(unsigned short msg_type);

class ProtoParse
{
public:
	ProtoParse()
	{
		seq = 0;
	}
	unsigned int get_next_seq()
	{
		return seq++;
	}
	//�����ⲿ���������Ϣ��accesscode:�������ͣ�ҵ�����ͣ�����ID
	static string Decoder(const char*data,int len);
	string get_mac_id(	char vehicle_no[21],unsigned char vehicle_color)
	{
		char buffer[32] = {0};
		sprintf(buffer,"%d_",vehicle_color);
		strncpy(buffer+strlen(buffer),vehicle_no,21);
		string str(buffer);
		return str;
	}
	string GetMacId(const char*data,int len);
	string GetCommandId(const char *data,int len);
	// ��������
	static void BuildHeader( Header &header, unsigned int msg_len, unsigned int msg_seq, unsigned int msg_type , unsigned int access_code ) ;

private:
	unsigned int seq;
};

// 5B����ת����
#define  MAX_BUFF  1024 // ������󻺴�ռ�
class C5BCoder
{
public:
	C5BCoder() ;
	virtual ~C5BCoder() ;
	// ת�����룬����ܴ�����㷨
	bool Encode( const char *data, const int len ) ;
	// ����
	bool Decode( const char *data, const int len ) ;

	// ȡ�ý���ĳ���
	const int    GetSize() ;
	// ȡ������
	const char * GetData() ;

private:
	// ���ݳ���
	int   _len  ;
	// ��������
	char *_data ;
	// ����BUF
	char _buf[MAX_BUFF] ;
	// ������ʱָ��
	char *_temp ;
};

#define BUFF_MB  1024*1024  // ���ڴ�����ܵĳ���
// ���ܴ�����
class CEncrypt
{
public:
	// ���ܴ���
	static bool encrypt( unsigned int M1, unsigned int IA1, unsigned int IC1, unsigned char *buffer, unsigned int size ) ;
	// ������Կ����
	static unsigned int rand_key( void ) ;
};

#endif /* PROTOPARSE_H_ */
