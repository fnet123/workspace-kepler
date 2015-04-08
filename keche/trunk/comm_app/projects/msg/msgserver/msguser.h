/*
 * msguser.h
 *
 *  Created on: 2011-11-10
 *      Author: humingqing
 */

#ifndef __MSGUSER_H__
#define __MSGUSER_H__

#include <map>
#include <string>
#include <Mutex.h>
using namespace std ;

class CMsgUser
{
	typedef std::map<std::string,std::string>  CMapUser ;
public:
	CMsgUser() ;
	~CMsgUser() ;

	// �����û�����
	bool LoadUser( const char *szfile ) ;
	// ����û��Ƿ��½
	int  CheckUser( const char *user ,const char *pwd ) ;
	// ����û���
	bool AddUser( const char *user , const char *pwd ) ;

private:
	// �û���
	share::Mutex 	       _mutex ;
	// �û���Ϣ
	CMapUser     		   _user_map ;
	// ��ʱ����Ϣ
	CMapUser			   _temp_user ;
};

#endif /* MSGUSER_H_ */
