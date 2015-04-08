/*
 * blacklist.h
 *
 *  Created on: 2012-7-23
 *      Author: humingqing
 *  memo: ǰ�û��������Ĵ���
 */

#ifndef __BLACKLIST_H__
#define __BLACKLIST_H__

#include <Mutex.h>
#include <set>
#include <string>

// ������ʹ�ü���˫���洦��
#define BACK_BLACKNUM   2

// �������������
class CBlackList
{
	typedef std::set<std::string>  SetString;
public:
	CBlackList() ;
	~CBlackList() ;

	// ���غ������û�
	bool LoadBlack( const char *filename ) ;
	// �ж��ֻ����Ƿ��ں�������
	bool OnBlack( const char *phone ) ;
private:
	// ��ǰʹ�õ�����
	int			 	_index ;
	// ˫�������л�����
	SetString    	_setBlack[BACK_BLACKNUM] ;
	// ���ݹ���������
	share::Mutex    _mutex ;
	// �������
	int 			_size ;
};
#endif /* BLACKLIST_H_ */
