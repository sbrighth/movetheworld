#ifndef CMSGSOCKETCLIENT_H
#define CMSGSOCKETCLIENT_H

#include <string>
#include <queue>
#include "pthread.h"
#include "socketclient.h"
#include "def.h"

using namespace std;

class CMsgSocketClient : public CSocketClient
{
public:
    CMsgSocketClient(int iCell, char *szServerAddr, int iServerPort);
    ~CMsgSocketClient();

public:
    int             InitSockData();
    void			StartThread(void (*SetFunc)(SockPack sockData));
    void			StopThread();
    int				SendCheckDummy();
    int				StripMark(string strBuf, string &strData);
    int				DataSplit(string strData, SockPack &sockData);

public:
    int				iCell;
    void			(*ProcFunc)(SockPack sockData);

    bool            bUseThread;
    int				condCheckThread;
    int             condProcThread;
    pthread_t		idCheckThread;
    pthread_t       idProcThread;

    queue<SockPack> qRecv;
    queue<SockPack> qSend;
};

#endif // CMSGSOCKETCLIENT_H
