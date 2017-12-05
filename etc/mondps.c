#include <signal.h>
#include "DuoBdLib.h"

int g_condCheck;
int SetSignal();
void ProcSignalStop(int sig_no);
int CheckProgRunning();
char szProgName[] = "mondps";

int main(int argc, char **argv)
{
	int cnt = CheckProgRunning();
	OpenPort(BOTH_PORT, 115200);

	int iPortIdx	= 0;
	int iChIdx		= 0;
	iPortIdx=PORT2;
	//for(iPortIdx=PORT_MIN; iPortIdx<PORT_CNT; iPortIdx++)		
	{
		if(IsConnected(iPortIdx) == false)
		{
			printf("PORT%d is not connected!\n", iPortIdx+1);
			return -1;
		}
	}

return 0;

	DpsStatu statDps[MAX_DPS_CNT];
    int idDpsShmem		= CreateShmem(KEY_DPS_SHARE, sizeof(statDps));
    int idDpsShmemLock	= CreateSem(KEY_DPS_SHARE_LOCK);

	double time;
	g_condCheck = 1;
	while(g_condCheck == 1)
	{
		memset(&statDps, 0, sizeof(DpsStatus));

		//for(iPortIdx=PORT_MIN; iPortIdx<PORT_CNT; iPortIdx++)		
		StartTimer(0);
		{
			for(iChIdx=DPS_CH_MIN; iChIdx<DPS_CH_CNT; iChIdx++)
			{
				GetVolt(iPortIdx, iChIdx, &statDps[iPortIdx].dVoltage[iChIdx]);
				GetCurrent(iPortIdx, iChIdx, &statDps[iPortIdx].dCurrent[iChIdx]);
			}
		}

		ReadTimer(0, &time);
		printf(">> time = %f\n", time);

		LockSem(idDpsShmemLock);
		SetShmem(idDpsShmem, &statDps, sizeof(statDps));
		UnlockSem(idDpsShmemLock);

		printf(">> 5V = %f\n", statDps[iPortIdx].dVoltage[0]);
		printf(">> 12V = %f\n", statDps[iPortIdx].dVoltage[1]);
		msleep(500);
	}

	ClosePort();

	return 0;
}

int SetSignal()
{
	sigset_t    tSigSet;
	sigemptyset( &tSigSet );
	sigaddset( &tSigSet, SIGIO );
	pthread_sigmask( SIG_SETMASK , &tSigSet , NULL );

	struct sigaction tAct;
	memset( &tAct , 0 , sizeof( struct sigaction ) );
	tAct.sa_handler = ProcSignalStop;
	tAct.sa_flags   |= SA_RESTART;

	//Ctrl+C interrupt signal
	if( sigaction( SIGINT , &tAct , 0 ) != 0 )
		return-1;

	//kill signal
	if( sigaction( SIGTERM , &tAct , 0 ) != 0 )
		return-1;

	return 0;
}

void ProcSignalStop(int sig_no)
{
	g_condCheck = 0;
}

int CheckProgRunning()
{
	FILE *file;
	char cmd[32] = {0,};
	char buf[32] = {0,};

    sprintf(cmd, "ps -e | grep %s | wc -l", szProgName);
	file = popen(cmd, "r");
	if(file == NULL)
		return -1;

	fgets(buf, sizeof(buf), file);
	pclose(file);

	return atoi(buf);
}
