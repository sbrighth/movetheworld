#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include "callback.h"
#include "base.h"
#include "msgsend.h"
#include "testmng.h"

using namespace std;

static int ProcMsg(MsgHdr hdr, char *msg_str);
static int ProcSock(MsgHdr hdr, char *msg_str);
static int CheckScriptFile(char *msg_str);

extern int g_idMsgq;
extern int g_idTpc;
extern char *g_szTestPath;
extern CTestMng **g_ppTestMng;
extern char *szProgVersion;

//msg file process callback
void ProcRecvMsg(MsgPack msg)
{
    int cell = msg.hdr.cell;

    if(cell != g_idTpc)
    {
        printf(">> invalid tpc no(c:%d, g_idTpc:%d)\n", cell, g_idTpc);
        return;
    }

    ProcMsg(msg.hdr, msg.string);
}

//msg sock process callback
void ProcRecvSock(SockPack sockData)
{
    int cell    	= sockData.hdr.cell;

    if(cell != g_idTpc)
    {
        printf(">> invalid tpc no(c:%d, g_idTpc:%d)\n", cell, g_idTpc);
        return;
    }

    ProcSock(sockData.hdr, sockData.pstring);
}

static int ProcMsg(MsgHdr hdr, char *msg_str)
{
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port-1;     //1: port1, 2: port2
    int msg_no	= hdr.msg_no;
    //int packet 		= msg.packet;
    //int flag 		= msg.flag;

    if(port < PORT_MIN || port > PORT_MAX)
        return -1;

    if(msg_str == NULL)
        return -2;

    if(msg_no == MSG_TEST_START)
    {
        if( g_ppTestMng[port]->IsTestOn(version) == 0 )
        {
            char szRealName[PATHNAME_SIZE] = {0,};

            if( SearchFile(g_szTestPath, msg_str, szRealName) == 0 )
            {
                if(g_ppTestMng[port]->StartTest(version, g_szTestPath, szRealName, "") == 0)
                {
                    printf(">> test is started!!\n");
                    SendMsg(g_idMsgq, version, cell, port+1, MSG_TEST,	0, "");
                    SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,	0, "TEST");
                }
            }
            else
            {
                printf(">> port%d script is not exist!!\n", port);
                SendMsg(g_idMsgq, version, cell, port+1, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,	0, "SCRIPT NOT EXIST");
            }
        }
        else
        {
            printf(">> port%d is testing!!\n", port+1);
        }
    }
    else if(msg_no == MSG_TEST_STOP)
    {
        if(g_ppTestMng[port]->StopTest(version) == 0)
        {
            SendMsg(g_idMsgq, version, cell, port+1, MSG_FAIL,	0, "");
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,	0, "ABORT OK");
        }
        else
        {
            SendMsg(g_idMsgq, version, cell, port+1, MSG_FAIL,	0, "");
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,	0, "ABORT FAIL");
        }
    }
    else if(msg_no == MSG_INIT)
    {
        SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,    0,	"MSG_INIT");
        SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT2,    0, "");
        SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT3,    0, "");
        SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT4,    0, "");

        /*
        switch(CheckScriptFile(msg_str))
        {
        case SC_FILE_DEFAULT:
            SendMsg(g_idMsgq, version, cell, port+1, MSG_INITACK,   SC_FILE_DEFAULT, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,    0,	"MSG_INIT");
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT2,    0, "");
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT3,    0, "");
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT4,    0, "");
            break;
        case SC_FILE_PASS:
            SendMsg(g_idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_PASS, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_SIZE_ERR:
            SendMsg(g_idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_SIZE_ERR, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_NONE_ERR:
            SendMsg(g_idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_NONE_ERR, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port+1, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_OTHER_ERR:
            break;
        }
        */
    }
    else if(msg_no == MSG_INITACK)
    {
        // do nothing
    }

    return 0;
}

