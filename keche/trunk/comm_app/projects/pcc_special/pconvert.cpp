#include "pconvert.h"
#include "pccutil.h"
#include <comlog.h>
#include <Base64.h>
#include <BaseTools.h>
#include <databuffer.h>

#define  MSG_VECHILE_NUM   			"104"       // ���ƺŵĴ���
#define  MSG_ACCESS_CODE    		"201"		// ������Ĵ���
#define  MSG_VECHILE_COLOR			"202"		// ����ɫ����

PConvert::PConvert()
{
	_pEnv     = NULL ;
	_istester = 0 ;   // Ϊ�˹���������⴦��
}

PConvert::~PConvert()
{

}

void PConvert::initenv( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	int nvalue = 0 ;
	if ( _pEnv->GetInteger( "pcc_is_tester", nvalue ) ) {
		_istester = (unsigned int)nvalue ;
	}

	char buf[1028] = {0};
	if ( _pEnv->GetString( "local_picpath", buf ) ) {
		_picdir = buf ;
	}
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

// ת������
char * PConvert::convert_urept( const string &key, const string &ome, const string &phone, const string &val, int &len , unsigned int &msgid , unsigned int &type )
{
	int ntype = 0 ;
	MapString map ;
	if ( ! get_map_header( val, map, ntype ) ) {
		OUT_ERROR( NULL, 0, NULL, "PConvert::convert_urept get map header failed") ;
		return NULL ;
	}

	type = METHOD_OTHER ;

	char *buf = NULL ;

	switch(ntype) {
		case 0:  //·���ͱ�������    λ�û㱨
		case 1:
			{
				UpExgMsgRealLocation msg;
				msg.header.msg_seq     = ntouv32( _seq_gen.get_next_seq() ) ;
				msg.header.msg_len     = ntouv32( sizeof(UpExgMsgRealLocation) ) ;
				//msg.header.access_code = ntouv32(access_code);
				//msg.exg_msg_header.vehicle_color  = car_color;
				//safe_memncpy( msg.exg_msg_header.vehicle_no, car_num.c_str() , sizeof(msg.exg_msg_header.vehicle_no) ) ;

				msg.exg_msg_header.data_length = ntouv32( sizeof(GnssData) ) ;

				if ( ! convert_gps_info( map, msg.gnss_data ) ) {
					OUT_ERROR( NULL, 0, phone.c_str(), "PConvert::convert_urept convert gps failed" ) ;
					return NULL ;
				}

				len = sizeof(UpExgMsgRealLocation) ;
				buf = new char[len+1];
				memset( buf, 0, len+1 );
				memcpy( buf, &msg, len );

				msgid = UP_EXG_MSG_REAL_LOCATION ;
			}
			break;
	    case 2: // ��ʻ��¼��
	        {
	        	int ntemp  = 0;
	        	//�õ�����
	        	if (!get_map_integer(map,"70",ntemp)){
	        		OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_TRAVEL_ACK get command_type failed" ) ;
	        		return NULL ;
	        	}

	        	string temp = "";
	        	//�õ���ʻ��¼������ Base64 ����
	        	if (!get_map_string(map,"61",temp)) {
	        		OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_TRAVEL_ACK get travel data failed" ) ;
	        		return NULL ;
	        	}

				int dlen  = 0 ;
				char *ptr = _pEnv->GetMsgCache()->GetData( key.c_str(), dlen ) ;

				UpCtrlMsgTaketravel msg;
				UpCtrlMsgTaketravel *resp = NULL ;

				if ( ptr == NULL ) {
					msg.header.msg_seq     = ntouv32( _seq_gen.get_next_seq() ) ;
					msg.header.msg_type    = ntouv16( UP_CTRL_MSG ) ;
					msg.ctrl_msg_header.data_type = ntouv16( UP_CTRL_MSG_TAKE_TRAVEL_ACK ) ;
					msg.command_type   	   = ntemp;
					resp = &msg ;
				} else {
					if ( dlen < (int) sizeof(UpCtrlMsgTaketravel) ) {
						OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_TRAVEL_ACK get msg cache length failed, length %d" , dlen ) ;
						return NULL ;
					}
					resp = ( UpCtrlMsgTaketravel *) ptr ;
				}

				// ���Ϊ�˹�����ԣ���Ϊ��������Ĳ�������������ַ���Ϊ��֤ͨ�����ͽ����滻Ϊ�����ݴ���
				if ( _istester & 0x01 ) {

					OUT_INFO( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_TRAVEL_ACK tester travel data"  ) ;
					const char szbuff[] = {0x55,0x7a,0x01,0x00,0x03,0x00,0x00,0x00,0x15,0x01} ;

					len = sizeof(UpCtrlMsgTaketravel)+ sizeof(szbuff) + sizeof(Footer) ;

					resp->ctrl_msg_header.data_length = ntouv32( sizeof(char) + sizeof(int) + sizeof(szbuff) ) ;
					resp->header.msg_len = ntouv32( len ) ;
					resp->travel_length  = ntouv32( sizeof(szbuff) ) ;

					buf = new char[len+1];
					memset(buf, 0, len+1);
					memcpy(buf, resp, sizeof(msg) );
					memcpy( buf + sizeof(msg), szbuff , sizeof(szbuff) ) ;

					Footer footer ;
					memcpy( buf + sizeof(msg) + sizeof(szbuff), &footer, sizeof(footer) ) ;

				} else {
					CBase64 base64;
					base64.Decode(temp.c_str(),temp.length()) ;
					len = sizeof(UpCtrlMsgTaketravel)+ base64.GetLength() + sizeof(Footer) ;

					resp->ctrl_msg_header.data_length = ntouv32( sizeof(char) + sizeof(int) + base64.GetLength() ) ;
					resp->header.msg_len = ntouv32( len ) ;
					resp->travel_length  = ntouv32( base64.GetLength() ) ;


					buf = new char[len+1];
					memset(buf, 0, len+1);
					memcpy(buf, resp, sizeof(msg) );

					if ( base64.GetLength() > 0 ) {
						memcpy( buf + sizeof(msg), base64.GetBuffer() , base64.GetLength() ) ;
					}
					Footer footer ;
					memcpy( buf + sizeof(msg) + base64.GetLength(), &footer, sizeof(footer) ) ;
				}
	        	msgid = UP_CTRL_MSG_TAKE_TRAVEL_ACK ;

	        	if ( ptr != NULL ) {
	        		// �ͷ�����
	        		_pEnv->GetMsgCache()->FreeData( ptr ) ;
	        	}
	        }
	        break;
	    case 3: // �����ϴ�ͼƬ���������������⴦��һ��
	    	{
				string temp ;
				// ȡͼƬ��URL����Ե�ַ
				if ( ! get_map_string( map, "125" , temp ) ) {
					get_map_string( map, "203", temp ) ;
				}
				// ���û��ȡ��ͼƬ�ĵ�ַ
				if ( temp.empty() ) {
					OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_PHOTO_ACK get file path failed" ) ;
					return NULL ;
				}

				// ��ome_phone_msgid��ȡ���·���ָ����Ϣ
				char szKey[256] = {0} ;
				sprintf( szKey, "%s_%s_%d" , ome.c_str(), phone.c_str(), UP_CTRL_MSG_TAKE_PHOTO_ACK ) ;

				int  nlen  =  0 ;
				char *pbuf = _pEnv->GetMsgCache()->GetData( szKey, nlen ) ;
				if ( pbuf == NULL ) {
					OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_PHOTO_ACK get data from cache failed, key %s" , szKey ) ;
					return NULL ;
				}

				UpCtrlMsgTakePhotoAck *ack = ( UpCtrlMsgTakePhotoAck * ) pbuf;

				if ( _istester & 0x02 ) {
					GnssData &gps = ack->ctrl_photo_body.gps ;
					// 1:65522247,2:18072200,20:5,24:100,3:670,4:20120329/180448,5:30,6:802,7:670,8:6147,9:125000
					gps.lon  = ntouv32( 109203745 ) ;
					gps.lat  = ntouv32( 30120333 ) ;
					gps.vec1 = ntouv16( 100 ) ; // �ٶ�808��Ϊ1/10m/s

					time_t ntime  = time(NULL) ;
					struct tm local_tm;
					struct tm *tm = localtime_r( &ntime, &local_tm ) ;

					gps.date[3]   = ( tm->tm_year + 1900)  % 256 ;
					gps.date[2]   = ( tm->tm_year + 1900) / 256 ;
					gps.date[1]   = tm->tm_mon + 1 ;
					gps.date[0]   = tm->tm_mday ;
					gps.time[0]   = tm->tm_hour ;
					gps.time[1]   = tm->tm_min ;
					gps.time[2]   = tm->tm_sec ;

					gps.direction   = ntouv16( 0 ) ;
					gps.state 		= ntouv32(0x03) ;
					gps.alarm 		= ntouv32(0) ;
					gps.vec3  		= ntouv32( 0 ) ;

				} else {
					// ����GPSλ�����������ṹ����
					convert_gps_info( map, ack->ctrl_photo_body.gps ) ;
				}

				// �ӱ��ض�ȡһ��ͼƬ
				int piclen 	  = 0 ;
				char *picdata = NULL ;
				// �������ͼƬ·��Ϊ�վʹ�HTTPȡͼƬ
				if ( ! _picdir.empty() ) {
					char szpath[1024] = {0};
					sprintf( szpath, "%s/%s", _picdir.c_str(), temp.c_str() ) ;
					picdata = ReadFile( szpath , piclen ) ;
				}

				// ����ļ��ڱ���ֱ�Ӷ�ȡ
				if ( picdata != NULL && piclen > 0 ) {
					// ֱ�Ӵӱ�����ȡͼƬ
					ack->header.msg_len = ntouv32( sizeof(UpCtrlMsgTakePhotoAck) + sizeof(Footer) + piclen ) ;
					ack->ctrl_msg_header.data_length = ntouv32( sizeof(UpCtrlMsgTakePhotoBody) + piclen ) ;
					ack->ctrl_photo_body.photo_len 	 = ntouv32( piclen ) ;

					DataBuffer dbuf ;
					dbuf.writeBlock( ack, sizeof(UpCtrlMsgTakePhotoAck) ) ;
					if ( piclen > 0 ) {
						dbuf.writeBlock( picdata, piclen ) ;
					}
					Footer footer ;
					dbuf.writeBlock( &footer, sizeof(footer) ) ;
					// ȡ�ý�������Ϣ
					unsigned int accesscode = ntouv32( ack->header.access_code ) ;
					// �������յ���Ƭ����
					_pEnv->GetPasClient()->HandlePasUpData( accesscode, dbuf.getBuffer(), dbuf.getLength() ) ;

					FreeBuffer( picdata ) ;

					OUT_SEND( NULL, 0, NULL, "UP_CTRL_MSG_TAKE_PHOTO_ACK:%s, picture length %d, path: %s",
							ack->ctrl_msg_header.vehicle_no , nlen , temp.c_str() ) ;
				} else {  // �������ȡ�ļ�
					// ȡ������ŵ�IDֵ
					unsigned int seqid = _pEnv->GetSequeue() ;

					_pEnv->GetCacheKey( seqid, szKey ) ;
					// ���·�������Ŷ�����
					_pEnv->GetMsgCache()->AddData( szKey, pbuf , nlen ) ;
					// �������ж�ȡͼƬ����
					((IMsgClient*)_pEnv->GetMsgClient())->LoadUrlPic( seqid , temp.c_str() ) ;

					OUT_SEND( NULL, 0, NULL, "UP_CTRL_MSG_TAKE_PHOTO_ACK picture path %s", temp.c_str() ) ;
				}
				_pEnv->GetMsgCache()->FreeData( pbuf ) ;
	    	}
	    	break ;
	case 5: {
		int result = 0;
		if (!get_map_integer(map, "18", result) || result != 1) {
			return NULL;
		}

		UpExgMsgRegister msg;
		msg.header.msg_seq = ntouv32(_seq_gen.get_next_seq());
		msg.header.msg_len = ntouv32(sizeof(UpExgMsgRegister));
		msg.header.msg_type = ntouv16(UP_EXG_MSG);
		msg.exg_msg_header.data_type = ntouv16(UP_EXG_MSG_REGISTER); //��ҵ������
		msg.exg_msg_header.data_length = ntouv32(sizeof(UpExgMsgRegister) - sizeof(Header) - sizeof(ExgMsgHeader) - sizeof(Footer));

		sprintf(msg.platform_id, "%s", PLATFORM_ID);

		len = sizeof(UpExgMsgRegister);
		buf = new char[len + 1];
		memset(buf, 0, len + 1);
		memcpy(buf, &msg, len);

		msgid = UP_EXG_MSG_REGISTER;

		type = METHOD_REG;
	}
		break;
	    case 8: // �����ϱ���ʻԱ��Ϣ�ɼ�
	       {
	    	   UpExgMsgReportDriverInfo msg;
	    	   msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq());
	    	   msg.header.msg_len  = ntouv32(sizeof(UpExgMsgReportDriverInfo));
	    	   msg.header.msg_type = ntouv16( UP_EXG_MSG);
	    	   // msg.header.access_code = access_code;
	    	   msg.exg_msg_header.data_type = ntouv16(UP_EXG_MSG_REPORT_DRIVER_INFO);
	    	   msg.exg_msg_header.data_length   = ntouv32(sizeof(UpExgMsgReportDriverInfo) - sizeof(Header) - sizeof(ExgMsgHeader) - sizeof(Footer));

	    	   string temp;
	    	    //��ʻԱ����
	    	   if (get_map_string(map, "110" , temp ))
	    	   {
	    		   safe_memncpy( msg.driver_name,temp.c_str(), sizeof(msg.driver_name));
	    	   }
	    	   //��ʻԱ����
	    	   if ( get_map_string(map, "111" , temp ))
	    	   {
	    		   safe_memncpy( msg.driver_id,temp.c_str(), sizeof(msg.driver_id));
	    	   }
	    	   //��ҵ�ʸ�֤����
	    	   if ( get_map_string(map, "112" , temp ))
	    	   {
	    	   	   safe_memncpy( msg.licence,temp.c_str(), sizeof(msg.licence));
	    	   }
	    	   //��֤��������
	    	   if ( get_map_string(map, "113" , temp ))
	    	   {
	    	   	   safe_memncpy( msg.org_name,temp.c_str(), sizeof(msg.org_name));
	    	   }

	    	   //OUT_SEND(NULL, 0, NULL, "UpExgMsgReportDriverInfo:%s",car_num.c_str());

	    	   len = sizeof(UpExgMsgReportDriverInfo) ;
	    	   buf = new char[len+1];
	    	   memset(buf, 0, len+1);
	    	   memcpy(buf, &msg, len);

	    	   msgid = UP_EXG_MSG_REPORT_DRIVER_INFO ;
	       }
	       break;
	    case 35: // �����ϱ������˵�
			{
				UpExgMsgReportEwaybillInfo msg;
				msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq());
				msg.header.msg_type = ntouv16( UP_EXG_MSG);
				// msg.header.access_code = access_code;
				msg.exg_msg_header.data_type = ntouv16(UP_EXG_MSG_REPORT_EWAYBILL_INFO);

				string temp = "";
				if (get_map_string(map,"87",temp)) { //�����˵�����
					CBase64 base64;
					base64.Decode(temp.c_str(),temp.length());

					len = sizeof(UpExgMsgReportEwaybillInfo)+ base64.GetLength()+ sizeof(Footer);

					msg.exg_msg_header.data_length = ntouv32( sizeof(int) + base64.GetLength() ) ;
					msg.ewaybill_length = ntouv32( base64.GetLength() ) ;
					msg.header.msg_len  = ntouv32( len ) ;

					buf = new char[len+1];
					memcpy( buf, &msg, sizeof(msg) );
					if ( base64.GetLength() > 0 ) {
						memcpy( buf + sizeof(msg), base64.GetBuffer(), base64.GetLength() );
					}
					// ��ӽ������
					Footer footer ;
					memcpy( buf + sizeof(msg) + base64.GetLength(), &footer, sizeof(footer) ) ;
				}
				msgid = UP_EXG_MSG_REPORT_EWAYBILL_INFO ;
				// OUT_SEND(NULL, 0, NULL, "UP_EXG_MSG_Ewaybill:%s", car_num.c_str());
			}
			break;
		case 36: // �ն�ע��,������ʱֻ�����ն˼�Ȩ
			{
				int result = 0 ;
				if ( get_map_integer( map, "45", result ) ) {
					// ����ն�ע��ʧ�ܾ�ֱ�ӷ����ˡ�
					if ( result != 0 ) {
						return NULL ;
					}
				}

				UpExgMsgRegister msg;
				msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq());
				msg.header.msg_len  = ntouv32( sizeof(UpExgMsgRegister) ) ;
				msg.header.msg_type = ntouv16( UP_EXG_MSG);
				msg.exg_msg_header.data_type     = ntouv16( UP_EXG_MSG_REGISTER ) ; //��ҵ������
				//msg.exg_msg_header.vehicle_color = car_color ;
				//safe_memncpy( msg.exg_msg_header.vehicle_no, car_num.c_str(), sizeof(msg.exg_msg_header.vehicle_no) ) ;
				msg.exg_msg_header.data_length   = ntouv32( sizeof(UpExgMsgRegister) - sizeof(Header) - sizeof(ExgMsgHeader) - sizeof(Footer) ) ;
				sprintf( msg.platform_id , "%s" , PLATFORM_ID ) ;

				string temp ;
				// ȡ���������ID
				if ( get_map_string( map, "42" , temp ) ) {
					safe_memncpy( msg.producer_id, temp.c_str(), sizeof(msg.producer_id) ) ;
				}
				// ȡ���ն�����
				if ( get_map_string( map, "43" , temp ) ) {
					safe_memncpy( msg.terminal_model_type, temp.c_str(), sizeof(msg.terminal_model_type) ) ;
				}
				// ȡ���ն�ID
				if ( get_map_string( map, "44" , temp ) ) {
					safe_memncpy( msg.terminal_id, temp.c_str(), sizeof(msg.terminal_id) ) ;
				}
				// ȡ���ֻ�SIM��
				if ( phone.length() > 0 ) {
					//reverse_copy(msg.terminal_simcode, sizeof(msg.terminal_simcode), temp.c_str(),  0 ) ;
					// ǰ�油�㿽��
					reverse_copy(msg.terminal_simcode, sizeof(msg.terminal_simcode), phone.c_str(), 0 );
				}

				len = sizeof(UpExgMsgRegister) ;
				buf = new char[len+1] ;
				memset(buf, 0, len+1) ;
				memcpy(buf, &msg, len) ;

				msgid = UP_EXG_MSG_REGISTER ;
		    }
			break;
		case 38: //�ն˼�Ȩ��Ҫ�첽�ص�ͨ��http,ͨ��pcc �ύ��Service ����sim����,�ն�����
		    {
		    	int result = 0 ;
		    	if ( get_map_integer( map, "48", result ) ) {
		    		// ����ն˼�Ȩʧ�ܾ�ֱ�ӷ����ˡ�
		    		if ( result != 0 ) {
		    			return NULL ;
		    		}
		    	}

		    	UpExgMsgRegister msg;
		    	msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq());
		    	msg.header.msg_len  = ntouv32( sizeof(UpExgMsgRegister) ) ;
		        msg.header.msg_type = ntouv16( UP_EXG_MSG);
		    	msg.exg_msg_header.data_type   = ntouv16(UP_EXG_MSG_REGISTER) ; //��ҵ������
		    	msg.exg_msg_header.data_length = ntouv32( sizeof(UpExgMsgRegister) - sizeof(Header) - sizeof(ExgMsgHeader) - sizeof(Footer));

		    	sprintf( msg.platform_id , "%s" , PLATFORM_ID ) ;

		    	len = sizeof(UpExgMsgRegister) ;
				buf = new char[len+1] ;
				memset(buf, 0, len+1) ;
				memcpy(buf, &msg, len) ;

				msgid = UP_EXG_MSG_REGISTER ;

				type  = METHOD_REG ;
		    }
		    break;
		default:
			{
				//OUT_ERROR( NULL, 0, phone.c_str(), "PConvert::convert_urept not support %s", val.c_str() ) ;
			}
			break ;
	}

	return buf ;
}

