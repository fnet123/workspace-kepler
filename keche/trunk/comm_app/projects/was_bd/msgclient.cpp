/**********************************************
 * MsgClient.cpp
 *
 *  Created on: 2010-7-12
 *      Author: LiuBo
 *       Email:liubo060807@gmail.com
 *    Comments:
 *********************************************/

#include "msgclient.h"
#include "comlog.h"
#include <GBProtoParse.h>
#include <Base64.h>
#include <tools.h>
#include <intercoder.h>

#include "../tools/utils.h"
#include "httpquery.h"

MsgClient::MsgClient( CacheDataPool &cache_data_pool ) :
		_cache_data_pool( cache_data_pool ), _dataqueue( this )
{
	_gb_proto_handler = GbProtocolHandler::getInstance();
	_thread_num = 8;
}

MsgClient::~MsgClient( )
{
	Stop();
	if ( _gb_proto_handler != NULL ) {
		GbProtocolHandler::FreeHandler( _gb_proto_handler );
		_gb_proto_handler = NULL;
	}
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv;

	char ip[128] = { 0 };
	if ( ! pEnv->GetString( "msg_connect_ip", ip ) ) {
		INFO_PRT( NULL, 0,"MsgClient", "Get msg_connect_ip failed" );
		return false;
	}
	_client_user._ip = ip;

	int port = 0;
	if ( ! pEnv->GetInteger( "msg_connect_port", port ) ) {
		INFO_PRT( NULL, 0,"MsgClient", "Get msg_connect_port failed" );
		return false;
	}
	_client_user._port = port;

	int thread = 0;
	if ( pEnv->GetInteger( "msg_thread_count", thread ) ) {
		_thread_num = thread;
	}

	char buf[1024] = { 0 };
	if ( pEnv->GetString( "msg_user_name", buf ) ) {
		_client_user._user_name = buf;
	}

	if ( pEnv->GetString( "msg_user_pwd", buf ) ) {
		_client_user._user_pwd = buf;
	}

	_client_user._fd = NULL;
	_client_user._user_type = "PIPE";
	_client_user._user_state = User::OFF_LINE;
	_client_user._connect_info.reconnect_times = 0;
	_client_user._socket_type = User::TcpConnClient;
	_client_user._connect_info.keep_alive = AlwaysReConn;
	_client_user._connect_info.timeval = 3;



	// ���÷ְ�����
	setpackspliter( & _pack_spliter );

	if ( ! pEnv->GetString( "base_filedir" , buf ) ) {
		printf( "load base_filedir failed\n" ) ;
		return false ;
	}

	int nvalue = 0;
	pEnv->GetInteger( "sendcache_speed", nvalue ) ;

	char temp[1024] = {0} ;
	sprintf( temp, "%s/wasv4", buf ) ;

	return _dataqueue.Init( temp, nvalue );
}

bool MsgClient::Start( void )
{
	return StartClient( _client_user._ip.c_str(), _client_user._port, _thread_num );
}

void MsgClient::Stop( void )
{
	StopClient();
}

void MsgClient::HandleUpData( const char *data, int len )
{
	if(len < 3) { //�ո�\r\n�����ֽ�
		return;
	}

	User &user = _client_user;
	if ( user._user_state != User::ON_LINE || user._fd == NULL ) {
		// �����ʱ���������
		_dataqueue.WriteCache( WAS_CLIENT_ID, ( void* ) data, len );
		return;
	}

	// ʵ�������·�Ӧ�����ϴ��Ŷ�
	if ( ! SendRC4Data( user._fd, data, len ) ) {
		OUT_ERROR( user._ip.c_str(), user._port, user._user_id.c_str() , "Send data failed");
		_dataqueue.WriteCache( WAS_CLIENT_ID, ( void * ) data, len );
		return;
	}

	OUT_SEND3( user._ip.c_str(), user._port, "SEND" , "%.*s" , len -3 , data );
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len )
{
	if ( len < 4 ) return;

	const char *ip = sock->_szIp ;
	int port = sock->_port ;

	string line( ( const char * ) data, len - 2);

	OUT_RECV3( ip, port, "MsgClient", "%.*s", line.size(), line.c_str() );

	User &user = _client_user;

	vector< string > vec_temp;
	if ( ! splitvector( line, vec_temp, " ", 0 ) )
		return;

	string head = vec_temp[0];

	if ( head == "LACK" ) {
		/*
		 RESULT
		 >=0:Ȩ��ֵ
		 -1:�������
		 -2:�ʺ��Ѿ���¼
		 -3:�ʺ��Ѿ�ͣ��
		 -4:�ʺŲ�����
		 -5:sql��ѯʧ��
		 -6:δ��¼���ݿ�
		 */
		if ( vec_temp.size() < 2 )
			return;

		string result = vec_temp[1];
		if ( result == "0" ) {//��¼�ɹ�
			user._user_state = User::ON_LINE;
			user._last_active_time = time( 0 );
			OUT_WARNING( ip, port, "MsgClient", "LOGI received LACK,login was success!" );
		} else if ( result == "-2" ) {
			OUT_WARNING( ip, port, "MsgClient", "LOGI received LACK,login fail -2, user already login" );
		} else {  // ����Ϊ��½��֤ʧ��
			OUT_WARNING( ip, port, "MsgClient", "LOGI received LACK,login fail %s" , result.c_str() );
		}

	} else if ( head == "NOOP_ACK" ) {
		// �����û����һ��ʱ��
		user._last_active_time = time( 0 );
	} else if ( head == "CAITS" ) {
		//OUT_RECV( NULL, 0,"MsgClient", "MsgClient:%s", line.c_str() );
		//��������ָ�
		if ( vec_temp.size() < 6 )
			return;

		if ( vec_temp[4] == "D_CALL" ) {
			HandleDcallMsg( line );
		} else if ( vec_temp[4] == "D_SETP" ) {
			HandleDsetpMsg( line );
		} else if ( vec_temp[4] == "D_GETP" )  // ȡ�ò�ѯ����
				{
			HandleDgetpMsg( line );
		} else if ( vec_temp[4] == "D_CTLM" ) {
			HandleDctlmMsg( line );
		} else if ( vec_temp[4] == "D_SNDM" ) {
			HandleDsndmMsg( line );
		} else if ( vec_temp[4] == "D_REQD" )  // ������ý��ָ��
				{
			HandleReqdMsg( line );
		}
	}
#ifdef _GPS_STAT
	else if ( head == "START" || head == "STOP" )
	{
		// ͨ��ָ���������Ƿ����
		_pEnv->SetGpsState( (head == "START") );
	}
#endif
}

// �������
static int split2map( vector< string > &vec, map< string, string > &mp, const char *split )
{
	mp.clear();

	int count = 0;
	int len = strlen( split );

	map< string, string >::iterator it;
	for ( int i = 0 ; i < ( int ) vec.size() ; ++ i ) {
		size_t pos = vec[i].find( split, 0 );
		if ( pos == string::npos ) {
			continue;
		}

		string key = vec[i].substr( 0, pos );
		string value = vec[i].substr( pos + len );
		if ( ! value.empty() ) {
			++ count;
		}
		it = mp.find( key );
		if ( it != mp.end() ) {  // ������ڶ��ͬ�����д���
			it->second += "|";
			it->second += value;
		} else {
			mp.insert( make_pair( key, value ) );
		}
	}
	return count;
}

// ����151��[]����
static bool parsemultidata( const string &content, const string &key, vector< string > &vec, const char cbegin,
		const char cend )
{
	vec.clear();

	size_t pos = content.find( key.c_str() );
	if ( pos == string::npos ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command param empty, line:%s" , content.c_str() );
		return false;
	}
	string sval = content.substr( pos + key.length() );
	if ( sval.empty() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command param %s error, line:%s" , key.c_str(), content.c_str() );
		return false;
	}

	size_t end = 0;
	pos = sval.find( cbegin, 0 );
	while ( pos != string::npos ) {
		end = sval.find( cend, pos + 1 );
		if ( end == string::npos ) {
			break;
		}
		// ��Ž�������������[]�ָ�����
		vec.push_back( sval.substr( pos + 1, end - pos - 1 ) );
		pos = sval.find( cbegin, end + 1 );
	}
	if ( vec.empty() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command %s empty, line %s" , key.c_str(), content.c_str() );
		return false;
	}
	return true;
}

// ���ַ���ʱ��תΪBCDʱ��
static void convert2bcd( const string &time, char bcd[6] )
{
	time_t ntime = atoll( time.c_str() );
	struct tm local_tm;
	struct tm *tm = localtime_r( & ntime, & local_tm );

	char buf[128] = { 0 };
	sprintf( buf, "%02d%02d%02d%02d%02d%02d", ( tm->tm_year + 1900 ) % 100, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
			tm->tm_min, tm->tm_sec );

	// ת��ΪBCDʱ��
	strtoBCD( buf, bcd );
}

// �۷�����
static bool splitmsgheader( const string &line, string &seq, string &macid, string &carid, string &command,
		string &content, map< string, string > &mp )
{
	vector< string > vec_temp;
	splitvector( line, vec_temp, " ", 0 );
	if ( vec_temp.size() < 6 )
		return false;

	seq = vec_temp[1];
	macid = vec_temp[2];
	command = vec_temp[4];
	content = vec_temp[5];

	if ( content.empty() )
		return false;

	string::size_type pos = macid.find( '_', 0 );
	if ( pos == string::npos ) {
		return false;
	}
	// ȡ�ó���
	carid = macid.substr( pos + 1 );

	//ȥ�����ߵĴ�����
	content.assign( content, 1, content.length() - 2 );

	vector< string > vk;
	splitvector( content, vk, ",", 0 );
	//��������������p_kv_map��
	split2map( vk, mp, ":" );

	return ( ! mp.empty() );
}

