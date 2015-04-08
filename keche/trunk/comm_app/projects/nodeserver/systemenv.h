/**********************************************
 * systemenv.h
 *
 *  Created on: 2011-07-24
 *      Author: humingqing
 *       Email: qshuihu@gmail.com
 *    Comments: ���������࣬��Ҫ������Ҫ�����Ķ��󣬶�������󽻻�֮����н磬
 *    �������κ���������֮��Ľ���������ͨ�������������ʵ��ֱ�ӽ�����ʹ������֮��͸������
 *    Ҳʹ�ýṹ����������ÿһ������ʵ�֣�������ʵ��Init Start Stop����������Ҫʵ��ϵͳ
 *    ֮���ͳһ�淶������
 *********************************************/
#ifndef __SYSTEMENV_H__
#define __SYSTEMENV_H__

#include "interface.h"

class CCConfig ;
class CSystemEnv : public ISystemEnv
{
public:
	CSystemEnv() ;
	~CSystemEnv() ;

	// ��ʼ��ϵͳ
	bool Init( const char *file , const char *logpath , const char *userfile , const char *logname ) ;
	// ��ʼϵͳ
	bool Start( void ) ;
	// ֹͣϵͳ
	void Stop( void ) ;
	// ȡ������ֵ
	bool GetInteger( const char *key , int &value ) ;
	// ȡ���ַ���ֵ
	bool GetString( const char *key , char *value ) ;
	// ȡ���û�����λ��
	const char * GetUserPath( void )  { return _user_file_path.c_str(); }
	// ȡ��Msg����
	INodeSrv * GetNodeSrv( void ){ return _msg_server; } ;
	// ȡ����Ϣ�ڴ�������
	IAllocMsg * GetAllocMsg( void ) { return _allocmsg; } ;
	// ȡ����Ϣ���з������
	IWaitGroup * GetWaitGroup( void ){ return _waitgroup; };

private:
	// ��ʼ����־ϵͳ
	bool InitLog( const char *logpath , const char *logname ) ;

private:
	// �����ļ���
	CCConfig		  *_config ;
	// �û��ļ�·��
	std::string 	   _user_file_path ;
	// Msg�������Ϳͻ���
	INodeSrv 		 * _msg_server ;
	// �Ƿ��ʼ��
	bool 			   _initialed ;
	// MSG�������
	IAllocMsg		 * _allocmsg ;
	// ���ݵȴ�����
	IWaitGroup		 * _waitgroup ;
};

#endif
