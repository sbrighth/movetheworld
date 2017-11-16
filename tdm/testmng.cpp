/*
 * testmng.cpp
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#include <sstream>
#include <string>
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
        char cCell[32];
        char cPort[32];

        sprintf(cCell, "%d", pthis->iCell);
        sprintf(cPort, "%d", pthis->iPort);

        pthis->iChildStatus[iVersion] = exevp(pthis->strRunFile[iVersion].c_str(),
                                              pthis->strRunFile[iVersion].c_str(),
                                              pthis->strRunArg[iVersion].c_str(),
                                              NULL);
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
        printf(">> child status = %d\n", pthis->iChildStatus[iVersion]);
	}

    pthis->idThread[iVersion] = 0;
	pthread_exit((void *)0);
}

int CTestMng::StartTest(int iMsgVer, string strPath, string strFileName, string strArg)
{
    if(iMsgVer < MSGVER_NONE || iMsgVer > MSGVER_PORT_TEST)
        return -1;

    if(idThread[iMsgVer] == 0)
	{
		//check script file extension
		stringstream ss;
		string strCmd;
		string strScriptProcFile;
		string strScriptOnlyName;
        string strExtType;

        int iExtPos = CheckScriptExt(strFileName, TEST_SCRIPT_RUN_EXT);
		if(iExtPos > 0)
		{
			strScriptOnlyName = strFileName.substr(0, iExtPos-1);

			ss.str("");
            ss << SYS_WORK_PATH <<  "/" << strFileName;
			strScriptProcFile = ss.str();
		}
		else
		{
            printf(">> '%s' file is not eixst\n", strExtType.c_str());
			return -2;
		}

		printf(">> strScriptOnlyName = %s\n", strScriptOnlyName.c_str());
		printf(">> strScriptProcFile = %s\n", strScriptProcFile.c_str());

		//copy script to work folder
		ss.str("");
		ss << "cp -f " << strPath << "/" << strFileName << " " << strScriptProcFile;
		strCmd = ss.str();
		int ret = system(strCmd.c_str());

		printf(">> cp cmd = %s\n", strCmd.c_str());
		printf(">> ret = %d\n", ret);

        //set argument
        strRunArg[iMsgVer] = strArg;

		//compile script
		ss.str("");
        ss << SYS_WORK_PATH << "/" << strScriptOnlyName << iPort;
        strRunFile[iMsgVer] = ss.str();
        unlink(strRunFile[iMsgVer].c_str());

		ss.str("");
        ss << SYS_LOG_PATH << "/" << "compile" << iPort << ".txt";
		string strCompileLog;
		strCompileLog = ss.str();

		ss.str("");
        ss << COMPILE_PROG << " " << COMPILE_INCPATH << " " << strScriptProcFile << " -o " << strRunFile[iMsgVer] << " " << COMPILE_LIBPATH << " " << COMPILE_LIB << " 2> " << strCompileLog;
		strCmd = ss.str();

		printf(">> strCmd = %s\n", strCmd.c_str());
		ret = system(strCmd.c_str());
		printf(">> ret = %d\n", ret);

		if(ret != 0)
        {
			printf(">> compile error is happened!!\n");
			char buf[256];
			sprintf(buf, "cat %s > /dev/stdout", strCompileLog.c_str());
			system(buf);
			return -3;
		}

		//delete script file
		unlink(strScriptProcFile.c_str());

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
    if(idThread[iMsgVer] > 0)
		return 1;
	else
		return 0;
}

int CTestMng::CheckScriptExt(string strFileName, string strCheckExt)
{
	string strExt;
	string strName;

	size_t pos = strFileName.find(".", 0);
	if(pos == 0)
		return -1;

	strName = strFileName.substr(0, pos);
	strExt = strFileName.substr(pos+1, -1);

	cout << "strName : " << strName << endl;
	cout << "strSuffix : " << strExt << endl;

	if(strExt.compare(strCheckExt) == 0)
		return pos+1;

	return -2;
}

