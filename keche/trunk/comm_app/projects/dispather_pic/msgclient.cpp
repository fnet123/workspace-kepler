#include "msgclient.h"
#include <tools.h>
#include "../tools/utils.h"

#include <string>
#include <vector>
#include <fstream>
using std::string;
using std::vector;
using std::ifstream;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

MsgClient::MsgClient(): _session( true ), _seqid(0)
{
	_last_handle_user_time = time(NULL) ;

	_dataroute = false;
	_pipe_uid = "";
	_save_url.clear();
}

MsgClient::~MsgClient( void )
{
	Stop() ;
}

// ����MSG���û��ļ�
bool MsgClient::LoadMsgUser( const char *userfile )
{
	if ( userfile == NULL ) return false ;

	int  len  = 0 ;
	char *ptr = ReadFile(userfile, len) ;
	if ( ptr == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "Read msg user failed, %s", userfile ) ;
		return false ;
	}

	int i = 0 ;
	// ���ָ������
	for( i = 0; i < len; ++ i ) {
		if ( ptr[i] != '\r' ) {
			continue ;
		}
		ptr[i] = '\n' ;
	}

	vector<string> vec ;
	if ( ! splitvector( ptr, vec, "\n", 0 ) ) {
		FreeBuffer( ptr ) ;
		OUT_ERROR( NULL, 0, NULL, "Split msg data failed" ) ;
		return false ;
	}

	int size = vec.size();;
	int count = 0 ;
	for( i = 0; i < size; ++ i ) {
		string &line = vec[i] ;

		if(line.empty() || line[0] == '#') {
			continue;
		}

		//1:10.1.99.115:8880:user_name:user_password:A3
		vector<string> vec_line ;
		if ( ! splitvector( line, vec_line, "$" , 7 ) ){
			continue ;
		}

		User user ;
		user._user_id     =  vec_line[0] + vec_line[1] ;
		user._access_code = atoi( vec_line[1].c_str() ) ;
		user._ip          =  vec_line[2] ;
		user._port        =  atoi( vec_line[3].c_str() ) ;
		user._user_name   =  vec_line[4] ;
		user._user_pwd    =  vec_line[5] ;
		user._user_type   =  vec_line[6] ;
		user._user_state  = User::OFF_LINE ;
		user._socket_type = User::TcpConnClient ;
		user._connect_info.keep_alive = AlwaysReConn ;
		user._connect_info.timeval    = 30 ;

		// ��ӵ��û�������
		_online_user.AddUser( user._user_id, user ) ;

		if ( user._user_id.compare(0, 10, MSG_SAVE_CLIENT) == 0) {
			string host = vec_line[7];
			_save_url.insert(make_pair(user._user_id, host));
		} else if(user._user_id.compare(0, 10, MSG_PIPE_CLIENT) == 0) {
			_pic_path = vec_line[7];
			_pipe_uid = user._user_id;
		}

		++ count ;
	}
	FreeBuffer( ptr ) ;

	OUT_INFO( NULL, 0, NULL, "load msg user success %s, count %d" , userfile , count ) ;

	return true ;
}

bool MsgClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	// ȡ���û��ļ�·��
	char user_file[256] = {0} ;
	if ( ! pEnv->GetString( "user_filepath" , user_file ) ) {
		printf( "load user file failed\n" ) ;
		return false ;
	}

	char temp[512] = {0} ;
	// ���ػ��������б�·��
	if ( pEnv->GetString( "base_dmddir" ,temp ) ) {
		_dmddir.assign( temp ) ;
		OUT_INFO( NULL, 0 , NULL, "Load base dmddir %s", temp ) ;
	}

	int nvalue = 0;
	if ( pEnv->GetInteger( "dataroute", nvalue ) ) {
		_dataroute = ( nvalue == 1 ) ;
	}

	setpackspliter( &_packspliter ) ;

	_httpClient.SetQueueSize( 64 ) ;

	_httpClient.SetDataProcessor(this);

	return LoadMsgUser( user_file ) ;
}


void MsgClient::Stop( void )
{
	OUT_INFO("Msg",0,"MsgClient","stop");

	StopClient() ;
}

bool MsgClient::Start( void )
{
	if( !_httpClient.Start(3, 3)) {
		return false;
	}

	if( ! StartClient( "0.0.0.0", 0, 3 )) {
		return false;
	}

	return true;
}

void MsgClient::on_data_arrived( socket_t *sock, const void* data, int len)
{
	if ( len < 4 ) {
		OUT_ERROR( sock->_szIp, sock->_port, NULL, "recv data error length: %d", len ) ;
		return ;
	}

    const char *ip 	    = sock->_szIp;
    unsigned short port = sock->_port;

	OUT_RECV( ip, port, NULL, "on_data_arrived:[%d]%s", len, (const char*)data );

	const char *ptr = ( const char *) data ;
	if ( strncmp( ptr, "CAIT" , 4 ) == 0  ) {
		// �׷���������
		HandleInnerData( sock, ptr, len ) ;
	} else {
		// �����½���
		HandleSession( sock, ptr, len ) ;
	}
}

