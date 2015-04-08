#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
# pragma warning(disable:4996)    // identifier was truncated in debug info
#endif

#else
#include <pthread.h>
#endif

#include "singleton.h"
#include <iostream>
#include <string>
#include <Mutex.h>
#include <list>

using namespace std;

#define LOG_FILE_NUM 		20
#define MAX_LOG_LENGTH   	10240
#define MAX_BACK_LOG	 	2
#define DEFAULT_MAXLOGSIZE  1024*1024

void debug_printf(const char *file, int line, const char *fmt, ...);
void info_printf(const char *fmt, ...);

class CLogThread ;
class CCLog : public CSingleton<CCLog>
{
	struct LOGBLOCK
	{
		unsigned int size ;
		unsigned int offset;
		char data[ DEFAULT_MAXLOGSIZE+1 ];
	};

public:
	CCLog() ;
    ~CCLog() ;

    // �����־��Ϣ
    bool print_net_msg(unsigned short log_level, const char *file, int line, const char *function,
    		const char *key_word, const char * ip, int port, const char *user_id, const char *format, ... );
    // ���ʮ���Ƶ���־����
    void print_net_hex(unsigned short log_level, const char *file, int line, const char *function,
    		const char * ip, int port, const char *user_id, const char *data, const int len ) ;

    void set_log_file(const char *s);
    void set_log_num(unsigned short num = 20){ _log_num = num; }
    void set_log_level( int loglevel = 3 ) { _log_level = loglevel; }
    void set_log_size(int newsize = 20)
    {
    	if(newsize < 1000)
    		_log_size = newsize*1000*1000;
    	else
    		_log_size = 20*1000*1000;
    }
    // �����־�ļ�����
    void checklogfile( void ) ;

private:
	bool update_file();
	// д���ļ�
	void private_log( const char *msg , const char *file, int line, const char *function , bool run ) ;
	// ���ļ�����
	void openfile( void ) ;
	// �ر��ļ�FD
	void closefile( void ) ;
	// ���ڴ���־д���ļ�
	void dumpfile( void ) ;
	// �����־�����Ƿ񳬳�����
	void checkfile( void ) ;
	// ���ڴ�����д�����
	void writedisk( void ) ;

private:
	// ��¼��־�ĵ��ļ���
    string  		_file_name;
    // ��־�ļ���С
    long 			_log_size;
    // ����¼��־�ļ�����
    int 			_log_num;
    // ��־����
    int 			_log_level;
    // ���ļ����ļ�ID
    int 			_file_fd ;
    // ���һ�μ��ʱ��
    int 		    _check_time ;
    // ������־��
	share::Mutex 	_mutex;
	// ��ʱ�����̶߳���
	CLogThread     *_pthread ;
	// ��ǰ��־��¼���ݶ���
	LOGBLOCK		_LogBlock ;
	// ������־ͳ��
	string 			_run_name ;
};

#endif