// ������ѯ����ʱʵ��ֻ��һ��
void MsgClient::HandleDgetpMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > kvmap;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, kvmap ) ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	string type = "";
	map< string, string >::iterator it = kvmap.find( "TYPE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}

	// ������������
	_SetData dval;

	string cache_key = "";
	type = ( string ) it->second;
	if ( type == "0" ) { // �ն˲�����ѯ
		//��ȡ����
		it = kvmap.find( "VALUE" );
		if(it == kvmap.end()) {
			dval.msgid = 0x8104; // ��ѯȫ���ն˲���
		} else {
			dval.msgid = 0x8106; // ��ѯָ���ն˲���

			size_t i;
			uint32_t  idVal;
			vector<uint32_t> gb808Ids;
			vector<string> innerIds;

			Utils::splitStr(it->second, innerIds, '|');
			for (i = 0; i < innerIds.size(); ++i) {
				Utils::str2int(innerIds[i].c_str(), idVal, ios::hex);
				switch (idVal) {
				case 0x1: //tcp port
					gb808Ids.push_back(htonl(0x0018));
					break;
				case 0x2: //IP
					gb808Ids.push_back(htonl(0x0013));
					break;
				case 0x3: //APN
					gb808Ids.push_back(htonl(0x0010));
					break;
				case 0x4: //APN username
					gb808Ids.push_back(htonl(0x0011));
					break;
				case 0x5: //APN pwd
					gb808Ids.push_back(htonl(0x0012));
					break;
				case 0x7: //�������
					gb808Ids.push_back(htonl(0x0001));
					break;
				case 0x10:  //��������
					gb808Ids.push_back(htonl(0x0040));
					break;
				case 0x9:  //��������
					gb808Ids.push_back(htonl(0x0048));
					break;
				case 0x15:  //���Ķ��ź���
					gb808Ids.push_back(htonl(0x0043));
					break;
				case 0x31:
					gb808Ids.push_back(htonl(0x0031));
					break;
				case 0x41: // ���ó��ƺ�
					gb808Ids.push_back(htonl(0x0083));
					break;
				case 0x42:  // ���ó�����ɫ
					gb808Ids.push_back(htonl(0x0084));
					break;
				case 0x100:  //TCP��ϢӦ��ʱʱ��
					gb808Ids.push_back(htonl(0x0002));
					break;
				case 0x101: // TCP�ش�����
					gb808Ids.push_back(htonl(0x0003));
					break;
				case 0x102: //UDP
					gb808Ids.push_back(htonl(0x0004));
					break;
				case 0x103: // UDP�ش�����
					gb808Ids.push_back(htonl(0x0005));
					break;
				case 0x104: // SMSӦ��ʱʱ��
					gb808Ids.push_back(htonl(0x0006));
					break;
				case 0x105: // SMS�ش�����
					gb808Ids.push_back(htonl(0x0007));
					break;
				case 0x106: //����APN
					gb808Ids.push_back(htonl(0x0014));
					break;
				case 0x107:
					gb808Ids.push_back(htonl(0x0015));
					break;
				case 0x108:
					gb808Ids.push_back(htonl(0x0016));
					break;
				case 0x109: //���ݷ�����IP
					gb808Ids.push_back(htonl(0x0017));
					break;
				case 0x110: //������UDP�˿�
					gb808Ids.push_back(htonl(0x0019));
					break;
				case 0x111: //�㱨����
					gb808Ids.push_back(htonl(0x0020));
					break;
				case 0x112: // λ�û㱨
					gb808Ids.push_back(htonl(0x0021));
					break;
				case 0x113:
					gb808Ids.push_back(htonl(0x0022));
					break;
				case 0x114: //����ʱλ�û㱨ʱ����
					gb808Ids.push_back(htonl(0x0027));
					break;
				case 0x115: // ��������ʱ�㱨ʱ����
					gb808Ids.push_back(htonl(0x0028));
					break;
				case 0x116: //
					gb808Ids.push_back(htonl(0x0029));
					break;
				case 0x117: //ȱʡ����㱨�������λΪ�ף�m����>0
					gb808Ids.push_back(htonl(0x002c));
					break;
				case 0x118:
					gb808Ids.push_back(htonl(0x002d));
					break;
				case 0x119:
					gb808Ids.push_back(htonl(0x002e));
					break;
				case 0x120:
					gb808Ids.push_back(htonl(0x002f));
					break;
				case 0x121:
					gb808Ids.push_back(htonl(0x0030));
					break;
				case 0x122: //��λ�绰���룬�ɲ��ô˵绰���벦���ն˵绰���ն˸�λ
					gb808Ids.push_back(htonl(0x0041));
					break;
				case 0x123: //�ָ��������õ绰���룬�ɲ��ô˵绰���벦���ն˵绰���ն˻ָ���������
					gb808Ids.push_back(htonl(0x0042));
					break;
				case 0x124: // �����ն�SMS�ı���������
					gb808Ids.push_back(htonl(0x0044));
					break;
				case 0x125: // �ն˵绰��������
					gb808Ids.push_back(htonl(0x0045));
					break;
				case 0x126: // ÿ���ͨ��ʱ��
					gb808Ids.push_back(htonl(0x0046));
					break;
				case 0x127: // �����ͨ��ʱ��
					gb808Ids.push_back(htonl(0x0047));
					break;
				case 0x128: //���ʱ��
					gb808Ids.push_back(htonl(0x0055));
					break;
				case 0x129:
					gb808Ids.push_back(htonl(0x0056));
					break;
				case 0x130:
					gb808Ids.push_back(htonl(0x0057));
					break;
				case 0x131: //�����ۼƼ�ʻʱ�����ޣ���λΪ�루s��
					gb808Ids.push_back(htonl(0x0058));
					break;
				case 0x132:
					gb808Ids.push_back(htonl(0x0059));
					break;
				case 0x133:
					gb808Ids.push_back(htonl(0x005a));
					break;
				case 0x134: //��������ʡ��ID
					gb808Ids.push_back(htonl(0x0081));
					break;
				case 0x135:
					gb808Ids.push_back(htonl(0x0082));
					break;
				case 0x136: //ͼ��/��Ƶ����-1��10��1���;
					gb808Ids.push_back(htonl(0x0070));
					break;
				case 0x137:
					gb808Ids.push_back(htonl(0x0071));
					break;
				case 0x138:
					gb808Ids.push_back(htonl(0x0072));
					break;
				case 0x139:
					gb808Ids.push_back(htonl(0x0073));
					break;
				case 0x140:
					gb808Ids.push_back(htonl(0x0074));
					break;
				case 0x141: //���ƽ̨��Ȩ���ź���
					gb808Ids.push_back(htonl(0x0049));
					break;
				case 0x142: //����������
					gb808Ids.push_back(htonl(0x0050));
					break;
				case 0x143: //���������ı�����SMS����
					gb808Ids.push_back(htonl(0x0051));
					break;
				case 0x144: //�������㿪��
					gb808Ids.push_back(htonl(0x0052));
					break;
				case 0x145: //��������洢��־
					gb808Ids.push_back(htonl(0x0053));
					break;
				case 0x146: //�ؼ�������־
					gb808Ids.push_back(htonl(0x0054));
					break;
				case 0x147: //������̱����
					gb808Ids.push_back(htonl(0x0080));
					break;
				case 0x180: //��ʱ����
					gb808Ids.push_back(htonl(0x0064));
					break;
				case 0x181: //��������
					gb808Ids.push_back(htonl(0x0065));
					break;
				case 0x187:
					gb808Ids.push_back(htonl(0xf108));
					break;
				case 0x190:
					gb808Ids.push_back(htonl(0xf109));
					break;
				case 0x200: //�¹��ɵ������ϱ�ģʽ
					gb808Ids.push_back(htonl(0xf100));
					break;
				case 0x201: //�����ٱ�����ֵ
					gb808Ids.push_back(htonl(0xf00c));
					break;
				case 0x202: //�����ٱ�����ֵ
					gb808Ids.push_back(htonl(0xf00d));
					break;
				case 0x203: //�����ͺ�
					gb808Ids.push_back(htonl(0xf10c));
					break;
				case 0x204: //VIN��
					gb808Ids.push_back(htonl(0xf10d));
					break;
				case 0x205: //��������
					gb808Ids.push_back(htonl(0xf10e));
					break;
				case 0x206: //�������ͺ�
					gb808Ids.push_back(htonl(0xf10f));
					break;
				case 0x207: //�յ������ٶȷ�ֵ
					gb808Ids.push_back(htonl(0xf000));
					break;
				case 0x208: //�յ�����ʱ�䷧ֵ
					gb808Ids.push_back(htonl(0xf001));
					break;
				case 0x209: //�յ�����ת�ٷ�ֵ
					gb808Ids.push_back(htonl(0xf002));
					break;
				case 0x210: //��������ת��ֵ
					gb808Ids.push_back(htonl(0xf003));
					break;
				case 0x211: //��������ת����ʱ�䷧ֵ
					gb808Ids.push_back(htonl(0xf004));
					break;
				case 0x212: //�������ٵ�ʱ�䷧ֵ
					gb808Ids.push_back(htonl(0xf005));
					break;
				case 0x213: //���ٵĶ��巧ֵ
					gb808Ids.push_back(htonl(0xf006));
					break;
				case 0x214: //���ٿյ�ʱ�䷧ֵ
					gb808Ids.push_back(htonl(0xf007));
					break;
				case 0x216: //������ת������
					gb808Ids.push_back(htonl(0xf00a));
					break;
				case 0x217: //������ת������
					gb808Ids.push_back(htonl(0xf00b));
					break;
				case 0x218: //ר��Ӧ��������
					gb808Ids.push_back(htonl(0xf010));
					break;
				case 0x219: //����ϵ���Զ���������
					gb808Ids.push_back(htonl(0xf011));
					break;
				case 0x300:  //���ٱ���Ԥ����ֵ WORD
					gb808Ids.push_back(htonl(0x005b));
					break;
				case 0x301: //ƣ�ͼ�ʻԤ����ֵ
					gb808Ids.push_back(htonl(0x005c));
					break;
				case 0x302: //����ϵ��
					gb808Ids.push_back(htonl(0xf101));
					break;
				case 0x303: //����ÿת������
					gb808Ids.push_back(htonl(0xf102));
					break;
				case 0x304: //������������λΪL
					gb808Ids.push_back(htonl(0xf104));
					break;
				case 0x305: //λ����Ϣ�㱨������Ϣ����
					gb808Ids.push_back(htonl(0xf105));
					break;
				case 0x306: //�ſ������տ���
					gb808Ids.push_back(htonl(0xf106));
					break;
				case 0x307: //�ն���Χ��������
					gb808Ids.push_back(htonl(0xf107));
					break;
				case 0x309: //�ֱ���
					gb808Ids.push_back(htonl(0xf10a));
					break;
				case 0x310: //���Ʒ���
					gb808Ids.push_back(htonl(0xf10b));
					break;
				case 0x1001a: //��·����֤IC����֤��������IP��ַ������
					gb808Ids.push_back(htonl(0x001a));
					break;
				case 0x1001b: //��·����֤IC����֤��������TCP�˿�
					gb808Ids.push_back(htonl(0x001b));
					break;
				case 0x1001c: //��·����֤IC����֤��������UDP�˿�
					gb808Ids.push_back(htonl(0x001c));
					break;
				case 0x1001d: //��·����֤IC����֤���ݷ�����IP��ַ������
					gb808Ids.push_back(htonl(0x001d));
					break;
				case 0x1005d: //��ײ������������
					gb808Ids.push_back(htonl(0x005d));
					break;
				case 0x1005e: //�෭������������
					gb808Ids.push_back(htonl(0x005e));
					break;
				case 0x10090: //GNSS ��λģʽ
					gb808Ids.push_back(htonl(0x0090));
					break;
				case 0x10091: //GNSS ������
					gb808Ids.push_back(htonl(0x0091));
					break;
				case 0x10092: //GNSS ģ����ϸ��λ�������Ƶ��
					gb808Ids.push_back(htonl(0x0092));
					break;
				case 0x10093: //GNSS ģ����ϸ��λ���ݲɼ�Ƶ��
					gb808Ids.push_back(htonl(0x0093));
					break;
				case 0x10094: //GNSS ģ����ϸ��λ�����ϴ���ʽ
					gb808Ids.push_back(htonl(0x0094));
					break;
				case 0x10095: //GNSS ģ����ϸ��λ�����ϴ�����
					gb808Ids.push_back(htonl(0x0095));
					break;
				case 0x10100: //CAN ����ͨ��1 �ɼ�ʱ����
					gb808Ids.push_back(htonl(0x0100));
					break;
				case 0x10101: //CAN ����ͨ��1 �ϴ�ʱ����
					gb808Ids.push_back(htonl(0x0101));
					break;
				case 0x10102: //CAN ����ͨ��2 �ɼ�ʱ����
					gb808Ids.push_back(htonl(0x0102));
					break;
				case 0x10103: //CAN ����ͨ��2 �ϴ�ʱ����
					gb808Ids.push_back(htonl(0x0103));
					break;
				case 0x10110: //CAN ����ID �����ɼ�����
					gb808Ids.push_back(htonl(0x0110));
					break;
				case 0x1f008: //���ص�ѹ�������޷�ֵ
					gb808Ids.push_back(htonl(0xf008));
					break;
				case 0x1f009: //���ص�ѹ�������޷�ֵ
					gb808Ids.push_back(htonl(0xf009));
					break;
				case 0x1f012: //��չ����������
					gb808Ids.push_back(htonl(0xf012));
					break;
				case 0x1f103: //����ϵ��
					gb808Ids.push_back(htonl(0xf103));
					break;
				}
			}

			dval.buffer.writeInt8(gb808Ids.size());
			dval.buffer.writeBlock(&gb808Ids[0], gb808Ids.size() * sizeof(uint32_t));
		}
	} else if (type == "1") {
		// ��ѯ�ն�����
		dval.msgid = 0x8107;
	} else if (type == "2") {
		// ��ѯ��ʻԱ�����Ϣ
		dval.msgid = 0x8702;
	}

	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}

