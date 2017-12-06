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

CStatSocket::CStatSocket(char *szServerAddr, int iServerPort)
    : CSocketClient(szServerAddr, iServerPort)
{

}

CStatSocket::~CStatSocket() {
    // TODO Auto-generated destructor stub

    StopThread();
}

int CStatSocket::SendData(StatData statData)
{
    qSend.push(statData);

    printf(">> StatSocket SendData\n");
    printf(">> queue cnt = %zu\n", qSend.size());
    //consider queue count limit...
    return 0;
}

int CStatSocket::InitData()
{
    while(!qRecv.empty())
    {
        qRecv.front();
        qRecv.pop();
    }

    while(!qSend.empty())
    {
        qSend.front();
        qSend.pop();
    }

    return 0;
}

static void *SocketSendThread( void *arg )
{
    CStatSocket *pthis = (CStatSocket *)arg;
    char cSendBuf[sizeof(StatData)+1] = {0,};

    pthis->condSendThread = ON;
    while( pthis->condSendThread == ON )                // Socket check Loop
    {
        if(pthis->CreateSocket() < 0)
        {
            pthis->CloseSocket();
            sleep(1);
            continue;
        }

        if(pthis->ConnectServer() < 0)
        {
            //printf( "ConnectServer() Error!!! errno=%d\n", errno );
            pthis->CloseSocket();
            sleep(1);
            continue;
        }

        if(pthis->qRecv.size() > 0)
        {
            StatData statData = pthis->qRecv.front();
            pthis->qRecv.pop();
            memcpy(cSendBuf, &statData, sizeof(statData));

            int iSendCnt = pthis->Send(cSendBuf, sizeof(cSendBuf));
            if(iSendCnt < (ssize_t)strlen(cSendBuf))
            {
                //printf("connection error!!\n");
                pthis->bConnect = false;
            }
        }
        else
        {
            msleep(100);
        }
    } // End of while( idSendThread )

    pthis->CloseSocket();
    return (void*)0;
}

void CStatSocket::StartThread()
{
    InitData();

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

    InitData();
}
