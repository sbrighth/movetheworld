#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include "def.h"

#define STRINGIFY(x)     #x
#define ARG_TO_STRING(x) STRINGIFY(x)

//<define>
//MSG_VER
//TPC_ID
//PORT_NUM
//TEST_FLAG
//LOG_FILE

#ifndef MSG_VER
int MSG_VER=0;
#endif

#ifndef TPC_ID
int TPC_ID=0;
#endif

#ifndef PORT_NUM
int PORT_NUM=0;
#endif

#ifndef TEST_FLAG
int TEST_FLAG=0;
#endif

#ifndef LOG_FILE
char szLogFile[256] = SYS_WORK_PATH"/log_noname.txt";
#else
char szLogFile[256] = ARG_TO_STRING(LOG_FILE);
#endif

int idTestMsgq;

int SetLogName(char *name)
{
    snprintf(szLogFile, sizeof(szLogFile)-1, "%s/%s", SYS_WORK_PATH, name);
    return 0;
}

int	Print(const char *fmt, ...)
{
    va_list	ap;
    char buf[1024]={0};

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, ap);
    va_end(ap);

    printf( "%s",  buf);
    fflush(stdout);

    char report[256] = {0,};
    sprintf(report, "%s", szLogFile);

    FILE* fp;
    fp = fopen( report, "a" );
    if( fp )
    {
        fwrite( buf, strlen(buf), 1, fp );
        fclose( fp );
    }

    return 0;
}

void TEST_INIT(argc, argv)
{
    idTestMsgq = CreateMsgq(KEY_TEST_MSGQ);
}

int SEND_TEXT1(char *string)
{
    if(string == NULL)
        return -1;

    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT1,	0, string);
    }
}

int SEND_TEXT2(char *string)
{
    if(string == NULL)
        return -1;

    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT2,	0, string);
    }
}

int SEND_TEXT3(char *string)
{
    if(string == NULL)
        return -1;

    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT3,	0, string);
    }
}

int SEND_TEXT4(char *string)
{
    if(string == NULL)
        return -1;

    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT4,	0, string);
    }
}

int MSG_STEP(int cur, int max)
{
    char string[20];
    sprintf(string, "%d/%d", cur, max);

    if(MSG_VER>0)
    {
        SEND_TEXT2(string);
    }
}

int TEST_PASS()
{
    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_PASS,	0, "");
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT1,	0, "pass");
    }
}

int TEST_FAIL()
{
    if(MSG_VER>0)
    {
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_FAIL,	0, "");
        SendMsg(idTestMsgq, MSG_VER, TPC_ID, PORT_NUM, MSG_TEXT1,	0, "fail");
    }
}

#endif // SCRIPT_H
