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
#include "socketserver.h"
#include "socketclient.h"
#include "status.h"
#include "callback.h"

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
char	*g_szWorkPath;
char    *g_szWorkPortPath[PORT_CNT];
int		g_condTestDm;
int		g_idTpc;
int		g_idResShare;
int     g_idMsgq;

CMsgqThread     *g_pTestMsgq = NULL;
CSocketServer   *g_pSocketServer = NULL;
CSocketClient   *g_pSocketClient = NULL;
CTestMng        **g_ppTestMng = NULL;
CStatus         *g_pStatusMon = NULL;

#endif /* TESTDM_H_ */
