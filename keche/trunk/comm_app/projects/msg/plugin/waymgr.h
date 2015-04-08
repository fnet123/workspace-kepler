/*
 * waymgr.h
 *
 *  Created on: 2012-5-31
 *      Author: think
 */

#ifndef __WAYMGR_H__
#define __WAYMGR_H__

#include <iplugin.h>
#include <map>
#include <vector>
#include <string>
#include <Mutex.h>
#include <Monitor.h>
#include <Thread.h>
#include <Ref.h>

// ����������
class CPlugWay : public share::Ref
{
public:
	CPlugWay( const char *path ) ;
	~CPlugWay() ;

	// ��Ҫ��ʼ������
	bool Init( IPlugin *handler, const char *url, int sendthread, int recvthread, int queuesize ) ;
	// ��ʼ�����ͨ��
	bool Start( void ) ;
	// ֹͣ���ͨ��
	bool Stop( void ) ;
	// ����͸��������
	bool Process( unsigned int fd, const char *data, int len , unsigned int cmd , const char *id ) ;

private:
	// ����ͨ��
	bool LoadWay( const char *path ) ;
	// �ͷ�ͨ��
	void FreeWay( void ) ;

private:
	// ��̬����ؾ��
	void	   *_hModule ;
	// ���ͨ��
	IPlugWay   *_pWay ;
};

// ��̬����Ĺ������
class CWayMgr : public share::Runnable
{
	struct _PlugInfo
	{
		unsigned int 			  _id ;  // Ĭ�ϵ�ID��
		std::vector<unsigned int> _vec;  // ���õ�͸��ָ��
		std::string 			  _path; // ��̬���·��
		std::string 			  _url;  // ����HTTP�����ַ
		int 					  _nsend; // �����̸߳���
		int 					  _nrecv; // �����̸߳���
		int 					  _nqueue;// ����HTTP����Ķ��д�С

		_PlugInfo() {
			_id     = 0 ;
			_nsend  = _nrecv = 1 ;
			_nqueue = 1000 ;
		}
	};
	// ͨ���������
	typedef std::map<std::string,_PlugInfo>     CMapN2Ids;
	typedef std::map<unsigned int,unsigned int> CPlugCmdMap ;
	typedef std::vector<CPlugWay*>			    CPlugWayVec ;
	typedef std::map<std::string,unsigned int>  CPlugNameMap ;
public:
	CWayMgr( IPlugin *handler ) ;
	~CWayMgr() ;

	// ��ʼ������
	bool Init( void ) ;
	// ��������
	bool Start( void ) ;
	// ֹͣ����
	bool Stop( void ) ;
	// ����������ȡ�ò��
	CPlugWay * CheckOut( unsigned int cmd , bool flag = false ) ;
	// �����ǩ��
	void CheckIn( CPlugWay *plug ) ;

public:
	// �߳�ִ�ж���
	void run( void *param ) ;

private:
	// ֹͣ����ͨ��
	void StopWay( void ) ;
	// ������������
	void Clear( void ) ;
	// ��������ļ�
	void CheckConf( void ) ;
	// �����ļ�
	bool LoadFile( const char *file, CMapN2Ids &mpids ) ;

private:
	// ����������
	share::Mutex     	 _mutex ;
	// �źŶ���
	share::Monitor		 _monitor ;
	// �̹߳������
	share::ThreadManager _thread ;
	// ��������ݵ�Handler
	IPlugin 			*_pHandler ;
	// ��̬��Ķ���
	CPlugWayVec			 _wayvec ;
	// ������ֵ�ID
	CPlugNameMap  	 	 _wayn2id ;
	// �����ֵ�ID
	CPlugCmdMap			 _wayc2id ;
	// �Ƿ�������ʼ��
	bool 				 _inited ;
	// DLL���������ļ�·��
	std::string 		 _dllconf ;
	// DLL��Ŀ¼
	std::string 		 _dllroot ;
};


#endif /* WAYMGR_H_ */
