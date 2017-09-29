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
#include <sys/stat.h>
#include <sys/types.h>
#include "../tbase/def.h"
#include "../tbase/tbase.h"
#include "msgctrl.h"

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.3"

int CheckProgRunning();
void ProcSignalStop(int sig_no);
int SetSignal();
int CreateTestFolder(string path);
void* TestMsgqThread(void *arg);

int g_condTestDm = 1;
int g_condTestMsgq = 1;
int g_idTestMsgq = 0;
int g_idDebugMsgq = 0;
int g_idTpc = 0;

pthread_t idTestMsgqThread;

CMsgBox *g_pTestMsgBox = NULL;
CMsgBox *g_pDebugMsgBox = NULL;

int main(int argc, char *argv[])
{
	//running check
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

	//get tpc bd id;
	g_idTpc = 1;

	//init test msg box
	string strMsgSendTo, strMsgRecvFrom;
	char path[128];

	sprintf(path, "%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);
	CreateTestFolder(path);

	strMsgSendTo = path + string(MSGBOX_SEND_TO);
	strMsgRecvFrom = path + string(MSGBOX_RECV_FROM);

	g_pTestMsgBox = new CMsgBox(strMsgSendTo, strMsgSendTo);
	g_pTestMsgBox->StartThread();

	//TestMsgqThread MsgBox <-> TestMsgq
	MsgPackQ proc_msgq;
	g_idTestMsgq = CreateMsgq(KEY_TEST_MSGQ);

	pthread_create(&idTestMsgqThread, NULL, &TestMsgqThread, (void*)0);

	while( g_condTestDm )
	{
		//send msg to MsgBox
		if(CurNumMsgq(g_idTestMsgq) > 0)
		{
			memset(&proc_msgq, 0, sizeof(MsgPackQ));
			proc_msgq.type = TYPE_MSGQ_RECV;

			if(RecvMsgq(g_idTestMsgq, &proc_msgq, sizeof(MsgPackQ)) == 0)
			{
				printf(">> incoming process msg!!!\n");
//				/g_pTestMsgBox->SendMsg(temp_msgq.msg);
			}
		}

		msleep(100);
	}

	printf("%s end!!\n", PROG_NAME);

	delete g_pTestMsgBox;
	return EXIT_SUCCESS;
}

int CheckProgRunning()
{
	int pid = getpid();

	FILE *file = fopen("/tmp/%s.pid", "r");
/*
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
*/
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

	if(g_pTestMsgBox != NULL)
		g_pTestMsgBox->StopThread();

	if(idTestMsgqThread != 0)
	{
		g_condTestMsgq = 0;
		pthread_join(idTestMsgqThread, NULL);
	}

	g_condTestDm = 0;
}

int CreateTestFolder(string path)
{
	string temp, subdir;
	size_t current, previous = 0;
	current = path.find_first_of('/');

	while(current != string::npos)
	{
		temp = path.substr(previous, current - previous);
		previous = current + 1;
		current = path.find_first_of('/', previous);

		if( temp.empty() )
			continue;

		subdir = path.substr(0, previous).c_str();
		if(access(subdir.c_str(), F_OK) != 0)
		{
			mkdir(subdir.c_str(), 0755);
		}
	}

	return 0;
}

void* TestMsgqThread(void *arg)
{
	int ret;
	MsgPack read_msg;
	MsgPackQ temp_msgq;

	while(g_condTestMsgq)
	{
		//send msg to MsgBox
		if(CurNumMsgq(g_idTestMsgq) > 0)
		{
			memset(&temp_msgq, 0, sizeof(MsgPackQ));
			temp_msgq.type = TYPE_MSGQ_SEND;

			ret = RecvMsgq(g_idTestMsgq, &temp_msgq, sizeof(MsgPackQ));
			if(ret == 0)
				g_pTestMsgBox->SendMsg(temp_msgq.msg);
		}

		//read msg from MsgBox
		ret = g_pTestMsgBox->RecvMsg(read_msg);
		if(ret == 0)
		{
			memset(&temp_msgq, 0, sizeof(MsgPackQ));
			temp_msgq.type = TYPE_MSGQ_RECV;
			temp_msgq.msg = read_msg;

			SendMsgq(g_idTestMsgq, &temp_msgq, sizeof(MsgPackQ));
		}

		msleep(100);
	}

	pthread_exit( (void* )0 );
}
