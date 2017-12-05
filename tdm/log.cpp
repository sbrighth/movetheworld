#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "log.h"
#include "base.h"

CLog::CLog()
{
    pthread_mutex_init(&mutexLog, NULL);
}

CLog::~CLog()
{
    pthread_mutex_destroy(&mutexLog);
}

int CLog::AddLogFileList(string strSourceFile, string strTargetFolder)
{
    int iListIdx = 0;
    pthread_mutex_lock(&mutexLog);

    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if(temp.strSource == strSourceFile)
        {
            printf("%s is already addded\n", strSourceFile.c_str());
            pthread_mutex_unlock(&mutexLog);
            return -1;
        }
    }

    CLogPath logPath(strSourceFile, strTargetFolder);
    listLogFile.push_back(logPath);
    iListIdx = listLogFile.size() - 1;

    pthread_mutex_unlock(&mutexLog);
    return iListIdx;
}

int CLog::DelLogFileList(string strSourceFile)
{
    int iRet = -1;
    pthread_mutex_lock(&mutexLog);

    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if(temp.strSource == strSourceFile)
        {
            listLogFile.erase(it);
            iRet = 0;
            break;
        }
    }

    pthread_mutex_unlock(&mutexLog);
    return iRet;
}

int CLog::CopyLogFile()
{
    pthread_mutex_lock(&mutexLog);

    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if(IsFileExist(temp.strSource.c_str()) == true)
            CopyFile(temp.strFolder.c_str(), temp.strSource.c_str());
    }

    pthread_mutex_unlock(&mutexLog);

    return 0;
}

int CLog::CopyLogFile(string strSourceFile)
{
    pthread_mutex_lock(&mutexLog);

    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if((IsFileExist(temp.strSource.c_str()) == true) && (temp.strSource == strSourceFile))
            CopyFile(temp.strFolder.c_str(), temp.strSource.c_str());
    }

    pthread_mutex_unlock(&mutexLog);

    return 0;
}
