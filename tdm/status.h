/*
 * status.h
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "def.h"
#include "json/json.h"
#include "string"

using namespace std;

class CStatus {
public:
	CStatus();
	virtual ~CStatus();

    int CheckAll();
    int CheckOs();
    int CheckDps(int port);
    int GetStatusFromPipe(const char *szCmd, char *sBuf, int iBufSize);
    int GetStatusFromFile(const char *szCmd, char *sBuf, int iBufSize);

public:
    OsStatus    statOs;
    DpsStatus   statDps[PORT_MAX];
    PerfStatus  statPerf[PORT_MAX];

    string  strJson;
    int idDpsShmem;
    int idDpsShmemLock;
};

#endif /* STATUS_H_ */
