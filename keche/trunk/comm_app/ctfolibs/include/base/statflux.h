/*
 * stat.h
 *
 *  Created on: 2011-11-16
 *      Author: humingqing
 */

#ifndef __STATFLUX_H_
#define __STATFLUX_H_

#include <time.h>
#include <Mutex.h>

#define FLUX_KB	 1024.0f

class CStatFlux
{
public:
	CStatFlux( int span = 5 ){
		_last   = time(0) ;
		_count  = 0 ;
		_flux   = 0.0f ;
		_span   = 5 ;
	}
	~CStatFlux(){
	}

	// ������ͳ��
	void AddFlux( int n ){
		share::Guard guard( _mutex ) ;

		// �ۼ�����
		_count  = _count + n ;

		time_t now = time(0) ;
		// ����ĳʱ��ε�����
		if ( now - _last >= _span ) {
			_flux   = (float)_count / (float)( now - _last ) ;
			_last   = now ;
			_count  = 0 ;
		}
	}

	// ȡ������
	float GetFlux( void ) {
		share::Guard guard( _mutex ) ;
		return _flux ;
	}
private:
	// ͬ����
	share::Mutex    	 _mutex ;
	// ���һ��ʱ��
	time_t   			 _last ;
	// ����������
	unsigned int 	 	 _count ;
	// ���һ�ε�����
	float 				 _flux ;
	// ƽ��ʱ����
	unsigned int		 _span ;
};

#endif /* STAT_H_ */
