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
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <queue>
#include "base.h"

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

    StopThread();
}

int CSocketClient::InitSockData()
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
       // printf( "connect() Error! errno=%d\n" , errno );

        if(errno == EINPROGRESS)
		{
			struct timeval tv;
            tv.tv_sec = 10;
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
                    //printf("getsockopt connect() error %d\n", iGetSocketErr);
					bConnect = false;
				}
				else
				{
                    //printf("getsockopt connect() ok!\n");
                    bConnect = true;
				}
			}
			else
			{
                //printf("connect() timeout error\n");
				bConnect = false;
			}
		}
		else
		{
            //printf("connect() error!!\n");
			bConnect = false;;
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
        //printf("connection error!!\n");
		bConnect = false;
		return -1;
    }

	return 0;
}

static void *SocketCheckThread( void *arg )
{
	CSocketClient *pthis = (CSocketClient *)arg;
    string strBuf;;

    pthis->condCheckThread = ON;
    while( pthis->condCheckThread == ON )                // Socket check Loop
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
            //printf( "ConnectServer() Error!!! errno=%d\n", errno );
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
                tPollEvent.revents = 0;     //reset clinet return event

				char cRecvBuf[SOCKET_BUF_SIZE];
				memset( cRecvBuf , 0 , sizeof( cRecvBuf ) );

				int iCnt = pthis->Recv(cRecvBuf, sizeof(cRecvBuf));
				if( iCnt <= 0 )
				{
					printf( "socket recv() Error! errno=%d\n" , errno );
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
		else
		{
			pthis->SendCheckDummy();
		}
    } // End of while( idCheckThread )

    pthis->CloseSocket();
	return (void*)0;
}

static void *SocketProcThread( void *arg )
{
    CSocketClient *pthis = (CSocketClient *)arg;

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

int CSocketClient::StripMark(string strBuf, string &strData)
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

int CSocketClient::DataSplit(string strData, SockPack &sockData)
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

void CSocketClient::StartThread(void (*SetFunc)(SockPack sockData))
{
    ProcFunc = SetFunc;
    InitSockData();

    if(idCheckThread == 0)
        pthread_create(&idCheckThread, NULL, &SocketCheckThread, (void*)this);

    if(idProcThread == 0)
        pthread_create(&idProcThread, NULL, &SocketProcThread, (void*)this);
}

void CSocketClient::StopThread()
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
