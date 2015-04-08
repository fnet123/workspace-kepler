/*
 * idatapool.h
 *
 *  Created on: 2012-5-22
 *      Author: think
 */

#ifndef __IDATAPOOL_H__
#define __IDATAPOOL_H__

#include <map>
#include <string>
#include <list>
#include <vector>
#include <Ref.h>
#include <packer.h>

using namespace std;

// �����ͷ����ݶ����Ĳ���
#define RELEASE_OBJ( p )   if( p != NULL ) { delete p ; p = NULL ;}

class CSqlObj
{
public:
	enum SQLTYPE{TYPE_INT = 0, TYPE_FLOAT = 1, TYPE_DOUBLE = 2, TYPE_STRING = 3, TYPE_MONGO = 4, TYPE_VAR = 5, TYPE_LONGLONG = 6};
	CSqlObj(){}
    virtual ~CSqlObj(){}
    // ���л��ӿ�
    virtual void Seralize( DataBuffer &buf ) = 0 ;
    // �����л�����
    virtual void UnSeralize( const char *ptr, int len ) = 0 ;
	// �ṹ�����
	virtual void AddInteger( const string &key, int val) = 0 ;
	// ��ӳ����α���
	virtual void AddLongLong(const string &key, long long val) = 0 ;
	// ����ַ�������
	virtual void AddString( const string &key, const string &val) = 0 ;

	//val��Ϊһ���������룬��������Ǵ�oracle������ȡ�����ġ�
	virtual void AddVar(const string &key, const string &val) = 0;

	//���ӱ���ֻ����MongoDB��ʵ�֡�
	virtual void AddCSqlObj(const string &key, CSqlObj *sql_obj) = 0 ;
	// �������Д���
	virtual void ClearObj( void ) =  0;
};

// ����WHERE�Ĵ���
class CSqlWhere
{
public:
	// ������ϵ����"", "AND", "OR"
	enum TWHERE{ TYPE_NO = 0, TYPE_AND = 1, TYPE_OR = 2 } ;
	// ���������� "=","!=",">",">=","<","<=","like"
	enum TOPER{ OP_EQ = 0, OP_NEQ = 1, OP_MORE = 2, OP_EMORE = 3, OP_LESS = 4 , OP_ELESS= 5, OP_LIKE = 6 } ;
	// �ṹ�����
	struct _WhereVal
	{
		TWHERE  _rel;
		string  _skey;  // ֵ
		TOPER   _op;
		string  _sval;
	};
	typedef list<_WhereVal> CWhereList ;
public:
	// ���WHERE����
	void AddWhere( const string &key, const string &val , TOPER op = OP_EQ, TWHERE rel = TYPE_NO ) ;
	// ȡ�ù�ϵ����
	bool GetWhereList( CWhereList &w ) const ;
	// ���WEHERE��LIST
	void ClearWhereList() ;
	// ��SQLWHERE�������л�
	void Seralize( DataBuffer &buf ) ;
	// �����ݶ������л�
	void UnSeralize( const char *ptr, int len ) ;

private:
	// �����б�
	CWhereList _wlst ;
};

// ��ѯȡ���ݽӿ�
class CSqlResult
{
public:
	virtual ~CSqlResult() {} ;
	// ȡ�ý����¼����
	virtual int GetCount( void )  =  0 ;
	// ȡ����ӛ䛂���
	virtual int GetColumn( void ) =  0 ;
	// ��Թ�ϵ�����ݿ�ȡ�ֶ�ȡ�ý����������
	virtual const char * GetValue( int row , int col ) = 0 ;
	// ���Mongoȡ��ָ���ֶε�����
	virtual const char * GetValue( int row , const char *name ) = 0 ;
};

//////////////////DATABASE AGENT OBJECT////////////////////////
#define DB_ERR_NOIMPLEMENT			   -1  // not implement error
#define DB_ERR_SUCCESS					0
#define DB_ERR_FAILED					1
#define DB_ERR_NOCONNECTION				2
#define DB_ERR_SQLERROR					3
#define DB_ERR_NOTIMPL					4
#define DB_ERR_TIMEOUT					5
#define DB_ERR_NOMEM					6
#define DB_ERR_PARAMERROR				7
#define DB_ERR_GETAUTOIDFAILED			8
#define DB_ERR_COMMITFAILED				9
#define DB_ERR_ROLLBACKFAILED			10
#define DB_ERR_STORERESULT				11
#define DB_ERR_SETSQLATTR				12
#define DB_ERR_ALLOCHANDLE				13

