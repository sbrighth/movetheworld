/*
 * socketserver.h
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include <pthread.h>
#include <netinet/in.h>
#include <string>
#include "def.h"

class CSocketServer {
public:
	CSocketServer(char *szAddr, int iPort);
	virtual ~CSocketServer();

public:
	void			StartThread();
	void			StopThread();
	int				IsConnected();
	int				CreateSocket();
	int				BindSocket();
	int				ListenSocket();
	int				AcceptClient();
	void			CloseServerSocket();
	void			CloseClientSocket();
	int				Recv(char *buf, int size);
	int				Send(char *buf, int size);
	int				ParseData(std::string &strBuf);
	int				SetData(std::string &strData, std::string strBuf);

public:
	int				condThread;
	pthread_t		idThread;

	bool			bAccept;
	int				iServerSocket;
	sockaddr_in		tServerAddr;
	int				iClientSocket;
	sockaddr_in		tClientAddr;

	char			szServerSocketAddr[16];
	int				iServerSocketPort;
};

#endif /* SOCKETSERVER_H_ */
