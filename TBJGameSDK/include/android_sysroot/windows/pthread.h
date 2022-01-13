#pragma once

#include <stdint.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define SCHED_FIFO		1
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_MUTEX_RECURSIVE 0

struct sched_param { 
    int32_t  sched_priority; 
    int32_t  sched_curpriority; 
    /*union { 
        int32_t  reserved[8]; 
        struct {    
            int32_t  __ss_low_priority;  
            int32_t  __ss_max_repl;  
            struct timespec     __ss_repl_period;   
            struct timespec     __ss_init_budget;   
        }           __ss;   
    }           __ss_un;    */
};

typedef uint64_t pthread_t;
typedef uint64_t pthread_attr_t;
typedef uint64_t pthread_mutexattr_t;
typedef uint64_t pthread_mutex_t;

inline int sched_get_priority_max( int policy ) {return 0;}
inline int sched_get_priority_min( int policy ) {return 0;}
inline int pthread_join( pthread_t thread, void** value_ptr ) {return 0;}
inline pthread_t pthread_self( void ) {return 0;}
inline int pthread_attr_init( pthread_attr_t *attr ) {return 0;}
inline int pthread_attr_setstacksize( pthread_attr_t * attr, size_t stacksize ) {return 0;}
inline int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate ) {return 0;}
inline int pthread_create( pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void* ), void* arg ) {return 0;}
inline int pthread_attr_destroy( pthread_attr_t * attr ) {return 0;}
inline int pthread_setschedparam( pthread_t thread,  int policy, const struct sched_param *param ) {return 0;}
inline int pthread_mutexattr_init( pthread_mutexattr_t* attr ) {return 0;}
inline int pthread_mutexattr_destroy( pthread_mutexattr_t* attr ) {return 0;}
inline int pthread_mutexattr_settype(pthread_mutexattr_t * attr, int type ) {return 0;}
inline int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr) {return 0;}
inline int pthread_mutex_lock(pthread_mutex_t* mutex) {return 0;}
inline int pthread_mutex_unlock(pthread_mutex_t* mutex) {return 0;}
inline int pthread_mutex_destroy(pthread_mutex_t* mutex) {return 0;}
inline int usleep( uint32_t useconds ) {::Sleep(useconds / 1000); return 0;}
