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
# include <pthread.h>
#else
# include <windows.h>
#endif

class CMyLock
{
public:
    CMyLock()
    {
#ifdef __linux__
//        pthread_spin_init(&spinLock, 1);
        pthread_mutex_init(&mutex, 0);
#else
        InitializeCriticalSection(&critSection);
#endif
    }
    ~CMyLock()
    {
#ifdef __linux__
//        pthread_spin_destroy(&spinLock);
        pthread_mutex_destroy(&mutex);
#else
        DeleteCriticalSection(&critSection);
#endif
    }
    
    void lock()
    {
#ifdef __linux__
//        pthread_spin_lock(&spinLock);
        pthread_mutex_lock(&mutex);
#else
        EnterCriticalSection(&critSection);
#endif
    }
    
    void unlock()
    {
#ifdef __linux__
//        pthread_spin_unlock(&spinLock);
        pthread_mutex_unlock(&mutex);
#else
        LeaveCriticalSection(&critSection);
#endif
    }
    
private:
#ifdef __linux__
//    pthread_spinlock_t spinLock;
    pthread_mutex_t mutex;
#else
    CRITICAL_SECTION critSection;
#endif

};

#endif // __MYLOCK_H__
