/*
 * gpsstat.h
 *
 *  Created on: 2012-3-20
 *      Author: humingqing
 *
 *  �򵥵�����ͳ�Ʋ���
 */

#ifndef __GPSSTAT_H__
#define __GPSSTAT_H__

#include <Mutex.h>

class CGpsStat
{
public:
	CGpsStat() {
		_enable = false ;
		_count  = 0 ;
	}
	~CGpsStat() {}

	void Add( unsigned int count = 1 ) {
		share::Guard g( _mutex ) ;
		if ( ! _enable )
			return ;
		_count = _count + count ;
	}

	bool Enable( void ) {
		share::Guard g( _mutex ) ;
		return _enable ;
	}

	void SetEnable( bool enable ) {
		share::Guard g( _mutex ) ;
		_enable = enable ;
		_count  = 0 ;
	}

	unsigned int Size( void ) {
		share::Guard g( _mutex ) ;
		return _count ;
	}

private:
	// ��������
	unsigned int _count ;
	// ������������
	share::Mutex _mutex ;
	// �Ƿ���
	bool 		 _enable ;
};

#endif /* GPSSTAT_H_ */
