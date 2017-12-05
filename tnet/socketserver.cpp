/*
 * socketserver.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "socketserver.h"

CSocketServer::CSocketServer(char *szAddr, int iPort) {
	if(strlen(szAddr) > sizeof(szServerSocketAddr))
		return;

	memcpy(szServerSocketAddr, szAddr, sizeof(szServerSocketAddr));
	iServerSocketPort = iPort;

	memset( &tServerAddr , 0 , sizeof( tServerAddr ) );
	tServerAddr.sin_family      = AF_INET;						//IPv4
	tServerAddr.sin_port        = htons( iServerSocketPort );	//port
	//tServerAddr.sin_addr.s_addr = htonl( INADDR_ANY );			//32bit IPv4 address
	tServerAddr.sin_addr.s_addr = inet_addr( szServerSocketAddr );

	sigset_t    tSigSet;
	sigemptyset( &tSigSet );
	sigaddset( &tSigSet, SIGPIPE );
	pthread_sigmask( SIG_BLOCK , &tSigSet , NULL );

	this->iServerSocket = -1;
	this->iClientSocket = -1;
	this->bAccept = false;
}

CSocketServer::~CSocketServer() {
   CloseClientSocket();
   CloseServerSocket();
}

int CSocketServer::IsConnected()
{
	if(iServerSocket >= 0 && iClientSocket >=0 && bAccept == true)
		return 1;
	else
		return 0;
}

int CSocketServer::CreateSocket()
{
	int iSetSocketOption = 1;

	//create socket (IPV4, TCP/IP)
	if((iServerSocket = socket(PF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0 )
	{
		printf( "socket() Error! errno=%d\n" , errno );
		return -1;
	}
	else
	{
        //printf( "socket() ok\n" );
	}

	//set socket option (SO_REUSEADDR)
	if(setsockopt(iServerSocket , SOL_SOCKET , SO_REUSEADDR , &iSetSocketOption , sizeof(iSetSocketOption)) < 0 )
	{
		printf( "setsockopt() Error! errno=%d\n" , errno );
		return -1;
	}
	else
	{
        //printf( "setsockopt() ok\n" );
	}

	int flag =	fcntl(iServerSocket, F_GETFL, 0);
	fcntl(iServerSocket, F_SETFL, flag | O_NONBLOCK);

	return 0;
}

int CSocketServer::BindSocket()
{
	//bind socket
	if(bind( iServerSocket , (struct sockaddr *) &tServerAddr , sizeof( tServerAddr )) < 0 )
	{
		printf( "bind() Error! errno=%d\n" , errno );
		return -1;
	}
	else
	{
        //printf( "bind() ok\n" );
	}

	return 0;
}

int CSocketServer::ListenSocket()
{
	if(listen( iServerSocket , 5 ) < 0)
	{
		printf( "listen() Error! errno=%d\n" , errno );
		return -1;
	}
	else
	{
        //printf( "listen() ok\n" );
	}

	return 0;
}

int CSocketServer::AcceptClient()
{
    if(bAccept)
        return 0;

	socklen_t iClientAddrSize = sizeof( tClientAddr );
	iClientSocket = accept( iServerSocket , (struct sockaddr *)&tClientAddr, &iClientAddrSize );

	if(iClientSocket < 0)
	{
		printf( "accept() Error! errno=%d\n" , errno );
		bAccept = false;
		return -1;
	}
	else
	{
        //printf( "accept() ok\n" );
		bAccept = true;
	}

	int flag =	fcntl(iClientSocket, F_GETFL, 0);
    fcntl(iClientSocket, F_SETFL, flag | O_NONBLOCK);

	return 0;
}

void CSocketServer::CloseServerSocket()
{
    //printf("close server socket\n");

	if(iServerSocket > 0)
	{
		shutdown( iServerSocket , SHUT_RDWR );
		close( iServerSocket );
		iServerSocket = -1;
	}
}

void CSocketServer::CloseClientSocket()
{
    //printf("close client socket\n");

	if(iClientSocket > 0)
	{
		shutdown( iClientSocket , SHUT_RDWR );
		close( iClientSocket );
		iClientSocket = -1;
		bAccept = false;
	}
}

int CSocketServer::Recv(char *buf, int size)
{
	if(IsConnected() == 0)
		return -1;

	return recv( iClientSocket, buf, size, 0 );
}

int CSocketServer::Send(char *buf, int size)
{
	if(IsConnected() == 0)
		return -1;

	return send( iClientSocket, buf, size, 0 );
}