/*  �������   */
#define DB_ERR_SOCK                     14

// ���ݿ�����ӿ�
class IDBFace: public share::Ref
{
public:
	virtual ~IDBFace() {}
	// ȡ�����Ӷ����ַ�����HASHת��ֵ
	virtual unsigned int GetId( void ) = 0 ;
	// ���ݿ������д������
	virtual int  Insert( const char *stable, const CSqlObj *obj ) = 0 ;

	/*************************************
	 * Ϊ������oracle��mongoDB���������������CSqlObjӦ��ʱָ�룬ʹ������Լ��ͷš�
	 * ����һ�ַ����������������ӿڣ��ڽӿ��о���ָ������SqlObj�����͡��ӽӿں�ʹ�õĽǶȣ�������ȻҲ���á�
	 *************************************/
	virtual bool InsertBatch( const char *stable, const vector<CSqlObj*> &vec ) = 0 ;
	// ���²���
	virtual bool Update( const char *stable, const CSqlObj *obj, const CSqlWhere *where ) = 0 ;
	// ��������ɾ������
	virtual bool Delete( const char *stable, const CSqlWhere *where ) = 0 ;
	// ����SQL��ѯȡ����,��������Թ�ϵ�����ݿ�Ľӿ�
	virtual CSqlResult* Select( const char *sql ) = 0 ;
	// �ͷŽ����
	virtual void FreeResult( CSqlResult *rs ) { RELEASE_OBJ(rs); };
};

// type=oracle;ip=;port=;user=;pwd=;db=;
// ���ݿ����صĽӿ�

// ���ݶ����
class IDataPool
{
public:
    enum DB_TYPE{ oracle, mongo, mysql, redis };
    enum DB_OPRE{ dothing, insert, update };

    // ȡ��SQL��OBJ����
    static CSqlObj* GetSqlObj(DB_TYPE type) ;
    // �ͷ����ݶ���
    static void Release(CSqlObj *obj) { RELEASE_OBJ(obj); }

	virtual ~IDataPool() {}
	// ��ʼ�����ݿ����Ӷ���
	virtual bool Init( void )  = 0 ;
	// �������ݿ����Ӷ���
	virtual bool Start( void ) = 0 ;
	// ֹͣ�������Ӷ���
	virtual bool Stop( void ) = 0 ;
	// ǩ�����ݲ�������,�������Ӵ�������������
	virtual IDBFace * CheckOut( const char *connstr ) = 0 ;
	// ǩ�����ݲ�������
	virtual void CheckIn( IDBFace *obj ) = 0 ;
	// ɾ�����ݲ�������
	virtual  void Remove(IDBFace *obj) = 0;
};

// ����ִ�ж���
class CSqlData
{
public:
	IDataPool::DB_OPRE 	oper;
	string 				table;
	CSqlObj   * 		sql_obj;
	CSqlWhere * 		sql_where;

	CSqlData()
	{
		oper 		= IDataPool::insert ;
		sql_obj 	= NULL;
		sql_where   = NULL;
	}

	~CSqlData()
	{
		IDataPool::Release(sql_obj) ;
		RELEASE_OBJ( sql_where ) ;
	}
};

// ���ݿ�ִ�е�Ԫ
class CSqlUnit
{
public:
	CSqlUnit( IDataPool::DB_TYPE type, CSqlData *data , unsigned int groupid = 0 ) ;
	CSqlUnit( const char *ptr, int len ) ;
	~CSqlUnit() ;
	// ���л����ݶ���
	void Seralize( DataBuffer &buf ) ;

private:
	// ���������ݶ���
	void UnSeralize( const char *ptr, int len ) ;

public:
	// ���ݿ�����
	IDataPool::DB_TYPE  _dbtype ;
	// ���ID��
	unsigned int        _groupid ;
	// ����ִ�ж���
	CSqlData	       *_sqldata ;
};

#endif /* IDATAPOOL_H_ */
