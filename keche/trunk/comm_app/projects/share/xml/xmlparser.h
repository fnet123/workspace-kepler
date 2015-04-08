/*
 * xmlPaser.h
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */

#ifndef __XMLPASER_H_
#define __XMLPASER_H_

#include <vector>
#include <map>
#include <qstring.h>

class TiXmlNode ;
class CXmlParser
{
	class CValue
	{
	public:
		CValue(){};
		~CValue(){};
		// ȡ�ò����ܸ���
		int GetCount( void ) ;
		// ȡ�õ�ǰֵ
		const char * GetValue( int index ) ;
		// ����ȡ�õ�ǰֵ
		const char* operator[] ( int index ) ;
		// ���Ԫ��
		void AddValue( const char *data ) ;
	private:
		// ��Ӷ����BUF
		typedef std::vector<CQString>  CVectorValue;
		// ���XMLֵ����
		CVectorValue _vecValue ;
	};
public:
	CXmlParser() ;
	~CXmlParser() ;

	// ȡ��XML��ֵ
	bool LoadXml( const char* xmlText ) ;
	// ȡ�ü�ֵ����
	const int GetCount( const char *key ) ;
	// ȡ���ַ���ֵ
	const char * GetString( const char *key, const int index ) ;
	// ȡ��BOOL�εı���
	const bool GetBoolean( const char *key, const int index ) ;
	// ȡ�����α���ֵ
	const int  GetInteger( const char *key, const int index ) ;

private:
	// ����XML����
	void ParseXml( TiXmlNode* pParent, const char *szName , bool bFirst ) ;
	// ����XML������
	void ParserAttr( TiXmlNode *pNode, const char *szName ) ;
	// ��ӵ�������������
	void AddMapKey( const char *key, const char *val ) ;

private:
	// ��Ӧkey��ֵ
	typedef std::map<CQString,CValue*>  CMapKey2Value ;
	// ��Ӧ��mapֵ
	CMapKey2Value  _mapValue ;
};

/*Xml ��װ�Ľ�����*/
class TiXmlDocument;
class TiXmlElement;
class CXmlBuilder
{
public:
	CXmlBuilder(const char* pRootName,const char* pChildName,const char* pItemName);
	virtual ~CXmlBuilder();

	void SetRootAttribute(const char* pName,const char* pValue);
	void SetChildAttribute(const char* pName,const char* pValue);
	void SetItemAttribute(const char* pName,const char* pValue);

	void InsertItem();
	void GetXmlText(CQString &sXmlText);

private:
	 TiXmlDocument *_pDoc;
	 TiXmlElement  *_pRoot;
	 TiXmlElement  *_pChild;
	 TiXmlElement  *_pItem;
};


#endif /* XMLPASER_H_ */
