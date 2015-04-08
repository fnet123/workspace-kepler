/*
 * r4coder.h
 *
 *  Created on: 2012-4-26
 *      Author: humingqing
 *  R4�����㷨��һ���������߼���
 */

#ifndef __R4CODER_H__
#define __R4CODER_H__

class CRC4Coder
{
public:
	// Ĭ�ϼ��ܵ����
	CRC4Coder(const char *key="1q2w3e123!@#") ;
	~CRC4Coder( void ) ;
	// ���ܺͽ��ܴ���
	unsigned char* r4code( unsigned char *val, unsigned int len ) ;

private:
	// ��ʼ����ֵ
	void rc4_init( unsigned char *k , unsigned int n ) ;
	// �������ֵ����
	unsigned char rc4_key() ;
	// ���ݽ���
	void rc4_swap( unsigned char *s, unsigned int i , unsigned int j ) ;

private:
	unsigned char _sv[256] ;  // ���ܽ��
	unsigned int  _i ;		  // λ�ü�¼
	unsigned int  _j ;		  // λ�ü�¼
	unsigned char _sk[256] ;  // ԭʼ���
	unsigned int  _klen;	  // ���ĳ���
};


#endif /* R4CODER_H_ */
