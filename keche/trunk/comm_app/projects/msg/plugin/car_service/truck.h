/*
 * truck.h
 *
 *  Created on: 2012-5-31
 *      Author: think
 */
#ifndef __TRUCK_H__
#define __TRUCK_H__

#include <iplugin.h>
#include "srvcaller.h"

namespace CarService{
	// ����������
	class CCarService:public IPlugWay
	{
	public:
		CCarService() ;
		~CCarService() ;

		// ��Ҫ��ʼ������
		bool Init( IPlugin *plug , const char *url, int sendthread, int recvthread, int queuesize ) ;
		// ��ʼ�����ͨ��
		bool Start( void ) ;
		// ֹͣ���ͨ��
		bool Stop( void ) ;
		// ����͸��������
		bool Process( unsigned int fd, const char *data, int len , unsigned int cmd , const char *id ) ;

	private:
		// �ص�����
		IPlugin    * _pCaller ;
		// ҵ�����
		CSrvCaller * _srvCaller ;
		// �������
		CPackFactory *_packfactory;
		// ����������ݽ������
		CCarServiceUnPackMgr _unpacker;
	};
};
#endif /* TRUCK_H_ */
