/*
 * tdm.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TDM_H_
#define TDM_H_

#include "def.h"

#define SC_FILE_DEFAULT		0x0
#define SC_FILE_PASS		0x1
#define SC_FILE_NAME_ERR	0x2
#define SC_FILE_SIZE_ERR	0x3
#define SC_FILE_NONE_ERR	0x4
#define SC_FILE_OTHER_ERR	0x5

#define PROG_NAME		"tdm"
#define PROG_VERSION	"0.0.7"

int		CheckProgRunning();
void	ProcSignalStop(int sig_no);
int		SetSignal();
int		CreateWorkFolder(string path);
void	RecvMsgProc(int idMsgq, MsgPack msg);
int		CheckTestRunning(int port);
int		StartTestThread(int port);
int		StopTestThread(int port);
int		CheckScriptFile(int msg_no, string msg_str);

char	g_test_path[256];
int		g_test_status[PORT_MAX];
int		g_condTestDm;
int		g_idTpc;
int		g_idResShare;

CMsgqThread *g_pTestMsgq = NULL;


#endif /* TDM_H_ */
