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



#define PROG_VERSION    "0.1"
#define PROG_NAME       "texec"

using namespace std;
enum mode{mode_none=0, mode_run, mode_stop, mode_list};

void help()
{
    printf("%s %s\n", PROG_NAME, PROG_VERSION);
    printf("usage: %s -r [SCRIPT_NAME] [FLAG1] [FLAG2] ...\n", PROG_NAME);
    printf("          -s [JOB_NUM]\n");
    printf("          -l\n");
    printf("\n");
    printf(" -r: run script\n");
    printf(" -s: stop specific job\n");
    printf(" -l: list job\n");
    printf("\n");
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
    else
    {
        iMode = mode_none;
    }

    return iMode;
}

int ProcExec(int iMode, string strArg)
{
    int idTpc = 2;
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

    cout << "socket packet = " << ssSocketPacket.str() << endl;

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

    if(iExecMode < 0)
    {
        return -1;
    }
    else if(iExecMode == mode_none)
    {
        help();
        return -1;
    }

    if(ProcExec(iExecMode, ssArg.str()) < 0)
    {
        printf("check tdm is running!\n");
    }

    return 0;
}
