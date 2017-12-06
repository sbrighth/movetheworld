#ifndef CSTATSOCKET_H
#define CSTATSOCKET_H

#include <string>
#include <queue>
#include "pthread.h"
#include "socketclient.h"

class CStatSocket : public CSocketClient
{
public:
    CStatSocket(char *szServerAddr, int iServerPort);
    virtual ~CStatSocket();

public:
    void			StartThread();
    void			StopThread();
    int             SendData();
    int             InitSockData();

public:
    int				condSendThread;
    pthread_t		idSendThread;
};

#endif // CSTATSOCKET_H
