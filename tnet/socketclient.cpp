/*
 * socketclient.cpp
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
#include "base.h"
#include "socketclient.h"

CSocketClient::CSocketClient(char *szServerAddr, int iServerPort) {
	// TODO Auto-generated constructor stub

	if(strlen(szServerAddr) > sizeof(szServerSocketAddr))
		return;

	memcpy(szServerSocketAddr, szServerAddr, sizeof(szServerSocketAddr));
	iServerSocketPort = iServerPort;

    memset( &tServerAddr , 0 , sizeof( tServerAddr ) );
	tServerAddr.sin_family      = AF_INET;						//IPv4
	tServerAddr.sin_port        = htons( iServerSocketPort );	//port
	//tServerAddr.sin_addr.s_addr = htonl( INADDR_ANY );			//32bit IPv4 address
	tServerAddr.sin_addr.s_addr = inet_addr( szServerSocketAddr );			//32bit IPv4 address

	sigset_t    tSigSet;
	sigemptyset( &tSigSet );
	sigaddset( &tSigSet, SIGPIPE );
	pthread_sigmask( SIG_BLOCK , &tSigSet , NULL );

	this->iClientSocket = -1;
	this->bConnect = false;

    this->iConnectTimeout = 10;   //default 10sec
    this->bReqStop = false;
}

CSocketClient::~CSocketClient()
{
    CloseSocket();
}

int CSocketClient::IsConnected()
{
	if(iClientSocket >=0 && bConnect == true)
		return 1;
	else
		return 0;
}

int CSocketClient::CreateSocket()
{
	if(iClientSocket >= 0)
		return 1;

	int iSetSocketOption = 1;

	//create socket (IPV4, TCP/IP)
	if((iClientSocket = socket(PF_INET , SOCK_STREAM , IPPROTO_TCP)) < 0 )
	{
		printf( "socket() Error! errno=%d\n" , errno );
		return -1;
	}
    else
    {
        //printf( "socket() ok\n" );
    }

	//set socket option (SO_REUSEADDR)
	if(setsockopt(iClientSocket , SOL_SOCKET , SO_REUSEADDR , &iSetSocketOption , sizeof(iSetSocketOption)) < 0 )
	{
		printf( "setsockopt() Error! errno=%d\n" , errno );
		return -1;
	}
    else
    {
        //printf( "setsockopt() ok\n" );
    }

	int flag =	fcntl(iClientSocket, F_GETFL, 0);
	fcntl(iClientSocket, F_SETFL, flag | SOCK_NONBLOCK);

	return 0;
}

int CSocketClient::ConnectServer()
{
	if(bConnect)
		return 1;

	if(connect( iClientSocket , (struct sockaddr *)&tServerAddr, sizeof( tServerAddr ) ) < 0)
	{
        //printf( "connect() Error! errno=%d\n" , errno );
        if(errno == EINPROGRESS)
		{
            for(int iLoop=0; iLoop<iConnectTimeout; iLoop++)
            {
                if(bReqStop == true)            //check program exit each 1sec
                    break;

                struct timeval tv;
                tv.tv_sec = 1;
                tv.tv_usec = 0;

                fd_set myset;
                FD_ZERO(&myset);
                FD_SET(iClientSocket, &myset);

                //printf("select (%d/%d) \n", iLoop+1, iConnecTimeout);
                if(select(iClientSocket+1, NULL, &myset, NULL, &tv) > 0)
                {
                    int iGetSocketErr = 0;
                    socklen_t len = sizeof(iGetSocketErr);

                    getsockopt(iClientSocket , SOL_SOCKET , SO_ERROR , &iGetSocketErr , &len);
                    if(iGetSocketErr == 0)
                    {
                        //printf("getsockopt connect() ok!\n");
                        bConnect = true;
                        break;
                    }
                }
            }
		}
    }
	else
	{
        //printf( "connect() ok\n" );
        bConnect = true;
	}

	if(bConnect)
		return 0;
	else
		return -1;
}

void CSocketClient::CloseSocket()
{
    //printf("close socket\n");

	if(iClientSocket >= 0)
	{
        shutdown( iClientSocket , SHUT_RDWR );
		close( iClientSocket );
	}

    iClientSocket = -1;
    bConnect = false;
    bReqStop = false;
}

int CSocketClient::Recv(char *buf, int size)
{
	if(IsConnected() == 0)
		return -1;

	return recv( iClientSocket, buf, size, 0 );
}

int CSocketClient::Send(char *buf, int size)
{
	if(IsConnected() == 0)
		return -1;

	return send( iClientSocket, buf, size, 0 );
}
