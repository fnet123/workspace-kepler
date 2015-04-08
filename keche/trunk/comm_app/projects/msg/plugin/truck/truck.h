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
#include <truckpack.h>

namespace TruckSrv{
	class CTruck :
		public IPlugWay
	{
	public:
		CTruck() ;
		~CTruck() ;

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
		// ���ݽ������
		CTruckUnPackMgr _unpacker ;
		// �������
		CPackFactory *_packfactory;
	};
};
#endif /* TRUCK_H_ */
