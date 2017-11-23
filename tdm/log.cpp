#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "log.h"
#include "base.h"

CLog::CLog()
{
}

int CLog::AddLogFile(string strSourceFile, string strTargetFile)
{
    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if(temp.strSource == strSourceFile)
        {
            printf("%s is already addded\n", strSourceFile.c_str());
            return -1;
        }
    }

    CLogPath logPath(strSourceFile, strTargetFile);
    listLogFile.push_back(logPath);
/*
    int fd, wd;

    fd = inotify_init();
    if(fd < 0)
    {
        printf("%s inotify_init() error\n");
        return -1;
    }

    wd = inotify_add_watch(fd, strSourceFile.c_str(), IN_MODIFY)
*/

    return 0;
}

int CLog::DelLogFile(string strSourceFile)
{
    int iRet = -1;

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

    return iRet;
}

int CLog::CopyLogFile()
{
    for(list<CLogPath>::iterator it = listLogFile.begin(); it != listLogFile.end(); it++)
    {
        CLogPath temp = *it;
        if(IsFileExist(temp.strSource.c_str()) == true)
            CopyFile(temp.strTarget.c_str(), temp.strSource.c_str());
    }

    return 0;
}
