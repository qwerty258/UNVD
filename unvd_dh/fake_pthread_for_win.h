#ifndef _FAKE_PTHREAD_H_
#define _FAKE_PTHREAD_H_

#include <Windows.h>

struct timespec
{
    time_t tv_sec;  // whole seconds(valid values are >= 0)
    long tv_nsec;   // nanoseconds(valid values are[0, 999999999])
};

typedef HANDLE pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;

typedef int pthread_mutexattr_t;
typedef int pthread_condattr_t;

int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t* attr);

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);

int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, struct timespec *abstime);

#endif