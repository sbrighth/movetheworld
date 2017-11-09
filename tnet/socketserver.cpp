/*
 * socketserver.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#include "socketserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <errno.h>
#include <string>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>

CSocketServer::CSocketServer(char *szAddr, int iPort) {
	// TODO Auto-generated constructor stub

	condThread = OFF;
	idThread = 0;

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
	// TODO Auto-generated destructor stub

	StopThread();
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
	//if(bAccept)
	//	return 0;

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

void *SocketServerThread( void *arg )
{
	CSocketServer *pthis = (CSocketServer *)arg;
	std::string strBuf;

	if(pthis->CreateSocket() < 0)
	{
		pthis->CloseServerSocket();
		return (void *)-1;
	}

	if(pthis->BindSocket() < 0)
	{
		pthis->CloseServerSocket();
		return (void *)-1;
	}

	if(pthis->ListenSocket() < 0)
	{
		pthis->CloseServerSocket();
		return (void *)-1;
	}

	struct pollfd m_tPollEvent[2];
	memset(m_tPollEvent, 0, sizeof(m_tPollEvent));

	m_tPollEvent[0].fd        = pthis->iServerSocket;
	m_tPollEvent[0].events    = POLLIN;
	m_tPollEvent[0].revents   = 0;

	m_tPollEvent[1].fd = -1;

	int rnd = 0;
	pthis->condThread = ON;
	while( pthis->condThread == ON )                // Socket receive Loop
	{
		if( poll((struct pollfd*)&m_tPollEvent, 2, 1000) > 0 )
		{
            //printf(">> 0 revent = %d\n", m_tPollEvent[0].revents);
            //printf(">> 1 revent = %d\n", m_tPollEvent[1].revents);

			if( m_tPollEvent[0].revents & POLLIN )
			{
				if(pthis->AcceptClient() < 0)
				{
					printf( "accept() Error!!! errno=%d\n", errno );
					pthis->CloseClientSocket();

					m_tPollEvent[1].fd = -1;
					m_tPollEvent[1].events = 0;
					m_tPollEvent[1].revents = 0;
				}
				else
				{
                    //printf( "accept() done\n" );

					m_tPollEvent[1].revents = 0;

					m_tPollEvent[1].fd = pthis->iClientSocket;
					m_tPollEvent[1].events = POLLIN;
					m_tPollEvent[1].revents = 0;
				}

				sleep(1);
				strBuf.erase();
				continue;
			}

			if( m_tPollEvent[1].revents & POLLIN )
			{
				char cRecvBuf[4096];
				char cSendBuf[4096];
				memset( cRecvBuf , 0 , sizeof( cRecvBuf ) );

				int iCnt = pthis->Recv(cRecvBuf, sizeof(cRecvBuf));
				if( iCnt <= 0 )
				{
					printf( "socket recv() Error! errno=%d\n" , errno );
					pthis->CloseClientSocket();

					m_tPollEvent[1].fd = -1;
					m_tPollEvent[1].events = 0;
					m_tPollEvent[1].revents = 0;
					continue;
				}

				strBuf.append(cRecvBuf, (strlen(cRecvBuf) > (size_t)iCnt)? iCnt : strlen(cRecvBuf));

                //printf(">> recv count = %ld, iCnt = %d\n", strlen(cRecvBuf), iCnt);
                //printf(">> before erase strBuf = %s\n", strBuf.c_str());

				int ret = pthis->ParseData(strBuf);
				if( ret > 0)
				{
					strBuf.erase(0, ret);
                    //printf(">> erase pos = %d\n", ret);
                    //printf(">> after erase strBuf = %s\n", strBuf.c_str());
				}
				else
				{
                    //printf(">> strBuf is not enogh!!\n");
					continue;
				}

/*
                //do process
                //SockPack *pSock = (SockPack *)cRecvBuf;
                //printf(">> v(%d), c(%d), p(%d), m(%d), p(%d), f(%d)\n", pSock->hdr.version, pSock->hdr.cell, pSock->hdr.port, pSock->hdr.msg_no, pSock->hdr.packet, pSock->hdr.flag);

                rnd++;
                memset(cSendBuf, 0, sizeof(cSendBuf));
                sprintf(cSendBuf, "<%d,%d,%d,%d,%d,%d,", rnd%7, 1, 0, 0, 0, 0 );

                char cTemp[512];
                memset(cTemp, 'a', sizeof(cTemp));
                sprintf(cSendBuf, "%s%s>", cSendBuf, cTemp);
                printf(">> strlen = %d\n", strlen(cSendBuf));

                iCnt = pthis->Send(cSendBuf, sizeof(cSendBuf));
                if( iCnt < 0 )
                {
                    printf( "socket send() Error! errno=%d\n" , errno );
                    pthis->CloseClientSocket();
                    m_tPollEvent[1].fd = -1;
                    m_tPollEvent[1].events = 0;
                    m_tPollEvent[1].revents = 0;
                    continue;
                }
                else
                {
                    printf( "socket send() ok\n" );
                }
*/
            }
		}
		else
		{
			continue;
		}

		sleep(1);
		continue;

		//do process
		//printf(">> iCnt = %d\n", iCnt);
		//SockPack *pSock = (SockPack *)cRecvBuf;
		//printf(">> v(%d), c(%d), p(%d), m(%d), p(%d), f(%d)\n", pSock->hdr.version, pSock->hdr.cell, pSock->hdr.port, pSock->hdr.msg_no, pSock->hdr.packet, pSock->hdr.flag);
