/*
 * blacklist.cpp
 *
 *  Created on: 2012-7-23
 *      Author: humingqing
 *  Memo: ǰ�û�������,ʹ�ü򵥵�˫�����л���ʵ�����ݸ���
 */

#include "blacklist.h"
#include <comlog.h>

CBlackList::CBlackList()
{
	_index = 0 ;
	_size  = 0 ;
}

CBlackList::~CBlackList()
{

}

// ���غ������û�
bool CBlackList::LoadBlack( const char *filename )
{
	if ( filename == NULL )
		return false ;

	char buf[1024] = {0};
	FILE *fp = NULL;
	fp = fopen( filename, "r" );
	if (fp == NULL) {
		// OUT_ERROR( NULL, 0, NULL, "Load black user file %s failed", filename ) ;
		return false;
	}

	int index = 0 ;
	_mutex.lock() ;
	index = ( _index + 1 ) % BACK_BLACKNUM ;
	_mutex.unlock() ;

	_setBlack[index].clear() ;

	int count = 0 ;
	while (fgets(buf, sizeof(buf), fp)) {
		unsigned int i = 0;
		while (i < sizeof(buf)) {
			if (!isspace(buf[i]))
				break;
			i++;
		}
		if (buf[i] == '#')
			continue;

		char temp[1024] = {0};
		for (int i = 0, j = 0; i < (int)strlen(buf); ++ i ) {
			if (buf[i] != ' ' && buf[i] != '\r' && buf[i] != '\n') {
				temp[j++] = buf[i];
			}
		}
		// ��ӵ�Set�ļ�����
		_setBlack[index].insert( SetString::value_type(temp) ) ;

		++ count ;
	}
	fclose(fp);
	fp = NULL;

	// OUT_PRINT( NULL, 0, NULL, "load black user success %s, count %d" , filename , count ) ;

	// ������������л�
	_mutex.lock() ;
	_index = index ;
	_size  = count ;
	_mutex.unlock() ;

	return true ;
}

// �ж��ֻ����Ƿ��ں�������
bool CBlackList::OnBlack( const char *phone )
{
    if ( phone == NULL )
		return false;

	bool bfind = false;

	_mutex.lock();

	if ( _size > 0 ) {
		bfind = (  _setBlack[_index].find( phone ) != _setBlack[_index].end() ) ;
	}

	_mutex.unlock();

	return bfind ;
}
