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
#include "base.h"
#include "msgqthread.h"

void* MsgqCheckThread(void *arg)
{
	CMsgqThread* pthis = (CMsgqThread *)arg;
	int ret;
	MsgPack read_msg;
	MsgPackQ temp_msgq;

	printf(">> %s start!!\n", __func__);

	while(pthis->condCheckThread)
	{
		//send msg to MsgBox
		if(CurNumMsgq(pthis->idMsgq) > 0)
		{
			memset(&temp_msgq, 0, sizeof(MsgPackQ));
			temp_msgq.type = TYPE_MSGQ_SEND;

			ret = PopMsgq(pthis->idMsgq, &temp_msgq, sizeof(MsgPackQ));
			if(ret == 0)
				pthis->SendMsg(temp_msgq.msg);
		}

		//read msg from MsgBox
		ret = pthis->RecvMsg(read_msg);
		if(ret == 0)
		{
			memset(&temp_msgq, 0, sizeof(MsgPackQ));
			temp_msgq.type = TYPE_MSGQ_RECV;
			temp_msgq.msg = read_msg;

			PushMsgq(pthis->idMsgq, &temp_msgq, sizeof(MsgPackQ));
		}

		msleep(100);
	}

	printf(">> %s end!!\n", __func__);
	pthread_exit( (void* )0 );
	return (void* )0;
}

void* MsgqProcThread(void *arg)
{
	CMsgqThread *pthis = (CMsgqThread *)arg;
	MsgPackQ proc_msgq;

	printf(">> %s start!!\n", __func__);

	while(pthis->condProcThread)
	{
		//Get msg from msgq
		if(CurNumMsgq(pthis->idMsgq) > 0)
		{
			memset(&proc_msgq, 0, sizeof(MsgPackQ));
			proc_msgq.type = TYPE_MSGQ_RECV;

			if(PopMsgq(pthis->idMsgq, &proc_msgq, sizeof(MsgPackQ)) == 0)
			{
				printf(">> incoming process msg!!!\n");
				//pthis->PrintMsg(proc_msgq.msg);
				pthis->ProcFunc(pthis->idMsgq, proc_msgq.msg);
			}
		}

		msleep(100);
	}

	printf(">> %s end!!\n", __func__);
	pthread_exit( (void* )0 );
	return (void* )0;
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