//	 * 0x0001:����ͨ��Ӧ��-->0x8103�ն˲�������
//	 *                      0x8105�ն˿���
//	 *                      0x8202��ʱλ�ø��ٿ���
//	 *                      0x8300�ı���Ϣ�·�
//	 *                      0x8301�¼�����
//	 *                      0x8302�����·�
//	 *                      0x8303����Ϣ�㲥�˵�����
//	 *                      0x8304����Ϣ����
//	 *                      0x8400���绰�ز�
//	 *                      0x8401�����õ绰��
//	 *                      0x8600������Բ������
//	 *                      0x8601��ɾ��Բ������
//	 *                      0x8602�����þ�������
//	 *                      0x8603��ɾ����������
//	 *                      0x8604�����ö��������
//	 *                      0x8605��ɾ�����������
//	 *                      0x8606������·��
//	 *                      0x8607��ɾ��·��
//	 *                      0x8701����ʽ��¼�����´�
//	 *                      0x8801������ͷ������������
//	 *                      0x8803: �洢��ý�������ϴ�ָ��
//	 *                      0x8804��¼����ʼ����
//	 *                      0x8900����������͸��
//	 *0x0104����ѯ�ն˲���Ӧ��-->0x8104
//	 *0x0201��λ����Ϣ��ѯӦ��-->0x8201
//	 *0x0500����������Ӧ��          -->0x8500
//	 *0x0700����ʻ��¼�������ϴ�-->0x8700
//	 *0x0802���洢��ý�����ݼ���Ӧ��-->0x8802
//	 *0x0A00: �ն�RSA��Կ-->0x8A00ƽ̨RSA��Կ
void MsgClient::HandleDsetpMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > p_kv_map;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, p_kv_map ) ) {
		OUT_ERROR( NULL,0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	string type = "";
	map< string, string >::iterator p = p_kv_map.find( "TYPE" );
	if ( p == p_kv_map.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}

	// ������������
	_SetData dval;

	string cache_key = "";
	type = ( string ) p->second;
	if ( type == "0" ) {
		DataBuffer buf;
		unsigned char pnum = 0;
		//���ò���
		if ( ! _gb_proto_handler->buildParamSet( & buf, p_kv_map, pnum ) ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Param set empty, line: %s" , line.c_str() );
			return;
		}

		// ������Ҫ����һ�����������Ĵ���
		dval.msgid = 0x8103;
		dval.buffer.writeInt8( pnum );
		dval.buffer.writeBlock( ( void* ) buf.getBuffer(), buf.getLength() );
	} else if ( type == "9" ) {
		dval.msgid = 0x8900;

		p = p_kv_map.find( "91" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient","DSET 0x8900 error not found 91 param: %s",line.c_str() );
			return;
		}

		string cmd = p->second;
		if ( cmd.empty() ) {
			OUT_ERROR( NULL, 0,"MsgClient","DSET 0x8900 cmd param error: %s", line.c_str() );
			return;
		}

		p = p_kv_map.find( "90" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient","DSET 0x8900 not found 90 error: %s", line.c_str() );
			return;
		}

		// ͸������
		string value = p->second;
		dval.buffer.writeInt8( ( unsigned char ) atoi( cmd.c_str() ) );
		// תΪ͸������
		CBase64 base64;
		if ( ! base64.Decode( value.c_str(), value.length() ) ) {
			OUT_ERROR( NULL, 0,"MsgClient","Base64 base decode %s failed" , value.c_str() );
			return;
		}
		// ��������͸������
		dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );
	} else if ( type == "10" ) {
		p = p_kv_map.find( "95" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0, "MsgClient","custom command not found 92 param: %s",line.c_str() );
			return;
		}

		string cmd = p->second;
		if ( cmd.empty() ) {
			OUT_ERROR( NULL, 0, "MsgClient","custom command not found 92 param: %s", line.c_str() );
			return;
		}
		dval.msgid = atoi(cmd.c_str());

		p = p_kv_map.find( "96" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0, "MsgClient","custom command not found 93 param: %s", line.c_str() );
			return;
		}

		// ͸������
		string value = p->second;
		CBase64 base64;
		if ( ! base64.Decode( value.c_str(), value.length() ) ) {
			OUT_ERROR( NULL, 0,"MsgClient","Base64 base decode %s failed" , value.c_str() );
			return;
		}

		seq = "";
		mac_id = "";
		command = "";

		// ��������͸������
		dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );
	} else if ( type == "11" ) // �¼�����
			{
		p = p_kv_map.find( "160" ); // �¼�����
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param, line:%s" , line.c_str() );
			return;
		}
		/**
		 * �������ͣ�ȡֵ��Χ��0|1|2|3|4����
		 0��ɾ���ն����������¼������160ֵΪ0������161����
		 1�������¼�
		 2��׷���¼�
		 3���޸��¼�
		 4��ɾ���ض������¼���161ֵ���������������
		 */

		/**
		 �¼����ø�ʽ��{TYPE:11,161:[�¼�1][�¼�2][�¼�3][�¼�N]}
		 �¼��������ṹ��
		 [
		 1:�¼�ID,
		 2:�¼�����(BASE64(GBK))
		 ]*/

		vector< string > vec;
		// ��������
		int cmd = atoi( p->second.c_str() );
		if ( cmd > 0 ) {
			if ( ! parsemultidata( content, "161:", vec, '[', ']' ) ) {
				OUT_ERROR( NULL, 0,"MsgClient", "Parse data error, line:%s" , line.c_str() );
				return;
			}
		}

		switch ( cmd )
		{
		case 0:
		{
			dval.msgid = 0x8301;
			dval.buffer.writeInt8( 0 );
		}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		{
			dval.msgid = 0x8301;

			EventHeader eheader;
			eheader.type = cmd;

			int count = 0;
			dval.buffer.writeFill( 0, ( int ) sizeof(EventHeader) );

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				// �������
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) < 0 ) {
					continue;
				}

				EventContentBody ebody;
				it = smp.find( "1" );  // 1:�¼�ID,
				if ( it == smp.end() ) {
					continue;
				}
				ebody.eventid = atoi( it->second.c_str() );
				if ( cmd == 4 ) {
					ebody.length = 0x00;
					dval.buffer.writeBlock( ( void* ) & ebody, ( int ) sizeof ( ebody ) );
					//dval.buffer.writeInt8( ebody.eventid ) ;  // ɾ���ض�����ʱֻ��Ҫ����¼�ID
				} else {
					it = smp.find( "2" );  // 2:�¼�����(BASE64(GBK))
					if ( it == smp.end() ) {
						continue;
					}

					CBase64 coder;
					// ���BASE64����ʧ���򲻴���
					if ( ! coder.Decode( it->second.c_str(), it->second.length() ) ) {
						continue;
					}
					ebody.length = coder.GetLength();
					dval.buffer.writeBlock( ( void* ) & ebody, sizeof ( ebody ) );
					dval.buffer.writeBlock( ( void* ) coder.GetBuffer(), ( int ) coder.GetLength() );
				}
				++ count;
			}
			// �������������
			if ( count > 0 ) {
				eheader.total = count;
				// ��������ͷ������
				dval.buffer.fillBlock( ( void* ) & eheader, sizeof ( eheader ), 0 );
			}
		}
			break;
		}
	} else if ( type == "12" )  // ��Ϣ�㲥�˵�����
			{
		p = p_kv_map.find( "165" ); // �¼�����
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param, line:%s" , line.c_str() );
			return;
		}
		/**
		 * �������ͣ�ȡֵ��Χ��0|1|2|3����
		 0����ʾɾ���ն�ȫ���˵���
		 1�����²˵�
		 2��׷�Ӳ˵�
		 3���޸Ĳ˵�
		 */

		/**
		 �˵����ø�ʽ��{TYPE:11,166:[�˵�1][�˵�2][�˵�3][�˵�N]}
		 �˵��������ṹ��
		 [
		 1:�˵�������ID����ͬ����ID�Ĳ˵���ᱻ���ǣ���
		 2:�˵������ƣ� BASE64(GBK����) ��
		 ]*/

		vector< string > vec;
		// ��������
		int cmd = atoi( p->second.c_str() );
		if ( cmd > 0 ) {
			if ( ! parsemultidata( content, "166:", vec, '[', ']' ) ) {
				OUT_ERROR( NULL, 0,"MsgClient", "Parse data error, line:%s" , line.c_str() );
				return;
			}
		}

		switch ( cmd )
		{
		case 0:
		{
			dval.msgid = 0x8303;
			dval.buffer.writeInt8( 0 );
		}
			break;
		case 1:
		case 2:
		case 3:
		{
			dval.msgid = 0x8303;

			DemandHeader eheader;
			eheader.type = cmd;

			int count = 0;
			dval.buffer.writeFill( 0, ( int ) sizeof(DemandHeader) );

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				// �������
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) < 0 ) {
					continue;
				}

				DemandContentBody ebody;
				it = smp.find( "1" );  // 1:�˵�������ID,
				if ( it == smp.end() ) {
					continue;
				}
				ebody.mtype = atoi( it->second.c_str() );

				it = smp.find( "2" );  // 2:�˵�������
				if ( it == smp.end() ) {
					continue;
				}

				CBase64 coder;
				// ���BASE64����ʧ���򲻴���
				if ( ! coder.Decode( it->second.c_str(), it->second.length() ) ) {
					continue;
				}
				ebody.length = htons( coder.GetLength() );
				dval.buffer.writeBlock( ( void* ) & ebody, ( int ) sizeof ( ebody ) );
				dval.buffer.writeBlock( ( void* ) coder.GetBuffer(), coder.GetLength() );

				++ count;
			}
			// �������������
			if ( count > 0 ) {
				eheader.total = count;
				dval.buffer.fillBlock( ( void* ) & eheader, sizeof ( eheader ), 0 );
			}
		}
			break;
		}
	} else if ( type == "13" ) // �绰������
			{
		p = p_kv_map.find( "170" ); // �¼�����
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param, line:%s" , line.c_str() );
			return;
		}
		/**
		 * �������ͣ�ȡֵ��Χ��0|1|2|3����
		 0��ɾ���ն������е���ϵ��
		 1�����µ绰����ɾ���ն�������ϵ�ˣ���׷����Ϣ�е���ϵ�ˣ�
		 2��׷�ӵ绰��
		 3���޸ĵ绰��
		 */

		/**
		 ��ϵ�����ø�ʽ��{TYPE:13,171:[�绰����ϵ��1][�绰����ϵ��2][�绰����ϵ��3][�绰����ϵ��N]}
		 �¼��������ṹ��
		 [
		 1:����������ã�1���룬2������3���������
		 2:�绰����
		 3:��ϵ�ˣ� BASE64(GBK����) ��
		 ]*/

		vector< string > vec;
		// ��������
		int cmd = atoi( p->second.c_str() );
		if ( cmd > 0 ) {
			if ( ! parsemultidata( content, "171:", vec, '[', ']' ) ) {
				OUT_ERROR( NULL, 0,"MsgClient", "Parse data error, line:%s" , line.c_str() );
				return;
			}
		}

		switch ( cmd )
		{
		case 0:
		{
			dval.msgid = 0x8401;
			dval.buffer.writeInt8( 0 );
		}
			break;
		case 1:
		case 2:
		case 3:
		{
			dval.msgid = 0x8401;

			PhoneHeader eheader;
			eheader.type = cmd;

			int count = 0;
			dval.buffer.writeFill( 0, sizeof(PhoneHeader) );

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				// �������
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) < 0 ) {
					continue;
				}

				it = smp.find( "1" );  // 1:�����������
				if ( it == smp.end() ) {
					continue;
				}
				dval.buffer.writeInt8( ( unsigned char ) atoi( it->second.c_str() ) );

				it = smp.find( "2" ); // 2:�绰����
				if ( it == smp.end() ) {
					continue;
				}
				dval.buffer.writeInt8( ( unsigned char ) it->second.length() );
				dval.buffer.writeBlock( ( void * ) it->second.c_str(), it->second.length() );

				it = smp.find( "3" );  // 3:��ϵ�ˣ� BASE64(GBK����) ��
				if ( it == smp.end() ) {
					continue;
				}

				CBase64 coder;
				// ���BASE64����ʧ���򲻴���
				if ( ! coder.Decode( it->second.c_str(), it->second.length() ) ) {
					continue;
				}
				dval.buffer.writeInt8( ( unsigned char ) coder.GetLength() );
				dval.buffer.writeBlock( ( void* ) coder.GetBuffer(), coder.GetLength() );

				++ count;
			}
			// �������������
			if ( count > 0 ) {
				eheader.total = count;
				dval.buffer.fillBlock( ( void* ) & eheader, sizeof ( eheader ), 0 );
			}
		}
			break;
		}
	} else if ( type == "14" ) { // ���õ���Χ��
		p = p_kv_map.find( "150" ) ;
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param, line:%s" , line.c_str() );
			return;
		}

		// ��������
		int cmd = atoi( p->second.c_str() );

		// ��������
		p = p_kv_map.find( "151" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param type error, line:%s" , line.c_str() );
			return;
		}
		int type = atoi( p->second.c_str() );
		if ( type < 1 || type > 3 ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Set type error, type %d, line:%s" , type, line.c_str() );
			return;
		}

		// �������Χ������
		vector< string > vec;
		if ( cmd > 0 ) {
			if ( ! parsemultidata( content, "152:", vec, '[', ']' ) ) {
				OUT_ERROR( NULL, 0,"MsgClient", "Parse data error, line:%s" , line.c_str() );
				return;
			}
		}

		switch ( cmd )
		{
		case 0:  // ɾ��ȫ������Χ��
		{
			if ( type == 1 ) {  // Բ��
				dval.msgid = 0x8601;
			} else if ( type == 2 ) { // ����
				dval.msgid = 0x8603;
			} else { // �����
				dval.msgid = 0x8605;
			}
			dval.buffer.writeInt8( 0 );
		}
			break;
		case 1: // ɾ��ָ���ĵ���Χ��
		{
			vector< int > ids;
			int count = 0;

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) > 0 ) {
					it = smp.find( "1" );
					if ( it == smp.end() ) {
						continue;
					}
					// ��ŵ���Χ����ID
					ids.push_back( atoi( it->second.c_str() ) );
					++ count;
				}
			}

			if ( count > 0 ) {
				if ( type == 1 ) {  // Բ��
					dval.msgid = 0x8601;
				} else if ( type == 2 ) { // ����
					dval.msgid = 0x8603;
				} else { // �����
					dval.msgid = 0x8605;
				}
				dval.buffer.writeInt8( count );  // ��һ���ֽ�Ϊ����

				// ����Ϊ32λ��ID��
				for ( int index = 0 ; index < count ; ++ index ) {
					dval.buffer.writeInt32( ids[index] );
				}
			}
		}
			break;
		case 2:  // ����
		case 3:  // ׷��
		case 4:  // �޸�
		{
			int op = cmd - 2;  // �ⲿЭ���ж�ӦΪ0,1,2

			if ( type == 1 ) {  // Բ��
				dval.msgid = 0x8600;
			} else if ( type == 2 ) { // ����
				dval.msgid = 0x8602;
			} else { // �����
				dval.msgid = 0x8604;
			}

			AreaSetHeader ahead;
			ahead.op = op;
			ahead.total = 0;

			// �����Ϊ�����
			if ( dval.msgid != 0x8604 ) {
				// ����ǰ��ͷ������
				dval.buffer.writeFill( 0, sizeof ( ahead ) );
			}

			int areacount = 0;

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				/**
				 1:����ID,
				 2:�������ԣ�32λ�����Ʊ�ʾ�����£���,
				 3:��ʼʱ�䣬
				 4:����ʱ�䣬
				 5:����ٶȣ�
				 6:���ٳ���ʱ�䣬
				 20:���������ı߽磨x1|y1|x2|y2|x3|y3|...|xn|yn��,
				 21:Բ���������ʽ�����ĵ�γ��|���ĵ㾭��|�뾶m��,
				 22:�����������ʽ�����ϵ�γ��|���ϵ㾭��|���µ�γ��|���µ㾭�ȣ�
				 */
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) <= 0 ) {
					continue;
				}

				it = smp.find( "1" );  // 1:����ID,
				if ( it == smp.end() ) {
					continue;
				}

				int areaid = atoi( it->second.c_str() );
				DataBuffer hbuf;  // ��¼�����BUF

				AreaHeader bhead;
				// ��ŵ���Χ����ID
				bhead.areaid = htonl( areaid );
				it = smp.find( "2" );  // 2:�������ԣ�32λ�����Ʊ�ʾ�����£���,
				if ( it == smp.end() ) {
					continue;
				}
				int attrid = atoi( it->second.c_str() );
				bhead.attr = htons( attrid );

				// ���һ������ͷ����
				hbuf.writeBlock( ( void* ) & bhead, sizeof ( bhead ) );

				DataBuffer tbuf;  // ��¼�������BUF
				// �Ƿ����ʱ��
				if ( attrid & 0x0001 ) {
					AreaTime atime;
					it = smp.find( "3" ); // 3:��ʼʱ�䣬
					if ( it != smp.end() ) {
						// ��UTCתΪBCDʱ��
						convert2bcd( it->second, atime.starttime );
					}
					it = smp.find( "4" ); // 4:����ʱ�䣬
					if ( it != smp.end() ) {
						// ��UTCתΪBCDʱ��
						convert2bcd( it->second, atime.endtime );
					}
					// ���ʱ��
					tbuf.writeBlock( ( void* ) & atime, sizeof ( atime ) );
				}
				// �Ƿ�����ٶ�
				if ( attrid & 0x0002 ) {
					AreaSpeed aspeed;
					it = smp.find( "5" ); // 5:����ٶȣ�
					if ( it != smp.end() ) {
						aspeed.speed = htons( atoi( it->second.c_str() ) ); //htonl
					}
					it = smp.find( "6" ); // 6:���ٳ���ʱ�䣬
					if ( it != smp.end() ) {
						aspeed.nlast = atoi( it->second.c_str() );
					}
					tbuf.writeBlock( ( void* ) & aspeed, sizeof ( aspeed ) );
				}

				switch ( type )
				{
				case 1:  // Բ��
				{
					BoundAreaBody abody;
					it = smp.find( "21" );  //Բ���������ʽ�����ĵ�γ��|���ĵ㾭��|�뾶m��
					if ( it == smp.end() ) {
						continue;
					}

					vector< string > vpoint;
					splitvector( it->second, vpoint, "|", 0 );
					if ( vpoint.size() < 3 ) {
						// �����������ȷ��˵����ʽ����ȷ
						continue;
					}
					abody.local.lat = htonl( atoi( vpoint[0].c_str() ) );
					abody.local.lon = htonl( atoi( vpoint[1].c_str() ) );
					abody.raduis = htonl( atoi( vpoint[2].c_str() ) );

					hbuf.writeBlock( ( void* ) & abody, sizeof ( abody ) );

					// ����и�����
					if ( tbuf.getLength() > 0 ) {
						// ��Ӹ�����
						hbuf.writeBuffer( tbuf );
					}
				}
					break;
				case 2:  // ����
				{
					RangleAreaBody abody;
					it = smp.find( "22" );  // 22:�����������ʽ�����ϵ�γ��|���ϵ㾭��|���µ�γ��|���µ㾭�ȣ�
					if ( it == smp.end() ) {
						continue;
					}
					vector< string > vpoint;
					splitvector( it->second, vpoint, "|", 0 );
					if ( vpoint.size() < 4 ) {
						continue;
					}
					abody.left_top.lat = htonl( atoi( vpoint[0].c_str() ) );
					abody.left_top.lon = htonl( atoi( vpoint[1].c_str() ) );
					abody.right_bottom.lat = htonl( atoi( vpoint[2].c_str() ) );
					abody.right_bottom.lon = htonl( atoi( vpoint[3].c_str() ) );

					hbuf.writeBlock( ( void* ) & abody, sizeof ( abody ) );

					// ������ڸ���������Ӹ�����
					if ( tbuf.getLength() > 0 ) {
						hbuf.writeBuffer( tbuf );
					}
				}
					break;
				case 3:  // �����
				{
					it = smp.find( "20" );  // 20:���������ı߽磨x1|y1|x2|y2|x3|y3|...|xn|yn��,
					if ( it == smp.end() ) {
						continue;
					}

					vector< string > vpoint;
					splitvector( it->second, vpoint, "|", 0 );
					if ( vpoint.size() % 2 != 0 ) {
						continue;
					}

					DataBuffer vbuf;

					int vcount = 0;
					for ( size_t vpos = 0 ; vpos < vpoint.size() ; vpos = vpos + 2 ) {
						LatLonPoint pt;
						pt.lat = htonl( atoi( vpoint[vpos].c_str() ) );
						pt.lon = htonl( atoi( vpoint[vpos + 1].c_str() ) );

						// ����������ֵ
						vbuf.writeBlock( ( void* ) & pt, sizeof ( pt ) );

						++ vcount;
					}
					// ������ڸ����������
					if ( tbuf.getLength() > 0 ) {
						hbuf.writeBuffer( tbuf );
					}

					// ��¼�������
					hbuf.writeInt16( vcount );

					// ��������ֵ
					hbuf.writeBuffer( vbuf );
				}
					break;
				}

				// ����һ��Χ������
				dval.buffer.writeBuffer( hbuf );

				++ areacount;
			}

			// �����Ϊ����δ���
			if ( dval.msgid != 0x8604 ) {
				ahead.total = areacount;
				dval.buffer.fillBlock( & ahead, sizeof ( ahead ), 0 );
			}
		}
			break;
		default:
			OUT_ERROR( NULL, 0,"MsgClient", "Command operator error, line:%s" , line.c_str() );
			break;
		}
	} else if ( type == "15" ) {  // ����·��

		p = p_kv_map.find( "155" );
		if ( p == p_kv_map.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command param, line:%s" , line.c_str() );
			return;
		}

		// ��������
		int cmd = atoi( p->second.c_str() );

		// ������·����
		vector< string > vec;
		if ( cmd > 0 ) {
			// �����߳�����
			if ( ! parsemultidata( content, "156:", vec, '[', ']' ) ) {
				OUT_ERROR( NULL, 0,"MsgClient", "Parse data error, line:%s" , line.c_str() );
				return;
			}
		}

		switch ( cmd )
		{
		case 0:  // ɾ��ȫ����·
		{
			dval.msgid = 0x8607;
			dval.buffer.writeInt8( 0 );
		}
			break;
		case 1: // ɾ��ָ������·
		{
			vector< int > ids;
			int count = 0;

			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) > 0 ) {
					it = smp.find( "1" );
					if ( it == smp.end() ) {
						continue;
					}
					// ��ŵ���Χ����ID
					ids.push_back( atoi( it->second.c_str() ) );
					++ count;
				}
			}

			if ( count > 0 ) {
				dval.msgid = 0x8607;
				dval.buffer.writeInt8( count ); // ��һ���ֽ�Ϊ����

				// ����Ϊ32λ��ID��
				for ( int index = 0 ; index < count ; ++ index ) {
					dval.buffer.writeInt32( ids[index] );
				}
			}
		}
			break;
		default: // ������·�Ĵ���
		{
			map< string, string > smp;
			map< string, string >::iterator it;
			for ( size_t i = 0 ; i < vec.size() ; ++ i ) {
				if ( vec[i].empty() ) {
					continue;
				}
				/**
				 1:·��ID��
				 2:·�����ԣ�
				 3:��ʼʱ�䣬
				 4:����ʱ�䣬
				 5:·�������ṹ��
				 ��1=�յ�ID|2=·��ID|3=�յ�γ��|4=�յ㾫��|5=·�ο��|6=·������|7=·����ʻ������ֵ|8=·����ʻ���̷�ֵ|
				 9=·������ٶ�|10=·�γ��ٳ���ʱ�䣩��·��2����·��3����·��4��
				 */
				vector< string > temp;
				splitvector( vec[i], temp, ",", 0 );
				if ( split2map( temp, smp, ":" ) < 0 ) {
					continue;
				}

				it = smp.find( "1" );  // 1:·��ID��
				if ( it == smp.end() ) {
					continue;
				}

				_SetData dvalue;
				RoadHeader rheader;
				rheader.roadid = htonl( atoi( it->second.c_str() ) );

				it = smp.find( "2" );  // 2:·�����ԣ�
				if ( it == smp.end() ) {
					continue;
				}
				int attrid = atoi( it->second.c_str() );
				rheader.attr = htons( attrid );
				dvalue.buffer.writeBlock( ( void* ) & rheader, sizeof ( rheader ) );

				if ( attrid & 0x0001 ) {
					AreaTime rtime;
					it = smp.find( "3" );  // 3:��ʼʱ�䣬
					if ( it != smp.end() ) {
						// ��UTCתΪBCDʱ��
						convert2bcd( it->second, rtime.starttime );
					}
					it = smp.find( "4" );  // 4:����ʱ�䣬
					if ( it != smp.end() ) {
						// ��UTCתΪBCDʱ��
						convert2bcd( it->second, rtime.endtime );
					}
					dvalue.buffer.writeBlock( ( void* ) & rtime, sizeof ( rtime ) );
				}

				/**
				 5:·�������ṹ��
				 ��1=�յ�ID|2=·��ID|3=�յ�γ��|4=�յ㾫��|5=·�ο��|6=·������|7=·����ʻ������ֵ|8=·����ʻ���̷�ֵ|
				 9=·������ٶ�|10=·�γ��ٳ���ʱ�䣩��·��2����·��3����·��4��
				 */
				unsigned short rcount = 0;
				vector< string > vecbend;
				if ( ! parsemultidata( vec[i], "5:", vecbend, '(', ')' ) ) {
					// �����յ�����
					continue;
				}

				DataBuffer tbuf;
				map< string, string > bmap;
				for ( size_t index = 0 ; index < vecbend.size() ; ++ index ) {
					// ��������
					vector< string > vecpt;
					splitvector( vecbend[index], vecpt, "|", 0 );
					// ������Ӧֵ
					if ( split2map( vecpt, bmap, "=" ) <= 0 ) {
						continue;
					}

					BendPoint rbendpt;
					it = bmap.find( "1" );  // �յ�ID
					if ( it == bmap.end() ) {
						continue;
					}
					rbendpt.bendid = htonl( atoi( it->second.c_str() ) );

					it = bmap.find( "2" );
					if ( it == bmap.end() ) {  // ·��ID
						continue;
					}
					rbendpt.segid = htonl( atoi( it->second.c_str() ) );

					it = bmap.find( "3" );
					if ( it == bmap.end() ) {  // �յ�γ��
						continue;
					}
					rbendpt.postion.lat = htonl( atoi( it->second.c_str() ) );

					it = bmap.find( "4" );
					if ( it == bmap.end() ) {  // �յ㾭��
						continue;
					}
					rbendpt.postion.lon = htonl( atoi( it->second.c_str() ) );

					it = bmap.find( "5" );
					if ( it == bmap.end() ) {  // ·�ο��
						continue;
					}
					rbendpt.width = atoi( it->second.c_str() );

					it = bmap.find( "6" );
					if ( it == bmap.end() ) {  // ·������
						continue;
					}
					int rbendattr = atoi( it->second.c_str() );
					rbendpt.battr = rbendattr;

					tbuf.writeBlock( ( void* ) & rbendpt, sizeof ( rbendpt ) );

					// �������ֵ��������
					if ( rbendattr & 0x0001 ) {
						Threshold thold;
						it = bmap.find( "7" );  // 7=·����ʻ������ֵ
						if ( it != bmap.end() ) {
							thold.more = htons( atoi( it->second.c_str() ) );
						}
						it = bmap.find( "8" );  // 8=·����ʻ���̷�ֵ
						if ( it != bmap.end() ) {
							thold.less = htons( atoi( it->second.c_str() ) );
						}
						tbuf.writeBlock( ( void* ) & thold, sizeof ( thold ) );
					}

					//  ����ٶȴ�������
					if ( rbendattr & 0x0002 ) {
						AreaSpeed rspeed;
						it = bmap.find( "9" );  // 9=·������ٶ�
						if ( it != bmap.end() ) {
							rspeed.speed = htons( atoi( it->second.c_str() ) );
						}
						it = bmap.find( "10" ); // 10=·�γ��ٳ���ʱ��
						if ( it != bmap.end() ) {
							rspeed.nlast = atoi( it->second.c_str() );
						}
						tbuf.writeBlock( ( void* ) & rspeed, sizeof ( rspeed ) );
					}

					++ rcount;
				}

				dvalue.msgid = 0x8606;
				dvalue.buffer.writeInt16( rcount );  // д��¼����
				dvalue.buffer.writeBuffer( tbuf );   // ��¼����

				// ��������������ܴ��ڶ�����ݵ����
				SendMsgData( dvalue, car_id, mac_id, command, seq );
			}
		}
			break;
		}

	} else if ( type == "30" ) {
		p = p_kv_map.find( "311" ); //��ʷ��������
		if ( p != p_kv_map.end() ) {
			dval.buffer.writeInt8(atoi(p->second.c_str()));
		} else {
			dval.buffer.writeInt8(0);
		}

		char bcdtime[6];

		memset(bcdtime, 0x00, 6);
		p = p_kv_map.find("312"); //��ѯ��ʼʱ��
		if (p != p_kv_map.end() && p->second.length() == 12) {
			strtoBCD(p->second.c_str(), bcdtime, 6);
		}
		dval.buffer.writeBlock(bcdtime, 6);

		memset(bcdtime, 0x00, 6);
		p = p_kv_map.find("313"); //��ѯ����ʱ��
		if (p != p_kv_map.end() && p->second.length() == 12) {
			strtoBCD(p->second.c_str(), bcdtime, 6);
		}
		dval.buffer.writeBlock(bcdtime, 6);

		p = p_kv_map.find("314"); //��ѯ����
		if(p != p_kv_map.end()) {
			dval.buffer.writeInt16(atoi(p->second.c_str()));
		} else {
			dval.buffer.writeInt16(0);
		}

		dval.msgid = 0x8f12;
	}

	// һ�δ�����������
	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}
