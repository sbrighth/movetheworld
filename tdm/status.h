/*
 * status.h
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "def.h"
#include "string"
#include "testmng.h"

using namespace std;

class CStatus {
public:
    CStatus(CTestMng *mng[]);
	virtual ~CStatus();

    int CheckAll();
    int CheckOs();
    int CheckTest(int port);
    int CheckDps(int port);
    int GetStatusFromPipe(const char *szCmd, char *sBuf, int iBufSize);
    int GetStatusFromFile(const char *szCmd, char *sBuf, int iBufSize);

public:
    OsStatus    statOs;
    TestStatus  statTest[PORT_CNT];
    DpsStatus   statDps[PORT_CNT];
    PerfStatus  statPerf[PORT_CNT];

    CTestMng *pTestMng[PORT_CNT];
    string  strJson;
    int idDpsShmem;
    int idDpsShmemLock;
};

#endif /* STATUS_H_ */
