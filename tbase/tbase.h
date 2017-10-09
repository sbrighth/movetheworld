/*
 * tbase.h
 *
 *  Created on: Sep 19, 2017
 *      Author: shjeong
 */

#ifndef TBASE_H_
#define TBASE_H_

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

//message queue
extern int			CreateMsgq(int key);
extern int			RemoveMsgq(int id);
extern int			CurNumMsgq(int id);
extern int			PushMsgq(int id, void *msg, int size);
extern int			PopMsgq(int id, void *msg, int size);

//semaphore
extern int			CreateSem(int key);
extern int			RemoveSem(int id);
extern int			LockSem(int id);
extern int			UnlockSem(int id);

//share memory
extern int			CreateShmem(int key, int size);
extern int			RemoveShmem(int id);
extern int			GetShmem(int id, int offset, int size, char *data);
extern int			SetShmem(int id, int offset, int size, char *data);

//memory map
extern int			CreateMmap(const char *filename, char **pmmap, int size);
extern int			RemoveMmap(int fd, char *pmmap, int size);

//time
extern int			StartTimer(int index);
extern int			ReadTimer(int index, double *p);
extern int			TimeToString(struct tm *t, char* cData);
extern void		msleep(int);

//file
extern int			IsFileExist(const char *filename);
extern long		GetFileSize(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* TBASE_H_ */
