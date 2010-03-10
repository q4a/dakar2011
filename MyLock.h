/****************************************************************
*                                                               *
*    Name: MyLock.h                                             *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*                                                               *
****************************************************************/

#ifndef __MYLOCK_H__
#define __MYLOCK_H__

#ifdef __linux__
#else
# include <windows.h>
#endif

class CMyLock
{
public:
    CMyLock()
    {
#ifdef __linux__
#else
        InitializeCriticalSection(&critSection);
#endif
    }
    ~CMyLock()
    {
#ifdef __linux__
#else
        DeleteCriticalSection(&critSection);
#endif
    }
    
    void lock()
    {
#ifdef __linux__
#else
        EnterCriticalSection(&critSection);
#endif
    }
    
    void unlock()
    {
#ifdef __linux__
#else
        LeaveCriticalSection(&critSection);
#endif
    }
    
private:
#ifdef __linux__
#else
    CRITICAL_SECTION critSection;
#endif

};

#endif // __MYLOCK_H__
