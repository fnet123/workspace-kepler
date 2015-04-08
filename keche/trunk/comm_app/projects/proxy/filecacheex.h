/*
 * filecache.h
 *
 *  Created on: 2011-12-21
 *      Author: humingqing
 *  ���ݻ������
 */

#ifndef __FILECACHEEX_H__
#define __FILECACHEEX_H__

#include "interface.h"
#include <Mutex.h>
#include <Thread.h>
#include <filecache.h>

#define DATA_FILECACHE   	0  // ���ݻ���
#define DATA_ARCOSSDAT	 	1  // �������

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

#define MAX_CHECK_TIME   	30    // �����ʱ��
#define USER_ERR_NOMARL      0    // ����״̬
#define USER_ERR_EMPTY		-1    // �Ƿ�Ϊ��
#define USER_ERR_OVERFLUX   -2    // ��ʱ����״̬

// �ļ�CACHE���
class CFileCacheEx : public share::Runnable
{
	// �����û�����
	class CListUser
	{
		// ���ٿ���
		class CFluxCtrl
		{
		public:
			CFluxCtrl() {
				_maxsend  = 0 ;
				_curcount = 0 ;
				_curtime  = time(NULL) ;
			};
			~CFluxCtrl() {}

			// ����Ƿ񳬳�����
			bool OverSpeed( void ) {
				if ( _maxsend <= 0 )
					return false ;

				time_t now = time(NULL) ;
				if ( _curtime != now ) {
					_curcount = 0 ;
					_curtime  = now ;
				}
				++ _curcount ;
				return ( _curcount > _maxsend ) ;
			}

			// ���÷����ٶ�
			void SetSpeed( unsigned int speed ){
				_maxsend = speed ;
			}

		private:
			// ������ٶ�
			unsigned int _maxsend  ;
			// ��ǰ���͸���
			unsigned int _curcount ;
			// ��ǰʱ��
			time_t		 _curtime  ;
		};

	public:
		struct _stUid
		{
			int	      id ; 	   // ��ǰ���ݱ���ID
			time_t    now ;    // ���һ�β���ʱ��
			CFluxCtrl flux ;   // ÿ�����ݿ�һ�����ٿ���
			int 	  state ;
		};
		typedef list<_stUid>  ListUid ;
	public:
		CListUser() {};
		~CListUser(){};

		// ����û�
		void AddUser( const int id, unsigned int speed = 1000 )
		{
			set< int >::iterator it = _setid.find( id );
			if ( it != _setid.end() )
				return;

			_stUid uid;
			uid.id 	= id;
			uid.now = time( NULL );
			uid.flux.SetSpeed( speed );
			uid.state  = USER_ERR_NOMARL;
			_list.push_back( uid );

			_setid.insert( set< int >::value_type( id ) );
		}

		// ɾ���û�
		void DelUser( const int id )
		{
			_setid.erase( id );

			if ( _list.empty() )
				return;

			list< _stUid >::iterator it;
			for ( it = _list.begin(); it != _list.end() ; ++ it ) {
				_stUid &uid = ( * it );
				if ( uid.id != id ) {
					continue;
				}
				_list.erase( it );
				break;
			}
		}

		// ɾ��SET������
		void DelSet( const int id ){ _setid.erase( id ) ; }
		// ���ص�ǰ�û��б�
		ListUid  &ListUser( void ){ return _list ;}

	private:
		// �����û�����
		ListUid 	 _list ;
		// �û�ID������
		set<int>  	 _setid ;
	};

public:
	CFileCacheEx(IOHandler *handler) ;
	~CFileCacheEx() ;

	// ��ʼ��ϵͳ
	bool Init( ISystemEnv *pEnv , const char *name ) ;
	// ����ϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// ��������
	bool WriteCache( int id, void *buf, int len ) ;
	// ��ӿ�����Ϣ
	void AddArcoss( int id, char color, char *vechile ) ;
	// ɾ��������Ϣ
	void DelArcoss( int id, char color, char *vechile ) ;
	// �޸Ŀ���ʱ��
	void ChgArcoss( int id, char color, char *vechile ) ;
	// �����û�
	void Online( int id ) ;
	// �����û�
	void Offline( int id ) ;

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
	// ���������ٶ�
	int					 _sendspeed;
	// �û�����ͬ��������
	share::Mutex	     _mutex ;
};

#endif /* FILECACHE_H_ */
