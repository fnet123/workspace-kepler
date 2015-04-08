/***
 * author: humingqing
 * date:   2011-09-07
 * memo:   ����HTTP����
 */
#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include "httpparser.h"

// ERROR DEFINES
#define E_HTTPCLIENT_SUCCESS							2000
#define E_HTTPCLIENT_FAILED								E_HTTPCLIENT_SUCCESS + 1
#define E_HTTPCLIENT_REQUESTINVALID						E_HTTPCLIENT_SUCCESS + 2
#define E_HTTPCLIENT_SERVERINVALID						E_HTTPCLIENT_SUCCESS + 3
#define E_HTTPCLIENT_RESPONSEINVALID					E_HTTPCLIENT_SUCCESS + 4
#define E_HTTPCLIENT_RESPTIMEOUT						E_HTTPCLIENT_SUCCESS + 5
#define E_HTTPCLIENT_REQQUEUELENGTH						E_HTTPCLIENT_SUCCESS + 6

/**
 * HTTP�������
 */
class CHttpRequest
{
public:

	typedef enum{ HTTP_METHOD_GET = 0 , HTTP_METHOD_POST } HTTP_METHOD ;

	CHttpRequest();
	~CHttpRequest();

private:

	// URL
	string 		    _strURL ;

	// HTTP����ķ�����Ŀǰ֧�֣�GET �� POST
	HTTP_METHOD     _iMethod ;

	// USER , PASS
	string 		    _strUser ;
	string 		    _strPass ;

	// �����������ַ�Ͷ˿�
	string 			_strProxyServer ;
	unsigned short  _iProxyPort ;

	// �û������Լ�����Զ��������ͷ��Ϣ��������Զ����ɵ����ظ����򸲸��Զ����ɵ�ͷ
	typedef struct
	{
		string strHeader ;
		string strValue ;

	}HTTP_HEADER ;

	list<HTTP_HEADER*> _HeaderList ;

	// POST ����
	char* 	_pData ;
	int 	_iDataSize ;

	// ���ɵ�HTTP����
	char* 	_pHttpReq ;
	int 	_iHttpReqlen ;


protected:

	void CleanHeaderList( void ) ;

	void CleanHttpReq()  ;

	// ��URL�з�������ַ,����,�˿�
	bool GetAddrAndParams ( const char* url  , string& addr , unsigned short& port , string& params ) ;

	/**
	 * ��hdr_list�����ͷ
	 * @param hdr_list list<HTTP_HEADER*>& ͷ�б�
	 * @param header const char* ͷ
	 * @param value const char* ֵ
	 */
	void AddHeader( list<HTTP_HEADER*>& hdr_list , const char* header , const char* value ) ;

	/**
	 * ��hdr_list��ɾ��header
	 * @param hdr_list list<HTTP_HEADER*>&
	 * @param header const char* ͷ
	 */
	void RemoveHeader( list<HTTP_HEADER*>& hdr_list , const char* header ) ;

	// ��ȡ�������ַ�����ʽ
	const char* GetMethodString( void ) ;

	// ��ѯ��һ��ͷ��ֵ
	const char* GetHeader( const char* header ) ;

	void my_strlwr( char* str )  ;

	// ���URL��������û�пո�����У���ת��Ϊ%20
	void CheckParam( const char* param , string& ret_params ) ;

public:

	void SetURL( const char* url ){ _strURL = url ;} ;
	const char* GetURL( void ){ return _strURL.c_str() ; };

	void SetProxy( const char* proxy_server , unsigned short proxy_port )
	{
		_strProxyServer = proxy_server ;
		_iProxyPort = proxy_port ;
	};

	void SetMethod( CHttpRequest::HTTP_METHOD method ){ _iMethod = method ; };
	CHttpRequest::HTTP_METHOD GetMethod(){ return _iMethod ; };

	void SetCookie( const char* cookie );
	const char* GetCookie( void );

	void SetAuthPass( const char* user , const char* pass , bool bProxy ) ;
	const char* GetUser( void ){ return _strUser.c_str() ; } ;
	const char* GetPass( void ){ return _strPass.c_str() ; } ;

	void SetUA( const char* ua );
	const char* GetUA( void );

	void SetContentType( const char* content_type );
	const char* GetContentType( void ) ;

	bool SetPostData( char* data , int size ) ;

	bool AddHeader( const char* header , const char* value ) ;

	// ��������HTTP����
	const char* GetHTTPRequest( string& server , unsigned short& port , int& len ) ;

	CHttpRequest& operator= ( const CHttpRequest& req ) ;
};


class CHttpResponse : public CHttpParser
{
public:
	// ���COOKIE
	const char* GetCookie( void ) const ;

	// ������ݵĳߴ�
	int GetDataSize( void ) const ;

	// ��ȡ����
	const char* GetData( void ) const ;

	// ��ȡCONTENT-TYPE
	const char* GetContentType( void ) const ;

	// ��ȡHTTP��Ӧ�Ĵ���
	const char* GetRespCode( void ) const ;

	// ��ȡHTTP��Ӧ����Ϣ
	const char* GetRespText( void ) const ;

	// �õ�HTTP�İ汾
	const char* GetHttpVer( void ) const ;


};

// ͬ������HTTP����
class CHttpClient
{
public:
	CHttpClient() ;
	~CHttpClient() ;
public:
	// ִ��һ��HTTP����
	int HttpRequest( CHttpRequest& request , CHttpResponse& resp ) ;

private:
	// ���ӷ�����
	int Open( const char *ip, unsigned short port ) ;
	// �ر�����
	void Close( int fd ) ;
};

#endif
