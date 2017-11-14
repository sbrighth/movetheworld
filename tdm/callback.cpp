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

static int ProcMsgVerNone(MsgHdr hdr, char *msg_str);
static int ProcMsgVerOther(MsgHdr hdr, char *msg_str);
static int ProcMsgVerTest(MsgHdr hdr, char *msg_str);
static int CheckScriptFile(char *msg_str);

extern int g_idTpc;
extern char *g_szTestPath;
extern CTestMng **g_ppTestMng;
extern string strProgVersion;

//msg file process callback
void ProcRecvMsg(MsgPack msg)
{
    int cell = msg.hdr.cell;

    if(cell != g_idTpc)
    {
        printf(">> invalid tpc no(c:%d, g_idTpc:%d)\n", cell, g_idTpc);
        return;
    }

    ProcMsgVerTest(msg.hdr, msg.string);
}

//msg sock process callback
void ProcRecvSock(SockPack sockData)
{
    int version		= sockData.hdr.version;
    int cell    	= sockData.hdr.cell;

    if(cell != g_idTpc)
    {
        printf(">> invalid tpc no(c:%d, g_idTpc:%d)\n", cell, g_idTpc);
        return;
    }

    if(version == MSGVER_NONE)
    {
        printf(">> this is MsgVerNone\n");
        ProcMsgVerNone(sockData.hdr, sockData.pstring);
    }
    else if(version == MSGVER_PORT_TEST)
    {
        printf(">> this is MsgVerTest\n");
        ProcMsgVerTest(sockData.hdr, sockData.pstring);
    }
    //else if( version == MSGVER_BD_INFO || version == MSGVER_BD_DIAG || version == MSGVER_BD_UPDATE || version == MSGVER_PORT_DPS || version == MSGVER_PORT_DIAG )
    else
    {
        printf(">> this is MsgVerOther\n");
        ProcMsgVerOther(sockData.hdr, sockData.pstring);
    }
}

//MSGVER_NONE
static int ProcMsgVerNone(MsgHdr hdr, char *msg_str)
{
/*
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port-1;
    int msg_no	= hdr.msg_no;
    int packet  = hdr.packet;
    int flag 	= hdr.flag;
*/
    stringstream ssArg;
    ssArg << msg_str;

    if(ssArg.str().empty())
        return -1;

    vector<string> vectArg;
    string strArg;

    while(ssArg >> strArg)
    {
        vectArg.push_back(strArg);
    }

    string strScriptPathName = vectArg.at(0);
    size_t posSplit = strScriptPathName.find_last_of('/');

    string strPath = strScriptPathName.substr(0,posSplit);
    string strScriptName = strScriptPathName.substr(posSplit+1);

    if(g_ppTestMng[0]->StartTest(strPath, strScriptName) == 0)
    {
        printf(">> test is started!!\n");
    }

    return 0;
}

//MSGVER_OTHER
static int ProcMsgVerOther(MsgHdr hdr, char *msg_str)
{
/*
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port-1;
    int msg_no	= hdr.msg_no;
    int packet  = hdr.packet;
    int flag 	= hdr.flag;

    int idMsgq  = KEY_TEST_MSGQ;

    if(port < PORT_MIN || port > PORT_MAX)
        return -1;

    if(msg_str == NULL)
        return -2;

    char szRealName[PATHNAME_SIZE] = {0,};

    if( SearchFile(g_szTestPath, msg_str, szRealName) == 0 )
    {
        if(g_ppTestMng[port]->StartTest(g_szTestPath, szRealName) == 0)
        {
            printf(">> test is started!!\n");
            SendMsg(idMsgq, version, cell, port+1, MSG_TEST,	0, "");
        }
    }
    else
    {
        printf(">> Info script is not exist!!\n");
        SendMsg(idMsgq, version, cell, port+1, MSG_DONE,	0, "SCRIPT NOT EXIST");
    }
*/
    return 0;
}

//MSGVER_PORT_TEST
static int ProcMsgVerTest(MsgHdr hdr, char *msg_str)
{
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port-1;	// 0, 1
    int msg_no	= hdr.msg_no;
    //int packet 		= msg.packet;
    //int flag 		= msg.flag;

    int idMsgq  = KEY_TEST_MSGQ;

    if(port < PORT_MIN || port > PORT_MAX)
        return -1;

    if(msg_str == NULL)
        return -2;

    if(msg_no == MSG_TEST_START)
    {
        if( g_ppTestMng[port]->IsTestOn() == 0 )
        {
            char szRealName[PATHNAME_SIZE] = {0,};

            if( SearchFile(g_szTestPath, msg_str, szRealName) == 0 )
            {
                if(g_ppTestMng[port]->StartTest(g_szTestPath, szRealName) == 0)
                {
                    printf(">> test is started!!\n");
                    SendMsg(idMsgq, version, cell, port+1, MSG_TEST,	0, "");
                }
            }
            else
            {
                printf(">> port %d script is not exist!!\n", port+1);
                SendMsg(idMsgq, version, cell, port+1, MSG_DONE,	0, "SCRIPT NOT EXIST");
            }
        }
        else
        {
            printf(">> port %d is testing!!\n", port+1);
        }
    }
    else if(msg_no == MSG_TEST_STOP)
    {
        if(g_ppTestMng[port]->IsTestOn() == 1)
        {
            g_ppTestMng[port]->StopTest();
            SendMsg(idMsgq, version, cell, port+1, MSG_FAIL,	0, "ABORT");
        }
    }
    else if(msg_no == MSG_INIT)
    {
        switch(CheckScriptFile(msg_str))
        {
        case SC_FILE_DEFAULT:
            SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_DEFAULT, strProgVersion.c_str());
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,    0,	"MSG_INIT");
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT2,    0, "");
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT3,    0, "");
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT4,    0, "");
            break;
        case SC_FILE_PASS:
            SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_PASS, strProgVersion.c_str());
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
            break;
        case SC_FILE_SIZE_ERR:
            SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_SIZE_ERR, strProgVersion.c_str());
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
            break;
        case SC_FILE_NONE_ERR:
            SendMsg(idMsgq, version, cell, port+1, MSG_INITACK,	SC_FILE_NONE_ERR, strProgVersion.c_str());
            SendMsg(idMsgq, version, cell, port+1, MSG_TEXT1,	0, "MSG_INIT");
            break;
        case SC_FILE_OTHER_ERR:
            break;
        }
    }
    else if(msg_no == MSG_INITACK)
    {
        // do nothing
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

        memset(szScriptName, 0, sizeof(szScriptName));
        memset(szScriptSize, 0, sizeof(szScriptSize));

        memcpy(szScriptName, msg_str, sizeof(szScriptName));
        memcpy(szScriptSize, msg_str+nNameSize, sizeof(szScriptSize));

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
