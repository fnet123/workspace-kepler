/**********************************************
 * msgclient.h
 *
 *  Created on: 2011-07-28
 *    Author:   humingqing
 *    Comments: ʵ������Ϣ��������ͨ���Լ�����ת��
 *********************************************/

#ifndef __MsgCLIENT_H__
#define __MsgCLIENT_H__

#include <httpclient.h>
#include <asynchttpclient.h>

#include "interface.h"
#include <BaseClient.h>
#include <OnlineUser.h>
#include <time.h>
#include <Session.h>
#include <packspliter.h>
#include <qstring.h>

#define MSG_SAVE_CLIENT   "SAVECLIENT"
#define MSG_PIPE_CLIENT   "PIPECLIENT"

struct WAIT_RSP_DATA {
	string       file;
	string       inner;
};

struct WAIT_RSP_TIME {
	time_t       time;
	unsigned int seqid;
};

class MsgClient : public BaseClient , public IMsgClient, public IHttpCallbacker
{
	typedef map<unsigned int, WAIT_RSP_DATA> WAIT_MAP;
public:
	MsgClient() ;
	virtual ~MsgClient() ;

	// ��ʼ��
	virtual bool Init( ISystemEnv *pEnv );
	// ��ʼ����
	virtual bool Start( void );
	// ֹͣ����
	virtual void Stop();
	// ���ݵ���ʱ����
	virtual void on_data_arrived( socket_t *sock, const void* data, int len);
	// �Ͽ����Ӵ���
	virtual void on_dis_connection( socket_t *sock );
	//Ϊ�����ʹ��
	virtual void on_new_connection( socket_t *sock, const char* ip, int port){};

	virtual void TimeWork();
	virtual void NoopWork();

	// ������½��Ϣ����
	virtual int  build_login_msg(User &user, char *buf, int buf_len);

	//
	void ProcHTTPResponse( unsigned int seq , const int err , const CHttpResponse& resp );

protected:
	// ����MSG���û��ļ�
	bool LoadMsgUser( const char *userfile ) ;
	// �׷��ڲ�����
	void HandleInnerData( socket_t *sock, const char *data, int len ) ;
	// �׷���½����
	void HandleSession( socket_t *sock, const char *data, int len ) ;
	// ���������û�
	void HandleOfflineUsers( void ) ;
	// ���������û�
	void HandleOnlineUsers(int timeval) ;
	// ���ض��Ĺ�ϵ�б�
	void LoadSubscribe( User &user ) ;
	//
	int split2map(vector<string> &vec, map<string, string> &mp, const string &split);
	//
	bool processPic(const string &userid, const string &data, const string &content);
	//
	unsigned int getSeqid() { return __sync_add_and_fetch(&_seqid, 1); };
	//
	bool createDir(const string &file);

private:
	// ����ָ��
	ISystemEnv *          _pEnv ;
	// ���һ�η���ʱ��
	time_t                _last_handle_user_time ;
	// �����û�����
	OnlineUser            _online_user;
	// �Ự�������
	CSessionMgr           _session ;
	// ΨһPIPE���͵�userid
	string                _pipe_uid;
	// ���SAVE���͵�URL
	map<string, string>   _save_url;
	//
	unsigned int          _seqid;
	//
	WAIT_MAP              _wait_data;
	//
	list<WAIT_RSP_TIME>   _wait_time;
	//
	share::Mutex          _wait_mutex;
	//
	CAsyncHttpClient      _httpClient ;
	// �ְ�������
	CBrPackSpliter        _packspliter ;
	// ���Ĺ�ϵ�б�
	string                _dmddir ;
	//
	string                _pic_path;
	// �Ƿ�Ϊ��������·��
	bool                  _dataroute;
};

#endif /* LISTCLIENT_H_ */
