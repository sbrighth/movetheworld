//============================================================================
// Name        : test.cpp
// Author      : shjeong
// Version     :
// Copyright   : exicon
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "def.h"
#include "base.h"
#include "msgsend.h"
#include "msgbox.h"

using namespace std;
int help_test();
int help_msg();
int test_msg();
int test_sem();
int test_shmem();
int test_mmap();
int test_compile_script();
int test_process();
int	test_msg_out();

int main(void) {
	int cond = 1;
	char cmd;

	cout << "<< test program >>" << endl << endl;
	do
	{
		help_test();
		cin >> cmd;
		cout << endl;

		switch(cmd)
		{
		case '1':	test_msg();		break;
		case '2':	test_sem();		break;
		case '3':	test_shmem();	break;
		case '4':	test_mmap();	break;
		case '5':   test_compile_script();
					test_process();	break;
		case '6':	test_msg_out();	break;
		case 'x':	cond = 0;		break;
		default:
			continue;
		}
	}
	while(cond);

	return EXIT_SUCCESS;
}

int help_test()
{
	cout << "--------------" << endl;
	cout << "1: MSGQ test" << endl;
	cout << "2: Semaphore test" << endl;
	cout << "3: Share memory test" << endl;
	cout << "4: memory map test" << endl;
	cout << "5: script test" << endl;
	cout << "6: write out msg(UI -> tdm)" << endl;
	cout << "x: exit       " << endl;
	cout << "--------------" << endl;
	cout << "select cmd >  ";

	return 0;
}

int test_msg()
{
	char cmd;
	char version[32];
	char cell[32];
	char port[32];
	char msg_no[32];
	char string[32];

	MsgPack msg;

	cout << "<< msg program >>" << endl << endl;

	int statMain = 1;
	int id_msg = CreateMsgq(KEY_TEST_MSGQ);

	while(statMain)
	{
		help_msg();
		cin >> cmd;
		cout << endl;

		if(cmd != 'x')
		{
			memset(version, 0, 32);
			memset(cell, 0, 32);
			memset(port, 0, 32);
			memset(msg_no, 0, 32);
			memset(string, 0, 32);

			cout << "version : "; cin >> version;
			cout << "cell    : "; cin >> cell;
			cout << "port    : "; cin >> port;
			cout << "msg_no  : "; cin >> msg_no;
			cout << "string  : "; cin >> string;

			memset(&msg, 0, sizeof(MsgPack));
			msg.version = atoi(version);
			msg.cell = atoi(cell);
			msg.port = atoi(port);
			msg.msg_no = atoi(msg_no);
			sprintf(msg.string, "%s", string);

			//msgq.type = TYPE_MSGQ_SEND;
			SendMsgPack(id_msg, msg);
		}
		else
		{
			statMain = 0;
		}
	}

	return EXIT_SUCCESS;
}


int help_msg()
{
	cout << "--------------" << endl;
	cout << "1: MSG send" << endl;
	cout << "x: exit       " << endl;
	cout << "--------------" << endl;
	cout << "select cmd >  ";

	return 0;
}

int test_sem()
{
	int id_sem = CreateSem(KEY_DPS_LOCK);
	RemoveSem(id_sem);

	LockSem(id_sem);
	UnlockSem(id_sem);

	return EXIT_SUCCESS;
}

int test_shmem()
{
	char buf[32];
	int id_shmem = CreateShmem(0x3000, sizeof(buf));

	memset(buf, 'a', sizeof(buf)-1);
	SetShmem(id_shmem, 0, 32, buf);
	printf(">> write data = %s\n", buf);

	memset(buf, 0, sizeof(buf));
	GetShmem(id_shmem, 0, 32, buf);
	printf(">> read data = %s\n", buf);

	//RemoveShmem(id_shmem);
	return EXIT_SUCCESS;
}

int test_mmap()
{
	char buf[32];
	char *mbuf = NULL;
	int fd_mmap = CreateMmap("/home/shjeong/mm", &mbuf, sizeof(buf));

	printf("mbuf[10] = %c\n", mbuf[10]);
	mbuf[10] += 1;
	int ret = RemoveMmap(fd_mmap, mbuf, sizeof(buf));
	printf(">> ret = %d\n", ret);
	return EXIT_SUCCESS;
}

int test_compile_script()
{
	int ret;
	string compiler = "g++";
	string script = "/tmp/exicon/script/sample.c";
	string target = "/tmp/exicon/script/sample";
	string incpath = "-I/tmp/exicon/include";
	string libpath = "-L/tmp/exicon/lib";
	string lib = "-ltbase -ltnet";

	pid_t pid;
	unlink(target.c_str());

	pid = fork();

	if(pid < 0)
	{
		perror("fork error");
		ret = -1;
	}
	else if(pid == 0)
	{
		printf("compile child process\n");
		ret = execlp(compiler.c_str(), compiler.c_str(), incpath.c_str(), "-o", target.c_str(), script.c_str(), libpath.c_str(), NULL);
	}
	else
	{
		wait(0);
		ret = pid;
	}

	printf("ret= %d\n", ret);
	return ret;
}

int test_process()
{
	int ret;
	string target = "/tmp/exicon/script/sample";
	size_t pos = target.rfind('/') + 1;
	char cell[8] = "1";
	char port[8] = "2";

	pid_t pid = fork();

	if(pid < 0)
	{
		perror("fork error");
		ret = -1;
	}
	else if(pid == 0)
	{
		printf("child process\n");
		ret = execl(target.c_str(), target.substr(pos).c_str(), cell, port, NULL);
	}
	else
	{
		wait(0);
		ret = pid;
	}

	printf("ret= %d\n", ret);
	return ret;
}

int run_prog(const char *fmt, ...)
{
	char buf[1024];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return 0;
}


int test_msg_out()
{
	char cmd;
	char version[32];
	char cell[32];
	char port[32];
	char msg_no[32];
	char string[32];
	int statMain = 1;

	MsgPack msg;
	CMsgBox msgbox("/tmp/exicon/athost/rack_001/tester002/exe/out", "/tmp/exicon/athost/rack_001/tester002/exe/temp");
	msgbox.StartThread();

	cout << "<< msg write program >>" << endl << endl;

	while(statMain)
	{
		help_msg();
		cin >> cmd;
		cout << endl;

		if(cmd != 'x')
		{
			memset(version, 0, 32);
			memset(cell, 0, 32);
			memset(port, 0, 32);
			memset(msg_no, 0, 32);
			memset(string, 0, 32);

			cout << "version : "; cin >> version;
			cout << "cell    : "; cin >> cell;
			cout << "port    : "; cin >> port;
			cout << "msg_no  : "; cin >> msg_no;
			cout << "string  : "; cin >> string;

			memset(&msg, 0, sizeof(MsgPack));
			msg.version = atoi(version);
			msg.cell = atoi(cell);
			msg.port = atoi(port);
			msg.msg_no = atoi(msg_no);
			sprintf(msg.string, "%s", string);

			msgbox.SendMsg(msg);
		}
		else
		{
			statMain = 0;
		}
	}

	return EXIT_SUCCESS;
}

