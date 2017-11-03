/*
 * socketclient.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: shjeong
 */

#include "socketclient.h"
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


CSocketClient::CSocketClient(int iCell, char *szServerAddr, int iServerPort) {
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

	this->iCell = iCell;
	this->iClientSocket = -1;
	this->bConnect = false;
}

CSocketClient::~CSocketClient() {
	// TODO Auto-generated destructor stub
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
		printf( "socket() ok\n" );
	}

	//set socket option (SO_REUSEADDR)
	if(setsockopt(iClientSocket , SOL_SOCKET , SO_REUSEADDR , &iSetSocketOption , sizeof(iSetSocketOption)) < 0 )
	{
		printf( "setsockopt() Error! errno=%d\n" , errno );
		return -1;
	}
	else
	{
		printf( "setsockopt() ok\n" );
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
		printf( "connect() Error! errno=%d\n" , errno );

		if(errno == EINPROGRESS)
		{
			struct timeval tv;
			tv.tv_sec = 30;
			tv.tv_usec = 0;

			fd_set myset;
			FD_ZERO(&myset);
			FD_SET(iClientSocket, &myset);

			if(select(iClientSocket+1, NULL, &myset, NULL, &tv) > 0)
			{
				int iGetSocketErr = 0;
				socklen_t len = sizeof(iGetSocketErr);

				getsockopt(iClientSocket , SOL_SOCKET , SO_ERROR , &iGetSocketErr , &len);
				if(iGetSocketErr)
				{
					printf("getsockopt connect() error %d\n", iGetSocketErr);
					bConnect = false;
				}
				else
				{
					printf("getsockopt connect() ok!\n");
					bConnect = true;
				}
			}
			else
			{
				printf("connect() timeout error\n");
				bConnect = false;
			}
		}
		else
		{
			printf("connect() error!!\n");
			bConnect = false;;
		}
	}
	else
	{
		printf( "connect() ok\n" );
		bConnect = true;
	}

	if(bConnect)
		return 0;
	else
		return -1;
}

void CSocketClient::CloseSocket()
{
	printf("close socket\n");

	if(iClientSocket >= 0)
	{
		shutdown( iClientSocket , SHUT_RDWR );
		close( iClientSocket );

		iClientSocket = -1;
		bConnect = false;
	}
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

int CSocketClient::SendCheckDummy()
{
	char cSendBuf[32] = {0,};
	sprintf(cSendBuf, "%s%d,%d,%d,%d,%d,%d,%s%s", SOCKET_START_MARK, 0, iCell, 0, 0, 0, 0, "", SOCKET_END_MARK );

	int iSendCnt = Send(cSendBuf, strlen(cSendBuf));
	if(iSendCnt < (ssize_t)strlen(cSendBuf))
	{
		printf("connection error!!\n");
		bConnect = false;
		return -1;
	}

	return 0;
}

void *SocketClientThread( void *arg )
{
	CSocketClient *pthis = (CSocketClient *)arg;
	std::string strBuf;;

	pthis->condThread = ON;
	while( pthis->condThread == ON )                // Socket receive Loop
	{
		if(pthis->CreateSocket() < 0)
		{
			pthis->CloseSocket();
			strBuf.erase();
			sleep(1);
			continue;
		}

		if(pthis->ConnectServer() < 0)
		{
			printf( "ConnectServer() Error!!! errno=%d\n", errno );
			pthis->CloseSocket();
			strBuf.erase();
			sleep(1);
			continue;
		}

		struct pollfd tPollEvent;
		tPollEvent.fd        = pthis->iClientSocket;
		tPollEvent.events    = POLLIN;
		tPollEvent.revents   = 0;

		if( poll((struct pollfd*)&tPollEvent, 1, 1000) > 0 )
		{
			if( tPollEvent.revents & POLLIN )
			{
				char cRecvBuf[SOCKET_BUF_SIZE];
				memset( cRecvBuf , 0 , sizeof( cRecvBuf ) );

				int iCnt = pthis->Recv(cRecvBuf, sizeof(cRecvBuf));
				if( iCnt <= 0 )
				{
					printf( "socket recv() Error! errno=%d\n" , errno );
					pthis->SendCheckDummy();
				}

				strBuf.append(cRecvBuf, (strlen(cRecvBuf) > (size_t)iCnt)? iCnt : strlen(cRecvBuf));

				printf(">> recv count = %ld, iCnt = %d\n", strlen(cRecvBuf), iCnt);
				printf(">> before erase strBuf = %s\n", strBuf.c_str());

				while(iCnt--)
				{
					std::string strPacketData;

					int ret = pthis->ParseData(strBuf, strPacketData);
					if(ret > 0)
					{
						strBuf.erase(0, ret);
						printf(">> erase pos = %d\n", ret);
						printf(">> after erase strBuf = %s\n", strBuf.c_str());
						printf(">> strData = %s\n", strPacketData.c_str());


					}
					else
					{
						if(strBuf.size() > SOCKET_MAX_BUF_SIZE)
						{
							std::size_t iEndMarkPos = strBuf.find(SOCKET_END_MARK);

							if(iEndMarkPos == std::string::npos)
								strBuf.erase();
							else
								strBuf.erase(0, iEndMarkPos+1);
						}
						break;
					}
				}
			}
		}
		else
		{
			pthis->SendCheckDummy();
		}
	} // End of while( idThread )

	pthis->CloseSocket();
	return (void*)0;
}

int CSocketClient::ParseData(std::string strBuf, std::string &strData)
{
	std::size_t iStartMarkPos = strBuf.find(SOCKET_START_MARK);
	std::size_t iEndMarkPos = strBuf.find(SOCKET_END_MARK);

	if(iStartMarkPos == std::string::npos)
	{
		return -1;	//this is not what I want packet
	}
	else if(iEndMarkPos == std::string::npos)
	{
		return 0;	//wait until all packet arrive.
	}
	else if(iStartMarkPos < iEndMarkPos)
	{
		strData = strBuf.substr(iStartMarkPos, iEndMarkPos-iStartMarkPos+1);
	}
	else
	{
		strData = strBuf.substr(0, iEndMarkPos+1);
	}

	return iEndMarkPos+1;
}

int CSocketClient::DataSplit(std::string strData, tSockPack2 &sockData)
{
	string *strResult = new string[6;

	return 0;
}

void CSocketClient::StartThread()
{
	if(idThread == 0)
		pthread_create(&idThread, NULL, &SocketClientThread, (void*)this);
}

void CSocketClient::StopThread()
{
	condThread = OFF;

	if(idThread != 0)
	{
		pthread_join(idThread, NULL);
		idThread = 0;
	}
}
