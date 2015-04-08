/*
 * datasaver.h
 *
 *  Created on: 2012-5-30
 *      Author: humingqing
 *  ���ݴ洢�����Ķ���
 */

#ifndef __DATASAVER_H__
#define __DATASAVER_H__

#include <interface.h>
#include <Thread.h>

class IDataPool ;
class Inter2SaveConvert ;

#define DB_THREAD      3 

class CDataSaver : public IMsgHandler,  public share::Runnable
{
public:
	CDataSaver() ;
	~CDataSaver() ;

	// ��ʼ��
	bool Init( ISystemEnv * pEnv ) ;
	// ��������
	bool Start( void ) ;
	// ֹͣ����
	bool Stop( void ) ;
	// ��������
	bool Process( InterData &data , User &user ) ;
	// �߳�ִ�ж���
	virtual void run( void *param );
	
private:
	// ��������ָ��
	ISystemEnv 		  *_pEnv ;
	// ת�����ݶ���
	Inter2SaveConvert *_inter_save_convert;
	// ���ݿ����ӳ�
	IDataPool 		  *_save_pool;
	// �Ƿ����洢����
	bool 			   	  _enable_save ;
	//������������ݵ��̡߳�
	share::ThreadManager  _db_thread ;
};


#endif /* DATASAVE_H_ */
