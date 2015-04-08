/*
 * picclient.cpp
 *
 *  Created on: 2012-11-15
 *      Author: humingqing
 *  ý������ͼƬ���ط�������
 */

#include "picclient.h"
#include <comlog.h>
#include <inetfile.h>
#include <tools.h>
#include <httpclient.h>

CPicClient::CPicClient()
{
	_picqueue   = new CDataQueue<_picData>(102400) ;
	_queuethread= new CQueueThread( _picqueue, this ) ;
	_pobj = NetFileMgr::getfileobj( NetFileMgr::ASYN_MODE ) ;
}

CPicClient::~CPicClient()
{
	if ( _queuethread != NULL ) {
		delete _queuethread ;
		_queuethread = NULL ;
	}

	if ( _picqueue != NULL ) {
		delete _picqueue ;
		_picqueue = NULL ;
	}
	// ��ʼ������
	if ( _pobj != NULL ) {
		NetFileMgr::release( _pobj ) ;
		_pobj = NULL ;
	}
}

bool CPicClient::Init( ISystemEnv *pEnv )
{
	_pEnv = pEnv ;

	char buf[1024] = {0};
	// ȡ��ͼƬ��ַ
	if ( ! pEnv->GetString( "media_dir", buf ) ) {
		OUT_ERROR( NULL, 0, NULL, "get media_dir failed" ) ;
		return false ;
	}
	_basedir = buf ;

	// ý��ͼƬURL
	if ( !pEnv->GetString( "media_url", buf ) ) {
		OUT_ERROR( NULL, 0, NULL, "get media_url failed" ) ;
		return false ;
	}
	_baseurl = buf ;

	if ( ! pEnv->GetString( "cfs_ip" , buf ) ){
		OUT_ERROR( NULL, 0, NULL, "get cfs ip failed" ) ;
		return false ;
	}
	_cfs_ip = buf ;

	if ( ! pEnv->GetString( "cfs_user" , buf ) ) {
		OUT_ERROR( NULL, 0 , NULL, "get ftp save dir failed" ) ;
		return false ;
	}
	_cfs_user = buf ;

	if ( ! pEnv->GetString( "cfs_pwd" , buf ) ) {
		OUT_ERROR( NULL, 0, NULL, "get cfs pwd failed" ) ;
		return false ;
	}
	_cfs_pwd = buf ;

	int ntemp  = 0 ;
	if ( ! pEnv->GetInteger( "cfs_port" , ntemp ) ) {
		OUT_ERROR( NULL, 0, NULL, " get scp port failed" ) ;
		return false ;
	}
	_cfs_port = ntemp ;

	return true ;
}

bool CPicClient::Start( void )
{
	// ���������߳�
	if ( ! _queuethread->Init( 1 ) ) {
		OUT_ERROR( NULL, 0, "MsgClient", "init pic thread failed" ) ;
		return false ;
	}
	return true ;
}

void CPicClient::Stop( void )
{
	// ֹͣ�̴߳���
	_queuethread->Stop() ;
}

// ���ý������
bool CPicClient::AddMedia( const char *data, int len )
{
	if ( len < 4 )
		return false ;

	// ���ΪͼƬ���Ƚ�ͼƬ�ϴ�����������Ȼ���ٴ���
	_picData *p = new _picData;
	p->_data.assign( data, len ) ;
	p->_next = NULL ;

	if ( ! _queuethread->Push( p ) ) {
		delete p ;
		return false;
	}
	return true ;
}