// ����D_CTLM
char * PConvert::convert_dctlm( const string &key, const string &val, int &len , unsigned int &msgid )
{
	return NULL ;
}

// ����D_SNDM
char * PConvert::convert_dsndm( const string &key, const string &val, int &len , unsigned int &msgid )
{
	return NULL ;
}

// ��������ͨ��Ӧ����Ϣ
char * PConvert::convert_comm( const string &key, const string &phone, const string &val, int &len, unsigned int &msgid )
{
	MapString map ;
	// ��������
	if ( ! parse_jkpt_value( val , map ) ) {
		OUT_ERROR( NULL, 0, NULL, "parse data %s failed", val.c_str() ) ;
		return NULL ;
	}

	int ret = 0 ;
	if ( ! get_map_integer( map, "RET", ret ) ){
		OUT_ERROR( NULL, 0, phone.c_str(), "PConvert::convert_dctlm get ret failed %s", val.c_str() ) ;
		return NULL ;
	}

	char *buf = _pEnv->GetMsgCache()->GetData( key.c_str(), len ) ;
	if ( buf == NULL || len < (int)( sizeof(Header) + sizeof(BaseMsgHeader) ) ) {
		OUT_ERROR( NULL, 0, phone.c_str(), "UP_CTRL_MSG length %d error" , len ) ;
		return NULL ;
	}

	BaseMsgHeader *ctrl = ( BaseMsgHeader *) ( buf + sizeof(Header) ) ;
	unsigned int datatype = ntouv16( ctrl->data_type ) ;
	switch( datatype )
	{
	case UP_CTRL_MSG_MONITOR_VEHICLE_ACK:   // �������
		{
			// �޸���Ӧ����
			UpCtrlMsgMonitorVehicleAck *resp = ( UpCtrlMsgMonitorVehicleAck *) buf ;
			resp->result = ntouv32(ret); //������� 0 ����ɹ� 1 ����ʧ��

			msgid = UP_CTRL_MSG_MONITOR_VEHICLE_ACK ;
		}
		break ;
	case UP_CTRL_MSG_TEXT_INFO_ACK:  // �����ı��·���ͨ��Ӧ��
		{
			UpCtrlMsgTextInfoAck *resp = ( UpCtrlMsgTextInfoAck *) buf ;
			resp->result = ntouv32(ret) ;

			msgid = UP_CTRL_MSG_TEXT_INFO_ACK ;
		}
		break ;
	case UP_CTRL_MSG_TAKE_TRAVEL_ACK:  // ������ʻ��¼�ǵ�ͨ��Ӧ��
		{
			OUT_RECV( NULL, 0, phone.c_str(), "DOWN_CTRL_MSG_TAKE_TRAVEL_REQ recv common response" ) ;
			// ���Ϊ�˹�����ԣ���Ϊ��������Ĳ�������������ַ���Ϊ��֤ͨ�����ͽ����滻Ϊ�����ݴ���
			if ( _istester & 0x01 ) {
				// ��¼���������е�ָ��
				char *ptr = ( char *) buf ;

				UpCtrlMsgTaketravel *resp = (UpCtrlMsgTaketravel* ) buf ;

				OUT_INFO( NULL, 0, phone.c_str(), "UP_CTRL_MSG_TAKE_TRAVEL_ACK tester travel data common response"  ) ;
				const char *pbuff = "AAAAAAAAAAAAAAAAAAAAAA" ;

				len = sizeof(UpCtrlMsgTaketravel)+ strlen(pbuff) + sizeof(Footer) ;

				resp->ctrl_msg_header.data_length = ntouv32( sizeof(char) + sizeof(int) + strlen(pbuff) ) ;
				resp->header.msg_len = ntouv32( len ) ;
				resp->travel_length  = ntouv32( strlen(pbuff) ) ;

				buf = new char[len+1];
				memset(buf, 0, len+1);
				memcpy(buf, resp, sizeof(UpCtrlMsgTaketravel) );
				memcpy( buf + sizeof(UpCtrlMsgTaketravel), pbuff , strlen(pbuff) ) ;

				Footer footer ;
				memcpy( buf + sizeof(UpCtrlMsgTaketravel) + strlen(pbuff), &footer, sizeof(footer) ) ;

				_pEnv->GetMsgCache()->FreeData( ptr ) ;

				msgid = UP_CTRL_MSG_TAKE_TRAVEL_ACK ;
			}
		}
		break ;
	case UP_CTRL_MSG_EMERGENCY_MONITORING_ACK: // ��������
		{
			UpCtrlMsgEmergencyMonitoringAck *resp = (UpCtrlMsgEmergencyMonitoringAck *)buf;
			resp->result = ntouv32(ret);

			msgid = UP_CTRL_MSG_EMERGENCY_MONITORING_ACK ;
		}
		break;
	default:
		OUT_ERROR( NULL, 0, phone.c_str(), "PConvert::convert_dctlm not support %s", val.c_str() ) ;
		break;
	}
	return buf ;
}

