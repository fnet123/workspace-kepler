#include "scpmedia.h"
#include <comlog.h>
#include <BaseTools.h>
#include <assert.h>
#include <fcntl.h>
#include <tools.h>
#include <unistd.h>
#include <sys/stat.h>

#include <gd.h>

// ��ȡͼƬ�����¼���������
static string getEventText(int event)
{
    const char *text[] = {
        "\xe5\xb9\xb3\xe5\x8f\xb0", "\xe5\xae\x9a\xe6\x97\xb6",
        "\xe5\x8a\xab\xe8\xad\xa6", "\xe7\xa2\xb0\xe6\x92\x9e",
        "\xe9\x97\xa8\xe5\xbc\x80", "\xe9\x97\xa8\xe5\x85\xb3", "OTC",
        "\xe5\xae\x9a\xe8\xb7\x9d", "\xe7\x99\xbb\xe5\xbd\x95"};
    const int size = sizeof(text) / sizeof(void*);

    event &= 0xf;
    if(event < 0 || event > size - 1) {
        return "\xe6\x9c\xaa\xe7\x9f\xa5";
    }

    return text[event];
}


// ���δ���Ŀ¼
static void reverse_mkdir( const char *root, const char *path )
{
	char buf[512] = {0} ;
	sprintf( buf, "%s" , path ) ;
	char *p = buf ;

	char szpath[1024] = {0} ;
	sprintf( szpath, "%s" , root ) ;

	char *q = strstr( p , "/" ) ;
	while ( q != NULL ) {
		*q = 0 ;
		strcat( szpath, "/" ) ;
		strcat( szpath, p   ) ;
		if ( access( szpath , 0 ) != F_OK ) {
			mkdir( szpath, 0777 ) ;
		}
		p = q + 1 ;
		q = strstr( p, "/" ) ;
	}
	if ( p != NULL ) {
		strcat( szpath, "/" ) ;
		strcat( szpath, p   ) ;
		if ( access( szpath , 0 ) != F_OK   ) {
			mkdir( szpath, 0777 ) ;
		}
	}
}

CScpMedia::CScpMedia() : _cfs_enable(false)
{
	_thread_num   = 1 ;
	_last_check   = 0 ;
	_netobj._pobj = NULL ;
	_mediaqueue   = new CDataQueue<MultiDesc>(102400) ;
	_queuethread= new CQueueThread( _mediaqueue, this ) ;
}

CScpMedia::~CScpMedia()
{
	Stop() ;

	if ( _queuethread != NULL ) {
		delete _queuethread ;
		_queuethread = NULL ;
	}

	if ( _mediaqueue != NULL ) {
		delete _mediaqueue ;
		_mediaqueue = NULL ;
	}

	// �ͷ������ļ�����
	if ( _netobj._pobj != NULL ) {
		NetFileMgr::release( _netobj._pobj ) ;
		_netobj._pobj = NULL ;
	}
}

