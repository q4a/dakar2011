/****************************************************************
*                                                               *
*    Name: MyThread.cpp                                         *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#include "MyThread.h"
#include <stdio.h>

#ifdef MY_DEBUG
#define dprintf(x) x
#else
#define dprintf(x)
#endif

#ifdef MY_PDEBUG
#define pdprintf(x) x
#else
#define pdprintf(x)
#endif

#ifdef __linux__
void* CMyThread::ThreadEntry(void* arg)
{
    CMyThread* myThread = (CMyThread*)arg;
    myThread->run_in();
    pthread_exit(0);
}
#else
DWORD WINAPI CMyThread::ThreadEntry (void* pArg)
{
    CMyThread* myThread = (CMyThread*)pArg;
    myThread->run_in();
    return 0;
}
#endif

CMyThread::CMyThread()
    : underTermination(false)
#ifdef __linux__
//      , currentMutex(0)
#else
      , tid(0)
//      , currentCritSection(0)
#endif
{
#ifdef __linux__
//    pthread_mutex_init(&mutex[0], 0);
//    pthread_mutex_init(&mutex[1], 0);
    pthread_mutex_init(&mutex, 0);
//    pthread_spin_init(&spinLock, 1);
#else
//    InitializeCriticalSection(&critSection[0]);
//    InitializeCriticalSection(&critSection[1]);
#endif
#ifdef __linux__
    lock(); // if have critSession use it also at WIndows code
    int rc = pthread_create(&thread, 0, ThreadEntry, this);
    dprintf(printf("create thread: rc %d thread %p\n", rc, &thread);)
//    if (rc)
//    {
//    }
#else
    handle = CreateThread(
                0, // Sec attr
                0, // stack size
                ThreadEntry,
                this,
                0 /*CREATE_SUSPENDED*/, // use suspend if critSession is used
                &tid);
    dprintf(printf("create thread: handle %p, tid %d\n", handle, tid);)
    /*
    // use resume if critSession is used
    if (handle)
    {
        ResumeThread(handle);
    }
    */
#endif
}

CMyThread::~CMyThread()
{
    kill();
#ifdef __linux__
//    pthread_mutex_destroy(&mutex[0]);
//    pthread_mutex_destroy(&mutex[1]);
    pthread_mutex_destroy(&mutex);
//    pthread_spin_destroy(&spinLock);
#else
//    DeleteCriticalSection(&critSection[0]);
//    DeleteCriticalSection(&critSection[1]);
    if (handle)
    {
        CloseHandle(handle);
        handle = 0;
    }
#endif
}

void CMyThread::kill()
{
#ifdef __linux__
    dprintf(printf("thread terminate: %p\n", &thread);)
#else
    dprintf(printf("thread terminate: handle %p, tid %d\n", handle, tid);)
#endif
    underTermination = true;
    unlock();
#ifdef __linux__
#else
    if (handle)
    {
        WaitForSingleObject(handle, 2000);
    }
#endif
}

void CMyThread::run_in()
{
#ifdef __linux__
    dprintf(printf("thread entering into the loop: %p\n", &thread);)
#else
    dprintf(printf("thread entering into the loop: handle %p, tid %d\n", handle, tid);)
#endif
    while (!underTermination)
    {
//    printf("remove this two line\n");
//    }
//    {
        lock();
#ifdef __linux__
//        pthread_mutex_unlock(&mutex[1-currentMutex]);
#else
//        LeaveCriticalSection(&critSection[1 - currentCritSection]);
#endif
     //   if (underTermination) break;
#ifdef __linux__
        dprintf(printf("thread call run: %p\n", &thread);)
#else
        dprintf(printf("thread call run: handle %p, tid %d\n", handle, tid);)
#endif
        run();
#ifdef __linux__
        dprintf(printf("thread run returned: %p\n", &thread);)
#else
        dprintf(printf("thread run returned: handle %p, tid %d\n", handle, tid);)
#endif
    }
#ifdef __linux__
    dprintf(printf("thread leave the loop will terminate: %p\n", &thread);)
#else
    dprintf(printf("thread leave the loop will terminate: handle %p, tid %d\n", handle, tid);)
#endif
}

void CMyThread::lock()
{
#ifdef __linux__
//    dprintf(printf("lock %d\n", currentMutex);)
//    pthread_mutex_lock(&mutex[currentMutex]);
    dprintf(printf("lock\n");)
    pthread_mutex_lock(&mutex);
//    dprintf(printf("lock\n");)
//    pthread_spin_lock(&spinLock);
#else
//    dprintf(printf("lock %d\n", currentCritSection);)
//    EnterCriticalSection(&critSection[currentCritSection]);
    dprintf(printf("suspend %p %d\n", handle, tid);)
    SuspendThread(handle);
#endif
}

void CMyThread::unlock()
{
#ifdef __linux__
//    dprintf(printf("unlock %d\n", currentMutex);)
//    currentMutex = 1 - currentMutex;
//    pthread_mutex_unlock(&mutex[1 - currentMutex]);
//    pthread_mutex_lock(&mutex[currentMutex]);
    dprintf(printf("unlock\n");)
    pthread_mutex_unlock(&mutex);
//    dprintf(printf("unlock\n");)
//    pthread_spin_unlock(&spinLock);
#else
//    dprintf(printf("unlock %d\n", currentCritSection);)
//    currentCritSection = 1 - currentCritSection;
//    LeaveCriticalSection(&critSection[1 - currentCritSection]);
//    EnterCriticalSection(&critSection[currentCritSection]);
    dprintf(printf("resume %p %d\n", handle, tid);)
    ResumeThread(handle);
#endif
}

void CMyThread::execute()
{
    unlock();
}
