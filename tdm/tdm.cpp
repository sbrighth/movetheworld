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
#include "msgbox.h"

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.2"

int CheckProgRunning();
void ProcSignalStop(int sig_no);
int SetSignal();
int CreateWorkFolder(string path);

int g_condTestDm = 1;
int g_idMsgQIn = 0;
int g_idMsgQOut = 0;
int g_idTpc = 0;

CMsgBox *pMsgBox;

int main(int argc, char *argv[])
{
	//running check
	if(CheckProgRunning() > 0)
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

	//init test folder
	string strTestMsgSendTo, strTestMsgRecvFrom;
	string strTestPath;
	char buf[64];

	//sprintf(buf, "%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);
	sprintf(buf, "/tmp/rack_001/tester%03d/exe/", g_idTpc);
	strTestPath = buf;
	CreateWorkFolder(strTestPath);

	strTestMsgSendTo = strTestPath + string(MSGBOX_SEND_TO);
	strTestMsgRecvFrom = strTestPath + string(MSGBOX_RECV_FROM);

	cout << "strTestMsgSendTo : " << strTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << strTestMsgRecvFrom << endl;

	pMsgBox = new CMsgBox(strTestMsgSendTo, strTestMsgSendTo);
	pMsgBox->StartThread();

	while( g_condTestDm )
	{

		sleep(1);
	}

	printf("%s end!!\n", PROG_NAME);

	delete pMsgBox;
	return EXIT_SUCCESS;
}

int CheckProgRunning()
{
	FILE *file;
	char cmd[32] = {0,};
	char buf[32] = {0,};

	sprintf(cmd, "ps -ae | grep ^%s$ | wc -l", PROG_NAME);
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

	if(pMsgBox != NULL)
		pMsgBox->StopThread();

	g_condTestDm = 0;
}

int CreateWorkFolder(string path)
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
