/*
 * tbase.cpp
 *
 *  Created on: Sep 19, 2017
 *      Author: shjeong
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "def.h"
#include "tbase.h"

#define MAX_RESERVED_INDEX		(10)
struct timeval 	g_tStartTime[ MAX_RESERVED_INDEX ];

#ifdef __cplusplus
extern "C" {
#endif

int CreateMsgq(int key)
{
	int id = msgget(key, IPC_CREAT|0666);
	return id;
}

int RemoveMsgq(int id)
{
	if(id < 0)
		return -1;

	if(msgctl(id, IPC_RMID, 0) < 0)
		return -2;

	return 0;
}

int CurNumMsgq(int id)
{
	if(id < 0)
		return -1;

	msqid_ds buf;
	if(msgctl(id, IPC_STAT, &buf ) < 0)
		return -2;

	return buf.msg_qnum;
}

int PushMsgq(int id, void *msg, int size)
{
	if(id < 0)
		return -1;

	if(size <= (int)sizeof(long))
		return -2;

	if(msgsnd(id, msg, size-sizeof(long), 0) < 0)
		return -3;

	return 0;
}

int PopMsgq(int id, void *msg, int size)
{
	if(id < 0)
		return -1;

	if(size <= (int)sizeof(long))
		return -2;

	long msg_type = *(long*)msg;

	if(msgrcv(id, msg, size-sizeof(long), msg_type, IPC_NOWAIT) < 0)
		return -3;

	return 0;
}

int CreateSem(int key)
{
	//create semaphore
	int id = semget(key, 1, 0666|IPC_CREAT|IPC_EXCL);
	if(id < 0)
	{
		//get existing semaphore
		if(errno == EEXIST)
		{
			id = semget(key, 0, 0);
		}
		else
			return -1;
	}
	else
	{
		union semun
		{
			int					val;
			struct semid_ds		*buf;
			unsigned short int	*array;
		} sem_union;

		 sem_union.val = 1;

		 //init semaphore
		 if(semctl(id, 0, SETVAL, sem_union) < 0)
			 return -2;
	}

	return id;
}

int RemoveSem(int id)
{
	if(id < 0)
		return -1;

	if(semctl(id, 0, IPC_RMID, 0) < 0)
		return -2;

	return 0;
}

int LockSem(int id)
{
	if(id < 0)
		return -1;

	struct sembuf buf = {0, -1, SEM_UNDO};
	if(semop(id, &buf, 1) < 0)
		return -2;

	return 0;
}

int UnlockSem(int id)
{
	if(id < 0)
		return -1;

	struct sembuf buf = {0, 1, SEM_UNDO};

	if(semop(id, &buf, 1) < 0)
		return -2;

	return 0;
}

int CreateShmem(int key, int size)
{
	if( size < 0 )
		return -1;

	int id = shmget(key, size, 0666|IPC_CREAT|IPC_EXCL);
	if( id < 0 )
	{
		if( errno == EEXIST )
			id = shmget(key, 0, 0);
		else
			return -2;
	}

	return id;
}


int RemoveShmem(int id)
{
	if(id < 0)
		return -1;

	if(shmctl( id, IPC_RMID, 0 ) < 0)
		return -2;

	return 0;
}


int GetShmem(int id, int offset, int size, char* data)
{
	if(id < 0)
		return -1;

	if(offset < 0)
		return -2;

	void* pShmAddr = NULL;
	pShmAddr = shmat( id, (void *)0, 0666|IPC_CREAT );

	if( pShmAddr == NULL || pShmAddr == (void*)-1 )
		return -3;

	memcpy(data, (char *)pShmAddr+offset, size);
	shmdt(pShmAddr);

	return 0;
}


int SetShmem(int id, int offset, int size, char* data)
{
	if( id < 0 )
		return -1;

	if( offset < 0 )
		return -2;

	void* pShmAddr;
	pShmAddr = shmat( id, (void *)0, 0666|IPC_CREAT );

	if( pShmAddr == NULL || pShmAddr == (void*)-1 )
		return -3;

	memcpy( (char *)pShmAddr+offset, (char*)data, size );
	shmdt(pShmAddr);

	return 0;
}

//return fd!!!
int CreateMmap(const char *filename, char **pmmap, int size)
{
	int fd = open(filename, O_RDWR|O_CREAT, 0666);
	if(fd < 0)
		return -1;

	if(size < 0)
		return -2;

	ftruncate(fd, size);

	char *temp_mmap = (char *)mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(temp_mmap == MAP_FAILED)
	{
		*pmmap = NULL;
		return -3;
	}
	else
		*pmmap = temp_mmap;

	return fd;
}

int RemoveMmap(int fd, char *pmmap, int size)
{
	if(fd < 0)
		return -1;

	if(munmap(pmmap, size) < 0)
		return -2;

	close(fd);
	return 0;
}

int StartTimer( int index )
{
	if( index < 0 || index > MAX_RESERVED_INDEX-1 )
		return -1;

	gettimeofday( &g_tStartTime[index], NULL );

	return 0;
}


int ReadTimer( int index, double *p )
{
	timeval tNowTime;
	double	t1, t2;

	if( index < 0 || index > MAX_RESERVED_INDEX-1 )
		return -1;

	if( g_tStartTime[index].tv_sec == 0 )
		return -2;

	gettimeofday( &tNowTime , NULL );
	t1 = g_tStartTime[index].tv_sec \
			+ (double)g_tStartTime[index].tv_usec*1e-6;
	t2 = tNowTime.tv_sec + (double)tNowTime.tv_usec*1e-6;

	if( p != NULL )
		*p =  t2 - t1;

	return 0;
}

int TimeToString(struct tm *t, char *cData)
{
	if(t == NULL || cData == NULL)
		return -1;

    sprintf(cData, "%04d-%02d-%02d %02d:%02d:%02d",
              t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
              t->tm_hour, t->tm_min, t->tm_sec);

    return 0;
}

void msleep(int time)
{
	usleep(time * 1000);
}

int IsFileExist(const char *filename)
{
	if(access(filename, F_OK) == 0)
		return 1;
	else
		return 0;
}

long GetFileSize(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if(file == NULL)
		return -1;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	fclose(file);
	return size;
}

#ifdef __cplusplus
}
#endif
