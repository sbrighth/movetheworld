/*
 * msgqthread.h
 *
 *  Created on: Sep 12, 2017
 *      Author: shjeong
 */

#ifndef MSGQTHREAD_H_
#define MSGQTHREAD_H_

#include <iostream>
#include <queue>
#include <pthread.h>
#include "def.h"
#include "msgbox.h"

using namespace std;

class CMsgqThread : public CMsgBox
{
public:
	CMsgqThread(int key, string strWriteTarget, string strReadTarget);
	virtual ~CMsgqThread();

public:
	int				InitMsgq();
    void			StartThread(void (*SetFunc)(MsgPack msg));
	void			StopThread();
	int				MsgqLog(MsgPack msg, int dir);

    void			(*ProcFunc)(MsgPack msg);

	int				idMsgq;
	pthread_t		idCheckThread;
	pthread_t		idProcThread;
	int				condCheckThread;
	int				condProcThread;
};

#endif /* MSGQTHREAD_H_ */
