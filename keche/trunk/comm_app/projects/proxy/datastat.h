/*
 * datastat.h
 *
 *  Created on: 2012-9-25
 *      Author: humingqing
 *  Memo: ��������ͳ��ģ��
 */

#ifndef __DATASTAT_H__
#define __DATASTAT_H__

#include <map>
#include <string>
#include <Mutex.h>

#define DF_KB  1024
#define DF_MB  1024*1024

class CStat
{
public:
	CStat( int span = 30 ) ;
	~CStat() {}
	// ������ͳ��
	void AddFlux( int n ) ;
	// ȡ������
	void GetFlux( float &count, float &speed ) ;
	// �Ƿ�ܳ�ʱ��û�з�������
	bool Check( int timeout ) ;

private:
	// ͬ����
	share::Mutex    	 _mutex ;
	// ���һ��ʱ��
	time_t   			 _last ;
	// ���һ�μ�����ʱ��
	time_t				 _atime ;
	// ����������
	unsigned int 	 	 _count ;
	// ��������
	unsigned int 		 _len ;
	// ���һ�ε�����
	float 				 _flux ;
	// ȡ�õ�ǰƽ������
	float 				 _nflux ;
	// ƽ��ʱ����
	unsigned int		 _span ;
};

// ��������ͳ�ƶ���
class CDataStat
{
	typedef std::map<int,CStat*> CMapStat ;
public:
	CDataStat() {};
	~CDataStat(){ Clear() ;};
	// �������ͳ��
	void AddFlux( int id, int len ) ;
	// ȡ�õ�������ͳ��
	void GetFlux( int id, float &count, float &speed ) ;
	// ȡ�������ַ���
	const std::string GetFlux( void ) ;
	// �Ƴ�ͳ��
	void Remove( int id ) ;

private:
	// ������������
	void Clear( void ) ;

private:
	// ����ͬ����������
	share::Mutex _mutex ;
	// ����ͳ�ƶ���
	CMapStat     _mpstat ;
};

#endif /* DATASTAT_H_ */
