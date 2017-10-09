/*
 * msgbox.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "../tbase/tbase.h"
#include "msgctrl.h"

CMsgBox::CMsgBox(string strWriteTarget, string strReadTarget)
{
	strWriteFileName = strWriteTarget;
	strReadFileName = strReadTarget;
	condThread = 1;
	idThread = 0;
}

CMsgBox::~CMsgBox()
{
	StopThread();
}

MsgPack CMsgBox::NewMsg(uint16 cell, uint16 port, uint16 msg_no, uint16 packet, uint16 flag, const char *data )
{
	MsgPack msg;

	msg.cell = cell;
	msg.port = port;
	msg.msg_no = msg_no;
	msg.packet = packet;
	msg.flag = flag;

	memcpy(msg.string, data, MAX_MSG_STRING_LENGTH);

	return msg;
}

void CMsgBox::PrintMsg(MsgPack msg)
{
	printf("cell:%d, port:%d, msg_no:%d, packet:%d, flag:%d, string:%s\n",
			msg.cell, msg.port, msg.msg_no, msg.packet, msg.flag, msg.string);
}

int CMsgBox::InitMsg()
{
	while( !qWriteMsg.empty() )
	{
		qWriteMsg.pop();
	}

	while( !qReadMsg.empty() )
	{
		qReadMsg.pop();
	}

	return 0;
}

int CMsgBox::SendMsg(MsgPack msg)
{
	qWriteMsg.push(msg);
	return 0;
}

int CMsgBox::RecvMsg(MsgPack &msg)
{
	if( qReadMsg.size() > 0)
	{
		msg = qReadMsg.front();
		qReadMsg.pop();
	}
	else
		return -1;

	return 0;
}

int CMsgBox::InitMsgFile()
{
	if(InitMsgFile(strWriteFileName) < 0)
		return -1;

	if(InitMsgFile(strReadFileName) < 0)
		return -1;

	return 0;
}

int CMsgBox::InitMsgFile(string filename)
{
	MsgPack msg;
	memset(&msg, 0, sizeof(MsgPack));

	FILE* fp = fopen(filename.c_str(), "w");
	if( fp )
	{
		msg.version = 0x3412;
		fwrite(&msg, MSG_PACKET_SIZE, 1, fp);
		fclose(fp);
	}
	else
		return -1;

	return 0;
}

int CMsgBox::WriteMsgFile()
{
	MsgPack *wmsg;
	MsgPack hmsg;		//head message
	int cur_cnt;
	int send_cnt;
	int size;

	send_cnt = qWriteMsg.size();
	if(send_cnt==0)
		return 0;

	FILE* fp = fopen(strWriteFileName.c_str(), "r+");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		cur_cnt = (size / MSG_PACKET_SIZE);

		wmsg = new MsgPack[send_cnt];
		memset(wmsg, 0, sizeof(MsgPack)*send_cnt);

		for(int i=0; i<send_cnt; i++)
		{
			wmsg[i] = qWriteMsg.front();
			wmsg[i].packet = cur_cnt + i;
			qWriteMsg.pop();
		}

		fwrite(wmsg, MSG_PACKET_SIZE, send_cnt, fp);
		fseek(fp, 0, SEEK_SET);

		memset(&hmsg, 0, sizeof(MsgPack));
		hmsg.version = 0x3412;
		hmsg.msg_no = cur_cnt + send_cnt - 1;
		fwrite(&hmsg, MSG_PACKET_SIZE, 1, fp);
		fseek(fp, 0, SEEK_END);

		fclose(fp);
		delete []wmsg;
	}
	else
		return -1;

	return 0;
}

int CMsgBox::ReadMsgFile()
{
	int size = 0;
	int rsize = 0;
	char *temp_buf;
	MsgPack msg;

	FILE* fp = fopen(strReadFileName.c_str(), "r");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		if(size == 0)
		{
			fclose(fp);
			return 0;
		}

		temp_buf = new char[size];
		rsize = fread(temp_buf, size, 1, fp);
		fclose(fp);

		InitMsgFile(strReadFileName);

		for(int i=1; i<size; i++)
		{
			msg = ((MsgPack *)temp_buf)[i];
			qReadMsg.push(msg);
		}

		delete []temp_buf;
	}
	else
		return -1;

	return rsize;
}

void* CMsgBox::MsgFileThread(void* arg)
{
	printf(">> %s start!!\n", __func__);

	CMsgBox* pthis = (CMsgBox*)arg;
	pthis->_MsgFileThread();

	printf(">> %s end!!\n", __func__);
	pthread_exit( (void* )0 );
}

void CMsgBox::_MsgFileThread()
{
	while(condThread == 1)
	{
		if( IsFileExist(strWriteFileName.c_str()) )
			WriteMsgFile();
		else
			InitMsgFile(strWriteFileName.c_str());

		if( IsFileExist(strReadFileName.c_str()) )
			ReadMsgFile();

		usleep(100*1000);
	}
}

void CMsgBox::StartThread()
{
	if(idThread == 0)
		pthread_create(&idThread, NULL, &MsgFileThread, (void*)this);
}

void CMsgBox::StopThread()
{
	condThread = 0;

	if(idThread != 0)
	{
		pthread_join(idThread, NULL);
		idThread = 0;
	}
}



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
				pthis->ProcFunc(proc_msgq.msg);
			}
		}

		msleep(100);
	}

	printf(">> %s end!!\n", __func__);
	pthread_exit( (void* )0 );
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

void CMsgqThread::StartThread(function<void(MsgPack)> SetFunc)
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

