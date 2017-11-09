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

typedef struct tOsStatus
{
    char cTime[32];
    char cCpuUsage[8];
    char cMemUsage[8];
    char cDiskUsage[8];
    bool bMount;
    bool bMcuConnect;
}OsStatus;

typedef struct tPortStatus
{
    float fDps5V[2];    //voltage, current
    float fDps12V[2];
    char cDpsStatus[8]; //on, off, ocp, ovp
    bool bDutExist;
    bool bDutType;
}PortStatus;

typedef struct tPerfStatus
{
    char cWrite[32];
    char cRead[32];
}PerfStatus;

class CStatus {
public:
	CStatus();
	virtual ~CStatus();

    int CheckStatus();
public:
    OsStatus    statOs;
    PortStatus  statPort[MAX_PORT];
    PerfStatus  statPerf[MAX_PORT];

    string  strJson;
};

#endif /* STATUS_H_ */
