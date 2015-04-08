#include "GbProtocolHandler.h"
#include <BaseTools.h>
#include <tools.h>
#include <comlog.h>
#include <arpa/inet.h>

#include <Base64.h>

#include "../../tools/utils.h"

#define  MAX_DWORD_INVALID   0xFFFFFFFF
#define  MAX_WORD_INVALID    0xFFFF

///////////////////////////// GbProtocolHandler /////////////////////////////////
GbProtocolHandler * GbProtocolHandler::_instance = NULL ;
GbProtocolHandler * GbProtocolHandler::getInstance()
{
	if ( _instance == NULL ) {
		_instance = new GbProtocolHandler ;
	}
	_instance->AddRef() ;

	return _instance ;
}

void GbProtocolHandler::FreeHandler( GbProtocolHandler *inst )
{
	if ( inst == NULL ) {
		return ;
	}
	inst->Release() ;
}

PlatFormCommonResp GbProtocolHandler::BuildPlatFormCommonResp(const GBheader*reqheaderptr,
		unsigned short downreq,unsigned char result)
{
	PlatFormCommonResp pcommonresp;
    memcpy(&(pcommonresp.header),reqheaderptr,sizeof(GBheader));

	unsigned short prop  = 0x0005;
	unsigned short msgid = 0x8001;
	msgid = htons(msgid);
	prop  = htons(prop);

	//��ʼ��ͨ�ûظ���Ϣͷ
	memcpy(&(pcommonresp.header.msgtype),&prop,sizeof(unsigned short));
	memcpy(&(pcommonresp.header.msgid),&msgid,sizeof(unsigned short));
	pcommonresp.header.seq = htons(downreq);

	pcommonresp.resp.resp_msg_id = reqheaderptr->msgid;
	pcommonresp.resp.resp_seq 	 = reqheaderptr->seq;
	pcommonresp.resp.result 	 = result;
	pcommonresp.check_sum 		 = get_check_sum((const char*)(&pcommonresp)+1, sizeof(PlatFormCommonResp) - 3);
	pcommonresp.end._end         = 0x7e ;

    return pcommonresp;
}

string GbProtocolHandler::ConvertEngeer( EngneerData *p )
{
	string dest ;
	if ( p == NULL )
		return dest ;
	dest +="4:"+ get_bcd_time(p->time) + ",";
	dest += "210:" + ustodecstr( ntohs(p->speed) ) +"," ;
	dest += "503:" + chartodecstr( p->torque )+"," ;
	dest += "504:" + chartodecstr( p->position ) ;

	return dest ;
}

// ת����ʻ��Ϊ�¼�
string GbProtocolHandler::ConvertEventGps( GpsInfo *gps )
{
	string dest ;
	if ( gps == NULL )
		return dest ;

	// [��ʼλ��γ��][��ʼλ�þ���][��ʼλ�ø߶�][��ʼλ���ٶ�][��ʼλ�÷���][��ʼλ��ʱ��]
	unsigned int lon = 0;
	unsigned int lat = 0;
	lon = (unsigned int)ntohl(gps->longitude)*6 / 10 ;
	lat = (unsigned int)ntohl(gps->latitude)*6 / 10 ;

	dest +="[" + uitodecstr(lat) + "]" ;
	dest +="[" + uitodecstr(lon) + "]" ;
	dest +="[" + ustodecstr(ntohs(gps->heigth)) + "]" ;
	dest +="[" + ustodecstr(ntohs(gps->speed))  + "]" ;
	dest +="[" + ustodecstr(ntohs(gps->direction)) + "]" ;
	dest +="[" + get_bcd_time(gps->date_time) + "]" ;

	return dest ;
}

static unsigned int getdword( const char *buf )
{
	unsigned int dword = 0 ;
	memcpy( &dword, buf, 4 ) ;
	dword = ntohl( dword ) ;
	return dword ;
}

static unsigned short getword( const char *buf )
{
	unsigned short word = 0 ;
	memcpy( &word, buf, 2 ) ;
	word = ntohs( word ) ;
	return word ;
}

static char * getbuffer( const char *buf, char *sz , int len )
{
	memcpy( sz, buf, len ) ;
	sz[len] = 0 ;
	return sz ;
}

// ����MAP��KEY��VALUEֵ�Ĺ�ϵ
static void addmapkey( const string &key, const string &val , map<string,string> &mp )
{
	if ( key.empty() ) {
		return ;
	}

	map<string,string>::iterator it = mp.find( key ) ;
	if ( it == mp.end() ) {
		mp.insert( pair<string,string>( key, val ) ) ;
	} else {
		it->second += "|" ;
		it->second += val ;
	}
}

static const string buildmapcommand( map<string,string> &mp )
{
	string sdata = "" ;
	if ( mp.empty() )
		return sdata ;

	map<string,string>::iterator it ;
	for ( it = mp.begin(); it != mp.end(); ++ it ) {
		if ( ! sdata.empty() ) {
			sdata += "," ;
		}
		sdata += it->first + ":" + it->second ;
	}
	return sdata ;
}

static string itodecstr(unsigned int intger,unsigned int max,bool bflag)
{
	char buf[128] = {0};

	if( max == intger || ( bflag && intger == 0xff) ) {
		sprintf(buf,"%d",-1);
	} else {
		sprintf(buf, "%u", intger);
	}
	return string(buf) ;
}

string GbProtocolHandler::getTermAttribute(const char *data, size_t len)
{
	string msg;
	size_t pos = sizeof(GBheader);
	uint8_t fieldSize;

	// �ն����ͣ�2�ֽ�
	if(pos + 2 > len) {
		return msg;
	}
	msg +=  ",20000:" + uitodecstr(ntohs(*(uint16_t*)(data + pos)));
	pos += 2;

	// ������ID��5�ֽ�
	if(pos + 5 > len) {
		return msg;
	}
	msg += ",20001:" + string(data + pos, strnlen(data + pos, 5));
	pos += 5;

	// �ն��ͺţ�20�ֽ�
	if(pos + 20 > len) {
		return msg;
	}
	msg += ",20002:" + string(data + pos, strnlen(data + pos, 20));
	pos += 20;

	// �ն�ID��7�ֽ�
	if(pos + 7 > len) {
		return msg;
	}
	msg += ",20003:" + string(data + pos, strnlen(data + pos, 7));
	pos += 7;

	// �ն�SIM��ICCID��10�ֽ�
	if(pos + 10 >len) {
		return msg;
	}
	msg += ",20004:" + BCDtostr((char*)data + pos, 10);
	pos += 10;

	// �ն�Ӳ���汾�ų��ȣ�1�ֽ�
	if(pos + 1 > len) {
		return msg;
	}
	fieldSize = *(uint8_t*)(data + pos);
	pos += 1;

	// �ն�Ӳ���汾�ţ�n�ֽ�
	if(pos + fieldSize > len) {
		return msg;
	}
	msg += ",20005:" + string(data + pos, strnlen(data + pos, fieldSize));
	pos += fieldSize;

	// �ն˹̼��汾�ų��ȣ�1�ֽ�
	if(pos + 1 > len) {
		return msg;
	}
	fieldSize = *(uint8_t*)(data + pos);
	pos += 1;

	// �ն˹̼��汾�ţ�n�ֽ�
	if(pos + fieldSize > len) {
		return msg;
	}
	msg += ",20006:" + string(data + pos, strnlen(data + pos, fieldSize));
	pos += fieldSize;

	// GNSS ģ�����ԣ�1�ֽ�
	if(pos + 1 > len) {
		return msg;
	}
	msg += ",20007:" + uitodecstr(*(uint8_t*)(data + pos));
	pos += 1;

	// ͨ��ģ�����ԣ�1�ֽ�
	if(pos + 1 > len) {
		return msg;
	}
	msg += ",20008:" + uitodecstr(*(uint8_t*)(data + pos));
	pos += 1;

	return msg;
}

