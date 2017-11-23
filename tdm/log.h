#ifndef LOG_H
#define LOG_H

#include <list>
#include <string>
#include "status.h"

using namespace std;

class CLogPath
{
public:
    CLogPath(string source, string target):strSource(source),strTarget(target){ };

    string strSource;
    string strTarget;
};
/*
class CDescriptorNotify
{
public:
    CDescriptorNotify(int file, int whach):file(file),watch(watch){ };

    int file;
    int watch;
};
*/
class CLog
{
public:
    CLog();

    int     AddLogFile(string strSourceFile, string strTargetFile);
    int     DelLogFile(string strSourceFile);
    bool    CheckShareMount();
    int     CopyLogFile();

public:
    list<CLogPath>          listLogFile;
    //list<CDescriptorNotify> listDescriptor;

};

#endif // LOG_H
