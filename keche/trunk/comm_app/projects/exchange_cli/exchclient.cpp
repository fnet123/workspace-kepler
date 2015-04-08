#include "exchclient.h"
#include <netutil.h>
#include <tools.h>
#include <BaseTools.h>
#include "../share/md5/md5.h"

#include <fstream>
using std::ifstream;

#include "exchprot.h"

ExchClient::ExchClient()
{
	_pEnv = NULL;
	_thread_num = 1;
	_exch_pack_tchdb = tchdbnew();
	//tchdbsetmutex(_exch_pack_tchdb);
	tchdbsetcache(_exch_pack_tchdb, 1000000);
}

ExchClient::~ExchClient()
{
	Stop();
}

bool ExchClient::Init(ISystemEnv *pEnv)
{
	_pEnv = pEnv ;

	char szbuf[512] = {0};

	if (!pEnv->GetString("exch_server_ip", szbuf)) {
		OUT_ERROR( NULL, 0, "conf", "exch_server_ip  empty" );
		return false;
	}
	_client_user._ip = szbuf;

	if (!pEnv->GetString("exch_server_port", szbuf)) {
		OUT_ERROR( NULL, 0, "conf", "exch_server_port  empty" );
		return false;
	}
	_client_user._port = atoi(szbuf);

	if (!pEnv->GetString("exch_user_name", szbuf)) {
		OUT_ERROR( NULL, 0, "conf", "exch_user_name  empty" );
		return false;
	}
	_client_user._user_name = szbuf;

	if (!pEnv->GetString("exch_user_pwd", szbuf)) {
		OUT_ERROR( NULL, 0, "conf", "exch_user_pwd  empty" );
		return false;
	}
	char md5buf[64] = "";
	md5_hex(szbuf, strlen(szbuf), md5buf);
	_client_user._user_pwd = md5buf;

	int nvalue = 0 ;
	if (pEnv->GetInteger("exch_tcp_thread", nvalue)) {
		_thread_num = nvalue;
	}

	nvalue = 0;
	if (!pEnv->GetInteger("exch_pack_timeout", nvalue)) {
		OUT_ERROR( NULL, 0, "conf", "exch_pack_timeout empty");
		return false;
	}
	_exch_pack_tqueue.init(nvalue);

	nvalue = 0;
	if (!pEnv->GetInteger("exch_pack_retries", nvalue)) {
		OUT_ERROR( NULL, 0, "conf", "exch_pack_retries empty");
		return false;
	}
	_exch_pack_retries = nvalue;

	if (!pEnv->GetString("exch_pack_dbfile", szbuf)) {
		OUT_ERROR( NULL, 0, "conf", "exch_pack_dbfile empty");
		return false;
	}
	if (!tchdbopen(_exch_pack_tchdb, szbuf, HDBOCREAT | HDBOTRUNC | HDBOWRITER)) {
		OUT_ERROR( NULL, 0, "conf", "tchdbopen error: %s", tchdberrmsg(tchdbecode(_exch_pack_tchdb)));
		return false;
	}

	_client_user._fd = NULL;
	_client_user._user_state = User::OFF_LINE;
	_client_user._last_active_time = time(NULL);
	_client_user._login_time = 0;
	_client_user._connect_info.last_reconnect_time = 0;
	_client_user._connect_info.timeval = 10;

	setpackspliter(&_spliter_exch);

	return true;
}

bool ExchClient::Start()
{
	return StartClient( _client_user._ip.c_str() , _client_user._port , _thread_num ) ;
}

void ExchClient::Stop()
{
	StopClient() ;
}

void ExchClient::TimeWork()
{
	uint32_t seqid;
	list<uint32_t> seqids;
	list<uint32_t>::iterator it;

	int dbvallen;
	DbValue *dbvalptr;
	int msglen;
	char *msgptr;

	while (Check()) {
		seqids.clear();

		_exch_pack_mutex.lock();
		size_t tqsize = _exch_pack_tqueue.check(seqids);
		OUT_INFO(NULL, 0, "TCHDB", "begin db size: %d, tqsize %u, get size %u", tchdbrnum(_exch_pack_tchdb), tqsize, seqids.size());
		for(it = seqids.begin(); it != seqids.end(); ++it) {
			seqid = *it;
			if ((dbvalptr = (DbValue*) tchdbget(_exch_pack_tchdb, &seqid, sizeof(int), &dbvallen)) == NULL) {
				OUT_INFO(NULL, 0, "Timeout", "in tchdb, seqid %u not found", seqid);
				continue;
			}
			tchdbout(_exch_pack_tchdb, &seqid, sizeof(int)); // ��tchdb�����ļ���ɾ��

			msglen = dbvallen - sizeof(DbValue);
			msgptr = (char*)dbvalptr->dat;

			if(dbvalptr->cnt <= 0) { //�Ѿ������ش�����
				OUT_HEX(_client_user._ip.c_str(), _client_user._port, "TOUT", msgptr, msglen);
				free(dbvalptr); //�ͷ��ڴ�
				continue;
			}

			SendData(_client_user._fd, msgptr, msglen);
			OUT_HEX(_client_user._ip.c_str(), _client_user._port, "TSND", msgptr, msglen);

			if(_exch_pack_tqueue.insert(seqid)) {
				--dbvalptr->cnt;
				tchdbput(_exch_pack_tchdb, &seqid, sizeof(int), dbvalptr, dbvallen);
			}

			free(dbvalptr); //�ͷ��ڴ�
		}
		OUT_INFO(NULL, 0, "TCHDB", "end db size: %d", tchdbrnum(_exch_pack_tchdb));
		tchdboptimize(_exch_pack_tchdb, 0, -1, -1, 255);
		_exch_pack_mutex.unlock();

		sleep(10);
	}
}

