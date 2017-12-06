/*
 * status.cpp
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "status.h"
#include "iostream"
#include "base.h"
#include "DuoBdLib.h"
#include "statsocket.h"

#define DEF_MON_DURATION_SEC    1
#define MIN_MON_DURATION_SEC    1
#define MAX_MON_DURATION_SEC    3600

#define DEF_UPDPS_DURATION_MS   500
#define MIN_UPDPS_DURATION_MS   500
#define MAX_UPDPS_DURATION_MS   900

extern CStatSocket *g_pStatSocket;

CStatus::CStatus(CTestMng *mng[])
{
	// TODO Auto-generated constructor stub
    memset(&statOs, 0, sizeof(statOs));
    memset(&statDps, 0, sizeof(statDps));
    memset(&statPerf, 0, sizeof(statPerf));

    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        if(mng[iPortIdx] != NULL)
            pTestMng[iPortIdx] = mng[iPortIdx];
        else
            pTestMng[iPortIdx] = NULL;
    }

    iMonitorDurationSec = DEF_MON_DURATION_SEC;
    iUpdateDpsDurationMs = DEF_UPDPS_DURATION_MS;

    pthread_mutex_init(&mutexDpsSync, NULL);
}

CStatus::~CStatus() {
	// TODO Auto-generated destructor stub
    pthread_mutex_destroy(&mutexDpsSync);
}

int CStatus::CheckOs()
{
    char buf[32] = {0,};
    char cmd[128];

    //time
    GetStatusFromPipe("date \"+%Y-%m-%d %H:%m:%S %Z\" | awk '{printf \"%s %s %s\", $1, $2, $3}'", buf, sizeof(buf));
    sprintf(statOs.sTime, "%s", buf);

    //cpu
    GetStatusFromPipe("sensors | grep \"Physical id\" | awk '{print$4}'", buf, sizeof(buf));
    sprintf(statOs.sCpuUsage, "%s", buf);

    //cpu temp
    GetStatusFromPipe("top -n 1 | grep -i \"cpu(s)\" | awk '{print $8}' | tr -d \"%id,\" | awk '{printf \"%d%\", 100-$1}'", buf, sizeof(buf));
    sprintf(statOs.sCpuTemp, "%s", buf);

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
    GetStatusFromFile(FILE_BD_CONNECT, buf, sizeof(buf));
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

void *MonitorThread(void *arg)
{
    CStatus *pthis = (CStatus *)arg;

    pthis->condMonitorThread = 1;
    while(pthis->condMonitorThread == 1)
    {
        pthis->CheckOs();

        for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
        {
            pthis->CheckTest(iPortIdx);
        }

        pthread_mutex_lock(&pthis->mutexDpsSync);

        //socket
        StatData statData;
        statData.statOS = pthis->statOs;
        memcpy(statData.statTest, pthis->statTest, sizeof(pthis->statTest));
        memcpy(statData.statDps, pthis->statDps, sizeof(pthis->statDps));
        g_pStatSocket->SendData(statData);

        pthread_mutex_unlock(&pthis->mutexDpsSync);

        sleep(pthis->iMonitorDurationSec);
    }

    pthread_exit((void *)0);
}

void *DpsThread(void *arg)
{
    CStatus     *pthis = (CStatus *)arg;
    DpsStatus   statDpsPort;
    bool        bConnectDps[PORT_CNT];
    memset(bConnectDps, true, sizeof(bConnectDps));

    OpenPort(BOTH_PORT, MCU_IF_BAUD_DEF);

    pthis->condDpsThread = 1;
    while(pthis->condDpsThread == 1)
    {
        int iConnectCnt = 0;

        for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
        {
            if(bConnectDps[iPortIdx] == false)
                continue;

            if(IsConnected(iPortIdx) == false)
            {
                bConnectDps[iPortIdx] = false;

                pthread_mutex_lock(&pthis->mutexDpsSync);
                pthis->statDps[iPortIdx].iConnect = 0;
                pthread_mutex_unlock(&pthis->mutexDpsSync);

                continue;
            }
            else
            {
                iConnectCnt++;
            }

            memset(&statDpsPort, 0, sizeof(DpsStatus));

            for(int iChIdx=0; iChIdx<DPS_CH_CNT; iChIdx++)
            {
                GetVolt(iPortIdx, iChIdx, &statDpsPort.dVoltage[iChIdx]);
                GetCurrent(iPortIdx, iChIdx, &statDpsPort.dCurrent[iChIdx]);
            }

            pthread_mutex_lock(&pthis->mutexDpsSync);
            pthis->statDps[iPortIdx].iConnect = 1;
            pthis->statDps[iPortIdx] = statDpsPort;
            pthread_mutex_unlock(&pthis->mutexDpsSync);
        }

        if(iConnectCnt == 0)
        {
            pthis->condDpsThread = 0;
            break;
        }

        msleep(pthis->iUpdateDpsDurationMs);
    }

    pthread_exit((void *)0);
}

void CStatus::StartThread()
{
    StartDpsThread();

    if(idMonitorThread == 0)
        pthread_create(&idMonitorThread, NULL, MonitorThread, (void*)this);
}

void CStatus::StopThread()
{
    condMonitorThread = 0;
    if(idMonitorThread != 0)
    {
        pthread_join(idMonitorThread, NULL);
        idMonitorThread = 0;
    }

    memset(&statOs, 0, sizeof(statOs));
    memset(&statPerf, 0, sizeof(statPerf));

    StopDpsThread();
}

int CStatus::SetMonitorDuration(int iSec)
{
    if(iSec < MIN_MON_DURATION_SEC || iSec > MAX_MON_DURATION_SEC)
        return -1;

    iMonitorDurationSec = iSec;
    return 0;
}

void CStatus::StartDpsThread()
{
    if(idDpsThread == 0)
        pthread_create(&idDpsThread, NULL, DpsThread, (void*)this);
}

void CStatus::StopDpsThread()
{
    condDpsThread = 0;
    if(idDpsThread!= 0)
    {
        pthread_join(idDpsThread, NULL);
        idDpsThread = 0;
    }

    pthread_mutex_lock(&mutexDpsSync);
    memset(&statDps, 0, sizeof(statDps));
    pthread_mutex_unlock(&mutexDpsSync);
}

int CStatus::SetUpdateDpsDuration(int iMs)
{
    if(iMs < MIN_UPDPS_DURATION_MS || iMs > MAX_UPDPS_DURATION_MS)
        return -1;

    iUpdateDpsDurationMs = iMs;
    return 0;
}
