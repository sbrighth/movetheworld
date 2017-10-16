/*
 * testmng.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TESTMNG_H_
#define TESTMNG_H_

#include <pthread.h>
#include <iostream>
#include "def.h"

using namespace std;


#define TEST_SCRIPT_ORI_EXT		"sct"
#define	TEST_SCRIPT_RUN_EXT		"c"

class CTestMng
{
public:
	CTestMng(int iCell, int iPort);
	virtual ~CTestMng();

public:
	int				StartTest(string strPath, string strFileName);
	int				StopTest();
	int				IsTestOn();
	int				CheckScriptExt(string strFileName);

	string			strRunFile;
	string			strWorkPath;
	int				iCell;
	int				iPort;
	pid_t			pidTestProcess;
	int				iChildStatus;
	int				condThread;
	pthread_t		idThread;
	pthread_cond_t	condSync;
	pthread_mutex_t	mutexSync;
};

#endif /* TESTMNG_H_ */
