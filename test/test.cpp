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
#include <unistd.h>
#include "def.h"
#include "base.h"
#include "msgsend.h"
#include "msgbox.h"
#include "socketserver.h"
#include "json/json.h"

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
int test_socket();
int test_json();

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
		case '7':	test_socket();	break;
        case '8':   test_json();    break;
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
	cout << "7: socket test" << endl;
    cout << "8: jsong test" << endl;
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
			msg.hdr.version = atoi(version);
			msg.hdr.cell = atoi(cell);
			msg.hdr.port = atoi(port);
			msg.hdr.msg_no = atoi(msg_no);
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
    int id_sem = CreateSem(KEY_BD_LOCK);
	RemoveSem(id_sem);

	LockSem(id_sem);
	UnlockSem(id_sem);

	return EXIT_SUCCESS;
}

int test_shmem()
{
    /*
	char buf[32];
    int id_shmem = CreateShmem(0x3000, sizeof(buf));

	memset(buf, 'a', sizeof(buf)-1);
    SetStringShmem(id_shmem, 0, 32, buf);
	printf(">> write data = %s\n", buf);

	memset(buf, 0, sizeof(buf));
    GetStringShmem(id_shmem, 0, 32, buf);
	printf(">> read data = %s\n", buf);
    */

    DpsStatus statDps[PORT_MAX];
    memset(statDps, 0, sizeof(statDps));

    int idDpsShmem = CreateShmem(KEY_DPS_SHARE, sizeof(statDps));
    int idDpsShmemLock = CreateSem(KEY_DPS_SHARE_LOCK);

    LockSem(idDpsShmemLock);
    statDps[PORT1].bDpsOcp = true;
    SetShmem(idDpsShmem, &statDps[PORT1], sizeof(statDps[PORT1]));
    UnlockSem(idDpsShmemLock);

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
			msg.hdr.version = atoi(version);
			msg.hdr.cell = atoi(cell);
			msg.hdr.port = atoi(port);
			msg.hdr.msg_no = atoi(msg_no);
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

int test_socket()
{
	CSocketServer *sock = new CSocketServer((char*)"127.0.0.1", PORT_TDM);

	sock->StartThread();

	while(1)
	{
        string version;
        string cell;
        string port;
        string text;

        cout << "version: ";
        cin >> version;
        if(version == "quit")
            break;

        cout << "cell: ";
        cin >> cell;
        if(cell == "quit")
            break;


        cout << "port: ";
        cin >> port;
        if(port == "quit")
            break;


        cout << "text: ";
        cin >> text;
        if(text == "quit")
            break;


        char buf[512] = {0,};
        sprintf(buf, "%s%s,%s,%s,%d,%d,%d,%s%s", SOCKET_START_MARK, version.c_str(), cell.c_str(), port.c_str(), 0, 0, 0, text.c_str(), SOCKET_END_MARK);

        printf(">> buf = %s\n", buf);
        int cnt = sock->Send(buf, strlen(buf));
        printf(">> cnt = %d\n", cnt);
	}

	sock->StopThread();
	delete (sock);


/*
	char buf[512] = {0,};
	SockPack *tmp = (SockPack *)buf;

	tmp->hdr.version = 1;
	tmp->hdr.cell = 2;
	tmp->hdr.port = 1;
	tmp->hdr.msg_no = 9;
	tmp->hdr.packet = 8;
	tmp->hdr.flag = 7;


	printf(">> line = %d\n", __LINE__);

	printf(">> tmp->string = %p\n", tmp->string);

	snprintf(tmp->string, SOCKET_STRING_LENGTH, "%s", "abc");
	printf(">> line = %d\n", __LINE__);

	for(int i=0; i<32; i++)
	{
		printf("%x ", buf[i]);
	}
	printf("\n");

	return 0;
	sock->CreateSocket();printf(">> line = %d\n", __LINE__);
	sock->ConnectServer();printf(">> line = %d\n", __LINE__);

	int cnt = sock->Send(buf, sizeof(buf));printf(">> line = %d\n", __LINE__);
	printf(">> cnt = %d\n", cnt);
	sock->CloseSocket();printf(">> line = %d\n", __LINE__);

	delete(sock);
*/
	return EXIT_SUCCESS;
}

int test_json()
{

    //cpu usage: top -n 1 | grep -i cpu\(s\) | awk '{print $5}' | tr -d "%id," | awk '{print 100-$1}'
    //mem usage: free | grep Mem | awk '{print $3/$2*100}'
    //storage usgae: df -h | grep [/]$ | awk '{print $5}'
    //date: date +"%Y-%m-%d %H:%m:%S %Z"

    Json::Value bdinfo;
    bdinfo["time"] = "2017-11-09 19:11:56 KST";
    bdinfo["mount"] = false;
    bdinfo["cpu"] = 12.3;
    bdinfo["mem"] = 30.3;
    bdinfo["stroage"] = "35.5%";


    Json::Value port1;
    port1["port"] = "1";
    port1["testmode"] = "SAS";
    port1["script"] = "script1.uts";
    port1["status"] = "running";

    Json::Value port2;
    port2["port"] = "2";
    port2["testmode"] = "SATA";
    port2["script"] = "script2.uts";
    port2["status"] = "stop";

    Json::Value portinfo;
    portinfo.append(port1);
    portinfo.append(port2);

    Json::Value root;
    root["bdinfo"] = bdinfo;
    root["portinfo"] = portinfo;

    Json::StyledWriter writer;
    string str = writer.write(root);
    cout << str << endl;


    Json::Reader reader;
    Json::Value root_read;
    bool parsingRet = reader.parse(str, root_read);

    if(!parsingRet)
    {
        cout << "fail parsing" << endl;
        return -1;
    }

    cout << "1\n";
    Json::Value read_bdinfo = root_read["bdinfo"];
    cout << "2\n";
    Json::Value read_portinfo = root_read["portinfo"];
    cout << "3\n";

    for (Json::Value::iterator it = root.begin(); it != root.end(); it++)
    {
        Json::Value key = it.key();
        Json::Value value = (*it);

        cout << "key: " << key.toStyledString();
        cout << "value: " << value.toStyledString();
    }

    return 0;
}
