/*
 * CFileServer.cpp
 *
 *  Created on: 2012-4-28
 *      Author: humingqing
 *  ��Ӽ򵥷�����ͨ�ſ��
 */

#include "synserver.h"
#include <comlog.h>
#include <stdlib.h>
#include <dbpool.h>
#include <tools.h>

// ת��ʮ������
static unsigned char hexvalue( char c )
{
	if ( c >= 48 && c <= 57 ) {  // 0-9
		return c - 48 ;
	}
	if ( c >= 97 && c <= 102 ) { // a-f
		return c - 87 ;
	}
	if ( c >= 65 && c <= 70 ) {  // A-F
		return c - 55 ;
	}
	return 0 ;
}

static int hex2str( const char *src, char *dest )
{
	if ( src == NULL ) {
		dest[0] = 0 ;
		return 0;
	}

	int pos = 0 ;
	int i   = 0 ;
	int len = (int) strlen(src) ;
	// ת��ʵ������
	unsigned char c = 0 ;
	for ( i = 0; i < len; ++i ) {
		if ( i % 2 == 0 ) {
			c = hexvalue( src[i] ) & 0xff ;
		}  else  {
			c = c << 4 ;
			c |= hexvalue( src[i] ) ;
			dest[pos++] = (char) c ;
		}
	}
	dest[pos] = 0 ;
	return pos ;
}

// ���ر���ת��UTF-8
/**
static bool locale2utf8( const char *szdata, const int nlen , string &out )
{
	int   len = nlen*4 + 1 ;
	char *buf = new char[ len ] ;
	memset( buf, 0 , len ) ;

	if( g2u( (char *)szdata , nlen , buf, len ) == -1 ){
		OUT_ERROR( NULL, 0, NULL , "locale2utf8 query %s failed" , szdata ) ;
		delete [] buf ;
		return false ;
	}
	buf[len] = 0 ;
	out.assign( buf , len ) ;
	delete [] buf ;

	return true ;
}*/

static const char *hex2utf8( const char *src, char *dest )
{
	char sz[256] = {0} ;
	int nsrc = hex2str( src, sz ) ;

	int len = 1024 ;
	if( g2u( (char *)sz , nsrc , dest , len ) == -1 ){
		OUT_ERROR( NULL, 0, NULL , "locale2utf8 query %s failed" , sz ) ;
		return NULL ;
	}
	dest[len] = 0 ;
	return dest ;
}

CSynServer::CSynServer()
	:_inited(false),_synmode(0x01),_syntime(180)
{
	_dbpool = new CDbPool;
}

CSynServer::~CSynServer(void)
{
	Stop();

	if ( _dbpool != NULL ) {
		delete _dbpool ;
		_dbpool = NULL ;
	}
}

bool CSynServer::Init(ISystemEnv *pEnv)
{
	_pEnv = pEnv;

	char szip[128] = {0} ;
	if ( ! pEnv->GetString( "db_ip", szip ) ) {
		OUT_ERROR( NULL, 0, NULL, "get database ip failed" ) ;
		return false ;
	}

	char szport[128] = {0} ;
	if ( ! pEnv->GetString( "db_port", szport ) ) {
		OUT_ERROR( NULL, 0, NULL, "get database port failed" ) ;
		return false ;
	}

	char szuser[128] = {0} ;
	if ( ! pEnv->GetString( "db_user", szuser ) ) {
		OUT_ERROR( NULL, 0, NULL, "get database user failed" ) ;
		return false ;
	}

	char szpwd[128] = {0} ;
	if ( ! pEnv->GetString( "db_pwd", szpwd ) ) {
		OUT_ERROR( NULL, 0, NULL, "get database pwd failed" ) ;
		return false ;
	}

	char szdb[128] = {0} ;
	if ( ! pEnv->GetString( "db_name", szdb ) ) {
		OUT_ERROR( NULL, 0 , NULL, "get database name failed" ) ;
		return false ;
	}
	sprintf( _szdb, "type=oracle;ip=%s;port=%s;user=%s;pwd=%s;db=%s",
			szip, szport, szuser, szpwd, szdb ) ;

	int nvalue = 0 ;
	// ����ͬ����ʽ
	if ( pEnv->GetInteger( "syn_mode", nvalue ) ) {
		_synmode = nvalue ;
	}
	// ȡ����Ҫͬ�����µ�ʱ��
	if ( pEnv->GetInteger( "syn_time", nvalue ) ) {
		_syntime = nvalue ;
	}

	if ( ! _thpool.init( 1, NULL, this ) ) {
		OUT_ERROR( NULL, 0, NULL , "init syn thread failed" ) ;
		return false ;
	}
	_inited = true ;

	if ( ! _dbpool->Init() ) {
		OUT_ERROR( NULL, 0, NULL, "init database pool failed" ) ;
		return false ;
	}

	return true ;
}

