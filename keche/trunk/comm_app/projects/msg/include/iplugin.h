/*
 * iplugin.h
 *
 *  Created on: 2012-5-30
 *      Author: humingqing
 *  ͸�������ݲ���ӿ�
 */

#ifndef __IPLUGIN_H__
#define __IPLUGIN_H__

#include <SocketHandle.h>

#define MSG_PLUG_IN   	0x0100   // ���������
#define MSG_PLUG_OUT  	0x0200   // ���������

// ���������Ҫ֧�ֵĽӿ�
class IPlugin
{
public:
	virtual ~IPlugin() {} ;
	// ȡ�������ļ��ַ���������
	virtual bool GetString( const char *key, char *buf ) = 0 ;
	// ȡ�������ļ����ε�����
	virtual bool GetInteger( const char *key , int &value ) = 0 ;
	// ��Ҫ�ص��ⲿ�ӷ��͵�����
	virtual void OnDeliver( unsigned int id, const char *data, int len , unsigned int cmd ) =  0 ;
};

// ���ִ��ͨ��
class IPlugWay
{
public:
	virtual ~IPlugWay() {} ;
	// ��Ҫ��ʼ������
	virtual bool Init( IPlugin *plug , const char *url, int sendthread, int recvthread, int queuesize ) = 0 ;
	// ��ʼ�����ͨ��
	virtual bool Start( void ) = 0 ;
	// ֹͣ���ͨ��
	virtual bool Stop( void ) = 0 ;
	// ����͸��������
	virtual bool Process( unsigned int id , const char *data, int len , unsigned int cmd , const char *mid ) = 0 ;
};


#endif /* IPLUGIN_H_ */
