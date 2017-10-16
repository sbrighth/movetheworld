#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>				// errno
#include <stdarg.h>				// va_start, va_end
#include <sys/stat.h>			// stat
#include <time.h>				// time
#include <sys/time.h>			// gettimeofday
#include <fcntl.h>				// open()
#include <pthread.h>

#include <iostream>
using namespace std;

#if defined(__linux)
	#include <sys/shm.h>  			// Shared Memory
	#include <sys/sem.h>			// Semaphore
	#include <sys/msg.h>			// Message Queue
	#include <sys/socket.h> 		// Socket
	#include <netinet/in.h>			// Socket
	#include <arpa/inet.h> 			// Socket
	#include <net/if.h>				// Socket
	#include <sys/ioctl.h>			// Socket
	#include <netinet/ether.h>		// Socket
	#include <sys/poll.h>			// POLL
	#include <termios.h>			// serial
#else
	#include <Winsock2.h>			// serial
	#include <windows.h>
#endif

#include "base_def.h"
#include "base.h"

#define NONE			(-1)
#define LOCK			(0)
#define UNLOCK			(1)

enum
{
	FLAG_SEND = 0,
	FLAG_RECV,
	FLAG_SEND_RECV,
};

union semun
{
	int                  val;
	struct   semid_ds   *buf;
	unsigned short int  *arrary;
};

#define MAX_RESERVED_INDEX		(10)

timeval 	g_tTimerStart[ MAX_RESERVED_INDEX ];
int			g_base_flag;

#ifdef __cplusplus
extern "C" {
#endif


const char* base_version()
{
	return "1.0.0";
}


int base_init( const char* arg )
{
	return 0;
}


void base_set_flag( int flag )
{
	g_base_flag = flag;
}



void msleep( uint64 time )
{
	base_msleep( time );
}


#ifdef __linux__
void Sleep( uint64 time )
{
	base_sleep( time );
}
#endif

void MSleep( uint64 time )
{
	base_msleep( time );
}


void USleep( uint64 time )
{
	base_usleep( time );
}


void delayu( uint64 time )
{
	base_usleep( time );
}


void delaym( uint64 time )
{
	base_usleep( time*1000 );
}


void delays( uint64 time )
{
	base_usleep( time*1000*1000 );
}


void base_usleep( uint64 time )
{
#if defined(__linux)
	usleep( time );
#else
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10*time); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#endif
}


void base_msleep( uint64 time )
{
	base_usleep( time*1000 );
}


void base_sleep( uint64 time )
{
	base_usleep( time*1000*1000 );
}


void base_atof( const char* p, double* pdata )
{
	*pdata = atof( p );
}


int base_start_timer( int index )
{
	if( index < 0 || index > MAX_RESERVED_INDEX-1 )
		return -1;

	gettimeofday( &g_tTimerStart[index], NULL );

	return 0;
}


int base_read_timer( int index, double *p )
{
	timeval tNowTime;
	double	t1, t2;

	if( index < 0 || index > MAX_RESERVED_INDEX-1 )
		return -1;

	if( g_tTimerStart[index].tv_sec == 0 )
		return -2;

	gettimeofday( &tNowTime , NULL );
	t1 = g_tTimerStart[index].tv_sec \
			+ (double)g_tTimerStart[index].tv_usec*1e-6;
	t2 = tNowTime.tv_sec + (double)tNowTime.tv_usec*1e-6;

	if( p != NULL )
		*p =  t2 - t1;

	return 0;
}


const char* base_cmd_get_stdout( const char* pCmd )
{
	int len = 0;
	char cmd[10240] = {0,};
	char file[16] = "/tmp/stdout";
	sprintf( cmd, "%s > %s", pCmd, file );
	system( cmd );

	len = base_file_get_size( file );

	len = base_file_read_text( file, cmd, 0, len );
	string str_out = cmd;

	return str_out.c_str();
}


int base_sem_create( int key, int* pid )
{
	int ret = 0;
#ifdef __linux__
	int iSemID = semget( (key_t)key, 1, 0666|IPC_CREAT|IPC_EXCL );
	union semun sem_union;

	if( iSemID < 0 )
	{
		if( errno == EEXIST )
			iSemID = semget( (key_t)key, 1, 0 );
		else
			return -1;
	}
	else
	{
		 sem_union.val = 1;
		 ret = semctl( iSemID, 0, SETVAL, sem_union);
		 if( ret != 0 )
			 return -2;

		 ret = 1;	// First Semaphore Create Flag;;
	}

	*pid = iSemID;
#endif
	return ret;
}


int base_sem_remove( int id )
{
	if( id < 0 )
		return -1;
#ifdef __linux__
	if( semctl( id, 0, IPC_RMID, 0 ) == -1 )
		return -2;
#endif
	return 0;
}


int base_sem_lock( int id )
{
	if( id < 0 )
		return -1;
#if defined(__linux)
	struct sembuf pbuf;

	pbuf.sem_num   = 0;
	pbuf.sem_op    = -1;
	pbuf.sem_flg   = SEM_UNDO;

	if( semop( id, &pbuf, 1 ) != 0 )
		return -2;
#endif
	return 0;

}


int base_sem_unlock( int id )
{
	if( id < 0 )
		return -1;
#if defined(__linux)
	struct sembuf pbuf;

	pbuf.sem_num   = 0;
	pbuf.sem_op    = 1;
	pbuf.sem_flg   = SEM_UNDO;

	if( semop( id, &pbuf, 1 ) != 0 )
		return -2;
#endif
	return 0;
}


