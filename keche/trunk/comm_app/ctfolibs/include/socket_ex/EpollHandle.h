/**
  * Name: 
  * Copyright: 
  * Author: lizp.net@gmail.com
  * Date: 2009-11-3 ���� 3:40:55
  * Description: �첽�¼��࣬˼������zhhg
  * Modification: 
  **/

#ifndef __EPOLL_HANDLE_H__
#define __EPOLL_HANDLE_H__

#ifndef _UNIX
#include <sys/epoll.h>
#endif

#include "SocketHandle.h"

#define EPOLL_EVENTS_NUM	1024

#ifndef _UNIX
class CEpollHandle : public CSocketHandle
{
public:
	CEpollHandle();
	virtual ~CEpollHandle();
public:	
	//����
    bool create(int max_socket_num = MAX_SOCKET_NUM);	
	bool destroy();

    bool add(socket_t *fd, unsigned int events);
	bool del(socket_t *fd, unsigned int events);
	bool modify(socket_t *fd, unsigned int events);
	int  poll(int timeout = 5000);			

    virtual void on_event(socket_t *fd,int events){}

	virtual bool is_read(int events);
	virtual bool is_write(int events);
	virtual bool is_excep(int events);

private:
	int	 ctl(int op, int fd, struct epoll_event &event);

private:
    int 		  _handle;
};
#endif

#endif