// ת�����Э��
char * PConvert::convert_lprov( const string &key, const string &seqid, const string &val , int &len, string &areacode )
{
	MapString map ;
	if ( ! parse_jkpt_value( val, map ) ) {
		OUT_ERROR( NULL, 0, NULL, "parse data %s failed", val.c_str() ) ;
		return NULL ;
	}

	string stype ;
	if ( ! get_map_string( map , "TYPE", stype ) ) {
		OUT_ERROR( NULL, 0, NULL, "get data %s type failed", val.c_str() ) ;
		return NULL ;
	}

	// ȡ��ʡ��ID
	if ( ! get_map_string( map, "AREA_CODE", areacode ) ) {
		OUT_ERROR( NULL, 0, NULL, "get area code failed, %s", val.c_str() ) ;
		return NULL ;
	}

	char *buf = NULL ;
	// AREA_CODE:ʡ����,CARNO:������ɫ_���ƺ�,ACCESS_CODE:��Ӫ�̽�����,TYPE:XXX,k1:v1...
	if ( stype == "D_PLAT" ) {

		string sval ;
		// ���Ϊƽ̨�����Ϣ����
		if ( get_map_string( map, "PLATQUERY" , sval ) ) {
			// ��ڶ�������ͣ�1����ǰ���ӵ��¼�ƽ̨��2���¼�ƽ̨������һҵ����3���¼�ƽ̨��������ҵ����|��ڶ����ID|��ϢID|Ӧ������"
			vector<string> vec ;
			if ( ! splitvector( sval, vec, "|" , 0 ) ) {
				OUT_ERROR( NULL, 0, areacode.c_str(), "split vector failed, sval %s" , sval.c_str() ) ;
				return NULL ;
			}
			// �����ָ���С���ĸ���ֱ�ӷ���
			if ( vec.size() < 4 ) {
				OUT_ERROR( NULL, 0, areacode.c_str(), "split vector param too less, value %s" , sval.c_str() ) ;
				return NULL ;
			}

			int dlen  = 0 ;
			char *ptr = _pEnv->GetMsgCache()->GetData( key.c_str(), dlen , false ) ;
			if ( ptr == NULL ) {
				OUT_ERROR( NULL, 0, areacode.c_str(), "get plat msg failed , key %s" , key.c_str() ) ;
				return NULL ;
			}

			string content = vec[3] ;
			CBase64 base;
			if ( base.Decode( content.c_str(), content.length() ) ) {
				content = base.GetBuffer() ;
			}
			len = sizeof(UpPlatformMsgPostQueryAck)   + content.length() + sizeof(Footer) ;
			buf = new char[len+1] ;

			// ����ƽ̨��ڵ���Ϣ
			UpPlatformMsgPostQueryAck *rsp = (UpPlatformMsgPostQueryAck *) ptr ;
			rsp->header.msg_len 			  = ntouv32( len ) ;
			rsp->up_platform_msg.data_length  = ntouv32( sizeof(UpPlatformMsgpostqueryData) + content.length() ) ;
			rsp->up_platform_post.msg_len     = ntouv32( content.length() ) ;
			rsp->up_platform_post.object_type = atoi( vec[0].c_str() ) ;
			safe_memncpy( rsp->up_platform_post.object_id, vec[1].c_str(), sizeof(rsp->up_platform_post.object_id) ) ;
			rsp->up_platform_post.info_id     = ntouv32( atoi(vec[2].c_str()) ) ;

			memcpy( buf, rsp, sizeof(UpPlatformMsgPostQueryAck) ) ;
			memcpy( buf+sizeof(UpPlatformMsgPostQueryAck) , content.c_str(), content.length() ) ;

			Footer footer ;
			memcpy( buf+sizeof(UpPlatformMsgPostQueryAck) + content.length(), &footer, sizeof(footer) ) ;

			OUT_PRINT( NULL, 0, areacode.c_str(), "platquery message %s", content.c_str() ) ;

			return buf ;
		}

		// ƽ̨����Ϣ
		if ( get_map_string( map, "PLATMSG", sval ) ) {
			// �ӻ�����ȡ����, ���ƽ̨����Ϣ��ƽ̨�����ʱ��������Դ
			char *ptr = _pEnv->GetMsgCache()->GetData( key.c_str(), len , false ) ;
			if ( buf == NULL ) {
				OUT_ERROR( NULL, 0, areacode.c_str(), "get msg from cache failed, key %s", key.c_str() ) ;
				return NULL ;
			}

			// ���¿�������
			buf = new char[len+1] ;
			memset( buf, 0, len + 1 ) ;
			memcpy( buf, ptr, len ) ;

			// ����ƽ̨��Ϣ��ID
			UpPlatFormMsgInfoAck *ack = (UpPlatFormMsgInfoAck *) buf ;
			ack->info_id = ntouv32( atoi(sval.c_str()) ) ;

			return buf ;
		}

	} else {  // ��ƽ̨��ڴ���
		string macid ;
		// ȡ�ó�����ɫ�ͳ��ƺ�
		if ( ! get_map_string( map, "CARNO" , macid ) ) {
			OUT_ERROR( NULL, 0, areacode.c_str(), "get carno failed, %s", val.c_str() ) ;
			return NULL ;
		}

		unsigned char carcolor = 0 ;
		string carnum ;
		// ͨ��MACIDȡ�ó���ɫ�ͳ��ƺ�
		if ( ! get_carinfobymacid( macid, carcolor, carnum ) ) {
			OUT_ERROR( NULL, 0, areacode.c_str(), "get car color and car num by macid failed, %d" , macid.c_str() ) ;
			return NULL ;
		}

		string sval ;
		// ���Ϊ����ҵ��
		if ( stype == "D_WARN" ) {
			// ��������
			if ( get_map_string( map, "WARNTODO", sval ) ) {
				buf = _pEnv->GetMsgCache()->GetData( key.c_str(), len ) ;
				if ( buf == NULL ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "get msg data failed, key %s, macid %s" , key.c_str(), macid.c_str() ) ;
					return NULL ;
				}
				UpWarnMsgUrgeTodoAck *rsp = ( UpWarnMsgUrgeTodoAck *) buf ;
				rsp->result = atoi(sval.c_str()) ;
				return buf ;
			}

			// �����ϱ�����
			if ( get_map_string( map, "WARNINFO" , sval ) ) {
				// ������Ϣ��Դ��1�������նˣ�2����ҵ���ƽ̨��3���������ƽ̨��9��������|��������(���5.3���������ͱ����)|����ʱ��(UTCʱ���ʽ)|��ϢID|������Ϣ����
				vector<string>  vec ;
				// �������ж��ŷָ��
				if ( ! splitvector( sval , vec, "|" , 0 ) ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "WARNINFO split vector error, key %s, macid %s, value: %s" , key.c_str(), macid.c_str() , sval.c_str() ) ;
					return false ;
				}
				if ( vec.size() < 5 ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "WARNINFO arg too less error, key %s, macid %s, value: %s" , key.c_str(), macid.c_str() , sval.c_str() ) ;
					return false ;
				}

				int nlen = vec[4].length() ;
				len = sizeof(UpWarnMsgAdptInfoHeader) + sizeof(UpWarnMsgAdptInfoBody) + nlen + sizeof(Footer) ;

				UpWarnMsgAdptInfoHeader req ;
				req.header.msg_len  = ntouv32( len ) ;  // �������Ȳ���
				req.header.msg_type = ntouv16( UP_WARN_MSG ) ;
				req.header.msg_seq  = ntouv32( _seq_gen.get_next_seq() ) ;
				req.warn_msg_header.vehicle_color = carcolor ;
				safe_memncpy( req.warn_msg_header.vehicle_no, carnum.c_str(), sizeof(req.warn_msg_header.vehicle_no) ) ;
				req.warn_msg_header.data_type   = ntouv16( UP_WARN_MSG_ADPT_INFO ) ;
				req.warn_msg_header.data_length = ntouv32( sizeof(UpWarnMsgAdptInfoBody) + nlen ) ;

				UpWarnMsgAdptInfoBody body ;
				body.warn_src    = atoi( vec[0].c_str() ) ;
				body.warn_type   = ntouv16( atoi(vec[1].c_str()) ) ;
				body.warn_time   = ntouv64( atoi64(vec[2].c_str()) ) ;
				body.info_id     = ntouv32( atoi(vec[3].c_str()) ) ;
				body.info_length = ntouv32( nlen ) ;

				int offset = 0 ;
				buf = new char[len+1] ;
				memcpy( buf+offset, &req, sizeof(req) ) ;
				offset += sizeof(req) ;

				memcpy( buf+offset, &body, sizeof(body) ) ;
				offset += sizeof(body) ;

				memcpy( buf+offset, vec[4].c_str(), nlen ) ;
				offset += nlen ;

				Footer footer ;
				memcpy( buf+offset, &footer, sizeof(footer) ) ;

				return buf ;
			}

			// ���������ϱ��������
			if ( get_map_string( map, "UPWARN", sval ) ) {
				// ������ϢID|������������0�������У�1���Ѵ�����ϣ�2����������3����������
				size_t pos = sval.find( "|" ) ;
				if ( pos == string::npos ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "upwarn result command error, value: %s" , val.c_str() ) ;
					return NULL ;
				}

				// ��װ�����ϱ��������������ݰ�
				UpWarnMsgAdptTodoInfo req ;
				req.header.msg_len  = ntouv32( sizeof(UpWarnMsgAdptTodoInfo) ) ;
				req.header.msg_type = ntouv16( UP_WARN_MSG ) ;
				req.header.msg_seq  = ntouv32( _seq_gen.get_next_seq() ) ;
				req.warn_msg_header.vehicle_color = carcolor ;
				safe_memncpy( req.warn_msg_header.vehicle_no, carnum.c_str(), sizeof(req.warn_msg_header.vehicle_no) ) ;
				req.warn_msg_header.data_type   = ntouv16( UP_WARN_MSG_ADPT_TODO_INFO ) ;
				req.warn_msg_header.data_length = ntouv32( sizeof(WarnMsgAdptTodoInfoBody) ) ;
				req.warn_msg_body.info_id 		= ntouv32( atoi( sval.c_str() ) ) ;
				req.warn_msg_body.result  		= atoi( sval.substr(pos+1).c_str() ) ;

				len = sizeof(req) ;
				buf = new char[len+1] ;
				memcpy( buf, &req, sizeof(req) ) ;

				return buf ;
			}
		} else if ( stype == "D_MESG" ) {
			// ����·����뽻��������Ϣ
			if ( get_map_string( map, "MONITORSTARTUP" , sval ) ) {
				// ��ʼʱ��(UTCʱ���ʽ)|����ʱ��(UTCʱ���ʽ)
				size_t pos = sval.find( "|" ) ;
				if ( pos == string::npos ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "down command error , value : %s" , val.c_str() ) ;
					return NULL ;
				}
				uint64 start = atoi64( sval.substr(0, pos).c_str() ) ;
				uint64 end   = atoi64( sval.substr( pos+1).c_str() ) ;

				UpExgMsgApplyForMonitorStartup req ;
				req.header.msg_len = ntouv32( sizeof(req) ) ;
				req.header.msg_seq = ntouv32( _seq_gen.get_next_seq() ) ;
				req.exg_msg_header.vehicle_color = carcolor ;
				safe_memncpy( req.exg_msg_header.vehicle_no, carnum.c_str(), sizeof(req.exg_msg_header.vehicle_no) ) ;
				req.exg_msg_header.data_length   = ntouv32( sizeof(uint64) * 2 ) ;
				req.start_time = ntouv64( start ) ;
				req.end_time   = ntouv64( end   ) ;

				len = sizeof(req) ;
				buf = new char[len+1] ;
				memcpy( buf, &req, sizeof(req) ) ;

				_pEnv->GetPasClient()->AddMacId2SeqId( UP_EXG_MSG_APPLY_FOR_MONITOR_STARTUP , macid.c_str(), seqid.c_str() ) ;

				return buf ;
			}

			// ��������������Ϣ
			if ( get_map_string( map , "MONITOREND" , sval ) ) {

				UpExgMsgApplyForMonitorEnd  req ;
				req.header.msg_len = ntouv32( sizeof(req) ) ;
				req.header.msg_seq = ntouv32( _seq_gen.get_next_seq() ) ;
				req.exg_msg_header.vehicle_color = carcolor ;
				safe_memncpy( req.exg_msg_header.vehicle_no, carnum.c_str(), sizeof(req.exg_msg_header.vehicle_no) ) ;
				req.exg_msg_header.data_length   = ntouv32( 0 ) ;

				len = sizeof(req) ;
				buf = new char[len+1] ;
				memcpy( buf, &req, sizeof(req) ) ;

				_pEnv->GetPasClient()->AddMacId2SeqId( UP_EXG_MSG_APPLY_FOR_MONITOR_END , macid.c_str(), seqid.c_str() ) ;

				return buf ;
			}

			// ����ƽ̨�����·�������һ��ƽ̨�Զ��·����������ָ��ʱ�䳵����Ϣ
			if ( get_map_string( map, "HISGNSSDATA" , sval ) ) {
				// ��ʼʱ��(UTCʱ���ʽ)|����ʱ��(UTCʱ���ʽ)
				size_t pos = sval.find( "|" ) ;
				if ( pos == string::npos ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "down command error , value : %s" , val.c_str() ) ;
					return NULL ;
				}
				uint64 start = atoi64( sval.substr(0, pos).c_str() ) ;
				uint64 end   = atoi64( sval.substr( pos+1).c_str() ) ;

				UpExgApplyHisGnssDataReq req ;
				req.header.msg_len  = ntouv32( sizeof(req) ) ;
				req.header.msg_type = ntouv16( UP_EXG_MSG ) ;
				req.header.msg_seq  = ntouv32( _seq_gen.get_next_seq() ) ;
				req.exg_msg_header.vehicle_color = carcolor ;
				safe_memncpy( req.exg_msg_header.vehicle_no, carnum.c_str(), sizeof(req.exg_msg_header.vehicle_no) ) ;
				req.exg_msg_header.data_length   = ntouv32( sizeof(uint64) * 2 ) ;
				req.exg_msg_header.data_type	 = ntouv16( UP_EXG_MSG_APPLY_HISGNSSDATA_REQ ) ;
				req.start_time = ntouv64( start ) ;
				req.end_time   = ntouv64( end   ) ;

				len = sizeof(req) ;
				buf = new char[len+1] ;
				memcpy( buf, &req, sizeof(req) ) ;

				_pEnv->GetPasClient()->AddMacId2SeqId( UP_EXG_MSG_APPLY_HISGNSSDATA_REQ , macid.c_str(), seqid.c_str() ) ;

				return buf ;
			}

			// ������ʷ�����ϱ�
			size_t pos = val.find( "HISTORY" ) ;
			// ȡ�ó�������ʷλ�������
			if ( pos != string::npos ) {
				size_t end = val.find( "}" , pos+8 ) ;
				if ( end == string::npos || end < pos ){
					OUT_ERROR( NULL, 0, areacode.c_str(), "HISTORY split vector error, key %s, macid %s, value: %s" , key.c_str(), macid.c_str() , val.c_str() ) ;
					return false;
				}
				sval = val.substr( pos+8, end - pos - 8 ) ;
				vector<string>  vec ;
				// �������ж��ŷָ��
				if ( ! splitvector( sval , vec, "|" , 0 ) ) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "HISTORY split vector error, key %s, macid %s, value: %s" , key.c_str(), macid.c_str() , sval.c_str() ) ;
					return false ;
				}

				int nsize = vec.size() ;

				UpExgMsgHistoryHeader msg ;
				msg.header.msg_type = ntouv16( UP_EXG_MSG ) ;
				msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq() ) ;
				msg.exg_msg_header.vehicle_color = carcolor ;
				safe_memncpy( msg.exg_msg_header.vehicle_no, carnum.c_str(), sizeof(msg.exg_msg_header.vehicle_no) ) ;
				msg.exg_msg_header.data_type   = ntouv16(UP_EXG_MSG_HISTORY_LOCATION) ;

				DataBuffer dbuf ;

				unsigned char ncount = 0 ;
				for ( int i = 0; i < nsize; ++ i ) {
					MapString gpsmap ;
					if ( ! split2map( vec[i] , gpsmap ) ) {
						continue ;
					}
					GnssData gnssdata;
					if ( ! convert_gps_info( gpsmap, gnssdata ) ){
						continue ;
					}
					dbuf.writeBlock( &gnssdata, sizeof(GnssData) ) ;
					++ ncount ;
				}

				len = sizeof(msg) + sizeof(char) + ncount*sizeof(GnssData) + sizeof(Footer) ;
				msg.exg_msg_header.data_length = ntouv32( sizeof(char) + ncount*sizeof(GnssData) ) ;
				msg.header.msg_len			   = ntouv32( len ) ;

				buf = new char[len+1] ;

				int offset = 0 ;
				memcpy( buf, &msg, sizeof(msg) ) ;
				offset += sizeof(msg) ;

				memcpy( buf+offset , &ncount, sizeof(char) )  ;
				offset += sizeof(char) ;

				memcpy( buf+offset, dbuf.getBuffer(), dbuf.getLength() ) ;
				offset += dbuf.getLength() ;

				Footer footer ;
				memcpy( buf+offset, &footer, sizeof(footer) ) ;

				return buf ;
			}

			// �������809������ܣ��ύ������̬���ݣ���ҵ������ΪDOWN_EXG_MSG_CAR_INFO
			if (get_map_string(map, "U_BASE", sval)) {
				CBase64 base64;
				if( ! base64.Decode(sval.c_str(), sval.length())) {
					OUT_ERROR( NULL, 0, areacode.c_str(), "base64 decode error, value : %s" , val.c_str() ) ;
					return NULL;
				}

				len = sizeof(DownExgMsgCarInfoHeader) + base64.GetLength() + sizeof(Footer);
				buf = new char[len + 1];

				DownExgMsgCarInfoHeader msg;
				msg.header.msg_len  = ntouv32( len ) ;
				msg.header.msg_seq  = ntouv32( _seq_gen.get_next_seq() ) ;
				msg.header.msg_type = ntouv16( UP_EXG_MSG );
				msg.exg_msg_header.vehicle_color = carcolor ;
				strncpy(msg.exg_msg_header.vehicle_no, carnum.c_str(),	sizeof(msg.exg_msg_header.vehicle_no));
				msg.exg_msg_header.data_type = ntouv16(DOWN_EXG_MSG_CAR_INFO);
				msg.exg_msg_header.data_length = ntouv32(base64.GetLength());

				int msgpos = 0;
				memcpy(buf + msgpos, &msg, sizeof(DownExgMsgCarInfoHeader));
				msgpos += sizeof(DownExgMsgCarInfoHeader);

				memcpy(buf + msgpos, base64.GetBuffer(), base64.GetLength());
				msgpos += base64.GetLength();

				Footer footer ;
				memcpy( buf + msgpos, &footer, sizeof(footer) ) ;

				return buf ;
			}
		}
	}
	return NULL ;
}

