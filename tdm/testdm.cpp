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
	g_idTpc = 2;
	g_idResShare = CreateShmem(KEY_RES_SHARE, 512);

	//init test folder
	char szTestMsgSendTo[PATHNAME_SIZE];
	char szTestMsgRecvFrom[PATHNAME_SIZE];


	CreateTestFolders();

	sprintf(szTestMsgSendTo, "%s/%s", g_szTestPath, MSGBOX_SEND_TO);
	sprintf(szTestMsgRecvFrom, "%s/%s", g_szTestPath, MSGBOX_RECV_FROM);

	cout << "strTestMsgSendTo : " << szTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << szTestMsgRecvFrom << endl;

	g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, szTestMsgSendTo, szTestMsgRecvFrom);
	g_pTestMsgq->StartThread(&RecvMsgProc);

	g_pTestMng = new CTestMng*[PORT_MAX];

	for(int idx=0; idx<PORT_MAX; idx++)
	{
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_INIT,	0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_INITCOLOR,0, "");
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_TEXT1, 	0, "EMPTY");
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_TEXT2, 	0, "");
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_TEXT3, 	0, PROG_VERSION);
		SendMsg(g_pTestMsgq->idMsgq, TYPE_MSG_TEST, g_idTpc, idx+1, MSG_TEXT4, 	0, "");

		g_pTestMng[idx] = new CTestMng(g_idTpc, idx);
	}

	g_condTestDm = 1;
	while( g_condTestDm )
	{
		//this is for monitoring job
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

int CreateTestFolders()
{
	char szMakePath[PATHNAME_SIZE];
	sprintf(szMakePath, "%s/rack_001/tester%03d/exe", SYS_ATH_PATH, g_idTpc);
	CreateFolder(szMakePath);
	memcpy(g_szTestPath, szMakePath, sizeof(g_szTestPath));

	sprintf(szMakePath, "%s", SYS_DATA_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_LIB_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_INC_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_LOG_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_EXEC_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_SCRIPT_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_WORK_PATH);
	CreateFolder(szMakePath);

	sprintf(szMakePath, "%s", SYS_UPDATE_PATH);
	CreateFolder(szMakePath);

	return 0;
}

int CreateFolder(char *path)
{
	if(path == NULL)
		return -1;

	char cmd[PATHNAME_SIZE];
	sprintf(cmd, "mkdir -p %s", path);
	system(cmd);

	return 0;
}

int CheckScriptFile(char *msg_str)
{
	int iSendMode = SC_FILE_DEFAULT;
	char szRealName[PATHNAME_SIZE] = {0,};
	char szFileName[PATHNAME_SIZE] = {0,};

	if(strlen(msg_str) > 0)
	{
		int nNameSize = 15;	//0~14
		int nFileSize = 4;	//15~18

		char *szScriptName = new char[nNameSize+1];
		char *szScriptSize = new char[nFileSize+1];

		memset(szScriptName, 0, sizeof(szScriptName));
		memset(szScriptSize, 0, sizeof(szScriptSize));

		memcpy(szScriptName, msg_str, sizeof(szScriptName));
		memcpy(szScriptSize, msg_str+nNameSize, sizeof(szScriptSize));

		for(int i=nNameSize; i>0; i--)
		{
			if(szScriptName[i-1] != '0')
			{
				szScriptName[i] = 0;
				break;
			}
		}

		printf("scriptName= %s\n", szScriptName);
		printf("scriptSize= %s\n", szScriptSize);

		if( SearchFile(g_szTestPath, szScriptName, szRealName) == 0 )
		{
			long lScriptSize = atoi(szScriptSize);

			sprintf(szFileName, "%s/%s", g_szTestPath, szRealName);
			printf(">. szFileName = %s\n", szFileName);
			long lFileSize = GetFileSize(szFileName);
			lFileSize = lFileSize % 10000;

			printf(">> msg file size = %ld, read file size = %ld\n", lScriptSize, lFileSize);
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

		delete szScriptName;
		delete szScriptSize;
	}

	return iSendMode;
}

void RecvMsgProc(int idMsgq, MsgPack msg)
{
	int version		= msg.version;
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
		if( g_pTestMng[port]->IsTestOn() == 0 )
		{
			char szRealName[PATHNAME_SIZE] = {0,};

			if( SearchFile(g_szTestPath, msg_str, szRealName) == 0 )
			{
				if(g_pTestMng[port]->StartTest(g_szTestPath, szRealName) == 0)
				{
					printf(">> test is started!!\n");
					SendMsg(idMsgq, version, cell, port+1, MSG_TEST,	0, "");
				}
			}
			else
			{
				printf(">> port %d script is not exist!!\n", port+1);
				//SendMsg(idMsgq, version, cell, port+1, MSG_DONE,	0, "SCRIPT NOT EXIST");
			}
		}
		else
		{
			printf(">> port %d is testing!!\n", port+1);
		}
	}
	else if(msg_no == MSG_TEST_STOP)
	{
		if(g_pTestMng[port]->IsTestOn() == 1)
		{
			g_pTestMng[port]->StopTest();
			//SendMsg(idMsgq, version, cell, port+1, MSG_FAIL,	0, "ABORT");
			//SendMsg(idMsgq, version, cell, port+1, MSG_FAIL,	0, "SLOT ABORT");
		}
	}
	else if(msg_no == MSG_INIT)
	{
		switch(CheckScriptFile(msg_str))
		{
		case SC_FILE_DEFAULT:
			SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_DEFAULT, PROG_VERSION);
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,    0,	"MSG_INIT");
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT2,    0, "");
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT3,    0, "");
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT4,    0, "");
			break;
		case SC_FILE_PASS:
			SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_PASS, PROG_VERSION);
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		case SC_FILE_SIZE_ERR:
			SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_SIZE_ERR, PROG_VERSION);
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		case SC_FILE_NONE_ERR:
			SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_NONE_ERR, PROG_VERSION);
			SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
			break;
		case SC_FILE_OTHER_ERR:
			break;
		}
	}
	else if(msg_no == MSG_INITACK)
	{

	}
}