/**����***/
void MsgClient::HandleDcallMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > kvmap;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, kvmap ) ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	map< string, string >::iterator it = kvmap.find( "TYPE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}
	string type = it->second;

	string cache_key = "";
	if ( car_id.length() > 12 ) {
		OUT_ERROR( NULL, 0,"MsgClient", car_id.c_str(), "Param error, data: %s" , line.c_str() );
		//ָ�����
		return;
	}

	// ��������
	_SetData dval;

	if ( type == "0" ) {   //��ʱ����
		it = kvmap.find( "0" );
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", car_id.c_str(), "Param error , data: %s" , line.c_str() );
			return;
		}
		string intervalstr = it->second;

		it = kvmap.find( "1" );
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", car_id.c_str(), "Param error ,, data: %s" , line.c_str() );
			return;
		}
		string timesstr = it->second;

		unsigned short intervalus = atoi( intervalstr.c_str() );
		unsigned short timesus = atoi( timesstr.c_str() );

		dval.msgid = 0x8202; // ��ʱλ�ø��ٿ���

		PlatformTraceBody body;
		body.period = htonl( intervalus * timesus );
		body.timeval = htons( intervalus );

		// ��������
		dval.buffer.writeBlock( ( void* ) & body, sizeof ( body ) );
	} else if ( type == "1" ) { //����
		it = kvmap.find( "2" );
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", car_id.c_str(), "Param error , data: %s" , line.c_str() );
			return;
		}

		string distancestr = it->second;
		if ( distancestr == "0" ) {
			dval.msgid = 0x8202; // ֹͣ����
			dval.buffer.writeInt16( 0 );
		}
	} else if ( type == "2" ) {   //λ����Ϣ��ѯ
		dval.msgid = 0x8201;
	} else {   //��������
//        	char a[10];
//        	AddUpData(a,10,1);
	}
	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}

