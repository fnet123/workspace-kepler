/**
 * date:   2011/07/21
 * author: humingqing
 * memo:   ���ü������󣬵�����ֵΪ��ʱ�ͷŶ���
 *  2012.12.10 �޸����ü����ݶ��󣬽���ʹ��ȫ��������Ϊ��������ʱ��̣ܶ�����������Թ���һ����
 *  �������Լ���Ӧ����������Դռ��
 */

#include <Ref.h>
#include <Mutex.h>
#include <assert.h>

namespace share{

/**
 * ���ü�����
 */
static Mutex _gmutex__ ;


Ref::Ref():_ref__(0)
{
}

/**
 * �������
 */
int Ref::AddRef()
{
	_gmutex__.lock() ;
	++ _ref__ ;
	_gmutex__.unlock() ;

	return _ref__ ;
}

/**
 * ȡ������
 */

int Ref::GetRef()
{
	Guard g( _gmutex__ ) ;

	return _ref__ ;
}

/**
 * �ͷ�����
 */
void Ref::Release()
{
	bool destory = false ;
	{
		_gmutex__.lock() ;
		-- _ref__ ;
		assert( _ref__ >= 0 ) ;

		if ( _ref__ == 0 ) {
			destory = true ;
		}
		_gmutex__.unlock() ;
	}
	if ( destory ) {
		delete this ;
	}
}


}
