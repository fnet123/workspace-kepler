/*
 * tester.h
 *
 *  Created on: 2012-9-7
 *      Author: think
 */

#ifndef TESTER_H_
#define TESTER_H_

#include <time.h>
#include <map>
#include <list>
#include <string>
#include <Monitor.h>
#include <Mutex.h>
#include <Thread.h>
#include <dataqueue.h>
#include <queuethread.h>
#include <sys/time.h>
typedef long long int64 ;

using namespace std ;

#define TV_USEC  10000000

static int64 gettime64( void )
{
	struct timeval val ;
	gettimeofday( &val, NULL ) ;
	return val.tv_sec * TV_USEC +  val.tv_usec ;
}

#define MAX_WORK_THREAD   4
#define max_data_pool_num 1000*10000
// ���ݶ���
template<class ClassType>
class CTestPool
{
protected:
	share::Monitor  _monitor ;
#ifdef _MUTIL_MUTEX
	share::Mutex    _mutex ;
#endif
	list<ClassType> _datapool;
	unsigned int 	_size ;

public:
	CTestPool( void ){ _size = 0 ;}
	~CTestPool(){}

	/**
	 *  ��ȡ���ݳ��е����ݡ�����ʽ����,Ϊʲô��Ҫ�������ý�ȥ�����������Ļ���û�����г�ʼ��������ͨ������ֵ���ж��Ƿ����ȥ�����ݳɹ���
	 */
	bool getData(ClassType &element, bool block = true )
	{
		share::Synchronized s(_monitor) ;
		{
			// ������������ͣ�û�����ݾ�ֱ��������
#ifdef _MUTIL_MUTEX
			if ( block && empty() )
#else
			if ( block && _size == 0 )
#endif
				_monitor.wait() ;

#ifdef _MUTIL_MUTEX
			_mutex.lock() ;
#endif
			//�п��ܳ��ֿ�ֵ�������������������Ժ�---->�ᴥ��sem_post��ʱ�����������sempost���¼�����semwait
			if ( _size > 0 )
			{
				element = (ClassType)_datapool.front();
				_datapool.pop_front();

				-- _size ;
#ifdef _MUTIL_MUTEX
				_mutex.unlock() ;
#endif
				return true ;
			}
#ifdef _MUTIL_MUTEX
			_mutex.unlock() ;
#endif
			return false ;
		}
	}

	/**
	 *  �����ݳ���������ݡ�
	 */
	bool addData(ClassType& rd,int flag = 1)
	{
#ifdef _MUTIL_MUTEX
		_mutex.lock() ;
#else
		_monitor.lock() ;
#endif
		if ( _size > max_data_pool_num )
		{
			ClassType element = _datapool.front();
			_datapool.pop_front();
			//if(flag) delete element;
			-- _size ;
		}
		_datapool.push_back(rd);

		++ _size ;

#ifdef _MUTIL_MUTEX
		_mutex.unlock() ;
#else
		_monitor.unlock() ;
#endif
		_monitor.notify() ;

		return true;
	}

	bool empty()
	{
#ifdef _MUTIL_MUTEX
		share::Guard g(_mutex) ;
#else
		share::Synchronized s(_monitor) ;
#endif
		return ( _size == 0 ) ;
	}

	// ��ֹ��������
	void notifyend() {
		// ��������ʱ��Ҫ����һ���źŹ�ȥ
		_monitor.notifyEnd() ;
	}
};

// ���������߳�
class CTestThread : public share::Runnable
{
public:
	CTestThread() { _count = 0; }
	~CTestThread() { stop() ; }

	void start( void ) {
		_thread.init( MAX_WORK_THREAD, NULL, this ) ;
		_thread.start() ;
		_start = gettime64() ;
		_stop  = false ;
	}

	void stop( void ) {
		_stop = true ;
		_thread.stop() ;
	}

	void add( int i ) {
		_testpool.addData(i) ;
	}

	void run( void *param ) {
		while( ! _stop ) {
			int n = 0;
			if ( _testpool.getData( n ) ) {
				_mutex.lock() ;
				++ _count ;
				_mutex.unlock() ;
			}
			if ( _count == max_data_pool_num ) {
				int64 nend = gettime64() ;
				printf( "datapool read span time: %f\n", (double)(nend - _start)/(double)TV_USEC ) ;
				_testpool.notifyend() ;
				break ;
			}
		}
	}

	int count( void ) {
		return _count ;
	}

private:
	CTestPool<int> 		 _testpool ;
	share::ThreadManager _thread ;
	int64				 _start ;
	bool 				 _stop ;
	share::Mutex		 _mutex ;
	int 				 _count ;
};