void MsgClient::on_dis_connection( socket_t *sock )
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	User user = _online_user.GetUserBySocket( sock );
	OUT_WARNING( sock->_szIp , sock->_port, user._user_id.c_str(), "Disconnection fd %d" , sock->_fd );
	user._user_state = User::OFF_LINE ;
	_online_user.SetUser( user._user_id, user ) ;
}

void MsgClient::TimeWork()
{
	/*
	 * 1.����ʱ������ȥ����
	 * 2.��ʱ����NOOP��Ϣ
	 * 3.Reload�����ļ��е��µ����ӡ�
	 * 4.
	 */
	while (Check()) {
		HandleOfflineUsers();
		HandleOnlineUsers(30);

		_session.CheckTimeOut(120);

		sleep(5);
	}
}

void MsgClient::NoopWork()
{
	time_t curTime;
	unsigned int seqid;

	list<WAIT_RSP_TIME>::iterator iteTime;
	WAIT_MAP::iterator  iteData;

	while ( Check()) {
		curTime = time(NULL);

		while(true) {
			share::Guard guard(_wait_mutex);

			iteTime = _wait_time.begin();
			if(iteTime == _wait_time.end()) {
				break;
			}
			if(iteTime->time < curTime) {
				break;
			}

			seqid = iteTime->seqid;
			iteData = _wait_data.find(seqid);
			if(iteData != _wait_data.end()) {
				OUT_ERROR(NULL, 0, NULL, "http request timeout, url %s", iteData->second.inner.c_str());
				_wait_data.erase(iteData);
			}

			_wait_time.erase(iteTime);

		}

		sleep(5);
	}
}

// ������½��Ϣ����
int  MsgClient::build_login_msg(User &user, char *buf, int buf_len)
{
	string stype = "SAVE" , sext = "\r\n" ;
	if ( user._user_type == "DMDATA" ) {
		stype = "SAVE" ; sext = "DM \r\n" ;
	} else {
		stype = user._user_type ;
	}
	sprintf( buf, "LOGI %s %s %s %s",
			stype.c_str() , user._user_name.c_str(), user._user_pwd.c_str() , sext.c_str() ) ;

	return strlen(buf) ;
}

// �׷���½�û��Ự����
void MsgClient::HandleSession( socket_t *sock, const char *data, int len )
{
	string line = data;

	vector<string> vec_temp ;
	if ( ! splitvector( line, vec_temp, " " , 1 ) ) {
		return ;
	}

	const char *ip 	    = sock->_szIp ;
	unsigned short port = sock->_port ;

	string head = vec_temp[0];

	if (head == "LACK") {
		int ret = atoi(vec_temp[1].c_str());
		switch (ret) {
		case 0:
		case 1:
		case 2:
		case 3: {
			User user = _online_user.GetUserBySocket(sock);
			if (user._user_id.empty()) {
				OUT_WARNING(ip, port, NULL, "Can't find the syn_user");
				return;
			}
			user._user_state = User::ON_LINE;
			user._last_active_time = time(NULL);
			_online_user.SetUser(user._user_id, user);

			OUT_CONN( ip , port, user._user_name.c_str(), "Login success, fd %d access code %d" , sock->_fd, user._access_code );
			// ��½�ɹ������Ϊ���ݶ������Ӿ�ֱ����Ҫ�����Ͷ��Ĵ���
			if (user._user_type == "DMDATA") LoadSubscribe(user);
		}
			break;
		case -1:
			OUT_ERROR(ip, port, NULL, "LACK, password error!");
			break;
		case -2:
			OUT_ERROR(ip, port, NULL, "LACK, the user has already login!");
			break;
		case -3:
			OUT_ERROR(ip, port, NULL, "LACK, user name is invalid!");
			break;
		default:
			OUT_ERROR( ip, port, NULL, "unknow result" );
			break;
		}

		// ������ش�����ֱ�Ӵ���
		if (ret < 0) {
			_tcp_handle.close_socket(sock);
		}
	} else if (head == "NOOP_ACK") {
		User user = _online_user.GetUserBySocket(sock);
		user._last_active_time = time(NULL);
		_online_user.SetUser(user._user_id, user);

		OUT_INFO( ip, port, user._user_name.c_str(), "NOOP_ACK");
	} else {
		OUT_WARNING( ip, port, NULL, "except message:%s", (const char*)data);
	}
}

