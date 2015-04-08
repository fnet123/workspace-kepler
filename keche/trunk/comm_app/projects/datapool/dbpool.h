/*
 * datapool.h
 *
 *  Created on: 2012-5-25
 *      Author: humingqing
 */

#ifndef __DB__POOL__H___
#define __DB__POOL__H___

#include "idatapool.h"
#include <time.h>
#include <Thread.h>
#include <Monitor.h>
#include <TQueue.h>

class CDbPool: public IDataPool, public share::Runnable
{
	// �������ݿ����Ӷ�����
	class ObjList
	{
		struct _DataObj
		{
			time_t _time;
			IDBFace *_pFace;
			_DataObj *_next;
			_DataObj *_pre;
		};
	public:
		// ��һ�����ݶ���ָ��
		ObjList *_next;
		// ǰһ�����ݶ���ָ��
		ObjList *_pre ;

	public:
		ObjList(){};
		~ObjList(){ Clear() ; };
		// ������ݿ����
		void AddObj(IDBFace *pface);
		// ȡ�����ݿ����
		IDBFace *GetObj(void);
		// ��ⳬʱ�Ķ���
		void Check(int timeout);
		// ȡ�õ�ǰ����
		int Size(void ){ return _queue.size(); }
	private:
		// �Ƴ����
		void Remove(_DataObj *p, bool release);
		// ���ȫ������
		void Clear(void);

	private:
		// ���ݿ�������
		TQueue<_DataObj> _queue;
	};

	typedef map<unsigned int, ObjList*> CMapObj;
public:
	CDbPool();
	~CDbPool();
	// ��ʼ�����ݿ����Ӷ���
	bool Init(void);
	// �������ݿ����Ӷ���
	bool Start(void);
	// ֹͣ�������Ӷ���
	bool Stop(void);
	// ǩ�����ݲ�������,�������Ӵ�������������
	IDBFace * CheckOut(const char *connstr);
	// ǩ�����ݲ�������
	void CheckIn(IDBFace *obj);
	// ʵ���ͷ��������ݵĽӿ�
	void Remove(IDBFace *obj);

public:
	// �߳�ִ����
	void run(void *param);

private:
	// ������ж���
	void Clear(void);

private:
	// �̵߳ȴ��ź�
	share::Monitor 		 _monitor;
	// ���ݲ���������
	share::Mutex 		 _mutex;
	// �̶߳���
	share::ThreadManager _thread;
	// �������Ͷ���
	CMapObj 			 _index;
	// ��������
	TQueue<ObjList> 	 _queue ;
	// �Ƿ��ʼ��
	bool 				 _inited;
};

#endif /* DATAPOOL_H_ */
