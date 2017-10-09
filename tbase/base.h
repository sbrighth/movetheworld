/*
 * base.h
 *
 *  Created on: Sep 19, 2017
 *      Author: shjeong
 */

#ifndef BASE_H_
#define BASE_H_

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

//message queue
int			CreateMsgq(int key);
int			RemoveMsgq(int id);
int			CurNumMsgq(int id);
int			PushMsgq(int id, void *msg, int size);
int			PopMsgq(int id, void *msg, int size);

//semaphore
int			CreateSem(int key);
int			RemoveSem(int id);
int			LockSem(int id);
int			UnlockSem(int id);

//share memory
int			CreateShmem(int key, int size);
int			RemoveShmem(int id);
int			GetShmem(int id, int offset, int size, char *data);
int			SetShmem(int id, int offset, int size, char *data);

//memory map
int			CreateMmap(const char *filename, char **pmmap, int size);
int			RemoveMmap(int fd, char *pmmap, int size);

//time
int			StartTimer(int index);
int			ReadTimer(int index, double *p);
int			TimeToString(struct tm *t, char* cData);
void		msleep(int);

//file
int			IsFileExist(const char *filename);
long		GetFileSize(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* BASE_H_ */
