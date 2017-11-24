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
#include <vector>
#include "def.h"
#include "log.h"

using namespace std;

class CTestMng
{
public:
    CTestMng(int iCell, int iPort);
	virtual ~CTestMng();

public:
    int             StartTest(int iMsgVer, string strProcFile, string strRunFile, string strArg);
    int				StopTest(int iMsgVer);
    int				IsTestOn(int iMsgVer);
	int				CheckScriptExt(string strFileName, string strCheckExt);
    int             StrToArgv(string strArgs, vector<char*> &argv);

    int             iVersion;
    int				iCell;
    int				iPort;

    string			strRunProg[MSGVER_CNT];
    string			strRunArg[MSGVER_CNT];
    vector<char*>   vectArgv[MSGVER_CNT];
    pid_t			pidTestProcess[MSGVER_CNT];
    int				iChildStatus[MSGVER_CNT];
    pthread_t		idThread[MSGVER_CNT];
    pthread_mutex_t mutexSync;
    pthread_cond_t  condSync;

    //Log Send
    CLog            mngLog;
};

#endif /* TESTMNG_H_ */
