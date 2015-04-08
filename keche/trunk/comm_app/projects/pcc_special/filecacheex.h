/*
 * filecache.h
 *
 *  Created on: 2011-12-21
 *      Author: Administrator
 */

#ifndef __FILECACHEEX_H__
#define __FILECACHEEX_H__

#include "interface.h"
#include <Thread.h>
#include <filecache.h>

#define VECHILE_LEN   21
#define VECHILE_OFF   0
#define VECHILE_ON    1

struct ArcossData
{
	int     areacode ;
	char    vechile[VECHILE_LEN] ;
	char    color ;
	uint64_t time  ;
	char    state ; // �Ƿ����״̬
	ArcossData *_next ;
	ArcossData *_pre  ;
};

class IOHandler ;
// ���򲹷�������
class CArcossCache
{
	struct ArcossHeader
	{
		int 		_size ;
		ArcossData *_head ;
		ArcossData *_tail ;
	};
	// ���ݴ�Žṹ��Ԫ
	typedef map<int,ArcossHeader *>  CMapArcoss;
	// ������ŵ�MAP
	typedef map<string,ArcossData *> CMapIndex ;
public:
	CArcossCache( IOHandler *handler ) ;
	~CArcossCache() ;
	// �����ļ�·��
	void Init( const char *szroot ) ;
	// ���߼�������
	void Load( int id ) ;
	// ���������
	void Push( int id, char color, char *vechile ) ;
	// ����������
	void Pop( int id, char color, char *vechile ) ;
	// �޸�����ʱ��
	void Change( int id, char color, char *vechile ) ;
	// ���������
	void Offline( int id ) ;
	// �������
	bool Check( int id ) ;

private:
	// ���л�����
	void serialize( int id ) ;
	// ���������
	bool AddNew( int id, char color, char *vechile ) ;
	// �Ƴ�����
	bool Remove( int id, char color, char *vechile ) ;
	// �޸Ĳ���
	bool ChangeEx( int id, char color, char *vechile ) ;
	// �����л�����
	void unserialize( int id ) ;
	// ɾ���ļ�
	void deletefile( int id ) ;
	// ���������
	bool AddData( int id, char color, char *vechile , uint64_t time , char state ) ;
	// �Ƴ�����
	ArcossData *RemoveIndex( int id, char color , char *vechile ) ;
	// ��������
	void Clear( void ) ;
	// �������Ҫ֪ͨ
	bool CheckEx( int id ) ;

private:
	// ���ݴ�����
	IOHandler  *  _handler ;
	// ����ӳ��
	CMapArcoss    _mapArcoss;
	// ��������
	CMapIndex     _mapIndex ;
	// ͬ������������
	share::Mutex  _mutex ;
	// �����ĸ�Ŀ¼
	string 		  _basedir ;
};

#define MAX_CHECK_TIME   30  // �����ʱ��
// �ļ�CACHE���
class CFileCacheEx : public share::Runnable
{
	// �����û�����
	class CListUser
	{
		struct _stUid
		{
			int 	areacode ;
			time_t  now ;
		};
	public:
		CListUser() {};
		~CListUser(){};

		// ����û�
		void AddUser( int areacode ) {
			share::Guard guard( _mutex ) ;
			_stUid uid ;
			uid.areacode = areacode ;
			uid.now		 = time(NULL) ;
			_list.push_back( uid ) ;
		}
		// ɾ���û�
		void DelUser( int areacode ) {
			share::Guard guard( _mutex ) ;
			if ( _list.empty() )
				return ;

			list<_stUid>::iterator it ;
			for ( it = _list.begin(); it != _list.end(); ++ it ) {
				_stUid &uid = (*it) ;
				if ( uid.areacode != areacode ) {
					continue ;
				}
				_list.erase( it ) ;
				break ;
			}
		}
		// �����û�
		bool PopUser( int &areacode ) {
			share::Guard guard( _mutex ) ;
			if ( _list.empty() )
				return false ;

			size_t cnt   = 0 ;
			size_t count = _list.size() ;
			time_t now   = time(NULL) ;

			areacode = -1 ;

			while( cnt < count ) {
				_stUid uid = _list.front() ;
				_list.pop_front() ;
				_list.push_back( uid ) ;

				if ( now - uid.now > MAX_CHECK_TIME ) {
					areacode = uid.areacode ;
					break ;
				}
				++ cnt ;
			}
			return ( areacode > 0 ) ;
		}

	private:
		// �û�������
		share::Mutex _mutex;
		// �����û�����
		list<_stUid> _list ;
	};


public:
	CFileCacheEx(IOHandler *handler) ;
	~CFileCacheEx() ;

	// ��ʼ��ϵͳ
	bool Init( ISystemEnv *pEnv ) ;
	// ����ϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// ��������
	bool WriteCache( int areacode, void *buf, int len ) ;
	// ��ӿ�����Ϣ
	void AddArcoss( int areacode, char color, char *vechile ) ;
	// ɾ��������Ϣ
	void DelArcoss( int areacode, char color, char *vechile ) ;
	// �޸Ŀ���ʱ��
	void ChgArcoss( int areacode, char color, char *vechile ) ;
	// �����û�
	void Online( int areacode ) ;
	// �����û�
	void Offline( int areacode ) ;

public:
	// �߳����ж���
	virtual void run( void *param ) ;

private:
	// ��������ָ��
	ISystemEnv 			*_pEnv ;
	// �����̶߳���
	share::ThreadManager _thread;
	// �����û�����
	CListUser			 _userlst;
	// �������ݵ�IO����
	IOHandler			*_handler ;
	// �Ƿ������߳�
	bool 				 _inited ;
	// ���ݻ������
	CDataCache			 _datacache ;
	// ���򻺴����
	CArcossCache		 _arcosscache ;
};

#endif /* FILECACHE_H_ */
