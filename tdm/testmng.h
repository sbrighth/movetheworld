/*
 * testmng.h
 *
 *  Created on: Oct 9, 2017
 *      Author: shjeong
 */

#ifndef TESTMNG_H_
#define TESTMNG_H_

#include <pthread.h>
#include <iostream>
#include "def.h"

using namespace std;


#define TEST_SCRIPT_ORI_EXT		"sct"
#define	TEST_SCRIPT_RUN_EXT		"c"

class CTestMng
{
public:
	CTestMng(int iCell, int iPort);
	virtual ~CTestMng();

public:
	int				StartTest(string strPath, string strFileName);
	int				StopTest();
	int				IsTestOn();

private:
	int				CheckScriptExt(string strFileName);

private:
	int				condThread;
	pthread_t		idThread;
	int				iCell;
	int				iPort;

	string			strScriptProcName;
};

#endif /* TESTMNG_H_ */