int base_sem_status( int id, int* piStat )
{
	if( id < 0 )
		return -1;
#if defined(__linux)
	*piStat = semctl( id, 0, GETVAL, 0 );
	if( *piStat == -1 )
		return -2;
#endif
	return 0;
}


int base_sem_is_lock( int id )
{
	int status = NONE;
	if( base_sem_status( id, &status ) != 0 )
		return false;

	if( status == LOCK )
		return true;

	return false;
}


int base_sem_is_unlock( int id )
{
	int status = NONE;
	if( base_sem_status( id, &status ) != 0 )
		return false;

	if( status == UNLOCK )
		return true;

	return false;
}


int base_shm_create( int data_size, int key, int* pid )
{
	int ret = NORMAL_END;

#if defined(__linux)
	if( data_size < 0 )
		return -1;

	int id = shmget( (key_t)key, data_size, 0666|IPC_CREAT|IPC_EXCL );

	if( id < 0 )
	{
		if( errno == EEXIST )
		{
			id = shmget( (key_t)key, 1, 0 );
			ret = 1;
		}
		else
			return -2;
	}
	else
	{
		 ret = 0;	// First Semaphore Create Flag;
	}

	if( pid != NULL )
		*pid = id;
#else

#endif
	return ret;
}


int base_shm_remove( int id )
{

#if defined(__linux)
	if( id < 0 )
		return -1;

	if( shmctl( id, IPC_RMID, 0 ) == -1 )
		return -2;
#else

#endif

	return 0;
}


int base_shm_get( int id, int offset, int data_sz, char* data )
{
#if defined(__linux)
	if( id < 0 )
		return -1;

	if( offset < 0 )
		return -2;

	void* pShmAddr = NULL;
	pShmAddr = shmat( id, (void *)0, 0666|IPC_CREAT );

	if( pShmAddr == NULL || pShmAddr == (void*)-1 )
		return -3;

	memcpy( data, (char *)pShmAddr+offset, data_sz );
#else

#endif

	return 0;
}


int base_shm_set( int id, int offset, int data_sz, char* data )
{
#if defined(__linux)
	if( id < 0 )
		return -1;

	if( offset < 0 )
		return -2;

	void* pShmAddr;
	pShmAddr = shmat( id, (void *)0, 0666|IPC_CREAT );

	if( pShmAddr == NULL || pShmAddr == (void*)-1 )
		return -3;

	memcpy( (char *)pShmAddr+offset, (char*)data, data_sz );

#else

#endif

	return 0;
}


int base_msgq_create( int key, int* pid )
{
#if defined(__linux)
	int id = NONE;
	id = msgget( (key_t)key, IPC_CREAT|0666 );

	if( id < 0 )
		return -1;

	if( pid != NULL )
		*pid = id;
#else

#endif

	return 0;
}


int base_msgq_remove( int id )
{
#if defined(__linux)
	if( id < 0 )
		return -1;

	if( msgctl( id, IPC_RMID, 0 ) < 0 )
		return -2;
#else

#endif

	return 0;
}


int base_msgq_send( int id, void* msg, int msg_sz )
{
#if defined(__linux)
	if( msg_sz <= (int)(sizeof(long)) )
		return -1;

	if( msgsnd( id, msg, msg_sz-sizeof(long), IPC_NOWAIT ) < 0 )
		return -2;
#else

#endif
	return 0;
}


int base_msgq_recv( int id, void* msg, int msg_sz, long msg_type, int msg_flag )
{
#if defined(__linux)
	if( msg_sz <= (int)(sizeof(long)) )
		return -1;

	msg_flag = msg_flag|IPC_NOWAIT;

	if( msgrcv( id, msg, msg_sz-sizeof(long), msg_type, msg_flag ) < 0 )
		return -2;
#else

#endif

	return 0;
}



int base_mount( const char* host, const char* shdir, const char* path, const char* /*id*/, const char* /*passwd*/ )
{
	char mount_cmd[128] = {0,};
	/*
	sprintf( mount_cmd,
			"mount -t cifs //%s/%s %s -o username=%s,password=%s,sec=ntlm",
			host, shdir, path, id, passwd );
	*/
	sprintf( mount_cmd,
				"mount -t nfs %s:/%s %s",
				host, shdir, path );

	printf( ">> mount_cmd: %s\n", mount_cmd );

	return system( mount_cmd );
}


int base_unmount( const char* path )
{
	char umount_cmd[128] = {0,};
	sprintf( umount_cmd,
			"umount %s", path );

	return system( umount_cmd );
}


int base_is_mount( const char* path )
{
	string str_out = base_cmd_get_stdout( "mount" );
	int ret = base_str_is_contains( str_out.c_str(), path );

	return ret;
}


