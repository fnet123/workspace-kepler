/*
 * fileserver.h
 *
 *  Created on: 2012-09-17
 *      Author: humingqing
 *  �������ͷ���
 */

#ifndef __FILESERVER_H__
#define __FILESERVER_H__

#include <interface.h>
#include <Thread.h>
#include <idatapool.h>

class CSynServer : public ISynServer, public share::Runnable
{
public:
	CSynServer() ;
	~CSynServer() ;

	// ͨ��ȫ�ֹ���ָ��������
	virtual bool Init( ISystemEnv *pEnv ) ;
	// ��ʼ����������
	virtual bool Start( void ) ;
	// STOP����
	virtual void Stop( void ) ;
	// �߳����ж���
	virtual void run( void *param )  ;

private:
	// ͬ�����ݲ���
	void SynData( void ) ;

private:
	// ��������ָ��
	ISystemEnv	 		*_pEnv ;
	// �߳�ִ�ж���
	share::ThreadManager _thpool;
	// �������Ӷ���
	IDataPool 		    *_dbpool;
	// �������Ӵ�
	char 				 _szdb[1024];
	// �Ƿ��ʼ����
	bool 				 _inited ;
	// ͬ����ʽ
	unsigned int 		 _synmode ;
	// ͬ��ʱ����
	int 		 		 _syntime ;
};


#endif /* PUSHSERVER_H_ */
