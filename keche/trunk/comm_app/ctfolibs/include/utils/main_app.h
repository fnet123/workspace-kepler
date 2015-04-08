/*=================================================================
** Copyright (c)2006,�ǰ���Ƽ��ɷ����޹�˾
** All rights reserved.
**
** �ļ����ƣ�main_app.h
**
** ��ǰ�汾��1.1
** ��    �ߣ�zhaojianbin
** ������ڣ�2006-11-24
**
===================================================================*/

#ifndef MAIN_APP_H
#define MAIN_APP_H

#include <getopt.h>

extern void HandleExitSignal(int sig);
extern void HandleHupSignal(int sig);
extern void HandleUsr1Signal(int sig);
extern void HandleUsr2Signal(int sig);

class MainApp
{
    friend void HandleExitSignal(int sig);
    friend void HandleHupSignal(int sig);
    friend void HandleUsr1Signal(int sig);        
    friend void HandleUsr2Signal(int sig);  
    
public:
    MainApp( const char *ver= "4.1.0_20121116_01" ,
    		const char *help = "running at stay back, using param: -daemon\n\t -v view current version\n" );
    virtual ~MainApp(); 
    // ��ʼ�������б�
    int InitMainApp( int argc, char** argv );
    // ȡ�ý����Ĳ���
    const char * getArgv( const char *key ) ;

    void OnExit();
    virtual void OnHup() {}
    virtual int  Usage( int argc, char **argv ) ;
    virtual void OnUsr1Sig() {}
    virtual void OnUsr2Sig() {}
	
protected:
    void StayBack();    
	char* GetAppName(char* str);
    
private:
    static MainApp* _appInstance;
    // ���̺�
    int         _myPid;
    // ��������
    char 		_appName[256] ;
    // ��Ӱ汾�Ź���
    char 		_version[256] ;
    // ������Ϣ��ʾ
    char 		_help[10240] ;
};

#endif // MAIN_APP_H

