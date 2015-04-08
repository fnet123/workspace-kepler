/***
 * author: humingqing
 * date: 2011-09-07
 */
#ifndef __HTTPPARSER_H__
#define __HTTPPARSER_H__

#include <list>
#include <string>
using namespace std ;

// ERROR DEFINES
#define HTTPPARSER_ERR_SUCCESS				0
#define HTTPPARSER_ERR_FAILED				-1
#define HTTPPARSER_ERR_NODATA				-2
#define HTTPPARSER_ERR_NOMEM				-3
#define HTTPPARSER_ERR_NOHEADER				-4
#define HTTPPARSER_ERR_NOCONTENTLEN			-5
#define HTTPPARSER_ERR_DATAERROR			-6

// �����Լ�����չͷ
#define EXT_HEADER_CODE			"x-resp-code"
#define EXT_HEADER_STATUSTEXT 	"x-resp_text"
#define EXT_HEADER_VERSION 		"x-version"
#define EXT_HEADER_METHOD       "x-req-method"  // ��Ϊ��������ʱ������Ҫ
#define EXT_HEADER_URI			"x-req-uri"		// ��Ϊ�������˽���ʱ��Ҫ

class CParamList
{
public:
	CParamList() ;
	~CParamList() ;

private:

	// ͷ�б�
	typedef struct _header_node
	{

		char* header ;
		char* value ;
		struct _header_node * next ;

	}HEADERNODE , *LPHEADERNODE;

	HEADERNODE 	_HeaderList ;

public:
	// ���ͷ�б������
	void CleanHeaderList( void ) ;

	// ��ȡĳ������ֵ
	const char* GetValue( const char* header ) const ;

	// ���ӱ�����
	void AddNode( const char* name , const char* value ) ;
};

class CHttpParser
{
public:

	CHttpParser() ;

	virtual ~CHttpParser() ;

protected:

	char* 		_pBody ;
	int 		_iBodySize ;

	CParamList  _ParamList ;

protected:

	// ���BODY����
	void CleanBody( void ) ;

	static void my_strlwr( char* str ) ;

	/**
	 *  ����CHUNK ģʽ��BODY
	 */
	int SetChunkBody( const char* data , const int size ) ;

public:

	// ��������Ƿ�����
	// ����true��ʾ���ݽ�������,�����ʾ���ݻ�û�н������.
	// data_error����true,��ʾ�ڷ�������ʱ,�������ݲ���ȷ
	static int DetectCompleteReq( const char* data , const int size ) ;

	// �ж�CHUNK�Ƿ����
	static int DetectCompleteChunk( const char* body , int body_size ) ;

	// ��16�����ַ���ת��Ϊ����
	static int hex2int( const char* str ) ;

	// ����HTTP REQUEST
	virtual int  Parse( const char* data , const int size ) ;

	int SetBody( const char* data , const int size ) ;

	const char* GetBody( int& size ) const
	{
		size = 	_iBodySize ;
		return 	_pBody ;
	};

	// ��ȡĳ��ͷ����Ϣ
	const char* GetHeader( const char* header ) const ;

};

#endif
