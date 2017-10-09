#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msgproc.h"
#include "../tbase/tbase.h"

static int g_idMsgq;

int InitMsgProc(int key)
{
	g_idMsgq = CreateMsgq(key);

	return 0;
}

void RecvMsgProc(MsgPack msg)
{
	MsgPack msg_temp;
	memset(&msg_temp, 0, sizeof(MsgPack));

	msg_temp.cell = msg.cell;
	msg_temp.port = msg.port;

	switch(msg.msg_no)
	{
	case MSG_TEST_START:
		//start process;
		msg_temp.msg_no = MSG_TEST;
		SendMsg(msg_temp);
		break;
	case MSG_TEST_STOP:
		//stop process;
		msg_temp.msg_no = MSG_FAIL;
		SendMsg(msg_temp);
		break;
	case MSG_INIT:
		break;
	case MSG_INITACK:
		break;
	case MSG_REBOOT:
		break;
	}
}

int SendMsg(MsgPack msg)
{
	MsgPackQ msgq;
	memset(&msgq, 0, sizeof(MsgPackQ));

	msgq.type = TYPE_MSGQ_SEND;
	msgq.msg = msg;

	return PushMsgq(g_idMsgq, &msgq, sizeof(MsgPackQ));
}

int SendMsg(int cell, int port, int msg_no, char *text)
{
	MsgPack msg;
	MsgPackQ msgq;
	memset(&msg, 0, sizeof(MsgPack));
	memset(&msgq, 0, sizeof(MsgPackQ));

	msg.cell = cell;
	msg.port = port;
	msg.msg_no = msg_no;
	memcpy(msg.string, text, MAX_MSG_STRING_LENGTH);

	msgq.type = TYPE_MSGQ_SEND;
	msgq.msg = msg;

	return PushMsgq(g_idMsgq, &msgq, sizeof(MsgPackQ));
}
