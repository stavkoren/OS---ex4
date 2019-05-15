//Stav Koren 207128539
#include "threadPool.h"
#include <stdio.h>

#define INSERT_TASK_SUCCEED 0
#define INSERT_TASK_FAILED -1
#define SUCCESS_VALUE 0
#define ERROR "Error in system call\n"
#define SHOULD_WAIT_VALUE 0
void freeMissions(ThreadPool* threadPool);
void writeErrorMessage(){
    write(fileno(stderr),ERROR,strlen(ERROR));
}
//Free memory of threads
void freeThreads(ThreadPool*threadPool){
    if (pthread_mutex_lock(threadPool->threadMutex) != SUCCESS_VALUE) {
        writeErrorMessage();
        osDestroyQueue(threadPool->threads);
        freeMissions(threadPool);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool->newMissionEnqueue);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    //clean all threads memory
    while (!osIsQueueEmpty(threadPool->threads)){
        if(pthread_cond_broadcast(threadPool->newMissionEnqueue)!=SUCCESS_VALUE){
            writeErrorMessage();
        }
        pthread_t *pthread=(pthread_t*)osDequeue(threadPool->threads);
        //if thread already running- wait
        pthread_join(*pthread,NULL);
        free(pthread);
    }
    if (pthread_mutex_unlock(threadPool->threadMutex) != SUCCESS_VALUE) {
        writeErrorMessage();
        osDestroyQueue(threadPool->threads);
        freeMissions(threadPool);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool->newMissionEnqueue);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
}

