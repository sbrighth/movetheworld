/*
 * status.cpp
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#include "status.h"
#include "iostream"
#include <stdio.h>

CStatus::CStatus() {
	// TODO Auto-generated constructor stub
}

CStatus::~CStatus() {
	// TODO Auto-generated destructor stub
}

int CStatus::CheckStatus()
{
    FILE *file;
    char cmd[512] = {0,};

    memset(statOs, 0, sizeof(tOs));
    for(int i=0; i<MAX_PORT; i++)
    {
        memset(statPort[i], 0, sizeof(statPort[i]));
        memset(statPerf[i], 0, sizeof(statPerf[i]));
    }

    //time
    sprintf(cmd, "date +%Y-%m-%d %H:%m:%S %Z");
    file = popen(cmd, "r");
    if(file == NULL)
        return -1;

    fread(statOs.cTime, sizeof(statOs.cTime), 1, file);
    pclose(file);

    //cpu
    sprintf(cmd, "top -n 1 | grep -i cpu\(s\) | awk '{print $5}' | tr -d "%id," | awk '{print 100-$1}");
    file = popen(cmd, "r");
    if(file == NULL)
        return -1;

    fread(statOs.cCpuUsage, sizeof(statOs.cCpuUsage), 1, file);
    pclose(file);

    //mem
    sprintf(cmd, "free | grep Mem | awk '{print $3/$2*100}");
    file = popen(cmd, "r");
    if(file == NULL)
        return -1;
    else
        fread(statOs.cMemUsage, sizeof(statOs.cMemUsage), 1, file);

    //disk
    sprintf(cmd, "df -h | grep [/]$ | awk '{print $5}");
    file = popen(cmd, "r");
    if(file == NULL)
        return -1;

    fread(statOs.cDiskUsage, sizeof(statOs.cDiskUsage), 1, file);
    pclose(file);

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

    return 0;
}
