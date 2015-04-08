/*
 * datasaver.cpp
 *
 *  Created on: 2012-5-30
 *      Author: humingqing
 */

#include "datasaver.h"
#include "idatapool.h"
#include "comlog.h"
#include "dbpool.h"
#include "proto_save.h"
#include <vector>
#include <string>

CDataSaver::CDataSaver():_enable_save(true)
{
	_pEnv = NULL ;
	_save_pool = new CDbPool;
	_inter_save_convert = new Inter2SaveConvert;
}

CDataSaver::~CDataSaver()
{
	Stop() ;

	if ( _save_pool != NULL )
	{
		delete _save_pool ;
		_save_pool = NULL ;
	}

	if ( _inter_save_convert != NULL )
	{
		delete _inter_save_convert ;
		_inter_save_convert = NULL ;
	}
}

// ��ʼ��
bool CDataSaver::Init(ISystemEnv * pEnv)
{
	_pEnv = pEnv ;

	int nvalue = 0 ;
	// �Ƿ����洢����
	if ( pEnv->GetInteger("dbsave_enable", nvalue ) ) {
		_enable_save = ( nvalue == 1 ) ;
	}
	// ���δ����ֱ�ӷ���
	if ( ! _enable_save ) return true ;

	if (!_save_pool->Init())
	{
        OUT_ERROR(NULL, 0, NULL, "init database connect pool fail");
		return false;
	}
	if(!_inter_save_convert->init(_save_pool, pEnv))
	{
		return false;
	}

    _db_thread.init(1, (void*)DB_THREAD, this);
	
	return true ;
}

// ��������
bool CDataSaver::Start( void )
{
	// ���δ����ֱ�ӷ���
	if ( ! _enable_save )
		return true ;

	if (!_save_pool->Start())
	{
		OUT_ERROR(NULL, 0, NULL, "start database connect pool fail");
        return false;
	}

	_db_thread.start();

	return true ;
}

// ֹͣ����
bool CDataSaver::Stop( void )
{
	// ���δ����ֱ�ӷ���
	if ( ! _enable_save )
		return true ;

	// ���߳��˳�����
	_inter_save_convert->stop() ;
	_save_pool->Stop() ;
    _db_thread.stop();

	return true ;
}

// ��������
bool CDataSaver::Process( InterData &data , User &user )
{
    _inter_save_convert->convert(&data);
	return true;
}

void CDataSaver::run( void *param )
{
	int i = 0;
	memcpy(&i, &param, sizeof(int));
	switch(i)
	{
		case DB_THREAD:
			_inter_save_convert->savedb();
			break;
	}

//	_inter_save_convert->savedb();
}
