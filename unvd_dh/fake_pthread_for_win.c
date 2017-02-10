#include "fake_pthread_for_win.h"

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t* attr)
{
    InitializeCriticalSection(mutex);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (CloseHandle(*cond))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
    *cond = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == *cond)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    if (SetEvent(*cond))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *abstime)
{
    DWORD sleeptime = abstime->tv_sec * 1000 + abstime->tv_nsec / 1000;
    DWORD temp = WaitForSingleObject(*cond, sleeptime);
    return 0;
}
