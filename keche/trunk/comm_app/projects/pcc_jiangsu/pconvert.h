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

#include <qstring.h>
#include "pccsession.h"
#include <string>
#include <vector>
#include <map>

using namespace std ;

static float my_atof( const char *p )
{
	if ( p == NULL )
		return 0 ;
	return atof( p ) ;
}

static int my_atoi( const char *p )
{
	if ( p == NULL )
		return 0 ;
	return atoi( p ) ;
}

class ISystemEnv ;
class PConvert
{
	typedef map<string,string> MapString ;
public:
	PConvert() ;
	~PConvert() ;

	// ��ʼ����������ָ��
	bool initenv( ISystemEnv *pEnv ) ;
	// ת��U_REPTָ�������
	void convert_urept( _stCarInfo &info, const string &val, CQString &buf )  ;
	// ������Э��ת�����ڲ�Э��
	static bool buildintergps( std::vector<std::string> &vec , const char *macid, CQString &buf , CQString &msg ) ;

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
	bool convertgps( _stCarInfo &info, MapString &mp, CQString &buf ) ;

private:
	// ϵͳ��������
	ISystemEnv * _pEnv ;
};


#endif /* PCONVERT_H_ */