int _base_sock_sendrecv( const char* ip, int port, void* send_data, int send_size, void* recv_data, int recv_size, int timeout, int flag )
{
#if defined(__linux)
	struct sockaddr_in  tClntAddr;
	int                 iClntSock;
	int                 iSetSockOpt = 1;

	memset( &tClntAddr , 0 , sizeof( tClntAddr ) );
	tClntAddr.sin_port = htons( port );
	tClntAddr.sin_family = AF_INET;
	tClntAddr.sin_addr.s_addr = inet_addr( ip );

	if( ( iClntSock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == ABNORMAL_END )
		return -1;

	if( setsockopt( iClntSock, SOL_SOCKET, SO_REUSEADDR, &iSetSockOpt, sizeof(iSetSockOpt) ) != NORMAL_END )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return -2;
	}

	if( connect( iClntSock, (struct sockaddr *)&tClntAddr, sizeof(tClntAddr) ) != NORMAL_END )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return -3;
	}

	if( (flag == FLAG_SEND) || (flag == FLAG_SEND_RECV)  )
	{
		if( send( iClntSock, (char *)send_data, send_size, 0 ) < 0 )
		{
			shutdown( iClntSock, SHUT_RDWR );
			close( iClntSock );
			return -4;
		}
	}

	if( flag == FLAG_SEND )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );

		return 0;
	}

	struct pollfd 	tPollEvent;

	tPollEvent.fd        = iClntSock;
	tPollEvent.events    = POLLIN | POLLERR;
	tPollEvent.revents   = 0;

	int iPollState = poll( (struct pollfd*)&tPollEvent, 1, timeout );

	if( 1 > iPollState )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return -5;
	}

	if( tPollEvent.revents & POLLERR )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return -6;
	}

	if( recv( iClntSock, (char *)recv_data, recv_size , 0 ) < 0 )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return -7;
	}

	shutdown( iClntSock, SHUT_RDWR );
	close( iClntSock );
#else
	WSADATA wsaData;

	int iStrLen = 0;

	SOCKADDR_IN servAddr;
	SOCKET hSocket;

	if( WSAStartup( MAKEWORD(2,2) , &wsaData ) != 0 )
	{
		printf( "WSAStartup() ERROR\n" );
		return -1;
	}

	hSocket = socket( PF_INET , SOCK_STREAM , 0 );
	if( hSocket == INVALID_SOCKET )
	{
		printf( "socket() ERROR\n" );
		return -2;
	}

	memset( &servAddr , 0 , sizeof( servAddr ) );
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr( ip );
	servAddr.sin_port = htons( port );

	for( int iConnectCnt = 0 ; iConnectCnt < 1 ; iConnectCnt++ )
	{
		if( connect( hSocket , (SOCKADDR*)&servAddr , sizeof( servAddr ) ) == SOCKET_ERROR )
		{
			if( iConnectCnt == 10 )
			{
				closesocket( hSocket );
				WSACleanup();

				printf( "connect() ERROR\n" );
				return -3;
			}

			continue;
		}

		break;
	}

	//SOCK_PACKET tRecvData;

	if( (flag == FLAG_SEND) || (flag == FLAG_SEND_RECV)  )
	{
		send( hSocket, (char*)send_data, send_size, 0 );
	}

	if( flag == FLAG_SEND )
	{
		closesocket( hSocket );
		WSACleanup();
	}

	recv( hSocket, (char*)recv_data, recv_size, 0 );

	closesocket( hSocket );
	WSACleanup();

#endif
	return 0;
}


int base_sock_send( const char* ip, int port, void* send_data, int send_size, int timeout )
{
	return _base_sock_sendrecv( ip, port, send_data, send_size, NULL, 0, timeout, FLAG_SEND );
}


int base_sock_recv( const char* ip, int port, void* recv_data, int recv_size, int timeout )
{
	return _base_sock_sendrecv( ip, port, NULL, 0, recv_data, recv_size, timeout, FLAG_RECV );
}


int base_sock_send_recv( const char* ip, int port, void* send_data, int send_size, void* recv_data, int recv_size, int timeout )
{
	return _base_sock_sendrecv( ip, port, send_data, send_size, recv_data, recv_size, timeout, FLAG_SEND_RECV );
}


int base_sock_is_connect( const char* ip, int port, int timeout )
{
#if defined(__linux)
	struct sockaddr_in  tClntAddr;
	int                 iClntSock;
	int                 iSetSockOpt = 1;

	memset( &tClntAddr , 0 , sizeof( tClntAddr ) );
	tClntAddr.sin_port = htons( port );
	tClntAddr.sin_family = AF_INET;
	tClntAddr.sin_addr.s_addr = inet_addr( ip );

	if( ( iClntSock = socket( PF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP ) ) == ABNORMAL_END )
		return false;

	if( setsockopt( iClntSock, SOL_SOCKET, SO_REUSEADDR, &iSetSockOpt, sizeof(iSetSockOpt) ) != NORMAL_END )
	{
		shutdown( iClntSock, SHUT_RDWR );
		close( iClntSock );
		return false;
	}

	if( connect( iClntSock, (struct sockaddr *)&tClntAddr, sizeof( tClntAddr ) ) < 0 )
	{
		if( errno == EINPROGRESS )
		{
			struct timeval tv;
			fd_set tNewFdSet;

			tv.tv_sec = timeout/1000;
			tv.tv_usec = timeout%1000;

			FD_ZERO( &tNewFdSet );
			FD_SET( iClntSock, &tNewFdSet );

			if( select( iClntSock+1, NULL, &tNewFdSet, NULL, &tv ) > 0 )
			{
				shutdown( iClntSock, SHUT_RDWR );
				close( iClntSock );

				return true;
			}
		}
	}

	shutdown( iClntSock, SHUT_RDWR );
	close( iClntSock );
#else

#endif
	return false;
}


int base_sock_get_ip( int eth_no, char *ip )
{
#if defined(__linux)
	int iSockFD;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	iSockFD = socket( AF_INET, SOCK_STREAM, 0 );
	if( iSockFD < 0 )
		return -1;

	sprintf( ifr.ifr_name, "eth%d", eth_no ) ;
	if( ioctl( iSockFD, SIOCGIFADDR, &ifr ) < 0 )
	{
		close( iSockFD );
		return -2;
	}

	sin = (struct sockaddr_in*)&ifr.ifr_addr;
	strcpy( ip, inet_ntoa(sin->sin_addr) );

	close( iSockFD );

	//int iIP = inet_addr( pIpAddr );
	//int iLastNo  = (iIP & 0xFF000000)>>24;
#else

#endif
	return 0;
}


