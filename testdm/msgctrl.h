/*
 * msgctrl.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef MSGCTRL_H_
#define MSGCTRL_H_

#include "def.h"

typedef struct tMsgPackHdr
{
	uint16		version;	// Version of this structure.
	uint16		cell;		// Cell from/to.
	uint16		port;		// Port from/to.
	uint16		msg_no;		// Msg.
	uint16		packet;		// This is running counter (every packet will have unique id
	uint16		flag;
}__attribute__ ((packed)) MsgPackHdr;

#define MSG_PACKET_SIZE				sizeof(MsgPackHdr)
#define MAX_MSG_STRING_LENGTH		(MSG_PACKET_SIZE - sizeof(MsgPackHdr))

typedef struct tMsgPack
{
	MsgPackHdr	header;
    char		string[MAX_MSG_STRING_LENGTH];
}__attribute__ ((packed)) MsgPack;

typedef struct tMsgQData
{
	long		data_type;
	MsgPack		msg;
}__attribute__ ((packed)) MsgQData;



int			CreateMsgQ(int key);
int			DeleteMsgQ(int msgq_id);
int			CurNumMsgQ(int msgq_id);
MsgPack		InitMsg(uint16 cell, uint16 port, uint16 msg_no, uint16 packet, uint16 flag, const char *data );
int			PutMsg(int msgq_id, MsgPack msg);
int			GetMsg(int msgq_id, MsgPack msg);
void		PrintMsg(MsgPack msg);
void		InitMsgThread();
void		StopMsgThread();

int			InitMsgFile(const char* filename);
int			WriteMsgFile(const char* filename, MsgPack msg);
int			ReadMsgFile(const char* filename, MsgPack *msg);
void		*MsgInFileThread(void* arg);
void		*MsgOutFileThread(void* arg);
void		MsgProcThread(void *arg);
#endif /* MSGCTRL_H_ */
