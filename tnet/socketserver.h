/*
 * socketserver.h
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include <netinet/in.h>

class CSocketServer {
public:
    CSocketServer(char *szAddr, int iPort);
    virtual ~CSocketServer();

public:
    int				IsConnected();
    int				CreateSocket();
	int				BindSocket();
	int				ListenSocket();
	int				AcceptClient();
	void			CloseServerSocket();
	void			CloseClientSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);

public:
	bool			bAccept;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;

	char			szServerSocketAddr[16];
	int				iServerSocketPort;
};

#endif /* SOCKETSERVER_H_ */
