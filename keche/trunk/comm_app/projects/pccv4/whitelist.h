/*
 * whitelist.h
 *
 *  Created on: 2012-5-18
 *      Author: humingqing
 *  �������ݵİ���������
 */

#ifndef __WHITELIST_H__
#define __WHITELIST_H__

#include <set>
#include <map>
#include <string>
#include <Mutex.h>

class CWhiteList
{
	typedef std::set<std::string>  SetString ;
	struct _WhiteIds
	{
		SetString  _lst;
	};
	typedef std::map<int,_WhiteIds*>  CMacList;
public:
	CWhiteList() ;
	~CWhiteList() ;
	// �����ļ�
	bool LoadList( const char *filename ) ;
	// �Ƿ��ڰ�������
	bool OnWhite( int id, const char *key ) ;

private:
	// ��������
	void Clear() ;

private:
	// ��ͬ��������
	share::Mutex _mutex ;
	// �������б�
	CMacList 	 _macList ;
};


#endif /* WHITELIST_H_ */