int base_net_is_run( int eth_no )
{
#if defined(__linux)
	int iSockFD;
	struct ifreq ifr;
	struct sockaddr_in *sin;

	iSockFD = socket( AF_INET, SOCK_STREAM, 0 );
	if( iSockFD < 0 )
		return -1;

	sprintf( ifr.ifr_name, "eth%d", eth_no ) ;

	if( ioctl( iSockFD, SIOCGIFFLAGS, &ifr ) < 0 )
	{
		close( iSockFD );
		return false;
	}

	/*
	if( ifr.ifr_flags & IFF_RUNNING )
	{
		close( iSockFD );
		return false;
	}

	if( ifr.ifr_flags & IFF_UP )
	{
		close( iSockFD );
		return false;
	}
	*/
	close( iSockFD );
#else

#endif
	return true;
}


int base_net_set_ip( int eth_no, const char* ip )
{
	int ret = 0;
#if defined(__linux)
	char cIfCmd[128] = {0,};
	sprintf( cIfCmd, "/sbin/ifconfig eth%d %s", eth_no, ip );

	ret = system( cIfCmd );
#else

#endif
	return ret;
}


int base_file_is_exist( const char* p )
{
	if( access( p, F_OK ) != -1 )
		return true;

	return false;
}


long base_file_get_size( const char* p )
{
	/*
	struct stat file_info;

	if( 0 > stat( p, &file_info ) )
		return 0;

	return file_info.st_size;
	*/

	FILE* fd = NULL;
	long sz = 0;

	fd = fopen( p, "rb" );
	if( fd == NULL )
	{
		return 0;
	}

	fseek( fd, 0L, SEEK_END );

	sz = ftell( fd );
	fclose( fd );

	return sz;
}


int base_file_copy( const char* obj_file, const char* copy_file )
{
	char cCopyCmd[128] = {0,};
	sprintf( cCopyCmd, "cp -r -f %s %s", obj_file, copy_file );

	int iRetVal = system( cCopyCmd );

	return iRetVal;
}


int base_file_create( const char* obj_file )
{
	char cCreateCmd[128] = {0,};
	sprintf( cCreateCmd , "touch %s" , obj_file );

	int iRetVal = system( cCreateCmd );

	return iRetVal;
}


int base_file_remove( const char* obj_file )
{
	if( base_file_is_exist( obj_file ) != true )
		return -1;

	char cRemoveCmd[128] = {0,};
	sprintf( cRemoveCmd , "rm -r -f %s" , obj_file );

	int iRetVal = system( cRemoveCmd );

	return iRetVal;
}


int base_file_get_time( const char* obj_file, char* time_str )
{
	if( base_file_is_exist( obj_file ) != true )
		return -1;

	struct stat tDateBuff;
	stat( obj_file, &tDateBuff );

	struct tm *t = localtime( &tDateBuff.st_ctime );

	if( time_str != NULL )
	{
		sprintf( time_str, "%04d-%02d-%02d %02d:%02d:%02d",
					  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					  t->tm_hour, t->tm_min, t->tm_sec );
	}

	return 0;
}