// �����̶߳��е�ģʽ
class CQueueTest:  public IQueueHandler
{
	struct _key
	{
		int _n ;
		_key *_next ;
	};
public:
	CQueueTest(){
		_count = 0 ;
		_queue = new CDataQueue<_key>(max_data_pool_num) ;
		_qthread = new CQueueThread(_queue, this) ;
	}
	~CQueueTest(){
		delete _qthread ;
		delete _queue ;
	}

	void start( void ) {
		_qthread->Init( MAX_WORK_THREAD ) ;
		_start = gettime64() ;
	}

	void stop( void ) {
		_qthread->Stop() ;
	}

	void add( int n ) {
		_key *p = new _key ;
		if ( p == NULL )
			return ;

		p->_n = n ;
		// �������ݶ�����
		if ( ! _qthread->Push( p ) ) {
			delete p ;
		}
	}

	int count( void ) {
		return _count ;
	}

	// �߳�ִ�ж��󷽷�
	void HandleQueue( void *packet ){
		_mutex.lock() ;
		++ _count ;
		_mutex.unlock() ;

		_key *p = ( _key *)packet ;
		if ( _count == max_data_pool_num ) {
			int64 nend = gettime64() ;
			printf( "queue thread read span time: %f\n", (double)(nend - _start)/(double)TV_USEC ) ;
		}
	}

private:
	// ���ݷ�������
	CDataQueue<_key> *_queue ;
	// �����̶߳���
	CQueueThread 	 *_qthread;
	// ��ʼʱ��
	int64			  _start ;

	share::Mutex	  _mutex ;
	int				  _count ;
};

typedef struct _StrInfo
{
    const char *pos;
    int offset;
}StrInfo;

//��str split��list����ʽ�� StrInfoֻ��ָ��ÿһ���ֶε�λ�ú�ƫ�����������޸�ԭʼ���ݵ�ֵ��
static bool split2list(const char *data, int data_len, std::list<StrInfo> &list,  const char *split)
{
	int dlen = data_len;
    int slen = strlen(split);
    int offset = 0;

    const  char *pos = strstr(data + offset, split);
    if(pos == NULL)
    	return false;

    StrInfo str_info;
    //������һ��split.
    while(offset < dlen && (NULL != (pos = strstr(data + offset, split))))
    {
    	str_info.pos = data + offset;
    	str_info.offset = pos - data - offset;
        offset += (pos - data - offset + slen);
        list.push_back(str_info);
    }

    str_info.pos = data + offset;
    str_info.offset =  dlen - offset;
    list.push_back(str_info);

    return true;
}

/********************************
 * Input: StrInfo �ָ�����ֶ�Ϊ  "key:value" ���ַ�����ʽ��
 * Output: �����map, map�е�string string Ϊ�¹���ģ�
 *****************************/
static int split2map(std::list<StrInfo> &list, map<string , string> &kv_map, const char *split)
{
	kv_map.clear();
    int ret = 0;
    int slen = strlen(split);
    //	int count = 0;
	map<string, string>::iterator it;
	std::list<StrInfo>::iterator iter = list.begin();

	const char *pos = NULL;
	for (; iter != list.end(); ++iter)
	{
		if ((pos = strstr(iter->pos, split)) == NULL)
			continue;
		string key(iter->pos, pos - iter->pos);
		string value(pos + slen, iter->offset - (pos - iter->pos) - slen);
		it = kv_map.find(key);
		if (it != kv_map.end())
		{ // ������ڶ��ͬ�����д���
			it->second += "|";
			it->second += value;
		}
		else
		{
			kv_map.insert(make_pair(key, value));
			ret ++;
		}
	}
	return ret;
}

// ����ַ�����key-value��MAP��
static int split2kvmap( const char *src, map<string,string> &kv, const char *s1 , const char *s2 )
{
	kv.clear() ;

	int n1  = strlen(s1) ;
	int n2  = strlen(s2) ;

	map<string,string>::iterator it ;

	char *p = (char *)src ;
	char *q = strstr( p, s1 ) ;

	size_t pos ;
	string key, val ;

	int ret = 0 ;

	while( q != NULL ) {
		if ( q > p ) {
			val.assign( p, q-p ) ;
			pos = val.find( s2 ) ;

			if ( pos != string::npos ) {
				key = val.substr( 0, pos ) ;
				val = val.substr( pos + n2 ) ;

				it = kv.find( key ) ;
				if ( it != kv.end() ) {
					it->second += "|";
					it->second += val;
				} else {
					kv.insert( make_pair(key, val ) ) ;

					++ ret ;
				}
			}
		}
		p = q + n1 ;
		q = strstr( p, s1 ) ;
	}

	if ( p != NULL ) {
		val = p ;
		pos = val.find( s2 ) ;

		if ( pos != string::npos ) {
			key = val.substr( 0, pos ) ;
			val = val.substr( pos + n2 ) ;

			it = kv.find( key ) ;
			if ( it != kv.end() ) {
				it->second += "|";
				it->second += val;
			} else {
				kv.insert( make_pair(key, val ) ) ;

				++ ret ;
			}
		}
	}
	return ret ;
}


