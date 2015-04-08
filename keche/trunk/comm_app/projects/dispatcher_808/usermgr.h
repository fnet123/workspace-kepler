/*
 * usermgr.h
 *
 *  Created on: 2014��1��20��
 *      Author: ycq
 */

#ifndef _USERMGR_H_
#define _USERMGR_H_ 1

#include <Thread.h>
#include "interface.h"

#include <set>
using std::set;
#include <map>
using std::map;
#include <string>
using std::string;

class UserMgr :  public share::Runnable, public IUserMgr {
	// ����ָ�봦��
	ISystemEnv  *		      _pEnv ;
	// ��ʱ����߳�
	share::ThreadManager  	  _time_thread ;
	// ������д��
	share::RWMutex            _rwMutex;
	// �Խ���ҵƽ̨�����ļ�
	string                    _corp_info_file;
	// �Խ�ƽ̨��Ϣ
	map<string, string>       _corp_detail;   //ת��ƽ̨�Ľ�����Ϣ
	// ·��ӳ���
	map<string, set<string> > _route_detail;  //һ���ֻ������Ӧ���ת��ƽ̨
public:
	UserMgr();
	virtual ~UserMgr();

	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv);
	// ��ʼ
	virtual bool Start(void);
	// ֹͣ
	virtual void Stop(void);

	// ��ʱ����߳�
	virtual void run( void *param );

	// ��ѯ�ַ�·��
	virtual set<string> getRoute(const string &macid);
	// ��ѯͨ����Ϣ
	virtual string getCorpInfo(const string &channelId);
	// ȷ�Ϸַ�·��
	virtual bool chkRoute(const string &userid);
	// ��ȡ����·��
	virtual set<string> getAllRoute();
};

#endif//_USERMGR_H_