string GbProtocolHandler::ConvertGpsInfo(GpsInfo*gps_info, const char *append_data, const int append_data_len)
{
	string dest;
	if (gps_info == NULL)
		return dest;

	unsigned int lon = 0;
	unsigned int lat = 0;
	lon = (unsigned int)ntohl(gps_info->longitude)*6 / 10 ;
	lat = (unsigned int)ntohl(gps_info->latitude)* 6 / 10 ;

	map<string,string>  mp ;

	addmapkey( "1" , uitodecstr(lon) , mp ) ;
	addmapkey( "2" , uitodecstr(lat) , mp ) ;
	addmapkey( "3" , uitodecstr(ntohs(gps_info->speed)) , mp ) ;
	addmapkey( "4" , get_bcd_time(gps_info->date_time) , mp ) ;

	//��������Ϊ0��˳ʱ�뷽�򣬵�λΪ2�ȡ�
	addmapkey( "5" , uitodecstr(ntohs(gps_info->direction)) , mp ) ;
	addmapkey( "6" , uitodecstr(ntohs(gps_info->heigth)) , mp ) ;

    /*�ź׸����� ***********************************************************/
	//DWORD,λ�û�����Ϣ״̬λ��B0~B15,�ο�JT/T808-2011,Page15����17
	unsigned int status =0;
	memcpy(&status,&(gps_info->state),sizeof(unsigned int));
	status = ntohl(status) ;
	// ����״̬
	addmapkey( "8" , uitodecstr(status) , mp ) ;

	//����������־λ
	int ala = 0;
	memcpy(&ala,&(gps_info->alarm),sizeof(int));
	ala = ntohl( ala ) ;

	// �����һ��������־λ
	addmapkey( "20", uitodecstr(ala) , mp ) ;

	//������������Ϣ
	if (append_data != NULL && append_data_len > 2)
	{
		unsigned short cur   = 0;
		unsigned char  amid  = 0;
		unsigned char  amlen = 0;
		unsigned short word  = 0;
		unsigned int   dword = 0;

		while(cur +2 < append_data_len)
		{
			word = 0;
			dword = 0;
            amid =  append_data[cur];
            amlen = append_data[cur+1];
            if( cur+2+amlen > append_data_len )
            	break;
            //printf("amid:%x,amlen:%x \n",amid,amlen);
            switch(amid)
            {
            case 0x01://���
            	addmapkey( "9",itodecstr(getdword(append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
            	break;
            case 0x02://������WORD��1/10L����Ӧ�������������
            	addmapkey( "24" , itodecstr(getword(append_data+cur+2),MAX_WORD_INVALID,false) , mp ) ;
            	break;
            case 0x03://��ʻ��¼���ܻ�ȡ���ٶȣ�WORD,1/10KM/h
            	addmapkey( "7" , uitodecstr(getword( append_data+cur+2 ) ) , mp ) ;
            	break;
            case 0x04: // ��Ҫ�˹�ȷ�ϱ����¼���ID��WORD,��1��ʼ����
            	addmapkey( "519" , ustodecstr(getword( append_data+cur+2)) , mp ) ; // ��ҳ����
            	break ;
            case 0x11://���ٱ���������Ϣ
            	if(amlen == 1)
            	{
            		word = append_data[cur+2];
                    if(word == 0) {
                    	addmapkey( "31" , "0|" , mp ) ;
                    }
            	}
            	else if(amlen == 5)
            	{
            		word = append_data[cur+2] ;
                    if(word >=1 && word<=4) {
                    	dword = getdword( append_data+cur+3 ) ;
                    	addmapkey( "31" , ustodecstr(word)+"|"+uitodecstr(dword) , mp ) ;
                    }
            	}
            	break;
            case 0x12://��������/·�߱���������Ϣ
            	if(amlen == 6)
            	{
            	    word = append_data[cur+2];
            	    if(word >=1 && word<=4)
            	    {
            	        dword = getdword( append_data+cur+3 ) ;
            	        unsigned short d = append_data[cur+2+5];
            	        if( d == 0 || d == 1 ){
            	        	addmapkey( "32" , ustodecstr(word)+"|"+uitodecstr(dword)+"|"+ustodecstr(d) , mp ) ;
            	        }
            	    }
            	}
            	break;
            case 0x13://·����ʻʱ�䲻��/��������������Ϣ
            	if(amlen == 7)
            	{
            		dword = getdword( append_data+cur+2 ) ;
            		word  = getword( append_data+cur+6 ) ;

            		unsigned short d = append_data[cur+8];
            	    if( d == 0 || d == 1 ){
            	        addmapkey( "35" , uitodecstr(dword)+"|"+ustodecstr(word)+"|"+ustodecstr(d) , mp ) ;
            	    }
            	}
            	break;
            case 0x14: // ���˹�ȷ�ϵı�����ˮ�ţ��������20-3  4�ֽ�
            	{
            		addmapkey( "520", ustodecstr(getdword( append_data+cur+2 )), mp ) ;
            	}
            	break ;
            case 0xE0: // �����Ϣ����
            	// ToDo:
            	break ;
            case 0x20: //������ת��
               {
            	   addmapkey( "210" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,false),mp) ;
               }
            	break;
            case 0x21: //˲ʱ�ͺ�
               {
            	   addmapkey("216" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,false),mp) ;
               }
            	break;
            case 0x22: // ������Ť�ذٷֱ�
            	{
            		addmapkey( "503" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
            	}
            	break ;
            case 0x23:  // ����̤��λ��
            	{
					addmapkey( "504" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
            	}
            	break ;
            case 0x24: // ��չ����������־λ
            	{
            		// ����Э�����չ��־λ
            		addmapkey( "21" , uitodecstr(getdword( append_data+cur+2 )), mp );//ustodecstr
            	}
            	break;
            case 0x25: // ��չ�����ź�״̬λ
				{
					addmapkey("500", ustodecstr(getdword( append_data+cur+2 )) , mp ) ;
				}
				break ;
            case 0x26: // �ۼ��ͺ�
				{
					addmapkey( "213" , itodecstr(getdword( append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x27:  // 0x00�����ٿ��ţ�0x01�����⿪�ţ�0x02�������ڿ��ţ�����ֵ������1�ַ�
				{
					addmapkey( "217", uitodecstr( (unsigned char)(*(append_data+cur+2)) ) , mp ) ;
				}
				break ;
            case 0x28:  // 0x28 VSS��GPS ������Դ
            	{
            		addmapkey( "218", uitodecstr( (unsigned char)(*(append_data+cur+2))) , mp ) ;
            	}
            	break ;
            case 0x29: // 0x29 �������ͺģ�1bit=0.01L,0=0L
				{
					addmapkey( "219",  itodecstr(getdword( append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x2a: // IO״̬λ
				addmapkey("580", itodecstr(getword(append_data + cur + 2), MAX_WORD_INVALID, false), mp);
            	break;
            case 0x2b: // ģ����
            	addmapkey( "581",  itodecstr(getdword( append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
            	break;
            case 0x30: // ����ͨ�������ź�ǿ��
            	addmapkey( "582", uitodecstr( (unsigned char)(*(append_data+cur+2))) , mp ) ;
            	break;
            case 0x31: // GNSS ��λ������
            	addmapkey( "583", uitodecstr( (unsigned char)(*(append_data+cur+2))) , mp ) ;
            	break;
            case 0x32:  // Զ������״̬����
            	{
            		addmapkey( "570", uitodecstr( (unsigned char)(*(append_data+cur+2))) , mp ) ;
            	}
            	break ;
            case 0x40: // ������������ʱ��
				{
					addmapkey( "505" , itodecstr(getdword( append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x41:  // �ն����õ�ص�ѹ
				{
					addmapkey( "506" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x42:  // ���ص�ѹ
				{
					addmapkey( "507" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x43:  // ������ˮ��
				{
					addmapkey( "214" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
				}
				break ;
            case 0x44:  // �����¶�
				{
					addmapkey( "508" , itodecstr(getword(append_data+cur+2),MAX_WORD_INVALID,true),mp);
				}
				break ;
            case 0x45:  // ��������ȴҺ�¶�
				{
					addmapkey( "509" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
				}
				break ;
            case 0x46:  // �����¶�
				{
					addmapkey( "510" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,false) , mp ) ;
				}
				break ;
            case 0x47:  // ����ѹ��
				{
					addmapkey( "215" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
				}
				break ;
            case 0x48:  // ����ѹ��
				{
					addmapkey( "511" , itodecstr(getword( append_data+cur+2),MAX_WORD_INVALID,true) , mp ) ;
				}
				break ;
            case 0x49:  // ·������
				{
					addmapkey( "549" , itodecstr(getdword( append_data+cur+2),MAX_DWORD_INVALID,false) , mp ) ;
				}
				break;
			case 0xe1: // �����Զ�����Ϣ����
				{
					getCommonExtend((unsigned char*) append_data + cur + 2, amlen, mp);
				}
				break;
            default:
            	break;
            }
            cur += 2+amlen;

		}
	}
/***********************************************************************/
	//dest = "{TYPE:0,RET:0," + dest;
	// �����ڲ�Э������
	dest += buildmapcommand(mp) ;
	//dest += "}";
	/*if ( !astr.empty() )
		dest += astr ;
	*/
	return dest;
}

bool GbProtocolHandler::getCommonExtend(const unsigned char *ptr, int len, map<string, string> &mp)
{
	int pos;

	unsigned char extKey;
	unsigned char extLen;

	char chrVal;
	int intVal;
	string strVal;

	pos = 0;
	while (pos + 2 <= len) {
		extKey = ptr[pos++];
		extLen = ptr[pos++];

		if (pos + extLen > len) {
			break;
		}

		if (extKey == 0x01 && extLen == 1) {
			chrVal = ptr[pos];
			addmapkey("550", Utils::int2str(chrVal, strVal), mp);
		} else if (extKey == 0x02 && extLen == 1) {
			chrVal = ptr[pos];
			addmapkey("551", Utils::int2str(chrVal, strVal), mp);
		} else if (extKey == 0x03 && extLen == 1) {
			chrVal = ptr[pos];
			addmapkey("552", Utils::int2str(chrVal, strVal), mp);
		} else if (extKey == 0x04 && extLen == 4) {
			intVal = ntohl(*(int*) (ptr + pos));
			addmapkey("553", Utils::int2str(intVal, strVal), mp);
		} else if (extKey == 0x05 && extLen == 4) {
			intVal = ntohl(*(int*) (ptr + pos));
			addmapkey("554", Utils::int2str(intVal, strVal), mp);
		} else if (extKey == 0x06 && extLen == 4) {
			intVal = ntohl(*(int*) (ptr + pos));
			addmapkey("555", Utils::int2str(intVal, strVal), mp);
		}

		pos += extLen;
	}

	return true;
}

// ��ȡ��ʻԱ�����Ϣ
bool GbProtocolHandler::GetDriverInfo( const char *buf, int len, DRIVER_INFO &info )
{
	int fieldLen;
	unsigned char *fieldPtr = (unsigned char*) buf;
	unsigned char *fieldEnd = fieldPtr + len;

	if (fieldPtr + 1 > fieldEnd) {
		return false;
	}
	info.state = *fieldPtr;
	fieldPtr += 1;

	if (fieldPtr + 6 > fieldEnd) {
		return false;
	}
	if(memcmp(fieldPtr, "\xff\xff\xff\xff\xff\xff", 6) == 0) {
		info.actionTime = "-1";
	} else {
		info.actionTime = BCDtostr((char*)fieldPtr, 6);
	}
	fieldPtr += 6;

	if(info.state != 1) {
		return true;
	}

	if (fieldPtr + 1 > fieldEnd) {
		return false;
	}
	info.result = *fieldPtr;
	fieldPtr += 1;

	if (fieldPtr + 1 > fieldEnd) {
		return false;
	}
	fieldLen = *fieldPtr;
	fieldPtr += 1;

	if (fieldPtr + fieldLen > fieldEnd) {
		return false;
	}
	info.driverName = string((char*)fieldPtr, fieldLen);
	fieldPtr += fieldLen;

	if (fieldPtr + 20 > fieldEnd) {
		return false;
	}
	fieldLen = strnlen((char*)fieldPtr, 20);
	info.certificateID = string((char*)fieldPtr, fieldLen);
	fieldPtr += 20;

	if (fieldPtr + 1 > fieldEnd) {
		return false;
	}
	fieldLen = *fieldPtr;
	fieldPtr += 1;

	if (fieldPtr + fieldLen > fieldEnd) {
		return false;
	}
	info.organization = string((char*)fieldPtr, fieldLen);
	fieldPtr += fieldLen;

	if (fieldPtr + 4 > fieldEnd) {
		return false;
	}
	info.timeLimit = BCDtostr((char*)fieldPtr, 4);

	return true ;
}
/*
 * buf����Ϣ��
 * buf_len:��Ϣ�峤��
 */
string  GbProtocolHandler::ConvertDriverInfo(char *buf, int buf_len, unsigned char result)
{
	DRIVER_INFO info ;
	if ( ! GetDriverInfo( buf, buf_len , info ) ) {
		return "";
	}

	// �ϱ���ʻԱ���ʶ����
	string dstr = "{TYPE:8,RESULT:" ;
	dstr += uitodecstr(result) ;

	// IC�����״̬
	dstr += ",107:" + uitodecstr(info.state);

	// IC�����ʱ��
	dstr += ",108:" + info.actionTime;

	if(info.state != 1) {
		dstr += "}";
		return dstr;
	}

	// IC����ȡ���
	dstr += ",109:" + uitodecstr(info.result);

	if(info.result != 0) {
		dstr += "}";
		return dstr;
	}

	// ��ʻԱ����
	dstr += ",110:" + info.driverName;

    //��ҵ�ʸ�֤��Ч��
    dstr += ",114:" + info.timeLimit;

    //��ҵ�ʸ�֤����
    dstr += ",112:" + info.certificateID;

    //��֤��������
    dstr += ",113:" + info.organization;

    return dstr + "}";
}

//flag 0:��ȡ��1����
bool  GbProtocolHandler::ConvertGetPara(char *buf, int buf_len, string &data)
{
	/***********�ź׸��޸�8-9*************************************/
	int curn = sizeof(GBheader);

	//unsigned short seq = getword(buf+curn) ;
	curn += 2;

	CBase64 base64;
	unsigned int u32Val;
	string binStr;

	unsigned short pnum = 0;
	pnum = buf[curn++];

	unsigned long pid = 0;
	unsigned char plen = 0;

	map<string, string> mp;

	char pchar[257];
	pchar[0] = 0;

	for (int i = 0; i < pnum; ++i) {
		pid = getdword(buf + curn);
		curn += 4;
		plen = (unsigned char) buf[curn++];
		switch (pid) {
		case 0x0001:
			addmapkey("7", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0002:
			addmapkey("100", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0003:
			addmapkey("101", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0004:
			addmapkey("102", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0005:
			addmapkey("103", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0006:
			addmapkey("104", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0007:
			addmapkey("105", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0010: //APN
			addmapkey("3", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0011:
			addmapkey("4", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0012:
			addmapkey("5", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0013:
			addmapkey("2", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0014: //����APN
			addmapkey("106", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0015:
			addmapkey("107", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0016:
			addmapkey("108", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0017: //����IP
			addmapkey("109", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0018:
			addmapkey("1", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0019: // �����ڲ�Э�����ⲿЭ���Ӧ��ϵ����
			addmapkey("110", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x001a:
			addmapkey("1001a", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x001b:
			addmapkey("1001b", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x001c:
			addmapkey("1001c", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x001d:
			addmapkey("1001d", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0020:
			addmapkey("111", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0021:
			addmapkey("112", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0022:
			addmapkey("113", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0027:
			addmapkey("114", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0028:
			addmapkey("115", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0029:
			addmapkey("116", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x002C:
			addmapkey("117", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x002D:
			addmapkey("118", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x002E:
			addmapkey("119", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x002F:
			addmapkey("120", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0030:
			addmapkey("121", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0031: // ����Χ���뾶
			addmapkey("31", uitodecstr(getword(buf + curn)), mp);
			break;
		case 0x0040: //���ƽ̨�绰����
			addmapkey("10", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0041: //��λ�绰����
			addmapkey("122", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0042:
			addmapkey("123", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0043:
			addmapkey("15", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0044:
			addmapkey("124", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0045:
			addmapkey("125", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0046:
			addmapkey("126", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0047:
			addmapkey("127", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0048:
			addmapkey("9", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0049: //���ƽ̨��Ȩ���ź���
			addmapkey("141", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0050:
			addmapkey("142", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0051:
			addmapkey("143", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0052:
			addmapkey("144", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0053:
			addmapkey("145", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0054:
			addmapkey("146", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0055:
			addmapkey("128", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0056:
			addmapkey("129", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0057:
			addmapkey("130", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0058:
			addmapkey("131", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0059:
			addmapkey("132", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x005A:
			addmapkey("133", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x005B: // ���ٱ���Ԥ����ֵ
			addmapkey("300", ustodecstr(getword(buf + curn)), mp);
			break;
		case 0x005C: // ����ϵ��
			addmapkey("301", ustodecstr(getword(buf + curn)), mp);
			break;
		case 0x005d:
			addmapkey("1005d", uitodecstr(getword(buf + curn)), mp);
			break;
		case 0x005e:
			addmapkey("1005e", uitodecstr(getword(buf + curn)), mp);
			break;
		case 0x0064: //��ʱ����
			u32Val = getdword(buf + curn);
			u32Val = ((u32Val & 0xfffe0000) >> 1) | (u32Val & 0xffff);
			addmapkey("180", uitodecstr(u32Val), mp);
			break;
		case 0x0065: //��������
			u32Val = getdword(buf + curn);
			u32Val = ((u32Val & 0xfffe0000) >> 1) | (u32Val & 0xffff);
			addmapkey("181", uitodecstr(u32Val), mp);
			break;
		case 0x0070:
			addmapkey("136", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0071:
			addmapkey("137", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0072:
			addmapkey("138", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0073:
			addmapkey("139", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0074:
			addmapkey("140", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0080:
			addmapkey("147", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0x0081:
			addmapkey("134", uitodecstr(getword(buf + curn)), mp);
			break;
		case 0x0082:
			addmapkey("135", ustodecstr(getword(buf + curn)), mp);
			break;
		case 0x0083:
			addmapkey("41", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0x0084: // ������ɫ
			addmapkey("42", ustodecstr(buf[curn]), mp);
			break;
		case 0x0090:
			addmapkey("10090" , ustodecstr(buf[curn]) , mp);
			break;
		case 0x0091:
			addmapkey("10091" , ustodecstr(buf[curn]) , mp);
			break;
		case 0x0092:
			addmapkey("10092" , ustodecstr(buf[curn]) , mp);
			break;
		case 0x0093:
			addmapkey("10093" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0x0094:
			addmapkey("10094" , ustodecstr(buf[curn]) , mp) ;
			break;
		case 0x0095:
			addmapkey("10095" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0x0100:
			addmapkey("10100" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0x0101:
			addmapkey("10101" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0x0102:
			addmapkey("10102" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0x0103:
			addmapkey("10103", ustodecstr(getword(buf + curn)), mp);
			break;
		case 0x0110:
			binStr = "";
			if (base64.Encode(buf + curn, plen)) {
				binStr.append(base64.GetBuffer(), base64.GetLength());
			}
			addmapkey("10110", binStr, mp);
			break;
		case 0xf000:
			addmapkey("207" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf001:
			addmapkey("208" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf002:
			addmapkey("209" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf003:
			addmapkey("210" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf004:
			addmapkey("211" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf005:
			addmapkey("212" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf006:
			addmapkey("213" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf007:
			addmapkey("214" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf008:
			addmapkey("1f008" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf009:
			addmapkey("1f009" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf00a:
			addmapkey("216" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf00b:
			addmapkey("217" , ustodecstr(getword(buf+curn)) , mp);
			break;
		case 0xf00c:
			binStr = "";
			if(base64.Encode(buf + curn, plen)) {
				binStr.append(base64.GetBuffer(), base64.GetLength());
			}
			addmapkey("201", binStr, mp);
			break;
		case 0xf00d:
			binStr = "";
			if (base64.Encode(buf + curn, plen)) {
				binStr.append(base64.GetBuffer(), base64.GetLength());
			}
			addmapkey("202", binStr, mp);
			break;
		case 0xf00e:
			addmapkey("1f00e" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf00f:
			addmapkey("1f00f" , uitodecstr(getdword(buf+curn)) , mp );
			break;
		case 0xf010:
			addmapkey("218" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf011:
			addmapkey("219" , ustodecstr(buf[curn]) , mp);
			break;
		case 0xf012:
			addmapkey("1f012" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf013:
			addmapkey("1f013" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf014:
			addmapkey("1f014" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf015:
			addmapkey("1f015", ustodecstr(buf[curn]), mp);
			break;
		case 0xf016:
			addmapkey("1f016" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf017:
			addmapkey("1f017" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf018:
			addmapkey("1f018", Utils::array2hex((uint8_t*) buf + curn, plen), mp);
			break;
		case 0xf019:
			addmapkey("1f019", Utils::array2hex((uint8_t*) buf + curn, plen), mp);
			break;
		case 0xf030:
			if (plen == 6) {
				addmapkey("1f030", Utils::array2hex((uint8_t*) buf + curn, plen).substr(1), mp);
			}
			break;
		case 0xf031:
			if (plen == 6) {
				addmapkey("1f031", Utils::array2hex((uint8_t*) buf + curn, plen).substr(1), mp);
			}
			break;
		case 0xf032:
			if (plen == 6) {
				addmapkey("1f032", Utils::array2hex((uint8_t*) buf + curn, plen).substr(1), mp);
			}
			break;
		case 0xf033:
			if (plen == 6) {
				addmapkey("1f033", Utils::array2hex((uint8_t*) buf + curn, plen).substr(1), mp);
			}
			break;
		case 0xf034:
			addmapkey("1f034", string(buf + curn, plen), mp);
			break;
		case 0xf100:
			addmapkey("200", ustodecstr(buf[curn]), mp);
			break;
		case 0xf101:
			addmapkey("302" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf102:
			addmapkey("303" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf103:
			addmapkey("1f103" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf104:
			addmapkey("304" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf105:
			binStr = "";
			if(base64.Encode(buf+curn, plen)) {
				binStr.append(base64.GetBuffer(), base64.GetLength());
			}
			addmapkey("305" , binStr, mp);
			break;
		case 0xf106:
			addmapkey("306" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf107:
			addmapkey("307" , uitodecstr(getdword(buf+curn)) , mp);
			break;
		case 0xf108:
			addmapkey("187", ustodecstr(buf[curn]), mp);
			break;
		case 0xf109:
			addmapkey("190" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf10a:
			addmapkey("309" , ustodecstr(getword(buf+curn)) , mp );
			break;
		case 0xf10b:
			addmapkey("310", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0xf10c:
			addmapkey("203", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0xf10d:
			addmapkey("204", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0xf10e:
			addmapkey("205", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0xf10f:
			addmapkey("206", getbuffer(buf + curn, pchar, plen), mp);
			break;
		case 0xf110:
			addmapkey("1f110", string(buf + curn, plen), mp);
			break;
		case 0xf111:
			addmapkey("1f111", string(buf + curn, plen), mp);
			break;
		case 0xf112:
			addmapkey("1f112", string(buf + curn, plen), mp);
			break;
		case 0xf113:
			addmapkey("1f113", string(buf + curn, plen), mp);
			break;
		case 0xf114:
			addmapkey("1f114", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0xf120:
			addmapkey("1f120", string(buf + curn, plen), mp);
			break;
		case 0xf121:
			addmapkey("1f121", string(buf + curn, plen), mp);
			break;
		case 0xf122:
			addmapkey("1f122", string(buf + curn, plen), mp);
			break;
		case 0xf123:
			addmapkey("1f123", string(buf + curn, plen), mp);
			break;
		case 0xf124:
			addmapkey("1f124", string(buf + curn, plen), mp);
			break;
		case 0xf125:
			addmapkey("1f125", string(buf + curn, plen), mp);
			break;
		case 0xf126:
			addmapkey("1f126", string(buf + curn, plen), mp);
			break;
		case 0xf127:
			addmapkey("1f127", string(buf + curn, plen), mp);
			break;
		case 0xf128:
			addmapkey("1f128", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0xf129:
			addmapkey("1f129", uitodecstr(getdword(buf + curn)), mp);
			break;
		case 0xf12a: //˫����ƽ̨ҵ��������
			addmapkey("1f12a", ustodecstr(buf[curn]), mp);
			break;
	  }
	  curn += plen;
	  if(curn >=buf_len)
		  break;
	}

	string items = buildmapcommand(mp);
	if(items.empty()) {
		data = "{TYPE:0,RET:0} \r\n";
	} else {
		data = "{TYPE:0,RET:0," + items+ "} \r\n";
	}

	return true;
}

static void setdword( DataBuffer *pbuf, unsigned int msgid, unsigned int dword )
{
	pbuf->writeInt32( msgid ) ;
	pbuf->writeInt8( 4 ) ;
	pbuf->writeInt32( dword ) ;
}

static void setword( DataBuffer *pbuf, unsigned int msgid, unsigned short word )
{
	pbuf->writeInt32( msgid ) ;
	pbuf->writeInt8( 2 ) ;
	pbuf->writeInt16( word ) ;
}

static void setstring( DataBuffer *pbuf ,unsigned int msgid, const char *data, int nlen )
{
	pbuf->writeInt32( msgid ) ;
	pbuf->writeInt8( (uint8_t)nlen ) ;

	if ( nlen > 0 ) {
		pbuf->writeBlock( (void*)data, nlen ) ;
	}
}

/*
static void setbytes( DataBuffer *pbuf ,unsigned int msgid, unsigned char *data, int nlen , int max )
{
	pbuf->writeInt32( msgid ) ;
	pbuf->writeInt8( max ) ;

	if ( nlen >= max ) {
		pbuf->writeBlock( data, max ) ;
	} else {
		pbuf->writeBlock( data , nlen ) ;
		pbuf->writeFill( 0, max-nlen ) ;
	}
}
*/

static void setbyte( DataBuffer *pbuf, unsigned int msgid, unsigned char c )
{
	pbuf->writeInt32( msgid ) ;
	pbuf->writeInt8( 1 ) ;
	pbuf->writeInt8( c ) ;
}

// ���ڲ�Э��Ĳ�������תΪ�ⲿЭ��
bool GbProtocolHandler::buildParamSet( DataBuffer *pbuf , map<string,string> &p_kv_map, unsigned char &pnum )
{
	p_kv_map.erase("TYPE");
	p_kv_map.erase("RETRY");

	if ( p_kv_map.empty() ) {
		return false ;
	}

	CBase64 base64;
	unsigned int u32Val;
	string binStr;
	vector<uint8_t> binBuf;

	string k,v;
	int uskey = 0;

	pnum = 0 ;

	//�������ֲ����б�
	vector<string> vec_v;
	typedef map<string, string>::iterator MapIter;

	for (MapIter p = p_kv_map.begin(); p != p_kv_map.end(); ++p) {
		k = (string) p->first;
		v = (string) p->second;
		if (v.size() > 256)
			continue;

		uskey = Utils::str2int(k.c_str(), uskey, ios::hex);
		switch(uskey) {
		case 0x1://tcp port
			setdword( pbuf, 0x0018, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x2:// master ip
			setstring(pbuf, 0x0013, v.c_str(), v.length());
			pnum++;
			break;
		case 0x3://APN
			setstring( pbuf, 0x0010, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x4://APN username
			setstring( pbuf, 0x0011, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x5://APN pwd
			setstring( pbuf, 0x0012, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x7://�������
			setdword( pbuf, 0x0001, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x9://��������
			setstring(pbuf, 0x0048, v.c_str(), v.length());
			++pnum;
			break;
		case 0x8:   //��������
		case 0x10:  //��������
			setstring(pbuf, 0x0040, v.c_str(), v.length());
			++pnum;
			break;
		case 0x15://���Ķ��ź���
			setstring( pbuf, 0x0043, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x31:
			setword( pbuf, 0x0031, atoi(v.c_str()) ) ; // ��ҳ��������Χ���뾶
			++ pnum ;
			break;
		case 0x41: // ���ó��ƺ�
			setstring( pbuf, 0x0083, v.c_str(), v.length() ) ;
			++ pnum ;
			break;
		case 0x42:  // ���ó�����ɫ
			setbyte( pbuf, 0x0084 , atoi(v.c_str()) ) ;
			pnum ++ ;
			break;
		case 0x100://TCP��ϢӦ��ʱʱ��
			setdword( pbuf, 0x0002, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x101: // TCP�ش�����
			setdword( pbuf, 0x0003, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x102://UDP
			setdword( pbuf, 0x0004, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x103: // UDP�ش�����
			setdword( pbuf, 0x0005, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x104:// SMSӦ��ʱʱ��
			setdword( pbuf, 0x0006, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x105: // SMS�ش�����
			setdword( pbuf, 0x0007, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x106://����APN
			setstring( pbuf, 0x0014, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x107:
			setstring( pbuf, 0x0015, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x108:
			setstring( pbuf, 0x0016, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x109://���ݷ�����IP
			setstring( pbuf, 0x0017, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x110://������UDP�˿�
			setdword( pbuf, 0x0019, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x111://�㱨����
			setdword( pbuf, 0x0020, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x112: // λ�û㱨
			setdword( pbuf, 0x0021, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x113:
			setdword( pbuf, 0x0022, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x114://����ʱλ�û㱨ʱ����
			setdword( pbuf, 0x0027, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x115: // ��������ʱ�㱨ʱ����
			setdword( pbuf, 0x0028, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x116: //
			setdword( pbuf, 0x0029, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x117://ȱʡ����㱨�������λΪ�ף�m����>0
			setdword( pbuf, 0x002C, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x118:
			setdword( pbuf, 0x002D, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x119:
			setdword( pbuf, 0x002E, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x120:
			setdword( pbuf, 0x002F, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x121:
			setdword( pbuf, 0x0030, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x122://��λ�绰���룬�ɲ��ô˵绰���벦���ն˵绰���ն˸�λ
			setstring( pbuf, 0x0041, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x123://�ָ��������õ绰���룬�ɲ��ô˵绰���벦���ն˵绰���ն˻ָ���������
			setstring( pbuf, 0x0042, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x124: // �����ն�SMS�ı���������
			setstring( pbuf, 0x0044, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x125: // �ն˵绰��������
			setdword( pbuf, 0x0045, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x126: // ÿ���ͨ��ʱ��
			setdword( pbuf, 0x0046, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x127: // �����ͨ��ʱ��
			setdword( pbuf, 0x0047, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x128://���ʱ��
			setdword( pbuf, 0x0055, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x129:
			setdword( pbuf, 0x0056, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x130:
			setdword( pbuf, 0x0057, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x131://�����ۼƼ�ʻʱ�����ޣ���λΪ�루s��
			setdword( pbuf, 0x0058, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x132:
			setdword( pbuf, 0x0059, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x133:
			setdword( pbuf, 0x005A, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x134://��������ʡ��ID
			setword( pbuf, 0x0081, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x135:
			setword( pbuf, 0x0082, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x136://ͼ��/��Ƶ����-1��10��1���;
			setdword( pbuf, 0x0070, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x137:
			setdword( pbuf, 0x0071, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x138:
			setdword( pbuf, 0x0072, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x139:
			setdword( pbuf, 0x0073, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x140:
			setdword( pbuf, 0x0074, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x141://���ƽ̨��Ȩ���ź���
			setstring( pbuf, 0x0049, v.c_str(), v.length() ) ;
			pnum ++;
			break;
		case 0x142://����������
			setdword( pbuf, 0x0050, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x143://���������ı�����SMS����
			setdword( pbuf, 0x0051, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x144://�������㿪��
			setdword( pbuf, 0x0052, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x145://��������洢��־
			setdword( pbuf, 0x0053, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x146://�ؼ�������־
			setdword( pbuf, 0x0054, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x147://������̱����
			setdword( pbuf, 0x0080, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x180: //��ʱ����
			u32Val = atoi(v.c_str());
			u32Val = ((u32Val & 0xffff0000) << 1) | (u32Val & 0xffff);
			setdword( pbuf, 0x0064, u32Val) ;
		    pnum ++;
			break;
		case 0x181: //��������
			u32Val = atoi(v.c_str());
			u32Val = ((u32Val & 0xffff0000) << 1) | (u32Val & 0xffff);
			setdword( pbuf, 0x0065, u32Val) ;
		    pnum ++;
			break;
		case 0x187: //�ٶ���Դ����
			setbyte(pbuf, 0xf108, atoi(v.c_str()));
			pnum++;
			break;
		case 0x190: //��ʻԱ��¼���տ���
			setword(pbuf, 0xf109, atoi(v.c_str()) ) ;
			pnum ++;
			break;
		case 0x200: //�¹��ɵ������ϱ�ģʽ
			setbyte(pbuf, 0xf100, atoi(v.c_str()));
			pnum++;
			break;
		case 0x201: //�����ٱ�����ֵ
			binStr = "";
			if (base64.Decode(v.c_str(), v.length())) {
				binStr.assign(base64.GetBuffer(), base64.GetLength());
			}
			setstring(pbuf, 0xf00c, binStr.c_str(), binStr.length());
			pnum++;
			break;
		case 0x202: //�����ٱ�����ֵ
			binStr = "";
			if (base64.Decode(v.c_str(), v.length())) {
				binStr.assign(base64.GetBuffer(), base64.GetLength());
			}
			setstring(pbuf, 0xf00d, binStr.c_str(), binStr.length());
			pnum++;
			break;
		case 0x203: //�����ͺ�
			setstring(pbuf, 0xf10c, v.c_str(), v.length());
			pnum++;
			break;
		case 0x204: //VIN��
			setstring(pbuf, 0xf10d, v.c_str(), v.length());
			pnum++;
			break;
		case 0x205: //��������
			setstring(pbuf, 0xf10e, v.c_str(), v.length());
			pnum++;
			break;
		case 0x206: //�������ͺ�
			setstring(pbuf, 0xf10f, v.c_str(), v.length());
			pnum++;
			break;
		case 0x207: //�յ������ٶȷ�ֵ
			setword(pbuf, 0xf000, atoi(v.c_str()));
			pnum++;
			break;
		case 0x208: //�յ�����ʱ�䷧ֵ
			setword(pbuf, 0xf001, atoi(v.c_str()));
			pnum++;
			break;
		case 0x209: //�յ�����ת�ٷ�ֵ
			setword(pbuf, 0xf002, atoi(v.c_str()));
			pnum++;
			break;
		case 0x210: //��������ת��ֵ
			setword(pbuf, 0xf003, atoi(v.c_str()));
			pnum++;
			break;
		case 0x211: //��������ת����ʱ�䷧ֵ
			setword(pbuf, 0xf004, atoi(v.c_str()));
			pnum++;
			break;
		case 0x212: //�������ٵ�ʱ�䷧ֵ
			setword(pbuf, 0xf005, atoi(v.c_str()));
			pnum++;
			break;
		case 0x213: //���ٵĶ��巧ֵ
			setword(pbuf, 0xf006, atoi(v.c_str()));
			pnum++;
			break;
		case 0x214: //���ٿյ�ʱ�䷧ֵ
			setword(pbuf, 0xf007, atoi(v.c_str()));
			pnum++;
			break;
		case 0x216: //������ת������
			setword(pbuf, 0xf00a, atoi(v.c_str()));
			pnum++;
			break;
		case 0x217: //������ת������
			setword(pbuf, 0xf00b, atoi(v.c_str()));
			pnum++;
			break;
		case 0x218: //ר��Ӧ��������
			setword(pbuf, 0xf010, atoi(v.c_str()));
			pnum++;
			break;
		case 0x219: //����ϵ���Զ���������
			setbyte(pbuf, 0xf011, atoi(v.c_str()));
			pnum++;
			break;
		case 0x300:  //���ٱ���Ԥ����ֵ WORD
			setword( pbuf, 0x005b, atoi(v.c_str()) ) ;
			++ pnum ;
			break ;
		case 0x301: //ƣ�ͼ�ʻԤ����ֵ
			setword( pbuf, 0x005c, atoi(v.c_str()) ) ;
			++ pnum ;
			break ;
		case 0x302: //����ϵ��
			setword(pbuf, 0xf101, atoi(v.c_str()));
			++pnum;
			break;
		case 0x303: //����ÿת������
			setword(pbuf, 0xf102, atoi(v.c_str()));
			++pnum;
			break;
		case 0x304: //��������
			setword(pbuf, 0xf104, atoi(v.c_str()));
			++pnum;
			break;
		case 0x305: //λ����Ϣ�㱨������Ϣ����
			binStr.assign(32, '\0');
			if (base64.Decode(v.c_str(), v.length())) {
				binStr.insert(0, base64.GetBuffer(), base64.GetLength());
			}
			setstring(pbuf, 0xf105, binStr.c_str(), 32);
			++pnum;
			break;
		case 0x306: //�ſ������տ���
			setdword(pbuf, 0xf106, atoi(v.c_str()));
			pnum++;
			break;
		case 0x307: //�ն���Χ��������
			setdword(pbuf, 0xf107, atoi(v.c_str()));
			pnum++;
			break;
		case 0x309: //�ֱ���
			setword(pbuf, 0xf10a, atoi(v.c_str()));
			pnum++;
			break;
		case 0x310: //���Ʒ���
			setstring(pbuf, 0xf10b, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1001a: //IC����֤��������IP
			setstring(pbuf, 0x001a, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1001b: //IC����֤��������TCP�˿�
			setdword(pbuf, 0x001b, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1001c: //IC����֤��������UDP�˿�
			setdword(pbuf, 0x001c, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1001d: //IC����֤���ݷ�����IP
			setstring(pbuf, 0x001d, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1005d: //��ײ��������
			setword(pbuf, 0x005d, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1005e: //�෭��������
			setword(pbuf, 0x005e, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10090: //GNSS ��λģʽ
			setbyte(pbuf, 0x0090, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10091: //GNSS ������
			setbyte(pbuf, 0x0091, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10092: //GNSS ģ����ϸ��λ�������Ƶ��
			setbyte(pbuf, 0x0092, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10093: //GNSS ģ����ϸ��λ���ݲɼ�Ƶ��
			setdword(pbuf, 0x0093, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10094: //GNSS ģ����ϸ��λ�����ϴ���ʽ
			setbyte(pbuf, 0x0090, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10095: //GNSS ģ����ϸ��λ�����ϴ�����
			setdword(pbuf, 0x0095, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10100: //CAN ����ͨ��1 �ɼ�ʱ����
			setdword(pbuf, 0x0100, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10101: //CAN ����ͨ��1 �ϴ�ʱ����
			setword(pbuf, 0x0101, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10102: //CAN ����ͨ��2 �ɼ�ʱ����
			setdword(pbuf, 0x0102, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10103: //CAN ����ͨ��2 �ϴ�ʱ����
			setword(pbuf, 0x0103, atoi(v.c_str()));
			pnum++;
			break;
		case 0x10110: //CAN ����ID �����ɼ�����
			binStr.assign(8, '\0');
			if (base64.Decode(v.c_str(), v.length())) {
				binStr.insert(0, base64.GetBuffer(), base64.GetLength());
			}
			setstring(pbuf, 0x0110, binStr.c_str(), 8);
			pnum++;
			break;
		case 0x1f008: //���ص�ѹ�������޷�ֵ
			setword(pbuf, 0xf008, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f009: //���ص�ѹ�������޷�ֵ
			setword(pbuf, 0xf009, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f00e: //���ص��籨����ֵ
			setword(pbuf, 0xf00e, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f00f: //�ն˶�����Ϣ����
			setdword(pbuf, 0xf00f, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f012: //��չ����������
			setdword(pbuf, 0xf012, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f013: //�ն����߳���ʱ��
			setdword(pbuf, 0xf013, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f014: //�ն�����ʱ��
			setdword(pbuf, 0xf014, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f015: //�ն�����������������
			setbyte(pbuf, 0xf015, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f016: //ҹ���г�����ٶ�
			setdword(pbuf, 0xf016, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f017: //ҹ��������ʻʱ������
			setdword(pbuf, 0xf017, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f018: //ҹ�俪ʼʱ��
			binBuf.resize(2);
			if (v.length() == 4 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf018, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f019: //ҹ�����ʱ��
			binBuf.resize(2);
			if (v.length() == 4 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf019, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f030: //��ά����Ȩ�޵绰����1
			binBuf.resize(6);
			if (v.length() == 11 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf030, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f031: //��ά����Ȩ�޵绰����1
			binBuf.resize(6);
			if (v.length() == 11 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf031, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f032: //��ά����Ȩ�޵绰����1
			binBuf.resize(6);
			if (v.length() == 11 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf032, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f033: //��ά����Ȩ�޵绰����1
			binBuf.resize(6);
			if (v.length() == 11 && Utils::hex2array(v, &binBuf[0]) == binBuf.size()) {
				setstring(pbuf, 0xf033, (char*)&binBuf[0], binBuf.size());
				pnum++;
			}
			break;
		case 0x1f034: //����ƽ̨�ط�����
			setstring(pbuf, 0xf034, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f103: //����ϵ��
			setword(pbuf, 0xf103, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f110: //��Ƶ������APN
			setstring(pbuf, 0xf110, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f111: //��Ƶ����������ͨ�Ų����û���
			setstring(pbuf, 0xf111, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f112: //��Ƶ����������ͨ�Ų�������
			setstring(pbuf, 0xf112, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f113: //��Ƶ��������ַ��IP ������
			setstring(pbuf, 0xf113, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f114: //��Ƶ�������˿�
			setdword(pbuf, 0xf114, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f120: //�ڶ�ƽ̨��������APN
			setstring(pbuf, 0xf120, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f121: //�ڶ�����ƽ̨������������ͨ�Ų����û���
			setstring(pbuf, 0xf121, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f122: //�ڶ�����ƽ̨������������ͨ�Ų�������
			setstring(pbuf, 0xf122, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f123: //�ڶ�����ƽ̨����������ַ
			setstring(pbuf, 0xf123, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f124: //�ڶ�����ƽ̨���ݷ�����APN
			setstring(pbuf, 0xf124, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f125: //�ڶ�����ƽ̨���ݷ���������ͨ�Ų����û���
			setstring(pbuf, 0xf125, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f126: //�ڶ�����ƽ̨���ݷ���������ͨ�Ų�������
			setstring(pbuf, 0xf126, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f127: //�ڶ�����ƽ̨���ݷ�������ַ,IP ������
			setstring(pbuf, 0xf127, v.c_str(), v.length());
			pnum++;
			break;
		case 0x1f128: //�ڶ�����ƽ̨������TCP�˿�
			setdword(pbuf, 0xf128, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f129: //�ڶ�����ƽ̨������UDP�˿�
			setdword(pbuf, 0xf129, atoi(v.c_str()));
			pnum++;
			break;
		case 0x1f12a: //˫����ƽ̨ҵ��������
			setbyte(pbuf, 0xf12a, atoi(v.c_str()));
			pnum++;
			break;
		}
	}

	return true ;
}

unsigned char GbProtocolHandler::get_check_sum(const char *buf,int len)
{
	if(buf == NULL || len < 1)
		return 0;
	unsigned char check_sum = 0;
	for(int i = 0; i < len; i++)
	{
		check_sum ^= buf[i];
	}
	return check_sum;
}

string GbProtocolHandler::get_bcd_time(char bcd[6])
{
	string dest;
	char buf[4] = {0};

	sprintf(buf,"%02x",bcd[0]);

	// 20110308/150234
	unsigned int year = atoi(buf) + 2000;
	dest += uitodecstr(year);

	// memset(buf,0,4);
	sprintf(buf,"%02x",bcd[1]);
	dest += buf;

	sprintf(buf,"%02x",bcd[2]);
	dest += buf;
	dest += "/";

	sprintf(buf,"%02x",bcd[3]);
	dest += buf;
	sprintf(buf,"%02x",bcd[4]);
	dest += buf;
	sprintf(buf,"%02x",bcd[5]);
	dest += buf;
	return dest;
}

//20110304
string GbProtocolHandler::get_date()
{
	string str_date;
	char date[128] = {0};
	time_t t;
	time(&t);
	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;

	sprintf(date, "%04d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
	str_date = date;
	return str_date;
}
//050507
string GbProtocolHandler::get_time()
{
	string str_time;
	char time1[128] = {0};

	time_t t;
	time(&t);
	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;

	sprintf(time1, "%04d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	str_time = time1;
	return str_time;

}

void GbProtocolHandler::bin2hex(unsigned char *bin, size_t len, char *dst)
{
	size_t i;

	for(i = 0; i < len; ++i) {
		sprintf(dst + i * 2, "%02x", bin[i]);
	}
}
