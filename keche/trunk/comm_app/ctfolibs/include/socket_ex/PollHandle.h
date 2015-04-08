/*
 * PollHandle.h
 *
 *  Created on: 2011-7-1
 *      Author: humingqing
 *
 *  ����������������ж���ʹ�����л�����ʽ������poll��
 *  ����poll����Ҫ����ͬ������
 */

#ifndef __POLLHANDLE_H__
#define __POLLHANDLE_H__

#include <SocketHandle.h>

#ifdef _USE_POLL
#include <poll.h>
#include <map>
#include <vector>

//  Pollset to pass to the poll function.
typedef std::vector<pollfd>   pollset_t ;
typedef std::vector<socket_t*> vecsocket_t ;

class pollarray_t ;
class CPollHandle : public CSocketHandle
{
public:
	CPollHandle();
	virtual ~CPollHandle();

public:
	//����
    bool create(int max_socket_num = MAX_SOCKET_NUM);
	bool destroy();
	//����fd
    bool add( socket_t *sock , unsigned int events);
	bool del( socket_t *sock , unsigned int events);
	bool modify( socket_t *sock, unsigned int events);
	int  poll(int timeout = 5000);

    virtual void on_event( socket_t *sock ,int events ){}

	virtual bool is_read(int events);
	virtual bool is_write(int events);
	virtual bool is_excep(int events);

private:
	// ȡ�õ�ǰ����FD
	int pollarray( pollset_t &fdarray , vecsocket_t &vec ) ;

private:
	// FD����
	pollarray_t   *_pollarray ;
};
#endif

#endif /* POLLHANDLE_H_ */
