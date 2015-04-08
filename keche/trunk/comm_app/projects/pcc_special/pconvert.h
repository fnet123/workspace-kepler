/**********************************************
 * pccutil.h
 *
 *  Created on: 2011-08-04
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments:
 *********************************************/

#ifndef __PCONVERT_H__
#define __PCONVERT_H__

#include "interface.h"
#include <map>
#include <string>
#include <Mutex.h>
#include <ProtoHeader.h>

#ifdef  PHONE_LEN
#undef  PHONE_LEN
#endif
#define CAR_TYPE        4 //4C54
#define PHONE_LEN   	11
#define PLATFORM_ID  	"ctfov00001"

#define METHOD_OTHER     0
#define METHOD_REG   	 1

class PConvert
{
	class CSequeue
	{
	public:
		CSequeue():_seq_id(0) {}
		~CSequeue() {}

		// ȡ������
		unsigned int get_next_seq( void ) {
			share::Guard g( _mutex ) ;
			if ( _seq_id >= 0xffffffff ) {
				_seq_id = 0 ;
			}
			return ++ _seq_id ;
		}

	private:
		// ����������
		share::Mutex  _mutex ;
		// ����ID��
		unsigned int  _seq_id ;
	};
	typedef std::map<std::string,std::string>   MapString;
public:
	PConvert() ;
	~PConvert() ;

	// ��ʼ����������
	void   initenv( ISystemEnv *pEnv ) ;
	// ת������
	char * convert_urept( const string &key, const string &ome, const string &phone, const string &val, int &len , unsigned int &msgid , unsigned int &type ) ;
	// ����D_CTLM
	char * convert_dctlm( const string &key, const string &val, int &len , unsigned int &msgid ) ;
	// ����D_SNDM
	char * convert_dsndm( const string &key, const string &val, int &len , unsigned int &msgid ) ;
	// ����ͨ��Ӧ��
	char * convert_comm( const string &key, const string &phone, const string &val, int &len, unsigned int &msgid ) ;
	// ת�����Э��
	char * convert_lprov( const string &key, const string &seqid, const string &val , int &len, string &areacode ) ;
	// �ͷŻ���
	void   free_buffer( char *buf ) ;
	// ȡ���ֻ��ź�OME��
	bool get_phoneome( const string &macid, string &phone, string &ome ) ;
	// ͨ��������MAC��ȡ�ó��ƺź���ɫ
	bool get_carinfobymacid( const string &macid, unsigned char &carcolor, string &carnum ) ;

public:
	// ��GnssDataת�ɼ������
	static void build_gps_info( string &dest, GnssData *gps_data ) ;

private:
	// ת��GPS����
	bool convert_gps_info( MapString &map, GnssData &gps ) ;
	// �����صĲ���
	bool parse_jkpt_value( const std::string &param, MapString &val ) ;
	// ȡ�ö�ӦMAP�ַ�ֵ
	bool get_map_string( MapString &map, const std::string &key , std::string &val ) ;
	// ȡ�ö�ӦMAP������ֵ
	bool get_map_integer( MapString &map, const std::string &key , int &val ) ;
	// ȡ��ͷ����
	bool get_map_header( const std::string &param, MapString &val, int &ntype ) ;
	// ������ݵ�MAP
	bool split2map( const std::string &s , MapString &val ) ;

private:
	// ����������
	CSequeue  	 _seq_gen ;
	// ��������
	ISystemEnv  *_pEnv ;
	// �Ƿ����⣬��Ϊģ���ն����������ַ�ԭ��
	unsigned int _istester ;
	// ���ش��ͼƬ·��
	string 		 _picdir ;
};

#endif
