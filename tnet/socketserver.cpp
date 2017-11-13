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
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include "base.h"

#include <iostream>
using namespace std;
CSocketServer::CSocketServer(char *szAddr, int iPort) {
	// TODO Auto-generated constructor stub

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

int CSocketServer::InitSockData()
{
    while(!qRecv.empty())
    {
        SockPack sockData = qRecv.front();
        qRecv.pop();

        if(sockData.pstring)
            delete sockData.pstring;
    }

    while(!qSend.empty())
    {
        SockPack sockData = qSend.front();
        qSend.pop();

        if(sockData.pstring)
            delete sockData.pstring;
    }

    return 0;
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

static void *SocketCheckThread( void *arg )
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

    pthis->condCheckThread = ON;
    while( pthis->condCheckThread == ON )                // Socket check Loop
	{
		if( poll((struct pollfd*)&m_tPollEvent, 2, 1000) > 0 )
		{
			if( m_tPollEvent[0].revents & POLLIN )
			{
                m_tPollEvent[0].revents = 0;            //reset return result

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

                    m_tPollEvent[1].fd = pthis->iClientSocket;
					m_tPollEvent[1].events = POLLIN;
					m_tPollEvent[1].revents = 0; 
				}

                strBuf.erase();
                continue;
			}

			if( m_tPollEvent[1].revents & POLLIN )
			{
                m_tPollEvent[1].revents = 0;    //reset return event

				char cRecvBuf[4096];
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

                while(iCnt--)
                {
                    string strPacketData;

                    int ret = pthis->StripMark(strBuf, strPacketData);
                    if(ret > 0)
                    {
                        strBuf.erase(0, ret);

                        SockPack sockData;
                        pthis->DataSplit(strPacketData, sockData);
                        pthis->qRecv.push(sockData);
                    }
                    else
                    {
                        if(strBuf.size() > SOCKET_MAX_BUF_SIZE)
                        {
                            size_t iEndMarkPos = strBuf.find(SOCKET_END_MARK);

                            if(iEndMarkPos == string::npos)
                                strBuf.erase();
                            else
                                strBuf.erase(0, iEndMarkPos+1);
                        }
                        break;
                    }
                }
            }
		}
	} // End of while( idThread )

	if(pthis->iClientSocket >= 0)
		pthis->CloseClientSocket();

	pthis->CloseServerSocket();
	return (void*)0;
}

static void *SocketProcThread( void *arg )
{
    CSocketServer *pthis = (CSocketServer *)arg;

    pthis->condProcThread = ON;
    while( pthis->condProcThread == ON )                // Socket process Loop
    {
        if(pthis->qRecv.size() > 0)
        {
            SockPack sockData = pthis->qRecv.front();
            pthis->qRecv.pop();
            pthis->ProcFunc(sockData);

            if(sockData.pstring)
                delete sockData.pstring;
        }
        else
        {
            msleep(100);
        }
    }

    return (void*)0;
}

int CSocketServer::StripMark(string strBuf, string &strData)
{
    size_t iStartMarkPos = strBuf.find(SOCKET_START_MARK);
    size_t iEndMarkPos = strBuf.find(SOCKET_END_MARK);

    if(iStartMarkPos == string::npos)
    {
        return -1;	//this is not what I want packet
    }
    else if(iEndMarkPos == string::npos)
    {
        return 0;	//wait until all packet arrive.
    }
    else if(iStartMarkPos < iEndMarkPos)
    {
        strData = strBuf.substr(iStartMarkPos+1, iEndMarkPos-iStartMarkPos-1);
    }
    else
    {
        strData = strBuf.substr(1, iEndMarkPos-1);
    }

    return iEndMarkPos+1;
}

int CSocketServer::DataSplit(string strData, SockPack &sockData)
{
    vector<string> vectData;
    size_t iFindPos = 0;
    int iMarkCnt = 0;
    int iDataCnt = 0;

    if(strData.length() == string::npos)
        return -1;

    for(size_t iPos=0; iPos<strData.size(); )
    {
        iFindPos = strData.find_first_of(SOCKET_SPLIT_MARK, iPos);

        if(iFindPos == string::npos || iMarkCnt == SOCKET_MARK_CNT)
        {
            //get last data
            vectData.push_back(strData.substr(iPos));
            iPos = strData.size();
            iDataCnt++;
        }
        else
        {
            //get chopped data
            vectData.push_back(strData.substr(iPos, iFindPos-iPos));
            iPos = iFindPos+1;
            iMarkCnt++;
            iDataCnt++;
        }
    }

    if(iMarkCnt != SOCKET_MARK_CNT)
        return -2;

    sockData.hdr.version    = atoi(((string)vectData.at(0)).c_str());
    sockData.hdr.cell       = atoi(((string)vectData.at(1)).c_str());
    sockData.hdr.port       = atoi(((string)vectData.at(2)).c_str());
    sockData.hdr.msg_no     = atoi(((string)vectData.at(3)).c_str());
    sockData.hdr.packet     = atoi(((string)vectData.at(4)).c_str());
    sockData.hdr.flag       = atoi(((string)vectData.at(5)).c_str());


    if(iDataCnt == SOCKET_DATA_CNT)
    {
        int iStringLength = ((string)vectData.at(6)).length()+1;
        sockData.pstring = new char[iStringLength];
        memcpy(sockData.pstring, ((string)vectData.at(6)).c_str(), iStringLength);
    }
    else
    {
        sockData.pstring = new char[MSG_STRING_LENGTH];
        memset(sockData.pstring, 0, MSG_STRING_LENGTH);
    }

    return 0;
}

void CSocketServer::StartThread(void (*SetFunc)(SockPack sockData))
{
    ProcFunc = SetFunc;
    InitSockData();

    if(idCheckThread == 0)
        pthread_create(&idCheckThread, NULL, &SocketCheckThread, (void*)this);

    if(idProcThread == 0)
        pthread_create(&idProcThread, NULL, &SocketProcThread, (void*)this);
}

void CSocketServer::StopThread()
{
    condCheckThread = OFF;
    condProcThread = OFF;

    if(idProcThread != 0)
    {
        pthread_join(idProcThread, NULL);
        idProcThread = 0;
    }

    if(idCheckThread != 0)
    {
        pthread_join(idCheckThread, NULL);
        idCheckThread = 0;
    }

    InitSockData();
}
