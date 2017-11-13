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

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.9"

using namespace std;

int main(int argc, char *argv[])
{
	//running check
	if(CheckProgRunning() > 2)
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

	//init test folder
	char szTestMsgSendTo[PATHNAME_SIZE];
	char szTestMsgRecvFrom[PATHNAME_SIZE];

	CreateTestFolders();

	sprintf(szTestMsgSendTo, "%s/%s", g_szTestPath, MSGBOX_SEND_TO);
	sprintf(szTestMsgRecvFrom, "%s/%s", g_szTestPath, MSGBOX_RECV_FROM);

	cout << "strTestMsgSendTo : " << szTestMsgSendTo << endl;
	cout << "strTestMsgRecvFrom : " << szTestMsgRecvFrom << endl;


	//MsgqThread
	g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, szTestMsgSendTo, szTestMsgRecvFrom);
    g_pTestMsgq->StartThread(ProcRecvMsg);

    //Socket Server
    g_pSocketServer = new CSocketServer((char *)"127.0.0.1", PORT_TDM);
    g_pSocketServer->StartThread(&ProcRecvSock);

    //Socket Client
    //g_pSocketClient  = new CSocketClient(g_idTpc, (char *)"127.0.0.1", PORT_TDM);
    g_pSocketClient  = new CSocketClient(g_idTpc, (char *)"192.168.10.68", 3132);
    g_pSocketClient->StartThread(&ProcRecvSock);

    //TestMng
    g_ppTestMng = new CTestMng*[PORT_MAX];

    for(int idx=0; idx<PORT_MAX; idx++)
    {
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_INIT,	0, PROG_VERSION);
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_INITCOLOR,0, "");
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT1, 	0, "EMPTY");
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT2, 	0, "");
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT3, 	0, PROG_VERSION);
        SendMsg(g_pTestMsgq->idMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT4, 	0, "");

        g_ppTestMng[idx] = new CTestMng(g_idTpc, idx);
    }

    //Monitoring
    g_pStatusMon = new CStatus();

    //TPC b'd status monitor
	g_condTestDm = 1;
	while( g_condTestDm )
	{
		//this is for monitoring job
        //msleep(100);
        //g_pStatusMon->CheckAll();
        sleep(1);
	}

    //clear resources
    if(g_pStatusMon != NULL)
    {
        delete g_pStatusMon;
    }

    if(g_pTestMsgq != NULL)
    {
        g_pTestMsgq->StopThread();
        delete g_pTestMsgq;
    }

    if(g_pSocketClient != NULL)
	{
        g_pSocketClient->StopThread();
        delete g_pSocketClient;
	}

    if(g_ppTestMng != NULL)
	{
		for(int idx=0; idx<PORT_MAX; idx++)
		{
            delete g_ppTestMng[idx];
		}

        delete []g_ppTestMng;
	}


	printf("%s end!!\n", PROG_NAME);
	return EXIT_SUCCESS;
}

int CheckProgRunning()
{
	FILE *file;
	char cmd[32] = {0,};
	char buf[32] = {0,};

	sprintf(cmd, "ps -a | grep %s | wc -l", PROG_NAME);
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
