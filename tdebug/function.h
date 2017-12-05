#ifndef FUNCTION_H
#define FUNCTION_H

#include "def.h"

using namespace std;

class CTestFunc
{
public:
    CTestFunc();

    int make_msg_packet(MsgPack &msg);
    int make_sock_packet(SockPack &msg);

    int run_prog(const char *fmt, ...);
    int help_msg();
    int test_msg();
    int test_sem();
    int test_shmem();
    int test_mmap();
    int test_compile_script();
    int test_process();
    int	test_msg_out();
    int test_socket_server();
    int test_socket_client();
    int test_json();
};

#endif // FUNCTION_H
