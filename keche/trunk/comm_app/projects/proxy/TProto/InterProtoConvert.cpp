/**********************************************
 * InterProtoConvert.cpp
 *
 *  Created on: 2010-7-17
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/

#include "InterProtoConvert.h"
#include "comlog.h"
#include <string.h>
#include "BaseTools.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ProtoParse.h"
#include <fcntl.h>
#include <errno.h>
#include <Base64.h>
#include "ProtoHeader.h"

static bool checkdir( const char *szpath , bool create )
{
	DIR *dir = opendir(szpath);
	if (dir != NULL)
	{
		closedir(dir) ;
		return true ;
	}
	// �Ƿ���Ҫ����
	if ( create )
	{
		return ( mkdir( szpath, 0755 ) == 0 ) ;
	}
	return false ;
}

static bool get_file_path( const char *szroot, string &path, const string &mac_id , char *sztime )
{
	char str_time[32] = {0};

	time_t t = time(0) ;

	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;
	sprintf(str_time, "%04d%02d%02d/", tm->tm_year + 1900,tm->tm_mon + 1, tm->tm_mday);

	path = szroot;
	if ( ! checkdir( path.c_str(), true ) )
		return false ;

	path += str_time;
	if ( ! checkdir( path.c_str(), true ) )
		return false ;

	path += mac_id;

	sprintf(sztime,"%02d%02d%02d",tm->tm_hour,tm->tm_min,tm->tm_sec ) ;

	return checkdir( path.c_str() , true ) ;
}

static bool mv_file( const string &old_file_name, const string &new_file_name)
{
	if(rename(old_file_name.c_str(),new_file_name.c_str()) == 0)
		return true;

	OUT_ERROR(NULL,0,NULL,"rename %s to %s error:%s", old_file_name.c_str(),new_file_name.c_str(),strerror(errno));
	return false ;
}

static bool get_file_name( const char *szroot, string &file_name, const string &mac_id, const unsigned char type , bool bnew )
{
	string path ;

	char sztime[32] = {0} ;
	if ( ! get_file_path( szroot, path, mac_id, sztime ) )
		return false ;

	path += "/";
	path += mac_id ;

	if ( bnew ) {
		path += "_" ;
		path += sztime ;
	}

	switch( type )
	{
	case IMG_JPG:
		path += ".jpg";
		break;
	case IMG_GIF:
		path += ".gif";
		break;
	case IMG_TIFF:
		path += ".tiff";
		break;
	default:
		path += ".jpg";
		break;
	}
	file_name = path;

	return true;
}

InterProtoConvert::InterProtoConvert():_seq_map(true)  // ��Ҫ��ʱ����
{
}

InterProtoConvert::~InterProtoConvert()
{
}

bool InterProtoConvert::build_gps_info( string &dest, GnssData *gps_data )
{
	dest.clear();

	//unsigned short state = ntouv16(gps_data->state);
	unsigned int alarm = ntouv32(gps_data->alarm);

	unsigned int gps_valid = ntouv32(gps_data->state) & 0x02;
	unsigned int acc_state = ntouv32(gps_data->state) & 0x01;

	if (gps_valid)
		dest += "15:1 ";
	else
		dest += "15:0 ";

	if (acc_state)
		dest += "26:4 ";
	else
		dest += "26:5 ";

	dest += "1:";
	dest += uitodecstr(ntouv32(gps_data->lon) * 6 / 10);
	dest += " ";

	dest += "2:";
	dest += uitodecstr(ntouv32(gps_data->lat) * 6 / 10);
	dest += " ";

	dest += "3:";
	dest += uitodecstr(ntouv16(gps_data->vec1));
	dest += " ";

	unsigned int iyear = ((unsigned char) (gps_data->date[2])) * 256 + (unsigned char) (gps_data->date[3]);
	string year   = uitodecstr(iyear);
	string month  = charto2decstr(gps_data->date[1]);
	string day    = charto2decstr(gps_data->date[0]);

	string hour   = charto2decstr(gps_data->time[0]);
	string minute = charto2decstr(gps_data->time[1]);
	string second = charto2decstr(gps_data->time[2]);

	dest += "4:" + year + month + day + "/" + hour + minute + second + " ";

	dest += "5:"  + ustodecstr(ntouv16(gps_data->direction)) + " ";
	dest += "21:" + ustodecstr(ntouv32(gps_data->vec3)) + " ";

	if (alarm & 0x01)//��������
	{
		dest += "20:0 ";
	}
	if (alarm & 0x02)//���ٱ���
	{
		dest += "20:41 ";
	}
	if (alarm & 0x04)//ƣ�ͼ�ʻ
	{
		dest += "20:10 ";
	}

	return true ;
}

// ��GPS����ת��GNSS
bool InterProtoConvert::convert_gps_info( const string &dest, GnssData &gps )
{
	if ( dest.empty() )
		return false ;

	vector<string> vec;
	if ( ! splitvector( dest, vec, " " , 0 ) ) {
		return false ;
	}

	gps.state = 0x00 ;
	gps.alarm = 0x00 ;

	size_t pos ;
	string stemp , svalue ;
	for ( size_t i = 0; i < vec.size(); ++ i ) {

		stemp = vec[i] ;
		pos   = stemp.find(":") ;

		if ( pos == string::npos ) {
			continue ;
		}
		svalue = stemp.substr( pos + 1 ) ;

		int ntype = atoi( stemp.c_str() ) ;
		switch( ntype ) {
			case 1:
				{
					gps.lon = ntouv32( atoi(svalue.c_str()) * 10 / 6 ) ;
				}
				break;
			case 2:
				{
					gps.lat = ntouv32( atoi(svalue.c_str()) *10 / 6 ) ;
				}
				break;
			case 3:
				{
					gps.vec1 = ntouv16( atoi(svalue.c_str()) ) ;
				}
				break ;
			case 4: // "4:" + year + month + day + "/" + hour + minute + second + " ";
				{
					int nyear = 0 , nmonth = 0 , nday = 0 , nhour = 0 ,nmin = 0 , nsec = 0 ;

					sscanf( svalue.c_str(), "%04d%02d%02d/%02d%02d%02d", &nyear, &nmonth, &nday, &nhour, &nmin, &nsec ) ;

					gps.date[3]   = nyear  % 256 ;
					gps.date[2]   = nyear / 256 ;
					gps.date[1]   = nmonth ;
					gps.date[0]   = nday ;

					gps.time[0]   = nhour ;
					gps.time[1]   = nmin ;
					gps.time[2]   = nsec ;
				}
				break ;
			case 5:
				{
					gps.direction = ntouv16( atoi(svalue.c_str()) ) ;
				}
				break ;
			case 15:
				{
					if ( atoi(svalue.c_str()) ) {
						gps.state = gps.state | 0x02 ;
					}
				}
				break ;
			case 20:
				{
					int nn = atoi( svalue.c_str() ) ;
					if ( nn == 0 ) {  		 //��������
						gps.alarm |= 0x01 ;
					} else if ( nn == 41 ) { //���ٱ���
						gps.alarm |= 0x02 ;
					} else if ( nn == 10 ) { //ƣ�ͼ�ʻ
						gps.alarm |= 0x04 ;
					}
				}
				break ;
			case 21:
				{
					gps.vec3  = ntouv32( atoi(svalue.c_str()) ) ;
				}
				break ;
			case 26:
				{
					if ( atoi(svalue.c_str()) ) {
						gps.state = gps.state | 0x01 ;
					}
				}
				break ;
		}
	}
	gps.state = ntouv32( gps.state ) ;
	gps.alarm = ntouv32( gps.alarm ) ;

	return true ;
}


bool InterProtoConvert::build_yutoo_gps(string &dest, const string &mac_id, GnssData* gps_data)
{
	char buf[16] = {0};
	dest.clear();

	//unsigned short state = ntouv16(gps_data->state);
	//unsigned short alarm = ntouv16(gps_data->alarm);

	unsigned short gps_valid = ntouv32(gps_data->state) & 0x02;
	//unsigned short acc_state = ntouv16(gps_data->state) & 0x01;

	unsigned int iyear = ((unsigned char)(gps_data->date[2]))*256 + (unsigned char )(gps_data->date[3]);
	string year  = uitodecstr(iyear);
	string month = charto2decstr(gps_data->date[1]);
	string day   = charto2decstr(gps_data->date[0]);

	string hour   = charto2decstr(gps_data->time[0]);
	string minute = charto2decstr(gps_data->time[1]);
	string second = charto2decstr(gps_data->time[2]);

	dest += year + month + day + "," + hour + minute + second + ",";
	dest += "H," ;//��˾����
	dest += mac_id + ","; //������ʶ��ʾ���������ֻ�������ʶ�ģ������ó�����ɫ�ͳ��ƺ�

	unsigned int longitude = ntouv32(gps_data->lon);
	unsigned int latitude  = ntouv32(gps_data->lat);

	double f_longitude = longitude/1000000.0000000;
	double f_latitude  = latitude/1000000.0000000;

//	char buf[16] = {0};
	sprintf(buf,"%f",f_longitude);
	dest += buf;
	dest += ",";


	memset(buf,0,16);
	sprintf(buf,"%f",f_latitude);
	dest += buf;
	dest += ",";

	dest += uitodecstr(ntouv16(gps_data->vec1)) + ",";
	dest += ustodecstr(ntouv16(gps_data->direction)/23) + ",";
	dest += "-1,";//���س�״̬
	if(gps_valid)
		dest += "1;";
	else
		dest += "0;";
	return true ;
}

void InterProtoConvert::append_gps( string &dest, const string &data )
{
	dest += data ;
	dest += " " ;
}

bool InterProtoConvert::build_inter_header(string &destheader, const string &header, const string &seq, const string &mac_id, const string &command, const string &com_access_code )
{
	destheader.clear() ;
	append_gps( destheader, header ) ;
	append_gps( destheader, seq ) ;
	append_gps( destheader, mac_id ) ;
	append_gps( destheader, command ) ;
	append_gps( destheader, com_access_code ) ;
	return true ;
}

bool InterProtoConvert::build_inter_proto( string &dest, const string &header, const string &seq, const string &mac_id, const string &command, const string &com_access_code, const string &command_value )
{
	dest.clear();

	append_gps( dest, header ) ;
	append_gps( dest, seq ) ;
	append_gps( dest, mac_id ) ;
	append_gps( dest, command ) ;
	append_gps( dest, com_access_code ) ;
	append_gps( dest, command_value ) ;

	dest += "\r\n";

	//  TRSF 000000_0000000000_0 1_��A10104 D_REPT 701116 MSG_CODE:000 15:0 26:5 1:69782082 2:23947540 3:5 4:20110516/153637 5:6 21:0
	return true ;
}

const string InterProtoConvert::build_platform_out_seq( const string &mac_id , unsigned short data_type )
{
	string seq  = mac_id ;  // MACֵ
	seq += "_" ;
	// ������Ӧ��������������,0x8000
	if ( ! ( data_type & 0x8000 ) ) {
		data_type |= 0x8000 ;
	}
	seq += get_type( data_type ) ;//���ϲ�������0x920A
	return seq;
}

// ����ͨ������
bool InterProtoConvert::build_common_dctlm_data( string &dest, const string &header, string &in_seq, const string &mac_id, const unsigned int msg_type,
		const string &access_code, const string &command, const string &data )
{
	string out_seq = build_platform_out_seq( mac_id, msg_type ) ;
	if( ! _seq_map.FindReqMap( out_seq, in_seq , true ) )
	{
//		ERRLOG(NULL,0,"Can't get this packet's inter seq, dropped!");
		OUT_ERROR(NULL,0,NULL,"out_seq=%s,msg type= %d, can't find the corresponding in_seq,dropped it!", out_seq.c_str(), msg_type );
		return false ;
	}
	return build_inter_proto( dest, header , in_seq, mac_id, command , access_code, data ) ;
}

// �·���������Ӧ����Ϣ
bool InterProtoConvert::build_ctrl_msg_text_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const int msgid, const unsigned char result )
{
	// "Ӧ��: ��ϢID|Ӧ������0���ɹ���1��ʧ�ܣ�"
	char buf[512] = {0} ;
	sprintf( buf, "TEXT:%d|%d", msgid, result ) ;

	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_CTLM",  buf ) ;
}

// �����������
bool InterProtoConvert::build_ctrl_msg_monitor_vehicle_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const unsigned char result )
{
	char buf[128] = {0} ;
	sprintf( buf, "MONITOR:%d", result ) ;

	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_CTLM",  buf ) ;
}

// ��������Ӧ����Ϣ
bool InterProtoConvert::build_ctrl_msg_take_photo_ack( string &dest, string &in_seq, const  unsigned int data_type, const string &mac_id,
			const string &access_code, const unsigned char photo_rsp_flag, const char * gnss_data, const unsigned char lens_id,
			const unsigned int photo_len, const unsigned char size_type, const unsigned char type, const char * photo , const int data_len, const char *szpath )
{
	//Ӧ��Ӧ���ʶ��0����֧�����գ�1��������գ�2��������գ���Ƭ�����Ժ��ͣ�3��δ���գ������ߣ���4��δ���գ��޷�ʹ��ָ����ͷ����5��δ���գ�����ԭ�򣩣�9�����ƺ������
	// |����λ�õص�|��ͷID|ͼƬ����|��Ƭ��С��1:320x240��2:640x480��
	//3:800x600��4:1024x768��5:176x144[Qcif]��
	//6:352*288[Cif]��7:704*288[HALF D1]��
	//8:704*576[D1]��|ͼ���ʽ��1��jpg��2��gif��3��tiff��4��png��|ͼƬurl"
	string filepath  ;
	// ���PAS�洢�����������ı�־
	if ( data_len > 4 && photo_len == (unsigned int) data_len && strncmp(photo, "PAS:", 4) == 0 ){
		// ���ͼƬ����С��256��Ӧ��ͼƬ�ļ�·��
		string new_file_name( photo+4, photo_len-4 ) ;
		filepath = new_file_name ;
	} else { // �������ͼƬ�ļ�
		string file_name ;
		if ( ! get_file_name( szpath, file_name, mac_id, type , false ) )
			return false ;

		FILE *fp = fopen( file_name.c_str(), "a+" ) ;
		if(fp == NULL){
			OUT_ERROR( NULL, 0, NULL, "open file %s failed", file_name.c_str() ) ;
			return false ;
		}

		fwrite( photo, sizeof(char), data_len , fp ) ;
		fseek(fp,0,SEEK_END);

		if(ftell(fp) < photo_len ) { //˵�����һ�����Ѿ�д���ļ�������
			fclose( fp ) ;
			OUT_ERROR(NULL,0,NULL," mac id %s , access code %s, photo length %d more than data length", mac_id.c_str(), access_code.c_str() , photo_len );
			return false ;
		}
		//������Ƭû���ϴ���ȫ������false,����������ͻ��ˡ�
		fclose(fp);

		// ���ļ�����
		string new_file_name;
		if( ! get_file_name( szpath, new_file_name, mac_id, type , true ) ){
			OUT_ERROR( NULL, 0, NULL, "get new file name %s failed", new_file_name.c_str() ) ;
			return false ;
		}
		if( ! mv_file( file_name, new_file_name ) ){
			OUT_ERROR( NULL, 0, NULL, "mv file %s to new file %s failed", file_name.c_str() , new_file_name.c_str() ) ;
			return false ;
		}
		filepath = new_file_name ;
	}

	char buf[2048] = {0} ;
	sprintf( buf, "PHOTO:%d|%s|%d|%d|%d|%d|%s", photo_rsp_flag, gnss_data, lens_id, photo_len, size_type, type, filepath.c_str() ) ;

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_CTLM",  buf ) ;
}

// �ϱ�������ʻ��¼Ӧ����Ϣ
bool InterProtoConvert::build_ctrl_msg_take_travel_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code,unsigned char command_type, const char *data )
{
	char buf[256]={0};
	sprintf(buf,"TRAFFICBLACKBOX:%d|",(int)command_type);
	
//	string val = "TRAFFICBLACKBOX:" ;
	string val=string(buf)+data;
//	val += data ;
	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_CTLM",  val ) ;
}

// ����Ӧ��������ƽ̨Ӧ����Ϣ
bool InterProtoConvert::build_ctrl_msg_emergency_monitoring_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id ,
		const string &access_code, const unsigned char result )
{
	char buf[256] = {0} ;
	sprintf( buf, "EMERGENCY:%d" , result ) ;

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_CTLM",  buf ) ;
}

// ��������Ӧ����Ϣ
bool InterProtoConvert::build_warn_msg_urge_todo_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code , const unsigned char result )
{
	string val = "WARNTODO:" + chartodecstr(result);
	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_WARN",  val ) ;
}

// �ϱ�������Ϣ��Ϣ
bool InterProtoConvert::build_warn_msg_adpt_info( string &dest, const string &seq, const string &mac_id,
		const string &access_code, const unsigned char warn_src, const unsigned short warn_type , const unsigned long long warn_time , const char *data )
{
	//"����
	//������Ϣ��Դ��1�������նˣ�2����ҵ���ƽ̨��3���������ƽ̨��9��������|��������(���5.3���������ͱ����)|����ʱ��(UTCʱ���ʽ)|��������"
	char buf[1024] = {0} ;
#ifdef _WIN32
	sprintf( buf, "WARNTIPS:%d|%d|%I64d|" , warn_src, warn_type, warn_time ) ;
#else
	sprintf( buf, "WARNTIPS:%d|%d|%lld|", warn_src, warn_type, warn_time ) ;
#endif
	string val = buf ;
	val += data ;

	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_WARN", access_code, val ) ;
}

// �����ϱ�������������Ϣ��Ϣ
bool InterProtoConvert::build_warn_msg_adpt_todo_info( string &dest, const string &seq, const string &mac_id,
		const string &access_code, const unsigned int info_id, const unsigned char result)
{
	char buf[200];
	sprintf(buf,"UPWARN:%u|%d",info_id,(int)result);
	string val = string(buf);

	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_WARN", access_code, val ) ;
}


// ����������̬��ϢӦ����Ϣ UP_BASE_MSG_VEHICLE_ADDED_ACK
bool InterProtoConvert::build_base_msg_vehicle_added_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id ,
			const string &access_code , const char *data )
{
	//"Ӧ��
	//VIN:=��A25307;VEHICLE_COLOR:=1;VEHICE_TYPE:=40;TRANS_TYPE:=030;
	//VEHICE_NATIONALIT:=330108;OWERS_ID:=382738;OWERS_NAME:=���ݻ��˴���˾;OWERS_ORIG_ID:=1000;OWERS_TEL:=13516814499��"

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_BASE",  data ) ;
}

// ����ƽ̨���Ӧ��
bool InterProtoConvert::build_platform_msg_post_query_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
		const string &com_access_code, UpPlatformMsgpostqueryData * pMsg, const char *data , const int len )
{
	char buf[200];
	sprintf(buf,"PLATQUERY:%d|",(int)pMsg->object_type);
	
	if ( data == NULL ) {
		return false ;
	}

	string val = string(buf) + string(pMsg->object_id,sizeof(pMsg->object_id)).c_str();
	
	val += "|" ;
	val += uitodecstr(ntouv32(pMsg->info_id)) ;
	val += "|" ;

	CBase64 base64;
	base64.Encode( data, len ) ;

	val += base64.GetBuffer() ;

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, com_access_code, "D_PLAT",  val ) ;
}

// �·�ƽ̨�䱨��Ӧ����ϢPLATFORMMSG
bool InterProtoConvert::build_platform_msg_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const int msgid )
{
	string val = "PLATMSG:" + uitodecstr(msgid) ;

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_PLAT",  val ) ;
}

// ��������λ��Ϣ����Ӧ����Ϣ
bool InterProtoConvert::build_exg_msg_arcossarea_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id,
		const string &access_code, const string &device_id , const int ack_type )
{
	//  ISMOVE:=1|0;ACCESS_CODE:=xxxxxxxx;MSG_CODE:=030;VIN:=��A25307
	char buf[512] = {0} ;
	sprintf( buf, "ISMOVE:=%d;ACCESS_CODE:=%s;VIN:=%s", ack_type , access_code.c_str() , device_id.c_str() ) ;
	// string &destheader, const string &header, const string &seq, const string &mac_id, const string &command, const string &com_access_code

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", seq, mac_id, data_type, access_code, "D_GECH",  buf ) ;
}


// ������������λ��Ϣ����Ӧ����Ϣ
bool InterProtoConvert::build_exg_msg_arcossarea_startup_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
		const string &access_code, const string &device_id )
{
	return build_exg_msg_arcossarea_ack( dest, in_seq, data_type, mac_id, access_code, device_id, 1 ) ;
}

// ������������λ��Ϣ����Ӧ����Ϣ
bool InterProtoConvert::build_exg_msg_arcossarea_end_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
		const string &access_code, const string &device_id )
{
	return build_exg_msg_arcossarea_ack( dest, in_seq, data_type, mac_id, access_code, device_id, 0 ) ;
}

// ����������λ��Ϣ����Ӧ����Ϣ
bool InterProtoConvert::build_exg_msg_return_startup_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id ,
		const string &access_code )
{
	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", seq, mac_id, data_type, access_code, "D_MESG",  "RETURNSTARTUP:" ) ;
}
// ����������λ��ϢЧ��Ӧ����Ϣ
bool InterProtoConvert::build_exg_msg_return_end_ack( string &dest, string &seq, const unsigned int data_type, const string &mac_id ,
			const string &access_code )
{
	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", seq, mac_id, data_type, access_code, "D_MESG",  "RETURNEND:" ) ;
}

// ע�ᳵ����Ϣ
bool InterProtoConvert::build_exg_msg_register( string &dest, const string &seq, const string &mac_id , const string &access_code,
		const string &platform_id, const string &producer_id, const string &terminal_model_type, const string &terminal_id, const string &terminal_simcode )
{
	// ƽ̨Ψһ����|�����ն˳���Ψһ����|�����ն��ͺ�|�����ն˱��|�����ն�SIM���绰����"
	char buf[1024] = {0} ;
	sprintf( buf, "REGISTER:%s|%s|%s|%s|%s", platform_id.c_str() , producer_id.c_str(), terminal_model_type.c_str() ,
			terminal_id.c_str(), terminal_simcode.c_str() ) ;

	return build_inter_proto( dest, "TRSF" , seq, mac_id, "D_MESG" , access_code, buf ) ;
}

// ���뽻��ָ��������λ��Ϣ������Ϣ
bool InterProtoConvert::build_exg_msg_apply_for_monitor_startup( string &dest, const string &seq, const string &mac_id,
		const string &access_code, const unsigned long long start, const unsigned long long end )
{
	// D_MESG MONITORSTARTUP��start| end
	char buf[256] = {0} ; //%lld
#ifdef _WIN32
	sprintf( buf, "MONITORSTARTUP:%I64d|%I64d", start, end ) ;
#else
	sprintf( buf, "MONITORSTARTUP:%lld|%lld", start, end ) ;
#endif
	// ������Ӧ����
	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_MESG", access_code, buf ) ;
}

// ȡ������ָ��������λ��Ϣ������Ϣ
bool InterProtoConvert::build_exg_msg_apply_for_monitor_end( string &dest, const string &seq, const string &mac_id, const string &access_code )
{
	char buf[256] = {0} ;
	sprintf( buf, "MONITOREND:") ;

	// ������Ӧ����
	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_MESG", access_code, buf ) ;
}

// ����������λ��Ϣ������Ϣ
bool InterProtoConvert::build_exg_msg_apply_hisgnssdata_req( string &dest, const string &seq, const string &mac_id, const string &access_code,
		const unsigned long long start, const unsigned long long end )
{
	char buf[256] = {0} ; //%lld
#ifdef _WIN32
	sprintf( buf, "HISGNSSDATA:%I64d|%I64d", start, end ) ;
#else
	sprintf( buf, "HISGNSSDATA:%lld|%lld", start, end ) ;
#endif
	// ������Ӧ����
	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_MESG", access_code, buf ) ;
}

//	�����ϱ���ʻԱ�����Ϣ��Ϣ
bool InterProtoConvert::build_exg_msg_report_driver_info(string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &driver_name ,  const string &driver_id , const string &licence , const string &org_name )
{
	char buf[1024] = {0} ;  // "���󣺼�ʻԱ����|���֤���|��ҵ�ʸ�֤��|��֤��������"

	sprintf( buf, "UPDRIVERINFO:%s|%s|%s|%s" , driver_name.c_str(), driver_id.c_str(), licence.c_str(), org_name.c_str() ) ;
	// ������Ӧ����
	return build_inter_proto( dest, "TRSF" , in_seq, mac_id, "D_MESG" , access_code, buf ) ;
	//return build_common_dctlm_data( dest, "TRSF", in_seq, mac_id, data_type, access_code, "D_MESG",  buf ) ;
}

// �ϱ�˾�����ʶ����ϢӦ����Ϣ
bool InterProtoConvert::build_exg_msg_report_driver_info_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code, const string &driver_name ,  const string &driver_id , const string &licence , const string &org_name )
{
	char buf[1024] = {0} ;  // "Ӧ�𣺼�ʻԱ����|���֤���|��ҵ�ʸ�֤��|��֤��������"

	sprintf( buf, "DRIVERINFO:%s|%s|%s|%s" , driver_name.c_str(), driver_id.c_str(), licence.c_str(), org_name.c_str() ) ;
	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_MESG",  buf ) ;
}

//	�����ϱ����������˵���Ϣ
bool InterProtoConvert::build_exg_msg_report_waybill_info( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code , const char *data, const int len )
{
	string value( data , len ) ;

	value = "UPEWAYBILL:" + value ;

	// ������������
	return build_inter_proto( dest, "TRSF", in_seq, mac_id,  "D_MESG", access_code,  value.c_str() ) ;
}

// �ϱ������˵�Ӧ��
bool InterProtoConvert::build_exg_msg_take_waybill_ack( string &dest, string &in_seq, const unsigned int data_type, const string &mac_id,
			const string &access_code , const char *data, const int len )
{
	string value( data , len ) ;

	value = "EWAYBILL:" + value ;

	// ������Ӧ����
	return build_common_dctlm_data( dest, "TRSR", in_seq, mac_id, data_type, access_code, "D_MESG",  value.c_str() ) ;
}

// ����MAS�����ƽ̨���
bool InterProtoConvert::build_mas_platform_msg_post_query_req( string &dest, const string &seq, const string &mac_id,
		const string &access_code, DownPlatformMsgPostQueryBody * pBody, const char *data , const int len )
{
	char buf[200];
	sprintf(buf,"PLATQUERY:%d|",(int)pBody->object_type);
	string val = string(buf)+string(pBody->object_id,sizeof(pBody->object_id)).c_str();
		
//	string val = "PLATQUERY:" ;
	if ( data == NULL ) {
		return false ;
	}
	val += "|" ;
	val += uitodecstr(ntouv32( pBody->info_id)) ;
	val += "|" ;

	CBase64 base64;
	base64.Encode( data, len ) ;

	val += base64.GetBuffer() ;

	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_PLAT", access_code, val ) ;
}

// ���Ͳ�����̬���ݴ���
bool InterProtoConvert::build_mas_down_base_msg_vehicle_added( string &dest, string &seq, const string &mac_id, const string &access_code )
{
	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_BASE", access_code, "" ) ;
}

// �����·�ƽ̨�䱨��
bool InterProtoConvert::build_mas_platform_msg_info_req( string &dest, const string &seq, const string &mac_id,
				const string &access_code, DownPlatformMsgInfoReq * pReq, const char *data , const int len )
{
	char buf[200];
	sprintf(buf,"PLATMSG:%d|",(int)pReq->object_type);
	string val = string(buf) + string(pReq->object_id,sizeof(pReq->object_id)).c_str();
//	string val = "PLATMSG:" ;
	if ( data == NULL ) {
		return false ;
	}
	val += "|" ;
	val += uitodecstr(ntouv32(pReq->info_id)) ;
	val += "|" ;

	CBase64 base64;
	base64.Encode( data, len ) ;

	val += base64.GetBuffer() ;

	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_PLAT", access_code, val ) ;
}

// ����MAS����������̬��Ϣ
bool InterProtoConvert::build_mas_exg_msg_car_info( string &dest, const string &seq, const string &mac_id, const string &access_code, const char *car_info )
{
	return build_inter_proto( dest, "TRSF" , seq, mac_id, "U_BASE" , access_code, car_info ) ;
}

// ����MAS����������λ��Ϣ����
bool InterProtoConvert::build_mas_exg_msg_return_startup( string &dest, const string &seq, const string &mac_id,
		const string &access_code, const unsigned char result )
{
	char buf[128] = {0} ;
	sprintf( buf, "RETURNSTARTUP:%d", result ) ;

	return build_inter_proto( dest, "TRSF", seq, mac_id, "D_MESG" , access_code, buf ) ;
}

// ����MAS����������λ��Ϣ����
bool InterProtoConvert::build_mas_exg_msg_return_end( string &dest, const string &seq, const string &mac_id,
		const string &access_code, const unsigned char result )
{
	char buf[128] = {0} ;
	sprintf( buf, "RETURNEND:%d", result ) ;

	return build_inter_proto( dest, "TRSF" , seq, mac_id, "D_MESG" , access_code, buf ) ;
}

// ����MAS���뽻��ָ��������λ��Ϣ
bool InterProtoConvert::build_mas_exg_msg_apply_for_monitor_startup_ack( string &dest, string &seq, const uint16 data_type, const string &mac_id,
		const string &access_code, const uint8 result )
{
	char buf[512] = {0} ;
	sprintf( buf, "MONITORSTARTUP:%d" , result ) ;

	return build_common_dctlm_data( dest, "TRSR", seq, mac_id, data_type, access_code, "D_MESG",  buf ) ;
}

// ����MASȡ������ָ��������λ��
bool InterProtoConvert::build_mas_exg_msg_apply_for_monitor_end_ack( string &dest, string &seq, const uint16 data_type, const string &mac_id,
		const string &access_code , const uint8 result )
{
	char buf[512] = {0} ;
	sprintf( buf, "MONITOREND:%d", result ) ;

	return build_common_dctlm_data( dest, "TRSR", seq, mac_id, data_type, access_code, "D_MESG",  buf ) ;
}


// ���ĳ��ź���ɫ��Ϣȡ��MACΨһ��ʶ
string InterProtoConvert::get_mac_id( const char *device_id, unsigned char device_color )
{
	string dest;
	char number[32] = {0};
	memcpy(number,device_id,21);
	dest += chartodecstr(device_color);
	dest += "_";
	dest += number;
	return dest;
}

// ����MAC�е���Ϣ
static bool parse_mac_id( const string &mac_id, string &device_id, unsigned char &color ){
	size_t pos = mac_id.find( '_' , 0 ) ;
	if ( pos == string::npos ){
		return false ;
	}
	color     = atoi(mac_id.substr(0,pos).c_str()) ;
	device_id = mac_id.substr( pos + 1 ) ;
	return true ;
}

char *InterProtoConvert::convert_mas_dctlm(const string &seq, const string &mac_id, const string &company_id,
		const string &operator_key, const string &operate_value, int &len )
{
	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if (operator_key == "TRAFFICBLACKBOX") {
		vector<string> vec = Split( operate_value , "|" ) ;
		if (vec.size()<2){
			OUT_ERROR( NULL, 0, NULL, "UP_CTRL_MSG TRAFFICBLACKBOX arg less than 2, value %s", operate_value.c_str() ) ;
			return NULL ;	
		}
		
		len = sizeof(UpCtrlMsgTaketravel)  + vec[1].length() + sizeof(Footer) ;

		UpCtrlMsgTaketravel resp ;
		ProtoParse::BuildHeader( resp.header, len , msg_seq, UP_CTRL_MSG  , id ) ;

		memcpy( resp.ctrl_msg_header.vehicle_no, device_id.c_str(), device_id.length() ) ;
		resp.ctrl_msg_header.vehicle_color = device_color ;
		resp.ctrl_msg_header.data_type     = ntouv16(UP_CTRL_MSG_TAKE_TRAVEL_ACK) ;
		resp.ctrl_msg_header.data_length   = ntouv32( sizeof(unsigned char) + sizeof(unsigned int) + vec[1].length() ) ;
		resp.command_type = (unsigned char)atoi(vec[0].c_str());
		resp.travel_length = ntouv32(vec[1].length());

		buf = new char[ len + 1 ] ;
		memset( buf, 0 , len+1) ;
		memcpy( buf, &resp, sizeof(UpCtrlMsgTaketravel));

		int offset = sizeof(UpCtrlMsgTaketravel);
		memcpy( buf+offset, vec[1].c_str(), vec[1].length()) ;
		offset += vec[1].length() ;

		Footer  footer ;
		memcpy( buf +offset, &footer, sizeof(Footer) ) ;

		OUT_SEND( NULL, 0,  "mas" , "UP_CTRL_MSG %s" , get_type(UP_CTRL_MSG_TAKE_TRAVEL_ACK) ) ;		
		
	}
	return buf;
}

// ת�����ݶ�ӦЭ������
char *InterProtoConvert::convert_dctlm(const string &seq, const string &mac_id, const string &company_id,
		const string &operator_key, const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_CTLM operate_value is too long!!!");
		return NULL ;
	}

	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if (operator_key == "MONITOR") {  // �������

		len = sizeof(DownCtrlMsgMonitorVehicleReq) ;

		DownCtrlMsgMonitorVehicleReq req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_CTRL_MSG  , id ) ;

		memcpy( req.ctrl_msg_header.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		req.ctrl_msg_header.vehicle_color = device_color ;
		req.ctrl_msg_header.data_type     = ntouv16(DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ) ;
		req.ctrl_msg_header.data_length   = ntouv32( sizeof(req.monitor_tel) ) ;

		memcpy(req.monitor_tel, operate_value.c_str(), operate_value.length() );

		buf = new char[len+1] ;
		memset( buf, 0, len ) ;

		memcpy(buf, &req, sizeof(DownCtrlMsgMonitorVehicleReq));

		OUT_SEND( NULL, 0, "cas", "DOWN_CTRL_MSG DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ" );

		// �����ⲿ���д���
		out_seq = build_platform_out_seq( mac_id, DOWN_CTRL_MSG_MONITOR_VEHICLE_REQ ) ;

	} else if (operator_key == "PHOTO") {  // ����
		//���շ��ؿ��ݲ�֧��
		len = sizeof(DownCtrlMsgTakePhotoReq) ;

		DownCtrlMsgTakePhotoReq req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq , DOWN_CTRL_MSG  , id ) ;

		memcpy( req.ctrl_msg_header.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		req.ctrl_msg_header.vehicle_color = device_color ;
		req.ctrl_msg_header.data_type     = ntouv16(DOWN_CTRL_MSG_TAKE_PHOTO_REQ) ;
		req.ctrl_msg_header.data_length   = ntouv32( sizeof(char)*2 ) ;

		// "���󣺾�ͷID|��Ƭ��С��1:320x240��2:640x480��
		// 3:800x600��4:1024x768��5:176x144[Qcif]��
		// 6:352*288[Cif]��7:704*288[HALF D1]��
		// 8:704*576[D1]��"

		size_t pos = operate_value.find( '|' , 0 ) ;
		if ( pos != string::npos ) {
			req.lens_id = (char) atoi( operate_value.substr( 0, pos ).c_str() ) ;
			req.size    = (char) atoi( operate_value.substr( pos + 1 ).c_str() ) ;
		}

		buf = new char[len+1] ;
		memset( buf, 0, len ) ;
		memcpy(buf, &req, sizeof(DownCtrlMsgTakePhotoReq));

		OUT_SEND( NULL, 0, "cas", "DOWN_CTRL_MSG DOWN_CTRL_MSG_TAKE_PHOTO_REQ" ) ;

		// �����ⲿ���д���
		out_seq = build_platform_out_seq( mac_id, DOWN_CTRL_MSG_TAKE_PHOTO_REQ ) ;

	} else if (operator_key == "TEXT") {  // ���������·�
		// ����Ϊ      ID|BASE64(Content)
		size_t pos = operate_value.find( '|', 0 ) ;
		if (  pos == string::npos ){
			OUT_ERROR( NULL, 0, NULL, "DOWN_CTRL_MSG TEXT data error, value %s, %s:%d", operate_value.c_str() , __FILE__, __LINE__ ) ;
			return NULL ;
		}
		// "����:��ϢID|���ȼ���0��������1��һ�㣩|Base64������Ϣ"

		// ȡ����Ϣ�����ID
		string infoid  = operate_value.substr( 0, pos ) ;

		size_t end = operate_value.find( '|', pos + 1 ) ;
		if ( end == string::npos ) {
			OUT_ERROR( NULL, 0, NULL, "DOWN_CTRL_MSG TEXT data error, value %s, %s:%d", operate_value.c_str() , __FILE__, __LINE__ ) ;
			return NULL ;
		}

		// ȡ�����ȼ�
		string prid    = operate_value.substr( pos + 1 , end - pos - 1  ) ;
		// ȡ������
		string content = operate_value.substr( end + 1  ) ;

		CBase64 base64;
		if ( ! base64.Decode( content.c_str(), content.length()) ) {
			OUT_ERROR(NULL, 0, NULL, "DOWN_CTRL_MSG TEXT base64.Base64Decode failed! before de:%s", operate_value.c_str());
			return NULL ;
		}

		char* de_sms_ptr = base64.GetBuffer() ;
		OUT_INFO(NULL, 0, NULL, "the decode short msg is :%s", de_sms_ptr);

		int data_len = strlen(de_sms_ptr) ;

		len = sizeof(DownCtrlMsgTextInfoHeader) + data_len + sizeof(Footer) ;

		DownCtrlMsgTextInfoHeader req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_CTRL_MSG  , id ) ;

		memcpy( req.ctrl_msg_header.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		req.ctrl_msg_header.vehicle_color = device_color ;
		req.ctrl_msg_header.data_type     = ntouv16(DOWN_CTRL_MSG_TEXT_INFO) ;
		req.ctrl_msg_header.data_length   = ntouv32( sizeof(int)*2 + sizeof(char) + data_len ) ;

		req.msg_priority = atoi( prid.c_str() ) ;
		req.msg_len  	 = ntouv32( data_len ) ;
		req.msg_sequence = ntouv32( atoi( infoid.c_str() ) ) ;

		int offset = 0 ;

		buf = new char[len+1] ;
		memset( buf, 0, len ) ;
		memcpy( buf, &req, sizeof(DownCtrlMsgTextInfoHeader) ) ;
		offset += sizeof(DownCtrlMsgTextInfoHeader) ;

		memcpy( buf+offset, de_sms_ptr, data_len ) ;
		offset += data_len ;

		Footer  footer ;
		memcpy( buf +offset, &footer, sizeof(Footer) ) ;

		OUT_SEND( NULL, 0, "cas", "DOWN_CTRL_MSG DOWN_TEXT_INFO" );
		// �����ⲿ���д���
		out_seq = build_platform_out_seq( mac_id, DOWN_CTRL_MSG_TEXT_INFO ) ;

	} else if ( operator_key == "TRAFFICBLACKBOX" ) { // �ϱ�������ʻ��¼
	
		len = sizeof(DownCtrlMsgTaketravelReq) ;

		DownCtrlMsgTaketravelReq req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_CTRL_MSG  , id ) ;

		memcpy( req.ctrl_msg_header.vehicle_no, device_id.c_str(), device_id.length() ) ;
		req.ctrl_msg_header.vehicle_color = device_color ;
		req.ctrl_msg_header.data_type     = ntouv16(DOWN_CTRL_MSG_TAKE_TRAVEL_REQ) ;
		req.ctrl_msg_header.data_length   = ntouv32( sizeof(unsigned char)) ;
		req.command_type = (unsigned char)atoi(operate_value.c_str());

		buf = new char[ len + 1 ] ;
		memset( buf, 0 , len ) ;
		memcpy( buf, &req, len ) ;  //DOWN_CTRL_MSG_TAKE_TRAVEL_REQ

		OUT_SEND( NULL, 0,  "cas" , "DOWN_CTRL_MSG %s" , get_type(DOWN_CTRL_MSG_TAKE_TRAVEL_REQ) ) ;

		out_seq = build_platform_out_seq( mac_id, DOWN_CTRL_MSG_TAKE_TRAVEL_REQ ) ;
			

	} else if ( operator_key == "EMERGENCY" ) {
		// ���ƽ̨�·��ļ�Ȩ��|���ŵ����ƣ�һ��Ϊ��������APN|�����û���|��������|��ַ��������IP��ַ������|������TCP�˿�|������UDP�˿�|����ʱ��(UTCʱ���ʽ)"
		vector<string> vec = Split( operate_value , "|" ) ;
		if ( vec.size() < 8 ) {
			OUT_ERROR( NULL, 0, NULL, "DOWN_CTRL_MSG EMERGENCY arg less than 8, value %s", operate_value.c_str() ) ;
			return NULL ;
		}

		len = sizeof( DownCtrlMsgEmergencyMonitoringReq ) ;

		DownCtrlMsgEmergencyMonitoringReq  req ;
		ProtoParse::BuildHeader( req.header , len , msg_seq , DOWN_CTRL_MSG , id ) ;

		memcpy( req.ctrl_msg_header.vehicle_no, device_id.c_str() , device_id.length() ) ;
		req.ctrl_msg_header.vehicle_color = device_color ;
		req.ctrl_msg_header.data_type     = ntouv16( DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ ) ;
		req.ctrl_msg_header.data_length   = ntouv32( sizeof(req.authentication_code) +
												sizeof(req.access_point_name) + sizeof(req.username) +
												sizeof(req.password) + sizeof(req.server_ip) +
												sizeof(req.tcp_port) + sizeof(req.udp_port) + sizeof(req.end_time) ) ;

		memcpy( req.authentication_code, vec[0].c_str(), vec[0].length() ) ;
		memcpy( req.access_point_name  , vec[1].c_str(), vec[1].length() ) ;
		memcpy( req.username		   , vec[2].c_str(), vec[2].length() ) ;
		memcpy( req.password           , vec[3].c_str(), vec[3].length() ) ;
		memcpy( req.server_ip          , vec[4].c_str(), vec[4].length() ) ;

		req.tcp_port  = ntouv16( atoi( vec[5].c_str() ) ) ;
		req.udp_port  = ntouv16( atoi( vec[6].c_str() ) ) ;
		req.end_time  = ntouv64( atoi64( vec[7].c_str() ) ) ;

		buf = new char[ len + 1 ] ;
		memset( buf, 0 ,len + 1 ) ;
		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "pas", "DOWN_CTRL_MSG DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_CTRL_MSG_EMERGENCY_MONITORING_REQ ) ;
	}

	if ( buf != NULL ){
		// ������ж�ӦMAP��
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// ת�����ݶ�ӦЭ������D_BASE
char * InterProtoConvert::convert_dbase( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
		const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_BASE operate_value is too long!!!" ) ;
		return NULL ;
	}

	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	int offset   = 0 ;
	len  = sizeof(Header) + sizeof(BaseMsgHeader) + sizeof(Footer) ;
	buf  = new char[ len + 1 ] ;
	memset( buf, 0, len ) ;

	ProtoParse::BuildHeader( header , len , msg_seq, DOWN_BASE_MSG , id ) ;
	memcpy( buf+offset, &header, sizeof(Header) ) ;
	offset += sizeof(Header) ;

	BaseMsgHeader wheader ;
	memcpy( wheader.vehicle_no, device_id.c_str(), device_id.length() ) ;
	wheader.vehicle_color = device_color ;
	wheader.data_type 	  = ntouv16(DOWN_BASE_MSG_VEHICLE_ADDED) ;
	wheader.data_length   = ntouv32( 0 ) ;
	memcpy( buf+offset, &wheader, sizeof(BaseMsgHeader) ) ;
	offset += sizeof(BaseMsgHeader) ;

	Footer footer ;
	memcpy( buf + offset, &footer, sizeof(Footer) ) ;

	OUT_SEND(NULL, 0, "pas", "DOWN_BASE_MSG DOWN_BASE_MSG_VEHICLE_ADDED" ) ;

	out_seq = build_platform_out_seq( mac_id, DOWN_BASE_MSG_VEHICLE_ADDED ) ;

	if ( buf != NULL ){
		// ������ж�ӦMAP��
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// ת�����ݶ�ӦЭ������D_WARN
char * InterProtoConvert::convert_dwarn( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
		const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_WARN operate_value is too long!!!" ) ;
		return NULL ;
	}

	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if ( operator_key == "WARNTODO" ) {  // ��������
		// "����
		// ������Ϣ��Դ��1�������նˣ�2����ҵ���ƽ̨��3���������ƽ̨��9��������|��������(���5.3���������ͱ����)|
		// ����ʱ��(UTCʱ���ʽ)|����ID|�����ֹʱ��(UTCʱ���ʽ)|���켶��0��������1��һ�㣩|������|������ϵ�绰|������ϵ�����ʼ�"

		vector<string> vec = Split(operate_value, "|");
		if ( vec.size() < 9 ) {
			OUT_ERROR( NULL, 0, NULL, "DOWN_WARN_MSG WARNTODO arg less than 9, value %s", operate_value.c_str() ) ;
			return NULL ;
		}

		len = sizeof(DownWarnMsgUrgeTodoReq) ;

		DownWarnMsgUrgeTodoReq req;
		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_WARN_MSG  , id ) ;

		memcpy( req.warn_msg_header.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		req.warn_msg_header.vehicle_color  			= device_color ;
		req.warn_msg_header.data_type      			= ntouv16(DOWN_WARN_MSG_URGE_TODO_REQ) ;
		req.warn_msg_header.data_length    			= ntouv32( sizeof(WarnMsgUrgeTodoBody) ) ;
		req.warn_msg_body.warn_src				    = (unsigned char)(  atoi( vec[0].c_str() ) ) ;
		req.warn_msg_body.warn_type  	   			= ntouv16( atoi( vec[1].c_str() ) ) ;
		req.warn_msg_body.warn_time        			= ntouv64( atoi64( vec[2].c_str() ) ) ;
		req.warn_msg_body.supervision_id			= ntouv32( atoi( vec[3].c_str() ) ) ;
		req.warn_msg_body.supervision_endtime  		= ntouv64( atoi64( vec[4].c_str() ) ) ;
		req.warn_msg_body.supervision_level			= (unsigned char)atoi(vec[5].c_str()) ;
		memcpy( req.warn_msg_body.supervisor 	  , vec[6].c_str(), vec[6].length() ) ;
		memcpy( req.warn_msg_body.supervisor_tel  , vec[7].c_str(), vec[7].length() ) ;
		memcpy( req.warn_msg_body.supervisor_email, vec[8].c_str(), vec[8].length() ) ;

		buf = new char[len + 1] ;
		memset( buf, 0, len ) ;
		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL,0,"pas","DOWN_WARN_MSG DOWN_WARN_MSG_URGE_TODO_REQ");

		// �����ⲿ���д���
		out_seq = build_platform_out_seq( mac_id, DOWN_WARN_MSG_URGE_TODO_REQ ) ;

	} else if ( operator_key == "WARNTIPS" || operator_key == "WARNEXG" ) {  // ����Ԥ����ʵʱ����������Ϣ����Ϣ
		// "����
		// ������Ϣ��Դ��1�������նˣ�2����ҵ���ƽ̨��3���������ƽ̨��9��������|��������(���5.3���������ͱ����)|����ʱ��(UTCʱ���ʽ)|��������"

		vector<string> vec = Split(operate_value, "|");
		if ( vec.size() < 4 ) {
			OUT_ERROR( NULL, 0, NULL, "DOWN_WARN_MSG WARNTIPS arg less than 4, value %s", operate_value.c_str() ) ;
			return NULL ;
		}

		unsigned short data_type = ( operator_key == "WARNTIPS" ) ? DOWN_WARN_MSG_INFORM_TIPS : DOWN_WARN_MSG_EXG_INFORM ;

		int offset   = 0 ;
		int data_len = vec[3].length() ;
		len  = sizeof(Header) + sizeof(WarnMsgHeader) + sizeof(WarnMsgInformTipsBody) + data_len + sizeof(Footer) ;
		buf  = new char[len+1] ;
		memset( buf, 0, len ) ;

		ProtoParse::BuildHeader( header , len , msg_seq, DOWN_WARN_MSG , id ) ;
		memcpy( buf+offset, &header, sizeof(Header) ) ;
		offset += sizeof(Header) ;

		WarnMsgHeader wheader ;
		memcpy( wheader.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		wheader.vehicle_color  = device_color ;
		wheader.data_type 	   = ntouv16( data_type ) ;
		wheader.data_length    = ntouv32( sizeof(WarnMsgInformTipsBody) + data_len ) ;
		memcpy( buf+offset, &wheader, sizeof(WarnMsgHeader) ) ;
		offset += sizeof(WarnMsgHeader) ;

		WarnMsgInformTipsBody  body ;
		body.warn_src    = (unsigned char) atoi( vec[0].c_str() ) ;
		body.warn_type   = ntouv16( atoi(vec[1].c_str()) ) ;
		body.warn_time   = ntouv64( atoi64(vec[2].c_str()) ) ;
		body.warn_length = ntouv32( data_len ) ;
		memcpy( buf+offset, &body, sizeof(WarnMsgInformTipsBody) ) ;
		offset += sizeof(WarnMsgInformTipsBody) ;

		memcpy( buf + offset, vec[3].c_str(), data_len ) ;
		offset += data_len ;

		Footer footer ;
		memcpy( buf + offset, &footer, sizeof(Footer) ) ;

		OUT_SEND(NULL, 0, "pas", "DOWN_WARN_MSG %s" , get_type(data_type) ) ;

		// ������Ӧ��
		if ( operator_key != "WARNEXG" ) {
			out_seq = build_platform_out_seq( mac_id, data_type ) ;
		}
	}else if (operator_key == "UPWARN"){
		vector<string> vec = Split(operate_value, "|");
		if ( vec.size() < 2 ) {
			OUT_ERROR( NULL, 0, NULL, "UP_WARN_MSG UPWARN arg less than 2, value %s", operate_value.c_str() ) ;
			return NULL ;
		}
		
		len = sizeof(UpWarnMsgAdptTodoInfo);
		buf  = new char[len+1] ;
		memset( buf, 0, len ) ;
		UpWarnMsgAdptTodoInfo * pReq = (UpWarnMsgAdptTodoInfo *)buf;
		
		pReq->header.begin._begin= '[';
		ProtoParse::BuildHeader( pReq->header, len , msg_seq, UP_WARN_MSG  , id ) ;		
		memcpy(pReq->warn_msg_header.vehicle_no, device_id.c_str() ,device_id.length() ) ;
		pReq->warn_msg_header.vehicle_color  = device_color ;
		pReq->warn_msg_header.data_type   = ntouv16(UP_WARN_MSG_ADPT_TODO_INFO) ;
		pReq->warn_msg_header.data_length = ntouv32( sizeof(UpWarnMsgAdptTodoInfo) ) ;
		pReq->warn_msg_body.info_id = ntouv32(strtoul(vec[0].c_str(),NULL,10));
		pReq->warn_msg_body.result = (unsigned char)atoi(vec[1].c_str());
		pReq->crc_code = 0;
		pReq->end._end = ']';
		
		OUT_SEND(NULL, 0, "mas", "UP_WARN_MSG %s" , get_type(UP_WARN_MSG_ADPT_TODO_INFO) ) ;
	
	}
		

	if ( buf != NULL && ! out_seq.empty() ){
		// ������ж�ӦMAP��
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// ����MASת����D_MESG��Ϣ
char * InterProtoConvert::convert_mas_dmesg( const string &seq, const string &mac_id, const string &company_id, const string &operator_key,
			const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_PLAT operate_value is too long, %s" , operate_value.c_str() );
		return NULL ;
	}

	Header header ;

	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if ( operator_key == "RETURNSTARTUP" ) {

		len = sizeof( UpExgMsgReturnStartupAck ) ;
		buf = new char[ len + 1  ] ;
		memset( buf, 0, len + 1 ) ;

		UpExgMsgReturnStartupAck req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, UP_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type   = ntouv16( UP_EXG_MSG_RETURN_STARTUP_ACK ) ;
		req.exg_msg_header.data_length = ntouv32( 0 ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND( NULL, 0, "cas2mas", "UP_EXG_MSG %s", get_type(UP_EXG_MSG_RETURN_STARTUP_ACK) );

	} else if ( operator_key == "RETURNEND" ) {

		len = sizeof( UpExgMsgReturnEndAck ) ;
		buf = new char[ len + 1 ] ;
		memset( buf, 0, len + 1 ) ;

		UpExgMsgReturnEndAck req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, UP_EXG_MSG, id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type   = ntouv16( UP_EXG_MSG_RETURN_END_ACK ) ;
		req.exg_msg_header.data_length = ntouv32( 0 ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND( NULL, 0, "cas2mas", "UP_EXG_MSG %s" , get_type(UP_EXG_MSG_RETURN_END_ACK) );

	} else if ( operator_key == "MONITORSTARTUP" ) {

		// ����Ϊ      start_time|end_time
		size_t pos = operate_value.find( '|', 0 ) ;
		if (  pos == string::npos ){
			OUT_ERROR( NULL, 0, NULL, "CAS to MAS UP_PLATFORM_MSG %s data error, value %s", operator_key.c_str(), operate_value.c_str() ) ;
			return NULL ;
		}

		string stime = operate_value.substr( 0, pos ) ;
		string etime = operate_value.substr( pos + 1 ) ;

		len = sizeof( UpExgMsgApplyForMonitorStartup ) ;
		buf = new char[ len + 1 ] ;
		memset( buf , 0 , len + 1 ) ;

		UpExgMsgApplyForMonitorStartup req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, UP_EXG_MSG, id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type   = ntouv16( UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP ) ;
		req.exg_msg_header.data_length = ntouv32( sizeof(uint32)*2 ) ;
		req.start_time				   = ntouv64( atoi64( stime.c_str() ) ) ;
		req.end_time				   = ntouv64( atoi64( etime.c_str() ) ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas2mas", "UP_EXG_MSG UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP" ) ;

		// �����ⲿ���д���
		out_seq = build_platform_out_seq( mac_id, UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP ) ;

	} else if ( operator_key == "MONITOREND" ) {

		len = sizeof( UpExgMsgApplyForMonitorEnd ) ;
		buf = new char[ len + 1 ] ;
		memset( buf, 0, len + 1 ) ;

		UpExgMsgApplyForMonitorEnd req ;
		ProtoParse::BuildHeader( req.header, len , msg_seq, UP_EXG_MSG, id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type   = ntouv16( UP_EXG_MSG_APPLY_FOR_MONITOR_END ) ;
		req.exg_msg_header.data_length = ntouv32( 0 ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas2mas", "UP_EXG_MSG UP_EXG_MSG_APPLY_FOR_MONITOR_END" ) ;

		out_seq = build_platform_out_seq( mac_id, UP_EXG_MSG_APPLY_FOR_MONITOR_END ) ;
	}

	if ( ! out_seq.empty() ) {
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// ����MASת������ƽ̨�������
char * InterProtoConvert::convert_mas_dplat( const string &seq, const string &mac_id, const string &company_id, const string &operator_key,
			const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_PLAT operate_value is too long, %s" , operate_value.c_str() );
		return NULL ;
	}

	Header header ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if ( operator_key == "PLATQUERY" ) {
		//MAS��λѲ�죬�����������Ӫ�̵�һ��������������ȷ����һ�����ƺ������ɫ��mac_id�ڴ˲�û���ã�������Ϊ0_00000
		//�ظ�����Ҫ��Ӫ�̵Ļظ�     �·�ƽ̨�䱨��������Ϣ���λѲ��Э��һ��

		// ����Ϊ      ID|BASE64(Content)
		vector<string> vec;
		if ( ! splitvector( operate_value, vec, "|" , 0 ) ) {
			return NULL ;
		}

		if (vec.size() < 4 ) {
			OUT_ERROR( NULL, 0, NULL, "UP_PLATFORM_MSG PLATQUERY arg less than 4, value %s", operate_value.c_str() ) ;
			return NULL;
		}
		
		string objecttype = vec[0];
		string objectid = vec[1];
		string infoid = vec[2];
		string content = vec[3];
		CBase64 base64;
		if ( ! base64.Decode(content.c_str(),content.length()) ) {
			OUT_ERROR(NULL, 0, NULL, "base64.Base64Decode failed! before de:%s", content.c_str());
			return NULL ;
		}

		char* de_sms_ptr =  base64.GetBuffer() ;
		OUT_INFO(NULL, 0, NULL, "the decode short msg is :%s", de_sms_ptr);

		int data_len = strlen(de_sms_ptr) ;
		len = sizeof(UpPlatformMsgpostqueryAckHeader) + sizeof(UpPlatformMsgpostqueryData) + sizeof(Footer) + data_len ;

		unsigned short data_type = UP_PLATFORM_MSG_POST_QUERY_ACK ;

		UpPlatformMsgpostqueryAckHeader pfm;
		pfm.up_platform_msg.data_type   = ntouv16(data_type) ;
		pfm.up_platform_msg.data_length = ntouv32( sizeof(UpPlatformMsgpostqueryData) + data_len ) ;

		UpPlatformMsgpostqueryData body;
		body.object_type = (unsigned char)atoi(objecttype.c_str());
		memset(body.object_id,0,sizeof(body.object_id));
		strncpy(body.object_id,objectid.c_str(),sizeof(body.object_id));
		body.info_id 	 = ntouv32( atoi(infoid.c_str()) ) ;
		body.msg_len     = ntouv32( data_len ) ;

		Footer footer;
		ProtoParse::BuildHeader( pfm.header, len , msg_seq, UP_PLATFORM_MSG , id ) ;

		buf = new char[len+1] ;
		memset( buf, 0, len+1 ) ;

		int offset = 0 ;

		memcpy(buf+offset , &pfm, sizeof(UpPlatformMsgpostqueryAckHeader) ) ;
		offset += sizeof(UpPlatformMsgpostqueryAckHeader) ;

		memcpy(buf+offset, &body, sizeof(UpPlatformMsgpostqueryData) ) ;
		offset += sizeof(UpPlatformMsgpostqueryData) ;

		memcpy(buf+offset, de_sms_ptr, data_len ) ;
		offset += data_len ;

		memcpy(buf+offset, &footer, sizeof(Footer) ) ;

		OUT_HEX(NULL,0,"cas2mas",buf,len);

		OUT_SEND( NULL, 0, "cas2mas", "UP_PLATFORM_MSG %s" , get_type(data_type) );
	}
	else if ( operator_key == "PLATMSG" ) {

		int infoid = atoi( operate_value.c_str() ) ;

		len = sizeof(UpPlatFormMsgInfoAck) ;

		UpPlatFormMsgInfoAck  ack ;
		ProtoParse::BuildHeader( ack.header , len , msg_seq, UP_PLATFORM_MSG , id ) ;
		ack.up_platform_msg.data_type   = ntouv16( UP_PLATFORM_MSG_INFO_ACK ) ;
		ack.up_platform_msg.data_length = ntouv32( sizeof(int) ) ;
		ack.info_id	= ntouv32( infoid ) ;

		buf = new char[len+1] ;
		memcpy( buf, &ack, sizeof(ack) ) ;
		buf[len] = 0 ;

		OUT_SEND( NULL, 0, "cas2mas" , "UP_PLATFORM_MSG UP_PLATFORM_MSG_INFO_ACK" ) ;
	}

	return buf ;
}

// ת�����ݶ�ӦЭ������D_BASE
char * InterProtoConvert::convert_mas_dbase( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
		const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024 ) {
		OUT_ERROR( NULL, 0, NULL, "D_BASE operate_value is too long!!!" ) ;
		return NULL ;
	}

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	int offset   = 0 ;
	len  = sizeof(UpbaseMsgVehicleAddedAck) + sizeof(Footer) + operate_value.length() ;
	buf  = new char[ len + 1 ] ;
	memset( buf, 0, len ) ;

	UpbaseMsgVehicleAddedAck ack ;

	ProtoParse::BuildHeader( ack.header , len , msg_seq, UP_BASE_MSG , id ) ;
	memcpy( ack.msg_header.vehicle_no, device_id.c_str(), device_id.length() ) ;
	ack.msg_header.vehicle_color = device_color ;
	ack.msg_header.data_type 	  = ntouv16( UP_BASE_MSG_VEHICLE_ADDED_ACK ) ;
	ack.msg_header.data_length    = ntouv32( operate_value.length() ) ;
	memcpy( buf+offset, &ack, sizeof(ack) ) ;
	offset += sizeof(ack) ;

	memcpy( buf + offset, operate_value.c_str(), operate_value.length() ) ;
	offset += operate_value.length() ;

	Footer footer ;
	memcpy( buf + offset, &footer, sizeof(Footer) ) ;

	OUT_SEND(NULL, 0, "mas", "UP_BASE_MSG UP_BASE_MSG_VEHICLE_ADDED" ) ;

	return buf ;
}


// ת�����ݶ�ӦЭ������D_PLAT
char * InterProtoConvert::convert_dplat( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
		const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_PLAT operate_value is too long!!!" );
		return NULL ;
	}

	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if ( operator_key == "PLATQUERY" || operator_key == "PLATMSG" ) {
		//��λѲ�죬�����������Ӫ�̵�һ��������������ȷ����һ�����ƺ������ɫ��mac_id�ڴ˲�û���ã�������Ϊ0_00000
		//�ظ�����Ҫ��Ӫ�̵Ļظ�     �·�ƽ̨�䱨��������Ϣ���λѲ��Э��һ��

		// ����Ϊ      ID|BASE64(Content)

		vector<string> vec;
		if ( ! splitvector( operate_value, vec, "|" , 0 ) ) {
			return NULL ;
		}

		if (vec.size() < 4 ) {
			OUT_ERROR( NULL, 0, NULL, "%s PLATQUERY arg less than 4, value %s", 
				operator_key.c_str(),operate_value.c_str() ) ;
			return NULL;
		}

		string objecttype = vec[0];
		string objectid = vec[1];
		string infoid = vec[2];
		string content = vec[3];		

		CBase64 base64;
		if ( ! base64.Decode(content.c_str(),content.length()) ) {
			OUT_ERROR(NULL, 0, NULL, "base64.Base64Decode failed! before de:%s", content.c_str());
			return NULL ;
		}

		char* de_sms_ptr = base64.GetBuffer() ;
		OUT_INFO(NULL, 0, NULL, "the decode short msg is :%s", de_sms_ptr);

		int data_len = strlen(de_sms_ptr) ;
		len = sizeof(Header) + sizeof(DownPlatformMsg) + sizeof(DownPlatformMsgPostQueryBody) + sizeof(Footer) + data_len ;

		unsigned short data_type = (operator_key == "PLATQUERY") ? DOWN_PLATFORM_MSG_POST_QUERY_REQ : DOWN_PLATFORM_MSG_INFO_REQ ;

		DownPlatformMsg pfm;
		pfm.data_type   = ntouv16(data_type) ;
		pfm.data_length = ntouv32( 2*sizeof(int) + data_len ) ;

		DownPlatformMsgPostQueryBody body;
		body.object_type = (unsigned char)atoi(objecttype.c_str());
		memset(body.object_id,0,sizeof(body.object_id));
		strncpy(body.object_id,objectid.c_str(),sizeof(body.object_id));
		body.info_id 	 = ntouv32( atoi(infoid.c_str()) ) ;
		body.info_length = ntouv32( data_len ) ;

		Footer footer;
		ProtoParse::BuildHeader( header, len , msg_seq, DOWN_PLATFORM_MSG , id ) ;

		buf = new char[len+1] ;
		memset( buf, 0, len+1 ) ;

		int offset = 0 ;

		memcpy(buf+offset , &header, sizeof(Header) ) ;
		offset += sizeof(Header) ;

		memcpy(buf+offset , &pfm, sizeof(DownPlatformMsg) ) ;
		offset += sizeof(DownPlatformMsg) ;

		memcpy(buf+offset, &body, sizeof(DownPlatformMsgPostQueryBody) ) ;
		offset += sizeof(DownPlatformMsgPostQueryBody) ;

		memcpy(buf+offset, de_sms_ptr, data_len ) ;
		offset += data_len ;

		memcpy(buf+offset, &footer, sizeof(Footer) ) ;

		OUT_SEND(NULL,0,"cas", "DOWN_PLATFORM_MSG %s", get_type(data_type) );

		char szid[128] = {0} ;
		sprintf( szid, "0_%s", infoid.c_str() ) ;
		// �����ⲿ���д���
		out_seq = build_platform_out_seq( szid , data_type ) ;

	}

	if ( buf != NULL ){
		// ������ж�ӦMAP��
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// ת�����ݶ�ӦЭ������D_MESG
char * InterProtoConvert::convert_dmesg( const string &seq, const string &mac_id, const string &company_id , const string &operator_key,
			const string &operate_value, int &len )
{
	if (sizeof(Header) + operate_value.length() > 1024) {
		OUT_ERROR( NULL, 0, NULL, "D_MESG operate_value is too long!!!" );
		return NULL ;
	}

	Header header ;
	string out_seq ;

	unsigned int id 	 = atoi(company_id.c_str());
	unsigned int msg_seq = _seq_map.GetSequeue() ;

	char *buf = NULL  ;

	string 	device_id = "00000000";
	unsigned char device_color = 0 ;
	parse_mac_id( mac_id, device_id, device_color ) ;

	if ( operator_key == "REGISTER" ) {



	} else if ( operator_key == "RETURNSTARTUP" ) {

		DownExgMsgReturnStartUp req ;

		len = sizeof(DownExgMsgReturnStartUp) ;
		buf = new char[ len + 1 ] ;
		memset( buf, 0 , len + 1 ) ;

		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no, device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_RETURN_STARTUP ) ;
		req.exg_msg_header.data_length   = ntouv32( sizeof(char) ) ;
		req.reason_code  				 = ( unsigned char ) atoi( operate_value.c_str() ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_RETURN_STARTUP" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_RETURN_STARTUP ) ;

	} else if ( operator_key == "RETURNEND" ) {

		DownExgMsgReturnEnd req ;

		len = sizeof( DownExgMsgReturnEnd ) ;
		buf = new char[ len + 1 ] ;
		memset( buf, 0, len + 1 ) ;

		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_RETURN_END ) ;
		req.exg_msg_header.data_length   = ntouv32( sizeof(char) ) ;
		req.reason_code 			     = ( unsigned char ) atoi( operate_value.c_str() ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_RETURN_END" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_RETURN_END ) ;

	} else if ( operator_key == "MONITORSTARTUP" ) {

		DownExgMsgApplyforMonitorstartUpAck req ;

		len = sizeof( DownExgMsgApplyforMonitorstartUpAck ) ;
		buf = new char[ len +  1 ] ;
		memset( buf, 0, len + 1  ) ;

		ProtoParse::BuildHeader( req.header , len , msg_seq , DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color   = device_color ;
		req.exg_msg_header.data_type       = ntouv16( DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK ) ;
		req.exg_msg_header.data_length     = ntouv32( sizeof(char) ) ;
		req.result						   = ( unsigned char ) atoi( operate_value.c_str() ) ;

		memcpy( buf, &req , len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_APPLY_FOR_MONITOR_STARTUP_ACK ) ;

	} else if ( operator_key == "MONITOREND" ) {

		DownExgMsgApplyforMonitorEndAck req ;

		len = sizeof( DownExgMsgApplyforMonitorEndAck  ) ;
		buf = new char[ len + 1 ] ;
		memset( buf , 0 , len + 1 ) ;

		ProtoParse::BuildHeader( req.header, len , msg_seq, DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str(), device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK ) ;
		req.exg_msg_header.data_length   = ntouv32( sizeof(char) ) ;
		req.result						 = ( unsigned char ) atoi( operate_value.c_str() ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK" ) ;

		out_seq = build_platform_out_seq( mac_id, DOWN_EXG_MSG_APPLY_FOR_MONITOR_END_ACK ) ;

	} else if ( operator_key == "HISGNSSDATA" ) {

		DownExgMsgApplyHisgnssdataAck req ;

		len = sizeof( DownExgMsgApplyHisgnssdataAck ) ;
		buf = new char[ len +  1 ] ;
		memset( buf, 0 , len + 1 ) ;

		ProtoParse::BuildHeader( req.header, len , msg_seq , DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK ) ;
		req.exg_msg_header.data_length   = ntouv32( sizeof(char) ) ;
		req.result 						 = ( unsigned char ) atoi( operate_value.c_str() ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_APPLY_HISGNSSDATA_ACK ) ;

	} else if ( operator_key == "HISTORY"  ) {  // ��������λ����Ϣ

		vector<string>  vec ;
		if ( ! splitvector( operate_value, vec, "|", 0 ) ) {
			OUT_ERROR( NULL, 0, "cas", "DOWN_EXG_MSG_HISTORY_ARCOSSAREA data error : %s" , operate_value.c_str() ) ;
			return NULL ;
		}

		int nsize = (int)vec.size() ;

		DownExgMsgHistoryArcossareaHeader req ;

		len = sizeof( DownExgMsgHistoryArcossareaHeader )  + nsize * sizeof(GnssData) + sizeof(Footer);
		buf = new char[ len + 1 ] ;
		memset( buf, 0, len + 1 ) ;

		ProtoParse::BuildHeader( req.header, len , msg_seq , DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_HISTORY_ARCOSSAREA ) ;
		req.exg_msg_header.data_length   = ntouv32( nsize *sizeof(GnssData) + sizeof(char) ) ;
		req.cnt_num                      = nsize ;

		int offset = 0 ;
		memcpy( buf, &req, sizeof(req) ) ;
		offset += sizeof(req) ;

		for ( int i = 0; i < nsize; ++ i ) {
			GnssData gps ;
			if ( ! convert_gps_info(vec[i], gps) ){
				continue ;
			}
			memcpy( buf + offset, &gps, sizeof(gps) ) ;
			offset += sizeof(gps) ;
		}

		Footer footer ;
		memcpy( buf + offset, &footer, sizeof(footer) ) ;
		offset += sizeof(footer) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_HISTORY_ARCOSSAREA" ) ;

	} else if ( operator_key == "DRIVERINFO" ) {

		// DOWN_EXG_MSG_REPORT_DRIVER_INFO
		DownExgMsgReportDriverInfo req ;

		len = sizeof( DownExgMsgReportDriverInfo ) ;
		buf = new char[ len + 1 ] ;
		memset( buf , 0 , len + 1 ) ;

		ProtoParse::BuildHeader( req.header , len , msg_seq , DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color  =  device_color ;
		req.exg_msg_header.data_type      = ntouv16( DOWN_EXG_MSG_REPORT_DRIVER_INFO ) ;
		req.exg_msg_header.data_length    = ntouv32( 0 ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_REPORT_DRIVER_INFO" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_REPORT_DRIVER_INFO ) ;

	} else if ( operator_key == "EWAYBILL" ) {

		// DOWN_EXG_MSG_TAKE_WAYBILL_REQ
		DownExgMsgTakeWaybillReq req ;

		len = sizeof( DownExgMsgTakeWaybillReq ) ;
		buf = new char[ len + 1 ] ;
		memset( buf, 0 , len + 1 ) ;

		ProtoParse::BuildHeader( req.header , len , msg_seq , DOWN_EXG_MSG , id ) ;

		memcpy( req.exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		req.exg_msg_header.vehicle_color = device_color ;
		req.exg_msg_header.data_type     = ntouv16( DOWN_EXG_MSG_TAKE_WAYBILL_REQ ) ;
		req.exg_msg_header.data_length   = ntouv32( 0 ) ;

		memcpy( buf, &req, len ) ;

		OUT_SEND(NULL, 0, "cas", "DOWN_EXG_MSG DOWN_EXG_MSG_TAKE_WAYBILL_REQ" ) ;

		out_seq = build_platform_out_seq( mac_id , DOWN_EXG_MSG_TAKE_WAYBILL_REQ ) ;

	}
	else if (operator_key == "UPDRIVERINFO"){
		
		vector<string> vec = Split(operate_value, "|");
		if ( vec.size() < 4 ) {
			OUT_ERROR( NULL, 0, NULL, "UP_EXG_MSG UPDRIVERINFO arg less than 4, value %s", operate_value.c_str() ) ;
			return NULL ;
		}

		len = sizeof(UpExgMsgReportDriverInfo);
		buf = new char[len+1];
		memset( buf, 0 , len + 1 ) ;
		UpExgMsgReportDriverInfo * pReq = (UpExgMsgReportDriverInfo *)buf;

		pReq->header.begin._begin='[';
		ProtoParse::BuildHeader( pReq->header , len , msg_seq , UP_EXG_MSG , id ) ;

		memcpy( pReq->exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		pReq->exg_msg_header.vehicle_color = device_color ;
		pReq->exg_msg_header.data_type     = ntouv16( UP_EXG_MSG_REPORT_DRIVER_INFO) ;
		int data_length = sizeof(pReq->driver_name)+sizeof(pReq->driver_id)
			+sizeof(pReq->licence)+sizeof(pReq->org_name);
		pReq->exg_msg_header.data_length   = ntouv32(data_length) ;
		
		strncpy(pReq->driver_name,vec[0].c_str(),sizeof(pReq->driver_name));
		strncpy(pReq->driver_id,vec[1].c_str(),sizeof(pReq->driver_id));
		strncpy(pReq->licence,vec[2].c_str(),sizeof(pReq->licence));
		strncpy(pReq->org_name,vec[3].c_str(),sizeof(pReq->org_name));

		pReq->crc_code = 0;
		pReq->end._end=']';

		OUT_SEND(NULL, 0, "cas", "UP_EXG_MSG_REPORT_DRIVER_INFO" ) ;
		return buf;
			
	} 
	else if (operator_key == "UPEWAYBILL"){

		int data_length = operate_value.length();
		
		len = sizeof( UpExgMsgReportEwaybillInfo ) + data_length + sizeof(uint16_t) + sizeof(End);
		buf = new char[ len + 1 ] ;
		memset( buf, 0 , len + 1 ) ;

		UpExgMsgReportEwaybillInfo * pReq = (UpExgMsgReportEwaybillInfo *)buf;

		pReq->header.begin._begin='[';
		ProtoParse::BuildHeader( pReq->header , len , msg_seq , UP_EXG_MSG , id ) ;

		memcpy( pReq->exg_msg_header.vehicle_no , device_id.c_str() , device_id.length() ) ;
		pReq->exg_msg_header.vehicle_color = device_color ;
		pReq->exg_msg_header.data_type     = ntouv16( UP_EXG_MSG_REPORT_EWAYBILL_INFO) ;
		pReq->exg_msg_header.data_length   = ntouv32( data_length + sizeof(int)) ;

		pReq->ewaybill_length = ntouv32( data_length) ;

		int offset = sizeof(UpExgMsgReportEwaybillInfo);
		memcpy(buf+offset,operate_value.c_str(),data_length);
		offset+=data_length;
		Footer * pFooter = (Footer *)(buf+offset);
		pFooter->crc_code = 0;
		pFooter->end._end =  ']';
		
		OUT_SEND(NULL, 0, "cas", "UP_EXG_MSG UP_EXG_MSG_REPORT_EWAYBILL_INFO" ) ;
		return buf;

	}

	if ( buf != NULL ){
		// ������ж�ӦMAP��
		_seq_map.AddReqMap( out_seq, seq ) ;
	}

	return buf ;
}

// �ͷ�����
void InterProtoConvert::release( char *&data )
{
	if ( data == NULL )
		return ;
	delete [] data ;
	data = NULL ;
}

void InterProtoConvert::clear_timeout_sequeue( const unsigned int timeout )
{
	// ����ʱ����������
	_seq_map.ClearKey( timeout ) ;
}