bool CSynServer::Start(void)
{
	_thpool.start() ;

	if ( ! _dbpool->Start() ){
		OUT_ERROR( NULL, 0, NULL,  "start database pool failed" ) ;
		return false ;
	}
	return true ;
}

// ����STOP����
void CSynServer::Stop(void)
{
	_inited = false ;

	_thpool.stop() ;
	_dbpool->Stop() ;
}

// �߳����ж���
void CSynServer::run( void *param )
{
	time_t last = 0 ;
	while( _inited ) {
		time_t now = time(NULL) ;
		if ( now - last > _syntime ) {
			last = now ;
			SynData()  ;
			OUT_RUNNING( NULL, 0, NULL, "run syn data span time %d", time(NULL) - now ) ;
		}
		sleep(1) ;
	}
}

// ���³��������ݵ�SQL
static const char *gvechile_sql = "SELECT sm.commaddr,sm.ent_id,ve.plate_color,utl_raw.cast_to_raw(ve.vehicle_no),tl.tmac,"
		"tl.oem_code,tl.auth_code,ve.vid,(ve.reg_status+1),sm.city "
		"FROM tb_sim sm,tr_serviceunit su,tb_vehicle ve,tb_terminal tl WHERE "
  "sm.enable_flag = 1 AND tl.enable_flag =1 AND ve.enable_flag =1 AND sm.sid = su.sid AND su.tid = tl.tid AND su.vid = ve.vid" ;
// ������ҵ��SQL
// select ent_id, parent_id from tb_organization
static const char *gcorp_sql = "SELECT parent_id,ent_id FROM tb_organization" ;

