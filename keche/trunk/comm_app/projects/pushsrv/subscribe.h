/*
 * Author     : yuanchunqing
 * Email      : yuanchunqing@ctfo.com
 * Date       : 2012-5-3
 * Description: �Ự���������û���ע���ն�ӳ�����
 *
 */

#ifndef __SUBSCRIBE_H__
#define __SUBSCRIBE_H__ 1

#include "Mutex.h"

#include <string>
#include <map>
#include <set>

using std::string;
using std::map;
using std::set;

struct NODE {
	set<string> def;
	set<string> sub;
};

class Subscribe {
	share::Mutex _mutex;
	map<string, NODE > _user2macid;      //stl����Ĭ�Ϸ�����ʹ�öѿռ�
	map<string, NODE > _macid2user;      //stl����Ĭ�Ϸ�����ʹ�öѿռ�
public:
	Subscribe();
	~Subscribe();

	//��ҵƽ̨����Ա��½ʱ��Ĭ�϶���Ȩ���������ն�
	bool regUser(const string &user, set<string> &macids);

	//��ҵƽ̨����Ա�˳�ʱ��ȡ�����ж���
	bool unregUser(const string &user);

	//���߹���Ա����ʱ���ö��ģ��ۼ�֮ǰ����
	bool apped(const string &user, const string &macid);

	//ɾ�������ն˶���
	bool erase(const string &user, const string &macid);

	//���߹���Ա����ʱ���ö��ģ�����֮ǰ����
	bool update(const string &user, const set<string> &macids);

	//�յ������ն˽�����Ϣʱ�����ҹ��������û�
	bool getDefUser(const string &macid, set<string> &users);

	//�յ������ն�һ����Ϣʱ�����Ҷ����û�
	bool getSubUser(const string &macid, set<string> &users);
};

#endif//__SUBSCRIBE_H__
