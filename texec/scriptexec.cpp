#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <vector>
#include <iterator>
#include <sstream>
#include "def.h"
#include "base.h"
#include "socketclient.h"

int CheckArg(int argc, char **argv, stringstream &ssArg);
int CheckScriptExt(string strFileName, string strCheckExt);
int ProcExec(int iMode, string strArg);
int DoModeRun(stringstream &ssArg);
int DoModeList();
int DoModeUpdate();

char szProgName[] = "texec";
char szProgVersion[] = "0.0.2";

using namespace std;
enum mode{mode_none=0, mode_run, mode_stop, mode_list, mode_update};

void help()
{
    printf("%s %s\n", szProgName, szProgVersion);
    printf("usage: %s -r [SCRIPT_NAME] [FLAG1] [FLAG2] ...\n", szProgName);
//    printf("          -s [JOB_NUM]\n");
//    printf("          -l\n");
    printf("\n");
    printf(" -r: run script\n");
//    printf(" -s: stop specific job\n");
    printf(" -l: list script\n");
    printf(" -u: update script\n");
    printf("\n");
}


int main(int argc, char **argv)
{
    if(argc <= 1)
    {
        help();
        return -1;
    }

    stringstream ssArg;
    int iExecMode;

    iExecMode = CheckArg(argc, argv, ssArg);

    if(iExecMode == mode_run)
    {
        DoModeRun(ssArg);
    }
    else if(iExecMode == mode_list)
    {
        DoModeList();
    }
    else if(iExecMode == mode_update)
    {
        DoModeUpdate();
    }
    else
    {
        help();
        return -1;
    }

    return 0;
}

int CheckScriptExt(string strFileName, string strCheckExt)
{
    string strExt;
    string strName;

    size_t pos = strFileName.find(".", 0);
    if(pos == 0)
        return -1;

    strName = strFileName.substr(0, pos);
    strExt = strFileName.substr(pos+1, -1);

    if(strExt.compare(strCheckExt) == 0)
        return pos+1;

    return -2;
}

int CheckArg(int argc, char **argv, stringstream &ssArg)
{
    int iMode = mode_none;
    string strMode = argv[1];

    if(strMode == "-r")
    {
        //texec -r script 1 2 3
        if(argc <= 2)
        {
            return mode_none;
        }

        string strScriptName = argv[2];

        if(!IsFileExist(strScriptName.c_str()))
        {
            stringstream ssExecScriptName;
            ssExecScriptName << SYS_EXEC_PATH << "/" << strScriptName;
            strScriptName = ssExecScriptName.str();

            if(!IsFileExist(strScriptName.c_str()))
            {
                printf("%s is not exist\n", strScriptName.c_str());
                return -1;
            }
        }

        printf("%s is exist\n", strScriptName.c_str());

        char szAbsPath[PATH_MAX+1];
        char *ret = realpath(strScriptName.c_str(), szAbsPath);

        if(ret == NULL)
        {
            printf("fail to get absolute file name\n");
            return -1;
        }

        ssArg << szAbsPath;

        if(argc>3)
        {
            ssArg << " ";

            for(int i=3; i<argc; i++)
            {
                ssArg << argv[i];
                if(i+1 < argc)
                    ssArg << " ";
            }
        }

        iMode = mode_run;
    }
    else if(strMode == "-s")
    {
        //texec -s 1 2 3
        if(argc <= 2)
        {
            //show running process
            iMode = mode_list;
        }
        else
        {
            for(int i=2; i<argc; i++)
            {
                ssArg << argv[i];
                if(i+1 < argc)
                    ssArg << " ";
            }

            iMode = mode_stop;
        }
    }
    else if(strMode == "-l")
    {
        //texec -l
        iMode = mode_list;
    }
    else if(strMode == "-u")
    {
        iMode = mode_update;
    }
    else
    {
        iMode = mode_none;
    }

    return iMode;
}

int ProcExec(int iMode, string strArg)
{
    int idTpc;
    char sTpc[32] = {0,};

    if(ReadFileText(SYS_DATA_PATH"/tpc_id.txt", sTpc, 0, sizeof(sTpc)) > 0)
    {
        idTpc = atoi(sTpc);
    }
    else
    {
        idTpc = 200;
    }

    CSocketClient *g_pSocketClient  = new CSocketClient(idTpc, (char *)"127.0.0.1", PORT_TDM);

    if(g_pSocketClient->CreateSocket() < 0)
    {
        printf("CreateSocket() Error\n");
        g_pSocketClient->CloseSocket();
        delete g_pSocketClient;

        return -1;
    }

    if(g_pSocketClient->ConnectServer() < 0)
    {
        printf( "ConnectServer() Error!!! errno=%d\n", errno );
        g_pSocketClient->CloseSocket();
        delete g_pSocketClient;

        return -2;
    }

    if(g_pSocketClient->SendCheckDummy() < 0)
    {
        printf( "SendCheckDummy() Error!!!\n" );
        g_pSocketClient->CloseSocket();
        delete g_pSocketClient;

        return -3;
    }

    stringstream ssSocketPacket;
    ssSocketPacket << SOCKET_START_MARK;
    ssSocketPacket << MSGVER_NONE << ",";
    ssSocketPacket << idTpc << ",";
    ssSocketPacket << 0 << ",";
    ssSocketPacket << iMode << ",";
    ssSocketPacket << 0 << ",";
    ssSocketPacket << 0 << ",";
    ssSocketPacket << strArg << SOCKET_END_MARK;

    int iSocketPacketLen = ssSocketPacket.str().length();
    int iSendCnt = g_pSocketClient->Send((char*)ssSocketPacket.str().c_str(), iSocketPacketLen);

    if(iSendCnt < iSocketPacketLen)
    {
        printf("send error!!\n");
    }

    g_pSocketClient->CloseSocket();
    delete g_pSocketClient;

    return 0;
}

