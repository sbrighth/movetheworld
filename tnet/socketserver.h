/*
 * socketserver.h
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include <netinet/in.h>
#include <string>
#include <poll.h>
#include <queue>
#include "def.h"

using namespace std;

class CSocketServer {
public:
    CSocketServer(char *szAddr, int iPort);
	virtual ~CSocketServer();

public:
    int             InitSockData();
    int				IsConnected();
    int				CreateSocket();
	int				BindSocket();
	int				ListenSocket();
	int				AcceptClient();
	void			CloseServerSocket();
	void			CloseClientSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);
    void			StartThread(void (*SetFunc)(SockPack sockData));
    void			StopThread();
    int				StripMark(string strBuf, string &strData);
    int				DataSplit(string strData, SockPack &sockData);

public:
    int				iCell;
    void			(*ProcFunc)(SockPack sockData);

    int				condCheckThread;
    int             condProcThread;
    pthread_t		idCheckThread;
    pthread_t       idProcThread;

	bool			bAccept;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;

	char			szServerSocketAddr[16];
	int				iServerSocketPort;

    queue<SockPack> qRecv;
    queue<SockPack> qSend;
};

#endif /* SOCKETSERVER_H_ */
