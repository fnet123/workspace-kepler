/*
 * plugin.h
 *
 *  Created on: 2012-5-30
 *      Author: humingqing
 *  Memo: ���ģʽ���������ݣ�������Ҫ��Ի��˺�˦��ҵ����������д���������������͸�������ݿ���ͨ�����������
 */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <interface.h>
#include <iplugin.h>
#include <Mutex.h>
#include "waymgr.h"

class CPlugin :
	public IMsgHandler, public IPlugin
{
	class CPlugUserMgr
	{
		struct _UserInfo
		{
			unsigned int _id ;
			string 	 _userid ;
			string   _macid ;
		};
		typedef map<string,_UserInfo*> 			CMapMacId ;
		typedef map<unsigned int, _UserInfo*>   CMapUserId ;
	public:
		CPlugUserMgr() ;
		~CPlugUserMgr() ;
		// ��ӵ��û�����
		unsigned int AddUser( const char *userid, const char *macid ) ;
		// ȡ���û�����
		bool GetUser( unsigned int id , string &userid, string &macid ) ;
		// ɾ������
		bool DelUser( unsigned int id ) ;

	private:
		// �������ж���
		void Clear( void ) ;

	private:
		// �û�������
		share::Mutex _mutex ;
		// MACID��Ӧ�Ĳ��ҹ�ϵ
		CMapMacId    _macids ;
		// �û�ID��Ӧ�Ĳ��ҹ�ϵ
		CMapUserId   _userids;
		// �Զ�������ID��
		unsigned int _id ;
	};
public:
	CPlugin() ;
	~CPlugin() ;

	// ��ʼ��
	bool Init( ISystemEnv * pEnv ) ;
	// ��������
	bool Start( void ) ;
	// ֹͣ����
	bool Stop( void ) ;
	// ��������
	bool Process( InterData &data , User &user ) ;

public:
	// ȡ�������ļ��ַ���������
	bool GetString( const char *key, char *buf ) ;
	// ȡ�������ļ����ε�����
	bool GetInteger( const char *key , int &value ) ;
	// ��Ҫ�ص��ⲿ�ӷ��͵�����
	void OnDeliver( unsigned int fd, const char *data, int len , unsigned int cmd ) ;

private:
	// ��������
	ISystemEnv *  _pEnv ;
	// �û��Ự�������
	CPlugUserMgr  _fdmgr ;
	// �Ƿ������ģ��
	bool 		  _enable ;
	// ͨ���������
	CWayMgr		 *_waymgr ;
};


#endif /* PLUGIN_H_ */
