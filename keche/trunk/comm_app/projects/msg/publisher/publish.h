/*
 * publish.h
 *
 *  Created on: 2012-4-16
 *      Author: humingqing
 */

#ifndef __PUBLISH_H__
#define __PUBLISH_H__

#include <interface.h>
#include <dataqueue.h>
#include <set>
#include <vector>

class Publisher :
	public IPublisher , public IQueueHandler
{
	// �������ݶ�����
	struct _pubData
	{
		unsigned int _cmd ;  	 // ��Ҫ���͵��û�����
		User		 _user ;	 // ������Դ�û�
		InterData    _data ;	 // ��������
		_pubData    *_next ;
	};

	// ���Ĵ���
	class SubscribeMgr
	{
		struct _macList
		{
			std::map<std::string,int> _mkmap ;
		};
	public:
		SubscribeMgr() ;
		~SubscribeMgr() ;

		// ��Ӷ��ĳ�����Ϣ
		bool Add( unsigned int ncode, const char *macid ) ;
		// ɾ�����ĳ�����Ϣ
		void Remove( unsigned int ncode, const char *macid ) ;
		// ɾ����ǰ��������Ϣ
		void Del( unsigned int ncode ) ;
		// ����Ƿ��ĳɹ�
		bool Check( unsigned int ncode, const char *macid , bool check ) ;

	private:
		// �Ƿ�ɾ��
		bool Find( unsigned int ncode, const char *macid , bool erase ) ;
		// ������������
		void Clear( void ) ;

	private:
		typedef std::map<unsigned int, _macList *>  MapSubscribe;
		// ��д������
		share::RWMutex  _rwmutex ;
		// ���Ĵ���
		MapSubscribe	_mapSuber ;
	};

public:
	Publisher() ;
	~Publisher() ;
	// ��ʼ����������
	bool Init( ISystemEnv *pEnv ) ;
	// �������������߳�
	bool Start( void ) ;
	// ֹͣ���������߳�
	bool Stop( void ) ;
	// ��ʼ��������
	bool Publish( InterData &data, unsigned int cmd , User &user ) ;
	// �������ݶ���
	bool OnDemand( unsigned int cmd , unsigned int group, const char *szval, User &user ) ;

public:
	// �߳�ִ�ж��󷽷�
	virtual void HandleQueue( void *packet ) ;

private:
	// �������ݴ���
	bool Deliver( _pubData *p ) ;
	// �Ӷ��Ĺ�ϵ
	void LoadSubscribe( const char *key, std::vector<std::string> &vec, std::set<std::string> &kset ) ;

private:
	// ��������ָ��
	ISystemEnv * 		  _pEnv ;
	// �������ݹ���
	SubscribeMgr		  _submgr ;
	// ���Ĵ����̸߳���
	int 				  _nthread ;
	// �û��������
	IGroupUserMgr 		 *_pusermgr ;
	// ��Ϣ���ͷ���
	IMsgClientServer     *_pmsgserver;
	// ���ݷ�������
	CDataQueue<_pubData> *_pubqueue ;
	// �����̶߳���
	CQueueThread 		 *_queuethread;
};


#endif /* PUBLISH_H_ */
