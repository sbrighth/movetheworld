#ifndef LOG_H
#define LOG_H

#include <list>
#include <string>
#include <pthread.h>

using namespace std;

class CLogPath
{
public:
    CLogPath(string source, string folder):strSource(source),strFolder(folder){ };

    string strSource;
    string strFolder;
};

class CLog
{
public:
    CLog();
    ~CLog();

    int     AddLogFileList(string strSourceFile, string strTargetFolder);
    int     DelLogFileList(string strSourceFile);
    int     CopyLogFile();
    int     CopyLogFile(string strSourceFile);

public:
    list<CLogPath>      listLogFile;
    pthread_mutex_t     mutexLog;
};

#endif // LOG_H
