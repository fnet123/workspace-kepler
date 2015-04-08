/*
 * xmlParser.cpp
 *
 *  Created on: 2011-12-12
 *      Author: humingqing
 */

#include <tinyxml.h>
#include "xmlparser.h"
#include <tools.h>

#ifndef _WIN32
#include <strings.h>
#define stricmp  strcasecmp
#endif
/////////////////////// CValue ���� /////////////////////////
// ȡ��ֵ����
int CXmlParser::CValue::GetCount( void )
{
	return (int) _vecValue.size() ;
}
// ȡ�ö�Ӧֵ
const char * CXmlParser::CValue::GetValue( int index )
{
	return (const char*) _vecValue[index] ;
}

// ʵ�������ŵĲ���
const char* CXmlParser::CValue::operator[] ( int index )
{
	return ( const char *) _vecValue[index] ;
}

// �������
void CXmlParser::CValue::AddValue( const char *data )
{
	_vecValue.push_back( data ) ;
}

////////////////////////// CXmlConfig /////////////////////////////////
CXmlParser::CXmlParser()
{

}

CXmlParser::~CXmlParser()
{
	if ( _mapValue.empty() ){
		return ;
	}

	CMapKey2Value::iterator it  ;
	for ( it = _mapValue.begin(); it != _mapValue.end(); ++ it ){
		delete it->second ;
	}
	_mapValue.clear() ;
}

// ȡ��XML��ֵ
bool CXmlParser::LoadXml( const char* xmlText )
{
	TiXmlDocument doc ;
	doc.Parse( xmlText ) ;

	TiXmlElement *root = doc.RootElement() ;
	if( root == NULL ) {
		return false ;
	}

	ParseXml( root , root->Value() , true ) ;

	return true ;
}

// ȡ�ü�ֵ����
const int CXmlParser::GetCount( const char *key )
{
	CMapKey2Value::iterator it = _mapValue.find( key ) ;
	if ( it == _mapValue.end() ){
		return 0 ;
	}
	CValue *pValue = it->second ;
	return pValue->GetCount() ;
}

// ȡ���ַ���ֵ
const char * CXmlParser::GetString( const char *key, const int index )
{
	CMapKey2Value::iterator it = _mapValue.find( key ) ;
	if ( it == _mapValue.end() ){
		return NULL ;
	}
	CValue *pValue = it->second ;
	return pValue->GetValue( index ) ;
}

// ȡ��BOOL�εı���
const bool CXmlParser::GetBoolean( const char *key, const int index )
{
	CMapKey2Value::iterator it = _mapValue.find( key ) ;
	if ( it == _mapValue.end() ){
		return false ;
	}
	CValue *pValue = it->second ;
	const char *ptr = pValue->GetValue(index) ;
	if ( ptr == NULL )
		return false ;

	if ( stricmp(ptr , "true" ) ==  0 ){
		return true ;
	}
	return false ;
}

// ȡ�����α���ֵ
const int  CXmlParser::GetInteger( const char *key, const int index )
{
	CMapKey2Value::iterator it = _mapValue.find( key ) ;
	if ( it == _mapValue.end() ){
		return -1 ;
	}

	CValue *pValue = it->second ;
	const char *ptr = pValue->GetValue(index) ;
	if ( ptr == NULL )
		return -1 ;

	return atoi( ptr ) ;
}

// ����XML����
void CXmlParser::ParserAttr( TiXmlNode *pNode, const char *szName )
{
	TiXmlAttribute* attr = pNode->ToElement()->FirstAttribute();
	if(attr){

		TiXmlNode* node = pNode;
		while(node){
			while(attr){
				CQString  temp = szName ;
				temp += ":" ;
				temp += attr->Name() ;

				AddMapKey( temp.GetBuffer() , attr->Value() ) ;

				attr = attr->Next();
			}
			node =  node->NextSiblingElement();
		}
	}
}

