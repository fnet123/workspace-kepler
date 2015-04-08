/**
 * Author: humingqing
 * Date:   2011-07-24
 * memo:   ���ݻ������
 */
#ifndef __MSG_CACHE_H__
#define __MSG_CACHE_H__

#include "interface.h"
#include <map>
#include <string>
#include <time.h>
#include <Mutex.h>

// ��Ϣ����ģ��
class MsgCache : public IMsgCache
{
	// ���ݻ���ṹ��
	struct _msg_data
	{
		int   len ;   // ���ݳ���
		char *buf ;   // ���ݵ�����
		time_t ent ;  // ������ݵ�ʱ��
	};
	// ��Ϣ���ݻ���
	typedef std::multimap<std::string,_msg_data*>  MapMsgData ;
	// ��Ϣ��ʱ������
	typedef std::multimap<time_t,std::string> MapMsgIndex;
public:
	MsgCache() ;
	~MsgCache() ;

	// �������
	bool AddData( const char *key, const char *buf, const int len ) ;
	// ȡ������
	char *GetData( const char *key, int &len ) ;
	// �ͷ�����
	void FreeData( char *data ) ;
	// ����ʱ������
	void CheckData( int timeout ) ;
	// �Ƴ�����
	bool Remove( const char *key ) ;

private:
	// �������
	bool RemoveData( const char *key ) ;
	// �����������
	void ClearAll( void ) ;

private:
	// ������߳�ʹ��
	share::Mutex  _mutex ;
	// ���ݻ������
	MapMsgData    _map_data ;
	// ʱ������
	MapMsgIndex   _map_index ;
};

#endif
