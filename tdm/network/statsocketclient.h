#ifndef CSTATSOCKETCLIENT_H
#define CSTATSOCKETCLIENT_H

#include <string>
#include <queue>
#include "pthread.h"
#include "socketclient.h"
#include "def.h"

using namespace std;

class CStatSocketClient : public CSocketClient
{
public:
    CStatSocketClient(int iCell, char *szServerAddr, int iServerPort);
    ~CStatSocketClient();

public:
    int             InitSockData();
    void			StartThread(void (*SetFunc)(SockPack sockData));
    void			StopThread();
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
};
#endif // CSTATSOCKETCLIENT_H
