#ifndef SCRIPT_H
#define SCRIPT_H

#endif // SCRIPT_H

//IDMSGQ
//MSGVER
//TPC_ID
//TPC_PORT
int _idMsgq;
char buf[10240];

void TEST_INIT(argc, argv)
{
    _idMsgq = CreateMsgq(KEY_TEST_MSGQ);
}

int MSG_TEXT1(char *string)
{
    if(string == NULL)
        return -1;

    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT1,	0, string);
}

int MSG_TEXT3(char *string)
{
    if(string == NULL)
        return -1;

    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT3,	0, string);
}

int MSG_TEXT4(char *string)
{
    if(string == NULL)
        return -1;

    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT4,	0, string);
}

int MSG_STEP(int cur, int max)
{
    char string[20];
    sprintf(string, "%d/%d", cur, max);
    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT2,	0, string);
}

int TEST_PASS()
{
    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_PASS,	0, "");
    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT1,	0, "pass");
}

int TEST_FAIL()
{
    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_FAIL,	0, "");
    SendMsg(_idMsgq, MSGVER, TPC_ID, TPC_PORT, MSG_TEXT1,	0, "fail");
}

int PRINT(char *string)
{
    if(string == NULL)
        return -1;

    Print(TPC_PORT, string);
}
