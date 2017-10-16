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
#include "testdm.h"


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
	g_idResShare = CreateShmem(KEY_RES_SHARE, 512);

	//init test folder
	string strTestMsgSendTo, strTestMsgRecvFrom;
	string strTestPath;

	//sprintf(g_szTestPath, "%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);
	sprintf(g_szTestPath, "/tmp/%s/rack_001/tester%03d/exe/", SYS_ATH_PATH, g_idTpc);

	strTestPath = g_szTestPath;
	CreateWorkFolder(strTestPath);

	strTestMsgSendTo = strTestPath + MSGBOX_SEND_TO;
	strTestMsgRecvFrom = strTestPath + MSGBOX_RECV_FROM;

	cout << "strTestMsgSendTo : " << strTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << strTestMsgRecvFrom << endl;

	g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, strTestMsgSendTo, strTestMsgRecvFrom);
	g_pTestMsgq->StartThread(&RecvMsgProc);

	g_pTestMng = new CTestMng*[PORT_MAX];

	for(int idx=0; idx<PORT_MAX; idx++)
	{
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_INIT, 		0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_INITCOLOR, 0, "");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_TEXT1, 	0, "EMPTY");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_TEXT2, 	0, "");
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_TEXT3, 	0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, g_idTpc, idx+1, MSG_TEXT4, 	0, "");

		g_pTestMng[idx] = new CTestMng(g_idTpc, idx);
	}

	g_condTestDm = 1;
	while( g_condTestDm )
	{
		msleep(100);
	}

	if(g_pTestMsgq != NULL)
	{
		g_pTestMsgq->StopThread();
		delete g_pTestMsgq;
	}

	if(g_pTestMng != NULL)
	{
		for(int idx=0; idx<PORT_MAX; idx++)
		{
			delete g_pTestMng[idx];
		}

		delete []g_pTestMng;
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

int CheckScriptFile(char *msg_str)
{
	int iSendMode = SC_FILE_DEFAULT;
	char szRealName[PATHNAME_SIZE] = {0,};
	char szFileName[PATHNAME_SIZE] = {0,};

	if(strlen(msg_str) > 0)
	{
		char szScriptName[14]={0,};
		char szScriptSize[5]={0,};

		memcpy(szScriptName, msg_str, sizeof(szScriptName));
		memcpy(szScriptSize, msg_str+sizeof(szScriptName)+1, sizeof(szScriptSize));

		if( SearchFile(g_szTestPath, szScriptName, szRealName) == 0 )
		{
			long lScriptSize = atoi(szScriptSize);

			sprintf(szFileName, "%s%s", g_szTestPath, szRealName);
			long lFileSize = GetFileSize(szFileName);
			lFileSize = lFileSize % 10000;

			if( lScriptSize == lFileSize )
			{
				iSendMode = SC_FILE_PASS;
			}
			else
			{
				iSendMode = SC_FILE_SIZE_ERR;
			}
		}
		else
		{
			iSendMode = SC_FILE_NONE_ERR;
		}
	}

	return iSendMode;
}

void RecvMsgProc(int idMsgq, MsgPack msg)
{
	//int version		= msg.version;
	int cell    	= msg.cell;
	int port    	= msg.port-1;	// 0, 1
	int msg_no		= msg.msg_no;
	//int packet 		= msg.packet;
	//int flag 		= msg.flag;
	char *msg_str	= msg.string;

	if(cell != g_idTpc)
	{
		printf(">> invalid tpc no(c:%d, g_idTpc:%d)\n", cell, g_idTpc);
		return;
	}

	if(msg.port < PORT_MIN || msg.port > PORT_MAX)
		return;

	if(msg_no == MSG_TEST_START)
	{
		if( g_pTestMng[port]->IsTestOn() == OFF )
		{
			char szRealName[PATHNAME_SIZE] = {0,};

			if( SearchFile(g_szTestPath,msg_str, szRealName) == 0 )
			{
				if(g_pTestMng[port]->StartTest(g_szTestPath, szRealName) == 0)
				{
					printf(">> test is started!!\n");
					SendMsg(idMsgq, cell, port+1, MSG_TEST,	0, "");
				}
			}
			else
			{
				printf(">> port %d script is not exist!!\n", port);
			}
		}
		else
		{
			printf(">> port %d is testing!!\n", port);
		}
	}
	else if(msg_no == MSG_TEST_STOP)
	{
		if(g_pTestMng[port]->IsTestOn() == ON)
		{
			g_pTestMng[port]->StopTest();
			SendMsg(idMsgq, cell, port+1, MSG_FAIL,	0, "");
		}
	}
	else if(msg_no == MSG_INIT)
	{
		switch(CheckScriptFile(msg_str))
		{
		case SC_FILE_DEFAULT:
			SendMsg(idMsgq, cell, port+1, MSG_INITACK,	SC_FILE_DEFAULT, PROG_VERSION);
			SendMsg(idMsgq, cell, port+1, MSG_TEXT1,    0,	"MSG_INIT");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT2,    0, 	"");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT3,    0, 	"");
			SendMsg(idMsgq, cell, port+1, MSG_TEXT4,    0, 	"");
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
		case SC_FILE_OTHER_ERR:
			break;
		}
	}
	else if(msg_no == MSG_INITACK)
	{

	}
}