void ExchClient::NoopWork()
{
	vector<uint8_t> msgbuf;
	DbValue *dbval;
	uint32_t seqid;
	vector<uint8_t> resbuf;

	while (Check()) {
		time_t now = time(NULL);
		User &user = _client_user;
		const char *ip = _client_user._ip.c_str();
		unsigned int port = _client_user._port;

		if (user._fd != NULL && user._user_state == User::ON_LINE && now - user._last_active_time < 180) {
			resbuf.clear();
			seqid = _pEnv->GetProtParse()->buildExchHeartBeat(resbuf);
			SendData(user._fd, (char*) &resbuf[0], resbuf.size());
			OUT_HEX(ip, port, "NOOP", (char* )&resbuf[0], resbuf.size());

			msgbuf.resize(sizeof(DbValue) + resbuf.size());
			dbval = (DbValue*) &msgbuf[0];
			dbval->cnt = 0;
			memcpy(dbval->dat, &resbuf[0], resbuf.size());

			share::Guard guare(_exch_pack_mutex);
			_exch_pack_tqueue.insert(seqid);
			tchdbput(_exch_pack_tchdb, &seqid, sizeof(int), &msgbuf[0], msgbuf.size());
		} else if (!ConnectServer(user, 30)) {
			OUT_INFO(ip, port, NULL, "connect server failure");
		}

		sleep(30);
	}
}

void ExchClient::on_data_arrived(socket_t *sock, const void *data, int len)
{
	const char *ip = sock->_szIp;
	unsigned int port = sock->_port;
	OUT_HEX(ip, port, "RECV", (char* )data, len);

	size_t msglen;
	uint8_t *msgptr;
	vector<uint8_t> msgbuf;

	msgbuf.resize(len);
	msgptr = &msgbuf[0];
	msglen = deCode((uint8_t*) data, len, &msgbuf[0]);

	if (msglen < sizeof(ExchHead) + sizeof(ExchTail)) {
		return;
	}
	_client_user._last_active_time = time(NULL);

	ExchHead *headPtr;
	uint32_t srcid; // ��Ϣ��Դ
	uint32_t dstid; // ��ϢĿ�ĵ�
	uint16_t msgid; // ��Ϣ����
	uint32_t seqid; // ��Ϣ���к�
	uint32_t length; // ��Ϣ�峤��

	//IRedisCache *redis = _pEnv->GetRedisCache();

	headPtr = (ExchHead*) msgptr;
	srcid = ntohl(headPtr->srcid);
	dstid = ntohl(headPtr->dstid);
	msgid = ntohs(headPtr->msgid);
	seqid = ntohl(headPtr->seqid);
	length = ntohl(headPtr->length);

	if(msgid == 0xc001) {
		ExchGenRsp *genRsp; //ͨ��Ӧ��
		uint32_t rspSeq;    //Ӧ�����к�
		uint8_t rspRes;     //Ӧ����
		DbValue *dbvalptr;
		int dbvallen;

		size_t rspMsglen;
		uint8_t *rspMsgptr;
		vector<uint8_t> rspMsgbuf;

		uint32_t newSeqid;
		vector<uint8_t> newMsgbuf;

		uint16_t rspMsgid; //��Ϣid
		uint16_t rspCmdid; //����id

		genRsp = (ExchGenRsp*)(msgptr + sizeof(ExchHead));
		rspSeq = ntohl(genRsp->seq);
		rspRes = genRsp->res;

		share::Guard guare(_exch_pack_mutex);

		if (!_exch_pack_tqueue.erase(rspSeq)) { //�ӳ�ʱ������ɾ��
			OUT_INFO(ip, port, "REPLY", "in tqueue, seqid %u not found", rspSeq);
		}

		if ((dbvalptr = (DbValue*) tchdbget(_exch_pack_tchdb, &rspSeq, sizeof(int), &dbvallen)) == NULL) {
			OUT_INFO(ip, port, "REPLY", "in tchdb, seqid %u not found", rspSeq);
			return;
		}
		tchdbout(_exch_pack_tchdb, &rspSeq, sizeof(int)); // ��tchdb�����ļ���ɾ��

		rspMsgbuf.resize(dbvallen);
		rspMsgptr = &rspMsgbuf[0];
		rspMsglen = deCode(dbvalptr->dat, dbvallen - sizeof(short), rspMsgptr);
		free(dbvalptr); //�ͷ�tchdb�������ڴ����

		ExchHead *rspHeadPtr;
		ExchPlatHead *rspPlatPtr;
		rspHeadPtr = (ExchHead*) rspMsgptr;
		rspMsgid = ntohs(rspHeadPtr->msgid);
		if(rspMsgid != 0x1100) {
			return;
		}

		rspPlatPtr = (ExchPlatHead*)(rspMsgptr + sizeof(ExchHead));
		rspCmdid = ntohs(rspPlatPtr->msgid);
		if(rspCmdid != 0x1101) {
			return;
		}

		if(rspRes != 0) { //��¼ʧ��
			_client_user._user_state = User::OFF_LINE;
			OUT_INFO(ip, port, "STAT", "login fail");
			CloseSocket(_client_user._fd);
			return;
		}

		_client_user._user_state = User::ON_LINE;
		OUT_INFO(ip, port, "STAT", "login succ");

		//�������б���������
		newMsgbuf.clear();
		newSeqid = _pEnv->GetProtParse()->buildExchSubUnits(newMsgbuf);
		SendData(sock, (char*)&newMsgbuf[0], newMsgbuf.size());
		OUT_HEX(ip, port, "SEND", (char*)&newMsgbuf[0], newMsgbuf.size());

		//��������������������
		newMsgbuf.clear();
		newSeqid = _pEnv->GetProtParse()->buildExchSubCmdid(newMsgbuf);
		SendData(sock, (char*) &newMsgbuf[0], newMsgbuf.size());
		OUT_HEX(ip, port, "SEND", (char*)&newMsgbuf[0], newMsgbuf.size());
	}
}

