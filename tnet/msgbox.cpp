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
#include "base.h"
#include "msgbox.h"

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
	if(qReadMsg.size() > 0)
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

	if(filename.empty())
		return -1;

	FILE* fp = fopen(filename.c_str(), "w");
	if(fp)
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
	int cntCurMsg;
	int cntWriteMsg;
	long lFileSize;

	cntWriteMsg = qWriteMsg.size();
	if(cntWriteMsg==0)
		return 0;

	if(strWriteFileName.empty())
		return -1;

	FILE* fp = fopen(strWriteFileName.c_str(), "r+");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		lFileSize = ftell(fp);

		if(lFileSize % MSG_PACKET_SIZE != 0)
		{
			fclose(fp);
			return -2;
		}
		else
		{
			cntCurMsg = (lFileSize / MSG_PACKET_SIZE) - 1;
		}

		wmsg = new MsgPack[cntWriteMsg];
		memset(wmsg, 0, sizeof(MsgPack)*cntWriteMsg);

		for(int i=0; i<cntWriteMsg; i++)
		{
			wmsg[i] = qWriteMsg.front();
			qWriteMsg.pop();
		}

		fwrite(wmsg, MSG_PACKET_SIZE, cntWriteMsg, fp);
		fseek(fp, 0, SEEK_SET);

		memset(&hmsg, 0, sizeof(MsgPack));
		hmsg.version = 0x3412;
		hmsg.msg_no = cntCurMsg + cntWriteMsg;
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
	int cntCurMsg;
	int cntReadMsg;
	long lFileSize = 0;
	char *buf;
	MsgPack msg;

	if(strReadFileName.empty())
		return -1;

	FILE* fp = fopen(strReadFileName.c_str(), "r");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		lFileSize = ftell(fp);
		if(lFileSize <= MSG_PACKET_SIZE)
		{
			fclose(fp);
			return 0;
		}
		else if(lFileSize % MSG_PACKET_SIZE != 0)
		{
			printf(">> %s read file size error\n", __func__);
			fclose(fp);
			return -2;
		}

		buf = new char[lFileSize];
		fseek(fp, 0, SEEK_SET);
		fread(buf, lFileSize, 1, fp);
		fclose(fp);

		msg = ((MsgPack *)buf)[0];
		cntCurMsg = msg.msg_no;
		cntReadMsg = lFileSize/MSG_PACKET_SIZE - 1;

		if(cntCurMsg != cntReadMsg)
		{
			printf(">> %s read file cnt error\n", __func__);
			return 0;
		}

		InitMsgFile(strReadFileName);

		for(int i=1; i<=cntCurMsg; i++)
		{
			msg = ((MsgPack *)buf)[i];
			qReadMsg.push(msg);
			PrintMsg(msg);
		}

		delete []buf;
	}
	else
		return -1;

	return cntCurMsg;
}

void* CMsgBox::MsgFileThread(void* arg)
{
	CMsgBox* pthis = (CMsgBox*)arg;
	pthis->_MsgFileThread();

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