// ͬ�����ݲ���
void CSynServer::SynData( void )
{
	IRedisCache *redis = _pEnv->GetRedisCache() ;
	if ( redis == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "get redis cache failed" ) ;
		return ;
	}

	IDBFace *db = _dbpool->CheckOut( _szdb ) ;
	if ( db == NULL ) {
		OUT_ERROR( NULL, 0, NULL, "connect database %s failed" , _szdb ) ;
		return ;
	}

	CSqlResult *rs1 = db->Select( gvechile_sql ) ;
	if ( rs1 != NULL ) {
		int count = rs1->GetCount() ;

		char szkey[128]  = {0} ;
		char szval[1024] = {0} ;
		char szcar[1024] = {0} ;
		// ���¹��ڳ��������е�����
		for ( int i = 0; i < count; ++ i ) {
			// �µ�ͬ������ģʽ������ͳһʹ��GBK���룬ʹ�ü��Ϸ�ʽΪHASH��
			if ( _synmode & 0x01 ) {
				// ��ԭʼʮ��������ת����GBK����
				hex2str( rs1->GetValue(i, 3) , szcar ) ;
				// area: lbs.phone key: ���ֻ��š� value: corpid:����ҵID��,color:��������ɫ��,vechile:�����ƺš�,termid:���ն�ID��,oem:��OEM�롿, auth:���ն˼�Ȩ�롿,vid:��VID��,flag:�������Ƿ�ע��״̬��0δע�ᣬ1��ע�ᣩ��
				sprintf( szkey, "%s", rs1->GetValue(i,0) ) ;
				sprintf( szval,
						"corpid:%s,color:%s,vechile:%s,termid:%s,oem:%s,auth:%s,vid:%s,flag:%s",
						rs1->GetValue( i, 1 ), rs1->GetValue( i, 2 ),  szcar,  rs1->GetValue( i, 4 ),
						rs1->GetValue( i, 5 ),  rs1->GetValue( i, 6 ), rs1->GetValue( i, 7 ) , rs1->GetValue( i, 8 )
						) ;
				redis->HSet( "lbs.phone", szkey, szval ) ;

				// mppas.��������ɫ��_�����ƺš� value: ��oem��_���ֻ��š�
				sprintf( szkey, "%s_%s", rs1->GetValue( i, 2 ), szcar ) ;
				sprintf( szval, "%s_%s", rs1->GetValue( i, 5), rs1->GetValue( i, 0 ) ) ;
				redis->HSet( "lbs.car", szkey, szval ) ;
			}
			// OUT_INFO( NULL, 0, NULL, szkey ) ;

			// �����ԭ���ɵ�ʹ��ͬ������ģʽ
			if( _synmode & 0x02 ) {
				const char *ptr = hex2utf8( rs1->GetValue(i, 3) , szcar ) ;
				// key: lbs.���ֻ��š� value: corpid:����ҵID��,color:��������ɫ��,vechile:�����ƺš�,termid:���ն�ID��,oem:��OEM�롿, auth:���ն˼�Ȩ�롿,vid:��VID��,flag:�������Ƿ�ע��״̬��0δע�ᣬ1��ע�ᣩ��
				sprintf( szkey, "lbs.%s", rs1->GetValue(i,0) ) ;
				sprintf( szval,
						"corpid:%s,color:%s,vechile:%s,termid:%s,oem:%s,auth:%s,vid:%s,flag:%s",
						rs1->GetValue( i, 1 ), rs1->GetValue( i, 2 ),  ptr,  rs1->GetValue( i, 4 ),
						rs1->GetValue( i, 5 ),  rs1->GetValue( i, 6 ), rs1->GetValue( i, 7 ) , rs1->GetValue( i, 8 )
						) ;
				redis->SetValue( szkey, szval ) ;

				// mppas.��������ɫ��_�����ƺš� value: ��oem��_���ֻ��š�
				sprintf( szkey, "mppas.%s_%s", rs1->GetValue( i, 2 ), ptr ) ;
				sprintf( szval, "%s_%s", rs1->GetValue( i, 5), rs1->GetValue( i, 0 ) ) ;
				redis->SetValue( szkey, szval ) ;
			}

			// key: pushserver.����ҵID�� value: ���ֻ����б�(array) ���߲�ѯָ�
			sprintf( szkey, "pushserver.%s", rs1->GetValue(i,1) ) ;
			redis->LRem( szkey, szval ) ;
			redis->PushValue( szkey, szval ) ;
		}
		db->FreeResult( rs1 ) ;

		OUT_RUNNING( NULL, 0, NULL, "update car count %d", count ) ;
	}

	CSqlResult *rs2 = db->Select( gcorp_sql ) ;
	if ( rs2 != NULL ) {
		char szkey[128] = {0} ;
		char szval[1024] = {0} ;

		int count = rs2->GetCount() ;
		for ( int i = 0; i < count; ++ i ) {

			int pid = atoi( rs2->GetValue( i, 0 ) ) ;
			if ( pid == -1 ) {
				continue ;
			}

			sprintf( szkey, "pushserver.%d", pid ) ;
			sprintf( szval, "JMP:pushserver.%s", rs2->GetValue( i, 1 ) ) ;

			redis->LRem( szkey, szval ) ;
			redis->PushValue( szkey, szval ) ;
		}
		db->FreeResult( rs2 ) ;

		OUT_RUNNING( NULL, 0, NULL, "update corp count %d", count ) ;
	}

	_dbpool->CheckIn( db ) ;
}