//MSGVER_OTHER
static int ProcSock(MsgHdr hdr, char *msg_str)
{
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port;     //0: all, 1: port1, 2: port2
    int msg_no	= hdr.msg_no;
    //int packet  = hdr.packet;
    //int flag 	= hdr.flag;

    //BD always port0
    if(version == MSGVER_BD_INFO || version == MSGVER_BD_DIAG || version == MSGVER_BD_UPDATE)
    {
        port = 0;
    }
    //1: port1, 2: port2
    else if(version == MSGVER_PORT_DPS || version == MSGVER_PORT_DIAG)
    {
        if(port < PORT_MIN+1 || port > PORT_MAX+1)
            return -1;
    }
    else
        return 0;

    if(msg_no == MSG_TEST_START)
    {
        if( g_ppTestMng[port]->IsTestOn(version) == 0 )
        {
            stringstream ssArg;
            ssArg << msg_str;

            if(ssArg.str().empty())
            {
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "STRING EMPTY");
                return -2;
            }

            vector<string> vectArg;
            string strArg;
            string strScriptPathName;
            string strScriptArg;

            while(ssArg >> strArg)
            {
                vectArg.push_back(strArg);
            }

            for(size_t i=0; i<vectArg.size(); i++)
            {
                if(i==0)
                {
                    strScriptPathName.append(SYS_PATH);
                    strScriptPathName.append("/");
                    strScriptPathName.append(vectArg.at(i));
                }
                else
                {
                    strScriptArg.append(vectArg.at(i));
                    strScriptArg.append(" ");
                }
            }

            if(!IsFileExist(strScriptPathName.c_str()))
            {
                printf(">> port%d script is not exist!!\n", port);
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "SCRIPT NOT EXIST");

                return -1;
            }
            else
            {
                size_t posSplit = strScriptPathName.find_last_of('/');
                string strPath = strScriptPathName.substr(0,posSplit);
                string strScriptName = strScriptPathName.substr(posSplit+1);

                if(g_ppTestMng[port]->StartTest(version, strPath, strScriptName, strScriptArg) == 0)
                {
                    printf(">> test is started!!\n");
                    SendMsg(g_idMsgq, version, cell, port, MSG_TEST,	0, "");
                    SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "TEST");
                }
            }
        }
        else
        {
            printf(">> port%d is testing!!\n", port);
        }
    }
    else if(msg_no == MSG_TEST_STOP)
    {
        if(g_ppTestMng[port]->IsTestOn(version) == 1)
        {
            if(g_ppTestMng[port]->StopTest(version) == 0)
            {
                SendMsg(g_idMsgq, version, cell, port, MSG_FAIL,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "ABORT OK");
            }
            else
            {
                SendMsg(g_idMsgq, version, cell, port, MSG_FAIL,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "ABORT FAIL");
            }
        }
    }

    return 0;
}

static int CheckScriptFile(char *msg_str)
{
    int iSendMode = SC_FILE_DEFAULT;
    char szRealName[PATHNAME_SIZE] = {0,};
    char szFileName[PATHNAME_SIZE] = {0,};

    if(strlen(msg_str) > 0)
    {
        int nNameSize = 15;	//0~14
        int nFileSize = 4;	//15~18

        char *szScriptName = new char[nNameSize+1];
        char *szScriptSize = new char[nFileSize+1];

        memset(szScriptName, 0, nNameSize+1);
        memset(szScriptSize, 0, nFileSize+1);

        memcpy(szScriptName, msg_str, nNameSize+1);
        memcpy(szScriptSize, msg_str+nNameSize, nFileSize+1);

        for(int i=nNameSize; i>0; i--)
        {
            if(szScriptName[i-1] != '0')
            {
                szScriptName[i] = 0;
                break;
            }
        }

        printf("scriptName= %s\n", szScriptName);
        printf("scriptSize= %s\n", szScriptSize);

        if( SearchFile(g_szTestPath, szScriptName, szRealName) == 0 )
        {
            long lScriptSize = atoi(szScriptSize);

            sprintf(szFileName, "%s/%s", g_szTestPath, szRealName);
            printf(">> szFileName = %s\n", szFileName);
            long lFileSize = GetFileSize(szFileName);
            lFileSize = lFileSize % 10000;

            printf(">> msg file size = %ld, read file size = %ld\n", lScriptSize, lFileSize);
            if( lScriptSize == lFileSize )
            {
                iSendMode = SC_FILE_PASS;
            }
            else
            {
                iSendMode = SC_FILE_SIZE_ERR;
            }
        }
        else
        {
            iSendMode = SC_FILE_NONE_ERR;
        }

        delete szScriptName;
        delete szScriptSize;
    }

    return iSendMode;
}
