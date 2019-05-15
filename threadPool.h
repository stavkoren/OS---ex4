//Stav Koren 207128539
#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#define _OPEN_THREADS
#include <sys/param.h>
#include <stdbool.h>
#include "osqueue.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
typedef struct  Mission
{
    void (*computeFunc)(void*);
    void *parameters;
}Mission;

typedef struct ThreadPool
{
    OSQueue* threads;
    OSQueue* missions;
    pthread_mutex_t* missionsMutex;
    pthread_mutex_t* threadMutex;
    pthread_mutex_t* newMissionMutex;
    pthread_cond_t* newMissionEnqueue;
    volatile bool isDestroyed;
    volatile bool runAllMissionsAfterDistruction;

}ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
