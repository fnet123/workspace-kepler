/**
 * Author: humingqing
 * Date:   2011-07-24
 * memo:   ���ݻ������
 */
#ifndef __MSG_CACHE_H__
#define __MSG_CACHE_H__

#include "imsgcache.h"
#include <map>
#include <string>
#include <time.h>
#include <Mutex.h>
#include <TQueue.h>

// ��Ϣ����ģ��
class MsgCache : public IMsgCache
{
	// ���ݻ���ṹ��
	struct _msg_data
	{
		std::string key ;   // �����KEY
		int   		len ;   // ���ݳ���
		char *		buf ;   // ���ݵ�����
		time_t 		ent ;   // ������ݵ�ʱ��
		_msg_data *_next ;  // ��һ��ָ��
		_msg_data *_pre ;	// ǰһ��ָ��
	};
	// ��Ϣ���ݻ���
	typedef std::multimap<std::string,_msg_data*>  MapMsgData ;
public:
	MsgCache() ;
	~MsgCache() ;

	// �������
	bool AddData( const char *key, const char *buf, const int len ) ;
	// ȡ������
	char *GetData( const char *key, int &len, bool erase=true ) ;
	// �ͷ�����
	void FreeData( char *data ) ;
	// ����ʱ������
	void CheckData( int timeout ) ;
	// �Ƴ�����
	bool Remove( const char *key ) ;

private:
	// �����������
	void ClearAll( void ) ;

private:
	// ������߳�ʹ��
	share::Mutex      _mutex ;
	// ���ݻ������
	MapMsgData    	  _index ;
	// ʱ������
	TQueue<_msg_data> _queue;
};

#endif
