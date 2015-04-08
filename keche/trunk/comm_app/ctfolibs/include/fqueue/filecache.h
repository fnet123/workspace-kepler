/*
 * filecache.h
 *
 *  Created on: 2012-4-25
 *      Author: humingqing
 *
 *  ʵ�������ڷ��͹��̣�δ�ɹ����͵�MSG������MSG�����쳣ʱ�����ݽ��б��棬���ָ�����ʱ�����ݷ��ͻ�MSG
 */

#ifndef __FILECACHE_H__
#define __FILECACHE_H__

#include <map>
#include <set>
#include <list>
#include <Mutex.h>
#include <string>
#include <time.h>
using namespace std ;

#define IOHANDLE_FAILED  	-1   // ��������ʧ��
#define IOHANDLE_SUCCESS  	0	 // �ɹ�
#define IOHANDLE_ERRDATA 	1	 // ��Ч����

// �������ݻص�����
class IOHandler
{
public:
	// ����IO����
	virtual ~IOHandler() {} ;
	// �������ݵĻص�����
	virtual int HandleQueue( const char *sid , void *buf, int len , int msgid = 0 ) = 0 ;
};

class CFileQueue ;
// ���ݻ������
class CDataCache
{
	// �ļ�����ӳ��
	typedef map<string, CFileQueue*>  CMapQueue ;
public:
	CDataCache( IOHandler *handler ) ;
	~CDataCache() ;

	// ���õ�ǰĿ¼
	void Init( const char *szroot ) ;
	// �����û�����
	void Load( const string &sid ) ;
	// ���������
	bool Push( const string &sid , void *buf, int len ) ;
	// ��������
	int  Pop( const string &sid ) ;

private:
	// ����µ����ݶ���
	CFileQueue * AddNewQueue( const string &sid ) ;
	// ������������
	void Clear( void ) ;

private:
	// ���ݴ�����
	IOHandler  * _handler ;
	// �ļ�ӳ�����
	CMapQueue	 _queue ;
	// �ļ�������
	share::Mutex _mutex ;
	// ����ַ
	string 		_basedir ;
};

#define MAX_CHECK_TIME      30   // �����ʱ��
#define USER_ERR_NOMARL      0   // ����״̬
#define USER_ERR_EMPTY		-1   // �Ƿ�Ϊ��
#define USER_ERR_OVERFLUX   -2   // ��ʱ����״̬

// �ļ�CACHE���
class CFileCache
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
			string	  userid ; // ��ǰ���ݱ���ID
			time_t    now ;    // ���һ�β���ʱ��
			CFluxCtrl flux ;   // ÿ�����ݿ�һ�����ٿ���
			int 	  state ;
		};
		typedef list<_stUid>  ListUid ;
	public:
		CListUser() {};
		~CListUser(){};

		// ����û�
		void AddUser( const string &userid, unsigned int speed )
		{
			set< string >::iterator it = _setid.find( userid );
			if ( it != _setid.end() )
				return;

			_stUid uid;
			uid.userid = userid;
			uid.now    = time( NULL );
			uid.flux.SetSpeed( speed );
			uid.state  = USER_ERR_NOMARL;
			_list.push_back( uid );

			_setid.insert( set< string >::value_type( userid ) );
		}

		// ɾ���û�
		void DelUser( const string &userid )
		{
			_setid.erase( userid );

			if ( _list.empty() )
				return;

			list< _stUid >::iterator it;
			for ( it = _list.begin(); it != _list.end() ; ++ it ) {
				_stUid &uid = ( * it );
				if ( uid.userid != userid ) {
					continue;
				}
				_list.erase( it );
				break;
			}
		}

		// ɾ��SET������
		void DelSet( const string &userid ){ _setid.erase( userid ) ; }
		// ���ص�ǰ�û��б�
		ListUid  &ListUser( void ){ return _list ;}

	private:
		// �����û�����
		ListUid 	 _list ;
		// �û�ID������
		set<string>  _setid ;
	};

public:
	CFileCache(IOHandler *handler) ;
	~CFileCache() ;

	// ��ʼ��ϵͳ
	bool Init( const char *dir, int maxsend = 0 ) ;
	// ��������
	bool WriteCache( const char *sid, void *buf, int len ) ;
	// �����û�
	void Online( const char *sid ) ;
	// �����û�
	void Offline( const char *sid ) ;
	// �߳����ж���
	bool Check( void ) ;

private:
	// �����û�����
	CListUser			 _userlst;
	// �������ݵ�IO����
	IOHandler			*_handler ;
	// ���ݻ������
	CDataCache			 _datacache ;
	// ÿ���û��������
	unsigned int 		 _maxsend;
	// ͬ�������û���
	share::Mutex 		 _mutex ;
};

#endif /* FILECACHE_H_ */
