//#include "stdafx.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Monitor.h>
#include <Thread.h>
#include <map>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
extern int errno;
#include <sys/syscall.h>
#else
#include <process.h>
#include <windows.h>
#endif

#include "cLog.h"
#define BUF_LEN 10240
#define TIME_BUF_LEN 20
#define KEY_WORD_LEN 10
#define IP_LEN  18
#define PORT_LEN 8
#define USER_ID_LEN 20

#ifndef S_IRWXUGO
# define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif

static string ustodecstr_log(unsigned short us)
{
	string dest;
	char buf[16] = {0};
	sprintf(buf,"%u",us);
	dest += buf;
	return dest;
}

/**
 * �����༶Ŀ¼, �ڸ�Ŀ¼������һͬ����
 */
static bool mkdirs_log(char *szDirPath)
{
	struct stat stats;
	if (lstat (szDirPath, &stats) == 0 && S_ISDIR (stats.st_mode))
		return true;

	mode_t umask_value = umask (0);
	umask (umask_value);
	mode_t mode = (S_IRWXUGO & (~ umask_value)) | S_IWUSR | S_IXUSR;

	char *slash = szDirPath;
	while (*slash == '/')
		slash++;

	while (1)
	{
		slash = strchr (slash, '/');
		if (slash == NULL)
			break;

		*slash = '\0';
		int ret = mkdir(szDirPath, mode);
		*slash++ = '/';
		if (ret && errno != EEXIST) {
			return false;
		}

		while (*slash == '/')
			slash++;
	}
	if (mkdir(szDirPath, mode)) {
		return false;
	}
	return true;
}

//========================== ��־�߳�  =============================
class CCLog ;
class CLogThread : public share::Runnable
{
public:
	CLogThread(CCLog *p):_inited(false) {
		_pLog = p ;
		start() ;
	}
	~CLogThread() {
		stop() ;
	}
	// �����߳�
	void run( void *param ){
		while( _inited ) {
			// ����ļ��Ƿ����
			_pLog->checklogfile() ;
			// ʹ��ͬ����
			share::Synchronized s(_monitor) ;
			// ÿ��3�뽫�ڴ���־д���ļ�
			_monitor.wait( 5 ) ;
		}
	}

	// ����֪ͨ�ź�
	void notify( void ) {
		_monitor.notify() ;
	}

private:
	void start( void ) {
		if ( ! _threadmgr.init( 1, this, this ) ) {
			printf( "start clear thread failed\n" ) ;
			return ;
		}
		_inited = true ;
		_threadmgr.start() ;
		printf( "start log check thread\n" ) ;
	}
	void stop( void ) {
		printf(" stop log check thread\n" ) ;
		if ( ! _inited )
			return ;
		_inited = false ;
		_monitor.notifyEnd() ;
		_threadmgr.stop() ;
	}
private:
	CCLog				 *_pLog ;
	// �̹߳������
	share::ThreadManager  _threadmgr ;
	// �Ƿ��ʼ��
	bool 				  _inited ;
	// �ź�������
	share::Monitor		  _monitor ;
};

//////////////////////////////////// ��־����Ĵ���  ///////////////////////////////////////////////
CCLog::CCLog()
{
	_log_size        = 2000 * 2000 ;
	_log_num         = 20 ;
	_log_level       = 3 ;
	_file_fd		 = -1 ;
	_check_time		 = time(NULL) ;
	_LogBlock.offset = 0 ;
	_LogBlock.size   = 0 ;
	// ��ʼ���ļ�����߳�
	_pthread = new CLogThread(this) ;
}

CCLog::~CCLog(void)
{
	// ֹͣ�ļ�����߳�
	if( _pthread != NULL ) {
		delete _pthread ;
		_pthread = NULL ;
	}
	writedisk() ;
	closefile() ;
}

