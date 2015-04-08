/*
 * imsgcache.h
 *
 *  Created on: 2012-12-12
 *      Author: humingqing
 */

#ifndef __IMSGCACHE_H_
#define __IMSGCACHE_H_

// ��Ϣ�������
class IMsgCache
{
public:
	virtual ~IMsgCache() {} ;
	// �������
	virtual bool AddData( const char *key, const char *buf, const int len ) = 0 ;
	// ȡ������
	virtual char *GetData( const char *key, int &len , bool erase=true ) = 0  ;
	// �ͷ�����
	virtual void FreeData( char *data ) = 0 ;
	// ����ʱ������
	virtual void CheckData( int timeout ) = 0 ;
	// �Ƴ�����
	virtual bool Remove( const char *key ) = 0 ;
};


#endif /* IMSGCACHE_H_ */