//free mission queue
void freeMissions(ThreadPool* threadPool){
    if (pthread_mutex_lock(threadPool->missionsMutex) !=SUCCESS_VALUE) {
        writeErrorMessage();
        freeThreads(threadPool);
        osDestroyQueue(threadPool->threads);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool->newMissionEnqueue);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    //clean all threads memory
    while (!osIsQueueEmpty(threadPool->missions)){
        Mission *mission=(Mission*)osDequeue(threadPool->missions);
        free(mission);
    }
    if (pthread_mutex_unlock(threadPool->missionsMutex) !=SUCCESS_VALUE) {
        writeErrorMessage();
        freeThreads(threadPool);
        osDestroyQueue(threadPool->threads);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool->newMissionEnqueue);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
}
//Free thread pool memory
void freeMemory(ThreadPool*threadPool){
    freeThreads(threadPool);
    osDestroyQueue(threadPool->threads);
    freeMissions(threadPool);
    osDestroyQueue(threadPool->missions);
    free(threadPool->missionsMutex);
    free(threadPool->threadMutex);
    free(threadPool->newMissionMutex);
    free(threadPool->newMissionEnqueue);
    free(threadPool);
}
//initialize data
ThreadPool* initializeThreadPool(){
    ThreadPool* threadPool=malloc(sizeof(ThreadPool));
    if(threadPool==NULL){
        writeErrorMessage();
        _exit(EXIT_FAILURE);
    }
    threadPool->threadMutex=malloc(sizeof(pthread_mutex_t));
    if(threadPool->threadMutex==NULL){
        writeErrorMessage();
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(threadPool->threadMutex,NULL)!=SUCCESS_VALUE){
        writeErrorMessage();
        free(threadPool->threadMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->newMissionMutex=malloc(sizeof(pthread_mutex_t));
    if(threadPool->newMissionMutex==NULL){
        writeErrorMessage();
        free(threadPool->threadMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(threadPool->newMissionMutex,NULL)!=SUCCESS_VALUE){
        writeErrorMessage();
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->missionsMutex=malloc(sizeof(pthread_mutex_t));
    if(threadPool->missionsMutex==NULL){
        writeErrorMessage();
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_mutex_init(threadPool->missionsMutex,NULL)!=SUCCESS_VALUE){
        writeErrorMessage();
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->missions=osCreateQueue();
    if(threadPool->missions==NULL){
        writeErrorMessage();
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->threads=osCreateQueue();
    if(threadPool->threads==NULL){
        writeErrorMessage();
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->newMissionEnqueue=malloc(sizeof(pthread_cond_t));
    if(threadPool->newMissionEnqueue==NULL){
        writeErrorMessage();
        osDestroyQueue(threadPool->threads);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_cond_init(threadPool->newMissionEnqueue,NULL)!=SUCCESS_VALUE){
        writeErrorMessage();
        free(threadPool->newMissionEnqueue);
        osDestroyQueue(threadPool->threads);
        osDestroyQueue(threadPool->missions);
        free(threadPool->missionsMutex);
        free(threadPool->threadMutex);
        free(threadPool->newMissionMutex);
        free(threadPool);
        _exit(EXIT_FAILURE);
    }
    threadPool->isDestroyed=false;
    threadPool->runAllMissionsAfterDistruction=false;
    return threadPool;
}

//wait until new mission enqueue
void waitForNewMission(ThreadPool* threadPool){
    //lock the mutex
    if(pthread_mutex_lock(threadPool->newMissionMutex)!=SUCCESS_VALUE){
        //lock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(threadPool->isDestroyed==false) {
        if (pthread_cond_wait(threadPool->newMissionEnqueue, threadPool->newMissionMutex) !=SUCCESS_VALUE) {
            writeErrorMessage();
            freeMemory(threadPool);
            _exit(EXIT_FAILURE);
        }
    }
    if(pthread_mutex_unlock(threadPool->newMissionMutex)!=SUCCESS_VALUE){
        //unlock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
}

//get atribute value of thread pool
bool getrunAllMissionsAfterDistruction(ThreadPool*threadPool){
    return threadPool->runAllMissionsAfterDistruction;
}

//get atribute value of thread pool
bool getIsMissionQueueEmpty(ThreadPool*threadPool){
    bool isEmpty=NULL;
    //lock the mutex
    if(pthread_mutex_lock(threadPool->missionsMutex)!=SUCCESS_VALUE){
        //lock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    isEmpty=osIsQueueEmpty(threadPool->missions);
    if(pthread_mutex_unlock(threadPool->missionsMutex)!=SUCCESS_VALUE){
        //unlock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    return isEmpty;
}

//Dequeue mission from mission queue
Mission* getMissionFromQueue(ThreadPool* threadPool) {
    //lock the mutex
    if (pthread_mutex_lock(threadPool->missionsMutex) != SUCCESS_VALUE) {
        //lock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    Mission *mission = osDequeue(threadPool->missions);
    //unlock mutex
    if (pthread_mutex_unlock(threadPool->missionsMutex) != SUCCESS_VALUE) {
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    return mission;
}
//Run mission if exist and free its memory
void runMission(Mission*mission){
    if(mission!=NULL) {
        mission->computeFunc(mission->parameters);
        //free memory
        free(mission);
    }
}

//threads running function- wait for missions and run them until destruction flag is true
void* threadRoutine(void* threadP){
    ThreadPool* threadPool=(ThreadPool*)threadP;
    while (threadPool->isDestroyed==false){
        //check if queue is empty
        if(getIsMissionQueueEmpty(threadPool)==true){
                waitForNewMission(threadPool);
        }
        runMission(getMissionFromQueue(threadPool));
    }
    if(getrunAllMissionsAfterDistruction(threadPool)==true){
        while (getIsMissionQueueEmpty(threadPool)==false){
            runMission(getMissionFromQueue(threadPool));
        }
    }
}

//initialize threads
void* initializeThreads(int numOfThreads,ThreadPool*threadPool){
    if (pthread_mutex_lock(threadPool->threadMutex) !=SUCCESS_VALUE) {
        writeErrorMessage();
        _exit(EXIT_FAILURE);
    }
    //create each thread
    int i=0;
    for(i;i<numOfThreads;i++){
        pthread_t *pthread=malloc(sizeof(pthread));
        if(pthread==NULL){
            writeErrorMessage();
            _exit(EXIT_FAILURE);
        }
        //insert thread into pid queue
        osEnqueue(threadPool->threads,pthread);
        //create thread
        if(pthread_create(pthread,NULL,threadRoutine,threadPool)!=SUCCESS_VALUE){
            writeErrorMessage();
            _exit(EXIT_FAILURE);
        }
    }
    if (pthread_mutex_unlock(threadPool->threadMutex) !=SUCCESS_VALUE) {
        writeErrorMessage();
        _exit(EXIT_FAILURE);
    }
    return NULL;
}
//Create thread pool
ThreadPool* tpCreate(int numOfThreads){
    ThreadPool*threadPool=initializeThreadPool();
    //inintialize
    initializeThreads(numOfThreads,threadPool);
    return threadPool;
}

//Create new mission
Mission* createMission(void (*computeFunc) (void *), void* param,ThreadPool* threadPool){
    Mission* mission= malloc(sizeof(mission));
    if(mission==NULL){
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    mission->computeFunc=computeFunc;
    mission->parameters=param;
    return mission;
}

//Send pthread_cond_signal when new mission enqueue
void newMissionMsg(ThreadPool* threadPool){
    //lock the mutex
    if(pthread_mutex_lock(threadPool->newMissionMutex)!=SUCCESS_VALUE){
        //lock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_cond_signal(threadPool->newMissionEnqueue)!=SUCCESS_VALUE){
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    if(pthread_mutex_unlock(threadPool->newMissionMutex)!=SUCCESS_VALUE){
        //unlock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
}

//Enqueue new mission
void insertMission(ThreadPool* threadPool,Mission*mission){
    //lock the mutex
    if (pthread_mutex_lock(threadPool->missionsMutex) !=SUCCESS_VALUE) {
        //lock failed
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    osEnqueue(threadPool->missions,mission);
    //unlock mutex
    if (pthread_mutex_unlock(threadPool->missionsMutex) !=SUCCESS_VALUE) {
        writeErrorMessage();
        freeMemory(threadPool);
        _exit(EXIT_FAILURE);
    }
    newMissionMsg(threadPool);
}

//Insert new task
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if(threadPool->isDestroyed==true){
        return INSERT_TASK_FAILED;
    }
    Mission*mission=createMission(computeFunc,param,threadPool);
    insertMission(threadPool,mission);
    return INSERT_TASK_SUCCEED;
}



//Destroy thread pool
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    if(threadPool->isDestroyed==true){
        return;
    }
    //finish only current running missions
       threadPool->isDestroyed=true;
    if(shouldWaitForTasks==SHOULD_WAIT_VALUE){
        threadPool->runAllMissionsAfterDistruction=false;
    }else{
        threadPool->runAllMissionsAfterDistruction=true;
    }
    freeMemory(threadPool);
}