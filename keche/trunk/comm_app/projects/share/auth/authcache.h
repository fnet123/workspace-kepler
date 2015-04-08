/*
 * authcache.h
 *
 *  Created on: 2012-5-14
 *      Author: humingqing
 *
 *  memo: ǰ�û���Ȩ���棬��Ҫ��ǰ�û����ݽ��г־û���������ͬ���������ػ���
 */

#ifndef __AUTHCACHE_H__
#define __AUTHCACHE_H__

#include <map>
#include <string>
#include <qstring.h>
#include <Mutex.h>
#include <TQueue.h>

#define AUTH_ERR_SUCCESS 	0   // ���سɹ�
#define AUTH_ERR_FAILED 	-1  // ����ʧ��
#define AUTH_ERR_TIMEOUT    -2  // ���س�ʱ

class CAuthCache
{
	struct _stAuth
	{
		char phone[12];
		char oem[8] ;
		char authcode[20] ;
		time_t time ;
		_stAuth *_next ;
		_stAuth *_pre ;
	};
	typedef std::map<std::string,_stAuth *>  CMapAuth ;
public:
	CAuthCache() ;
	~CAuthCache() ;
	// ���ؼ�Ȩע���ļ�
	bool LoadFile( const char *filename ) ;
	// ��������Ȩ
	int  TermAuth( const char *phone, const char *auth, CQString &ome , time_t n = 0 ) ;
	// �����Ȩ��Ϣ
	bool AddAuth( const char *oem, const char *phone, const char *authcode ) ;
	// �Ƴ�����
	void Remove( const char *phone ) ;
	// �ӻ�����ȡ������
	bool GetCache( const char *phone, CQString &ome ) ;
	// ��ʱ�������л�����
	void Check( int timeout ) ;

private:
	// ��������
	void Clear( void ) ;
	// ���л�����
	bool Serialize( void ) ;
	// �����л�����
	bool UnSerialize( void ) ;

private:
	// ͬ������������
	share::Mutex    _mutex ;
	// MAPӳ�䴦��
	CMapAuth 	    _mpAuth ;
	// ���л����ļ���
	CQString        _filename ;
	// ��Ȩ���ݶ���
	TQueue<_stAuth> _queue ;
	// ���һ�α����ʱ��
	time_t 		    _last ;
	// ������ݵĸ���
	int 			_change ;
};


#endif /* AUTHCACHE_H_ */
