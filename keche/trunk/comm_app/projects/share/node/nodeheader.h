/*
 * nodeheader.h
 *
 *  Created on: 2011-11-7
 *      Author: humingqing
 */

#ifndef __NODEHEADER_H__
#define __NODEHEADER_H__

#include <netorder.h>

#pragma pack(1)

#define NODE_CONNECT_REQ   		0x1001   // ����������
#define NODE_CONNECT_RSP   		0x8001   // ������Ӧ����
#define NODE_DISCONN_REQ   		0x1002   // ע��������Ϣ������
#define NODE_DISCONN_RSP   		0x8002   // ע��������Ӧ
#define NODE_LINKTEST_REQ		0x1003   // ��·������������
#define NODE_LINKTEST_RSP   	0x8003   // ��·����������Ӧ
#define NODE_USERNAME_REQ   	0x1004   // �����û���������
#define NODE_USERNAME_RSP   	0x8004   // ��Ӧ�����û���
#define NODE_USERNOTIFY_REQ 	0x1005   // �׷��û���������
#define NODE_USERNOTIFY_RSP     0x8005   // �׷��û�����Ӧ��
#define NODE_GETMSG_REQ         0x1006   // ȡ��MSG��������ַ
#define NODE_GETMSG_RSP 		0x8006   // ȡ��MSG��������ַ�б�
#define NODE_MSGERROR_REQ		0x1007   // MSG�쳣�����
#define NODE_MSGERROR_RSP		0x8007   // MSG�쳣����Ӧ
#define NODE_MSGCHG_REQ		    0x1008   // MSG���²�����Ҫ����MSG�������ص���
#define NODE_MSGCHG_RSP			0x8008   // MSG���²���Ӧ��

#define NODE_CTFO_TAG          "CTFO"
#define NODE_MSG_DEL			0		// ɾ��MSG������
#define NODE_MSG_ADD			1		// ���MSG������

// ���������Ҫ���������� PIPE,STORE,MSG
#define FD_NODE_WEB     		0x1000 // WEBģʽֻ���·�����
#define FD_NODE_PIPE			0x2000 // �ܵ�ģʽ˫��
#define FD_NODE_STORE   		0x4000 // �洢ģʽʲô������
#define FD_NODE_MSG				0x8000 // ΪMSG������ģʽ

// �ڵ�ͷ������
struct NodeHeader
{
	unsigned char tag[4] ;   // Ĭ�ϱ��CTFO
	unsigned short cmd ;     // ָ����
	unsigned int   seq ;	 // ���
	unsigned int   len ;	 // ����
};

// �ڵ��������ַ
struct AddrInfo
{
	char ip[32] ;   // IP ��ַ
	unsigned short port  ;   // �˿�
};

// ��½��Ϣ��
struct NodeLoginReq
{
	unsigned int   id ;    // ��½�ڵ��ʶ
	unsigned short group ; // �������ID 0x1000 WEB , 0x2000 PIPE , 0x4000 STROE , 0x8000 MSG
	AddrInfo       addr ;  // ��������ַ
};

// ��½Ӧ��
struct NodeLoginRsp
{
	unsigned char result ;  // ���ض�Ӧ�룬0�ɹ���1 ID����2 IP����ȷ��3 �˿ڲ���ȷ��4��������
};

// ע��Ӧ��
struct NodeLogoutRsp
{
	unsigned char result ;   // Ӧ�����ɹ�Ϊ0��1ʧ��
};

// ��·ά��
struct NodeLinkTestReq
{
	unsigned int num ;  // ���߳�����
};

// �û���Ϣ
struct UserInfo
{
	char user[12] ;  // �û���
	char pwd[8] ;    // ����
};

// ���سɹ���ӵ��û�
struct NodeUserNameRsp
{
	UserInfo user ;  // ���سɹ���ӵ��û�
};

// ����û�֪ͨ
struct NodeUserNotify
{
	unsigned short num ; // �������û�����
	// ����������Ϊ����û���Ϣ�ṹ��
};

// ����û���Ӧ
struct NodeUserNotifyRsp
{
	unsigned short success ;// ��ӳɹ����û�����
};

// ��ȡMSG�ķ������Ӧ
struct NodeGetMsgRsp
{
	unsigned short num ;  // �����������б��з�������Ϣ����
};

// �ڵ����Ĵ���
struct NodeErrorRsp
{
	unsigned char result ;// �ɹ�0��ʧ��1
};

// MSG�������֪ͨ
struct NodeMsgChg
{
	unsigned char  op ;  // ���� 0 ��ʾɾ����1��ʾ���
	unsigned short num ; // ��������������
};

#pragma pack()

#endif /* NODEHEADER_H_ */
