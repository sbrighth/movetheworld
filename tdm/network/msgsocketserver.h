#ifndef CMSGSOCKETSERVER_H
#define CMSGSOCKETSERVER_H

#include <string>
#include <queue>
#include "pthread.h"
#include "socketserver.h"
#include "def.h"

using namespace std;

class CMsgSocketServer : public CSocketServer
{
public:
    CMsgSocketServer(char *szAddr, int iPort);
    virtual ~CMsgSocketServer();

public:
    int             InitSockData();
    void			StartThread(void (*SetFunc)(SockPack sockData));
    void			StopThread();
    int				StripMark(string strBuf, string &strData);
    int				DataSplit(string strData, SockPack &sockData);

public:
    void			(*ProcFunc)(SockPack sockData);

    int				condCheckThread;
    int             condProcThread;
    pthread_t		idCheckThread;
    pthread_t       idProcThread;

    queue<SockPack> qRecv;
    queue<SockPack> qSend;
};

#endif // CMSGSOCKETSERVER_H
