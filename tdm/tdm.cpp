//============================================================================
// Name        : testdm.cpp
// Author      : shjeong
// Version     :
// Copyright   : exicon
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "def.h"
#include "base.h"
#include "msgqthread.h"
#include "msgsend.h"
#include "tdm.h"

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.5"

using namespace std;

int g_condTestDm = 1;
int g_idTpc = 0;
CMsgqThread *g_pTestMsgq = NULL;

int main(int argc, char *argv[])
{
	printf(">> cplusplus version = %d\n", __cplusplus);
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
	sprintf(buf, "/tmp/exicon/rack_001/tester%03d/exe/", g_idTpc);
	strTestPath = buf;
	CreateWorkFolder(strTestPath);

	strTestMsgSendTo = strTestPath + string(MSGBOX_SEND_TO);
	strTestMsgRecvFrom = strTestPath + string(MSGBOX_RECV_FROM);

	cout << "strTestMsgSendTo : " << strTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << strTestMsgRecvFrom << endl;

	g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, strTestMsgSendTo, strTestMsgRecvFrom);
	g_pTestMsgq->StartThread(&RecvMsgProc);

	for(int idx=PORT_MIN; idx<=PORT_MAX; idx++)
	{
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_INIT, 		PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_INITCOLOR, 	"");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT1, 		"EMPTY");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT2, 		"");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT3, 		PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT4, 		"");
	}

	while( g_condTestDm )
	{
		msleep(100);
	}

	printf("%s end!!\n", PROG_NAME);
	delete g_pTestMsgq;
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

	if(g_pTestMsgq != NULL)
		g_pTestMsgq->StopThread();

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

void RecvMsgProc(int idMsgq, MsgPack msg)
{
	MsgPack msg_temp;
	memset(&msg_temp, 0, sizeof(MsgPack));

	msg_temp.cell = msg.cell;
	msg_temp.port = msg.port-1;

	if(msg.cell != g_idTpc)
		return;

	if(msg.port < PORT_MIN || msg.port > PORT_MAX)
		return;

	switch(msg.msg_no)
	{
	case MSG_TEST_START:
		//compile
		//run
		msg_temp.msg_no = MSG_TEST;
		SendMsgPack(idMsgq, msg_temp);
		break;
	case MSG_TEST_STOP:
		msg_temp.msg_no = MSG_FAIL;
		SendMsgPack(idMsgq, msg_temp);
		break;
	case MSG_INIT:
		//init dut status
		//init exe folder
		msg_temp.msg_no = MSG_INITACK;
		SendMsgPack(idMsgq, msg_temp);
		break;
	case MSG_INITACK:
		break;
	}
}


int test_compile_script()
{
	int ret;
	string compiler = "g++";
	string script = "/tmp/exicon/script/sample.c";
	string target = "/tmp/exicon/script/sample";
	string incpath = "-I/tmp/exicon/include";
	string libpath = "-L/tmp/exicon/lib";
	string lib = "-ltbase -ltnet";

	int status;
	pid_t pid;
	unlink(target.c_str());

	pid = fork();

	if(pid < 0)
	{
		perror("fork error");
		ret = -1;
	}
	else if(pid == 0)
	{
		printf("compile child process\n");
		ret = execlp(compiler.c_str(), compiler.c_str(), incpath.c_str(), "-o", target.c_str(), script.c_str(), libpath.c_str(), NULL);
	}
	else
	{
		waitpid(pid, &status, 0);
		ret = pid;
	}

	printf("ret= %d\n", ret);
	return ret;
}

int test_process()
{
	int ret;
	int status;
	string target = "/tmp/exicon/script/sample";
	size_t pos = target.rfind('/') + 1;
	char cell[8] = "1";
	char port[8] = "2";

	pid_t pid = fork();

	if(pid < 0)
	{
		perror("fork error");
		ret = -1;
	}
	else if(pid == 0)
	{
		printf("child process\n");
		ret = execl(target.c_str(), target.substr(pos).c_str(), cell, port, NULL);
	}
	else
	{
		waitpid(pid, &status, 0);
		ret = pid;
	}

	printf("ret= %d\n", ret);
	return ret;
}
