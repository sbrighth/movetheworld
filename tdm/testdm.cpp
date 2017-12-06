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

char szProgName[] = "tdm";
char szProgVersion[] = "0.0.11";

int main(int argc, char *argv[])
{
	//running check
	int cnt = CheckProgRunning();
	if(cnt > 1)
	{
        printf("%s is runinning\n", szProgName);
        return -1;
    }
	else
	{
        printf("%s start!!\n", szProgName);
	}

    //set stop signal
	if(SetSignal() < 0)
		return -1;

    GetTpcId();
    CreateTestFolders();
    InitResource();
    NotifyProgReady();

    g_condTestDm = 1;
	while( g_condTestDm )
	{
        for(int iPortIdx=PORT_MIN; iPortIdx<PORT_MAX; iPortIdx++)
            g_pTestMng[iPortIdx]->mngLog.CopyLogFile();

        sleep(3);
	}

    DeleteResource();
    printf("%s end!!\n", szProgName);

	return EXIT_SUCCESS;
}

void InitResource()
{
    //msg read/write file
    char szTestMsgSendTo[PATHNAME_SIZE];
    char szTestMsgRecvFrom[PATHNAME_SIZE];
    sprintf(szTestMsgSendTo, "%s/%s", g_szTesterPath, MSGBOX_SEND_TO);
    sprintf(szTestMsgRecvFrom, "%s/%s", g_szTesterPath, MSGBOX_RECV_FROM);

    cout << "strTestMsgSendTo : " << szTestMsgSendTo << endl;
    cout << "strTestMsgRecvFrom : " << szTestMsgRecvFrom << endl;

    //MsgqThread
    g_pTestMsgq = new CMsgqThread(KEY_TEST_MSGQ, szTestMsgSendTo, szTestMsgRecvFrom);
    g_pTestMsgq->StartThread(ProcRecvMsg);
    g_idTestMsgq = g_pTestMsgq->idMsgq;

    //Socket Server
    g_pMsgSocketServer = new CMsgSocket(g_idTpc, (char *)LOCAL_IP, LOCAL_PORT, true);
    g_pMsgSocketServer->StartThread(&ProcRecvSock);

    //Msg Socket Client
    g_pMsgSocketClient  = new CMsgSocket(g_idTpc, (char *)SERVER_IP, SERVER_PORT, false);
    g_pMsgSocketClient->StartThread(&ProcRecvSock);

    //TestMng
    for(int idx=0; idx<PORT_CNT; idx++)
    {
        g_pTestMng[idx] = new CTestMng(g_idTpc, idx);
    }

    //Status
    g_pStatusMon = new CStatus(g_pTestMng);
    //g_pStatusMon->StartThread();
}


void DeleteResource()
{
    if(g_pStatusMon != NULL)
    {
        g_pStatusMon->StopThread();
        delete g_pStatusMon;
    }

    if(g_pTestMsgq != NULL)
    {
        g_pTestMsgq->StopThread();
        delete g_pTestMsgq;
    }

    if(g_pMsgSocketServer != NULL)
    {
        g_pMsgSocketServer->StopThread();
        delete g_pMsgSocketServer;
    }

    if(g_pMsgSocketClient != NULL)
    {
        g_pMsgSocketClient->StopThread();
        delete g_pMsgSocketClient;
    }

    for(int idx=0; idx<PORT_CNT; idx++)
    {
        if(g_pTestMng[idx])
            delete g_pTestMng[idx];
    }

    if(g_szTesterPath)
        delete []g_szTesterPath;

    if(g_szWorkExecPath)
        delete []g_szWorkExecPath;

    for(int idx=0; idx<PORT_CNT; idx++)
    {
        if(g_szTesterPortPath[idx])
            delete []g_szTesterPortPath[idx];

        if(g_szWorkPortPath[idx])
            delete []g_szWorkPortPath[idx];
    }
}

int CheckProgRunning()
{
	FILE *file;
	char cmd[32] = {0,};
	char buf[32] = {0,};

    sprintf(cmd, "ps -e | grep %s | wc -l", szProgName);
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

int GetTpcId()
{
    char buf[32] = {0,};
    int cnt = ReadFileText((char*)FILE_TPC_ID, buf, 0, sizeof(buf));

    if(cnt == 0)
    {
        g_idTpc = TPC_ID_DEBUG;
        return -1;
    }

    g_idTpc = atoi(buf);
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

    sprintf(szMakePath, "%s", SYS_SHA_PATH);
    CreateFolder(szMakePath);

    sprintf(szMakePath, "%s", SYS_WORK_PATH);
    CreateFolder(szMakePath);

    sprintf(szMakePath, "%s", SYS_UPDATE_PATH);
    CreateFolder(szMakePath);

    //for share folder
    g_szTesterPath = new char[PATHNAME_SIZE];
    sprintf(g_szTesterPath, "%s/%03d", SYS_SHA_TESTER_PATH, g_idTpc);

    for(int iPortIdx=PORT_MIN; iPortIdx<PORT_CNT; iPortIdx++)
    {
        g_szTesterPortPath[iPortIdx] = new char[PATHNAME_SIZE];
        sprintf(g_szTesterPortPath[iPortIdx], "%s/%02d", g_szTesterPath, iPortIdx+1);
        CreateFolder(g_szTesterPortPath[iPortIdx]);

        sprintf(szMakePath, "%s/exec", g_szTesterPath);
        CreateFolder(szMakePath);
    }

    //for work folder
    g_szWorkExecPath = new char[PATHNAME_SIZE];
    sprintf(g_szWorkExecPath, "%s/exec", SYS_WORK_PATH);
    CreateFolder(g_szWorkExecPath);

    for(int iPortIdx=PORT_MIN; iPortIdx<PORT_CNT; iPortIdx++)
    {
        g_szWorkPortPath[iPortIdx] = new char[PATHNAME_SIZE];
        sprintf(g_szWorkPortPath[iPortIdx], "%s/%02d", SYS_WORK_PATH, iPortIdx+1);
        CreateFolder(g_szWorkPortPath[iPortIdx]);
    }

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

int NotifyProgReady()
{
    if(g_pTestMsgq == NULL)
        return -1;

    if(g_idTestMsgq < 0)
        return -2;

    for(int idx=0; idx<PORT_CNT; idx++)
    {
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_INIT,	0, szProgVersion);
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_INITCOLOR,0, "");
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT1, 	0, "EMPTY");
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT2, 	0, "");
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT3, 	0, szProgVersion);
        SendMsg(g_idTestMsgq, MSGVER_PORT_TEST, g_idTpc, idx+1, MSG_TEXT4, 	0, "");
    }

    return 0;
}
