/*
 * testmng.cpp
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sstream>
#include "def.h"
#include "testmng.h"

CTestMng::CTestMng(int iCell, int iPort):iCell(iCell),iPort(iPort)
{
	condThread = 1;
	idThread = 0;
}

CTestMng::~CTestMng()
{
}

void *TestThread(void *arg)
{
	CTestMng *pthis = (CTestMng*)arg;
	int status = 0;
	pthis->iChildStatus = 0;

	pid_t pid = fork();
	if(pid < 0)
	{
		perror("fork error");
	}
	else if(pid == 0)
	{
		char cCell[2];
		char cPort[2];
		sprintf(cCell, "%d", pthis->iCell);
		sprintf(cPort, "%d", pthis->iPort);

		pthis->iChildStatus = execl(pthis->strRunFile.c_str(), pthis->strRunFile.c_str(), cCell, cPort, NULL);
	}
	else
	{
		pthis->pidTestProcess = pid;
		waitpid(pid, &status, 0);
		printf(">> child status = %d\n", pthis->iChildStatus);
	}

	pthis->idThread = 0;
	pthread_exit((void *)0);
}

int CTestMng::StartTest(string strPath, string strFileName)
{
	if(idThread == 0)
	{
		//check script file extension
		stringstream ss;
		string strCmd;
		string strScriptProcFile;
		string strScriptOnlyName;

		int iExtPos = CheckScriptExt(strFileName, TEST_SCRIPT_ORI_EXT);
		if(iExtPos > 0)
		{
			strScriptOnlyName = strFileName.substr(0, iExtPos-1);

			ss.str("");
			ss << SYS_WORK_PATH <<  "/" << strFileName.substr(0, iExtPos) << TEST_SCRIPT_RUN_EXT;
			strScriptProcFile = ss.str();
		}
		else
		{
			printf(">> '%s' file is not eixst\n", TEST_SCRIPT_ORI_EXT);
			return -2;
		}

		printf(">> strScriptOnlyName = %s\n", strScriptOnlyName.c_str());
		printf(">> strScriptProcFile = %s\n", strScriptProcFile.c_str());
		//copy script to work folder

		ss.str("");
		ss << "cp -f " << strPath << "/" << strFileName << " " << strScriptProcFile;
		strCmd = ss.str();
		system(strCmd.c_str());

		printf(">> cp cmd = %s\n", strCmd.c_str());

		//compile script
		/*
		string compiler	= "/usr/bin/gcc";
		string incpath	= "-I/tmp/exicon/include";
		string libpath	= "-L/tmp/exicon/lib";
		string lib		= "-ltbase -ltnet";
		*/

		ss.str("");
		ss << SYS_WORK_PATH << "/" << strScriptOnlyName << iPort+1;
		strRunFile = ss.str();
		unlink(strRunFile.c_str());

		ss.str("");
		ss << COMPILE_PROG << " " << COMPILE_INCPATH << " " << strScriptProcFile << " -o " << strRunFile << " " << COMPILE_LIBPATH << " " << COMPILE_LIB << " 2>&1";
		strCmd = ss.str();

		FILE *file;
		file = popen(strCmd.c_str(), "r");
		if(file == NULL)
		{
			printf(">> compile execute error!!\n");
			return -3;
		}
		else
		{
			char buf[256] = {0,};
			stringstream ssCompileErr;
			ssCompileErr.str("");
			printf(">>  size = %zu\n", ssCompileErr.str().size());

			while(fgets(buf, sizeof(buf), file))
			{
				ssCompileErr << buf;
			}

			pclose(file);
			printf(">>  size = %zu\n", ssCompileErr.str().size());
			if(ssCompileErr.str().size() > 0)
			{
				printf(">> compile error is happened!!\n");
				printf("str = %s\n", ssCompileErr.str().c_str());
				printf("errono = %d\n", errno);
				return -4;
			}
		}

		//delete script file
		unlink(strScriptProcFile.c_str());

		//run script
		pthread_create(&idThread, NULL, TestThread, (void*)this);
		pthread_detach(idThread);
	}
	else
	{
		printf(">> test is running\n");
		return -1;
	}

	return 0;
}

int CTestMng::StopTest()
{
	if(idThread > 0)
	{
		printf("child process status = %d\n", iChildStatus);
		kill(pidTestProcess, SIGKILL);

		int cnt = 0;
		do
		{
			usleep( 100*1000 );
			cnt++;

		} while( idThread > 0 && cnt < 10 );

		if(idThread > 0)
			return -1;
	}

	return 0;
}

int CTestMng::IsTestOn()
{
	if(idThread > 0)
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

