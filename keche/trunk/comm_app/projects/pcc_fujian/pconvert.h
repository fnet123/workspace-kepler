/*
 * pconvert.h
 *
 *  Created on: 2012-3-2
 *      Author: think
 *
 *  ʵ���ڲ�Э��ת�ɽ��ռ��ƽ̨Э��
 */

#ifndef __PCONVERT_H__
#define __PCONVERT_H__

#include <string>
#include <vector>
#include <map>
#include <databuffer.h>
#include <Mutex.h>
#include "header.h"
#include <Session.h>

using namespace std ;

class ISystemEnv ;
class PConvert
{
	class CSequeueGen
	{
	public:
		CSequeueGen():_id(0){}
		~CSequeueGen(){}

		unsigned int get_next_seq(void) {
			unsigned int tmp = 0 ;
			_mutex.lock() ;
			tmp = ++ _id ;
			_mutex.unlock() ;
			return tmp ;
		}

	private:
		share::Mutex _mutex ;
		unsigned int _id ;
	};
	typedef map<string,string> MapString ;
public:
	PConvert() ;
	~PConvert() ;

	// ��ʼ����������ָ��
	bool initenv( ISystemEnv *pEnv ) ;
	// ת��U_REPTָ�������
	void convert_urept( const string &macid , const string &val, DataBuffer &buf , bool bcall )  ;
	// ת��ͨ��Ӧ��Ĵ���
	void convert_comm( const string &seqid, const string &macid, const string &val, DataBuffer &buf ) ;
	// ȡ����һ����������
	unsigned int get_next_seq( void ) { return _gen.get_next_seq(); }
	// ����Э��ͷ������
	void buildheader( DataBuffer &buf, const char *phone, unsigned int len , unsigned int result = 0 , unsigned int seq = 0 ) ;
	// ת����������
	bool build_caller( unsigned int seq, const char *phone, const char *data, int len ) ;
	// ת��������Ϣ
	bool build_photo( unsigned int seq, const char *phone, const char *data, int len ) ;
	// �·�������Ϣ
	bool build_sendmsg( unsigned int seq, const char *phone, const char *data, int len ) ;
	// ����ȡ�ص�ͼƬ
	void sendpicture( unsigned int seq, const char *data, const int len ) ;

private:
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
	// ת���ϱ���GPS������
	bool convertgps( const char *szphone, MapString &mp, DataBuffer &buf , bool bcall ) ;
	// ����GPS������
	bool buildgps( MapString &mp, _Gps &gps ) ;
	// ͨ��MACIDȡ�ö�Ӧ���ն˿�
	bool gettermidbymacid( const string &macid, char *szphone ) ;

private:
	// ϵͳ��������
	ISystemEnv * _pEnv ;
	// ���в�����
	CSequeueGen  _gen ;
	// ��MACIDת���ն˺Ŷ�Ӧ��ϵ
	CSessionMgr  _cache;
	// ��������ļ�·��
	string 		 _picdir ;
};


#endif /* PCONVERT_H_ */
