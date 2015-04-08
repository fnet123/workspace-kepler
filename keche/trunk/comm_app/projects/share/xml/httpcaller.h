/**
 * author: humingqing
 * date:   2011-09-08
 * memo:   �����ն˼�Ȩ,������Ҫ�첽HTTP�ĵ��ô��������첽������Ҫ�ȴ�����
 */
#ifndef __CARAUTHCALLER_H__
#define __CARAUTHCALLER_H__

#include <asynchttpclient.h>

#define TIMEOUT_LIVE   180

#define HTTP_CALL_SUCCESS  0

// �����Ȩ��Ӧ����
class ICallResponse
{
public:
	virtual ~ICallResponse() {}
	// ����HTTP����Ӧ�ص�����
	virtual void ProcessResp( unsigned int seqid, const char *data, const int len , const int err ) = 0 ;
};

class CHttpCaller : public IHttpCallbacker
{
public:
	CHttpCaller() ;
	~CHttpCaller() ;
	// ��ʼ��
	bool Init( int sendthread , int recvthread , int queuesize ) ;
	// ����
	bool Start( void ) ;
	// ֹͣ
	void Stop( void );
	// �첽��HTTP�Ļص��ӿڴ���
	void ProcHTTPResponse( unsigned int seq_id , const int err , const CHttpResponse& resp ) ;
	// ����XML�������
	bool Request( unsigned int seqid, const char *url, const char *data = NULL , const int len = 0 , const char *ctype = "application/xml" ) ;
	// ȡ���������
	unsigned int GetSequeue( void ) ;
	//���ý���ص�����
	void SetReponse( ICallResponse *resp ) { _response = resp ;}

private:
	// �첽��HTTP�Ŀͻ���
	CAsyncHttpClient  		_httpClient ;
	// HTTP�����߳�
	unsigned int 			_send_thread;
	// HTTP�����߳�
	unsigned int 		    _recv_thread;
	// HTTP����������
	int 					_queue_size ;
	//����ص�����
    ICallResponse		   *_response ;
};

#endif
