/******************************************************
*  CopyRight: �����н���·�Ƽ����޹�˾(2012-2015)
*   FileName: saverutil.cpp
*     Author: liubo  2012-7-23
*Description:
*******************************************************/
#include "saverutil.h"

#define OUTLEN 255


#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

/* interestingly, we assume leap-years */
static int month[12] = {
    0,
    DAY*(31),
    DAY*(31+29),
    DAY*(31+29+31),
    DAY*(31+29+31+30),
    DAY*(31+29+31+30+31),
    DAY*(31+29+31+30+31+30),
    DAY*(31+29+31+30+31+30+31),
    DAY*(31+29+31+30+31+30+31+31),
    DAY*(31+29+31+30+31+30+31+31+30),
    DAY*(31+29+31+30+31+30+31+31+30+31),
    DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

long kernel_mktime(struct tm * tm)
{
    long res;
    int year;

    year = tm->tm_year - 70;

/* magic offsets (y+1) needed to get leapyears right.*/

    res = YEAR*year + DAY*((year+1)/4);
    res += month[tm->tm_mon];

/* and (y+2) here. If it wasn't a leap-year, we have to adjust */

    if (tm->tm_mon>1 && ((year+2)%4))
        res -= DAY;
    res += DAY*(tm->tm_mday-1);
    res += HOUR*tm->tm_hour;
    res += MINUTE*tm->tm_min;
    res += tm->tm_sec;

    //��ȥ�˸�Сʱ��ʱ��
    return res - (8 * 3600);
}

//��str split��list����ʽ�� StrInfoֻ��ָ��ÿһ���ֶε�λ�ú�ƫ�����������޸�ԭʼ���ݵ�ֵ��
bool split2list(const char *data, int data_len, std::list<StrInfo> &list,  const char *split)
{
	int dlen = data_len;
    int slen = strlen(split);
    int offset = 0;

    const  char *pos = strstr(data + offset, split);
    if(pos == NULL)
    	return false;

    StrInfo str_info;
    //������һ��split.
    while(offset < dlen && (NULL != (pos = strstr(data + offset, split))))
    {
    	str_info.pos = data + offset;
    	str_info.offset = pos - data - offset;
        offset += (pos - data - offset + slen);
        list.push_back(str_info);
    }

    str_info.pos = data + offset;
    str_info.offset =  dlen - offset;
    list.push_back(str_info);

    return true;
}

/********************************
 * Input: StrInfo �ָ�����ֶ�Ϊ  "key:value" ���ַ�����ʽ��
 * Output: �����map, map�е�string string Ϊ�¹���ģ�
 *****************************/
int split2map(std::list<StrInfo> &list, map<string , string> &kv_map, const char *split)
{
	kv_map.clear();
    int ret = 0;
    int slen = strlen(split);
    //	int count = 0;
	map<string, string>::iterator it;
	std::list<StrInfo>::iterator iter = list.begin();

	const char *pos = NULL;
	for (; iter != list.end(); ++iter)
	{
		if ((pos = strstr(iter->pos, split)) == NULL)
			continue;
		// ���������key:value�Ĺ��򻰾ͻ�����coredump
		int n = iter->offset - (pos - iter->pos) - slen ;
		if ( n <= 0 ) continue ;

		string key(iter->pos, pos - iter->pos) ;
		string value(pos + slen, n ) ;

		it = kv_map.find(key);
		if (it != kv_map.end())
		{ // ������ڶ��ͬ�����д���
			it->second += "|";
			it->second += value;
		}
		else
		{
			kv_map.insert(make_pair(key, value));
			ret ++;
		}
	}
	return ret;
}
