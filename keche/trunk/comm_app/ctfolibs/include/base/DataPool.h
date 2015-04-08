/***********************************************************************
 ** Copyright (c)2009,����ǧ���Ƽ��������޹�˾
 ** All rights reserved.
 ** 
 ** File name  : DataPool.h
 ** Author     : lizp (lizp.net@gmail.com)
 ** Date       : 2010-1-3 ���� 05:02:25
 ** Comments   : ��תվ�����ClassType��Ϣ
 **
 ** 2011-09-14 humingqing �޸Ļ���ض������ʱ������������ʱ���ݣ�
 ** ����MAP�Զ������ܣ��������Լ���MAP�ı����������Ͼ�MAP�ı�����Ч���Ǻܵ͵�
 ***********************************************************************/

#ifndef _DATA_POOL_H_
#define _DATA_POOL_H_

#include <list>
#include <string>
#include <errno.h>
#include <comlog.h>
#include <tools.h>
#include <Monitor.h>
#include <map>

#define max_data_pool_num 10*10000

// ���ݶ���
template<class ClassType>
class CDataPool
{
/**
 * <humingqing 2012.07.10>
 * ����ʹ�����Ѽ�����ԭ���� ��Ҫ�������ܲ����з���list��size()����Ч�ʺܵͣ�û��ͨ����������������Ч�ʸ�
 * ���߳�ʹ�� getDataʱ���п���ȡ������Ԫ��Ϊ�յ������������Ҫ�жϵ�ȡ����Ԫ���Ƿ���Ч����Ҫԭ������Ϊ�������ò������������
 */
protected:
	share::Monitor  _monitor ;
	share::Mutex    _mutex ;
	list<ClassType> _datapool ;
	unsigned int    _maxsize ;
	unsigned int    _cursize ;

public:
	CDataPool( void ):_maxsize(max_data_pool_num)
	{
		_cursize = 0 ;
	}
	CDataPool( int size ): _maxsize(size)
	{
		_cursize = 0 ;
	}
	~CDataPool(){}

	/**
	 *  ��ȡ���ݳ��е����ݡ�����ʽ����,Ϊʲô��Ҫ�������ý�ȥ�����������Ļ���û�����г�ʼ��������ͨ������ֵ���ж��Ƿ����ȥ�����ݳɹ���
	 */
	ClassType getData(ClassType &element, bool block = true )
	{
		// ������������ͣ�û�����ݾ�ֱ��������
		if ( block && empty() ) {
			_monitor.lock() ;
			_monitor.wait() ;
			_monitor.unlock() ;
		}

		_mutex.lock() ;
		//�п��ܳ��ֿ�ֵ�������������������Ժ�---->�ᴥ��sem_post��ʱ�����������sempost���¼�����semwait
		if ( _cursize > 0 )
		{
			element = (ClassType)_datapool.front();
			_datapool.pop_front();
			-- _cursize ;
		}
		_mutex.unlock() ;

		return element;
	}

	/**
	 *  �����ݳ���������ݡ�
	 */
	bool addData(ClassType& rd,int flag = 1)
	{
		_mutex.lock() ;

		if ( _cursize > _maxsize )
		{
			ClassType element = _datapool.front();
			_datapool.pop_front();
			if(flag) delete element;
			-- _cursize ;
		}
		_datapool.push_back(rd);
		++ _cursize ;

		_mutex.unlock() ;

		_monitor.notify() ;

		return true;
	}

	/**
	 * ���ݶ����Ƿ�Ϊ��
	 */
	bool empty()
	{
		share::Guard g(_mutex) ;
		{
			return ( _cursize == 0 ) ;
		}
	}
	// ȡ�ó������ֵ
	int get_max_size(){ return _maxsize; }
	// δ����������������ͬ�������ǻ��е�Ӱ��
	int get_cur_size(){ return _cursize; }

	// ��ֹ��������
	void notifyend() {
		// ��������ʱ��Ҫ����һ���źŹ�ȥ
		_monitor.notifyEnd() ;
	}
};
#pragma pack(1)

typedef struct _CacheData
{
	string user_id; //��¼�û���ID�����Ƿ�����Ϣ�е�mac_id;
	string str_send_msg;
	string timeout_msg;

	string seq;
	string mac_id;
	string command;
	string company_id;

	char *send_data;
	int send_data_len;

	time_t send_time;

	_CacheData()
	{
		send_data 		= NULL;
		send_data_len   = 0;
		send_time 		= time(0);
	}
} CacheData;
#pragma pack()

// �����
class CacheDataPool
{
public:
	CacheDataPool(){}
	~CacheDataPool(){}

	// ��ӻ�������
	void add( const string &req, const CacheData &data)
	{
		share::Guard g( _mutex ) ;
		{
			_cache[req] = data;
			// ���ʱ������
			_index.insert( pair<time_t,string>(data.send_time, req) ) ;
		}
	}

	// ǩ����������
	CacheData checkData( const string &req)
	{
		share::Guard g( _mutex ) ;
		{
			CacheData d;
			map<string, CacheData>::iterator p = _cache.find(req);
			if (p != _cache.end())
			{
				d = (CacheData) p->second;
				_cache.erase(req);
			}
			// �������
			RemoveIndex( d.send_time, req ) ;

			return d;
		}
	}

	// ����ʱ����
	bool timeoutData( int seconds , list<CacheData> &lst )
	{
		share::Guard g( _mutex ) ;
		{
			if ( _index.empty() )
				return false ;

			time_t now = time(0) - seconds ;

			map<string,CacheData>::iterator itx ;
			multimap<time_t,string>::iterator it ;
			for ( it = _index.begin(); it != _index.end(); ){
				if ( it->first > now ) {
					break ;
				}
				// ��������
				itx = _cache.find( it->second ) ;
				if ( itx == _cache.end() ) {
					_index.erase( it ++ ) ;
					continue ;
				}
				// ��ӳ�ʱ������
				lst.push_back( itx->second ) ;

				_cache.erase( itx ) ;
				_index.erase( it ++ ) ;
			}

			return ( ! lst.empty() ) ;
		}
	}

private:
	// �Ƴ�����
	void RemoveIndex( time_t t , const string &seq )
	{
		multimap<time_t,string>::iterator it = _index.find( t ) ;
		if ( it == _index.end() ){
			return ;
		}

		for ( ; it != _index.end(); ++ it ){
			if ( it->first != t ) {
				break ;
			}
			if ( it->second == seq ){
				_index.erase( it ) ;
				break ;
			}
		}
	}

protected:
	share::Mutex           	_mutex ;
	// ����MAP
	map<string, CacheData> 	_cache ;
	// ��������
	multimap<time_t,string> _index ;
};

#endif

