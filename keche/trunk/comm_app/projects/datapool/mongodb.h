/*
 * mongodb_pool.h
 *
 *  Created on: 2012-5-28
 *      Author: xifengming
 */

#ifndef _MONGO_DB_H
#define _MONGO_DB_H

#include <BaseClient.h>
#include <OnlineUser.h>
#include <Session.h>
#include <Mutex.h>
#include <tools.h>
#include "idatapool.h"
#include "mongo/client/dbclient.h"

using namespace std;
using namespace mongo;
using namespace bson;

class MongoDBSqlObj : public CSqlObj
{
public:
	MongoDBSqlObj(){}
	virtual ~MongoDBSqlObj(){}

	BSONObjBuilder & GetBsonObj() ;
	void AddInteger( const string &key, int val) ;
	void AddLongLong(const string &key, long long val) ;
	void AddString( const string &key, const string &val) ;
	//���ӱ���ֻ����MongoDB��ʵ�֡�
	void AddCSqlObj(const string &key, CSqlObj *sql_obj) ;

	void AddVar(const string &key, const string &val){return;}
	// �������Д���
	void ClearObj( void ) ;
	// ���л��ӿ�
	void Seralize( DataBuffer &buf ) ;
	// �����л�����
	void UnSeralize( const char *ptr, int len ) ;

private:
	BSONObjBuilder _bson_obj;
};

class CMongoDB: public IDBFace
{
public:
	CMongoDB(unsigned int id) : _id(id){};
	virtual ~CMongoDB(){};

	// ��ʼ�������ݿ�����
	bool Init(const char *ip, const unsigned short port, const char *user, const char *pwd,
			const char *dbname);
	// ȡ�����Ӷ����ַ�����HASHת��ֵ
	unsigned int GetId(void);
	// ���������return value, success : 0, error : err number defined
	virtual int Insert(const char *stable, const CSqlObj *obj);
	// �����������ݿ����
	virtual bool InsertBatch(const char *stable, const vector<CSqlObj*> &vec);
	// ���²���
	virtual bool Update(const char *stable, const CSqlObj *obj, const CSqlWhere *where);
	// ��������ɾ������
	virtual bool Delete(const char *stable, const CSqlWhere *where);
	// ����SQL��ѯȡ����,��������Թ�ϵ�����ݿ�Ľӿ�
	virtual CSqlResult * Select( const char *sql ) { return NULL; }

private:
	bool MongoDB_Insert(const char *stable, const vector<BSONObj> &pbulk_obj);
	// �콨WHERE�ı��ʽ
	bool BuildWhere(const CSqlWhere *where, Query &query);
private:
	// ���ݿ����Ӷ���
	DBClientConnection  _conn;
	// ���ݿ����Ӷ���ID
	unsigned int 		_id;
	// ���ݿ���
	string 				_mongo_db_name;
	// ������Ϣ
	string 				_errinfo;
};
#endif
