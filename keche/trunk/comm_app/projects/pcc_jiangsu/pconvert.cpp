/*
 * pconvert.cpp
 *
 *  Created on: 2012-3-2
 *      Author: think
 */

#include <stdio.h>
#include "pconvert.h"
#include <comlog.h>
#include <tools.h>
#include "mybase64.h"

PConvert::PConvert()
{
	_pEnv = NULL ;
}

PConvert::~PConvert()
{

}

bool PConvert::split2map( const std::string &s , MapString &val )
{
	vector<string>  vec ;
	// �������ж��ŷָ��
	if ( ! splitvector( s , vec, "," , 0 ) ) {
		return false ;
	}

	string temp  ;
	size_t pos = 0 , end = 0 ;
	// ��������
	for ( pos = 0 ; pos < vec.size(); ++ pos ) {
		temp = vec[pos] ;
		end  = temp.find( ":" ) ;
		if ( end == string::npos ) {
			continue ;
		}
		val.insert( pair<string,string>( temp.substr(0,end), temp.substr( end+1 ) ) ) ;
	}
	// ���������ƽ̨��������
	return ( ! val.empty() ) ;
}

// �������ƽ̨�Ĳ���
bool PConvert::parse_jkpt_value( const std::string &param, MapString &val )
{
	// {TYPE:0,104:��A10104,201:701116,202:1,15:0,26:5,1:69782082,2:23947540,3:5,4:20110516/153637,5:6,21:0}
	size_t pos = param.find("{") ;
	if ( pos == string::npos ) {
		return false ;
	}

	size_t end = param.find("}", pos ) ;
	if ( end == string::npos || end < pos + 1 ) {
		return false ;
	}
	// ���������ƽ̨��������
	return split2map( param.substr( pos+1, end-pos-1 ), val ) ;
}

// ȡ��ͷ����
bool PConvert::get_map_header( const std::string &param, MapString &val, int &ntype )
{
	if ( ! parse_jkpt_value( param, val ) ) {
		OUT_ERROR( NULL, 0, NULL, "parse data %s failed", param.c_str() ) ;
		return false ;
	}

	if ( ! get_map_integer( val, "TYPE", ntype ) ) {
		OUT_ERROR( NULL, 0, NULL, "get data %s type failed", param.c_str() ) ;
		return false ;
	}

	return true ;
}

bool PConvert::get_map_string( MapString &map, const std::string &key , std::string &val )
{
	MapString::iterator it = map.find( key ) ;
	if ( it == map.end() ) {
		return false ;
	}
	val = it->second ;
	return true ;
}

bool PConvert::get_map_integer( MapString &map, const std::string &key , int &val )
{
	MapString::iterator it = map.find( key ) ;
	if ( it == map.end() ) {
		return false ;
	}
	val = atoi( it->second.c_str() ) ;
	return true ;
}

bool PConvert::initenv( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	return true ;
}

// ת��U_REPTָ�������
void PConvert::convert_urept( _stCarInfo &info, const string &val, CQString &buf )
{
	int ntype = 0 ;
	MapString map ;
	if ( ! get_map_header( val, map, ntype ) ) {
		OUT_ERROR( NULL, 0, NULL, "PConvert::convert_urept get map header failed, %s:%d",  __FILE__, __LINE__ ) ;
		return;
	}

	switch(ntype) {
		case 0:  //·���ͱ�������    λ�û㱨
		case 1:
			{
				// ToDo: ��λ���ϱ���ϢתΪ��Ӧ��Э��
				convertgps( info, map, buf ) ;
			}
			break;
		case 2: // ��ʻ��¼��
			{
				// ToDo:
			}
			break;
		case 3: // �����ϴ�ͼƬ���������������⴦��һ��
			{
				// ToDo:
			}
			break ;
		case 8: // �����ϱ���ʻԱ��Ϣ�ɼ�
		   {
			   // ToDo:
		   }
		   break;
		case 35: // �����ϱ������˵�
			{
				// ToDo:
			}
			break;
		case 36: // �ն�ע��,������ʱֻ�����ն˼�Ȩ
			{
				// ToDo:
			}
			break;
		case 38: //�ն˼�Ȩ��Ҫ�첽�ص�ͨ��http,ͨ��pcc �ύ��Service ����sim����,�ն�����
			{
				// ToDo:
			}
			break;
		default:
			{
				OUT_ERROR( NULL, 0, info.macid.c_str(), "PConvert::convert_urept not support %s, %s:%d", val.c_str(), __FILE__, __LINE__ ) ;
			}
			break ;
	}
}

