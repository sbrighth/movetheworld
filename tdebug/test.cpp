//============================================================================
// Name        : test.cpp
// Author      : shjeong
// Version     :
// Copyright   : exicon
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include "function.h"

using namespace std;


int help_test()
{
    cout << "--------------" << endl;
    cout << "1: MSGQ test" << endl;
    cout << "2: Semaphore test" << endl;
    cout << "3: Share memory test" << endl;
    cout << "4: memory map test" << endl;
    cout << "5: script test" << endl;
    cout << "6: write out msg(UI -> tdm)" << endl;
    cout << "7: socket test server" << endl;
    cout << "8: socket test client" << endl;
    cout << "9: socket test json" << endl;
    cout << "x: exit       " << endl;
    cout << "--------------" << endl;
    cout << "select cmd >  ";

    return 0;
}

int main(void) {
	int cond = 1;
	char cmd;
    CTestFunc func;

	cout << "<< test program >>" << endl << endl;
	do
	{
		help_test();
		cin >> cmd;
		cout << endl;

		switch(cmd)
		{
        case '1':	func.test_msg();                break;
        case '2':	func.test_sem();                break;
        case '3':	func.test_shmem();              break;
        case '4':	func.test_mmap();               break;
        case '5':   func.test_compile_script();
                    func.test_process();            break;
        case '6':	func.test_msg_out();            break;
        case '7':	func.test_socket_server();      break;
        case '8':	func.test_socket_client();      break;
        case '9':	func.test_json();               break;
        case 'x':	cond = 0;                       break;
		default:
			continue;
		}
	}
	while(cond);

    return 0;
}
