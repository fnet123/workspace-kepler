/*
 * businfoloader.h
 *
 *  Created on: 2012-3-19
 *      Author: think
 */

#ifndef __BUSLOADER_H__
#define __BUSLOADER_H__

#include <string>
#include <vector>
#include <Thread.h>

// ϵͳ��������
class ISystemEnv ;
// ���س�����̬��Ϣ����
class BusLoader : public share::Runnable
{
public:
	BusLoader() ;
	~BusLoader() ;

	// ��ʼ��ϵͳ
	bool Init( ISystemEnv *pEnv ) ;
	// �����߳�
	bool Start( void ) ;
	// ֹͣ�߳�
	void Stop( void ) ;
	// �߳�ִ�ж���
	void run( void *param ) ;

private:
	// ������̬�ļ�
	void loadbusfile( void ) ;

private:
	// �߳�ִ�ж���
	share::ThreadManager  _thread ;
	// ϵͳ��������
	ISystemEnv *		  _pEnv ;
	// �Ƿ��ʼ������
	bool 				  _inited ;
	// ��Ҫ��ȡ�ľ�̬�����ļ�
	std::string			  _sbusfile;
	// �ļ�������IDֵ
	int 				  _index ;
};


#endif /* BUSINFOLOADER_H_ */
