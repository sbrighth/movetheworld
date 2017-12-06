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
#include "statsocket.h"

CStatSocket::CStatSocket(har *szServerAddr, int iServerPort)
    : CSocketClient(szServerAddr, iServerPort)
{
}

CStatSocket::~CStatSocket() {
    // TODO Auto-generated destructor stub

    StopThread();
}

int CStatSocket::InitSockData()
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

static void *SocketSendThread( void *arg )
{
    CStatSocket *pthis = (CStatSocket *)arg;
    string strBuf;;

    pthis->condSendThread = ON;
    while( pthis->condSendThread == ON )                // Socket check Loop
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

        char cSendBuf[32] = {0,};
        sprintf(cSendBuf, "%s%d,%d,%d,%d,%d,%d,%s%s", SOCKET_START_MARK, 0, iCell, 0, 0, 0, 0, "", SOCKET_END_MARK );

        int iSendCnt = pthis->Send(cSendBuf, strlen(cSendBuf));
        if(iSendCnt < (ssize_t)strlen(cSendBuf))
        {
            //printf("connection error!!\n");
            pthis->bConnect = false;
            return -1;
        }
    } // End of while( idSendThread )

    pthis->CloseSocket();
    return (void*)0;
}

static void *SocketProcThread( void *arg )
{
    CMsgSocket *pthis = (CMsgSocket *)arg;

    pthis->condProcThread = ON;
    while( pthis->condProcThread == ON )                // Socket process Loop
    {
        if(pthis->qRecv.size() > 0)
        {
            SockPack sockData = pthis->qRecv.front();
            pthis->qRecv.pop();
            pthis->ProcFunc(sockData);

            if(sockData.pstring)
                delete [] sockData.pstring;
        }
        else
        {
            msleep(100);
        }
    }

    return (void*)0;
}

void CStatSocket::StartThread()
{
    InitSockData();

    if(idSendThread == 0)
        pthread_create(&idSendThread, NULL, &SocketSendThread, (void*)this);
}

void CStatSocket::StopThread()
{
    condSendThread = OFF;

    if(idSendThread != 0)
    {
        pthread_join(idSendThread, NULL);
        idSendThread = 0;
    }

    InitSockData();
}