#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

/* interestingly, we assume leap-years */
static int month[12] = {
    0,
    DAY*(31),
    DAY*(31+29),
    DAY*(31+29+31),
    DAY*(31+29+31+30),
    DAY*(31+29+31+30+31),
    DAY*(31+29+31+30+31+30),
    DAY*(31+29+31+30+31+30+31),
    DAY*(31+29+31+30+31+30+31+31),
    DAY*(31+29+31+30+31+30+31+31+30),
    DAY*(31+29+31+30+31+30+31+31+30+31),
    DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

inline long kernel_mktime(struct tm * tm)
{
    long res;
    int year;

    year = tm->tm_year - 70;

/* magic offsets (y+1) needed to get leapyears right.*/

    res = YEAR*year + DAY*((year+1)/4);
    res += month[tm->tm_mon];

/* and (y+2) here. If it wasn't a leap-year, we have to adjust */

    if (tm->tm_mon>1 && ((year+2)%4))
        res -= DAY;
    res += DAY*(tm->tm_mday-1);
    res += HOUR*tm->tm_hour;
    res += MINUTE*tm->tm_min;
    res += tm->tm_sec;

    //��ȥ�˸�Сʱ��ʱ��
    return res - (8 * 3600);
}


static time_t ConvertUtcTime( string str_time )
{
	//20120429/233607
	if ( str_time.length() != 15 ) {
		return 0;
	}

	char buffer[8] = { 0 };
	const char *char_time = str_time.c_str();
	struct tm utc_time;
	strncpy( buffer, & char_time[0], 4 );
	utc_time.tm_year = atoi( buffer ) - 1900;
	memset( buffer, 0, sizeof ( buffer ) );
	strncpy( buffer, & char_time[4], 2 );
	utc_time.tm_mon = atoi( buffer ) - 1;
	strncpy( buffer, & char_time[6], 2 );
	utc_time.tm_mday = atoi( buffer );
	strncpy( buffer, & char_time[9], 2 );
	utc_time.tm_hour = atoi( buffer );
	strncpy( buffer, & char_time[11], 2 );
	utc_time.tm_min = atoi( buffer );
	strncpy( buffer, & char_time[13], 2 );
	utc_time.tm_sec = atoi( buffer );

	return kernel_mktime( & utc_time );
}

// �ƶ��ļ�
static bool copyfile( const char *root, const char *dest, const char *name )
{
	// 20120531165541-4C54_15910604518-23-0-1-0-0.jpeg
	char *p = strrchr( name, '/' ) ;
	if ( p == NULL || p == name )
		return false ;

	// ȡ��·��
	char path[256] = {0} ;
	memcpy( path, name, p - name ) ;

	// ȡ��PATH
	char file[256] = {0} ;
	strcpy( file, p + 1 ) ;

	int len = strlen(file) ;
	if ( len < 30 )  // �������򲻺Ϸ��ݲ�����ˮӡ
		return false ;

	char sztime[1024] = {0} ;
	sztime[0]  = file[0] ;
	sztime[1]  = file[1] ;
	sztime[2]  = file[2] ;
	sztime[3]  = file[3] ;
	sztime[4]  = '.' ;
	sztime[5]  = file[4] ;
	sztime[6]  = file[5] ;
	sztime[7]  = '.' ;
	sztime[8]  = file[6] ;
	sztime[9]  = file[7] ;
	sztime[10] = ' ' ;
	sztime[11] = file[8] ;
	sztime[12] = file[9] ;
	sztime[13] = ':' ;
	sztime[14] = file[10] ;
	sztime[15] = file[11] ;
	sztime[16] = ':' ;
	sztime[17] = file[12] ;
	sztime[18] = file[13] ;
	sztime[19] = ' ' ;
	sztime[20] = '#' ;

	int b = 0 ,e = 0 ,c = 0 ;
	int i = len-1 ;
	while( i > 0 ) {
		if ( file[i] == '-' ) {
			++ c ;
			if ( c == 2 ) e = i ;
			if ( c == 3 ) {
				b = i+1 ;
				break ;
			}
		}
		-- i ;
	}
	// ȡ��ͨ��IDֵ
	for ( i = b; i < e; ++ i ) {
		sztime[21+i-b] = file[i] ; // ȡ��ͨ��IDֵ
	}

	printf( "%s\n", sztime ) ;
}


#endif /* TESTER_H_ */