bool CScpMedia::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char buf[1024] = {0};
	if ( ! pEnv->GetString( "cfs_ip" , buf ) ){
		printf( "get ftp ip failed, %s:%d\n", __FILE__, __LINE__ ) ;
		return false ;
	}
	_cfs_ip = buf ;

	if ( ! pEnv->GetString( "cfs_user" , buf ) ) {
		printf( "get ftp save dir failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}
	_cfs_user = buf ;

	if ( ! pEnv->GetString( "cfs_pwd" , buf ) ) {
		printf( "get ftp pwd failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}
	_cfs_pwd = buf ;

	int ntemp  = 0 ;
	if ( ! pEnv->GetInteger( "cfs_port" , ntemp ) ) {
		printf( " get scp port failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}
	_cfs_port = ntemp ;

	if ( pEnv->GetInteger( "cfs_thread" , ntemp ) ) {
		_thread_num = ntemp ;
	}
	// �Ƿ���FTP�ļ�����
	if ( pEnv->GetInteger( "cfs_enable" , ntemp ) ) {
		_cfs_enable = ( ntemp == 1 ) ;
	}
	// �Ƿ�������ˮӡ,����ϵͳ���밲װ��imagemagick���
	if ( pEnv->GetInteger( "water_enable", ntemp ) ) {
		_water_enable = ( ntemp == 1 ) ;
	}

	gdFTUseFontConfig(1);

	// �������Զ���ļ�����ͳ�ʼ������
	if ( _cfs_enable ) {
#ifdef _SYN_MODE_
		_netobj._pobj = NetFileMgr::getfileobj( NetFileMgr::SYN_MODE ) ;
#else
		_netobj._pobj = NetFileMgr::getfileobj( NetFileMgr::ASYN_MODE ) ;
#endif
	}
	// ��ʼSCP�ļ��������
	return _scp_manager.Init( pEnv ) ;
}

bool CScpMedia::Start( void )
{
	return _queuethread->Init( _thread_num ) ;
}

void CScpMedia::Stop( void )
{
	_queuethread->Stop() ;
}

// ȡ���ļ���׺
static const char * getfileext( int ntype )
{
	 switch(ntype)
	 {
	 case 0:
		return ".jpeg";
	 case 1:
		return ".tif";
	 case 2:
		return ".mp3";
	 case 3:
		return ".wav";
	 case 4:
		return ".wmv";
	 default:
		break;
	 }
	 return ".data" ;
}

// �������ݰ�����
bool CScpMedia::SavePackage( int fd, const char *macid, int id, int total, int index, int type,
		int ftype,int event,int channel, const char * data, int len , const char *gps )
{
	if ( data == NULL || len == 0 ) {
		return false ;
	}

	MultiDesc  *pdesc = new MultiDesc;
	// ����SCP�ļ����ݣ�����ֿ��ļ�����ֱ�ӱ������ڴ��д���
	if ( ! _scp_manager.SaveScpFile( fd, macid, id ,total, index, type, ftype, event, channel, data, len , gps , pdesc ) ){
		OUT_INFO( NULL, 0, "scpmedia" , "save macid %s , id %d , data len %d , index %d , total %d , type %d , ftype %d, event %d ,channel %d , %s:%d" ,
				macid, id, len, index, total, type, ftype, event, channel, __FILE__, __LINE__ ) ;
		delete pdesc ;
		return false ;
	}

	// ������ݣ����Ժ��߳��ϴ�FTP
	if ( ! _queuethread->Push( pdesc ) ) {
		delete pdesc ;
		return false ;
	}
	return true ;
}

// �������������
void CScpMedia::RemovePackage( const char *macid )
{
	if ( macid == NULL ) {
		return ;
	}
	// �Ƴ���һ�γ���û���ϴ���ɵ�ͼƬ�ļ�
	_scp_manager.RemoveScpFile( macid ) ;
}

// ���ַ���ʱ��תΪBCDʱ��
static const string getstrtime( void )
{
	time_t ntime  = time(NULL) ;
	struct tm local_tm;
	struct tm *tm = localtime_r( &ntime, &local_tm ) ;

	char buf[256] = {0} ;
	sprintf( buf, "%04d.%02d.%02d %02d:%02d:%02d",
			( tm->tm_year + 1900 ) , tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec) ;

	// ת��ΪBCDʱ��
	return string(buf) ;
}

// ִ�н���
static bool executeprocess( char *cmd )
{
	// ִ�н���
	FILE *fp = NULL;
	if( ( fp=popen( cmd, "r" ) ) == NULL ) {
		//�쳣����
		return false ;
	}
	pclose(fp);

	return true ;
}

// д��Զ���ļ�
bool CScpMedia::writeFile( const char *dir, const char *name, const char *path )
{
	if ( ! _cfs_enable || _netobj._pobj == NULL )
		return true ;

	int   len = 0 ;
	char *ptr = ReadFile( path, len ) ;
	if ( ptr == NULL ) {
		OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "scpmedia", "read file %s failed" , path ) ;
		//�쳣����
		return false ;
	}

	char szfile[256] = {0} ;
	sprintf( szfile, "%s/%s", dir, name ) ;

	// ���ﴮ�л�����ͼƬ�ϴ�
	share::Guard guard( _netobj._mutex ) ;

#ifdef _SYN_MODE_
	// ���û�����ӻ��߷���ʧ��
	int ret = _netobj._pobj->open( _cfs_ip.c_str(), _cfs_port, _cfs_user.c_str(), _cfs_pwd.c_str() ) ;
	if ( ret != FILE_RET_SUCCESS ) {
		OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "scpmedia", "open file mgr failed, result %d" , ret ) ;
		FreeBuffer( ptr ) ;
		return false ;
	}
#else
	int ret = _netobj._pobj->write( szfile, ptr, len ) ;
	if ( ret == FILE_RET_SUCCESS ) {
		FreeBuffer( ptr ) ;
		return true ;
	}

	// ���û�����ӻ��߷���ʧ��
	if ( ret == FILE_RET_NOCONN || ret == FILE_RET_SENDERR || ret == FILE_RET_READERR ) {
		ret = _netobj._pobj->open( _cfs_ip.c_str(), _cfs_port, _cfs_user.c_str(), _cfs_pwd.c_str() ) ;
		if ( ret != FILE_RET_SUCCESS ) {
			OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "scpmedia", "open file mgr failed, result %d" , ret ) ;
			FreeBuffer( ptr ) ;
			return false ;
		}
	}
#endif

	int retry = 0 ;
	// ��������ļ�ʧ�����Լ���
	while( ++ retry < 3 ) {
		ret = _netobj._pobj->write( szfile, ptr, len ) ;
		if ( ret == FILE_RET_SUCCESS ) {
			break ;
		}
	}
	FreeBuffer( ptr ) ;

	// �����Ƿ�д���ļ��ɹ�
	return ( ret == FILE_RET_SUCCESS ) ;
}

// ��ⳬʱ����
void CScpMedia::CheckTimeOut( void )
{
	time_t now = time(NULL) ;
	// ÿ����ʮ����һ��
	if ( now - _last_check > 30 ) {
		_last_check = now ;
		// ����SCP�ĳ�ʱ����
		_scp_manager.CheckTimeOut( SCP_MAX_TIMEOUT ) ;
	}
}

static bool picNote(const char *path, char *text)
{
	FILE *fp;
	int c;
	int x, y, w, h;
	int rect[8] = { 0 };
	gdImagePtr im;

	if ((fp = fopen(path, "rb")) == NULL) {
		return false;
	}

	im = gdImageCreateFromJpeg(fp);
	fclose(fp);
	if (im == NULL) {
		return false;
	}

	c = gdImageColorAllocate(im, 255, 255, 255);
	w = gdImageSX(im);
	h = gdImageSY(im);

	memset(rect, 0x00, sizeof(rect));
	gdImageStringFT(NULL, rect, c, "SimHei", 13, 0.0, 0, 0, text);
	x = w - rect[4] - 5;
	y = h - 10;
	gdImageStringFT(im, NULL, c, "SimHei", 13, 0.0, x, y, text);

	if ((fp = fopen(path, "wb")) == NULL) {
		gdImageDestroy(im);
		return false;
	}

	gdImageJpeg(im, fp, -1);
	fclose(fp);

	gdImageDestroy(im);

	return true;
}

// �߳�ִ�ж��󷽷�
void CScpMedia::HandleQueue( void *packet )
{
	MultiDesc *elem = (MultiDesc*) packet ;
	// mogrify -pointsize 16 -fill white -weight bolder -gravity southeast -annotate +5+5 "2012.03.03 16:59 #1" 1.jpg
	if ( _water_enable && elem->_type == 0 ) {  // �����ˮӡ��ΪͼƬʱ�ſ���ˮӡ����
		string stime = getstrtime() ;
		char buffer[1024] = {0};

		snprintf(buffer, 1024, "%s #%d%s", stime.c_str(), elem->_channel, getEventText(elem->_event).c_str());
		if ( ! picNote(elem->_path.c_str(), buffer) ) {
			OUT_ERROR( NULL, 0, "scpmedia", "Water %s , file path %s, water date %s failed", buffer, elem->_path.c_str(), stime.c_str() ) ;
		} else {
			OUT_PRINT( NULL, 0, "scpmedia",  "Water %s, file path %s, water date %s success", buffer, elem->_path.c_str(), stime.c_str() ) ;
		}
	}

	// д��Զ�̷�����
	if ( ! writeFile( elem->_abspath.c_str(), elem->_name.c_str(), elem->_path.c_str() ) ) {
		OUT_ERROR( NULL, 0, "scpmedia", "upload file to server failed, %s" , elem->_name.c_str() ) ;
		return ;
	}

	char buf[2048] = {0} ;
	sprintf( buf, "CAITS 0_0 %s 0 U_REPT {TYPE:3,120:%d,124:%d,121:%d,122:%d,123:%d,125:%s/%s,%s,CHANNEL_TYPE:0} \r\n",
			elem->_macid.c_str(), elem->_id, elem->_channel, elem->_type, elem->_ftype, elem->_event, elem->_abspath.c_str(), elem->_name.c_str() , elem->_gps.c_str() ) ;
	// �ϴ�����
	_pEnv->GetMsgClient()->HandleUpData( buf, strlen(buf) ) ;
}
///////////////////////////////////CScpFileManager////////////////////////////////////////

CScpMedia::CScpFileManager::CScpFileManager()
{

}

CScpMedia::CScpFileManager::~CScpFileManager()
{
	share::Guard g( _mutex ) ;

	int size = 0 ;
	CScpFile *p = _queue.move(size) ;
	if ( size == 0 ) {
		return ;
	}

	while( p != NULL ) {
		p = p->_next ;
		delete p->_pre ;
	}
	_index.clear() ;
}

// ��ʼ������
bool CScpMedia::CScpFileManager::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char buf[512] = {0} ;
	if ( ! pEnv->GetString( "cur_dir" , buf ) ) {
		printf( "get current save file failed, %s:%d\n" , __FILE__, __LINE__ ) ;
		return false ;
	}

	_cur_dir = buf ;
	// ȡ�û���·��
	getenvpath( _cur_dir.c_str(), buf ) ;
	// ���Ŀ¼�������򴴽���
	mkdir( buf, 0777 ) ;
	// ���´���·��
	_cur_dir = buf ;

	return true ;
}

// �������ݰ�����
bool CScpMedia::CScpFileManager::SaveScpFile( int fd, const char *macid, int id, int total, int index, int type,
			int ftype,int event,int channel, const char * data, int len , const char *gps , MultiDesc *desc )
{
	share::Guard g( _mutex ) ;
	{
		//����Ψһ��KEYֵ macid ;
		string key = macid ;
		key += "_" ;
		key += uitodecstr(total) ;

		bool binsert = false ;

		CScpFile *p = NULL ;
		MapScpFile::iterator it = _index.find( key ) ;
		if ( it == _index.end() ) {
			p = new CScpFile(key.c_str());
			// ��ʼ����������
			binsert = p->Init( fd, macid, id, total, type, ftype, event, channel, _cur_dir.c_str() , gps ) ;
		} else {
			p = it->second ;
			// �����һ�������ǵ�һ����������Ҫ���������ļ���
			if ( index == 0x01 ) {
				p->SetParam( fd, macid, id, total, type, ftype, event, channel, _cur_dir.c_str() , gps ) ;
			}
			p = _queue.erase(p) ;
		}

		// ���Ϊ�����ļ����ݳɹ�
		if ( p->SaveData( index, data, len , desc ) ) {
			if( ! binsert )
				_index.erase( key ) ;
			delete p ;
			return true;
		}

		OUT_INFO( NULL, 0, "scpmedia", "save data macid %s , total %d , index %d , len %d" , macid, total, index, len ) ;

		_queue.push( p ) ;
		if ( binsert ) {
			// ���Ϊ�������
			_index.insert( pair<string,CScpFile*>( key, p ) ) ;
		}
		return false ;
	}
}


// ���ָ����MAC�����Ķ�ý����
void CScpMedia::CScpFileManager::RemoveScpFile( const char *macid )
{
	share::Guard g( _mutex ) ;
	{
		string key = macid ;
		MapScpFile::iterator it = _index.find( key ) ;
		if ( it == _index.end() ) {
			return ;
		}

		CScpFile *p = it->second ;
		_index.erase( it ) ;

		delete _queue.erase(p) ;
	}
}

// ��ⳬʱ���ڴ��ļ�
void CScpMedia::CScpFileManager::CheckTimeOut( const int timeout )
{
	share::Guard g( _mutex ) ;
	{
		if ( _queue.size() == 0 ){
			return ;
		}
		time_t now = time(NULL) - timeout ;

		CScpFile *t = NULL ;
		CScpFile *p = _queue.begin() ;
		while( p != NULL ) {
			t = p ;
			p = p->_next ;
			if ( t->_last > now )
				break ;

			_index.erase( t->_key ) ;
			delete _queue.erase(t) ;
		}
	}
}

/////////////////////////////////////// CScpFile /////////////////////////////////////
CScpMedia::CScpFile::CScpFile( const char *key )
	: _cur(0), _key(key)
{
}

CScpMedia::CScpFile::~CScpFile()
{
	if ( _vec.empty() ) {
		return ;
	}

	// �����ڴ��ļ�
	MemFile *p = NULL ;
	for ( size_t i = 0; i < _vec.size(); ++ i ){
		p = _vec[i] ;
		if ( p->_ptr != NULL && p->_len != 0 ){
			delete [] p->_ptr ;
		}
		delete p ;
	}
	_vec.clear() ;
}

// ���ò�������
bool CScpMedia::CScpFile::SetParam( int fd, const char *macid, int id, int total, int type, int ftype, int event,
				int channel , const char *curdir , const char *gps )
{
	time_t cur_t = _now ;
	struct tm local_tm;
	struct tm *nowtms = localtime_r( &cur_t, &local_tm ) ;

	char tbuf[256];
	strftime(tbuf,256,"%Y%m%d%H%M%S",nowtms);

	string filename = tbuf ;
	//filename = datetime-macid-mid-mevent-mchannel-mtype-mfiletype;
	filename += "-" ;
	filename += macid ;
	filename += "-"+uitodecstr(id) ;
	filename += "-"+ustodecstr(event) ;
	filename += "-"+ustodecstr(channel) ;
	filename += "-"+ustodecstr(type) ;
	filename += "-"+ustodecstr(ftype) ;
	filename += getfileext(ftype) ;

	strftime(tbuf,256,"%Y/%m/%d" , nowtms ) ;

	string filepath = curdir ;
	filepath += "/" ;
	filepath += tbuf ;
	filepath += "/" ;
	filepath += filename ;

	// ��Ҫ���δ���Ŀ¼
	reverse_mkdir( curdir , tbuf ) ;

	// ������ݣ����Ժ��߳��ϴ�FTP
	_desc._fd      = fd ;
	_desc._macid   = macid ;
	_desc._id      = id ;
	_desc._total   = total ;
	_desc._type    = type ;
	_desc._ftype   = ftype ;
	_desc._event   = event ;
	_desc._channel = channel ;
	_desc._name    = filename ;
	_desc._path    = filepath ;
	_desc._abspath = tbuf ;
	if ( gps )
		_desc._gps = gps ;

	return true ;
}
// ��ʼ������
bool CScpMedia::CScpFile::Init( int fd, const char *macid, int id, int total, int type, int ftype, int event, int channel , const char *curdir , const char *gps )
{
	time_t cur_t = time(0);
	_now = cur_t ;

	// ���ò���
	SetParam( fd, macid, id, total, type, ftype, event, channel, curdir , gps ) ;

	// û�зְ�����
	if ( total == 1 )
		return true;

	// ����ְ�����
	for ( int i = 0; i < total; ++ i )
	{
		MemFile *p = new MemFile ;
		p->_index  = 0 ;
		p->_len	   = 0 ;
		p->_ptr    = NULL ;
		_vec.push_back( p ) ;
	}

	return true ;
}

// �����ļ�
bool CScpMedia::CScpFile::SaveData( const int index, const char * data, int len , MultiDesc *desc )
{
	// �ȼ�����ݵ���ȷ��
	if ( data == NULL || len == 0 || index > _desc._total ){
		OUT_ERROR( NULL, 0, "ftp" , "file index %d total %d len %d" , index, _desc._total , len ) ;
		return false ;
	}

	_last = time(NULL) ;
	// ���ֻ��һ�����ݰ�ֱ�ӱ���
	if ( _desc._total == 1 ) {
		// ���浽�����ļ�Ŀ¼
		if ( ! AppendFile( _desc._path.c_str(), data, len ) ) {
			OUT_ERROR( NULL, 0, "scpmedia" , "ftp save file to path %s failed" , _desc._path.c_str() ) ;
			return false ;
		}
		// ������������
		desc->Copy( _desc ) ;

		OUT_INFO( NULL, 0, "scpmedia" , "ftp save file path %s" , _desc._path.c_str() ) ;

		return true ;
	}
	// �������ֵ��������������ֱ�ӷ���
	if ( index > (int)_vec.size() ) {
		OUT_ERROR( NULL, 0, "scpmedia" , "ftp save file index %d over max total %d" , index, _vec.size()  ) ;
		return false ;
	}
	// ȡ������ֵ
	MemFile *p = _vec[index-1] ;
	if ( p->_ptr == NULL ) {
		// ���һ�ϴ������ݲż�¼�ϴ���ȷ�İ���
		++ _cur ;
		// �����µĿռ�
		p->_ptr    = new char[len] ;

	} else {
		// �����һ�δ������ݳ���Ҫ������һ���ϴ�����
		if ( p->_len < len ) {
			// �ͷ�ԭ�����ڴ�
			delete [] p->_ptr ;
			p->_ptr   = new char[len] ;
		}
	}

	p->_len    = len ;
	p->_index  = index ;
	memcpy( p->_ptr, data, len ) ;

	if ( (int)_cur != _desc._total ) {
		OUT_INFO( NULL, 0, "scpmedia" , "scp save index file %d, current count %d total %d" , index, _cur, _desc._total ) ;
		return false ;
	}

	// ����������ݰ�������ɹ�����ֱ�ӱ�����ļ�
	for ( size_t i = 0; i < _vec.size(); ++ i ) {
		p = _vec[i] ;
		if ( p->_ptr == NULL || p->_len == 0 ){
			continue;
		}
		// �����ļ��������ļ�����
		AppendFile( _desc._path.c_str(), p->_ptr, p->_len ) ;
	}
	// ������������
	desc->Copy( _desc ) ;

	OUT_INFO( NULL, 0, "scpmedia" , "ftp save file path %s" , _desc._path.c_str() ) ;

	return true ;
}