//��ȫ�Ժ�Ч�ʵ�ͳһ
bool CCLog::print_net_msg(unsigned short log_level, const char *file, int line, const char *function, const char *key_word,
		const char * ip, int port, const char *user_id, const char *format, ...)
{
	//��־����1-7������ԽС��־��־����Խ�ߡ�
	if( _log_level == 0 )
		return false;
	if(log_level > _log_level )
		return false;

	char msg[MAX_LOG_LENGTH + 1] = {0};
	int pos = 0;

	time_t t;
	time(&t);
	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;

	pos = snprintf(msg, MAX_LOG_LENGTH, "%04d%02d%02d-%02d:%02d:%02d", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	key_word = (key_word != NULL) ? key_word : "NULL";
	pos += snprintf(msg + pos, MAX_LOG_LENGTH - pos, "--%.4s", key_word);

	ip = (ip != NULL) ? ip : "NULL";
	pos += snprintf(msg + pos, MAX_LOG_LENGTH - pos, "--%.15s:%d", ip, port);

	user_id = (user_id != NULL) ? user_id : "NULL";
	pos += snprintf(msg + pos, MAX_LOG_LENGTH - pos, "--%.16s--", user_id);

	if (format != NULL && pos < MAX_LOG_LENGTH){
		va_list ap;
		va_start(ap, format);
		vsnprintf(msg + pos, MAX_LOG_LENGTH - pos, format, ap);
		va_end(ap);
	}

	private_log( msg , file, line, function , ( strncmp( key_word, "RUNNING", 7 ) == 0 ) ) ;

	return true;
}

// ���ʮ���Ƶ���־����
void CCLog::print_net_hex( unsigned short log_level, const char *file, int line, const char *function, const char * ip, int port, const char *user_id, const char *data, const int len )
{
	// ����رյ�����־
	if( log_level > _log_level ) return ;
	// ��ȷ��һ���Ƿ񳬳�����
	if ( 2*len+300 > MAX_LOG_LENGTH || len <= 0 || data == NULL ) {
		return  ;
	}

	// ʹ�ö�����ռ�Ч�ʻ��һ��
	char buf[MAX_LOG_LENGTH + 1] = {0} ;
	int pos = 0;
	int ret;

	time_t t;
	time(&t);
	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;

	pos = snprintf(buf, MAX_LOG_LENGTH, "%04d%02d%02d-%02d:%02d:%02d", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	pos += snprintf(buf + pos, MAX_LOG_LENGTH - pos, "--DUMP");

	ip = (ip != NULL) ? ip : "NULL";
	pos += snprintf(buf + pos, MAX_LOG_LENGTH - pos, "--%.15s:%d", ip, port);

	user_id = (user_id != NULL) ? user_id : "NULL";
	pos += snprintf(buf + pos, MAX_LOG_LENGTH - pos, "--%.16s--", user_id);

	const char *TAB = "0123456789abcdef";
	unsigned char uch;

	for (int i = 0; i < len; ++i) {
		uch = data[i];
		buf[pos++] = TAB[uch >> 4];
		buf[pos++] = TAB[uch & 0xf];
	}
	buf[pos] = '\0';

	//for(int i = 0; i < len; ++i){
	//	pos += sprintf( buf + pos,"%02x",(unsigned char)data[i]);
	//}
	// д��ʮ����������
	private_log( buf , file, line, function , false ) ;
}

static int getfilesize( const char *filename )
{
	struct stat buf ;
	if ( lstat( filename , &buf ) != 0 ){
		// ����ļ���Ϊ�޷����ʣ�����ʣ��ռ�Ϊ0����˼���ٽ��ռ�
		return 0 ;
	}
	return buf.st_size ;
}

// д���ļ�
void CCLog::private_log( const char *msg , const char *file, int line, const char *function , bool run )
{
	if ( msg == NULL ) {
		return ;
	}

	char buf[256] = {0} ;
	if ( file != NULL && line > 0 && function != NULL && ! run ) {
		sprintf( buf, ",%s,%s:%d\n" , function, file, line ) ;
	} else {
		sprintf( buf, "\n" ) ;
	}

	int nsize = strlen( msg ) ;
	int nbuff = strlen( buf ) ;

	// �Ƿ�Ϊ������־
	if ( run ) {
		// ���Ϊ������־��ֱ�Ӵ��ļ�ֱ��д������
		int fd = open( _run_name.c_str() , O_CREAT|O_APPEND|O_RDWR , 0755 ) ;
		if ( fd != -1 ) {
			write( fd, msg, nsize ) ;
			write( fd, buf, nbuff ) ;
			close( fd ) ;
		}
	}

	_mutex.lock() ;

	// ���д��������Ƿ��������ڴ滺��
	if ( _LogBlock.offset + nsize + nbuff > DEFAULT_MAXLOGSIZE ){
		dumpfile() ;
	}

	// ������д���ڴ滺����
	memcpy( _LogBlock.data + _LogBlock.offset, msg, nsize ) ;
	_LogBlock.offset = _LogBlock.offset + nsize ;

	memcpy( _LogBlock.data + _LogBlock.offset, buf, nbuff ) ;
	_LogBlock.offset = _LogBlock.offset + nbuff ;

	_LogBlock.size   = _LogBlock.size + nsize + nbuff ;

	_mutex.unlock() ;


}

bool CCLog::update_file()
{
	time_t t;
	time(&t);
	struct tm local_tm;
	struct tm *tm = localtime_r( &t, &local_tm ) ;

	char buf[128]={0};
	sprintf(buf, "%04d%02d%02d-%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon
			+ 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	// ��Ŀ¼��������Ŀ¼���й��ദ��
	char szdir[256] = {0};
	sprintf( szdir, "%04d/%02d/%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday ) ;

	string sfile = _file_name + "." ;
	sfile += buf ;
	size_t npos = _file_name.rfind( '/' ) ;
	if ( npos != string::npos ) {
		string sdir  = _file_name.substr( 0, npos+1 ) + szdir ;
		if( access( sdir.c_str(), 0 ) != 0 ) {
			mkdirs_log( (char*)sdir.c_str() ) ;
		}
		sfile = sdir + _file_name.substr( npos ) + ".";
		sfile += buf ;
	}

	string snew = sfile ;
	// ������־������־ȫ������
	int pos = 0 ;
	while ( access( snew.c_str(), 0 ) == F_OK ) {
		snew  = sfile + "-" + ustodecstr_log(++pos) ;
	}
	rename( _file_name.c_str(), snew.c_str() ) ;
	// ���´�һ���ļ�����
	openfile() ;
	/**
	unsigned short counter = _log_num;
	string new_file;
	string old_file;
	for(int i = counter; i > 0; i--)
	{
		new_file = _file_name + "." + ustodecstr_log(counter);
		old_file = _file_name +"." + ustodecstr_log(--counter);
		if(counter == 0)
			old_file = _file_name;
		rename(old_file.c_str(),new_file.c_str());
	}*/
	return true;
}

// ���ļ�����
void CCLog::openfile( void )
{
	if ( _file_fd > 0 ) {
		close( _file_fd ) ;
	}
	// ���ļ�FD������־
	_file_fd = open( _file_name.c_str() , O_CREAT|O_APPEND|O_RDWR , 0755 );
}

// �ر��ļ�FD
void CCLog::closefile( void )
{
	if ( _file_fd > 0 ) {
		close( _file_fd ) ;
	}
	_file_fd = -1 ;
}

void CCLog::set_log_file(const char *s)
{
	if(s == NULL)
		return;
	_file_name = s;
	size_t pos = _file_name.rfind( '/' ) ;
	if ( pos != string::npos ) {
		// ����Ŀ¼
		mkdirs_log( (char*)_file_name.substr( 0, pos ).c_str() ) ;
	}
	_run_name = _file_name + ".running" ;

	openfile() ;
}

/////////////////////////////////////////// CDirectoryFile //////////////////////////////////////////////
// Ŀ¼�ļ��������
class CDirectoryFile
{
public:
	CDirectoryFile(){}
	~CDirectoryFile(){}

	// ���Ŀ¼�����Ƿ�����
	bool Check( const char *root, const char *name , int log_num )
	{
		map<string,string>  filemap ;
		// ȡ���ļ������ݴ���
		int count = GetLogFileList( root , name , filemap ) ;
		if( count <= log_num )
			return false ;

		int num = count - log_num ;
		// ����ɾ����Ҫɾ�����ļ�
		map<string,string>::iterator it ;
		for ( it = filemap.begin(); it != filemap.end(); ++ it ) {
			unlink( (it->second).c_str() ) ;
			if ( --num <= 0 ) break ;
		}
		//filemap.clear() ;

		return true ;
	}

private:
	// �Ƿ�ΪĿ¼
	bool isDirectory(const char *szDirPath)
	{
		struct stat stats;
		if (lstat (szDirPath, &stats) == 0 && S_ISDIR (stats.st_mode))
			return true;
		return false;
	}

	// ���������ļ��б�
	int GetLogFileList( const char* root_dir, const char *name, map<string,string> &filemap )
	{
		DIR* dir_handle = opendir( root_dir );
		if ( dir_handle == NULL )
			return 0;

		int   count = 0 ;
		char  buf[1024] = {0};
		struct dirent* entry = (struct dirent*)buf ;
		struct dirent* dir   = NULL ;

		while ( dir_handle ){
			int ret_code = readdir_r( dir_handle , entry , &dir );
			if ( ret_code != 0 || dir == NULL ){
				break ;
			}

			if ( strcmp( dir->d_name , "." ) == 0 || strcmp( dir->d_name , ".." ) == 0 ){
				continue ;
			}

			char szbuf[1024] = {0} ;
			sprintf( szbuf, "%s/%s", root_dir, dir->d_name ) ;
			// �����Ŀ¼��ֱ�ӵݹ����
			if ( isDirectory(szbuf) ) {
				count += GetLogFileList( szbuf, name, filemap ) ;
				continue ;
			}

			// ����Ǳ���־�ļ�����Ӵ���
			if ( strcmp( dir->d_name ,name ) == 0 ) {
				continue ;
			}

			// ���Ϊ��ʷ��־�ļ���Ӵ��������
			if ( strncmp( dir->d_name, name, strlen(name) ) != 0 ){
				continue ;
			}
			// printf( "add file: %s\n", szbuf ) ;
			filemap.insert( make_pair( dir->d_name, szbuf ) ) ;

			++ count ;
		}
		closedir( dir_handle ) ;

		return count ;
	}
};

// ���ڴ�����д�����
void CCLog::writedisk( void )
{
	_mutex.lock() ;
	dumpfile() ;
	_mutex.unlock() ;
}

// �����־����
void CCLog::checklogfile( void )
{
	writedisk() ;
	checkfile() ;
}

// ���ڴ���־д���ļ�
void CCLog::dumpfile( void )
{
	// ����ʹ����־˫����������������ֻ����л�һ�»������Ϳ�����
	if ( _LogBlock.offset == 0 ) {
		return ;
	}

	// ���ļ���������ļ����ܱ��򿪣��ͷ��ش���
	if ( _file_fd <= 0 ) openfile() ;
	if ( _file_fd > 0 ) {
		int size = 0 , cnt = 0 ;
		// ���д��ʧ���������δ���
		while( size <= 0 && ++ cnt < 2 ) {
			// д����Ϣ
			size += write( _file_fd , _LogBlock.data , _LogBlock.offset ) ;
			if ( size <= 0 ) openfile() ;
			// ����޷����ļ���������ֱ���˳���
			if ( _file_fd <= 0 ) break ;
		}
	} else {
		printf( "%s\n" , _LogBlock.data ) ;
	}
	_LogBlock.offset = 0 ;

	// �����¼�������ֵ
	if ( _LogBlock.size > DEFAULT_MAXLOGSIZE ) {
		// ȡ���ļ���С
		if ( getfilesize(_file_name.c_str()) > _log_size ) {
			// �������ļ�
			update_file() ;
		}
		_LogBlock.size = 0 ;
	}
}

// �����־�����Ƿ񳬳�����
void CCLog::checkfile( void )
{
	// printf( "check log file\n" ) ;
	// �����Ҫ�������ļ�����������ֻ������ô��ʱ����ļ���
	if ( _log_num <= 0 )
		return ;

	time_t now = time(NULL) ;
	// ���û�е�ʱ�����´���,ÿ��5���Ӽ��һ��
	if ( now - _check_time < 300 ) {
		return ;
	}
	_check_time = now ;

	size_t pos = _file_name.rfind( '/' ) ;
	if ( pos == string::npos ) {
		return ;
	}

	string path = _file_name.substr( 0, pos ) ;
	string name = _file_name.substr( pos+1 )  ;

	// Ŀ¼�ļ��������
	CDirectoryFile dirfile ;
	dirfile.Check( path.c_str(), name.c_str() , _log_num ) ;
}

void debug_printf(const char *file, int line, const char *fmt, ...)
{
	va_list ap;

#ifdef  WIN32
	DWORD pid = GetCurrentThreadId();
#else
	//pthread_t pid = pthread_self();
#ifdef _UNIX
	pid_t pid = getpid() ;
#else
	pid_t pid =  (long)syscall(__NR_gettid);
#endif
#endif

	fprintf(stdout, "(%s:%d:PID %d:TID %d)\n", file, line, getpid(), pid);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fprintf(stdout, "\n");
	fflush(stdout);
}

void info_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);

	fprintf(stdout, "\n");
	fflush(stdout);
}