long base_file_read_bin( const char* p, char* info, long offset, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "rb" );
	if( fd == NULL )
		return 0;

	if( fseek( fd, offset, SEEK_SET ) != 0 )
	{
		fclose( fd );
		return 0;
	}

	len = fread( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_write_bin( const char* p, char* info, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "wb" );
	if( fd == NULL )
		return 0;

	len = fwrite( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_update_bin( const char* p, char* info, long offset, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "r+b" );
	if( fd == NULL )
		return 0;

	if( fseek( fd, offset, SEEK_SET ) != 0 )
	{
		fclose( fd );
		return 0;
	}

	len = fwrite( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_add_bin( const char* p, char* info, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "ab" );
	if( fd == NULL )
		return 0;

	len = fwrite( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_read_text( const char* p, char* info, long offset, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "r" );
	if( fd == NULL )
		return 0;

	if( fseek( fd, offset, SEEK_SET ) != 0 )
	{
		fclose( fd );
		return 0;
	}

	len = fread( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_write_text( const char* p, char* info, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "w" );
	if( fd == NULL )
		return 0;

	len = fwrite( info, 1, size, fd );
	fclose( fd );

	return len;
}


long base_file_add_text( const char* p, char* info, long size )
{
	FILE* fd = NULL;
	long len = 0;

	fd = fopen( p, "a" );
	if( fd == NULL )
		return 0;

	len = fwrite( info, 1, size, fd );
	fclose( fd );

	return len;
}


uint64 base_rand_get_int( uint64 start_val, uint64 end_val )
{
	uint64 uiRetVal = 0;
	uint64 uiProcMaxVal = end_val-start_val+1;

	if( uiProcMaxVal == 1 )
		return start_val;
	else if( uiProcMaxVal < 1 )
	{
		uint64 uiTempVal = start_val;
		start_val = end_val;
		end_val = uiTempVal;
		uiProcMaxVal = end_val-start_val+1;
	}

	timeval tTimerStart;
	gettimeofday( &tTimerStart , NULL );
	int iSeedVal = (int)tTimerStart.tv_sec + (int)tTimerStart.tv_usec;

	srand( iSeedVal+(unsigned int)getpid() );

	uiRetVal = (((uint64)rand() << 0  ) * 0x000000000000FFFFull ) |
				 (((uint64)rand() << 16 ) * 0x00000000FFFF0000ull ) |
				 (((uint64)rand() << 32 ) * 0x0000FFFF00000000ull ) |
				 (((uint64)rand() << 48 ) * 0xFFFF000000000000ull );

	uiRetVal = (uiRetVal * rand()) % uiProcMaxVal + start_val;

	return uiRetVal;
}

double base_rand_get_double( double start_val, double end_val )
{
	timeval tTimerStart;
	gettimeofday( &tTimerStart , NULL );
	int iSeedVal = (int)tTimerStart.tv_sec + (int)tTimerStart.tv_usec;

	srand( iSeedVal+(unsigned int)getpid() );

	double dRandValue = (double)rand() / RAND_MAX;

	return start_val + dRandValue * ( end_val - start_val );

}


int base_set_correct( double *dXx )
{
	unsigned long long ullXx;
	unsigned int uiExp;

	memcpy( &ullXx, dXx, sizeof(ullXx) );
	memcpy( &uiExp, dXx, sizeof(uiExp) );
	if( uiExp & 0xFFFF )
	{
		uiExp+=2;
		ullXx |= uiExp;
	}
	memcpy( dXx, &ullXx, sizeof(dXx) );

	return 0;
}


int base_get_pattern( int index, void* buff, int len )
{
	int             i;
	int             j;
	unsigned char   *pucBuff1 = (unsigned char *)buff;
	unsigned short  *pucBuff2 = (unsigned short *)buff;
	unsigned int    *pucBuff3 = (unsigned int *)buff;

	switch( index )
	{
	case 1:  // 0xFF00FF00
		for( i = 0; i < len/2; i++ )
		{
			*pucBuff1++ = 0x00;
			*pucBuff1++ = 0xff;
		}
		break;

	case 2: // 0xAA55AA55
		for( i = 0; i < len/2; i++ )
		{
			*pucBuff1++ = 0x55;
			*pucBuff1++ = 0xaa;
		}
		break;

	case 3: // 0x76543210
		for( i = 0; i < len; i++ )
		{
			*pucBuff1++ = i;
		}
		break;

	case 4: // 0xfd02fe01
		for( i = 0; i < len/2; i++ )
		{
			*pucBuff1++ = 1 << (i & 7);
			*pucBuff1++ = ~(1 << (i & 7));
		}
		break;

	case 5: // 0xFFFF0000
		for( i = 0; i < len/4; i++ )
		{
			*pucBuff2++ = 0x0000;
			*pucBuff2++ = 0xffff;
		}
		break;

	case 6: // 0xAAAA5555
		for( i = 0; i < len/4; i++ )
		{
			*pucBuff2++ = 0x5555;
			*pucBuff2++ = 0xaaaa;
		}
		break;

	case 7: // 0x30201000
		for( i = 0; i < len/2; i++ )
		{
			*pucBuff2++ = i;
		}
		break;

	case 8: //
		for( i = 0; i < len/4; i++ )
		{
			*pucBuff2++ = 1 << (i & 15);
			*pucBuff2++ = ~(1 << (i & 15));
		}
		break;
	case 9:
		for( j = 0; j < len/0x200; j++ )
		{
			for( i = 0x0; i < 0xc0; i++ )
				*pucBuff1++ = 0x32;
			for( i = 0xc0; i < 0x1fe; i++ )
				*pucBuff1++ = 0x4a;
			for( i = 0x1fe; i < 0x200; i++ )
				*pucBuff1++ = 0x0;
		}
		break;
	case 10:
		for( j = 0; j < len/0x200; j++ )
		{
			for( i = 0x0; i < 0x200; i++ )
				*pucBuff1++ = 0xb5;
		}
		break;
	case 11:
		for( j = 0; j < len/0x200; j++ )
		{
			for( i = 0x0; i < 0x200; i++ )
				*pucBuff1++ = 0x4a;
		}
		break;
	case 12:  // RANDOM
		srand((unsigned int)time(NULL)+(unsigned int)getpid());
		for( i = 0; i < len/4; i++ )
		{
			*pucBuff3++ = rand();
		}
	default:
		return -1;
	}

	return 0;
}

int base_get_cpu_val( double* pdCpuPerVal )
{
	const char*	dummy_file = "/tmp/dummy";
	FILE*		fpDummyFile;
	char		cCommandLine[128];
	char		cCntData[16] = {0,};
	int 		iRetVal = 0;

	memset( cCommandLine , 0 , sizeof( cCommandLine ) );
	sprintf( cCommandLine , "mpstat | tail -1 | awk \'{print 100-$12}\' > %s" , dummy_file );

	system( cCommandLine );

	if( ( fpDummyFile = fopen( dummy_file , "r" ) ) != NULL )
	{
		if( fread( &cCntData , 1 , sizeof( cCntData ), fpDummyFile ) > 0 )
		{
			*pdCpuPerVal = atof( cCntData );
		}

		fclose( fpDummyFile );

		memset( cCommandLine , 0 , sizeof( cCommandLine ) );
		sprintf( cCommandLine , "rm %s" , dummy_file );
		system( cCommandLine );
	}

	return iRetVal;
}


int base_get_proc_cnt( const char *cProcName )
{
	const char*	dummy_file = "/tmp/dummy";
	FILE*		fpDummyFile;
	char		cCommandLine[128] = {0,};
	char		cCntData = '0';
	int 		iRetVal = 0;

	memset( cCommandLine, 0, sizeof(cCommandLine) );
	sprintf( cCommandLine, "ps -ae | grep %s | wc -l > %s", cProcName, dummy_file );

	system( cCommandLine );

	if( ( fpDummyFile = fopen( dummy_file, "r" ) ) != NULL )
	{
		if( ( fread( &cCntData, sizeof(cCntData), 1, fpDummyFile ) ) == sizeof(cCntData) )
			iRetVal = cCntData - 0x30;

		fclose( fpDummyFile );

		memset( cCommandLine, 0, sizeof(cCommandLine) );
		sprintf( cCommandLine, "rm %s", dummy_file );
		system( cCommandLine );
	}

	return iRetVal;
}


int base_str_is_contains( const char* obj_str, const char* find_str )
{
	if( strstr( obj_str, find_str ) == NULL )
		return false;

	return true;
}


struct ThreadArg
{
	void* (*pfunc)(void*);
	int   timeout;
};


void* single_thread( void* pParm )
{
	ThreadArg* pArg = (ThreadArg*)pParm;
	int timeout = pArg->timeout;
	void* (*pfunc)(void*) = pArg->pfunc;

	usleep( timeout );

	return (void*)pfunc( NULL );
}


int base_timer_func( void* (*pfunc)(void*), int timeout )
{
	int ret = 0;

#if defined(__linux)
	pthread_t tThreadID;
	ThreadArg tThreadArg;

	tThreadArg.pfunc    = pfunc;
	tThreadArg.timeout = timeout;

	ret = pthread_create( &tThreadID, NULL, single_thread, &tThreadArg );

	for( int i=0; i<10000000; i++ ) ;

#else

#endif
	return ret;
}


const char*	base_path_get_ath_test( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d/exe", SYS_ATH_RACK_PATH, id );

	return path;
}


const char*	base_path_get_ath_in_file( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d/exe/%s", SYS_ATH_RACK_PATH, id, SYS_IN_FILENAME );

	return path;
}


const char*	base_path_get_ath_out_file( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d/exe/%s", SYS_ATH_RACK_PATH, id, SYS_OUT_FILENAME );

	return path;
}


const char*	base_path_get_sha_test( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d", SYS_SHA_PATH, id );

	return path;
}


const char*	base_path_get_sha_in_file( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d/%s", SYS_SHA_PATH, id, SYS_IN_FILENAME );

	return path;
}


const char*	base_path_get_sha_out_file( int id )
{
	static char path[128] = {0,};
	sprintf( path, "%s/tester%03d/%s", SYS_SHA_PATH, id, SYS_OUT_FILENAME );

	return path;
}


int base_msg_init_header( const char* filename )
{
	int ret = 0;

	ExMsgPack packet;
	memset( &packet, 0, sizeof( packet ) );

	packet.hdr.version = MSG_HEADER_VERSION;
	packet.hdr.cell = 2;
	packet.hdr.msg  = 0;
	sprintf( packet.msgstr, "HEADER INFO" );

	int packet_size = (g_base_flag==0)?MSG_PACKET_SIZE:MSG_PACKET_SIZE_2;
	if( base_file_write_bin( filename, (char*)&packet, packet_size )
			!= packet_size )
		return -1;

	return ret;
}


int base_msg_write( const char* filename, ExMsgPack* packet )
{
	if( base_file_is_exist( filename ) != true )
		base_msg_init_header( filename );

	ExMsgPack header;
	memset( &header, 0, sizeof( header ) );

	int packet_size = (g_base_flag==0)?MSG_PACKET_SIZE:MSG_PACKET_SIZE_2;

	FILE* fd = NULL;
	long len = 0;

	fd = fopen( filename, "r+b" );
	if( fd == NULL )
		return 0;

	len = fread( &header, 1, packet_size, fd );
	header.hdr.msg += 1;
	header.hdr.cell = header.hdr.msg;

	packet->hdr.version = 0;
	packet->hdr.packet  = header.hdr.msg;

	fseek( fd, 0, SEEK_END );
	len = fwrite( packet, 1, packet_size, fd );

	fseek( fd, 0, SEEK_SET );
	len = fwrite( &header, 1, packet_size, fd );

	fclose( fd );

	return len;
}


int	_base_msg_send( int port, ExMsgPack* msg_pack, long timeout, int flag )
{
	int	ret = 0;
	/*
	ret = _base_sock_sendrecv( 	"127.0.0.1",
								port,
								(void*)msg_pack,
								MSG_PACKET_SIZE,
								NULL,
								0,
								timeout,
								flag
								);
	*/

	///*
	//if( g_base_flag == 0 || port == TMON_MSG_PORT(msg_pack->hdr.cell) )
	if( g_base_flag == 0 )
	{
		base_pack_msgq_send( msg_pack->hdr.cell, msg_pack );
	}
	else
	{
		base_pack_d_msgq_send( msg_pack->hdr.cell, msg_pack );
	}
	//*/

	return ret;
}


int	base_msg_send( int port, ExMsgPack* msg_pack )
{
	return _base_msg_send( port, msg_pack, 1*1000, FLAG_SEND );
}


int base_msg_send_recv( int port, ExMsgPack* msg_pack )
{
	return _base_msg_send( port, msg_pack, 1*1000, FLAG_SEND_RECV );
}


int base_msg_display( const char* p )
{
	int cnt = 0;
	int ret = 0;
	int len = 0;

	int packet_size = (g_base_flag==0)?MSG_PACKET_SIZE:MSG_PACKET_SIZE_2;

	cnt = base_file_get_size( p )/packet_size;
	if( cnt < 1 )
		return -1;

	FILE* fd = NULL;
	fd = fopen( p, "rb" );
	if( fd == NULL )
		return -2;

	printf( "   [NO.]  VER    CELL  PORT  MSG  PACK  FLG  DESC\n" );
	printf( "   ============================================================\n" );
	for( int i=0; i<cnt; i++ )
	{
		ExMsgPack msg;
		memset( &msg, 0, packet_size );

		len = fread( &msg, 1, packet_size, fd );
		if( len < packet_size )
		{
			fclose( fd );
			return -3;
		}

		printf( "   [%3d]  %5d  %4d  %4d  %3d  %4d  %3d  %s\n", i+1,
				msg.hdr.version,
				msg.hdr.cell,
				msg.hdr.port,
				msg.hdr.msg,
				msg.hdr.packet,
				msg.hdr.flag,
				msg.msgstr );
	}
	fclose( fd );

	return 0;
}

const char*
base_msg_get_text( int msg )
{
	if( msg == MSG_NONE )					return "MSG_NONE";
	else  if( msg == MSG_TEST_START )		return "MSG_TEST_START";
	else  if( msg == MSG_TEST_STOP )		return "MSG_TEST_STOP";
	else  if( msg == MSG_INIT )				return "MSG_INIT";
	else  if( msg == MSG_INITACK )			return "MSG_INITACK";

	else  if( msg == MSG_INITCOLOR )		return "MSG_INITCOLOR";
	else  if( msg == MSG_PASS )				return "MSG_PASS";
	else  if( msg == MSG_FAIL )				return "MSG_FAIL";
	else  if( msg == MSG_TEST )				return "MSG_TEST";
	else  if( msg == MSG_DONE )				return "MSG_DONE";

	else  if( msg == MSG_TEXT1 )			return "MSG_TEXT1";
	else  if( msg == MSG_TEXT2 )			return "MSG_TEXT2";
	else  if( msg == MSG_TEXT3 )			return "MSG_TEXT3";
	else  if( msg == MSG_TEXT4 )			return "MSG_TEXT4";

	else  if( msg == MSG_TIMER_START )		return "MSG_TIMER_START";
	else  if( msg == MSG_TIMER_STOP )		return "MSG_TIMER_STOP";

	else  if( msg == MSG_INFOR )			return "MSG_INFOR";
	else  if( msg == MSG_DIAGSTART )		return "MSG_DIAGSTART";
	else  if( msg == MSG_DIAGPASS )			return "MSG_DIAGPASS";
	else  if( msg == MSG_DIAGFAIL )			return "MSG_DIAGFAIL";
	else  if( msg == MSG_SETBADPORT )		return "MSG_SETBADPORT";
	else  if( msg == MSG_RELEASEBADPORT )	return "MSG_RELEASEBADPORT";
	else  if( msg == MSG_LEDCTRL )			return "MSG_LEDCTRL";

	else  if( msg == MSG_MOUNT_CHECK )		return "MSG_MOUNT_CHECK";
	else  if( msg == MSG_COM_OPEN )			return "MSG_COM_OPEN";
	else  if( msg == MSG_COM_CLOSE )		return "MSG_COM_CLOSE";
	else  if( msg == MSG_COM_TEST )			return "MSG_COM_TEST";

	else  if( msg == MSG_SERIAL_NUMBER )	return "MSG_SERIAL_NUMBER";
	else  if( msg == MSG_CONFIGURATION )	return "MSG_CONFIGURATION";
	else  if( msg == DATE_TIME_SET )		return "DATE_TIME_SET";

	else  if( msg == MSG_REBOOT )			return "MSG_REBOOT";
	else  if( msg == MSG_RESPONSE )			return "MSG_RESPONSE";
	else  if( msg == MSG_DISCON )			return "MSG_DISCON";
	else  if( msg == MSG_SBL_FAIL )			return "MSG_SBL_FAIL";
	else  if( msg == MSG_DUT_FAIL )			return "MSG_DUT_FAIL";

	else  if( msg == MSG_SYSCODE )			return "MSG_SYSCODE";
	else  if( msg == MSG_TESTFAIL )			return "MSG_TESTFAIL";

	else  if( msg == MSG_DPS_CHK_START )	return "MSG_DPS_CHK_START";
	else  if( msg == MSG_DPS_CHK_STOP )		return "MSG_DPS_CHK_STOP";
	else  if( msg == MSG_DPS_RESET )		return "MSG_DPS_RESET";

	else  if( msg == MSG_DPS_SET_VOLT )		return "MSG_DPS_SET_VOLT";
	else  if( msg == MSG_DPS_GET_VOLT )		return "MSG_DPS_GET_VOLT";
	else  if( msg == MSG_DPS_GET_CURRENT )	return "MSG_DPS_GET_CURRENT";

	else  if( msg == MSG_EX_TEST_CONFIG )	return "MSG_EX_TEST_CONFIG";
	else  if( msg == MSG_EX_USER_COMMAND )	return "MSG_EX_USER_COMMAND";
	else  if( msg == MSG_EX_SYS_CMD )		return "MSG_EX_SYS_CMD";

	return "Un-Known MSG";
}


int
_base_msg_test_status_send( int id, int port, int msg, int flag, const char* msg_str )
{
	ExMsgPack msg_pack;
	memset( &msg_pack, 0, MSG_PACKET_SIZE );

	msg_pack.hdr.cell = id;
	msg_pack.hdr.port = port;
	msg_pack.hdr.msg  = msg;
	msg_pack.hdr.flag = flag;
	sprintf( msg_pack.msgstr, "%s", msg_str );

	int sock_port = TMON_MSG_PORT( id );
	if( g_base_flag == 1 )
		sock_port = TMON_MSG_D_PORT( id );

	return base_msg_send( sock_port, &msg_pack );
}


int
base_msg_test_start_send( int id, int port )
{
	return _base_msg_test_status_send( id, port, MSG_TEST_START, 0, "" );
}


int
base_msg_test_stop_send( int id, int port )
{
	return _base_msg_test_status_send( id, port, MSG_TEST_STOP, 0, "" );
}


int
base_msg_test_test_send( int id, int port, const char* msg_str )
{
	return _base_msg_test_status_send( id, port, MSG_TEST, 0, msg_str );
}


int
base_msg_test_pass_send( int id, int port, const char* msg_str )
{
	return _base_msg_test_status_send( id, port, MSG_PASS, 0, msg_str );
}


int
base_msg_test_fail_send( int id, int port, int flag, const char* msg_str )
{
	return _base_msg_test_status_send( id, port, MSG_FAIL, flag, msg_str );
}


int
base_msg_test_done_send( int id, int port, int flag, const char* msg_str )
{
	return _base_msg_test_status_send( id, port, MSG_DONE, flag, msg_str );
}


int
base_msg_test_text_send( int id, int port, int index, const char* msg_str )
{
	return _base_msg_test_status_send( id, port, MSG_TEXT1+index-1, 0, msg_str );
}


int
base_main_msg_send( int port, ExMsgPack* msg_pack )
{
	int ret = 0;
	int packet_size = (g_base_flag==0)?MSG_PACKET_SIZE:MSG_PACKET_SIZE_2;

	ret = _base_sock_sendrecv( 	"127.0.0.1",
								port,
								(void*)msg_pack,
								packet_size,
								NULL,
								0,
								1000,
								0
								);

	return ret;
}


int
base_debug_msg_send( int id, const char* cmd, char* p, int timeout )
{
	int	ret = 0;
	char addr[32] = {0,};
	sprintf( addr, "%s%d", DEFAULT_IP, id );

	ret = _base_sock_sendrecv( 	addr,
								TMON_DEBUG_PORT(id),
								(void*)cmd,
								strlen( cmd ),
								p,
								256,		// recv size
								timeout,	// timeout
								FLAG_SEND_RECV
								);
	return ret;
}




int
base_pack_msgq_send( int id, ExMsgPack* packet )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_Q_KEY(id), &pid );
	if( ret < 0 )
		return -1;

	ExSysMsg msg;
	memset( &msg, 0, sizeof(msg) );
	msg.lMsgType = 1;
	memcpy( &msg.pack, packet, sizeof(ExMsgPack) );

	ret = base_msgq_send( pid, (void*)&msg, sizeof(msg) );

	return ret;
}


int
base_pack_msgq_recv( int id, ExMsgPack* packet )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_Q_KEY(id), &pid );
	if( ret < 0 )
		return -1;

	ExSysMsg msg;
	memset( &msg, 0, sizeof(msg) );
	msg.lMsgType = 1;

	long lMsgType = 0;
	int iMsgFlag = 0;

#if defined(__linux)
	iMsgFlag | IPC_NOWAIT;
#endif

	ret = base_msgq_recv( pid, (void*)&msg, sizeof(msg), lMsgType, iMsgFlag );
	memcpy( packet, &msg.pack, sizeof(ExMsgPack) );

	return ret;
}