int DoModeRun(stringstream &ssArg)
{

    if(ssArg.str().empty())
        return -1;

    vector<string> vectArg;
    string strArg;

    while(ssArg >> strArg)
    {
        vectArg.push_back(strArg);
    }

    string strScriptAbsName;
    string strScriptArg;

    for(size_t i=0; i<vectArg.size(); i++)
    {
        if(i==0)
        {
            strScriptAbsName = vectArg.at(i);
        }
        else
        {
            strScriptArg.append(vectArg.at(i));
            strScriptArg.append(" ");
        }
    }

    size_t posSplit = strScriptAbsName.find_last_of("/");
    string strScriptPath = strScriptAbsName.substr(0, posSplit);
    string strScriptName = strScriptAbsName.substr(posSplit+1);

    //check script file extension
    stringstream ss;
    string strScriptProcFile;
    string strScriptOnlyName;

    int iExtPos;

    if((iExtPos = CheckScriptExt(strScriptName.c_str(), TEST_SCRIPT_ORI_EXT)) > 0)
    {
        strScriptOnlyName = strScriptName.substr(0, iExtPos-1);

        ss << SYS_WORK_PATH <<  "/" << strScriptOnlyName << "." << TEST_SCRIPT_RUN_EXT;
        strScriptProcFile = ss.str();
    }
    else if((iExtPos = CheckScriptExt(strScriptName.c_str(), TEST_SCRIPT_RUN_EXT)) > 0)
    {
        strScriptOnlyName = strScriptName.substr(0, iExtPos-1);

        ss << SYS_WORK_PATH <<  "/" << strScriptName;
        strScriptProcFile = ss.str();
    }
    else
    {
        printf(">> '%s, %s' file is not eixst\n", TEST_SCRIPT_ORI_EXT, TEST_SCRIPT_RUN_EXT);
        return -2;
    }

    //copy script to work folder
    ss.str("");
    ss << "cp -f " << strScriptAbsName << " " << strScriptProcFile;
    string strCmd = ss.str();
    int ret = system(strCmd.c_str());

    //compile script
    ss.str("");
    ss << SYS_WORK_PATH << "/exec/" << strScriptOnlyName << " ";
    string strRunFile = ss.str();
    unlink(strRunFile.c_str());

    ss.str("");
    ss << SYS_LOG_PATH << "/" << "compile.txt";
    string strCompileLog;
    strCompileLog = ss.str();

    ss.str("");
    ss << COMPILE_PROG << " "
       << COMPILE_INCPATH << " "
       << strScriptProcFile << " "
       << " -include sctbasic.h "
       << "-o " << strRunFile << " "
       << COMPILE_LIBPATH << " "
       << COMPILE_LIB << " "
       << "2> " << strCompileLog;
    strCmd = ss.str();

    ret = system(strCmd.c_str());
    if(ret != 0)
    {
        printf(">> compile error is happened!!\n");
        char buf[256];
        sprintf(buf, "cat %s > /dev/stdout", strCompileLog.c_str());
        system(buf);
        return -3;
    }

    //delete script file
    unlink(strScriptProcFile.c_str());

    //run script
    string strRunCmd;
    strRunCmd.append(strRunFile);
    strRunCmd.append(" ");
    strRunCmd.append(strScriptArg);
    system(strRunCmd.c_str());

    return 0;
}

int DoModeList()
{
    printf("[exec file list]\n");

    char cmd[256];
    sprintf(cmd, "ls %s", SYS_EXEC_PATH);
    system(cmd);

    return 0;
}

int DoModeUpdate()
{
    printf("[update exec file]\n");

    char cmd[256];
    sprintf(cmd, "cp -f %s/*.%s %s", SYS_SHA_EXEC_PATH, TEST_SCRIPT_RUN_EXT, SYS_EXEC_PATH);
    system(cmd);

    sprintf(cmd, "ls %s", SYS_EXEC_PATH);
    system(cmd);

    return 0;
}
