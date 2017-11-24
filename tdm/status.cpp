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

    cout << "----------------------" << endl;
    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        //cout << "<PORT" << iPortIdx+1 << ">" << endl;
        CheckDps(iPortIdx);
        CheckTest(iPortIdx);
        //cout << endl;
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
        printf(">> buf=%s\n", buf);
        printf(">> mount ok!\n");
        SET_BIT(statOs.iBitStatus, bMOUNT);
    }
    else
    {
        printf(">> buf=%s\n", buf);
        printf(">> mount no!\n");
        CLR_BIT(statOs.iBitStatus, bMOUNT);
    }

    //bd connect
    sprintf(cmd, "%s/bd_connect.txt", SYS_DATA_PATH);
    GetStatusFromFile((const char*)cmd, buf, sizeof(buf));
    if(strcmp(buf, "1") == 0)
    {
        SET_BIT(statOs.iBitStatus, bBDCONNECT);
    }
    else
    {
        CLR_BIT(statOs.iBitStatus, bBDCONNECT);
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

int CStatus::CheckDps(int port)
{
    //get status from share memory where dps check script save status
    LockSem(idDpsShmemLock);
    GetShmem(idDpsShmem, &statDps[port], sizeof(statDps[port]));
    UnlockSem(idDpsShmemLock);
/*
    cout << "<PORT" << port+1 << ">" << endl;
    cout << "DPS5V set/get voltage = " << statDps[port].sDpsSetVoltage[DPS_CH1] << " / " << statDps[port].sDpsGetVoltage[DPS_CH1] << endl;
    cout << "DPS12V set/get voltage = " << statDps[port].sDpsSetVoltage[DPS_CH2] << " / " << statDps[port].sDpsGetVoltage[DPS_CH2] << endl;
    cout << "DPS power = " << statDps[port].bDpsPower << endl;
    cout << "DPS OCP = " << statDps[port].bDpsOcp << endl;
    cout << "DPS OVP = " << statDps[port].bDpsOvp << endl << endl;
*/
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