// �������ݶ��л���
void CPicClient::HandleQueue( void *packet )
{
	_picData *p = ( _picData *)packet ;
	// ToDo: ����ͼƬ���ϴ���ͼƬ������
	size_t pos = p->_data.find("125:") ;
	// ���û����ͼƬ·��ֱ�Ӵ���
	if ( pos == string::npos ) {
		// ���ܴ�������
		_pEnv->GetMsgClient()->HandleData( p->_data.c_str(), p->_data.length(), false ) ;
		return ;
	}
	size_t end = p->_data.find( ",", pos ) ;
	if ( end == string::npos ) {
		end = p->_data.find( "}", pos ) ;
	}
	if ( end == string::npos ) {
		// ���ܴ�������
		_pEnv->GetMsgClient()->HandleData( p->_data.c_str(), p->_data.length(), false ) ;
		return ;
	}

	// �����ϴ�Զ�̷������ɹ���񶼴����ڲ�Э��
	string path = p->_data.substr( pos+4, end-pos-4 ) ;
	if ( ! path.empty() ) {
		// printf( "path %s\n", path.c_str() ) ;
		// �����ȡ����ͼƬû�оʹ���������ȡ
		if ( ! writeFile( path.c_str() ) ) {
			writeHttp( path.c_str() ) ;
		}
	}
	// ���ܴ�������
	_pEnv->GetMsgClient()->HandleData( p->_data.c_str(), p->_data.length(), false ) ;
}

// ���浽�������ϴ���
bool CPicClient::saveFile( const char *path, const char *ptr, int len )
{
	// ���ﴮ�л�����ͼƬ�ϴ�
	int ret = _pobj->write( path, ptr, len ) ;
	if ( ret == FILE_RET_SUCCESS ) {
		return true ;
	}

	// ���û�����ӻ��߷���ʧ��
	if ( ret == FILE_RET_NOCONN || ret == FILE_RET_SENDERR || ret == FILE_RET_READERR ) {
		ret = _pobj->open( _cfs_ip.c_str(), _cfs_port, _cfs_user.c_str(), _cfs_pwd.c_str() ) ;
		if ( ret != FILE_RET_SUCCESS ) {
			OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "Pic", "open file mgr failed, result %d" , ret ) ;
			return false ;
		}
	}

	int retry = 0 ;
	// ��������ļ�ʧ�����Լ���
	while( ++ retry < 3 ) {
		ret = _pobj->write( path, ptr, len ) ;
		if ( ret == FILE_RET_SUCCESS ) {
			break ;
		}
	}
	// �����Ƿ�д���ļ��ɹ�
	return ( ret == FILE_RET_SUCCESS ) ;
}

// д��Զ���ļ�
bool CPicClient::writeFile( const char *path )
{
	if ( _basedir.empty() )
		return false ;

	char szdir[1024] = {0} ;
	sprintf( szdir, "%s/%s", _basedir.c_str(), path ) ;

	int len = 0 ;
	char *ptr = ReadFile( szdir, len ) ;
	if ( ptr == NULL ) {
		OUT_WARNING( _cfs_ip.c_str(), _cfs_port, "Pic", "read dir %s pic failed", szdir ) ;
		return false ;
	}

	// �����ļ�
	if ( ! saveFile( path, ptr, len ) ) {
		OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "Pic", "save file %s failed" , path ) ;
		FreeBuffer(ptr) ;
		return false ;
	}
	FreeBuffer(ptr) ;
	//�쳣����
	return true ;
}

// ͨ��HTTP��ȡͼƬ������
bool CPicClient::writeHttp( const char *path )
{
	if ( _baseurl.empty() )
		return false ;

	char szurl[1024] = {0} ;
	sprintf( szurl, "%s/%s", _baseurl.c_str(), path ) ;

	CHttpRequest req ;
	req.SetURL( szurl ) ;

	CHttpResponse rsp ;
	CHttpClient httper ;

	// ͨ������HTTP����ȡͼƬ
	if ( httper.HttpRequest( req, rsp ) != E_HTTPCLIENT_SUCCESS ) {
		OUT_ERROR( NULL, 0, "http", "get pic from url %s failed", szurl ) ;
		return false ;
	}

	int len = 0 ;
	const char *ptr = rsp.GetBody( len ) ;
	if ( len == 0 ) {
		OUT_ERROR( NULL, 0, "http", "get pic from url %s failed, data length zero", szurl ) ;
		return false ;
	}
	// �����ļ�
	if ( ! saveFile( path, ptr, len ) ) {
		OUT_ERROR( _cfs_ip.c_str(), _cfs_port, "Pic", "save file %s failed" , path ) ;
		return false ;
	}
	return true ;
}




