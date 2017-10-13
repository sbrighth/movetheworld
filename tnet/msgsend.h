#ifndef MSGSEND_H_
#define MSGSEND_H_

#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

int SendMsgPack(int idMsgq, MsgPack msg);
int SendMsg(int idMsgq, int cell, int port, int msg_no, int flag, const char* str);

#ifdef __cplusplus
}
#endif

#endif	/* MSGSEND_H_ */
