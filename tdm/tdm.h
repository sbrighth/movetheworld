/*
 * tdm.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TDM_H_
#define TDM_H_

int		CheckProgRunning();
void	ProcSignalStop(int sig_no);
int		SetSignal();
int		CreateWorkFolder(string path);
void	RecvMsgProc(int idMsgq, MsgPack msg);


#endif /* TDM_H_ */