int
base_pack_msgq_remove( int id )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_Q_KEY(id), &pid );
	if( ret < 0 )
		return 0;

	ret = base_msgq_remove( pid );

	return ret;
}


int
base_pack_d_msgq_send( int id, ExMsgPack* packet )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_D_Q_KEY(id), &pid );

	if( ret < 0 )
		return -1;

	ExSysMsg msg;
	memset( &msg, 0, sizeof(msg) );
	msg.lMsgType = 1;
	memcpy( &msg.pack, packet, sizeof(ExMsgPack) );

	//ret = base_msgq_send( pid, (void*)&msg, sizeof(msg)-sizeof(long) );
	ret = base_msgq_send( pid, (void*)&msg, sizeof(msg) );

	return ret;
}


int
base_pack_d_msgq_recv( int id, ExMsgPack* packet )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_D_Q_KEY(id), &pid );
	if( ret < 0 )
		return -1;

	ExSysMsg msg;
	memset( &msg, 0, sizeof(msg) );
	msg.lMsgType = 1;

	long lMsgType = 0;
	int iMsgFlag = 0;

#if defined(__linux)
	iMsgFlag | IPC_NOWAIT;
#endif

	//ret = base_msgq_recv( pid, (void*)&msg, sizeof(msg)-sizeof(long), lMsgType, iMsgFlag );
	ret = base_msgq_recv( pid, (void*)&msg, sizeof(msg), lMsgType, iMsgFlag );
	memcpy( packet, &msg.pack, sizeof(ExMsgPack) );

	return ret;
}


int
base_pack_d_msgq_remove( int id )
{
	int ret = 0;

	int pid = 0;
	ret = base_msgq_create( TMON_MSG_D_Q_KEY(id), &pid );
	if( ret < 0 )
		return 0;

	ret = base_msgq_remove( pid );

	return ret;
}

int
base_set_correct_value( double *dXx )
{
	unsigned long long ullXx;
	unsigned int uiExp;

	memcpy( &ullXx, dXx, sizeof(ullXx) );
	memcpy( &uiExp, dXx, sizeof(uiExp) );
	if( uiExp & 0xFFFF)
	{
		uiExp+=2;
		ullXx |= uiExp;
	}
	memcpy( dXx, &ullXx, sizeof(dXx) );

	return 0;
}


#ifdef __cplusplus
}
#endif

