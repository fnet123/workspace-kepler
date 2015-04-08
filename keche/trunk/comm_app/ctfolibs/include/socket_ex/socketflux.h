/*
 * socketflux.h
 *
 *  Created on: 2012-1-12
 *      Author: Administrator
 */

#ifndef __SOCKETFLUX_H__
#define __SOCKETFLUX_H__

#include <time.h>
#include <Mutex.h>

#define FLUX_KB	 1024.0f

class CSocketFlux
{
public:
	CSocketFlux( int span = 5 ){
		_last  = time(0) ;
		_recv  = _send = 0 ;
		_rflux = _wflux = 0.0f ;
		_span  = 5 ;
	}
	~CSocketFlux(){
	}

	// ������ͳ��
	void AddFlux( int rn , int wn){
		share::Guard guard( _mutex ) ;

		// �ۼ�����
		_recv  = _recv + rn ;
		_send  = _send + wn ;

		time_t now = time(0) ;
		unsigned int nspan = now - _last ;
		// ����ĳʱ��ε�����
		if ( nspan >= _span ) {
			_rflux   = (float)_recv / (float)nspan ;
			_wflux   = (float)_send / (float)nspan ;
			_last    = now ;
			_recv    = _send   =   0 ;
			printf( "recv flux: %f, send flux: %f\n" , _rflux, _wflux ) ;
		}
	}

	// ȡ������
	float GetRFlux( void ) {
		share::Guard guard( _mutex ) ;
		return _rflux ;
	}

	// ȡ�ö���������
	float GetWFlux( void ) {
		share::Guard guard( _mutex ) ;
		return _wflux ;
	}

private:
	// ͬ����
	share::Mutex    	 _mutex ;
	// ���һ��ʱ��
	time_t   			 _last ;
	// ����������
	unsigned int 	 	 _recv ;
	// ����������
	unsigned int 		 _send ;
	// ���һ�ε�����
	float 				 _rflux ;
	// д���ݵ�����
	float 				 _wflux ;
	// ƽ��ʱ����
	unsigned int		 _span ;
};


#endif /* SOCKETFLUX_H_ */
