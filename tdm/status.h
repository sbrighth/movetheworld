/*
 * status.h
 *
 *  Created on: Oct 18, 2017
 *      Author: shjeong
 */

#ifndef STATUS_H_
#define STATUS_H_

#include "def.h"

typedef struct tTestStatus
{
    int mode;
    int run_script_cnt;
    char *script;
    int step;
    int status;
}TestStatus;

class CStatus {
public:
	CStatus();
	virtual ~CStatus();

public:
    bool    mount;
    float   cpu;
    float   ram;
    float   storage;
    char    time[32];

    int     test_mode[PORT_MAX];
    char    test_script[PORT_MAX][256];

    int     device_perf[PORT_MAX];
    int

};

#endif /* STATUS_H_ */
