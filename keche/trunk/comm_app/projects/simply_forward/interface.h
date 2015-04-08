#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <icontext.h>

class ISystemEnv ;

#include <utility>
using std::pair;
#include <string>
using std::string;
#include <vector>
using std::vector;

// tcpת�������
class ITcpServer {
public:
	virtual ~ITcpServer() {} ;
	// ��ʼ��
	virtual bool Init(int port, const vector<string> &addrs) = 0;
	// ��ʼ
	virtual bool Start( void ) = 0;
	// ֹͣ
	virtual void Stop( void ) = 0;
	// ɾ������ͨ��
	virtual bool ChkChannel(const string &userid) = 0;
	// �ύ���ݴ���
	virtual bool HandleData(const string &userid, const void *data, int len) = 0;
};

// tcpת���ͻ���
class ITcpClient
{
public:
	virtual ~ITcpClient() {} ;
	// ��ʼ��
	virtual bool Init(void) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0 ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
	// ��������ͨ��
	virtual bool AddChannel(ITcpServer *svr, const string &userid, const char *ip, int port) = 0;
	// ɾ������ͨ��
	virtual bool DelChannel(const string &userid) = 0;
	// �ύ���ݴ���
	virtual bool HandleData(const string &userid, const void *data, int len) = 0;
};

// udpת�������
class IUdpServer {
public:
	virtual ~IUdpServer() {} ;
	// ��ʼ��
	virtual bool Init(int port, const vector<string> &addrs) = 0;
	// ��ʼ
	virtual bool Start( void ) = 0;
	// ֹͣ
	virtual void Stop( void ) = 0;
	// ɾ������ͨ��
	virtual bool ChkChannel(const string &userid) = 0;
	// �ύ���ݴ���
	virtual bool HandleData(const string &userid, const void *data, int len) = 0;
};

// udpת���ͻ���
class IUdpClient
{
public:
	virtual ~IUdpClient() {} ;
	// ��ʼ��
	virtual bool Init(void) = 0 ;
	// ��ʼ
	virtual bool Start( void ) = 0 ;
	// ֹͣ
	virtual void Stop( void ) = 0 ;
	// ��������ͨ��
	virtual bool AddChannel(IUdpServer *svr, const string &userid, const char *ip, int port) = 0;
	// ɾ������ͨ��
	virtual bool DelChannel(const string &userid) = 0;
	// �ύ���ݴ���
	virtual bool HandleData(const string &userid, const void *data, int len) = 0;
};

// ��������ָ��
class ISystemEnv : public IContext
{
public:
	virtual ~ISystemEnv() {} ;
	// ȡ��tcpת���ͻ���
	virtual ITcpClient * GetTcpClient( void ) =  0;
	// ȡ��udpת���ͻ���
	virtual IUdpClient * GetUdpClient( void ) =  0;
};

#endif
