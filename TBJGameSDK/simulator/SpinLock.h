#pragma once

#include <atomic>
#include <memory>

namespace WasmGame {
    class SpinLock {
    public:
        constexpr SpinLock() = default;
        ~SpinLock() = default;
        
        inline void lock() {
            static_assert(sizeof(lock_) == sizeof(int), "int and lock_ are different sizes");
            while (lock_.exchange(true, std::memory_order_acquire)) {}
        }
        
        inline void unlock() {
            lock_.store(false, std::memory_order_release);
        }
        
    private:
        std::atomic_int lock_{0};
    };
}
