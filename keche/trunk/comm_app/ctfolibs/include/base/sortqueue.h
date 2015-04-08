/*
 * sortqueue.h
 *
 *  Created on: 2012-12-27
 *      Author: humingqing
 *
 *  ������д�����Ҫʵʱ����ʱ������������ݣ���չԭ�е�TQueue����
 *  ������ʹ��   _next, _pre, _time ǰ������ָ���ʱ���������
 *
 */

#ifndef __SORTQUEUE_H__
#define __SORTQUEUE_H__

#include <TQueue.h>

template<typename T>
class TSortQueue : public TQueue<T>
{
	typedef TQueue<T>  TBase;
public:
	TSortQueue() {
		TBase::_head = TBase::_tail = NULL;
		TBase::_size = 0 ;
	}
	~TSortQueue() { TBase::clear() ; }

	// ������й���
	void insert( T *o )
	{
		o->_next = o->_pre = NULL ;
		// ���û��Ԫ�ؾ�ֱ����ӵ�ͷ��
		if ( TBase::_head == NULL ) {
			TBase::_head = TBase::_tail = o ;
		} else {
			// ���һ��Ԫ�صĳ�ʱʱ����ڵ�ǰԪ�صĳ�ʱʱ��
			if ( TBase::_tail->_time > o->_time ) {
				// ֱ���ж�ͷ��Ҳ�Ƿ������
				if ( TBase::_head->_time > o->_time ) {
					TBase::_head->_pre = o ;
					o->_next    	   = TBase::_head ;
					TBase::_head       = o ;
				} else { //������϶����м�����
					T *t = TBase::_tail->_pre ;
					// ����������������
					while( t != NULL ) {
						if ( t->_time <= o->_time ){ // �ڶ����м����Ԫ��
							o->_pre  = t ;
							o->_next = t->_next ;
							if ( t->_next )
								t->_next->_pre = o ;
							t->_next = o ;
							break ;
						}
						t = t->_pre ;
					}
				}
			} else { //���������ֱ�ӷŵ���β����
				TBase::_tail->_next  =  o ;
				o->_pre      		 =  TBase::_tail ;
				TBase::_tail 		 =  o ;
			}
		}
		TBase::_size = TBase::_size + 1 ;
	}
};

#endif /* SORTQUEUE_H_ */
