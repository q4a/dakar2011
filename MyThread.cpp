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
#else
DWORD WINAPI CMyThread::ThreadEntry (void* pArg)
{
    CMyThread* myThread = (CMyThread*)pArg;
    myThread->run_in();
    return 0;
}
#endif

CMyThread::CMyThread() : underTermination(false), tid(0), currentCritSection(0)
{
#ifdef __linux__
#else
    handle = CreateThread(
                0, // Sec attr
                0, // stack size
                ThreadEntry,
                this,
                CREATE_SUSPENDED,
                &tid);
    InitializeCriticalSection(&critSection[0]);
    InitializeCriticalSection(&critSection[1]);
#endif
    lock();
    dprintf(printf("create thread: handle %p, tid %d\n", handle, tid);)
    if (handle)
    {
        ResumeThread(handle);
    }
}

CMyThread::~CMyThread()
{
    kill();
#ifdef __linux__
#else
    DeleteCriticalSection(&critSection[0]);
    DeleteCriticalSection(&critSection[1]);
    if (handle)
    {
        CloseHandle(handle);
        handle = 0;
    }
#endif
}

void CMyThread::kill()
{
    dprintf(printf("thread terminate: handle %p, tid %d\n", handle, tid);)
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
    dprintf(printf("thread entering into the loop: handle %p, tid %d\n", handle, tid);)
    while (!underTermination)
    {
        lock();
#ifdef __linux__
#else
        LeaveCriticalSection(&critSection[1 - currentCritSection]);
#endif
        if (underTermination) break;
        dprintf(printf("thread call run: handle %p, tid %d\n", handle, tid);)
        run();
        dprintf(printf("thread run returned: handle %p, tid %d\n", handle, tid);)
    }
    dprintf(printf("thread leave the loop will terminate: handle %p, tid %d\n", handle, tid);)
}

void CMyThread::lock()
{
#ifdef __linux__
#else
    dprintf(printf("lock %d\n", currentCritSection);)
    EnterCriticalSection(&critSection[currentCritSection]);
#endif
}

void CMyThread::unlock()
{
#ifdef __linux__
#else
    dprintf(printf("unlock %d\n", currentCritSection);)
    currentCritSection = 1 - currentCritSection;
    LeaveCriticalSection(&critSection[1 - currentCritSection]);
    EnterCriticalSection(&critSection[currentCritSection]);
#endif
}

void CMyThread::execute()
{
    unlock();
}