void ExchClient::on_dis_connection(socket_t *sock)
{
	//ר�Ŵ���ײ����·ͻȻ�Ͽ��������������ʱ�����������µĶϿ������
	OUT_INFO( sock->_szIp, sock->_port, "Conn", "Recv disconnect fd %d", sock->_fd ) ;

	_client_user._fd = NULL;
	_client_user._user_state  = User::OFF_LINE ;
}

int ExchClient::build_login_msg( User &user, char *buf,int buf_len )
{
	vector<uint8_t> msgbuf;
	DbValue *dbval;
	uint32_t seqid;
	vector<uint8_t> resbuf;

	seqid = _pEnv->GetProtParse()->buildExchLogin(user._user_name, user._user_pwd, resbuf);
	copy(resbuf.begin(), resbuf.end(), (uint8_t*)buf);

	msgbuf.resize(sizeof(DbValue) + resbuf.size());
	dbval = (DbValue*)&msgbuf[0];
	dbval->cnt = 0;
	memcpy(dbval->dat, &resbuf[0], resbuf.size());

	share::Guard guare(_exch_pack_mutex);
	_exch_pack_tqueue.insert(seqid);
	tchdbput(_exch_pack_tchdb, &seqid, sizeof(int), &msgbuf[0], msgbuf.size());

	return resbuf.size();
}

bool ExchClient::HandleData(uint32_t seqid, const char *data, int len)
{
	const char *ip = _client_user._ip.c_str();
	unsigned int port = _client_user._port;

	vector<uint8_t> msgbuf;
	DbValue *dbval;

	msgbuf.resize(sizeof(DbValue) + len);
	dbval = (DbValue*) &msgbuf[0];
	dbval->cnt = _exch_pack_retries;
	memcpy(dbval->dat, data, len);

	share::Guard guare(_exch_pack_mutex);
	if (_exch_pack_tqueue.insert(seqid)) {
		tchdbput(_exch_pack_tchdb, &seqid, sizeof(int), &msgbuf[0], msgbuf.size());
	}

	if (_client_user._fd == NULL || _client_user._user_state != User::ON_LINE) {
		OUT_HEX(ip, port, "OFFLINE", data, len);
		return false;
	}

	OUT_HEX(ip, port, "SEND", data, len);
	return SendData(_client_user._fd, data, len);
}

size_t ExchClient::enCode(const uint8_t *src, size_t srclen, uint8_t *dst)
{
	return 0;
}
size_t ExchClient::deCode(const uint8_t *src, size_t srclen, uint8_t *dst)
{
	size_t dstlen;

	dst[0] = 0x5b;
	dstlen = _pEnv->GetProtParse()->deCode(src + 1, srclen - 2, dst + 1) + 2;
	dst[dstlen - 1] = 0x5d;

	return dstlen;
}

//bool ExchClient::p
