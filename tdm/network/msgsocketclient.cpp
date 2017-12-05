#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include "base.h"
#include "msgsocketclient.h"

CMsgSocketClient::CMsgSocketClient(int iCell, char *szServerAddr, int iServerPort)
    : CSocketClient(szServerAddr, iServerPort)
{
    this->iCell = iCell;
    this->bUseThread = false;
}

CMsgSocketClient::~CMsgSocketClient() {
    // TODO Auto-generated destructor stub

    StopThread();
}

int CMsgSocketClient::InitSockData()
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

int CMsgSocketClient::SendCheckDummy()
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
    CMsgSocketClient *pthis = (CMsgSocketClient *)arg;
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
                    if(errno == EAGAIN || errno == EWOULDBLOCK)
                        continue;

                    //printf( "socket recv() Error! errno=%d\n" , errno );
                    pthis->CloseSocket();
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
    CMsgSocketClient *pthis = (CMsgSocketClient *)arg;

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

int CMsgSocketClient::StripMark(string strBuf, string &strData)
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

int CMsgSocketClient::DataSplit(string strData, SockPack &sockData)
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
/*
    cout << "version = " << sockData.hdr.version << endl;
    cout << "cell = " << sockData.hdr.cell<< endl;
    cout << "port = " << sockData.hdr.port<< endl;
    cout << "msg_no = " << sockData.hdr.msg_no<< endl;
    cout << "packet = " << sockData.hdr.packet<< endl;
    cout << "flag = " << sockData.hdr.flag << endl;
    cout << "pstring = " << sockData.pstring<< endl;
*/
    return 0;
}

void CMsgSocketClient::StartThread(void (*SetFunc)(SockPack sockData))
{
    ProcFunc = SetFunc;
    InitSockData();

    if(idCheckThread == 0)
        pthread_create(&idCheckThread, NULL, &SocketCheckThread, (void*)this);

    if(idProcThread == 0)
        pthread_create(&idProcThread, NULL, &SocketProcThread, (void*)this);
}

void CMsgSocketClient::StopThread()
{
    condCheckThread = OFF;
    condProcThread = OFF;
    bReqStop = true;

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