// �ͷŻ���
void PConvert::free_buffer( char *buf )
{
	if ( buf == NULL )
		return ;
	// �ͷŻ�������
	delete [] buf ;
	buf = NULL ;
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

bool PConvert::get_phoneome( const string &macid, string &phone, string &ome )
{
	if ( macid.length() < PHONE_LEN ) {
		return false ;
	}

	size_t pos = macid.find( '_' ) ;
	if ( pos == string::npos ) {
		return false ;
	}

	phone = macid.substr( pos+1 ) ;
	ome   = macid.substr( 0, pos ) ;

	return true ;
}

bool PConvert::get_carinfobymacid( const string &macid, unsigned char &carcolor, string &carnum )
{
	if ( macid.length() < 3 ) {
		return false ;
	}

	size_t pos = macid.find( '_' ) ;
	if ( pos == string::npos ) {
		return false ;
	}
	carcolor = atoi( macid.c_str() ) ;
	carnum   = macid.substr( pos+1 ) ;

	return true ;
}

// ������ʱ��ת�ɸ�����ұʱ��
static bool checktime( int nyear, int nmonth, int nday, int nhour, int nmin, int nsec )
{
	struct tm curtm ;
	curtm.tm_year = nyear - 1900 ;
	curtm.tm_mon  = nmonth - 1 ;
	curtm.tm_mday = nday ;
	curtm.tm_hour = nhour ;
	curtm.tm_min  = nmin ;
	curtm.tm_sec  = nsec ;

	time_t now = time(NULL) ;
	time_t t   = mktime( &curtm ) ;  // ת�ɸ�����ұʱ��

	// ʱ��ǰ��һ��֮��
	if ( t < now - 86400 || t > now + 86400 ) {
		return false ;
	}
	return true ;
}

// ��GPS����ת��GNSS
bool PConvert::convert_gps_info( MapString &mp, GnssData &gps )
{
	if ( mp.empty() )
		return false ;

	memset(&gps, 0x00, sizeof(GnssData));

	int nval = 0 ;
	if ( get_map_integer( mp, "1", nval ) ) {
		nval = nval * 10 / 6 ;
	}
	// �����Ȳ����й���Χ�� 619066885
	if ( nval < 72000000 || nval > 140000000 ){  // ���ȷ�Χ72-136
		OUT_ERROR( NULL, 0, NULL, "error lon %u", nval ) ;
		return false ;
	}
	gps.lon = ntouv32( nval ) ;

	nval = 0;
	if ( get_map_integer( mp, "2", nval ) ) {
		nval = nval * 10 / 6 ;
	}
	// ����γ�Ȳ����й���Χ��
	if ( nval < 18000000 || nval > 55000000 ) {  // γ�ȷ�Χ18-54
		OUT_ERROR( NULL, 0, NULL, "error lat %u", nval ) ;
		return false ;
	}
	gps.lat = ntouv32( nval ) ;

	nval = 0;
	if ( get_map_integer( mp, "3", nval ) ) {
		nval = nval/10 ; // �ٶ�808��Ϊ1/10km/h
	}
	// �����ٶȲ���ȷ
	if ( nval > 220 )  {  // 220km/h
		OUT_ERROR( NULL, 0, NULL, "error speed %u", gps.vec1 ) ;
		return  false ;
	}
	gps.vec1 = ntouv16( nval ) ;

	nval = 0;
	if ( get_map_integer( mp, "7", nval ) ) {
		nval /= 10; // ��ʻ��¼���ٶ�808��Ϊ1/10km/h
	}
	gps.vec2 = (nval == 0) ? gps.vec1 : ntouv16( nval );

	string sval ;
	if ( ! get_map_string( mp, "4", sval ) ) {
		OUT_ERROR( NULL, 0, NULL, "error time empty" ) ;
		// ���û��ʱ���ֱ�ӷ�����
		return false ;
	}

	int nyear = 0 , nmonth = 0 , nday = 0 , nhour = 0 ,nmin = 0 , nsec = 0 ;

	sscanf( sval.c_str(), "%04d%02d%02d/%02d%02d%02d", &nyear, &nmonth, &nday, &nhour, &nmin, &nsec ) ;
	// ���ʱ���Ƿ���ȷ
	if ( ! checktime( nyear, nmonth, nday, nhour, nmin, nsec ) ) {
		OUT_ERROR( NULL, 0, NULL, "error time %s", sval.c_str() ) ;
		return false ;
	}

 	gps.date[3]   = nyear  % 256 ;
	gps.date[2]   = nyear / 256 ;
	gps.date[1]   = nmonth ;
	gps.date[0]   = nday ;

	gps.time[0]   = nhour ;
	gps.time[1]   = nmin ;
	gps.time[2]   = nsec ;

	// ���
	if ( get_map_integer( mp, "9" , nval ) ) {
		gps.vec3  = ntouv32( nval / 10 );
	}

	// ����
	if ( get_map_integer( mp, "5", nval )){
		gps.direction = (nval > 360) ? ntouv16(360) : ntouv16( nval ) ;
	}

	// ����
	if ( get_map_integer( mp, "6", nval )){
		gps.altitude = (nval > 8000) ? ntouv16(8000) : ntouv16( nval ) ;
	}

	// ����
	if ( get_map_integer( mp, "20" , nval ) ) {
		gps.alarm = ntouv32(nval) ;
	}

	// ״̬
	if ( get_map_integer( mp, "8" , nval ) ) {
		gps.state = ntouv32(nval) ;
	}

	return true ;
}

// ת�����ڲ�Э�鴦��
void PConvert::build_gps_info( string &dest, GnssData *gps_data )
{
	dest.clear();

	dest += "1:" + uitodecstr(ntouv32(gps_data->lon) * 6 / 10) + ",";
	dest += "2:" + uitodecstr(ntouv32(gps_data->lat) * 6 / 10) + ",";
	dest += "3:" + ustodecstr(ntouv16(gps_data->vec1)*10) + ",";

	unsigned int iyear = ((unsigned char) (gps_data->date[2])) * 256 + (unsigned char) (gps_data->date[3]);
	string year   = uitodecstr(iyear);
	string month  = charto2decstr(gps_data->date[1]);
	string day    = charto2decstr(gps_data->date[0]);

	string hour   = charto2decstr(gps_data->time[0]);
	string minute = charto2decstr(gps_data->time[1]);
	string second = charto2decstr(gps_data->time[2]);

	dest += "4:" + year + month + day + "/" + hour + minute + second + ",";

	dest += "5:"  + ustodecstr(ntouv16(gps_data->direction)) + ",";
        dest += "6:"  + ustodecstr(ntouv16(gps_data->altitude)) + ",";
	dest += "9:" + ustodecstr(ntouv32(gps_data->vec3) * 10)  + ",";
	dest += "8:"  + uitodecstr(ntouv32(gps_data->state)) + ",";
	dest += "20:" + ustodecstr(ntouv32(gps_data->alarm)) ;
}
