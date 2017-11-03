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


typedef struct tSockPack2
{
	MsgHdr			hdr;
	std::string		text;
} SockPack2;

class CSocketClient {
public:
	CSocketClient(int iCell, char *szServerAddr, int iServerPort);
	virtual ~CSocketClient();

public:

	int				IsConnected();
	int				CreateSocket();
	int				ConnectServer();
	void			CloseSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);
	void			StartThread();
	void			StopThread();
	int				SendCheckDummy();
	int				ParseData(std::string strBuf, std::string &strData);
	int				DataSplit(std::string strData, tSockPack2 &sockData);

public:
	int				iCell;
	int				condThread;
	pthread_t		idThread;

	bool			bConnect;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;

	char			szServerSocketAddr[16];
	int				iServerSocketPort;
};

#endif /* SOCKETCLIENT_H_ */
