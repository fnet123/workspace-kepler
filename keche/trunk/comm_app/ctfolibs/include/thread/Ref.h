/**
 * date:   2011/07/21
 * author: humingqing
 * memo:   ���ü������󣬵�����ֵΪ��ʱ�ͷŶ���
 */

#ifndef __SHARE_REF_H__
#define __SHARE_REF_H__

namespace share{

class Ref{
public:
	Ref() ;
	virtual ~Ref() {} ;

	/**
	 * �������
	 */
	int AddRef() ;

	/**
	 * ȡ������
	 */
	int GetRef() ;

	/**
	 * �ͷ�����
	 */
	void Release() ;

private:
	/**
	 * ���ü���ֵ
	 */
	unsigned int _ref__ ;
};

}

#endif
