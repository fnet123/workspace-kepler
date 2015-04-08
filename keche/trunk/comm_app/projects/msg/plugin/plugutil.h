/*
 * plugutil.h
 *
 *  Created on: 2012-5-30
 *      Author: humingqing
 *  Memo: һЩ���õ��ַ���ֵĺ���
 */

#ifndef PLUGUTIL_H_
#define PLUGUTIL_H_

#include <map>
#include <string>
#include <tools.h>

class CPlugUtil
{
	typedef std::map<std::string,std::string>  MapString ;
public:
	CPlugUtil(){}
	~CPlugUtil(){}

	// ��������
	bool parse( const char *param ) ;
	// ȡ�����ε�����
	bool getinteger( const char *key, int &value ) ;
	// ȡ���ַ����ε�����
	bool getstring( const char *key, string &value ) ;

private:
	// �������ƽ̨�Ĳ���
	bool parsevalue( const std::string &param, MapString &val ) ;
	// ���KEY-VALUE������
	bool split2map( const std::string &s , MapString &val ) ;

private:
	// ȡ�ö�Ӧֵ
	MapString  _kv ;
} ;


#endif /* PLUGUTIL_H_ */
