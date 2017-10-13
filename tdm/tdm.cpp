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

using namespace std;

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

	//sprintf(g_test_path, "%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);
	sprintf(g_test_path, "/tmp/exicon/rack_001/tester%03d/exe/", g_idTpc);

	strTestPath = g_test_path;
	CreateWorkFolder(strTestPath);

	return 0;
	strTestMsgSendTo = strTestPath + string(MSGBOX_SEND_TO);
	strTestMsgRecvFrom = strTestPath + string(MSGBOX_RECV_FROM);

	cout << "strTestMsgSendTo : " << strTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << strTestMsgRecvFrom << endl;

	g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, strTestMsgSendTo, strTestMsgRecvFrom);
	g_pTestMsgq->StartThread(&RecvMsgProc);

	for(int idx=PORT_MIN; idx<=PORT_MAX; idx++)
	{
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_INIT, 		0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_INITCOLOR, 	0, "");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT1, 		0, "EMPTY");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT2, 		0, "");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT3, 		0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx, MSG_TEXT4, 		0, "");
	}

	while( g_condTestDm )
	{
		msleep(100);
	}

	if(g_pTestMsgq != NULL)
	{
		g_pTestMsgq->StopThread();
		delete g_pTestMsgq;
	}

	printf("%s end!!\n", PROG_NAME);

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

int CheckTestRunning(int port)
{
	if(port < PORT_MIN || port > PORT_MAX)
		return -1;

	return g_test_status[port];
}

void RecvMsgProc(int idMsgq, MsgPack msg)
{
	//int version		= msg.version;
	int cell    	= msg.cell;
	int port    	= msg.port-1;
	int msg_no		= msg.msg_no;
	//int packet 		= msg.packet;
	//int flag 		= msg.flag;
	string msg_str	= msg.string;

	if(cell != g_idTpc)
	{
		printf(">> wrong tpc no\n");
		return;
	}

	if(msg.port < PORT_MIN || msg.port > PORT_MAX)
		return;

	if(msg_no == MSG_TEST_START)
	{
		// test
		if( CheckTestRunning( port ) == OFF )
		{
			if(msg_str.length() == 0)
			{
				//set script1.uts to share memory
			}
			else
			{
				char real_name[256];

				if( SearchFile(g_test_path, msg_str.c_str(), real_name) == 0 )
				{
					//set main script to share memory
				}
			}

			StartTestThread(port);
			SendMsg(idMsgq, cell, port+1, MSG_TEST,	0, "");
		}
		else
		{
			printf(">> port %d is testing!!\n", port);
		}
	}
	else if(msg_no == MSG_TEST_STOP)
	{
		if( CheckTestRunning(port) == ON )
		{
			StopTestThread(port);
			SendMsg(idMsgq, cell, port+1, MSG_FAIL,	0, "");
		}
	}
	else if(msg_no == MSG_INIT)
	{
		int send_mode;

		if(msg_str.length() == 0)
		{
			send_mode = SC_FILE_DEFAULT;
		}
		else
		{
			string strScriptName = msg_str.substr(0, 14);
			string strScriptSize = msg_str.substr(15, 5);

			char buf[256];
			char real_name[256];

			//sprintf(buf, "%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);
			sprintf(buf, "/tmp/exicon/rack_001/tester%03d/exe/", g_idTpc);

			if( SearchFile(buf, strScriptName.c_str(), real_name) == 0 )
			{
				long in_size = atoi(strScriptSize.c_str());

				strcat(buf, real_name);
				long real_size = GetFileSize( buf );
				real_size = real_size % 10000;

				if( in_size == real_size )
				{
					send_mode = SC_FILE_PASS;
				}
				else
				{
					send_mode = SC_FILE_SIZE_ERR;
				}
			}
			else
			{
				send_mode = SC_FILE_NONE_ERR;
			}
		}

		switch(send_mode)
		{
		case SC_FILE_DEFAULT:
			SendMsg(idMsgq, cell, port+1, MSG_INITACK,	0, PROG_VERSION);
			SendMsg(idMsgq, cell, port+1, MSG_TEXT1, 	0, "");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT2, 	0, "");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT3, 	0, "");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT4, 	0, "");
			break;
		case SC_FILE_PASS:
			SendMsg(idMsgq, cell, port+1, MSG_INITACK,	SC_FILE_PASS, PROG_VERSION);
			SendMsg(idMsgq, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		case SC_FILE_SIZE_ERR:
			SendMsg(idMsgq, cell, port+1, MSG_INITACK,	SC_FILE_SIZE_ERR, PROG_VERSION);
			SendMsg(idMsgq, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		case SC_FILE_NONE_ERR:
			SendMsg(idMsgq, cell, port+1, MSG_INITACK,	SC_FILE_NONE_ERR, PROG_VERSION);
			SendMsg(idMsgq, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		}
	}
	else if(msg_no == MSG_INITACK)
	{
		printf(">> msg_no == MSG_INITACK\n");
	}
}

int StartTestThread(int port)
{
	return 0;
}

int StopTestThread(int port)
{
	return 0;
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
