/*
 * def.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef DEF_H_
#define DEF_H_

#define uint8	unsigned char
#define uint16	unsigned short
#define uint32	unsigned int
#define uint64	unsigned long long

#define int8	char
#define int16	short
#define int32	int
#define int64	long long

#define SYS_PATH				"/ssd"
#define SYS_BIN_PATH			SYS_PATH "/bin"
#define SYS_LIB_PATH			SYS_PATH "/lib"
#define SYS_INC_PATH			SYS_PATH "/include"
#define SYS_CFG_PATH			SYS_PATH "/config"
#define SYS_ATH_PATH			SYS_PATH "/athost"
#define SYS_SCR_PATH			SYS_PATH "/script"
#define SYS_SHE_PATH			SYS_PATH "/shell"
#define SYS_SHA_PATH			SYS_PATH "/share"
#define SYS_TMP_PATH			SYS_PATH "/temp"
#define SYS_IMG_PATH			SYS_PATH "/images"
#define SYS_SRC_PATH			SYS_PATH "/src"
#define SYS_DATA_PATH 			SYS_PATH "/data"
#define SYS_FIRM_PATH 			SYS_PATH "/firm"
#define SYS_UTIL_PATH 			SYS_PATH "/util"
#define SYS_DIAG_PATH 			SYS_PATH "/diag"
#define SYS_DEBUG_PATH 			SYS_PATH "/debug"

#define KEY_TEST_MSGQ			0x1000
#define KEY_DEBUG_MSGQ			0x1001

#define KEY_DPS_LOCK			0x2000

#define MSGBOX_SEND_TO			"in"
#define MSGBOX_RECV_FROM		"out"

#define TYPE_MSGQ_SEND			1
#define TYPE_MSGQ_RECV			2

#define ON		1
#define	OFF		0

typedef struct tMsgPackHdr
{
	uint16		version;	// Version of this structure.
	uint16		cell;		// Cell from/to.
	uint16		port;		// Port from/to.
	uint16		msg_no;		// Msg.
	uint16		packet;		// This is running counter (every packet will have unique id
	uint16		flag;
}__attribute__ ((packed)) MsgPackHdr;

#define MSG_PACKET_SIZE			32
#define MAX_MSG_STRING_LENGTH	(MSG_PACKET_SIZE - sizeof(MsgPackHdr))

typedef struct tMsgPack
{
	MsgPackHdr	header;
    char		string[MAX_MSG_STRING_LENGTH];
}__attribute__ ((packed)) MsgPack;

typedef struct tMsgPackQ
{
	long		type;
	MsgPack		msg;
}__attribute__ ((packed)) MsgPackQ;

#endif /* DEF_H_ */
