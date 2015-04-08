/*
 * httpresp.h
 *
 *  Created on: 2012-2-29
 *      Author: humingqing
 *
 *  HTTP��������Ӧ����
 */

#ifndef __HTTPRESP_H__
#define __HTTPRESP_H__

#include <map>
#include <string>
#include <qstring.h>

/**
 *  HTTP ��Ӧ����
 */
class IHttpResponse
{
public:

	virtual ~IHttpResponse(){} ;

	/**
	 *  ����HTTP��Ӧ�� ���磺200 ��500 ��404 ��
	 */
	virtual void SetRespCode( int code ) = 0 ;

	/**
	 * ������Ӧ��ͷ��Ϣ
	 */
	virtual void AddHeader( const char* header , const char* value ) = 0 ;

	/**
	 * ���÷��ص�����
	 */
	virtual void SetRespData( const char* resp_data , const int resp_size ) = 0 ;

	/**
	 * �õ���װ�õ�HTTP��Ӧ����
	 */
	virtual const char* GetRespData( int& size ) = 0 ;
};

class CServerHttpResp : public IHttpResponse
{
	typedef std::map<CQString,CQString> CHeaderMap ;
public:

	CServerHttpResp() ;
	~CServerHttpResp() ;


private:
	// ����HTTP����
	int	_iError ;
	// ����HTTPͷ������
	CHeaderMap _HeaderMap ;
	// ��������
	CQString   _sReqData  ;
	// ��Ӧ����
	CQString   _sRespData ;

private:
	void RemoveHeader( const char* header ) ;

public:
	// ����HTTP������
	void SetRespCode( int code ){ _iError = code ; } ;
	// ���HTTP����ͷ����Ϣ
	void AddHeader( const char* header , const char* value );
    // ������Ӧ����
	void SetRespData( const char* resp_data , const int resp_size ) ;
	// ȡ��HTTP��Ӧ����
	const char* GetRespData( int& size ) ;
};



#endif /* HTTPRESP_H_ */
