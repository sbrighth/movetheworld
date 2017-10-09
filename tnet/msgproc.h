#ifndef TBASE_H_
#define TBASE_H_

#include "../tbase/def.h"

int InitMsgProc(int key);

void RecvMsgProc(MsgPack msg);

int SendMsgInit();
int SendMsgInitAck();
int SendMsgPass();
int SendMsgFail();
int SendMsgInitColor();

int SendMsgText1(char *text);
int SendMsgText2(char *text);
int SendMsgText3(char *text);
int SendMsgText4(char *text);

int SendMsg(MsgPack msg);
int SendMsg(int cell, int port, int msg_no, char* text);

int	idMsgq;

#endif
