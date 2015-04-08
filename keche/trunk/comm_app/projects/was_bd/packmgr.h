/**
 * Author: humingqing
 * Date:   2011-10-14
 * Memo:   ���ְ������ݰ��ϳ�һ�����ݰ������������Ϊһ�����������Ի��е��������
 */
#ifndef __PACKMGR_H__
#define __PACKMGR_H__

#include <time.h>
#include <map>
#include <string>
#include <vector>
#include <databuffer.h>
#include <Mutex.h>
#include <TQueue.h>
using namespace std ;

#define LONG_PACK_CHECK    30
#define LONG_PACK_TIMEOUT  180  // 3���������û������ϴ��Ͷ���

class CPackMgr
{
	class CMemFile
	{
	public:
		CMemFile( const char *id ) ;
		~CMemFile() ;

		// ����ڴ�����
		bool    AddBuffer( DataBuffer &pack, const int index, const int count, const char *buf, int len ) ;
		// ȡ�����һ��ʹ��ʱ��
		time_t  GetLastTime( void ) { return _last ; }
		// ȡ�ö�Ӧ���ڴ�ID��
		const char *GetId( void ) { return _id.c_str(); }

	private:
		// ��ǰ�������
		unsigned int 			_cur ;
		// �ڴ��ļ�����
		vector<DataBuffer *>    _vec ;
		// ���һ��ʱ��
		time_t 					_last ;
		// ������ID��
		std::string 			_id ;

	public:
		// �ڴ��ļ���������ָ��
		CMemFile *				_next ;
		CMemFile *				_pre ;
	};

	typedef map<string,CMemFile*> 	CMapFile ;
public:
	CPackMgr() ;
	~CPackMgr() ;
	// ��ӵ����ݰ�����
	bool AddPack( DataBuffer &pack, const char *carid, const int msgid, const int index, const int count, const int seq, const char *buf, int len ) ;
	// ����ʱ�����ݰ�
	void CheckTimeOut( unsigned int timeout ) ;

private:
	// �Ƴ����ݶ���
	void RemovePack( const string &key ) ;

private:
	// ������Դ��
	share::Mutex 	 _mutex ;
	// ����ڴ�����
	CMapFile 	 	 _mapPack ;
	// ���һ�μ��ʱ��
	time_t		 	 _lastcheck ;
	// ���ݶ���
	TQueue<CMemFile> _queue ;
};

#endif
