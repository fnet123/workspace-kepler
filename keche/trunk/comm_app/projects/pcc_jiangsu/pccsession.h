/*
 * pccsession.h
 *
 *  Created on: 2011-03-02
 *      Author: humingqing
 *
 *  �Խ��յ�ת����������
 */

#ifndef __PCCSESSION_H_
#define __PCCSESSION_H_

#include "interface.h"
#include <Mutex.h>
#include <string>
#include <map>
using namespace std ;

// ����������Ϣ
struct _stCarInfo
{
	string macid;       // �ն������ֻ���
	string areacode;    // ��������	AreaCode
	string color;	    // ������ɫ	CarColor
	string carmodel;    // ���ʹ���	CarModel
	string vehicletype; // ��������	VehicleType
	string vehiclenum;  // ���ƺ���	VehicleNum
};

// 4C54_15001088478:WZ:1:A1:18:��E-Y8888
// ��Ӧ�ĳ�����Ϣ����
class CPccSession
{
	class CCarInfoMgr
	{
		struct _stCarList
		{
			_stCarInfo info ; 		// ���ݽṹ��
			_stCarList *next,*pre ; //  ͷβָ��
		};
		typedef map<string,_stCarList *>  CMapCarInfo ;
	public:
		CCarInfoMgr() ;
		~CCarInfoMgr() ;
		// ��ӳ�����Ϣ
		bool AddCarInfo( _stCarInfo &info ) ;
		// ȡ�ó�����Ϣ
		bool GetCarInfo( const string &key, _stCarInfo &info , bool byphone ) ;
		// �Ƴ�������Ϣ
		void RemoveInfo( const string &key, bool byphone ) ;
		// �����������
		void ClearAll( void ) ;
		// ȡ������MACID
		bool GetAllMacId( string &s ) ;
		// ȡ�ó����ĸ���
		int  GetSize( void ) { return _size ; }

	private:
		// �Ƴ������
		void RemoveNode( _stCarList *p ) ;

	private:
		// ���ݲ�������
		CMapCarInfo   _phone2car;
		CMapCarInfo   _vehice2car;

		// ����ͷβָ��
		_stCarList *  _head ;
		_stCarList *  _tail ;
		int 		  _size ;
		share::Mutex  _mutex ;
	};

public:
	CPccSession() ;
	~CPccSession() ;

	// ��������
	bool Load( const char *file ) ;
	// �����ֻ�MACȡ�ó���
	bool GetCarInfo( const char *key, _stCarInfo &info ) ;
	// ���ݳ���ȡ�ö�Ӧ���ֻ�MAC
	bool GetCarMacId( const char *key, char *macid ) ;
	// ȡ������MAC����
	bool GetCarMacData( string &s ) ;
	// ȡ�õ�ǰ��������
	int  GetCarTotal( void ) { return _mgr.GetSize(); }

private:
	// ������Ϣ�������
	CCarInfoMgr  _mgr ;
};


#endif /* PCCSESSION_H_ */
