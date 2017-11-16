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

class CTestMng
{
public:
	CTestMng(int iCell, int iPort);
	virtual ~CTestMng();

public:
    int             StartTest(int iMsgVer, string strPath, string strFileName, string strArg);
    int				StopTest(int iMsgVer);
    int				IsTestOn(int iMsgVer);
	int				CheckScriptExt(string strFileName, string strCheckExt);

    int             iVersion;
    int				iCell;
    int				iPort;

    string			strRunFile[MSGVER_CNT];
    string			strRunArg[MSGVER_CNT];
    pid_t			pidTestProcess[MSGVER_CNT];
    int				iChildStatus[MSGVER_CNT];
    pthread_t		idThread[MSGVER_CNT];
    pthread_mutex_t syncMutex;
    pthread_cond_t  syncCond;
};

#endif /* TESTMNG_H_ */
