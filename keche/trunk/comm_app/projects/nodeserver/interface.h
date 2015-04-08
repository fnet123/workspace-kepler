/**********************************************
 * interface.h
 *
 *  Created on: 2011-07-24
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments: ��������ӿ��ඨ�壬��Ҫʵ��������֮�佻���Ľӿڶ���
 *********************************************/
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <inodeface.h>
#include <SocketHandle.h>

class ISystemEnv ;
class INodeSrv
{
public:
	virtual ~INodeSrv() {} ;

	virtual bool Init( ISystemEnv *pEnv ) = 0 ;
	virtual bool Start( void ) = 0 ;
	// ����STOP����
	virtual void Stop( void ) = 0 ;
	// �������ݴ���
	virtual bool HandleData( socket_t *sock, const char *data, int len ) = 0 ;
	// �ڲ�����쳣��������
	virtual void CloseClient( socket_t *sock ) = 0 ;
};


class ISystemEnv
{
public:
	virtual ~ISystemEnv() {} ;
	// ��ʼ��ϵͳ
	virtual bool Init( const char *file , const char *logpath , const char *userfile  , const char *logname ) = 0 ;
	// ��ʼϵͳ
	virtual bool Start( void ) = 0 ;
	// ֹͣϵͳ
	virtual void Stop( void ) = 0 ;
	// ȡ������ֵ
	virtual bool GetInteger( const char *key , int &value ) = 0 ;
	// ȡ���ַ���ֵ
	virtual bool GetString( const char *key , char *value ) = 0 ;
	// ȡ���û�����·��
	virtual const char * GetUserPath( void ) = 0 ;
	// ȡ��NodeSrv����
	virtual INodeSrv * GetNodeSrv( void ) = 0 ;
	// ȡ����Ϣ�ڴ�������
	virtual IAllocMsg * GetAllocMsg( void ) = 0 ;
	// ȡ����Ϣ���з������
	virtual IWaitGroup * GetWaitGroup( void ) = 0 ;
};

#endif
