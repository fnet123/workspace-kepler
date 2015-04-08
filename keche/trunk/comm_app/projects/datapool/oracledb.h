/*
 * oracledb.h
 *
 *  Created on: 2012-5-26
 *      Author: humingqing
 */

#ifndef __ORACLEDB_H__
#define __ORACLEDB_H__

#include "idatapool.h"

// KEY��VALUE�Ľӿڶ���
class OracleSqlObj : public CSqlObj
{
public:
	// �ṹ�����
	struct _SqlVal
	{
		SQLTYPE _type;
		string _sval;
		int _nval;
        long long _llval;
	};
	typedef map< string, _SqlVal > CKVMap;

public:
	OracleSqlObj( ) { }
	virtual ~OracleSqlObj( ){ }

	// �����������
	void AddInteger( const string &key, int val ) ;
	// ����ַ���������
	void AddString( const string &key, const string &val ) ;

	void AddVar(const string &key, const string &val);
	//oracle ��ʵ��������
	void AddCSqlObj( const string &key, CSqlObj *sql_obj ) {}
	void AddLongLong( const string &key, long long val );
	// �Ƿ�Ϊ��
	bool IsEmpty( void ) const ;
	// �������Д���
	void ClearObj( void ) ;
	// ���л��ӿ�
	void Seralize( DataBuffer &buf ) ;
	// �����л�����
	void UnSeralize( const char *ptr, int len ) ;

public:
	CKVMap _kv;
};

// Oracle��ѯ�����ݽ����
class OracleResult : public CSqlResult
{
	struct _colvalue
	{
		int    _col ;
		char **_val ;

		_colvalue( int col ) {
			_col = col ;
			_val = NULL ;
			_val = new char*[col] ;

			for ( int i = 0; i < col; ++ i ) {
				_val[i] = NULL ;
			}
		}

		~_colvalue() {
			if ( _val == NULL ) {
				return ;
			}
			for ( int i = 0; i < _col; ++ i ) {
				if ( _val[i] == NULL )
					continue ;
				delete [] _val[i] ;
			}
			delete [] _val ;
		}

		void setvalue( int index, const char* val ) {
			if ( _val[index] != NULL ) {
				delete [] _val[index] ;
				_val[index] = NULL ;
			}
			if ( val == NULL )
				return ;

			int len = strlen( val ) ;
			_val[index]  = new char[len+1] ;
			memset( _val[index], 0, len+1 ) ;
			memcpy( _val[index], val, len ) ;
		}
	};
	typedef std::vector<_colvalue*>  VecValue ;
public:
	OracleResult():_row(0),_col(0){}
	~OracleResult() ;
	// ȡ�ý����¼����
	int GetCount( void )  { return _row; }
	// ȡ����ӛ䛂���
	int GetColumn( void ) { return _col; }
	// ��Թ�ϵ�����ݿ�ȡ�ֶ�ȡ�ý����������
	const char * GetValue( int row , int col ) ;
	// ���Mongoȡ��ָ���ֶε�����
	const char * GetValue( int row , const char *name ) { return NULL; }

public:
	// ���ÿ���ʹ�õ�����
	void SetColumn( int col ) { _col = col; }
	// ���ֵ����
	void AddValue( int index, int col, const char *value ) ;

private:
	// ȡ�ü�¼����
	int      _row ;
	// ȡ���еĸ�����¼
	int 	 _col ;
	// ȡ�ý��������
	VecValue _vec ;
};

class COracleDB : public IDBFace
{
public:
	COracleDB( unsigned int id );
	~COracleDB( );

	// ��ʼ�������ݿ�����
	bool Init( const char *ip, const unsigned short port, const char *user, const char *pwd, const char *dbname );
	// ȡ�����Ӷ����ַ�����HASHת��ֵ
	unsigned int GetId( void );
	// ���ݿ������д������
	int Insert( const char *stable, const CSqlObj *obj );
	// �����������ݿ����
	bool InsertBatch( const char *stable, const vector< CSqlObj* > &vec );
	// ���²���
	bool Update( const char *stable, const CSqlObj *obj, const CSqlWhere *where );
	// ��������ɾ������
	bool Delete( const char *stable, const CSqlWhere *where );
	// ����SQL��ѯȡ����,��������Թ�ϵ�����ݿ�Ľӿ�
	CSqlResult * Select( const char *sql ) ;

private:
	// �Ƿ��ύ�������
	bool Execute( const char *stable, const OracleSqlObj *obj, bool commit );
	// �콨WHERE�ı��ʽ
	bool BuildWhere( const CSqlWhere *where, string &s );
	// ���������ַ��ı��ʽ
	bool BuildUpdate( const OracleSqlObj *obj, string &s );

private:
	// ��ѯ����
	int	oracle_select( const char* sql , OracleResult *sql_result );
	// ִ������
	int oracle_exec( const char* sql, bool commit = false );
	// �������ݿ�
	void* oracle_connect( const char* addr, const unsigned short port, const char *user, const char* pass,
			const char* db_name );
	// �ر�����
	int oracle_close( void );
	// �ύ
	int oracle_commit( void );
	// �ع�
	int oracle_rollback( void );

private:
	// ���ݿ����Ӷ���ID
	unsigned int _id;
	// ���ݿ����Ӷ���
	void * _handle;
	// ������Ϣ
	string _errinfo;
};

#endif /* ORACLEDB_H_ */
