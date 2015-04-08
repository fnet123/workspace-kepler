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

class CStatInfo
{
	// ���߳���ͳ��
	struct _CarInfo
	{
		time_t		  _time ;
		std::string   _carnum   ;
		unsigned char _carcolor ;
		unsigned int  _errcnt   ;
		_CarInfo *_next, *_pre ;
	};

	typedef std::map<std::string,_CarInfo*> CMapCar ;
	// ���ߵĿͻ���ͳ��
	struct _ClientInfo
	{
		std::string 	_ip ;
		// TCP��ʱ��
		unsigned short 	_tcp ;
		// �����UDP�˿�
		unsigned short 	_udp ;
		// ���һ��ʱ��
		time_t		    _time ;
		// ��½ʱ��
		time_t			_login ;
		// �ϴ����ݳ������
		unsigned int    _errcnt;
		// �ϴ������ݸ���
		unsigned int 	_send ;
		// ���յ������ݸ���
		unsigned int 	_recv ;
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

	typedef std::map<std::string,_ClientInfo*> CMapClient ;
public:
	CStatInfo() ;
	~CStatInfo() ;

	// ���ó�������
	void SetTotal( unsigned int total ) ;
	// ���õĿͻ���IP�Ͷ˿�
	void SetClient( const char *ip, unsigned short tcpport, unsigned short udpport ) ;
	// �Ƿ����
	void AddVechile( const char *ip, const char *carnum, unsigned char color, bool error ) ;
	// ���յ��ĸ���
	void AddRecv( const char *ip ) ;
	// ���ͳ�ȥ�ĸ���
	void AddSend( const char *ip ) ;
	// ����Ƿ�ʱ
	void Check( void ) ;
	// ��ӡ��ǰXML������
	void Print( const char *xml ) ;

private:
	// ɾ���ͻ�������
	void DelClient( _ClientInfo *p ) ;
	// ���ͻ�������
	int CheckClient( _ClientInfo *p , time_t now ) ;
	// ��ӿͻ���
	_ClientInfo* AddClient( const char *ip, unsigned short tcpport, unsigned short udpport ) ;
	// �����������
	void Clear( void ) ;

private:
	// ��������
	unsigned int _ntotal ;
	// ���߳�����
	unsigned int _nonline ;
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
	// ͬ��������
	share::Mutex _mutex ;
};


#endif /* STATINFO_H_ */
