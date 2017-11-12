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

    for(int iPortIdx=0; iPortIdx<PORT_MAX; iPortIdx++)
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

/*
    string abc="time";

    Json::Value bdinfo;
    bdinfo[abc] = "2017-11-09 19:11:56 KST";
    bdinfo["mount"] = false;
    bdinfo["cpu"] = 12.3;
    bdinfo["mem"] = 30.3;
    bdinfo["stroage"] = "35.5%";


    Json::Value port1;
    port1["port"] = "1";
    port1["testmode"] = "SAS";
    port1["script"] = "script1.uts";
    port1["status"] = "running";

    Json::Value port2;
    port2["port"] = "2";
    port2["testmode"] = "SATA";
    port2["script"] = "script2.uts";
    port2["status"] = "stop";

    Json::Value portinfo;
    portinfo.append(port1);
    portinfo.append(port2);

    Json::Value root;
    root["bdinfo"] = bdinfo;
    root["portinfo"] = portinfo;

    Json::StyledWriter writer;
    string str = writer.write(root);
    cout << str << endl;
*/
    return 0;
}

int CStatus::CheckOs()
{
    char buf[32] = {0,};
    char cmd[128];

    //time
    GetStatusFromPipe("date \"+%Y-%m-%d %H:%m:%S %Z\" | awk '{printf \"%s %s %s\", $1, $2, $3}'", buf, sizeof(buf));
    sprintf(statOs.sTime, "%s", buf);
    cout << "time = " << statOs.sTime << endl;

    //cpu
    GetStatusFromPipe("top -n 1 | grep -i \"cpu(s)\" | awk '{print $5}' | tr -d \"%id,\" | awk '{printf \"%d%\", 100-$1}'", buf, sizeof(buf));
    sprintf(statOs.sCpuUsage, "%s", buf);
    cout << "cpu = " << statOs.sCpuUsage << endl;

    //mem
    GetStatusFromPipe("free | grep Mem | awk '{printf \"%d%\", $3/$2*100}'", buf, sizeof(buf));
    sprintf(statOs.sMemUsage, "%s", buf);
    cout << "mem = " << statOs.sMemUsage << endl;

    //disk
    GetStatusFromPipe("df -h | grep [/]$ | awk '{printf \"%s\", $5}'", buf, sizeof(buf));
    sprintf(statOs.sDiskUsage, "%s", buf);
    cout << "disk = " << statOs.sDiskUsage << endl;

    //mount
    sprintf(cmd, "mountpoint -q %s; echo $?", SYS_ATH_PATH);
    GetStatusFromPipe((const char*)cmd, buf, sizeof(buf));

    if(strcmp(buf, "0") == 0)
    {
        sprintf(statOs.sMount, "ok");
    }
    else
    {
        sprintf(statOs.sMount, "none");
    }

    cout << "mount = " << statOs.sMount << endl;

    //bd connect
    sprintf(cmd, "%s/bd_connect.txt", SYS_DATA_PATH);
    GetStatusFromFile((const char*)cmd, buf, sizeof(buf));

    sprintf(statOs.sBdConnect, "%s", buf);
    cout << "bd connect = " << statOs.sBdConnect << endl;

    return 0;
}

int CStatus::CheckDps(int port)
{
    //get status from share memory where dps check script save status
    LockSem(idDpsShmemLock);
    GetShmem(idDpsShmem, &statDps[port], sizeof(statDps[port]));
    UnlockSem(idDpsShmemLock);

    cout << "<PORT" << port+1 << ">" << endl;
    cout << "DPS5V set/get voltage = " << statDps[port].fDpsSetVoltage[DPS_CH1] << " / " << statDps[port].fDpsGetVoltage[DPS_CH1] << endl;
    cout << "DPS12V set/get voltage = " << statDps[port].fDpsSetVoltage[DPS_CH2] << " / " << statDps[port].fDpsGetVoltage[DPS_CH2] << endl;
    cout << "DPS power = " << statDps[port].bDpsPower << endl;
    cout << "DPS OCP = " << statDps[port].bDpsOcp << endl;
    cout << "DPS OVP = " << statDps[port].bDpsOvp << endl << endl;

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
