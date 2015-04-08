/**
  * Name: 
  * Copyright: 
  * Author: lizp.net@gmail.com
  * Date: 2009-11-3 ���� 3:42:52
  * Description: 
  * Modification: 
  **/
#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

//UNICODE��תΪGB2312��
int u2g(char *inbuf, int inlen, char *outbuf, int& outlen);

//GB2312��תΪUNICODE��
int g2u(char *inbuf, size_t inlen, char *outbuf, int& outlen);

// ���IP����Ч��
bool check_addr( const char *ip ) ;
// ��ȫ�ڴ濽��
char * safe_memncpy( char *dest, const char *src, int len ) ;

// �Զ����ǰ����ָ������
bool splitvector( const string &str, std::vector<std::string> &vec, const std::string &split , const int count ) ;

// ������Ҫ�������·���д��� env:LBS_ROOT/lbs ֮���·��
bool getenvpath( const char *value, char *szbuf ) ;

/**
 *  ȡ�õ�ǰ��������·��,
 *	env Ϊ�����������ƣ�buf ���·���Ļ���, sz Ϊ���Ӻ�׺, def Ĭ�ϵ��о�
 */
const char * getrunpath( const char *env, char *buf, const char *sz, const char *def ) ;

// ȡ��Ĭ�ϵ�CONF·��
const char * getconfpath( const char *env, char *buf, const char *sz, const char *def, const char *conf ) ;

// ׷��д���ļ�����
bool AppendFile( const char *szName, const char *szBuffer, const int nLen ) ;

// �������ļ�д��
bool WriteFile( const char *szName, const char *szBuffer, const int nLen ) ;

// ��ȡ�ļ�
char *ReadFile( const char *szFile , int &nLen ) ;

// �ͷ�����
void  FreeBuffer( char *buf ) ;

#endif
