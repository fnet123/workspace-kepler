/*
 * picclient.h
 *
 *  Created on: 2012-11-15
 *      Author: humingqing
 *  ý��ͼƬ�����ϴ�����
 */

#ifndef __PICCLIENT_H__
#define __PICCLIENT_H__

#include <interface.h>
#include <dataqueue.h>

class INetFile;
class CPicClient : public IQueueHandler
{
	struct _picData
	{
		std::string _data ;
		_picData *  _next ;
	};
public:
	CPicClient() ;
	~CPicClient() ;

	bool Init( ISystemEnv *pEnv ) ;
	bool Start( void ) ;
	void Stop( void ) ;

	// ���ý������
	bool AddMedia( const char *data, int len ) ;
	// �������ݶ��л���
	void HandleQueue( void *packet ) ;

private:
	// д��Զ���ļ�����
	bool writeFile( const char *path ) ;
	// ���浽�������ϴ���
	bool saveFile( const char *path, const char *ptr, int len ) ;
	// ͨ��HTTP��ȡͼƬ����
	bool writeHttp( const char *path ) ;

private:
	// ϵͳ��������
	ISystemEnv *		  _pEnv ;
	// ���ݷ�������
	CDataQueue<_picData> *_picqueue ;
	// �����̶߳���
	CQueueThread 		 *_queuethread;
	// ͼƬ���ݶ�ȡ·��
	std::string 		  _basedir ;
	// ����ͼƬ����·��
	std::string 		  _baseurl ;
	// ͼƬ�ļ��ϴ�����
	INetFile    		 *_pobj ;
	// FTP��IP
	std::string 		  _cfs_ip ;
	// FTP��½���û���
	std::string 		  _cfs_user ;
	// FTP��½���û���
	std::string 		  _cfs_pwd ;
	// FTP�Ķ˿�
	unsigned int 		  _cfs_port ;
};


#endif /* PICCLIENT_H_ */