void MsgClient::HandleDctlmMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > kvmap;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, kvmap ) ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	map< string, string >::iterator it;

	it = kvmap.find( "TYPE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}
	string type = it->second;

	it = kvmap.find( "VALUE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}
	string value = it->second;

	_SetData dval;

	//��ʽ��ȷ
	unsigned short ustype = atoi( type.c_str() );
	switch ( ustype ) {
	case 4:  // �ն˿��ƹػ�
	case 15: // ֵ: 3�ն˹ػ���4�ն˸�λ��5�ն˻ָ��������ã�6�ر�����ͨ�ţ�7�ر���������ͨ��
		dval.msgid = 0x8105;  // �ն˿���
		dval.buffer.writeInt8((value.empty()) ? 3 : atoi(value.c_str())); // �ն˹ػ�������
		break;
	case 7:  // ����
	case 8:  // ����
		dval.msgid = 0x8500;
		dval.buffer.writeInt8((ustype == 7) ? 0x01 : 0x00); // �����Ϳ����Ŀ���
		break;
	case 9: //����,valueΪ�绰����;
	case 16: //ͨ��
		dval.msgid = 0x8400;  // �绰�ز�
		if (ustype == 9)  //����
			dval.buffer.writeInt8(1);
		else
			//ͨ��
			dval.buffer.writeInt8(0);
		dval.buffer.writeBlock(value.c_str(), value.length());
		break;
	case 10: //_TakePhoto
		{
			dval.msgid = 0x8801;  // ���մ���

			TakePhotoBody photo;

			vector< string > vec_p;
			splitvector( value, vec_p, "|", 0 );
			photo.camera_id = vec_p.size() > 0 && vec_p[0].size() > 0 ? atoi( vec_p[0].c_str() ) : 1;
			photo.photo_num = vec_p.size() > 1 && vec_p[1].size() > 0 ? htons( atoi( vec_p[1].c_str() ) ) : htons( 1 );
			photo.time_interval = vec_p.size() > 2 && vec_p[2].size() > 0 ? htons( atoi( vec_p[2].c_str() ) ) : 0;
			photo.is_save = vec_p.size() > 3 && vec_p[3].size() > 0 ? atoi( vec_p[3].c_str() ) : 0;
			photo.sense = vec_p.size() > 4 && vec_p[4].size() > 0 ? (atoi( vec_p[4].c_str() ) + 1) : 2;
			photo.photo_quality = vec_p.size() > 5 && vec_p[5].size() > 0 ? atoi( vec_p[5].c_str() ) : 5;
			photo.liangdu = vec_p.size() > 6 && vec_p[6].size() > 0 ? atoi( vec_p[6].c_str() ) : 128;
			photo.duibidu = vec_p.size() > 7 && vec_p[7].size() > 0 ? atoi( vec_p[7].c_str() ) : 64;
			photo.baohedu = vec_p.size() > 8 && vec_p[8].size() > 0 ? atoi( vec_p[8].c_str() ) : 64;
			photo.sedu = vec_p.size() > 9 && vec_p[9].size() > 0 ? atoi( vec_p[9].c_str() ) : 128;

			dval.buffer.writeBlock( & photo, sizeof ( photo ) );

			// ToDo: ������ͨ��չ�����·������������
			it = kvmap.find( "191" ) ;
			if ( it != kvmap.end() ) {
				dval.buffer.writeInt32( atoi( it->second.c_str() ) ) ;
			}
		}
		break;
	case 11:  //¼����ʼ����
		{
			dval.msgid = 0x8804;   // ¼����ʼ

			VoiceRecordBody body;

			vector< string > vec;
			splitvector( value, vec, "|", 0 );
			body.command = atoi( vec[0].c_str() );
			body.recordtime = ( vec.size() > 1 ) ? htons( atoi( vec[1].c_str() ) ) : htons( 0 );
			body.saveflag = ( vec.size() > 2 ) ? atoi( vec[2].c_str() ) : 0;
			body.samplerates = ( vec.size() > 3 ) ? atoi( vec[3].c_str() ) : 0;

			dval.buffer.writeBlock( & body, sizeof ( body ) );
		}
		break;
	case 20:  // ��������
		{
			// ����URL��ַ��APN���ƣ�APN�û�����APN���룻��������ַ��������TCP�˿ڣ�������UDP�˿ڣ�������ID��Ӳ���汾���̼��汾�����ӵ�ָ��������ʱ��
			dval.msgid = 0x8105;  // ������������
			dval.buffer.writeInt8( 1 );

			vector< string > vec;
			splitvector( value, vec, ";", 0 );
			if ( vec.size() < 11 ) {
				OUT_ERROR( NULL, 0, "MsgClient","CTML split value %s failed" , value.c_str() );
				return;
			}

			// ������������
			for ( int i = 0 ; i < ( int ) vec.size() ; ++ i ) {
				string &temp = vec[i];
				if ( i == 5 || i == 6 || i == 10 ) { // TCP�˿ں�UDP�˿�, ���ӵ�ʱ�� WORD
					int port = 0;
					if (!temp.empty()) {
						port = atoi(temp.c_str());
					}
					dval.buffer.writeInt16(port);
				} else if ( i == 7 ) {  // ������BYTE[5]
					if ( ! temp.empty() ) {
						dval.buffer.writeBytes( ( void * ) temp.c_str(), temp.length(), 5 );
					} else {
						dval.buffer.writeFill( 0, 5 );
					}
				} else {  // �ַ�������
					if ( ! temp.empty() ) {
						dval.buffer.writeBlock( temp.c_str(), temp.length() );
					}
				}
				if ( i != 10 ) {
					dval.buffer.writeInt8( ';' );
				}
			}
		}
		break;
	case 21:  // ���ӿ���
		{
			// ���ӿ���;���ƽ̨��Ȩ��;���ŵ�����;�����û���;��������;��ַ;TCP�˿�;UDP�˿�;���ӵ�������ָ����ʱ��
			dval.msgid = 0x8105; // ���ӿ���Ӧ������
			dval.buffer.writeInt8( 2 );

			vector< string > vec;
			splitvector( value, vec, ";", 0 );
			if ( vec.size() < 9 ) {
				OUT_ERROR( NULL, 0, "MsgClient", "CTML split value %s failed", value.c_str() );
				return;
			}

			for ( int i = 0 ; i < ( int ) vec.size() ; ++ i ) {
				string &temp = vec[i];
				if ( i == 0 ) {
					int flag = atoi( temp.c_str() );
					dval.buffer.writeInt8( flag );
					if ( flag == 1 ) {
						break;
					}
				} else if ( i >= 6 ) {  // ���ݴ���
					unsigned short word = 0;
					if ( ! temp.empty() ) {
						word = atoi( temp.c_str() );
					}
					dval.buffer.writeInt16( word );
				} else {
					dval.buffer.writeBlock( temp.c_str(), temp.length() );
				}
				if ( i != 8 ) {
					dval.buffer.writeInt8( ';' );
				}
			}
		}
		break;
	case 24: // ����ȷ��Ӧ��
	{
		vector<string> vec;
		splitvector(value, vec, "|", 0);

		if(vec.size() > 0 && atoi(vec[0].c_str()) == -1) {
			dval.msgid = 0x8001; // ��ӦͨӦӦ��
			dval.buffer.writeInt16(0);  // Ӧ�����
			dval.buffer.writeInt16(0x0200); // λ���ϱ�ͨ��Ӧ��
			dval.buffer.writeInt8(4);  // ����ȷ��

			seq = "";
			mac_id = "";
			command = "";
		} else if(vec.size() == 2) {
			dval.msgid = 0x8203; // �˹�ȷ�ϱ�����Ϣ
			dval.buffer.writeInt16(atoi(vec[1].c_str())); // ������Ϣ��ˮ��
			dval.buffer.writeInt32(atoi(vec[0].c_str())); // �˹�ȷ�ϱ�������
		}
	}
		break;
	case 25: //�·��ն�������
	{
		vector<string> fields;

		if(Utils::splitStr(value, fields, '|') != 4) {
			OUT_WARNING(NULL, 0, NULL, "inner msg error: %s", line.c_str());
			break;
		}

		CBase64 base64;
		if( ! base64.Decode(fields[3].c_str(), fields[3].length())) {
			OUT_WARNING(NULL, 0, NULL, "inner msg error: %s", line.c_str());
			break;
		}

		HttpQuery hQuery;
		if( ! hQuery.get(string(base64.GetBuffer(), base64.GetLength()))) {
			OUT_WARNING(NULL, 0, NULL, "inner msg error: %s", line.c_str());
			break;
		}

		dval.msgid = 0x8108;
		dval.buffer.writeInt8(atoi(fields[0].c_str()));
		dval.buffer.writeBlock((fields[1] + string(5, '\0')).c_str(), 5);
		dval.buffer.writeInt8(fields[2].length());
		dval.buffer.writeBlock(fields[2].c_str(), fields[2].length());
		dval.buffer.writeInt32(hQuery.size());
		dval.buffer.writeBlock(hQuery.data(), hQuery.size());
	}
		break;
	case 26:
	{
		vector<string> fields;

		if (Utils::splitStr(value, fields, '|') != 5) {
			OUT_WARNING(NULL, 0, NULL, "inner msg error: %s", line.c_str());
			break;
		}

		if((fields[3] != "-1" && fields[3].length() != 14) ||
				(fields[4] != "-1" && fields[4].length() != 14)) {
			OUT_WARNING(NULL, 0, NULL, "inner msg error: %s", line.c_str());
			break;
		}

		char bcdtime[6];

		dval.msgid = 0x8f16;
		dval.buffer.writeInt16(1);
		dval.buffer.writeInt8(atoi(fields[0].c_str()));

		if(fields[1] == "-1") {
			dval.buffer.writeInt8(0xff);
		} else {
			dval.buffer.writeInt8(atoi(fields[1].c_str()));
		}

		if(fields[2] == "-1") {
			dval.buffer.writeInt16(0xffff);
		} else {
			dval.buffer.writeInt16(atoi(fields[2].c_str()));
		}

		if(fields[3] == "-1") {
			memset(bcdtime, 0xff, 6);
		} else {
			strtoBCD(fields[3].substr(2).c_str(), bcdtime, 6);
		}
		dval.buffer.writeBlock(bcdtime, 6);

		if(fields[4] == "-1") {
			memset(bcdtime, 0xff, 6);
		} else {
			strtoBCD(fields[4].substr(2).c_str(), bcdtime, 6);
		}
		dval.buffer.writeBlock(bcdtime, 6);
	}
		break;
	case 27:
		dval.msgid = 0x8f16;
		dval.buffer.writeInt16(2);
		break;
	default:
		break;
	}
	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}

void MsgClient::HandleDsndmMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > kvmap;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, kvmap ) ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	string type = "";
	map< string, string >::iterator it = kvmap.find( "TYPE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}
	type = it->second;

	_SetData dval;

	if ( type == "1" ) {  // �ı��·�

		it = kvmap.find( "1" );  // �ı��·����λ
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Command tag type error,line :%s" , line.c_str() );
			return;
		}
		unsigned char cflag = atoi( it->second.c_str() );

		it = kvmap.find( "2" );  //  BASE64�ı�����
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Send text empty, line:%s" ,line.c_str() );
			return;
		}

		CBase64 base64;
		if ( ! base64.Decode( it->second.c_str(), it->second.length() ) ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Base64 decode failed, %s", line.c_str() );
			return;
		}

		dval.msgid = 0x8300;
		// �����һ��λ
		dval.buffer.writeInt8( cflag );
		dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );

	} else if ( type == "4" )  // ��Ϣ����
			{
		it = kvmap.find( "11" ); // ��Ϣ���������ID������Ϣ��������ʱ����ID���Ӧ
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Message server id error, line: %s" , line.c_str() );
			return;
		}

		MsgServiceBody body;
		body.type = atoi( it->second.c_str() );

		it = kvmap.find( "12" ); // ��Ϣ��������ݣ�BASE64���루��Ϣ���ݲ���GBK���룩
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient","Message content error, line:%s" , line.c_str() );
			return;
		}
		CBase64 base64;
		if ( ! base64.Decode( it->second.c_str(), it->second.length() ) ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Base64 decode failed, %s", line.c_str() );
			return;
		}

		dval.msgid = 0x8304; // ��Ϣ����
		// �����һ��λ
		body.len = htons( base64.GetLength() );
		dval.buffer.writeBlock( & body, sizeof ( body ) );
		dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );
	} else if ( type == "5" )  // �����·�
			{
		it = kvmap.find( "16" );  // �ʴ���������
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Message ask attrib error, line: %s" , line.c_str() );
			return;
		}

		QuestAskBody body;
		body.flag = atoi( it->second.c_str() );

		it = kvmap.find( "17" ); // ��������ݣ�BASE64���루�������ݲ���GBK���룩
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient","Message content empty, line:%s" , line.c_str() );
			return;
		}

		CBase64 base64;
		if ( ! base64.Decode( it->second.c_str(), it->second.length() ) ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Base64 decode failed, %s", line.c_str() );
			return;
		}
		body.len = base64.GetLength();

		dval.msgid = 0x8302; // �����·�
		dval.buffer.writeBlock( & body, sizeof ( body ) );
		dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );

		vector< string > vecanswer;
		if ( parsemultidata( content, "18:", vecanswer, '[', ']' ) ) {
			map< string, string > amap;
			// �������������
			for ( size_t i = 0 ; i < vecanswer.size() ; ++ i ) {
				vector< string > vkey;
				splitvector( vecanswer[i], vkey, ",", 0 );
				if ( vkey.empty() ) {
					continue;
				}

				// ��������ֵ
				if ( split2map( vkey, amap, ":" ) <= 0 ) {
					continue;
				}

				it = amap.find( "1" );  // ��ID
				if ( it == amap.end() ) {
					continue;
				}
				QuestAnswerBody abody;
				abody.aflag = atoi( it->second.c_str() );

				it = amap.find( "2" ); // ������
				if ( it == amap.end() ) {
					continue;
				}
				CBase64 coder;
				if ( ! coder.Decode( it->second.c_str(), it->second.length() ) ) {
					continue;
				}
				abody.alen = htons( coder.GetLength() );

				// ����ͷ��
				dval.buffer.writeBlock( & abody, sizeof ( abody ) );
				// ��������
				dval.buffer.writeBlock( coder.GetBuffer(), coder.GetLength() );
			}
		}
	}

	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}

