#include <comlog.h>
#include <main_app.h>
#include <utility.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "systemenv.h"
#include "simutil.h"
#include <stdlib.h>

int main(int argc, char**argv)
{
	MainApp app;
//	���ʱ�ǵ���Ϊ���Զ��رյĵȴ�������Ӧ������
	app.InitMainApp(argc, argv);

	char szlog[1024] = {0} ;
	getrunpath( MTRANS_ENV, szlog, ABS_LOG_PATH , LOG_PATH ) ;

	char szconf[1024] = {0} ;
	getconfpath( MTRANS_ENV, szconf, ABS_CONF_PATH , CONF_PATH , CONFIG ) ;

	// ����и�������Ӵ���
	if ( argc > 1 ) {
		if ( strstr(argv[1] , "-") == NULL )
		sprintf( szconf, "%s" , argv[1] ) ;
	}
	// ���ָ����־·��
	if ( argc > 2 ) {
		if ( strstr(argv[2] , "-") == NULL )
			sprintf( szlog, "%s" , argv[2] ) ;
	}

	printf( "argc: %d, conf: %s, logpath: %s\n" , argc, szconf, szlog ) ;

	char szuser[1024] = {0};
	getconfpath( MTRANS_ENV, szuser, ABS_CONF_PATH , CONF_PATH , USER_INFO_FILE ) ;

	CSystemEnv env ;
	if ( ! env.Init( szconf, szlog , szuser ) )
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


//	pipe_server.Stop();
	sleep(1);
	
	INFO_PRT("press enter to leave!");
	getchar();
	
	return 0;
}