void MsgClient::HandleInnerData( socket_t *sock, const char *data, int len )
{
	const char *ip 		= sock->_szIp ;
	unsigned short port = sock->_port ;

	string line( data, len ) ;
	vector<string>  vec ;

	User user = _online_user.GetUserBySocket( sock ) ;
	if ( user._user_id.empty()  ) {
		OUT_ERROR( ip, port, "CAIS" , "find fd %d user failed, data %s", sock->_fd, data ) ;
		return ;
	}

	user._last_active_time = time(NULL) ;
	_online_user.SetUser( user._user_id, user ) ;

	if ( ! splitvector( line, vec, " " , 6 )  ) {
		OUT_ERROR( ip, port, "" , "fd %d data error: %s", sock->_fd, data ) ;
		return ;
	}

	string uid       = "";
	string head      = vec[0];
	string macid     = vec[2] ;
	string command   = vec[4];
	if ( user._user_id.compare(0, 10, MSG_PIPE_CLIENT) == 0) {
		if(_dataroute) {
			return; //����ֻ�����ת��
		}

		if ( ! _session.GetSession(macid, uid) ) {
			OUT_ERROR( NULL, 0, macid.c_str(), "Get Session Failed, Data %s" , data ) ;
			return ;
		}
	} else if(user._user_id.compare(0, 10, MSG_SAVE_CLIENT) == 0) {
		_session.AddSession( macid, user._user_id ) ;

		if(command != "U_REPT" || !processPic(user._user_id, line, vec[5]) ) {
			uid = _pipe_uid;
		}
	}

	// ����δ���߻�ͼƬ��Ϣ��������غ���ת��
	if(uid.empty()) {
		return;
	}

	user = _online_user.GetUserByUserId( uid ) ;
	if ( user._user_id.empty() || user._user_state != User::ON_LINE || user._fd == NULL ) {
		OUT_ERROR( NULL, 0, "", "Get User empty , User id: %s, data %s" , uid.c_str() , data ) ;
		return ;
	}

	// add end split string "\r\n"
	string sdata = line + "\r\n" ;

	// ��������
	SendData( user._fd, sdata.c_str(), sdata.length() ) ;
}

void MsgClient::HandleOfflineUsers()
{
	vector<User> vec_users = _online_user.GetOfflineUsers(3 * 60);
	for (int i = 0; i < (int) vec_users.size(); i++) {
		User &user = vec_users[i];
		user._user_state = User::OFF_LINE;

		if (!ConnectServer(user, 10)) {
			OUT_WARNING(user._ip.c_str(), user._port, user._user_name.c_str(), "connect fail");
		}

		_online_user.AddUser(user._user_id, user);
	}
}

void MsgClient::HandleOnlineUsers(int timeval)
{
	time_t now = time(NULL);
	if (now - _last_handle_user_time < timeval) {
		return;
	}
	_last_handle_user_time = now;

	vector<User> vec_users = _online_user.GetOnlineUsers();
	for (int i = 0; i < (int) vec_users.size(); i++) {
		User &user = vec_users[i];
		string loop = "NOOP \r\n";
		SendData(user._fd, loop.c_str(), loop.length());
		OUT_SEND(user._ip.c_str(), user._port, user._user_id.c_str(), "NOOP");
	}
}

// ���ض�������
void MsgClient::LoadSubscribe( User &user )
{
	if (_dmddir.empty())
		return;

	char szbuf[1024] = {0};
	sprintf( szbuf, "%s/%d", _dmddir.c_str(), user._access_code ) ;

    string line;
    ifstream ifs;

	size_t prev;
	size_t next;
	size_t size;
	string value;
	string inner;
	string order;

	inner = "";
	order = "DMD";
	ifs.open(szbuf);
	while (getline(ifs, line)) {
		if ((size = line.size()) > 0 && line[size - 1] == '\r') {
			line.erase(size - 1);
		}

		if (line.empty() || line[0] == '#') {
			continue;
		}

		prev = 0;
		size = line.size();
		while (prev < size) {
			if ((next = line.find_first_of(", ", prev)) == string::npos) {
				next = size;
			}
			value = line.substr(prev, next - prev);
			prev = next + 1;

			if (value.empty()) {
				continue;
			}

			if (inner.empty()) {
				inner.assign(order + " 0 {" + value);
			} else {
				inner.append("," + value);
			}

			if (inner.size() > 4096) {
				inner.append("} \r\n");
				SendData(user._fd, inner.c_str(), inner.length());
				OUT_SEND(user._ip.c_str(), user._port, "SEND", "%s", line.c_str());
				inner = "";
				order = "ADD";
			}
		}
	}
	ifs.close();

	if (!inner.empty()) {
		inner.append("} \r\n");
		SendData(user._fd, inner.c_str(), inner.length());
		OUT_SEND(user._ip.c_str(), user._port, "SEND", "%s", line.c_str());
	}
}