// ���λ�Ƿ���ֵ
static bool isset( unsigned int nval, unsigned int pos )
{
	unsigned int n = ( 1 << pos ) ;
	return ( nval & n ) ;
}

// ת���ϱ���GPS������
bool PConvert::convertgps( _stCarInfo &info, MapString &mp, CQString &buf )
{
	if ( mp.empty() )
		return false ;

	int nval = 0 ;
	if ( ! get_map_integer( mp, "1", nval ) ) {
		return false ;
	}
	float flon = (float)( nval * 10 / 6 ) / (float) 1000000;

	if ( ! get_map_integer( mp, "2", nval ) ) {
		return false ;
	}
	float flat = (float)( nval * 10 / 6 ) / (float) 1000000;

	// �������ݿ�ʼֵ
	buf.AppendBuffer( "*" ) ;
	buf.AppendBuffer( _pEnv->GetPasClient()->GetSrvId() ) ;
	buf.AppendBuffer( "|" ) ;
	buf.AppendBuffer( "GPSPosInfo|" ) ;
	buf.AppendBuffer( "1.0.4|" ) ;
	buf.AppendBuffer( info.areacode.c_str() ) ;
	buf.AppendBuffer( "|" ) ;
	buf.AppendBuffer( info.color.c_str() ) ;
	buf.AppendBuffer( "|" ) ;
	buf.AppendBuffer( info.carmodel.c_str() ) ;
	buf.AppendBuffer( "|" ) ;
	buf.AppendBuffer( info.vehicletype.c_str() ) ;
	buf.AppendBuffer( "|" ) ;

	CBase64Ex base64 ;
	base64.Encode( info.vehiclenum.c_str(), info.vehiclenum.length() ) ;
	buf.AppendBuffer( base64.GetBuffer(), base64.GetLength() ) ;
	buf.AppendBuffer( "|" ) ;

	int alam = 0 ;
	get_map_integer( mp, "20", alam ) ;
	if ( isset(alam, 0 ) ) {
		buf.AppendBuffer( "1" ) ;
	} else if ( isset( alam, 1 ) ) {
		buf.AppendBuffer( "2" ) ;
	} else if ( isset( alam, 20 ) ) {
		buf.AppendBuffer( "3" ) ;
	} else if ( isset( alam, 26) ) {
		buf.AppendBuffer( "4" ) ;
	} else if ( isset( alam, 4) || isset(alam, 5) || isset( alam, 6 ) ) {
		buf.AppendBuffer( "5" ) ;
	}else{
		buf.AppendBuffer( "0" ) ;
		alam = 0 ;
	}
	buf.AppendBuffer( "|0|" ) ;

	char tmp[256] = {0};
	sprintf( tmp, "%.4f", flon ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	sprintf( tmp, "%.4f", flat ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	if ( get_map_integer( mp, "3", nval ) ) { // �ٶ�808��Ϊ1/10m/s
		nval = nval / 10 ;
	}
	float fspeed = (float) ( nval * 3.6 ) ;
	sprintf( tmp, "%.2f", fspeed ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	get_map_integer( mp, "6", nval ) ;  // �߶�
	sprintf( tmp, "%d", nval ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	get_map_integer( mp, "5", nval ) ;  // ����
	sprintf( tmp,"%d", nval ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	time_t ntime = 0 ;
	string sval ;
	if ( get_map_string( mp, "4", sval ) ) {
		int nyear = 0 , nmonth = 0 , nday = 0 , nhour = 0 ,nmin = 0 , nsec = 0 ;
		sscanf( sval.c_str(), "%04d%02d%02d/%02d%02d%02d", &nyear, &nmonth, &nday, &nhour, &nmin, &nsec ) ;

		struct tm tm ;
		tm.tm_year = nyear - 1900 ;
		tm.tm_mon  = nmonth - 1 ;
		tm.tm_mday = nday ;
		tm.tm_hour = nhour ;
		tm.tm_min  = nmin ;
		tm.tm_sec  = nsec ;

		ntime = mktime(&tm);
	}
	sprintf( tmp, "%lu", ntime ) ;
	buf.AppendBuffer( tmp ) ;
	buf.AppendBuffer( "|" ) ;

	get_map_integer( mp, "8", nval ) ;
	if ( alam ) {
		buf.AppendBuffer( "1" ) ;
	}else if( isset(nval,0) ) {
		buf.AppendBuffer( "0" ) ;
	} else {
		buf.AppendBuffer( "2" ) ;
	}
	buf.AppendBuffer( "|" ) ;

	buf.AppendBuffer( "-1|" ) ;   // װ������޷��ṩ
	base64.Encode( "�н���·" , 8 ) ;  // ��ʶΪ�н���·����
	buf.AppendBuffer( base64.GetBuffer(), base64.GetLength() ) ;
	buf.AppendBuffer( "#" ) ;

	return true ;
}

// ���ַ���ʱ��תΪBCDʱ��
static const string convert2intertime( const string &time )
{
	if ( time.empty() )
		return "" ;

	time_t ntime  = 0 ;
	sscanf( time.c_str() , "%lld", &ntime) ;

	struct tm local_tm;
	struct tm *tm = localtime_r( &ntime, &local_tm ) ;

	char buf[128] = {0} ;
	sprintf( buf, "%04d%02d%02d/%02d%02d%02d",
			tm->tm_year + 1900 , tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec) ;
	return buf ;
}

// ת���ϱ���GPS������
bool PConvert::buildintergps( std::vector<std::string> &vec , const char *macid,  CQString &buf , CQString &msg )
{
	//*0431|GPSPosInfo|1.0.4|XQ|4|P|17|mxJ5B4AqAX4|0|0|118.3315|33.7856|99.98|+93.9|308|1340187770|0|-1|nhs#
	if ( vec.empty() ) {
		msg = "�����������󣬲�������Ϊ��" ;
		return false ;
	}
	//CAITS 0_1 E001_15294613494 0 U_REPT {TYPE:0,RET:0,1:72524220,2:18893455,21:0,3:833,4:20120725/093325,5:262,6:7,7:861,8:3,9:502313,20:134217728}
	if ( vec[10].empty() || vec[11].empty() ) {
		msg = "���Ⱥ�γ��Ϊ�գ�������������" ;
		return false ;
	}
	unsigned int lon = my_atof( vec[10].c_str() ) * 600000 ;  // 1
	unsigned int lat = my_atof( vec[11].c_str() ) * 600000 ;  // 2

	unsigned int alam = 0;   // 20
	switch( my_atoi( vec[8].c_str() ) ){
	case 1:
		alam = 1 ;
		break ;
	case 2:
		alam = ( 1 << 1 ) ;
		break ;
	case 3:
		alam = ( 1 << 20 ) ;
		break ;
	case 4:
		alam = ( 1 << 26 ) ;
		break ;
	case 5:
		alam = ( 1 << 4 ) | ( 1 << 5 ) | ( 1 << 6 ) ;
		break ;
	default:
		break ;
	}
	unsigned int speed  = (( my_atof( vec[12].c_str() )  * 10.0 ) / 3.6 ) ;   // 3
	unsigned int height =  0 ;  // 6
	char *ptr = (char *)vec[13].c_str() ;
	if ( ptr != NULL ) {
		while( *ptr != 0  ) {
			if (*ptr > '0' || *ptr < '9') {
				++ ptr ;
				continue ;
			}
			break ;
		}
		if ( *ptr != 0 )  height = atoi(ptr) ;
	}

	unsigned int direction = my_atoi( vec[14].c_str() ) ;  // 5
	string stime = convert2intertime( vec[15] ) ;  // 4

	unsigned int state = 0 ;  // 8
	switch( my_atoi(vec[16].c_str()) ) {
	case 0:
		state = 1 ;
		break ;
	default:
		state = 3 ;
		break ;
	}

	char szbuf[1024] = {0} ;
	sprintf( szbuf, "CAITS 0_0 %s 0 U_REPT {TYPE:0,RET:0,1:%d,2:%d,3:%d,4:%s,5:%d,8:%d,20:%d} \r\n",
			macid, lon, lat, speed, stime.c_str(), direction, state, alam ) ;
	buf.AppendBuffer( szbuf ) ;

	sprintf( szbuf, "����������,����:%d,γ��:%d,�ٶ�:%d,ʱ��:%s,����:%d,״̬:%d,�澯:%d", lon, lat, speed, stime.c_str(), direction, state, alam ) ;
	msg = szbuf ;

	return true ;
}

