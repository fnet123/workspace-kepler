/*
 * postquery.h
 *
 *  Created on: 2012-7-5
 *      Author: humingqing
 *
 *  ƽ̨����Զ�Ӧ����
 */

#ifndef __POSTQUERY_H__
#define __POSTQUERY_H__

#include <map>
#include <vector>
#include <string>
#include <Mutex.h>

class CPostQueryMgr
{
	typedef std::vector<std::string> VecString ;
	struct _PostQuery
	{
		int 	  _index ;
		int 	  _size ;
		VecString _vec ;
	};
	typedef std::map<std::string,_PostQuery*> CMapPostQuery;
public:
	CPostQueryMgr() ;
	~CPostQueryMgr() ;
	// ����ƽ̨��ڵ�����
	bool LoadPostQuery( const char *path, int accesscode ) ;
	// ȡ��ƽ̨��ڵ�����
	bool GetPostQuery( int accesscode, unsigned char type, std::string &content ) ;

private:
	// ����ƽ̨��ڵ�����
	void ClearPost( void ) ;
	// �����������Ŀ
	bool SplitItem( int accesscode, char * p ) ;
	// ������������
	bool SplitContent( const char *key, char *body ) ;

private:
	// �����������Ĳ���
	share::Mutex   _mutex ;
	// ƽ̨����Զ�Ӧ�������
	CMapPostQuery  _mpPost ;
};


#endif /* AUTOPOSTQUERY_H_ */
