/*
 * testdm.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TESTDM_H_
#define TESTDM_H_

#include "def.h"
#include "def.h"
#include "base.h"
#include "msgqthread.h"
#include "msgsend.h"
#include "socketclient.h"
#include "testmng.h"
#include "socketclient.h"

#define SC_FILE_DEFAULT		0x0
#define SC_FILE_PASS		0x1
#define SC_FILE_NAME_ERR	0x2
#define SC_FILE_SIZE_ERR	0x3
#define SC_FILE_NONE_ERR	0x4
#define SC_FILE_OTHER_ERR	0x5

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.9"

int		CheckProgRunning();
void	ProcSignalStop(int sig_no);
int		SetSignal();
int		CreateTestFolders();
int		CreateFolder(char *path);
void	RecvMsgProc(MsgPack msg);
void	RecvSockProc(SockPack sockData);
int		CheckScriptFile(char *msg_str);
int     ProcTypeMsgInfo(MsgHdr hdr, char *msg_str);
int     ProcTypeMsgTest(MsgHdr hdr, char *msg_str);

char	g_szTestPath[PATHNAME_SIZE];
int		g_condTestDm;
int		g_idTpc;
int		g_idResShare;

CMsgqThread *g_pTestMsgq = NULL;
CTestMng	**g_pTestMng = NULL;

#endif /* TESTDM_H_ */