void MsgClient::HandleReqdMsg( string &line )
{
	string seq, mac_id, command, content, car_id;
	map< string, string > kvmap;
	if ( ! splitmsgheader( line, seq, mac_id, car_id, command, content, kvmap ) ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Split command error, line:%s" , line.c_str() );
		return;
	}

	string type = "";
	map< string, string >::iterator it = kvmap.find( "TYPE" );
	if ( it == kvmap.end() ) {
		OUT_ERROR( NULL, 0,"MsgClient", "Command error, line:%s" , line.c_str() );
		return;
	}

	type = it->second;

	_SetData dval;
	if ( type == "1" || type == "2" ) {
		/**
		 ��������ָ��:
		 1:��ý�����ͣ�
		 2:ͨ��ID��
		 3:�¼�����룬
		 4:��ʼʱ�䣬
		 5:����ʱ�䣬
		 6:ɾ����־(0��������1��ɾ��)
		 */

		MediaDataBody body;
		it = kvmap.find( "1" );  // 1:��ý�����ͣ�
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Media type error,line %s" , line.c_str() );
			return;
		}
		body.type = atoi( it->second.c_str() );

		it = kvmap.find( "2" );
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Media channel error,line %s" , line.c_str() );
			return;
		}
		body.channel = atoi( it->second.c_str() );

		it = kvmap.find( "3" ); // 3:�¼�����룬
		if ( it != kvmap.end() ) {
			body.event = atoi( it->second.c_str() );
		}

		it = kvmap.find( "4" );  // ��ʼʱ��
		if ( it != kvmap.end() ) {
			// ��UTCתΪBCDʱ��
			convert2bcd( it->second, body.starttime );
		}

		it = kvmap.find( "5" );  // ����ʱ��
		if ( it != kvmap.end() ) {
			// ��UTCתΪBCDʱ��
			convert2bcd( it->second, body.endtime );
		}

		if ( type == "1" ) {  // ��ý���ݼ���
			dval.msgid = 0x8802;
			dval.buffer.writeBlock( & body, sizeof ( body ) );
		} else if ( type == "2" ) { //  ��ý�������ϴ�

			dval.msgid = 0x8803;
			dval.buffer.writeBlock( & body, sizeof ( body ) );

			unsigned char flag = 0x00;
			it = kvmap.find( "6" );
			if ( it != kvmap.end() ) {
				flag = atoi( it->second.c_str() );
			}
			dval.buffer.writeInt8( flag );
		}
	} else if ( type == "3" ) {  // �����洢��ý�����ݼ����ϴ�����
		dval.msgid = 0x8805;

		unsigned int mid = 0;
		it = kvmap.find( "7" );  // ��ý��ID��DWORD����ҳ����
		if ( it != kvmap.end() ) {
			mid = atoi( it->second.c_str() );
		}

		unsigned char flag = 0x00;
		it = kvmap.find( "6" );  // ɾ�����߱���
		if ( it != kvmap.end() ) {
			flag = atoi( it->second.c_str() );
		}
		dval.buffer.writeInt32( mid );  // ��ý��ID
		dval.buffer.writeInt8( flag );	 // 0������1ɾ��
	} else if ( type == "4" ) { //�ɼ�������ʻ��¼
		it = kvmap.find( "30" ); //�ɼ�����
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,"MsgClient", "Get travel record data cmd error" );
			return;
		}
		string scmd = it->second;

		dval.msgid = 0x8700;  // �ɼ���ʻ��¼������
		dval.buffer.writeInt8( atoi( scmd.c_str() ) );

		it = kvmap.find( "31" ); // ��ʻ��¼������
		if ( it != kvmap.end() ) {
			string sdata = it->second;
			CBase64 base64;  // �´������ݿ�
			if ( ! base64.Decode( sdata.c_str(), sdata.length() ) ) {
				OUT_ERROR( NULL, 0, "MsgClient", "Base64 decode %s failed", sdata.c_str() );
				return;
			}
			dval.buffer.writeBlock( base64.GetBuffer(), base64.GetLength() );
		}
	} else if ( type == "5" ) { //�ϱ�������ʻ��¼
		it = kvmap.find( "30" ); //�ɼ�����
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,NULL, "Get travel record data cmd error" );
			return;
		}
		string scmd = it->second;

		it = kvmap.find( "31" ); // ��ʻ��¼������
		if ( it == kvmap.end() ) {
			OUT_ERROR( NULL, 0,NULL, "Get travel record data block error" );
			return;
		}
		string sdata = it->second;

		dval.msgid = 0x8701;  // ��ʻ��¼���´�����
		dval.buffer.writeInt8( atoi( scmd.c_str() ) ); // ���������ֲ���

		CBase64 base64;  // �´������ݿ�
		if (!base64.Decode(sdata.c_str(), sdata.length())) {
			OUT_ERROR(NULL, 0, NULL, "Base64 decode %s failed",sdata.c_str());
			return;
		}
		dval.buffer.writeBlock(base64.GetBuffer(), base64.GetLength());
	} else {
		OUT_ERROR( NULL, 0,"MsgClient", "Msg type error type %s, line %s" , type.c_str(), line.c_str() );
		return;
	}

	if ( dval.msgid > 0 ) {
		// �������ݴ���
		SendMsgData( dval, car_id, mac_id, command, seq );
	}
}

