#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "callback.h"
#include "base.h"
#include "msgsend.h"
#include "testmng.h"

using namespace std;

static int ProcMsg(MsgHdr hdr, char *msg_str);
static int ProcSock(MsgHdr hdr, char *msg_str);
//static int CheckScriptFile(char *path, char *msg_str);
static int CheckScriptExt(string strFileName, string strCheckExt);

extern char *g_szTesterPath;
extern char *g_szTesterPortPath[PORT_CNT];
extern char *g_szWorkExecPath;
extern char *g_szWorkPortPath[PORT_CNT];
extern int g_idMsgq;
extern int g_idTpc;
extern CTestMng *g_pTestMng[PORT_CNT];
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
    int cell = sockData.hdr.cell;

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
    int port    = hdr.port;
    int msg_no	= hdr.msg_no;
    //int packet 		= msg.packet;
    //int flag 		= msg.flag;

    int iPortIdx = port-1;

    if(iPortIdx < PORT_MIN || iPortIdx > PORT_MAX)
        return -1;

    if(msg_str == NULL)
        return -2;

    if(msg_no == MSG_TEST_START)
    {
        if( g_pTestMng[iPortIdx]->IsTestOn(version) == 0 )
        {
            char szScriptName[PATHNAME_SIZE] = {0,};
            if( SearchFile(g_szTesterPortPath[iPortIdx], msg_str, szScriptName) < 0 )
            {
                printf(">> script = %s\n", szScriptName);
                printf(">> port%d script is not exist!!\n", port);
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "SCRIPT NOT EXIST");

                return -3;
            }

            //check script file extension
            stringstream ss;
            string strCmd;
            string strScriptProcFile;
            string strScriptRunFile;
            string strScriptName(szScriptName);

            int iExtPos = CheckScriptExt(strScriptName, TEST_SCRIPT_RUN_EXT);
            if(iExtPos > 0)
            {
                ss.str("");
                ss << g_szWorkPortPath[iPortIdx] <<  "/" << strScriptName;
                strScriptProcFile = ss.str();

                ss.str("");
                ss << g_szWorkPortPath[iPortIdx] <<  "/" << strScriptName.substr(0, iExtPos-1);
                strScriptRunFile = ss.str();
            }
            else
            {
                printf(">> script = %s\n", szScriptName);
                printf(">> '%s' file is not eixst\n", TEST_SCRIPT_RUN_EXT);
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "EXT NOT EXIST");

                return -4;
            }

            printf(">> strScriptProcFile = %s\n", strScriptProcFile.c_str());
            printf(">> strScriptRunFile = %s\n", strScriptRunFile.c_str());

            //copy script to work folder
            ss.str("");
            ss << "cp -f " << g_szTesterPortPath[iPortIdx] << "/* " << g_szWorkPortPath[iPortIdx];
            strCmd = ss.str();
            int ret = system(strCmd.c_str());

            printf(">> cp cmd = %s\n", strCmd.c_str());
            printf(">> ret = %d\n", ret);

            //start test
            if(g_pTestMng[iPortIdx]->StartTest(version, strScriptProcFile, strScriptRunFile, "") == 0)
            {
                printf(">> test is started!!\n");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEST,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "TEST");
            }
        }
        else
        {
            printf(">> port%d is testing!!\n", port);
        }
    }
    else if(msg_no == MSG_TEST_STOP)
    {
        if(g_pTestMng[iPortIdx]->StopTest(version) == 0)
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
    /*
    else if(msg_no == MSG_INIT)
    {
        SendMsg(g_idMsgq, version, cell, port, MSG_INITACK, 0,  szProgVersion);
        SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,   0,  "MSG_INIT");
        SendMsg(g_idMsgq, version, cell, port, MSG_TEXT2,   0,  "");
        SendMsg(g_idMsgq, version, cell, port, MSG_TEXT3,   0,  "");
        SendMsg(g_idMsgq, version, cell, port, MSG_TEXT4,   0,  "");

        switch(CheckScriptFile(g_szTesertPortPath[iPortIdx], msg_str))
        {
        case SC_FILE_DEFAULT:
            SendMsg(g_idMsgq, version, cell, port, MSG_INITACK,   SC_FILE_DEFAULT, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,    0,	"MSG_INIT");
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT2,    0, "");
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT3,    0, "");
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT4,    0, "");
            break;
        case SC_FILE_PASS:
            SendMsg(g_idMsgq, version, cell, port, MSG_INITACK,	SC_FILE_PASS, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_SIZE_ERR:
            SendMsg(g_idMsgq, version, cell, port, MSG_INITACK,	SC_FILE_SIZE_ERR, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_NONE_ERR:
            SendMsg(g_idMsgq, version, cell, port, MSG_INITACK,	SC_FILE_NONE_ERR, szProgVersion);
            SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,     0, "MSG_INIT");
            break;
        case SC_FILE_OTHER_ERR:
            break;
        }
    }
    else if(msg_no == MSG_INITACK)
    {
        // do nothing
    }
    */
    return 0;
}

