#ifndef __JENOVA_DEFINED_pthread_t
#define __JENOVA_DEFINED_pthread_t

#define thread_local "WebAssembly does not support thread_local!!! Do not use it!!"

#define __DEFINED_pthread_attr_t
#define __DEFINED_pthread_mutex_t
#define __DEFINED_mtx_t
#define __DEFINED_pthread_cond_t
#define __DEFINED_cnd_t
#define __DEFINED_pthread_rwlock_t
#define __DEFINED_pthread_barrier_t
#define __DEFINED_pthread_t
#define __DEFINED_pthread_spinlock_t
#define __DEFINED_pthread_mutexattr_t
#define __DEFINED_pthread_condattr_t
#define __DEFINED_pthread_barrierattr_t
#define __DEFINED_pthread_rwlockattr_t

#ifdef __wasm32__
#define ADDRESS_PADDING(name)   long name;
#else
#define ADDRESS_PADDING(name)
#endif

#define JENOVA_DEFINE_PTR(type, name)   \
    type name;  \
    ADDRESS_PADDING(padding_unused_##name);

#ifdef __cplusplus
extern "C" {
#endif

#define _Atomic(type) type

#define JENOVA_PTHREAD_MUTEX_INITIALIZER  {{{0, 0, 0, 0, 0, 0, 0}}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define JENOVA_PTHREAD_RWLOCK_INITIALIZER {{{0, 0, 0, 0, 0, 0, 0, 0}}, {0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L}}
#define JENOVA_PTHREAD_COND_INITIALIZER {{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}, {0}}

typedef struct { union { int __i[11]; volatile int __vi[11]; unsigned __s[11]; } __u; char _padding[20]; } pthread_attr_t;
// ios/mac pthread_t is 64 byte. addtional with a atomic exchange place (sizeof(int)) for init mutlithread copy. 68 byte
typedef struct { union { int __i[7]; volatile int __vi[7]; volatile void *__p[7]; } __u; int _padding[10]; } pthread_mutex_t;
typedef struct { union { int __i[7]; volatile int __vi[7]; volatile void *__p[7]; } __u; int _padding[10]; } mtx_t;
// ios/mac pthread_cond_t is 48 byte. addtional with a atomic exchange place (sizeof(int)) for init mutlithread copy. 52 byte
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; int _padding[1]; } pthread_cond_t;
typedef struct { union { int __i[12]; volatile int __vi[12]; void *__p[12]; } __u; int _padding[1]; } cnd_t;
// ios/mac pthread_rwlock_t is 200 byte. addtional with a atomic exchange place (sizeof(int)) for init mutlithread copy. 204 byte
typedef struct { union { int __i[8]; volatile int __vi[8]; void *__p[8]; } __u; unsigned long long _padding[22]; } pthread_rwlock_t;
typedef struct { union { int __i[5]; volatile int __vi[5]; void *__p[5]; } __u; } pthread_barrier_t;

// forbiden use long in this file. as this struct will allocate in native.

typedef struct __locale_struct* locale_t;
// 'struct pthread' is the same memory align with native! Never change it unless you understand what means.
#pragma pack (push, 1)
struct __pthread
{
    unsigned long long native_pthread_t[2];   // native pthread_t; posix is unsigned long, windows's size if (unsigned long long + unsigned int)

    int stack;
	int stack_size;
	int start;
	int start_param;
	int result;
	unsigned long long native_runtime;

    JENOVA_DEFINE_PTR(struct __pthread*, self);
    JENOVA_DEFINE_PTR(void**, dtv);
    unsigned long long sysinfo;
    unsigned long long CANARY;
    unsigned long long tid;
    int tsd_used, errno_val;
    volatile int cancel, canceldisable, cancelasync;
	int detached;
    JENOVA_DEFINE_PTR(struct __ptcb *, cancelbuf);
	JENOVA_DEFINE_PTR(void**, tsd);
    int unblock_cancel;
    volatile int timer_id;
    JENOVA_DEFINE_PTR(locale_t, locale);
    volatile int killlock[2];
	volatile int exitlock[2];
	volatile int startlock[2];
	JENOVA_DEFINE_PTR(char *, dlerror_buf);
	int dlerror_flag;
	JENOVA_DEFINE_PTR(void*, stdio_locks);
	unsigned long long canary_at_end;
	JENOVA_DEFINE_PTR(void **, dtv_copy);
	//	JENOVA_DEFINE_PTR(unsigned char*, map_base);
//	unsigned long long map_size;
};
#pragma pack (pop)

typedef struct __pthread* pthread_t;

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

#ifdef JENOVA_USE_NATIVE_PRINT
#define printf printf_native
#endif

#define __c11_atomic_load __atomic_load_n_native
#define __c11_atomic_store __atomic_store_n_native
#define __c11_atomic_init __c11_atomic_init_native
#define __c11_atomic_exchange __atomic_exchange_n_native
#define __c11_atomic_thread_fence __atomic_thread_fence_native
#define __c11_atomic_signal_fence __atomic_signal_fence_native
#define __c11_atomic_compare_exchange_strong(ptr, expected, desired, success_memorder, failure_memorder) __atomic_compare_exchange_n_native(ptr, expected, desired, 0, success_memorder, failure_memorder)
#define __c11_atomic_compare_exchange_weak(ptr, expected, desired, success_memorder, failure_memorder) __atomic_compare_exchange_n_native(ptr, expected, desired, 1, success_memorder, failure_memorder)
#define __c11_atomic_fetch_add __atomic_fetch_add_native
#define __c11_atomic_fetch_sub __atomic_fetch_sub_native
#define __c11_atomic_fetch_and __atomic_fetch_and_native
#define __c11_atomic_fetch_nand __atomic_fetch_nand_native
#define __c11_atomic_fetch_or __atomic_fetch_or_native
#define __c11_atomic_fetch_xor __atomic_fetch_xor_native
#define __c11_atomic_add_fetch __atomic_add_fetch_native
#define __c11_atomic_sub_fetch __atomic_sub_fetch_native
#define __c11_atomic_and_fetch __atomic_and_fetch_native
#define __c11_atomic_nand_fetch __atomic_nand_fetch_native
#define __c11_atomic_or_fetch __atomic_or_fetch_native
#define __c11_atomic_xor_fetch __atomic_xor_fetch_native

#define __atomic_load __atomic_load_native
#define __atomic_load_n __atomic_load_n_native
#define __atomic_store __atomic_store_native
#define __atomic_store_n __atomic_store_n_native
#define __atomic_exchange __atomic_exchange_native
#define __atomic_exchange_n __atomic_exchange_n_native
#define __atomic_compare_exchange __atomic_compare_exchange_native
#define __atomic_compare_exchange_n __atomic_compare_exchange_n_native
#define __atomic_add_fetch __atomic_add_fetch_native
#define __atomic_sub_fetch __atomic_sub_fetch_native
#define __atomic_and_fetch __atomic_and_fetch_native
#define __atomic_xor_fetch __atomic_xor_fetch_native
#define __atomic_or_fetch __atomic_or_fetch_native
#define __atomic_nand_fetch __atomic_nand_fetch_native
#define __atomic_fetch_add __atomic_fetch_add_native
#define __atomic_fetch_sub __atomic_fetch_sub_native
#define __atomic_fetch_and __atomic_fetch_and_native
#define __atomic_fetch_xor __atomic_fetch_xor_native
#define __atomic_fetch_or __atomic_fetch_or_native
#define __atomic_fetch_nand __atomic_fetch_nand_native
#define __atomic_test_and_set __atomic_test_and_set_native
#define __atomic_clear __atomic_clear_native
#define __atomic_thread_fence __atomic_thread_fence_native
#define __atomic_signal_fence __atomic_signal_fence_native

#define __sync_fetch_and_add __sync_fetch_and_add_native
#define __sync_fetch_and_sub __sync_fetch_and_sub_native
#define __sync_fetch_and_or __sync_fetch_and_or_native
#define __sync_fetch_and_and __sync_fetch_and_and_native
#define __sync_fetch_and_xor __sync_fetch_and_xor_native
#define __sync_fetch_and_nand __sync_fetch_and_nand_native
#define __sync_add_and_fetch __sync_add_and_fetch_native
#define __sync_sub_and_fetch __sync_sub_and_fetch_native
#define __sync_or_and_fetch __sync_or_and_fetch_native
#define __sync_and_and_fetch __sync_and_and_fetch_native
#define __sync_xor_and_fetch __sync_xor_and_fetch_native
#define __sync_nand_and_fetch __sync_nand_and_fetch_native
#define __sync_bool_compare_and_swap __sync_bool_compare_and_swap_native
#define __sync_val_compare_and_swap __sync_val_compare_and_swap_native
#define __sync_synchronize __sync_synchronize_native
#define __sync_lock_test_and_set __sync_lock_test_and_set_native
#define __sync_lock_release __sync_lock_release_native

///////////// c11 atomic start ////////////
void __c11_atomic_init_size(volatile void* ptr, long long val, int size);
#define __c11_atomic_init_native(ptr, val) __c11_atomic_init_size(ptr, val, sizeof(val))
///////////// c11 atomic end ////////////
// another redefine method: https://en.cppreference.com/w/c/language/generic

long long __atomic_load_n_size(const volatile void* ptr, int memorder, int size);
#ifdef __cplusplus
#define __atomic_load_n_native(ptr, memorder) (remove_reference_predefine_t<decltype(*ptr)>)(__atomic_load_n_size(ptr, memorder, sizeof(remove_reference_predefine_t<decltype(*ptr)>)))
#else
#define __atomic_load_n_native(ptr, memorder) (typeof(*ptr))(__atomic_load_n_size(ptr, memorder, sizeof(*(ptr))))
#endif
float __atomic_load_n_float(volatile float * ptr, int memorder);
double __atomic_load_n_double(volatile double * ptr, int memorder);

void __atomic_load_size(const volatile void* ptr, void* ret, int memorder, int size);
#define __atomic_load_native(ptr, ret, memorder) __atomic_load_size(ptr, ret, memorder, sizeof(*(ret)))

void __atomic_store_n_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_store_n_native(ptr, val, memorder) __atomic_store_n_size(ptr, val, memorder, sizeof(val))
void __atomic_store_n_float(volatile float * ptr, float val, int memorder);
void __atomic_store_n_double(volatile double * ptr, double val, int memorder);

long long __atomic_exchange_n_size(const volatile void *ptr, long long val, int memorder, int size);
#ifdef __cplusplus
#define __atomic_exchange_n_native(ptr, val, memorder) (decltype(val))__atomic_exchange_n_size(ptr, (long long)(intptr_t)(val), memorder, sizeof(val))
#else
#define __atomic_exchange_n_native(ptr, val, memorder) (typeof(val))__atomic_exchange_n_size(ptr, (long long)(intptr_t)(val), memorder, sizeof(val))
#endif
float __atomic_exchange_n_float(volatile float* ptr, float val, int memorder);
double __atomic_exchange_n_double(volatile double * ptr, double val, int memorder);

void __atomic_exchange_size(const volatile void *ptr, void *val, void *ret, int memorder, int size);
#define __atomic_exchange_native(ptr, val, ret, memorder) __atomic_exchange_size(ptr, val, ret, memorder, sizeof(*(val)))

int __atomic_compare_exchange_n_size(const volatile void *ptr, void *expected, long long desired, int weak, int success_memorder, int failure_memorder, int size);
#define __atomic_compare_exchange_n_native(ptr, expected, desired, weak, success_memorder, failure_memorder)   \
__atomic_compare_exchange_n_size(ptr, expected, desired, weak, success_memorder, failure_memorder, sizeof(desired))
float __atomic_compare_exchange_n_float(volatile float * ptr, float* expected, float desired, int weak, int success_memorder, int failure_memorder);
double __atomic_compare_exchange_n_double(volatile double * ptr, double* expected, double desired, int weak, int success_memorder, int failure_memorder);

int __atomic_compare_exchange_size(const volatile void *ptr, void *expected, void *desired, int weak, int success_memorder, int failure_memorder, int size);
#define __atomic_compare_exchange_native(ptr, expected, desired, weak, success_memorder, failure_memorder) \
__atomic_compare_exchange_size(ptr, expected, desired, weak, success_memorder, failure_memorder, sizeof(*(ptr)))

long long __atomic_add_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#ifdef __cplusplus
#define __atomic_add_fetch_native(ptr, val, memorder) (decltype(val))__atomic_add_fetch_size(ptr, val, memorder, sizeof(val))
#else
#define __atomic_add_fetch_native(ptr, val, memorder) (typeof(val))__atomic_add_fetch_size(ptr, val, memorder, sizeof(val))
#endif

long long __atomic_sub_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#ifdef __cplusplus
#define __atomic_sub_fetch_native(ptr, val, memorder) (decltype(val))__atomic_sub_fetch_size(ptr, val, memorder, sizeof(val))
#else
#define __atomic_sub_fetch_native(ptr, val, memorder) (typeof(val))__atomic_sub_fetch_size(ptr, val, memorder, sizeof(val))
#endif

long long __atomic_and_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#ifdef __cplusplus
#define __atomic_and_fetch_native(ptr, val, memorder) (decltype(val))__atomic_and_fetch_size(ptr, val, memorder, sizeof(val))
#else
#define __atomic_and_fetch_native(ptr, val, memorder) (typeof(val))__atomic_and_fetch_size(ptr, val, memorder, sizeof(val))
#endif

long long __atomic_xor_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_xor_fetch_native(ptr, val, memorder) __atomic_xor_fetch_size(ptr, val, memorder, sizeof(val))

long long __atomic_or_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_or_fetch_native(ptr, val, memorder) __atomic_or_fetch_size(ptr, val, memorder, sizeof(val))

long long __atomic_nand_fetch_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_nand_fetch_native(ptr, val, memorder) __atomic_nand_fetch_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_add_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_add_native(ptr, val, memorder) __atomic_fetch_add_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_sub_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_sub_native(ptr, val, memorder) __atomic_fetch_sub_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_and_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_and_native(ptr, val, memorder) __atomic_fetch_and_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_xor_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_xor_native(ptr, val, memorder) __atomic_fetch_xor_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_or_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_or_native(ptr, val, memorder) __atomic_fetch_or_size(ptr, val, memorder, sizeof(val))

long long __atomic_fetch_nand_size(const volatile void *ptr, long long val, int memorder, int size);
#define __atomic_fetch_nand_native(ptr, val, memorder) __atomic_fetch_nand_size(ptr, val, memorder, sizeof(val))

int __atomic_test_and_set_native(const volatile void *ptr, int memorder);
void __atomic_clear_native (const volatile int *ptr, int memorder);
void __atomic_thread_fence_native (int memorder);
void __atomic_signal_fence_native (int memorder);

long long __sync_lock_test_and_set_size(const volatile void *ptr, long long value, int size);
#define __sync_lock_test_and_set_native(ptr, value) __sync_lock_test_and_set_size(ptr, value, sizeof(value))

void __sync_lock_release_size(const volatile void* ptr, int size);
#define __sync_lock_release_native(ptr)  __sync_lock_release_size(ptr, sizeof(*ptr))

#ifdef __cplusplus
}
#endif

#endif