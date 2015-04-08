/**********************************************
 * pccmain.cpp
 *
 *  Created on: 2011-08-11
 *      Author: humingqing
 *    Comments:
 *********************************************/
#include "main_app.h"
#include "utility.h"
#include "systemenv.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "pccutil.h"

#define CONFIG  			"pcc_special.conf"
#define LOG_PATH 			"/var/lbs/log/ws"
#define CONF_PATH   		"/usr/local/lbs/conf/ws"
#define MTRANS_ENV			"MTRANS_PRJ_HOME"
#define ABS_LOG_PATH		"/lbs/log/ws"
#define ABS_CONF_PATH		"/lbs/conf/ws"
#define LOG_NAME			"pcc_special.log"
#define RUN_LOG_PATH		"log/ws"
#define RUN_CONF_PATH		"conf/ws"
#define PCCV4_VERSION  		"V4.1.1_20121127_01"

int main(int argc, char**argv)
{
	MainApp app( PCCV4_VERSION ) ;
//	���ʱ�ǵ���Ϊ���Զ��رյĵȴ�������Ӧ������
	app.InitMainApp(argc, argv);
//	signal(SIGPIPE, SIG_IGN);
//	signal(SIGPIPE, SignalPipe ) ;

	const char *szrun = app.getArgv( "r" ) ;
	char szlog[1024] = {0} ;
	if ( szrun != NULL )
		sprintf( szlog, "%s/%s", szrun, RUN_LOG_PATH ) ;
	else
		getrunpath( MTRANS_ENV, szlog, ABS_LOG_PATH , LOG_PATH ) ;

	char szconf[1024] = {0} ;
	if ( szrun != NULL )
		sprintf( szconf, "%s/%s/%s", szrun, RUN_CONF_PATH , CONFIG ) ;
	else
		getconfpath( MTRANS_ENV, szconf, ABS_CONF_PATH , CONF_PATH , CONFIG ) ;

	printf( "run conf file path:%s , log path: %s\n" , szconf, szlog ) ;

	CSystemEnv env ;
	if ( ! env.Init( szconf, szlog , "", LOG_NAME ) )
	{
		printf( "CSystemEnv init failed\n" ) ;
		return 0 ;
	}

	if ( ! env.Start() )
	{
		printf( "CSystemEnv start failed\n" ) ;
		return 0 ;
	}

	while (1)
	{
		usleep(1000*1000*1000);
	}

	sleep(1);
//	INFO_PRT("press enter to leave!");
//	getchar();

	return 0;
}







