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
#include <sstream>
#include "def.h"
#include "testmng.h"

void *TestThread(void *)
{
	//script run.
	pthread_exit((void *)0);
}

CTestMng::CTestMng(int iCell, int iPort):iCell(iCell),iPort(iPort)
{
	condThread = 1;
	idThread = 0;
}

CTestMng::~CTestMng()
{
}

int CTestMng::StartTest(string strPath, string strFileName)
{
	if(idThread == 0)
	{
		//copy script file to work folder
		int iExtPos = CheckScriptExt(strFileName);
		if(iExtPos < 0)
		{
			return -1;
		}
		else if(iExtPos == 0)
		{
			strScriptProcName = strFileName;
		}
		else
		{
			strScriptProcName = strFileName.substr(0, iExtPos) + TEST_SCRIPT_RUN_EXT;
		}

		string strWorkPath = "/tmp/";
		string strCmd;
		strCmd = "cp -f " + strPath + "/" + strFileName + " " + strWorkPath + "/" + strScriptProcName;
		//system(strCmd.c_str());

		//compile script
		string compiler	= "/usr/bin/gcc";
		string incpath	= "-I/tmp/exicon/include";
		string libpath	= "-L/tmp/exicon/lib";
		string lib		= "-ltbase -ltnet";

		stringstream ss;
		ss << strWorkPath << "test_script_" << iPort+1;
		string target = ss.str();
		unlink(target.c_str());

		strCmd = compiler + " " + incpath + " " + strWorkPath + strScriptProcName + " -o " + target + " " + libpath + " " + lib + " 2>&1";

		FILE *file;
		file = popen(strCmd.c_str(), "r");
		if(file == NULL)
		{
			printf(">> compile fail!!\n");
		}
		else
		{
			char buf[256];
			while(fgets(buf, sizeof(buf), file))
			{
				//compile error msg
				printf(">> buf = %s\n", buf);
			}

			pclose(file);

			//cout << "this is system!! - " << endl;
			//system(strCmd.c_str());




			//system(target.c_str());
			//system(strCmd.c_str());
			//start test
			//pthread_create(&idThread, NULL, TestThread, (void *)0);
			//pthread_detach(idThread);
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

int CTestMng::StopTest()
{
	if(idThread != 0)
	{
		idThread = 0;
	}

	return 0;
}

int CTestMng::IsTestOn()
{
	if(idThread == 0)
		return 0;
	else
		return 1;
}

int CTestMng::CheckScriptExt(string strFileName)
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

	if(strExt.compare("c") == 0)
		return 0;
	else if(strExt.compare("sct") == 0)
		return pos+1;

	return -2;
}




/*
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
*/

