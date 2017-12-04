/*
 * status.cpp
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "status.h"
#include "iostream"
#include "base.h"

CStatus::CStatus(CTestMng *mng[]) {
	// TODO Auto-generated constructor stub
    memset(&statOs, 0, sizeof(statOs));

    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        memset(&statDps[iPortIdx], 0, sizeof(statDps[iPortIdx]));
        memset(&statPerf[iPortIdx], 0, sizeof(statPerf[iPortIdx]));

        if(mng[iPortIdx] != NULL)
            pTestMng[iPortIdx] = mng[iPortIdx];
        else
            pTestMng[iPortIdx] = NULL;
    }

    idDpsShmem = CreateShmem(KEY_DPS_SHARE, sizeof(statDps));
    idDpsShmemLock = CreateSem(KEY_DPS_SHARE_LOCK);
}

CStatus::~CStatus() {
	// TODO Auto-generated destructor stub
}

int CStatus::CheckAll()
{
    CheckOs();
    CheckDps();

    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        CheckTest(iPortIdx);
    }

    return 0;
}

int CStatus::CheckOs()
{
    char buf[32] = {0,};
    char cmd[128];

    //time
    GetStatusFromPipe("date \"+%Y-%m-%d %H:%m:%S %Z\" | awk '{printf \"%s %s %s\", $1, $2, $3}'", buf, sizeof(buf));
    sprintf(statOs.sTime, "%s", buf);

    //cpu
    GetStatusFromPipe("top -n 1 | grep -i \"cpu(s)\" | awk '{print $8}' | tr -d \"%id,\" | awk '{printf \"%d%\", 100-$1}'", buf, sizeof(buf));
    sprintf(statOs.sCpuUsage, "%s", buf);

    //mem
    GetStatusFromPipe("free | grep Mem | awk '{printf \"%d%\", $3/$2*100}'", buf, sizeof(buf));
    sprintf(statOs.sMemUsage, "%s", buf);

    //disk
    GetStatusFromPipe("df -h | grep [/]$ | awk '{printf \"%s\", $5}'", buf, sizeof(buf));
    sprintf(statOs.sDiskUsage, "%s", buf);

    //mount
    sprintf(cmd, "mountpoint -q %s; echo $?", SYS_SHA_PATH);
    GetStatusFromPipe((const char*)cmd, buf, sizeof(buf));

    if(strcmp(buf, "0") == 0)
    {
        SET_BIT(statOs.iBitStatus, BIT_MOUNT);
    }
    else
    {
        CLR_BIT(statOs.iBitStatus, BIT_MOUNT);
    }

    //bd connect
    sprintf(cmd, "%s/bd_connect.txt", SYS_DATA_PATH);
    GetStatusFromFile((const char*)cmd, buf, sizeof(buf));
    if(strcmp(buf, "1") == 0)
    {
        SET_BIT(statOs.iBitStatus, BIT_BDCONNECT);
    }
    else
    {
        CLR_BIT(statOs.iBitStatus, BIT_BDCONNECT);
    }

/*
    cout << "time = " << statOs.sTime << endl;
    cout << "cpu = " << statOs.sCpuUsage << endl;
    cout << "mem = " << statOs.sMemUsage << endl;
    cout << "disk = " << statOs.sDiskUsage << endl;
    cout << "mount = " << statOs.bMount << endl;
    cout << "bd connect = " << statOs.bBdConnect << endl;
*/
    return 0;
}

int CStatus::CheckTest(int port)
{
    int tmp;

    for(int idx=MSGVER_NONE; idx<MSGVER_CNT; idx++)
    {
        if(pTestMng[port] != NULL)
        {
            tmp = pTestMng[port]->IsTestOn(idx);
            if(tmp >= 0)
                SET_BIT(statTest[port].iBitRun, idx);
            else
                CLR_BIT(statTest[port].iBitRun, idx);

            //cout << "TEST[" << idx << "] = " << tmp << endl;
        }
    }

    return 0;
}

int CStatus::CheckDps()
{
    //get status from share memory where dps check script save status
    LockSem(idDpsShmemLock);
    GetShmem(idDpsShmem, &statDps, sizeof(statDps));
    UnlockSem(idDpsShmemLock);

    cout << endl;
    cout << "------------------------------------------------" << endl;
    cout << "P1 DPS5V  get voltage = " << statDps[PORT1].dVoltage[DPS_CH1] << " / " << statDps[PORT1].dVoltage[DPS_CH1] << endl;
    cout << "P2 DPS12V get voltage = " << statDps[PORT2].dVoltage[DPS_CH2] << " / " << statDps[PORT2].dVoltage[DPS_CH2] << endl;
    cout << "------------------------------------------------" << endl;

    return 0;
}

int CStatus::GetStatusFromPipe(const char *szCmd, char *sBuf, int iBufSize)
{
    FILE *file;
    memset(sBuf, 0, iBufSize);

    if(szCmd == NULL || sBuf == NULL)
        return -1;

    file = popen(szCmd, "r");
    if(file == NULL)
        return -2;

    fread(sBuf, iBufSize, 1, file);
    pclose(file);

    int size = strlen(sBuf);
    if(sBuf[size-1]=='\n' && size>0)
        sBuf[size-1] = '\0';

    return 0;
}

int CStatus::GetStatusFromFile(const char *szCmd, char *sBuf, int iBufSize)
{
    FILE *file;
    memset(sBuf, 0, iBufSize);

    if(szCmd == NULL || sBuf == NULL)
        return -1;

    file = fopen(szCmd, "r");
    if(file == NULL)
        return -2;

    fread(sBuf, iBufSize, 1, file);
    fclose(file);

    int size = strlen(sBuf);
    if(sBuf[size-1]=='\n' && size>0)
        sBuf[size-1] = '\0';

    return 0;
}
