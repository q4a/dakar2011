/****************************************************************
*                                                               *
*    Name: MyThread.h                                           *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __MYTHREAD_H__
#define __MYTHREAD_H__

#ifdef __linux__
#else
# include <windows.h>
#endif

class CMyThread
{
public:
    CMyThread();
    ~CMyThread();
    
    void lock();
    void unlock();
    void execute();
    
    void kill();
    
protected:
    virtual void run() = 0;
    void run_in();
#ifdef __linux__
#else
    static DWORD WINAPI ThreadEntry (void* pArg);
#endif

    volatile bool underTermination;
#ifdef __linux__
#else
    HANDLE handle;
    CRITICAL_SECTION critSection[2];
    volatile int currentCritSection;
    DWORD tid;
#endif
};

#endif // __MYTHREAD_H__
