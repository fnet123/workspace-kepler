/**********************************************
 * msgclient.h
 *
 *  Created on: 2014-06-30
 *    Author:   ycq
 *********************************************/

#ifndef _MSMCLIENT_H_
#define _MSMCLIENT_H_ 1

#include "interface.h"
#include <BaseClient.h>
#include <interpacker.h>
#include <Mutex.h>

#include <time.h>

class MsgClient : public BaseClient , public IMsgClient {
public:
	MsgClient() ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ��MSG�ϴ���Ϣ
	virtual bool HandleData(const char *data, int len);
public:
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();
	// ������½����
	virtual int build_login_msg( User &user, char *buf,int buf_len ) ;
protected:
	// ���͵�ָ����������û�
	bool SendDataToUser( const string &area_code, const char *data, int len ) ;
	// ����MSG���û��ļ�
	bool LoadMsgUser( const char *userfile ) ;
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	// ���ض��Ĺ�ϵ�б�
	void LoadSubscribe() ;
	// �����ڲ�Э������к�
	string getSeqid(const string &sim);
	// ��ѯredis����macid
	string getMacid(const string &sim);
	// ת�������ϴ���Ϣ
	bool converUrpt(const string &macid, const string &seqid, const string &param, DataBuffer &buf);
	// ת���ظ�Ӧ����Ϣ
	bool converResp(const string &macid, const string &seqid, const string &param, DataBuffer &buf);
	// �����ڲ�Э����Ϣ��
	void parseParam(const string &param, map<string, string> &detail);
	// ��ѯ��ȡ����ֵ
	string queryParam(const string & key, const map<string, string> &detail);
private:
	// ����ָ��
	ISystemEnv      *_pEnv;
	// �ְ�������ְ�
	CInterSpliter    _packspliter;
	// ���Ĺ�ϵ�б�
	string           _dmdfile;
	// ͼƬ����·��
	string           _scppath;
	// ��̬�����ļ�
	string           _datfile;
	// msg�ͻ��˹����߳���
	int             _threadnum;
	// �ڲ�Э�����к�
	unsigned int   _seqid;
	// Ӧ�𻺴�����
	map<string, string> _replycache;
	// Ӧ�𻺴�ͬ����
	share::Mutex        _replymutex;
	// �����嵥�����ڻ�ȡmacid
	map<string, string> _macidquery;
	// �����嵥ͬ����
	share::Mutex        _macidmutex;
};

#endif//_MSMCLIENT_H_
