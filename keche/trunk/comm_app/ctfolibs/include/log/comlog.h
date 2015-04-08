/**
 * author: Liubo
 * date: 2011-09-23
 * memo: <humingqing> ��־�������,��־���Ը��ݼ����������Ƿ�Ҫ�����������Խ������ֵԽС
 * 		�����ü���Խ��ʱ��ʾ��¼����־��Խ�٣���Ϊ��ʱ�ر���־,1����־ֻΪ����;��棬2��Ϊһ�����������̣�3��Ϊ���Լ���
 */
#ifndef _HEAD_COMLOG_H
#define _HEAD_COMLOG_H

#include "cLog.h"

#ifdef _XDEBUG
#define OUT_DEBUG( fmt, ... )  		printf( fmt, ## __VA_ARGS__ )
#define DEBUG_PRT(fmt, ...)			debug_printf(__FILE__, __LINE__, fmt, ## __VA_ARGS__)
#else
#define OUT_DEBUG( fmt, ... )
#define DEBUG_PRT(fmt, ...)
#endif
#define INFO_PRT(fmt, ...)			info_printf(fmt, ## __VA_ARGS__)

#define CHGLOG(pathname)     CCLog::GetInstance()->set_log_file(pathname);
#define CHGLOGSIZE(newsize)  CCLog::GetInstance()->set_log_size(newsize);
#define CHGLOGNUM(log_num)   CCLog::GetInstance()->set_log_num(log_num);
#define FFLUSH
#define SETLOG(pathname)     CCLog::GetInstance()->set_log_file(pathname);
#define SETLOGSIZE(newsize)  CCLog::GetInstance()->set_log_size(newsize);
#define SETLOGNUM(log_num)   CCLog::GetInstance()->set_log_num(log_num);
#define SETLOGLEVEL(level)   CCLog::GetInstance()->set_log_level(level);
#define LOG_NUM_LEVEL(level) level, __FILE__, __LINE__, __FUNCTION__
#define LOG_NULL_LEVEL(level) level, NULL, 0, NULL

// һ����־�����󱨾���������Ҫ��¼������ֵ������ļ��к��Լ���������
#define OUT_WARNING(ip,port,user_id,msg,...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NUM_LEVEL(1),"WARN",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_ERROR(ip,port,user_id,msg,...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NUM_LEVEL(1),"ERROR",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_ASSERT(ip,port,user_id,msg,...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NUM_LEVEL(1),"ASSERT",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_RUNNING(ip,port,user_id,msg,...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NUM_LEVEL(1),"RUNNING",ip,port,user_id,msg,  ## __VA_ARGS__)

// ������־�����̼�����
#define OUT_INFO( ip, port, user_id, msg, ...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(2),"INFO",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_RECV( ip, port, user_id, msg, ...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(2),"RECV",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_SEND( ip, port, user_id, msg, ...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(2),"SEND",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_CONN( ip, port, user_id, msg, ...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(2),"CONN",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_DISCONN( ip, port, user_id, msg, ...)\
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(2),"DISCON",ip,port,user_id,msg,  ## __VA_ARGS__)

// ������־Ϊ���Դ���
#define OUT_RECV3( ip, port, user_id, msg, ...) \
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(3),"RECV",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_SEND3( ip, port, user_id, msg, ... ) \
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(3),"SEND",ip,port,user_id,msg,  ## __VA_ARGS__)
#define OUT_PRINT( ip, port, user_id,msg, ... ) \
		CCLog::GetInstance()->CCLog::print_net_msg(LOG_NULL_LEVEL(3),"PRINT",ip,port,user_id,msg,  ## __VA_ARGS__)
// ���ʮ����������
#define OUT_HEX( ip, port, user_id, data, len )  \
		CCLog::GetInstance()->print_net_hex(LOG_NULL_LEVEL(3), ip, port, user_id, data, len )


#endif  // #ifndef _HEAD_COMLOG_H