static int ProcSock(MsgHdr hdr, char *msg_str)
{
    int version	= hdr.version;
    int cell    = hdr.cell;
    int port    = hdr.port;
    int msg_no	= hdr.msg_no;
    //int packet  = hdr.packet;
    //int flag 	= hdr.flag;

    int iPortIdx = port-1;

    if(version == MSGVER_BD_INFO || version == MSGVER_BD_DIAG || version == MSGVER_BD_UPDATE)
    {
        port = 0;
        iPortIdx = PORT_MIN;
    }
    else if(version == MSGVER_PORT_DPS || version == MSGVER_PORT_DIAG || version == MSGVER_PORT_TEST)
    {
        if(iPortIdx < PORT_MIN || iPortIdx > PORT_MAX)
            return -1;
    }
    else
        return 0;

    if(msg_no == MSG_TEST_START)
    {
        if( g_pTestMng[iPortIdx]->IsTestOn(version) == 0 )
        {
            //check packet string part
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

            while(ssArg >> strArg)
            {
                vectArg.push_back(strArg);
            }

            //get script name and argument
            string strScriptPathName;
            string strScriptArg;

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

            //check script file
            if(!IsFileExist(strScriptPathName.c_str()))
            {
                printf(">> port%d script is not exist!!\n", port);
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "SCRIPT NOT EXIST");

                return -3;
            }

            //get path;
            size_t posSplit = strScriptPathName.find_last_of('/');
            string strScriptPath = strScriptPathName.substr(0, posSplit);
            string strScriptName = strScriptPathName.substr(posSplit+1);
            string strWorkPath;

            if(version == MSGVER_PORT_TEST)
            {
                strWorkPath = g_szWorkPortPath[iPortIdx];
            }
            else
            {
                strWorkPath = g_szWorkExecPath;
            }

            //check script file extension
            stringstream ss;
            string strCmd;
            string strScriptProcFile;
            string strScriptRunFile;

            int iExtPos = CheckScriptExt(strScriptName, TEST_SCRIPT_RUN_EXT);
            if(iExtPos > 0)
            {
                ss.str("");
                ss << strWorkPath <<  "/" << strScriptName;
                strScriptProcFile = ss.str();

                ss.str("");
                ss << strWorkPath <<  "/" << strScriptName.substr(0, iExtPos-1);
                strScriptRunFile = ss.str();
            }
            else
            {
                printf(">> '%s' file is not eixst\n", TEST_SCRIPT_RUN_EXT);
                SendMsg(g_idMsgq, version, cell, port, MSG_DONE,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "EXT NOT EXIST");

                return -4;
            }

            printf(">> strScriptProcFile = %s\n", strScriptProcFile.c_str());
            printf(">> strScriptRunFile = %s\n", strScriptRunFile.c_str());

            //copy script to work folder
            ss.str("");

            if(version == MSGVER_PORT_TEST)
            {
                ss << "cp -f " << strScriptPath << "/* " << strWorkPath;
                strCmd = ss.str();
            }
            else
            {
                ss << "cp -f " << strScriptPathName << " " << strWorkPath;
                strCmd = ss.str();
            }

            int ret = system(strCmd.c_str());

            printf(">> cp cmd = %s\n", strCmd.c_str());
            printf(">> ret = %d\n", ret);

            //start test
            if(g_pTestMng[iPortIdx]->StartTest(version, strScriptProcFile, strScriptRunFile, strScriptArg) == 0)
            {
                printf(">> test is started!!\n");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEST,	0, "");
                SendMsg(g_idMsgq, version, cell, port, MSG_TEXT1,	0, "TEST");
            }
        }
        else
        {
            printf(">> port%d is testing!!\n", port);
        }
    }
    else if(msg_no == MSG_TEST_STOP)
    {
        if(g_pTestMng[iPortIdx]->IsTestOn(version) == 1)
        {
            if(g_pTestMng[iPortIdx]->StopTest(version) == 0)
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
/*
static int CheckScriptFile(char *path, char *msg_str)
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

        if( SearchFile(path, szScriptName, szRealName) == 0 )
        {
            long lScriptSize = atoi(szScriptSize);

            sprintf(szFileName, "%s/%s", path, szRealName);
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
*/

static int CheckScriptExt(string strFileName, string strCheckExt)
{
    string strExt;
    string strName;

    size_t pos = strFileName.find(".", 0);
    if(pos == 0)
        return -1;

    strName = strFileName.substr(0, pos);
    strExt = strFileName.substr(pos+1, -1);

    cout << "strName : " << strName << endl;
    cout << "strSuffix : " << strExt << endl;

    if(strExt.compare(strCheckExt) == 0)
        return pos+1;

    return -2;
}
