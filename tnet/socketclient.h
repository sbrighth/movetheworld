/*
 * socketclient.h
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include <netinet/in.h>

class CSocketClient {
public:
    CSocketClient(char *szServerAddr, int iServerPort);
    virtual ~CSocketClient();

public:
	int				IsConnected();
	int				CreateSocket();
	int				ConnectServer();
	void			CloseSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);

public:
    bool            bReqStop;
	bool			bConnect;
    int             iConnectTimeout;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;
	char			szServerSocketAddr[16];
	int				iServerSocketPort;  
};

#endif /* SOCKETCLIENT_H_ */
