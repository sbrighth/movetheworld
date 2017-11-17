/*
 * status.cpp
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include "status.h"
#include "iostream"
#include "base.h"

CStatus::CStatus() {
	// TODO Auto-generated constructor stub
    memset(&statOs, 0, sizeof(statOs));

    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        memset(&statDps[iPortIdx], 0, sizeof(statDps[iPortIdx]));
        memset(&statPerf[iPortIdx], 0, sizeof(statPerf[iPortIdx]));
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

    for(int iPortIdx=PORT_MIN; iPortIdx<PORT_MAX; iPortIdx++)
    {
        CheckDps(iPortIdx);
    }

    MakeJsonString();

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
        sprintf(statOs.sMount, "ok");
    }
    else
    {
        sprintf(statOs.sMount, "none");
    }

    //bd connect
    sprintf(cmd, "%s/bd_connect.txt", SYS_DATA_PATH);
    GetStatusFromFile((const char*)cmd, buf, sizeof(buf));
    sprintf(statOs.sBdConnect, "%s", buf);
/*
    cout << "time = " << statOs.sTime << endl;
    cout << "cpu = " << statOs.sCpuUsage << endl;
    cout << "mem = " << statOs.sMemUsage << endl;
    cout << "disk = " << statOs.sDiskUsage << endl;
    cout << "mount = " << statOs.sMount << endl;
    cout << "bd connect = " << statOs.sBdConnect << endl;
*/
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

    return 0;
}

int CStatus::MakeJsonString()
{
    Json::Value jsonOs;
    jsonOs["time"] = statOs.sTime;
    jsonOs["cpu"] = statOs.sCpuUsage;
    jsonOs["mem"] = statOs.sMemUsage;
    jsonOs["disk"] = statOs.sDiskUsage;
    jsonOs["mount"] = statOs.sMount;
    jsonOs["bdcon"] = statOs.sBdConnect;
/*
    port1["port"] = "1";
    port1["testmode"] = "SAS";
    port1["script"] = "script1.uts";
    port1["status"] = "running";
*/

    Json::Value jsonDpsAll;

    for(int iPortIdx=0; iPortIdx<PORT_CNT; iPortIdx++)
    {
        Json::Value jsonDpsSetV;
        Json::Value jsonDpsGetV;
        Json::Value jsonDpsVoltage;
        jsonDpsSetV["set"] = statDps[iPortIdx].sDpsSetVoltage;
        jsonDpsGetV["get"] = statDps[iPortIdx].sDpsGetVoltage;
        jsonDpsVoltage.append(jsonDpsSetV);
        jsonDpsVoltage.append(jsonDpsGetV);

        Json::Value jsonDpsSetA;
        Json::Value jsonDpsGetA;
        Json::Value jsonDpsCurrent;
        jsonDpsSetA["set"] = statDps[iPortIdx].sDpsSetCurrent;
        jsonDpsGetA["get"] = statDps[iPortIdx].sDpsGetCurrent;
        jsonDpsCurrent.append(jsonDpsSetA);
        jsonDpsCurrent.append(jsonDpsGetA);

        Json::Value jsonDps;
        jsonDps["port"] = iPortIdx+1;
        jsonDps["voltage"] = jsonDpsVoltage;
        jsonDps["current"]= jsonDpsCurrent;
        jsonDps["power"] = statDps[iPortIdx].bDpsPower;
        jsonDps["ocp"] = statDps[iPortIdx].bDpsOcp;
        jsonDps["ovp"] = statDps[iPortIdx].bDpsOvp;

        jsonDpsAll.append(jsonDps);
    }

    Json::Value root;
    root["os"] = jsonOs;
    root["dps"] = jsonDpsAll;

    Json::StyledWriter writer;
    string str = writer.write(root);
   // cout << str << endl;

    return 0;
}