/*
		//memset( &cSendBuf, 0 , sizeof( cSendBuf ) );
		memcpy(cSendBuf, cRecvBuf, sizeof( cRecvBuf ));

		iCnt = pthis->Send(cSendBuf, sizeof(cSendBuf));
		if( iCnt < 0 )
		{
			printf( "socket send() Error! errno=%d\n" , errno );
			pthis->CloseSocket(pthis->iClientSocket);
			continue;
		}
		else
		{
			printf( "socket send() ok\n" );
		}
*/
		//pthis->CloseClientSocket();
		//printf( "socket() Shutdown done\n" );
	} // End of while( idThread )

	if(pthis->iClientSocket >= 0)
		pthis->CloseClientSocket();

	pthis->CloseServerSocket();
	return (void*)0;
}

int CSocketServer::ParseData(std::string &strBuf)
{
	std::string strData;
	std::size_t iStartMarkPos	= strBuf.find(SOCKET_START_MARK);
	std::size_t iEndMarkPos	= strBuf.find(SOCKET_END_MARK);

	if(iStartMarkPos == std::string::npos)
	{
		return -1;	//this is not that I want packet
	}
	else if(iEndMarkPos == std::string::npos)
	{
		return 0;	//wait until all packet arrive.
	}
	else if(iStartMarkPos < iEndMarkPos)
	{
		strData = strData = strBuf.substr(iStartMarkPos, iEndMarkPos-iStartMarkPos+1);

        //printf(">> start < end\n");
        //printf(">> sfound = %ld, efound = %ld\n", iStartMarkPos, iEndMarkPos);
        //printf(">> strBuf = %s\n", strData.c_str());
	}
	else
	{
		strData = strData = strBuf.substr(0, iEndMarkPos+1);

        //printf(">> start > end\n");
        //printf(">> sfound = %ld, efound = %ld\n", iStartMarkPos, iEndMarkPos);
        //printf(">> strBuf = %s\n", strData.c_str());
	}

	return iEndMarkPos+1;
}


void CSocketServer::StartThread()
{
	if(idThread == 0)
		pthread_create(&idThread, NULL, &SocketServerThread, (void*)this);
}

void CSocketServer::StopThread()
{
	condThread = OFF;

	if(idThread != 0)
	{
		pthread_join(idThread, NULL);
		idThread = 0;
	}
}
