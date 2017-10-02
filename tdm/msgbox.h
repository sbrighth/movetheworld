/*
 * msgbox.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef MSGBOX_H_
#define MSGBOX_H_

#include <iostream>
#include <queue>
#include <pthread.h>
#include "../tbase/def.h"

using namespace std;

class CMsgBox
{
public:
	CMsgBox(string strSendTarget, string strRecvTarget);
	virtual ~CMsgBox();

public:
	MsgPack			NewMsg(uint16 cell, uint16 port, uint16 msg_no, uint16 packet, uint16 flag, const char *data);
	void			PrintMsg(MsgPack msg);
	int				InitMsg();
	int				SendMsg(MsgPack msg);		//in
	int				RecvMsg(MsgPack &msg);		//out
	void			StartThread();
	void			StopThread();

private:
	int				InitMsgFile();
	int				InitMsgFile(string filename);
	int				WriteMsgFile();				//in
	int				ReadMsgFile();				//out
	int				IsFileExist(string filename);
	int				GetFileSize(string filename);
	static void*	MsgFileThread(void* arg);
	void			_MsgFileThread();

	int				condThread;
	pthread_t		idThread;
	queue<MsgPack>	qWriteMsg;
	queue<MsgPack>	qReadMsg;
	string			strWriteFileName;			//in
	string			strReadFileName;			//out
};

class CMsgQBox
{
	CMsgQBox();
	virtual ~CMsgQBox();

public:
	static void*	MsgQThread(void *arg);
	void			_MsgQThread();
	void			StartThread();
	void			StopThread();

	int				condThread;
	pthread_t		idThread;
	int				idMsgQIn;
	int				idMsgQOut;
};

#endif /* MSGBOX_H_ */
