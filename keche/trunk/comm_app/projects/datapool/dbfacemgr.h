/*
 * dbfacemgr.h
 *
 *  Created on: 2012-5-26
 *      Author: humingqing
 */

#ifndef __DBFACEMGR_H__
#define __DBFACEMGR_H__

#include "idatapool.h"
// ���ݿ����ӿ��ٶ���
class CDBFaceMgr
{
public:
	// ȡ���ַ�����ID
	static unsigned int GetHash( const char *s ) ;
	// ȡ�����ݿ����
	static IDBFace * GetDBFace( const char *s , unsigned int key ) ;
};

#endif /* DATAALLOC_H_ */
