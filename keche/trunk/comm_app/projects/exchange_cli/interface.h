/**********************************************
 * interface.h
 *
 *  Created on: 2014-05-15
 *      Author: ycq
 *    Comments: ��������ӿ��ඨ�壬��Ҫʵ��������֮�佻���Ľӿڶ���
 *********************************************/
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <icontext.h>
#include <icache.h>

class ISystemEnv ;

#include <set>
using std::set;
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include <stdint.h>

class IAmqClient {
public:
	virtual ~IAmqClient() = 0;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0;
	// ��ʼ
	virtual bool Start(void) = 0;
	// ֹͣ
	virtual void Stop(void) = 0;
};

// MSG���ݴ�����
class IMsgClient {
public:
	virtual ~IMsgClient() = 0;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0;
	// ��ʼ
	virtual bool Start(void) = 0;
	// ֹͣ
	virtual void Stop(void) = 0;
	// �������ݴ�����
	virtual bool HandleData(const char *data, int len) = 0;
};

// �������ͷ���
class IExchClient {
public:
	virtual ~IExchClient() = 0;
	// ��ʼ��
	virtual bool Init(ISystemEnv *pEnv) = 0;
	// ��ʼ
	virtual bool Start(void) = 0;
	// ֹͣ
	virtual void Stop(void) = 0;
	// �������ݴ�����
	virtual bool HandleData(uint32_t seqid, const char *data, int len) = 0;
};

//
class IProtParse {
public:
	virtual ~IProtParse() = 0;

	virtual bool Init(ISystemEnv *pEnv) = 0;

	virtual size_t enCode(const unsigned char *src, size_t len, unsigned char *dst) = 0;
	virtual size_t deCode(const unsigned char *src, size_t len, unsigned char *dst) = 0;

	virtual bool parseInnerParam(const string &detail, map<string, string> &params) = 0;

	virtual uint32_t buildExchLogin(const string &un, const string &pw, vector<unsigned char> &resBuf) = 0;
	virtual uint32_t buildExchHeartBeat(vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchSubUnits(vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchSubCmdid(vector<unsigned char> &resbuf) = 0;

	virtual uint32_t buildExchStatus(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchTermReg(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchTermAuth(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchLocation(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchHistory(const string &num, const map<string,string> &params, vector<unsigned char> &resBuf) = 0;
	virtual uint32_t buildExchMMData(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchMMEvent(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchDriverEvent(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchTermVersion(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;
	virtual uint32_t buildExchTTData(const string &num, const map<string,string> &params, vector<unsigned char> &resbuf) = 0;

	virtual uint32_t buildExchBaseTable(const string &str, uint16_t msgid, vector<uint8_t> &resbuf) = 0;
};

// ��������ָ��
class ISystemEnv: public IContext {
public:
	virtual ~ISystemEnv() = 0;
	// ȡ��MSG�Ĳ���
	virtual IMsgClient * GetMsgClient(void) = 0;
	//
	virtual IExchClient * GetExchClient(void) = 0;
	//
	virtual IProtParse  * GetProtParse(void) = 0;
	// ȡ��RedisCache
	virtual IRedisCache * GetRedisCache(void) = 0;
};

#endif
