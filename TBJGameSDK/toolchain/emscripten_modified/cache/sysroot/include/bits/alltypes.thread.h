#ifndef __JENOVA_DEFINED_pthread_t
#define __JENOVA_DEFINED_pthread_t

#define __DEFINED_pthread_attr_t
#define __DEFINED_pthread_mutex_t
#define __DEFINED_mtx_t
#define __DEFINED_pthread_cond_t
#define __DEFINED_cnd_t
#define __DEFINED_pthread_rwlock_t
#define __DEFINED_pthread_barrier_t
#define __DEFINED_pthread_t
#define __DEFINED_pthread_once_t
#define __DEFINED_pthread_spinlock_t
#define __DEFINED_pthread_mutexattr_t
#define __DEFINED_pthread_condattr_t
#define __DEFINED_pthread_barrierattr_t
#define __DEFINED_pthread_rwlockattr_t

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { union { int __i[11]; volatile int __vi[11]; unsigned __s[11]; } __u; char _padding[20]; } pthread_attr_t;
typedef struct { union { int __i[7]; volatile int __vi[7]; volatile void *__p[7]; } __u; char _padding[20]; } pthread_mutex_t;
typedef struct { union { int __i[7]; volatile int __vi[7]; volatile void *__p[7]; } __u; char _padding[20]; } mtx_t;
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; } pthread_cond_t;
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; } cnd_t;
typedef struct { union { int __i[8]; volatile int __vi[8]; void *__p[8]; } __u; char _padding[168]; } pthread_rwlock_t;
typedef struct { union { int __i[5]; volatile int __vi[5]; void *__p[5]; } __u; } pthread_barrier_t;

typedef struct pthread_t
{
#ifdef __cplusplus
    pthread_t() { _id = 0; }
    pthread_t(int id) { _id = id; }
    pthread_t(unsigned long long id) { _id = id; }
#endif
    unsigned long long _id;
    unsigned long long __pading[2];
} pthread_t;


typedef struct pthread_once_t {
    char __pading[16];
} pthread_once_t;

typedef struct pthread_spinlock_t {
    char __pading[200];
} pthread_spinlock_t;

typedef struct pthread_mutexattr_t {
    unsigned int __attr;
    char __pading[12];
} pthread_mutexattr_t;

typedef struct { unsigned __attr; char __pading[12]; } pthread_condattr_t;
typedef struct { unsigned __attr; char __pading[12]; } pthread_barrierattr_t;
typedef struct { unsigned __attr[2]; char __pading[16]; } pthread_rwlockattr_t;


int pthread_equal(pthread_t, pthread_t);

#ifdef __cplusplus

static inline bool operator==(pthread_t left, pthread_t right)
{
    return pthread_equal(left, right);
}

static inline bool operator!=(const pthread_t& left, const pthread_t& right)
{
    return !pthread_equal(left, right);
}

static inline bool operator<(const pthread_t& left, const pthread_t& right)
{
    return left._id < right._id;
}

static inline bool operator>(const pthread_t& left, const pthread_t& right)
{
    return left._id > right._id;
}
#endif

#ifdef __cplusplus
}
#endif

#ifdef JENOVA_USE_NATIVE_PRINT
#define printf printf_threadsafe
#endif

#ifdef __cplusplus
extern "C" {
#endif

//int __atomic_load_tbj(int *ptr, int memorder);
//void __c11_atomic_thread_fence_tbj(int __order);
//
//#define __c11_atomic_load __c11_atomic_load_tbj
//#define __c11_atomic_store __c11_atomic_store_tbj
//#define __c11_atomic_init __c11_atomic_init_tbj
//#define __c11_atomic_exchange __c11_atomic_exchange_tbj
//#define __c11_atomic_thread_fence __c11_atomic_thread_fence_tbj
//#define __c11_atomic_signal_fence __c11_atomic_signal_fence_tbj
//#define __c11_atomic_is_lock_free __c11_atomic_is_lock_free_tbj
//#define __c11_atomic_compare_exchange_strong __c11_atomic_compare_exchange_strong_tbj
//#define __c11_atomic_compare_exchange_weak __c11_atomic_compare_exchange_weak_tbj
//#define __c11_atomic_fetch_add __c11_atomic_fetch_add_tbj
//#define __c11_atomic_fetch_sub __c11_atomic_fetch_sub_tbj
//#define __c11_atomic_fetch_or __c11_atomic_fetch_or_tbj
//#define __c11_atomic_fetch_xor __c11_atomic_fetch_xor_tbj
//
//
//#define __atomic_load __atomic_load_tbj
//#define __atomic_load_n __atomic_load_n_tbj
//#define __atomic_store __atomic_store_tbj
//#define __atomic_store_n __atomic_store_n_tbj
//#define __atomic_exchange __atomic_exchange_tbj
//#define __atomic_exchange_n __atomic_exchange_n_tbj
//#define __atomic_compare_exchange __atomic_compare_exchange_tbj
//#define __atomic_compare_exchange_n __atomic_compare_exchange_n_tbj
//#define __atomic_add_fetch __atomic_add_fetch_tbj
//#define __atomic_sub_fetch __atomic_sub_fetch_tbj
//#define __atomic_and_fetch __atomic_and_fetch_tbj
//#define __atomic_xor_fetch __atomic_xor_fetch_tbj
//#define __atomic_or_fetch __atomic_or_fetch_tbj
//#define __atomic_nand_fetch __atomic_nand_fetch_tbj
//#define __atomic_fetch_add __atomic_fetch_add_tbj
//#define __atomic_fetch_sub __atomic_fetch_sub_tbj
//#define __atomic_fetch_and __atomic_fetch_and_tbj
//#define __atomic_fetch_xor __atomic_fetch_xor_tbj
//#define __atomic_fetch_or __atomic_fetch_or_tbj
//#define __atomic_fetch_nand __atomic_fetch_nand_tbj
//#define __atomic_test_and_set __atomic_test_and_set_tbj
//#define __atomic_clear __atomic_clear_tbj
//#define __atomic_thread_fence __atomic_thread_fence_tbj
//#define __atomic_signal_fence __atomic_signal_fence_tbj
//#define __atomic_always_lock_free __atomic_always_lock_free_tbj
//#define __atomic_is_lock_free __atomic_is_lock_free_tbj
//
//#define __c11_atomic_load __c11_atomic_load_tbj
//#define __c11_atomic_store __c11_atomic_store_tbj
//#define __c11_atomic_init __c11_atomic_init_tbj
//#define __c11_atomic_exchange __c11_atomic_exchange_tbj
//#define __c11_atomic_thread_fence __c11_atomic_thread_fence_tbj
//#define __c11_atomic_signal_fence __c11_atomic_signal_fence_tbj
//#define __c11_atomic_is_lock_free __c11_atomic_is_lock_free_tbj
//#define __c11_atomic_compare_exchange_strong __c11_atomic_compare_exchange_strong_tbj
//#define __c11_atomic_compare_exchange_weak __c11_atomic_compare_exchange_weak_tbj
//#define __c11_atomic_fetch_add __c11_atomic_fetch_add_tbj
//#define __c11_atomic_fetch_sub __c11_atomic_fetch_sub_tbj
//#define __c11_atomic_fetch_or __c11_atomic_fetch_or_tbj
//#define __c11_atomic_fetch_xor __c11_atomic_fetch_xor_tbj

#ifdef __cplusplus
}
#endif

#endif