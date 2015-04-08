/**
 * Author: humingqing
 * Date:   2011-10-13
 * Memo:   ���ݻ��������Ҫ���������ڴ��Զ����ٺ����������ݴ�˴���
 */
#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netorder.h>

typedef unsigned char  uint8_t  ;
typedef unsigned short uint16_t ;
typedef unsigned int   uint32_t ;
//typedef unsigned long long uint64_t ;

class DataBuffer
{
public:
	DataBuffer():_ronly(0) {
		_pbuf = _pdata = _pfree = _pend =  NULL ;
	}
	~DataBuffer() {
		if ( _pbuf && _ronly == 0 ) {
			free( _pbuf ) ;
		}
		_pbuf = _pdata = _pfree = _pend =  NULL ;
	}

	// ȡ������
	char * getBuffer() {
		return (char*)_pbuf ;
	}
	// ȡ��ʵ�����ݳ���
	int getLength(){
		return (int)( _pfree - _pbuf ) ;
	}

	 /*
	 * д����
	 */
	void writeInt8(uint8_t n) {
		expand( 1 ) ;
		*(_pfree++) = n ;
	}

	/**
	 * д����������
	 */
	void writeInt16(uint16_t n) {
		expand( 2 ) ;
		n = htons( n ) ;
		memcpy( _pfree , &n, 2 ) ;
		_pfree += 2 ;
	}

	/*
	 * д������
	 */
	void writeInt32(uint32_t n) {
		expand( 4 ) ;
		n = htonl( n ) ;
		memcpy( _pfree , &n, 4 ) ;
		_pfree += 4 ;
	}

	/**
	 * д64������
	 */
	void writeInt64(uint64_t n) {
		expand( 8 ) ;
		n = htonll( n ) ;
		memcpy( _pfree , &n, 8 ) ;
		_pfree += 8 ;
	}

	/**
	 * д���ݿ�
	 */
	void writeBlock(const void *src, int len) {
		expand(len);
		memcpy(_pfree, src, len);
		_pfree += len;
	}

	// ����̶����ȵ�����
	void writeBytes( const void *src, int len , int max ) {
		expand( max ) ;
		memset( _pfree, 0, max ) ;
		memcpy( _pfree, src, (len>max) ? max : len ) ;
		_pfree += max ;
	}

	// ���ָ�����ַ�����
	void writeFill( int c, int len ) {
		expand( len ) ;
		memset(_pfree, c, len ) ;
		_pfree += len;
	}

	// дָ����λ������
	void fillInt8( uint8_t n, int offset ){
		*(_pbuf+offset) = n ;
	}

	// дָ��λ�õĶ�������
	void fillInt16( uint16_t n, int offset ) {
		n = htons( n ) ;
		memcpy( _pbuf + offset ,&n , 2 ) ;
	}

	void fillInt32( uint32_t n, int offset ) {
		n = htonl( n ) ;
		memcpy( _pbuf + offset, &n , 4 ) ;
	}

	void fillInt64( uint64_t n , int offset ) {
		n = htonll( n ) ;
		memcpy( _pbuf + offset, &n , 8 ) ;
	}

	// дָ��λ�õ��ڴ��
	void fillBlock( const void *src, int len , int offset ) {
		memcpy(_pbuf+offset, src, len ) ;
	}

	/*
	 * ������
	 */
	uint8_t readInt8() {
		if ( _pdata == NULL )
			return 0 ;
		return *(_pdata++) ;
	}

	uint16_t readInt16() {
		uint16_t n = 0 ;
		if ( _pdata + 2 > _pfree )
			return n ;

		memcpy( &n, _pdata, 2 ) ;
		_pdata += 2 ;
		return ntohs( n ) ;
	}

	uint32_t readInt32() {
		uint32_t n = 0 ;
		if ( _pdata + 4 > _pfree )
			return n ;

		memcpy( &n, _pdata, 4 ) ;
		_pdata += 4 ;
		return ntohl( n ) ;
	}

	uint64_t readInt64() {
		uint64_t n = 0 ;
		if ( _pdata + 8 > _pfree )
			return n ;

		memcpy( &n, _pdata, 8 ) ;
		_pdata += 8 ;
		return ntohll( n ) ;
	}

