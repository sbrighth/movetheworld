/*
 * testdm.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TESTDM_H_
#define TESTDM_H_

#include "def.h"
#include "base.h"
#include "msgqthread.h"
#include "msgsend.h"
#include "testmng.h"
#include "status.h"
#include "callback.h"
#include "network/msgsocketserver.h"
#include "network/msgsocketclient.h"

void    InitResource();
void    DeleteResource();
int		CheckProgRunning();
void	ProcSignalStop(int sig_no);
int		SetSignal();
int     GetTpcId();
int		CreateTestFolders();
int		CreateFolder(char *path);
int     NotifyProgReady();

char	*g_szTesterPath;
char    *g_szTesterPortPath[PORT_CNT];
char	*g_szWorkExecPath;
char    *g_szWorkPortPath[PORT_CNT];

int		g_condTestDm;
int		g_idTpc;
int		g_idResShare;
int     g_idTestMsgq;

CMsgqThread         *g_pTestMsgq = NULL;
CMsgSocketServer    *g_pMsgSocketServer = NULL;
CMsgSocketClient    *g_pMsgSocketClient = NULL;
CTestMng            *g_pTestMng[PORT_CNT];
CStatus             *g_pStatusMon = NULL;

#endif /* TESTDM_H_ */
