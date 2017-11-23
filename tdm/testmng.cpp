/*
 * testmng.cpp
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "def.h"
#include "testmng.h"


CTestMng::CTestMng(int iCell, int iPort):iCell(iCell),iPort(iPort)
{
    memset(idThread, 0, sizeof(idThread));

    pthread_mutex_init(&syncMutex, NULL);
    pthread_cond_init(&syncCond, NULL);
}

CTestMng::~CTestMng()
{
    pthread_cond_destroy(&syncCond);
    pthread_mutex_destroy(&syncMutex);
}

void *TestThread(void *arg)
{
    CTestMng *pthis = (CTestMng *)arg;
    pthread_mutex_lock(&pthis->syncMutex);

    int iVersion = pthis->iVersion;
    pthread_cond_signal(&pthis->syncCond);
    pthread_mutex_unlock(&pthis->syncMutex);

    int iStatus = 0;

    pthis->iChildStatus[iVersion] = 0;

	pid_t pid = fork();
	if(pid < 0)
	{
		perror("fork error");
	}
	else if(pid == 0)
	{
        string strArgv;
        strArgv.append(pthis->strRunProg[iVersion].c_str());
        strArgv.append(" ");
        strArgv.append(pthis->strRunArg[iVersion].c_str());

        pthis->StrToArgv(strArgv, pthis->vectArgv[iVersion]);
        pthis->iChildStatus[iVersion] = execvp( pthis->vectArgv[iVersion][0],
                                                &pthis->vectArgv[iVersion][0]);
	}
	else
	{
        pthis->pidTestProcess[iVersion] = pid;
        waitpid(pid, &iStatus, 0);
        if(WIFSIGNALED(iStatus))
		{
            if(WTERMSIG(iStatus) == SIGSEGV)
				printf(">> pid:%d segmentaion fault\n", pid);
			else
                printf(">> pid:%d signal:%d\n", pid, WIFSIGNALED(iStatus));
		}

        for(size_t i=0; i<pthis->vectArgv[iVersion].size(); i++)
        {
            delete [] pthis->vectArgv[iVersion][i];
        }

        printf(">> child status = %d\n", pthis->iChildStatus[iVersion]);
	}

    pthis->idThread[iVersion] = 0;
	pthread_exit((void *)0);
}

int CTestMng::StrToArgv(string strArgs, vector<char*> &argv)
{
    istringstream iss(strArgs);
    string token;

    while(iss >> token)
    {
        char *arg = new char [token.size()+1];
        arg[token.size()] = '\0';
        copy(token.begin(), token.end(), arg);
        argv.push_back(arg);
    }
    argv.push_back(0);

    return 0;
}

int CTestMng::StartTest(int iMsgVer, string strProcFile, string strRunFile, string strArg)
{
    if(iMsgVer < MSGVER_NONE || iMsgVer > MSGVER_PORT_TEST)
        return -1;

    if(idThread[iMsgVer] == 0)
    {
        //set run argument
        strRunProg[iMsgVer] = strRunFile;
        strRunArg[iMsgVer] = strArg;

		//compile script
        unlink(strRunFile.c_str());

        stringstream ss;
        ss << SYS_LOG_PATH << "/" << "compile_v" << iMsgVer << "_p" << iPort+1 << ".txt";
        string strCompileLog;
		strCompileLog = ss.str();

		ss.str("");
        ss << COMPILE_PROG << " " << COMPILE_INCPATH << " " << strProcFile << " -o " << strRunFile << " " << COMPILE_LIBPATH << " " << COMPILE_LIB << " 2> " << strCompileLog;
        string strCmd = ss.str();

		printf(">> strCmd = %s\n", strCmd.c_str());
        int ret = system(strCmd.c_str());
		printf(">> ret = %d\n", ret);

		if(ret != 0)
        {
			printf(">> compile error is happened!!\n");
			char buf[256];
			sprintf(buf, "cat %s > /dev/stdout", strCompileLog.c_str());
			system(buf);
            return -2;
		}

        unlink(strProcFile.c_str());

        //set log file
        mngLog.AddLogFile()


		//run script
        pthread_mutex_lock(&syncMutex);

        this->iVersion = iMsgVer;
        pthread_create(&idThread[iMsgVer], NULL, TestThread, (void*)this);

        pthread_cond_wait(&syncCond, &syncMutex);
        pthread_mutex_unlock(&syncMutex);
        pthread_detach(idThread[iMsgVer]);
	}
	else
	{
		printf(">> test is running\n");
		return -1;
	}

	return 0;
}

int CTestMng::StopTest(int iMsgVer)
{
    if(iMsgVer < MSGVER_NONE || iMsgVer > MSGVER_PORT_TEST)
        return -1;

    if(idThread[iMsgVer] > 0)
	{
        printf("child process status = %d\n", iChildStatus[iMsgVer]);
        kill(pidTestProcess[iMsgVer], SIGKILL);

		int cnt = 0;
		do
		{
			usleep( 100*1000 );
			cnt++;

        } while( idThread[iMsgVer] > 0 && cnt < 10 );

        if(idThread[iMsgVer] > 0)
			return -1;
	}

	return 0;
}

int CTestMng::IsTestOn(int iMsgVer)
{
    if(iMsgVer < MSGVER_NONE || iMsgVer > MSGVER_PORT_TEST)
        return -1;

    if(idThread[iMsgVer] > 0)
		return 1;
	else
		return 0;
}
