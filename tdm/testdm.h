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

int		CheckProgRunning();
void	ProcSignalStop(int sig_no);
int		SetSignal();
int		CreateTestFolders();
int		CreateFolder(char *path);

char	g_szTestPath[PATHNAME_SIZE];
int		g_condTestDm;
int		g_idTpc;
int		g_idResShare;

CMsgqThread     *g_pTestMsgq = NULL;
CSocketServer   *g_pSocketServer = NULL;
CSocketClient   *g_pSocketClient = NULL;
CTestMng        **g_ppTestMng = NULL;
CStatus         *g_pStatusMon = NULL;

#endif /* TESTDM_H_ */
