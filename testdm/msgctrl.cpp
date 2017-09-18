/*
 * msgctrl.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "msgctrl.h"
#include "def.h"


#define MSG_TYPE_FILE	1		//in/out file

static int g_condMsgInFileThread = 1;
static int g_condMsgOutFileThread = 1;
static int g_idMsgQIn = 0;
static int g_idMsgQOut = 0;

int CreateMsgQ(int key)
{
	int msgq_id = msgget(key, IPC_CREAT|0666);

	printf("[%s-%s] key: %d, msgq_id: %d\n", __FILE__, __func__, key, msgq_id);
	return msgq_id;
}

int DeleteMsgQ(int msgq_id)
{
	if(msgq_id >= 0)
	{
		printf("[%s-%s] msgq_id: %d\n", __FILE__, __func__, msgq_id);
		return msgctl(msgq_id, IPC_RMID, 0);
	}

	return 0;
}

int CurNumMsgQ(int msgq_id)
{
	msqid_ds buf;
	msgctl(msgq_id, IPC_STAT, &buf );

	return buf.msg_qnum;
}

MsgPack InitMsg(uint16 cell, uint16 port, uint16 msg_no, uint16 packet, uint16 flag, const char *data )
{
	MsgPack msg;

	msg.header.cell = cell;
	msg.header.port = port;
	msg.header.msg_no = msg_no;
	msg.header.packet = packet;
	msg.header.flag = flag;

	memcpy(msg.string, data, MAX_MSG_STRING_LENGTH);

	return msg;
}

int PutMsg(int msgq_id, MsgPack msg)
{
	int ret;
	MsgQData msg_data;

	msg_data.data_type = MSG_TYPE_FILE;
	msg_data.msg = msg;

	ret = msgsnd(msgq_id, &msg_data, sizeof(MsgQData)-sizeof(long), 0);
	if(ret < 0)
		printf("msgq errno = %d\n", errno);

	PrintMsg(msg);
	return ret;
}

int GetMsg(int msgq_id, MsgPack *msg)
{
	int ret;
	MsgQData msg_data;

	ret = msgrcv(msgq_id, &msg_data, sizeof(MsgQData)-sizeof(long), MSG_TYPE_FILE, IPC_NOWAIT);

	PrintMsg(msg_data.msg);
	memcpy(msg, &msg_data.msg, MSG_PACKET_SIZE);

	return ret;
}

void PrintMsg(MsgPack msg)
{
	printf("cell:%d, port:%d, msg_no:%d, packet:%d, flag:%d, string:%s\n",
			msg.header.cell, msg.header.port, msg.header.msg_no, msg.header.packet, msg.header.flag, msg.string);
}

void InitMsgThread()
{
	g_condMsgInFileThread = 1;
	g_condMsgOutFileThread = 1;

	g_idMsgQIn = CreateMsgQ(KEY_MSGQ_IN);
	g_idMsgQOut = CreateMsgQ(KEY_MSGQ_OUT);
}

void StopMsgThread()
{
	g_condMsgInFileThread = 0;
	g_condMsgOutFileThread = 0;
}

int IsFileExist(const char* filename)
{
	if(access(filename, F_OK) == 0)
		return 1;
	else
		return 0;
}

int GetFileSize(const char* filename)
{
	int nSize = 0;
	FILE* fp = fopen(filename, "r");
	if( fp )
	{
		fseek(fp,0,SEEK_END);
		nSize = ftell(fp);
		fclose(fp);
	}

	return nSize;
}

int InitMsgFile(const char* filename)
{
	MsgPack msg;

	FILE* fp = fopen(filename, "w");
	if( fp )
	{
		msg.header.version = 0x3412;
		fwrite(&msg, MSG_PACKET_SIZE, 1, fp);
		fclose(fp);
	}

	return 0;
}

int WriteMsgFile(const char* filename, MsgPack msg)
{
	MsgPack hmsg;
	int msg_cnt;
	int size;

	FILE* fp = fopen(filename, "a+");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		msg_cnt = (size / MSG_PACKET_SIZE);
		msg.header.packet = msg_cnt;
		fwrite(&msg, MSG_PACKET_SIZE, 1, fp);

		fseek(fp, 0, SEEK_SET);
		hmsg.header.version = 0x3412;
		hmsg.header.msg_no = msg_cnt;
		fwrite(&hmsg, MSG_PACKET_SIZE, 1, fp);

		fclose(fp);
	}

	return 0;
}

int ReadMsgFile(const char* filename, MsgPack *msg)
{
	int size = 0;
	int rsize = 0;

	FILE* fp = fopen(filename, "r");
	if( fp )
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rsize = fread(msg, size, 1, fp);
		fclose(fp);
	}

	return rsize;
}

void *MsgInFileThread(void* arg)
{
	char filename[128];
	int cell = *(int *)arg;
	MsgPack msg;

	sprintf(filename, SYS_ATH_PATH "/rack_001/tester%03d/exe/in", cell);

	printf(">> MsgInFileThread !!\n");
	while(g_condMsgInFileThread == 1)
	{
		if( IsFileExist(filename) )
		{
			if( CurNumMsgQ(g_idMsgQIn) > (int)MSG_PACKET_SIZE )
			{
				if(GetMsg(g_idMsgQIn, &msg) > 0)
				{
					WriteMsgFile(filename, msg);
				}
			}
		}
		else
		{
			InitMsgFile(filename);
		}

		usleep(100*1000);
	}

	printf(">> MsgInFileThread end!!\n");
	pthread_exit( (void* )0 );
}

void *MsgOutFileThread(void* arg)
{
	char filename[128];
	int cell = *(int*)arg;
	MsgPack temp_msg[256+1];		//msg can't exceed 256
	int size;
	int i;

	sprintf(filename, SYS_ATH_PATH "/rack_001/tester%03d/exe/out", cell);
	printf(">> MsgOutFileThread !!\n");

	while(g_condMsgOutFileThread == 1)
	{
		if( IsFileExist(filename) )
		{
			size = ReadMsgFile(filename, temp_msg);
			if(size > 0)
			{
				InitMsgFile(filename);

				for(i=1; i<size; i++)
					PutMsg(g_idMsgQIn, temp_msg[i]);
			}
		}

		usleep(100*1000);
	}

	printf(">> MsgOutFileThread end!!\n");
	pthread_exit( (void* )0 );
}

void MsgProcThread(void *arg)
{

}
