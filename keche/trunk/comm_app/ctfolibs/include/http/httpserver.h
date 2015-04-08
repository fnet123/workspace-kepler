/*
 * httpserver.h
 *
 *  Created on: 2012-2-29
 *      Author: humingqing
 *
 *  HTTP��������ܣ�ʵ�ּ򵥵�HTTP���ݴ�����ʱ������HTTP���͹��������ݣ���Ҫ���ⲿ���������н�������
 */

#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include <map>
#include "httpresp.h"
#include "NetHandle.h"
#include <protocol.h>

/**
 * HTTP������������
 */
#define HTTPSVR_ERR_SUCCESS								0
#define HTTPSVR_ERR_FAILED								-1
#define HTTPSVR_ERR_ALREADYSTART						-2
#define HTTPSVR_ERR_RECVPORTBINDFAILED					-3
#define HTTPSVR_ERR_LISTENFAILED						-4
#define HTTPSVR_ERR_STARTTHREADPOOLFAILED				-5
#define HTTPSVR_ERR_PARAMERROR							-6
#define HTTPSVR_ERR_PROCING								-7
#define HTTPSVR_ERR_SAVECONNFAILED						-8

/**
 * ҵ����������ӿ�
 */

#define SERVERCONTAINER_ERR_SUCCESS							0
#define SERVERCONTAINER_ERR_PROCING							1
#define SERVERCONTAINER_ERR_FAILED							-1
#define SERVERCONTAINER_ERR_DATAILLEGAL						-2

class IHttpResponse ;
class IHTTPServerContainer
{
public:
	virtual ~IHTTPServerContainer(){}
	/**
	 * ��������������HTTP	����
	 * @param conn_id const time_t ����ID�����첽����ʱ����
	 */
	virtual int ProcRequest( const char* data , const int size , IHttpResponse* resp ) = 0 ;
};

class CHttpServer : public CNetHandle,public IPackSpliter
{
public:
	CHttpServer(IHTTPServerContainer* pContainer);
	~CHttpServer();

	/**
	 * ����������,���÷������˿�
	 */
	int Start( unsigned int thread, unsigned short port ) ;

	/**
	 * ֹͣ������
	 */
	void Stop( void ) ;

public:
	// ���ݵ���
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ƿ�Ͽ�����
	virtual void on_dis_connection( socket_t *sock ){};
	// �����ӵ���
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};
	// �ְ�����
	virtual struct packet * get_kfifo_packet( DataBuffer *fifo ) ;
	// �ͷ����ݰ�
	virtual void free_kfifo_packet( struct packet *packet ) {
		free_packet( packet ) ;
	}

private:
	// �Ƿ�����������
	bool 				  _initalized ;
	// ������������
	IHTTPServerContainer *_pContainer ;
};



#endif /* HTTPSERVER_H_ */
