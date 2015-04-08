/*
 * iplist.h
 *
 *  Created on: 2012-8-3
 *      Author: humingqing
 *  ip �����б�ֱ�Ӵ�IP��������
 */

#ifndef __IPLIST_H__
#define __IPLIST_H__

#include <list>
#include <Mutex.h>
// ������ʹ�ü���˫���洦��
#define BACK_IPNUM   2

class CIpList
{
	struct IpInfo
	{
		int  _len ;
		char _szip[128] ;
	};
	typedef std::list<IpInfo>  IpList;
public:
	CIpList() ;
	~CIpList() ;

	// ����IP�ĺ�����
	bool LoadIps( const char *filename ) ;
	// ���IP�Ƿ��ں�������
	bool Check( const char *ip ) ;

private:
	// ��ǰʹ�õ�����
	int		 _index ;
	// ��ǰԪ�صĸ���
	int      _size ;
	// IP�ĵ�ַ�б�
	IpList   _ips[BACK_IPNUM] ;
	// ͬ��������
	share::Mutex _mutex ;
};


#endif /* IPLIST_H_ */
