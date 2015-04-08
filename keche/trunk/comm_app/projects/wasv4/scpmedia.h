#ifndef __SCPMEDIA_H__
#define __SCPMEDIA_H__

#include "interface.h"
#include <inetfile.h>
#include <dataqueue.h>
#include <TQueue.h>
#include <map>
#include <vector>
#include <string>
using namespace std ;

#define SCP_MAX_TIMEOUT     60  // 1�������ݳ�ʱ

class CScpMedia : public IQueueHandler
{
	struct _stNetObj
	{
		INetFile    *_pobj ;
		share::Mutex _mutex ;
	};
	// �ļ�����
	struct MultiDesc
	{
		int _fd    ;
		int _total ;
		int _id    ;
		int _type  ;
		int _ftype ;
		int _event ;
		int _channel ;
		string _macid ;
		string _name ;
		string _path ;
		string _gps  ;
		string _abspath ;  // ���·��
		MultiDesc *_next;  // ��һ������ָ��

		void Copy( const MultiDesc &desc ){
			_fd 	 = desc._fd ;
			_total   = desc._total ;
			_id      = desc._id ;
			_type    = desc._type ;
			_ftype   = desc._ftype ;
			_event   = desc._event ;
			_channel = desc._channel ;
			_macid   = desc._macid ;
			_name    = desc._name ;
			_path    = desc._path ;
			if ( ! desc._gps.empty() )
				_gps = desc._gps  ;
			_abspath = desc._abspath ;
		}
	};

	// �����ڴ��ļ���������Ҫ�����ְ����ļ�����
	class CScpFile
	{
		// �ڴ��ļ������
		struct MemFile
		{
			int      _index ;
			int      _len ;
			char    *_ptr ;
		};
	public:
		CScpFile( const char *key ) ;
		~CScpFile() ;
		// ��ʼ������
		bool Init( int fd, const char *macid, int id, int total, int type, int ftype, int event,
				int channel , const char *curdir , const char *gps ) ;
		// ���ò�������
		bool SetParam( int fd, const char *macid, int id, int total, int type, int ftype, int event,
				int channel , const char *curdir , const char *gps ) ;
		// �����ļ�
		bool SaveData( const int index, const char * data, int len , MultiDesc *desc ) ;

	private:
		// ��ǰ�������
		unsigned int 		_cur  ;
		// �����������
		MultiDesc  			_desc ;
		// ����ڴ�����
		vector<MemFile *>   _vec ;
		// ���õ�ǰʱ��
		time_t 				_now ;

	public:
		std::string 		_key ;
		// ���һ��ʱ��
		time_t 				_last ;
		// ʹ��TQueue��ָ�����
		CScpFile *  		_next ;
		CScpFile * 	 		_pre ;
	};

	typedef map<string, CScpFile *>   MapScpFile ;

	// �������е�SCP�ļ�
	class CScpFileManager
	{
	public:
		CScpFileManager() ;
		~CScpFileManager() ;

		// ��ʼ������
		bool Init( ISystemEnv *pEnv ) ;

		// �������ݰ�����
		bool SaveScpFile( int fd, const char *macid, int id, int total, int index, int type,
					int ftype,int event,int channel, const char * data, int len , const char *gps,  MultiDesc *desc ) ;
		// ��ⳬʱ���ڴ��ļ�
		void CheckTimeOut( const int timeout ) ;
		// ���ָ����MAC�����Ķ�ý����
		void RemoveScpFile( const char *macid ) ;

	private:
		// ��Դ��
		share::Mutex		    _mutex ;
		// SCP�ֿ��ļ�
		MapScpFile			    _index ;
		// ��ʱ����
		TQueue<CScpFile>		_queue ;
		// ����ļ�Ŀ¼
		std::string 			_cur_dir ;
		// ϵͳ����
		ISystemEnv			   *_pEnv ;
	};

public:
	CScpMedia() ;
	~CScpMedia() ;

	bool Init( ISystemEnv *pEnv ) ;
	bool Start( void ) ;
	void Stop( void ) ;
	// �������ݰ�����
	bool SavePackage( int fd, const char *macid, int id, int total, int index, int type,
			int ftype,int event,int channel, const char * data, int len , const char *gps ) ;
	// �������������
	void RemovePackage( const char *macid ) ;
	// ��ⳬʱ����
	void CheckTimeOut( void ) ;

public:
	// �߳�ִ�ж��󷽷�
	virtual void HandleQueue( void *packet ) ;

private:
	// д��Զ���ļ�
	bool writeFile( const char *dir, const char *name, const char *path ) ;

private:
	// ��������ָ��
	ISystemEnv			*  _pEnv ;
	// ���ݷ�������
	CDataQueue<MultiDesc> *_mediaqueue ;
	// �����̶߳���
	CQueueThread 		 * _queuethread;
	// �̸߳���
	unsigned int 		   _thread_num ;
	// FTP��IP
	string 				   _cfs_ip ;
	// FTP��½���û���
	string 				   _cfs_user ;
	// FTP��½���û���
	string 				   _cfs_pwd ;
	// FTP�Ķ˿�
	unsigned int 		   _cfs_port ;
	// ���һ�μ��ʱ��
	time_t				   _last_check ;
	// ��������SCP�ļ�
	CScpFileManager		   _scp_manager ;
	// �Ƿ���FTP
	bool 				   _cfs_enable ;
	// �Ƿ�������ˮӡ
	bool 				   _water_enable;
	// �������ݶ���
	_stNetObj			   _netobj ;
};

#endif