// �Ӻ󿽱�����������ַ�
static void reverse_copy( char *buf, int len, const char *src, const char fix )
{
	int nlen = ( int ) strlen( src );
	int offset = len - nlen;
	if ( offset < 0 ) {
		offset = 0;
	}
	if ( offset > 0 ) {
		for ( int i = 0 ; i < offset ; ++ i ) {
			buf[i] = fix;
		}
	}
	memcpy( buf + offset, src, nlen );
}

// ���ʹ�������
void MsgClient::SendMsgData( _SetData &val, const string &car_id, const string &mac_id, const string &command,
		const string &seq )
{
	GBheader header;
	GBFooter footer;

	char key[128] = { 0 };
	reverse_copy( key, 12, car_id.c_str(), '0' );
	strtoBCD( key, header.car_id );

	header.msgid = htons( val.msgid );

	// ȡ����Ҫ���͵�������
	char *ptr = val.buffer.getBuffer();
	int len = val.buffer.getLength();
	// ����ְ���
	int count = ( len / MAX_SPLITPACK_LEN ) + ( ( len % MAX_SPLITPACK_LEN == 0 && len > 0 ) ? 0 : 1 );

	int offset = 0, left = len, readlen = 0;

	// ���ݷְ��������Ҫ��������
	unsigned short seqid = _pEnv->GetSequeue( car_id.c_str(), count ) - count;

	// ����ְ���������
	for ( int i = 0 ; i < count ; ++ i ) {
		// �����ܴ�����
		readlen = ( left > MAX_SPLITPACK_LEN ) ? MAX_SPLITPACK_LEN : left;

		// ����Ϊ��Ϣ���ݵĳ���ȥ��ͷ��β
		unsigned short mlen = 0;
		if ( count > 1 ) { // ���Ϊ������ݣ���Ҫ���÷ְ�λ��־
			mlen = htons( ( readlen & 0x23FF ) | 0x2000 );
		} else {
			mlen = htons( readlen & 0x03FF );
		}
		memcpy( & ( header.msgtype ), & mlen, sizeof(short) );

		unsigned short downseq = seqid + i + 1;
		if(val.msgid == 0x8001) {
			downseq = 0; //ƽ̨ͨ��Ӧ��Ľ⾯���ܲ���Ҫ�ն�Ӧ��
		}
		header.seq = htons( downseq );

		DataBuffer buf;
		// д��ͷ������
		buf.writeBlock( & header, sizeof ( header ) );

		// ���Ϊ�ְ�������Ҫ����ְ��İ����Լ������
		if ( count > 1 ) {
			buf.writeInt16( count );
			buf.writeInt16( i + 1 );
		}
		// ��ȡ����
		if ( readlen > 0 ) {
			// ��ȡÿ�����ݳ���
			buf.writeBlock( ptr + offset, readlen );
			// �����ȡ���Ⱥ�ƫ��
			offset += readlen;
			left = left - readlen;
		}

		footer.check_sum = _gb_proto_handler->get_check_sum( buf.getBuffer() + 1, buf.getLength() - 1 );
		buf.writeBlock( & footer, sizeof ( footer ) );

		// �·�����ʱ��ֱ����ȴ���Ӧ�����У����û���ž��ط�������ж�����ݰ�����ֻ��ÿһ����ֱ���·����������ݰ��ȴ��ն���Ӧ���������߳�ʱ����
		_pEnv->GetClientServer()->HandleDownData( car_id.c_str(), buf.getBuffer(), buf.getLength() , downseq, ( i == 0 ) ) ;

		if(mac_id.empty() && command.empty() && seq.empty()) {
			continue; //����Ҫ��Ӧ�÷���Ӧ���ָ��
		}

		string cache_key = car_id + "_" + ustodecstr( downseq );

		CacheData cache_data;
		cache_data.str_send_msg = "EXIT";
		cache_data.command = command;
		cache_data.seq = seq;
		cache_data.mac_id = mac_id;
		cache_data.send_time = time( 0 );
		_cache_data_pool.add( cache_key, cache_data );
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	User &user = _client_user;

	OUT_WARNING( user._ip.c_str() , user._port , user._user_id.c_str() , "fd %d on_dis_connection", sock->_fd );

	user._fd = NULL;
	user._user_state = User::OFF_LINE;
	user._last_active_time = time( 0 );
}

void MsgClient::TimeWork( )
{
	while ( Check() ) {
		// ��������쳣����
		if ( ! _dataqueue.Check() ) {
			sleep( 1 );
		}

		list<CacheData> lst;
		list<CacheData>::iterator it;
		if (_cache_data_pool.timeoutData(120, lst)) {
			for (it = lst.begin(); it != lst.end(); ++it) {
				CacheData &req = *it;
				// ����ʱ����
				string caits = "CAITR " + req.seq + " " + req.mac_id + " 0 "
						+ req.command + " {RET:5} \r\n";
				// ���ͳ�ʱ��Ӧ����
				HandleUpData(caits.c_str(), caits.length());
			}
		}

		sleep(10);
	}
}

void MsgClient::NoopWork( )
{
	while ( Check() ) {
		HandleUserStatus();

		sleep( 10 );
	}
}

// ����ʧ�ܵ�����
void MsgClient::on_send_failed( socket_t *sock , void* data, int len )
{
	OUT_INFO( NULL, 0, "MsgClient", "Failed:add fd %d length %d, data:%s" , sock->_fd, len , (const char*)data );
	// �������ʧ�ܵ����ݲ�Ϊ�������ݲ���Ҫ��������
	if ( strncmp((const char*)data, "CAIT", 4 ) != 0 && len < 50 ) {
		return ;
	}
	// ���뻺�������
	_dataqueue.WriteCache( WAS_CLIENT_ID, ( void* ) data, len );
}

int MsgClient::build_login_msg( User &user, char *buf, int buf_len )
{
	string login_msg = "LOGI " + user._user_type + " " + user._user_name + " " + user._user_pwd + " \r\n";

	memcpy( buf, login_msg.c_str(), login_msg.length() );
	return login_msg.length();
}

void MsgClient::HandleUserStatus( )
{
	time_t now = time( NULL );
	User &user = _client_user;

	if(user._fd == NULL || user._user_state != User::ON_LINE || now - user._last_active_time > MAX_TIMEOUT_USER) {
		_dataqueue.Offline( WAS_CLIENT_ID);

		if ( ConnectServer( user, 10 ) ) {
			OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(), "fd %d connect server" , user._fd->_fd);
		} else {
			OUT_WARNING( user._ip.c_str(), user._port, user._user_name.c_str(), "connect msg server failed");
		}
	} else {
		string loop = "NOOP \r\n";
		SendRC4Data(user._fd, loop.c_str(), loop.length());
		_dataqueue.Online( WAS_CLIENT_ID );
		OUT_INFO( user._ip.c_str(), user._port, user._user_id.c_str(), "NOOP" );
	}
}

//=============================================================================
// �������ݻص��ӿ�
int MsgClient::HandleQueue( const char *sid, void *buf, int len , int msgid )
{
	// �������ʧ�ܵ����ݲ�Ϊ�������ݲ���Ҫ��������
	if ( strncmp((const char*)buf, "CAIT", 4 ) != 0 && len < 50 ) {
		return IOHANDLE_SUCCESS;
	}

	User &user = _client_user;
	if(user._user_state != User::ON_LINE || user._fd == NULL) {
		return IOHANDLE_FAILED;
	}

	// ʵ�������·�Ӧ�����ϴ��Ŷ�
	if ( ! SendRC4Data( user._fd, ( const char * ) buf, len ) ) {
		return IOHANDLE_FAILED;
	}
	OUT_SEND3( user._ip.c_str(), user._port, user._user_id.c_str() , "from cache msg send:%s" , (const char *)buf );

	return IOHANDLE_SUCCESS;
}

// ����RC4����
bool MsgClient::SendRC4Data( socket_t *sock , const char *data, int len )
{
	if ( ! SendData( sock , data, len ) ) {
		return false ;
	}
	// ���ͳ��
	_sendstat.AddFlux( len );
	return true ;
}