// ����XMLԪ��
void CXmlParser::ParseXml( TiXmlNode* pParent, const char *szName , bool bFirst )
{
	if(pParent == NULL) return;

	CQString s ;
	if ( szName != NULL ){
		s = szName ;
	}

	if ( bFirst && szName ) {
		ParserAttr( pParent , szName ) ;
	}

	TiXmlNode* pchild = pParent->FirstChild();
	while(pchild){
		int t = pchild->Type();
		if( t == TiXmlNode::TINYXML_ELEMENT ){
			CQString temp = s ;
			if ( ! temp.IsEmpty() ){
				temp += "::" ;
			}
			temp += pchild->Value() ;

			ParserAttr( pchild, temp.GetBuffer() ) ;
			ParseXml( pchild, (char *)temp , false );

		} else if( t == TiXmlNode::TINYXML_TEXT ) {
			AddMapKey( s.GetBuffer() , pchild->Value() ) ;
		}
		pchild = pchild->NextSibling();
	}
}

// UTF-8ת�ɱ��ر���
static bool utf82locale( const char *szdata, const int nlen , CQString &out )
{
	int   len = nlen + 1024 ;
	char *buf = new char[ len ] ;
	memset( buf, 0 , len ) ;

	if( u2g( (char *)szdata , nlen , buf, len ) == -1 ){
		delete [] buf ;
		return false ;
	}
	buf[len] = 0 ;
	out.SetString( buf ) ;
	delete [] buf ;

	return true ;
}

// ���ӳ�������
void CXmlParser::AddMapKey( const char *key, const char *val )
{
	if ( key == NULL || val == NULL )
		return ;

	CValue *pValue = NULL ;
	CMapKey2Value::iterator it = _mapValue.find( key ) ;
	if ( it != _mapValue.end() ) {
		pValue = it->second ;
	}else{
		pValue = new CValue ;
		_mapValue.insert( std::pair<CQString, CValue*>( key, pValue ) ) ;
	}

	// ����UTF-8������
	CQString stemp ;
	utf82locale( val, strlen(val) , stemp ) ;
	pValue->AddValue( stemp.GetBuffer() ) ;
}

//========================================================================

#define XML_HEADER  "<?xml version='1.0' encoding='utf-8' ?>"
//----------------------------------------------------------------------------
CXmlBuilder::CXmlBuilder(const char* pRootName,const char* pChildName,const char* pItemName)
{
	_pDoc   = new TiXmlDocument ;
	_pRoot  = new TiXmlElement(pRootName);
	_pChild = new TiXmlElement(pChildName);
	_pItem  = new TiXmlElement(pItemName);
	_pDoc->Parse( XML_HEADER ) ;
}
//----------------------------------------------------------------------------
CXmlBuilder::~CXmlBuilder()
{
	if(_pRoot)
		delete _pRoot;
	if(_pChild)
		delete _pChild;
	if(_pItem)
		delete _pItem;
	if ( _pDoc )
		delete _pDoc ;
}
//----------------------------------------------------------------------------
void CXmlBuilder::SetRootAttribute(const char* pName,const char* pValue)
{
	_pRoot->SetAttribute(pName,pValue);
}
//----------------------------------------------------------------------------
void CXmlBuilder::SetChildAttribute(const char* pName,const char* pValue)
{
	_pChild->SetAttribute(pName,pValue);
}
//----------------------------------------------------------------------------
void CXmlBuilder::SetItemAttribute(const char* pName,const char* pValue)
{
	_pItem->SetAttribute(pName,pValue);
}
//----------------------------------------------------------------------------
void CXmlBuilder::InsertItem()
{
	_pChild->InsertEndChild(*_pItem);
}
//----------------------------------------------------------------------------
void CXmlBuilder::GetXmlText(CQString &sXmlText)
{
	TiXmlPrinter  Printer;
	_pRoot->InsertEndChild(*_pChild);
	_pDoc->InsertEndChild(*_pRoot);
	_pDoc->Accept(&Printer);
	sXmlText.SetString( Printer.CStr() ) ;
}