	// ��ȡ���ݿ�
	bool readBlock(void *dst, int len) {
		if (_pdata + len > _pfree) {
			return false;
		}
		memcpy(dst, _pdata, len);
		_pdata += len;
		return true;
	}

	// ��ָ��λ������
	uint8_t  fetchInt8( int offset ) {
		return *(_pbuf+offset) ;
	}

	uint16_t fetchInt16( int offset ) {
		uint16_t n = 0 ;
		memcpy( &n, _pbuf+offset, 2 ) ;
		return ntohs( n ) ;
	}

	uint32_t fetchInt32( int offset ) {
		uint32_t n = 0 ;
		memcpy( &n, _pbuf+offset, 4 ) ;
		return ntohl( n ) ;
	}

	uint64_t fetchInt64( int offset ) {
		uint64_t n = 0 ;
		memcpy( &n, _pbuf+offset, 8 ) ;
		return ntohll( n ) ;
	}

	// ��ȡָ���������ݿ�
	bool fetchBlock( int offset, void *dst, int len ){
		// �жϴ�����
		if ( offset < 0 || len < 0  || (_pfree-_pbuf) < len ) {
			return false ;
		}

		memcpy(dst, _pbuf+offset, len ) ;
		return true ;
	}

	// ���¶�λ���ݴ���
	void seekPos( int offset ) {
		_pdata = _pbuf + offset ;
	}

	// ����һ���ڴ��
	void freePos( int offset ) {
		if ( _pfree == NULL )
			return ;
		_pfree = _pfree - offset ;
		if ( _pfree < _pbuf ) {
			_pfree = _pbuf ;
		}
	}

	// �������ݿռ�
	void resetBuf( void ) {
		// ���������ݹ���
		memset( _pbuf, 0, ( _pfree - _pbuf ) ) ;
		_pfree = _pdata = _pbuf ;
	}

	// д��һ��BUF
	void writeBuffer( DataBuffer &buf ) {
		// дһ������BUF��
		writeBlock( (void *) buf.getBuffer(), buf.getLength() ) ;
	}

	// �Ƴ�ǰ�沿������
	void removePos( int len ) {
		if ( len < 0 || _pfree == NULL || _pbuf == NULL )
			return ;
		int size = (int)( _pfree - _pbuf ) ;
		if ( len > size ) {
			resetBuf() ;
			return ;
		}

		int left = size - len ;
		memmove( _pbuf, _pbuf+len, left ) ;
		_pfree = _pbuf + left ;
	}

	// ȷ����len�Ŀ���ռ�
	void ensureFree(int len) {
		expand(len);
	}
	// �ƶ���������ݿ���ָ��
	void pourData(int len) {
		assert(_pend - _pfree >= len);
		_pfree += len;
	}

	// ȡ�ÿ���ָ��
	char *getFree() {
		return (char*)_pfree;
	}

	// ȡ�ÿ��ÿռ��С
	int getFreeLen() {
		return (_pend - _pfree);
	}

protected:
	// ��չ���ݵ��ڴ�
	inline void expand( int need ) {
		if (_pbuf == NULL) {
			int len = 256;
			while (len < need) len <<= 1;
			_pfree = _pdata = _pbuf = (unsigned char*)malloc(len);
			_pend  = _pbuf + len;
		} else if (_pend - _pfree < need) { // �ռ䲻��
			int flen = _pend  - _pfree;  // �����ڴ�ռ�
			int dlen = _pfree - _pbuf;	 // ���ݿռ��С

			if (flen < need || flen * 4 < dlen) {
				int bufsize = (_pend - _pbuf) * 2;
				while (bufsize - dlen < need)
					bufsize <<= 1;

				unsigned char *newbuf = (unsigned char *)malloc(bufsize);

				assert(newbuf != NULL);
				if (dlen > 0) {
					memcpy(newbuf, _pbuf, dlen );
				}
				free(_pbuf);

				_pdata = _pbuf = newbuf;
				_pfree = _pbuf + dlen;
				_pend  = _pbuf + bufsize;
			}
		}
	}

protected:
	// ���ݶ���
	unsigned char *_pbuf ;
	// ���ݻ������
	unsigned char *_pdata ;
	// ���ݿռ��С
	unsigned char *_pend ;
	// ����ʹ��ƫ��
	unsigned char *_pfree ;
	// �Ƿ�Ϊֻ��������
	unsigned char  _ronly;
};

#endif
