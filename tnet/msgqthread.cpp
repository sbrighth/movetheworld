/*
 * msgqthread.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include "base.h"
#include "msgqthread.h"

void* MsgqCheckThread(void *arg)
{
	CMsgqThread* pthis = (CMsgqThread *)arg;
	int ret;
	MsgPack msgRead;
	MsgPackQ msgqCheck;

	while(pthis->condCheckThread)
	{
		//pop send msg in MSGQ and pass to MsgBox
		if(CurNumMsgq(pthis->idMsgq) > 0)
		{
			memset(&msgqCheck, 0, sizeof(MsgPackQ));
			msgqCheck.type = TYPE_MSGQ_SEND;

			ret = PopMsgq(pthis->idMsgq, &msgqCheck, sizeof(MsgPackQ));
			if(ret == 0)
			{
				pthis->MsgqLog(msgqCheck.msg, 1);
				pthis->SendMsg(msgqCheck.msg);
			}
		}

		//check recv msg in MsgBox and push to MSGQ
		memset(&msgRead, 0, sizeof(MsgPack));
		ret = pthis->RecvMsg(msgRead);
		if(ret == 0)
		{
			memset(&msgqCheck, 0, sizeof(MsgPackQ));
			msgqCheck.type = TYPE_MSGQ_RECV;
			msgqCheck.msg = msgRead;

			PushMsgq(pthis->idMsgq, &msgqCheck, sizeof(MsgPackQ));
		}

		msleep(100);
	}

	pthread_exit( (void* )0 );
	return (void* )0;
}

void* MsgqProcThread(void *arg)
{
	CMsgqThread *pthis = (CMsgqThread *)arg;
	MsgPackQ msgqProc;

	while(pthis->condProcThread)
	{
		//pop recv msg and pass to proc function
		if(CurNumMsgq(pthis->idMsgq) > 0)
		{
			memset(&msgqProc, 0, sizeof(MsgPackQ));
			msgqProc.type = TYPE_MSGQ_RECV;

			if(PopMsgq(pthis->idMsgq, &msgqProc, sizeof(MsgPackQ)) == 0)
			{
				pthis->MsgqLog(msgqProc.msg, 0);
				pthis->ProcFunc(pthis->idMsgq, msgqProc.msg);
			}
		}

		msleep(100);
	}

	pthread_exit( (void* )0 );
	return (void* )0;
}

int CMsgqThread::MsgqLog(MsgPack msg, int dir)
{
	char dt[20];
	char fbuf[1032]={0};
	time_t t = time(NULL);
	struct tm *tp = localtime(&t);

	strftime(dt, sizeof(dt), "%Y-%m-%d %H:%M:%S", tp);

	sprintf( fbuf, "[%s] version:%5d, cell:%3d, port:%3d, msg_no:%3d, packet:%3d, flag:%3d, string:%s\n",
			dt , msg.version, msg.cell, msg.port, msg.msg_no, msg.packet, msg.flag, msg.string);

	char report[128] = {0,};
	sprintf(report, "%s/%s_%s.txt", SYS_LOG_PATH, MSGQ_LOG_NAME, dir>0?"write":"read");

	FILE* fp;
	fp = fopen( report, "a" );
	if( fp )
	{
		fwrite( fbuf, strlen(fbuf), 1, fp );
		fclose( fp );
	}

	return 0;
}

CMsgqThread::CMsgqThread(int key, string strWriteTarget, string strReadTarget) : CMsgBox(strWriteTarget, strReadTarget)
{
	//TestMsgqThread MsgBox <-> TestMsgq
	idMsgq = CreateMsgq(key);

	condCheckThread = 1;
	condProcThread = 1;
	idCheckThread = 0;
	idProcThread = 0;
}

CMsgqThread::~CMsgqThread()
{
	StopThread();
}

void CMsgqThread::StartThread(void (*SetFunc)(int idMsgq, MsgPack msg))
{
	ProcFunc = SetFunc;
	CMsgBox::StartThread();

	if(idCheckThread == 0)
		pthread_create(&idCheckThread, NULL, &MsgqCheckThread, (void*)this);

	if(idProcThread == 0)
		pthread_create(&idProcThread, NULL, &MsgqProcThread, (void*)this);

}

void CMsgqThread::StopThread()
{
	condCheckThread = 0;
	condProcThread = 0;

	if(idCheckThread != 0)
	{
		pthread_join(idCheckThread, NULL);
		idCheckThread = 0;
	}

	if(idProcThread != 0)
	{
		pthread_join(idProcThread, NULL);
		idProcThread = 0;
	}

	CMsgBox::StopThread();
	ProcFunc = NULL;
}

