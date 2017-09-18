//============================================================================
// Name        : testdm.cpp
// Author      : shjeong
// Version     :
// Copyright   : exicon
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "msgctrl.h"

#define PROG_NAME		"testdm"
#define PROG_VERSION	"0.0.1"

int CheckProgRunning();
void ProcSignalStop(int);
int SetSignal();

int g_condTestDm = 1;
int g_idMsgQIn = 0;
int g_idMsgQOut = 0;

int main(int argc, char *argv[])
{
	//running check
	int board_id = 10;

	if(CheckProgRunning() != 1)
	{
		printf("%s is runinning\n", PROG_NAME);
		return -1;
	}
	else
	{
		printf("%s start!!\n", PROG_NAME);
	}

	if(SetSignal() < 0)
		return -1;

	pthread_t msg_in_t;
	pthread_t msg_out_t;
	int stat_msg_in_t;
	int stat_msg_out_t;

	pthread_create(&msg_in_t, NULL, &MsgInFileThread, (void*)&board_id);
	pthread_create(&msg_out_t, NULL, &MsgOutFileThread, (void*)&board_id);
	//Thread
	//	- MsgCont
	//	- MsgProc

	while( g_condTestDm )
	{
		sleep(1);
	}

	printf("%s while end!!\n", PROG_NAME);

	pthread_join(msg_in_t, (void **)&stat_msg_in_t);
	pthread_join(msg_out_t, (void **)&stat_msg_out_t);

	printf("%s thread end!!\n", PROG_NAME);

	return EXIT_SUCCESS;
}

int CheckProgRunning()
{
	FILE *file;
	char cmd[32] = {0,};
	char buf[32] = {0,};

	sprintf(cmd, "ps -ae | grep %s | wc -l", PROG_NAME);
	file = popen(cmd, "r");
	if(file == NULL)
		return -1;

	fgets(buf, sizeof(buf), file);
	pclose(file);

	return atoi(buf);
}

int SetSignal()
{
	sigset_t    tSigSet;
	sigemptyset( &tSigSet );
	sigaddset( &tSigSet, SIGIO );
	pthread_sigmask( SIG_SETMASK , &tSigSet , NULL );

	struct sigaction tAct;
	memset( &tAct , 0 , sizeof( struct sigaction ) );
	tAct.sa_handler = ProcSignalStop;
	tAct.sa_flags   |= SA_RESTART;

	//Ctrl+C interrupt signal
	if( sigaction( SIGINT , &tAct , 0 ) != 0 )
		return-1;

	//kill signal
	if( sigaction( SIGTERM , &tAct , 0 ) != 0 )
		return-1;

	return 0;
}

void ProcSignalStop(int sig_no)
{
	printf("testdm stop signal (%d)\n", sig_no);

	StopMsgThread();
	g_condTestDm = 0;

	//exit(0);
}
