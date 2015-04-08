/******************************************************
 *  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
 *   FileName: alertanalyse.h
 *     Author: liubo  2012-10-31 
 *Description: �¼���������������������ʻ��Ϊ�¼��ȵĿ�ʼʱ�����ʱ�䡣
 *******************************************************/

#ifndef EVENTANALYSE_H_
#define EVENTANALYSE_H_

#include <list>
#include <map>

#define ALERT808   0
#define ALERT808b  1

using namespace std;

class AlertInfo
{
public:
	time_t begin_time;
	time_t end_time;

	AlertInfo():begin_time(0), end_time(0){}

	void reset()
	{
		begin_time = end_time = 0;
	}
};

class AlertEvent
{
public:
	int event;
	unsigned int single_alert_id; /* ��һ������ID */
	AlertInfo alert_info;
};

class AlertValue
{
public:
	AlertValue(): bit_map808(0), bit_map808b(0){}
	~AlertValue(){}
	list<AlertEvent> check(unsigned int alert_id, int type, time_t gps_time);

private:
	/*****************************
	 * �����ڵı�����ʶλ���������õ�change_map��map808�е�������
	 * map808�У�0��ʾ���λ�ı�����31��ʾ���λ�ı�����
	 *****************************/
	bool bit_value(unsigned int bit_map, unsigned int index)
	{
		return (bit_map & (0x80000000 >> index)) != 0;
	}
	//ȡ��map��index��Ӧ�ı���ֵ��
	unsigned int alert_value(unsigned int index)
	{
		return 0x80000000 >> index;
	}

private:
    //���ڵı���λͼ, �洢����һ�εı���ֵ��
	unsigned  int bit_map808;
	AlertInfo map808[32];

	//808bЭ��Ϊ��չ�����ڵı������Ҳ�ʵ�֡�
	unsigned  int bit_map808b;
	AlertInfo map808b[32];
};

class AlertAnalyse
{
public:
	AlertAnalyse(){}
	~AlertAnalyse(){}
    list<AlertEvent> check_alert(long long car_id,
    		int type, unsigned int alert_id, time_t gps_time);
private:
    map<long long , AlertValue> _map_car_alert;
};

#endif /* ALERTANALYSE_H_ */
