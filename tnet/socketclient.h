/*
 * socketclient.h
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include <netinet/in.h>
#include "def.h"
#include <string>
#include <poll.h>
#include <queue>

using namespace std;

typedef struct tSockPack2
{
    MsgHdr		hdr;
    string		text;
} SockPack2;

class CSocketClient {
public:
    CSocketClient(int iCell, char *szServerAddr, int iServerPort);
	virtual ~CSocketClient();

public:
    int             InitSockData();
	int				IsConnected();
	int				CreateSocket();
	int				ConnectServer();
	void			CloseSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);
    void			StartThread(void (*SetFunc)(SockPack sockData));
	void			StopThread();
	int				SendCheckDummy();
    int				ParseData(string strBuf, string &strData);
    int				DataSplit(string strData, SockPack &sockData);

public:
	int				iCell;
    void			(*ProcFunc)(SockPack sockData);

    int				condCheckThread;
    pthread_t		idCheckThread;

    int             condProcThread;
    pthread_t       idProcThread;

	bool			bConnect;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;

	char			szServerSocketAddr[16];
	int				iServerSocketPort;

    queue<SockPack> qRecv;
    queue<SockPack> qSend;
};

#endif /* SOCKETCLIENT_H_ */