bool MsgClient::processPic(const string &userid, const string &msg, const string &content)
{
	int ret;

	string              type;
	string              host;
	string              file;
	string              arg_str;
	vector<string>      arg_vec;
	map<string, string> arg_map;

	arg_str = content.substr(1, content.length() - 2); //ȥ��ǰ���{}
	splitvector( arg_str, arg_vec, "," , 0);
	split2map(arg_vec, arg_map, ":");

	type = arg_map["TYPE"];
	file = arg_map["125"];
	host = _save_url[userid];
	if(type != "3" || host.empty() || file.empty()) {
		// ���Ǳ�׼�Ķ�ý����Ϣֱ��ת��
		return false;
	}

	string       url;
	CHttpRequest req;
	unsigned int seqid;

	url = host + "/" + file;
    req.SetMethod( CHttpRequest::HTTP_METHOD_GET ) ;
    req.SetURL( url.c_str() ) ;

    seqid = getSeqid();
    if((ret = _httpClient.HttpRequest(req, seqid)) == E_HTTPCLIENT_SUCCESS) {
    	WAIT_RSP_DATA rsp_data;
    	WAIT_RSP_TIME rsp_time;

    	share::Guard guard(_wait_mutex);

    	rsp_data.file = file;
    	rsp_data.inner = msg + "\r\n";
    	rsp_time.time = time(NULL) + 60;
    	rsp_time.seqid = seqid;

    	_wait_data.insert(make_pair(seqid, rsp_data));
    	_wait_time.push_back(rsp_time);
    } else {
    	OUT_ERROR(NULL, 0, NULL, "HttpRequest return %d, data %s", ret, msg.c_str());
    }

	return true;
}

void MsgClient::ProcHTTPResponse( unsigned int seq , const int err , const CHttpResponse& resp )
{
	//printf("%d, %d, %d\n", seq, err, resp.GetDataSize());
    WAIT_MAP::iterator wait_ite;
    WAIT_RSP_DATA rsp_data;

    {
    	share::Guard guard(_wait_mutex);
    	wait_ite = _wait_data.find(seq);
    	if(wait_ite == _wait_data.end()) {
    		return;
    	}

    	rsp_data = wait_ite->second;
    	_wait_data.erase(wait_ite);
    }

    if ( err != E_HTTPCLIENT_SUCCESS || resp.GetDataSize() == 0) {
    	OUT_ERROR(NULL, 0, NULL, "ProcHTTPResponse return %d, data %s", err, rsp_data.inner.c_str());
		return;
	}

    if(createDir(rsp_data.file) == false) {
    	// ûȨ�޴�����Ŀ¼
    	return;
    }

    string picFile;
    int fd;
    int ret;

    int dataLen;
    const char *dataPtr;

    picFile = _pic_path + "/" + rsp_data.file;
    fd = open(picFile.c_str(), O_CREAT | O_WRONLY | O_EXCL, 0644);
    if(fd < 0) {
    	return;
    }

    dataLen = resp.GetDataSize();
    dataPtr = resp.GetData();
    ret = write(fd, dataPtr, dataLen);
    close(fd);
    if(ret != dataLen) {
    	return;
    }

	User user = _online_user.GetUserByUserId( _pipe_uid ) ;
	if ( user._user_id.empty() || user._user_state != User::ON_LINE || user._fd == NULL ) {
		OUT_ERROR( NULL, 0, "", "Pipe User Offline , data %s" ,  rsp_data.inner.c_str() ) ;
		return ;
	}

	dataPtr = rsp_data.inner.c_str();
	dataLen = rsp_data.inner.length();
	SendData( user._fd , dataPtr, dataLen);
}

bool MsgClient::createDir(const string &file)
{
    string path;
    string::size_type posPrev;
    string::size_type posNext;

    posPrev = 0;
    while((posNext = file.find('/', posPrev)) != string::npos) {
        posPrev = posNext + 1;

        path = _pic_path + "/" + file.substr(0, posNext);
        if(access(path.c_str(), R_OK | W_OK | X_OK) == 0) {
            continue;
        }

       if(mkdir(path.c_str(), 0777) != 0) {
    	   return false;
       }
    }

    return true;
}

int MsgClient::split2map(vector<string> &vec, map<string, string> &mp, const string &split)
{
	mp.clear();

	int count = 0;
	int len = split.length();

	map<string, string>::iterator it;

	for (int i = 0; i < (int) vec.size(); ++i) {
		size_t pos = vec[i].find(split, 0);

		if (pos == string::npos) {
			continue;
		}

		++count;

		string key = vec[i].substr(0, pos);
		string value = vec[i].substr(pos + len);
		mp.insert(make_pair(key, value));
	}

	return count;
}
