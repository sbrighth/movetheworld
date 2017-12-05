/*
 * status.h
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#ifndef STATUS_H_
#define STATUS_H_

#include <string>
#include "def.h"
#include "testmng.h"
#include "pthread.h"

using namespace std;

class CStatus {
public:
    CStatus(CTestMng *mng[]);
	virtual ~CStatus();

    int CheckOs();
    int CheckTest(int port);
    int GetStatusFromPipe(const char *szCmd, char *sBuf, int iBufSize);
    int GetStatusFromFile(const char *szCmd, char *sBuf, int iBufSize);

    void StartThread();
    void StopThread();
    int SetMonitorDuration(int iSec);

    void StartDpsThread();
    void StopDpsThread();
    int SetUpdateDpsDuration(int iMs);

public:
    OsStatus    statOs;
    TestStatus  statTest[PORT_CNT];
    DpsStatus   statDps[PORT_CNT];
    PerfStatus  statPerf[PORT_CNT];

    CTestMng *pTestMng[PORT_CNT];
    string  strJson;

    pthread_mutex_t mutexDpsSync;
    pthread_t idMonitorThread;
    pthread_t idDpsThread;
    int condMonitorThread;
    int condDpsThread;

    int iMonitorDurationSec;
    int iUpdateDpsDurationMs;
};

#endif /* STATUS_H_ */
