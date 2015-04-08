/***********************************************************************
 ** Copyright (c)2011
 ** All rights reserved.
 **
 ** File name  : CBase64
 ** Author     : humingqing ( qshuihu@gmail.com )
 ** Date       : 2011-07-26 ���� 13:36:38
 ** Comments   : Base64����ͽ���
 ***********************************************************************/

#ifndef __BASE64_H__
#define __BASE64_H__

class CBase64  
{
public:
	CBase64();
	virtual ~CBase64();

public:
	// ����
    bool Encode( const char *data, const int len ) ;
    // ����
	bool Decode( const char *data, const int len ) ;
	// ȡ������
	char * GetBuffer( void ) { return m_pData; }
	// ȡ�����ݳ���
	int    GetLength( void ) { return m_nSize; }

private:
    char* m_pData ;
    int   m_nSize ;
};

#endif
