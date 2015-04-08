/**
 * author: humingqing
 * date:   2011-09-09
 * memo:   �Ự������������Ǽ򵥴���Ự��ʱ��Ӻ��Ƴ����ڲ����Ựʱ��
 * 	�������ƻص�������մ���֪ͨ����������ɾ��ʱ���ǻ�֪ͨ��紦��
 * 	����֪ͨ���ڼ���������У����������Ļص������в����ٵ���SESSION����ķ�������������������״̬
 * 	<2012-05-04> ���ڳ�ʱ��⣬����ʵ����һ����ʱ����������ʱ������ͷŵ�������ǰ�棬������ⳬʱֻ��Ҫ���ǰ��Ԫ���Ƿ�ʱ�����û�оͲ���Ҫ�ټ������
 */
#ifndef __SESSION_H__
#define __SESSION_H__

#include <time.h>
#include <map>
#include <string>
#include <Mutex.h>
#include <TQueue.h>
using namespace std ;

#define SESSION_CHECK_SPAN  30

// �Ự�仯֪ͨ����
#define SESSION_ADDED    0x01  // ��ӻỰ
#define SESSION_REMOVE   0x02  // ɾ���Ự

class ISessionNotify
{
public:
	// �Ự����ɾ��֪ͨ����
	virtual ~ISessionNotify() {}
	// �������֪ͨ
	virtual void NotifyChange( const char *key, const char *val , const int op ) = 0 ;
};

class CSessionMgr
{
protected:
	struct _Session
	{
		time_t    _time ;
		string    _key ;
		string    _value ;
		_Session *_next, *_pre ;
	};
public:
	CSessionMgr( bool timeout = false  ) ;
	~CSessionMgr() ;
	// ��ӻỰ
	void AddSession( const string &key, const string &val ) ;
	// ȡ�ûỰ�Ƿ����ʱ��
	bool GetSession( const string &key, string &val , bool update = true ) ;
	// �Ƴ��Ự
	void RemoveSession( const string &key ) ;
	// ��ⳬʱ����
	void CheckTimeOut( int timeout ) ;
	// ���ñ�ص�����
	void SetNotify( ISessionNotify *pNotify ) { _notfiy = pNotify ; }
	// ȡ�õ�ǰ�Ự��
	int  GetSize( void ) ;
	// �����ڴ�
	void RecycleAll( void ) ;

private:
	// �Ƴ�����
	void RemoveValue( _Session *p , bool clean ) ;
	// ��ӽڵ�����
	void AddValue( _Session *p ) ;
	// �Ƴ�����
	void RemoveIndex( const string &key ) ;

protected:
	share::Mutex  	   _mutex ;
	// �Ự�������
	typedef map<string,_Session*>  CMapSession;
	CMapSession   	   _mapSession ;
	// ���һ�μ��ʱ��
	time_t		  	   _last_check ;
	// �Ƿ���Ҫ��ʱ����
	bool 		  	   _btimeout ;
	// �Ự֪ͨ����
	ISessionNotify *   _notfiy ;
	// ���й������
	TQueue<_Session>   _queue ;
};

#endif
