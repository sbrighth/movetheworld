#ifndef CMSGSOCKET_H
#define CMSGSOCKET_H

#include <string>
#include <queue>
#include "pthread.h"
#include "def.h"
#include "socketclient.h"
#include "socketserver.h"

using namespace std;

class CMsgSocket
{
public:
    CMsgSocket(int iCell, char *szServerAddr, int iServerPort, bool bServer);
    ~CMsgSocket();

public:
    int             InitSockData();
    void			StartThread(void (*SetFunc)(SockPack sockData));
    void			StopThread();
    int				SendCheckDummy();
    int				StripMark(string strBuf, string &strData);
    int				DataSplit(string strData, SockPack &sockData);

public:
    bool            bServer;
    int             iCell;
    void			(*ProcFunc)(SockPack sockData);

    int				condCheckThread;
    int             condProcThread;
    pthread_t		idCheckThread;
    pthread_t       idProcThread;

    queue<SockPack> qRecv;
    queue<SockPack> qSend;

    CSocketClient   *sockClient;
    CSocketServer   *sockServer;
};

#endif // CMSGSOCKET_H
