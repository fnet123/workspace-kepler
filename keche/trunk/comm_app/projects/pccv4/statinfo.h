/*
 * statinfo.h
 *
 *  Created on: 2012-7-27
 *      Author: humingqing
 *  ����ͳ�ƴ���,����ǰ�����û�
 */

#ifndef __STATINFO_H__
#define __STATINFO_H__

#include <string>
#include <map>
#include <time.h>
#include <Mutex.h>

#define STAT_NO	    0x00  // ����Ҫ����
#define STAT_RECV   0x01  // ���յ�����
#define STAT_SEND   0x02  // ���͵�����
#define STAT_ERROR  0x04  // ��������

class CStatInfo
{
	// ���߳���ͳ��
	struct _CarInfo
	{
		time_t		  _time ;
		std::string   _macid ;
		unsigned int  _errcnt ;
		_CarInfo *_next, *_pre ;
	};

	typedef std::map<std::string,_CarInfo*> CMapCar ;
	// ���ߵĿͻ���ͳ��
	struct _ClientInfo
	{
		// ����ʡ��ID
		unsigned int    _id ;
		// ���һ��ʱ��
		time_t		    _time ;
		// �ϴ������ݸ���
		unsigned int 	_send ;
		// ���յ������ݸ���
		unsigned int 	_recv ;
		// ����ĵ����ݸ���
		unsigned int 	_errcnt ;
		// ͷָ��
		_CarInfo 	  * _head ;
		// βָ��
		_CarInfo 	  * _tail ;
		// Ԫ�ظ���
		int 		    _size ;
		// ������Ӧ���б�
		CMapCar		    _mpcar ;
		// ˫������ָ��
		_ClientInfo    *_next, *_pre ;
	};

	typedef std::map<unsigned int,_ClientInfo*> CMapClient ;
public:
	CStatInfo( const char *name ) ;
	~CStatInfo() ;
	// �Ƿ����
	void AddVechile( unsigned int id, const char *macid , int flag = STAT_NO ) ;
	// ���յ��ĸ���
	void AddRecv( unsigned int id ) ;
	// ���ͳ�ȥ�ĸ���
	void AddSend( unsigned int id ) ;
	// ����Ƿ�ʱ
	void Check( void ) ;

private:
	// ɾ���ͻ�������
	void DelClient( _ClientInfo *p ) ;
	// ���ͻ�������
	int CheckClient( _ClientInfo *p , time_t now ) ;
	// ��ӿͻ���
	_ClientInfo* AddClient( unsigned int id ) ;
	// �����������
	void Clear( void ) ;

private:
	// ��ǰ�����ܳ�����
	int 		 _ncar ;
	// ���߿ͻ�������
	_ClientInfo *_head ;
	// ���߿ͻ���βָ��
	_ClientInfo *_tail ;
	// ����Ԫ�صĸ���
	int 		 _size ;
	// �Ծ͵�MAP��ӳ��
	CMapClient   _mpClient ;
	// ���һ��ʹ��ʱ��
	time_t 	     _lasttime ;
	// ͳ�Ʒ��������
	std::string  _name ;
	// ͬ��������
	share::Mutex _mutex ;
};


#endif /* STATINFO_H_ */
