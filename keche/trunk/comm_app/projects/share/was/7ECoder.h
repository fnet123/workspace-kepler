/*
 * C7ECoder.h
 *
 *  Created on: 2011-7-29
 *      Author: zhangdeke
 */

#ifndef __7ECODER_H_
#define __7ECODER_H_

#ifndef  MAX_BUFF
#define  MAX_BUFF  1500 // ������󻺴�ռ�
#endif

class C7eCoder
{
public:
	C7eCoder() ;
	virtual ~C7eCoder() ;
	// ת������
	bool Encode( const char *data, const int len ) ;
	// ����
	bool Decode( const char *data, const int len ) ;

	// ȡ�ý���ĳ���
	const int    GetSize() ;
	// ȡ������
	const char * GetData() ;

private:
	// ���ݳ���
	int   _len  ;
	// ��������
	char *_data ;
	// ����BUF
	char _buf[MAX_BUFF] ;
	// ������ʱָ��
	char *_temp ;
};

#endif /* C7ECODER_H_ */
