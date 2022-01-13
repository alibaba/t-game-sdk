#pragma once

#include <stdint.h>

typedef volatile unsigned char atomic_t;
typedef atomic_t 	sem_t;

#define __ATOMIC_SEQ_CST 5

inline int sem_init(sem_t *sem, int pshared, unsigned value) {return 0;}
inline int sem_destroy( sem_t * sem ) {return 0;}
inline int sem_wait( sem_t * sem ) {return 0;}
inline int sem_post( sem_t * sem ) {return 0;}

inline int __atomic_add_fetch(int*, int, int) {return 0;}
inline int __atomic_sub_fetch(int*, int, int) {return 0;}
inline int __atomic_compare_exchange(volatile int64_t*, int64_t*, int64_t*, bool, int, int) {return 0;}
inline int __atomic_compare_exchange(volatile int32_t*, int32_t*, int32_t*, bool, int, int) {return 0;}
inline int __atomic_thread_fence(int) {return 0;}
