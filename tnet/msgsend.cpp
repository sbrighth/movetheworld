#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msgsend.h"
#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

int SendMsgPack(int idMsgq, MsgPack msg)
{
	MsgPackQ msgq;
	memset(&msgq, 0, sizeof(MsgPackQ));

	msgq.type = TYPE_MSGQ_SEND;
	msgq.msg = msg;

	return PushMsgq(idMsgq, &msgq, sizeof(MsgPackQ));
}

int SendMsg(int idMsgq, int version, int cell, int port, int msg_no, int flag, const char* str)
{
	MsgPack msg;
	memset(&msg, 0, sizeof(MsgPack));

	int len = strlen(str);

	msg.hdr.version	= version;
	msg.hdr.cell	= cell;
	msg.hdr.port	= port;
	msg.hdr.msg_no	= msg_no;
	msg.hdr.flag 	= flag;
	memcpy(msg.string, str, (len > (int)MSG_STRING_LENGTH) ? MSG_STRING_LENGTH : len);

	return SendMsgPack(idMsgq, msg);
}


#ifdef __cplusplus
}
#endif
