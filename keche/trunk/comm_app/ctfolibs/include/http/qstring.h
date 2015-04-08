#ifndef __QSTRING_H__
#define __QSTRING_H__

#include <string.h>
/************************************************************************/
/* author: humingqing													*/
/* date: 2011-03-10                                                     */
/* memo: CQString��Ϊͨ���ַ����࣬��Ҫʵ�����ݸ�ֵ�͸�ʽ����������,	*/
/*		�ַ������Ӳ����Լ������ַ�������,ȡ�Ӵ�,�滻�Ӵ�,ȥ����β����	*/
/*		���ַ�,�Ƴ�ָ���ַ����ַ�����Сдת��,							*/
/*		������鷳���ڴ������ͷŹ���									*/
/************************************************************************/
class CQString
{
public:
	// ���캯��
	CQString( const char *sz ) ;
	// Ĭ�Ϻ���
	CQString( const CQString &ss ) ;
	// ���캯��
	CQString( void ) ;
	// ��������
	virtual ~CQString( void ) ;
	// ���صȺ�����
	const CQString &operator = ( const char *sz ) ;
	// ���ط�������
	const CQString &operator = ( const CQString &ss ) ;
	// ʵ���ַ�������
	const CQString &operator + ( const char *sz ) ;
	// ʵ���ַ�������
	const CQString &operator + ( const CQString &ss ) ;
	// ��Ӷ�Ԫ����
	const CQString &operator +=( const char *sz ) ;
	// ��Ӷ�Ԫ����
	const CQString &operator +=( const CQString &ss ) ;
	// ��������
	operator const char* ( void ) ;
	// ��������
	operator char* ( void ) ;
	// ��������
	void SetString( const char *data, const int len = 0 ) ;
	// ȡ�����ݳ���
	const int  GetLength( void ) const ;
	// ȡ������
	const char* GetBuffer( void ) const ;
	// ת����Сд
	const char* ToLower( void ) ;
	// ת���ɴ�д
	const char* ToUpper( void ) ;
	// �Ƴ�����
	const char* Remove( const char *sz ) ;
	// ȥ����β�Ŀհ�"\r\n\t"
	const char* Trim( void ) ;
	// �滻�ַ���
	const char* Replace( const char *src, const char *dest ) ;
	// ����ַ��滻�ɶ��ַ�
	const char* NReplace( int n, const char *c[], const char *s[] ) ;
	// ���ĸ�λ�ÿ�ʼ����
	const int   Find( const char *sz, const int pos = 0 ) ;
	// ȡ�Ӵ�,��lenΪ����߸�ͬȡ����β,���ߴ��ڱ�����
	const char* SubString( const int pos, const int len ) ;
	// �������
	void   AppendBuffer( const char *data, const int len = 0 ) ;
	// �Ƿ�Ϊ�մ�
	bool  IsEmpty( void ) ;
	// ���ڴ����ȥ�����ٸ��ַ�
	void  MemTrimLeft( const int count ) ;
	// ���ڴ��ұ�ȥ�����ٸ��ַ�
	void  MemTrimRight( const int count ) ;
	// ֻ���¶೤����
	void  MemTrimLength( const int count ) ;
	// ת��16������ʾ
	const char *ToHex( void ) ;
	// �������
	void Clear( void ) ;

private:
	// �����ڴ�
	void Expand( const int len ) ;
	// ���Ƶ��ڴ���
	void Memcopy( const char *ptr, int len ) ;

private:
	// ���ݳ���
	int		_nLength;
	// ����BUFFER
	char*	_szBuffer;
	// ��ʱ����BUFFER
	char*   _szTemp;
	// �����ڴ泤��
	int     _nMemLen;
};

inline bool operator == (const CQString & a, const CQString & b)
{
	return    ( a.GetLength() == b.GetLength() )				// optimization on some platforms
	       && ( strcmp(a.GetBuffer(), b.GetBuffer()) == 0 );	// actual compare
}
inline bool operator < (const CQString & a, const CQString & b)
{
	return strcmp(a.GetBuffer(), b.GetBuffer()) < 0;
}

inline bool operator != (const CQString & a, const CQString & b) { return !(a == b); }
inline bool operator >  (const CQString & a, const CQString & b) { return b < a; }
inline bool operator <= (const CQString & a, const CQString & b) { return !(b < a); }
inline bool operator >= (const CQString & a, const CQString & b) { return !(a < b); }

inline bool operator == (const CQString & a, const char* b) { return strcmp(a.GetBuffer(), b) == 0; }
inline bool operator == (const char* a, const CQString & b) { return b == a; }
inline bool operator != (const CQString & a, const char* b) { return !(a == b); }
inline bool operator != (const char* a, const CQString & b) { return !(b == a); }

#endif
