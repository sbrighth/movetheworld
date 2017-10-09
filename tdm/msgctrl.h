/*
 * msgbox.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef MSGCTRL_H_
#define MSGCTRL_H_

#include <iostream>
#include <queue>
#include <pthread.h>
#include <functional>
#include "../tbase/def.h"

using namespace std;

class CMsgBox
{
public:
	CMsgBox(string strWriteTarget, string strReadTarget);
	virtual ~CMsgBox();

public:
	MsgPack			NewMsg(uint16 cell, uint16 port, uint16 msg_no, uint16 packet, uint16 flag, const char *data);
	void			PrintMsg(MsgPack msg);
	int				InitMsg();
	int				SendMsg(MsgPack msg);		//in
	int				RecvMsg(MsgPack &msg);		//out
	void			StartThread();
	void			StopThread();

	string			strWriteFileName;			//in
	string			strReadFileName;			//out

private:
	int				InitMsgFile();
	int				InitMsgFile(string filename);
	int				WriteMsgFile();				//in
	int				ReadMsgFile();				//out
	static void*	MsgFileThread(void* arg);
	void			_MsgFileThread();

	int				condThread;
	pthread_t		idThread;
	queue<MsgPack>	qWriteMsg;
	queue<MsgPack>	qReadMsg;
};

class CMsgqThread : public CMsgBox
{
public:
	CMsgqThread(int key, string strWriteTarget, string strReadTarget);
	virtual ~CMsgqThread();

public:
	int				InitMsgq();
	void			StartThread(function<void(MsgPack)> SetFunc);
	void			StopThread();

	function<void(MsgPack)> ProcFunc;

	int				idMsgq;
	pthread_t		idCheckThread;
	pthread_t		idProcThread;
	int				condCheckThread;
	int				condProcThread;
};

#endif /* MSGCTRL_H_ */
