#ifndef CSTATSOCKET_H
#define CSTATSOCKET_H

#include <string>
#include <queue>
#include "pthread.h"
#include "socketclient.h"
#include "def.h"

class CStatSocket : public CSocketClient
{
public:
    CStatSocket(char *szServerAddr, int iServerPort);
    virtual ~CStatSocket();

public:
    void			StartThread();
    void			StopThread();
    int             SendData(StatData sockData);
    int             InitData();

public:
    int				condSendThread;
    pthread_t		idSendThread;

    std::queue<StatData> qRecv;
    std::queue<StatData> qSend;
};

#endif // CSTATSOCKET_H
