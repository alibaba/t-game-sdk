#pragma once

#include <queue>
#include <vector>

#ifdef JENOVA_SIMULATOR
#include "SpinLock.h"
#else
#include "Thread/SpinLock.h"
#endif

#include <__mutex_base>

namespace WasmGame {
    template<typename T>
    class Queue {
    public:
        void push(T v)
        {
//            std::lock_guard<SpinLock> guard(mLock);
            mLock.lock();
            mQueue.push(v);
            mLock.unlock();
        }
        
        bool pop(T& v)
        {
            std::lock_guard<SpinLock> guard(mLock);
            if (!mQueue.empty())
            {
                v = mQueue.front();
                mQueue.pop();
                return true;
            }
            return false;
        }
        
        void swap(std::queue<T>& q)
        {
            mLock.lock();
            mQueue.swap(q);
            mLock.unlock();
        }
        
    private:
        std::queue<T> mQueue;
        SpinLock mLock;
    };

    template<typename T>
    class Vector {
    public:
        inline void push_back(T v)
        {
            mLock.lock();
            mVec.push_back(v);
            mLock.unlock();
        }
        
        inline void extend(T*const begin, T*const end)
        {
            mLock.lock();
            mVec.insert(mVec.end(), begin, end);
            mLock.unlock();
        }
        
        
        inline void swap(std::vector<T>& q)
        {
            mLock.lock();
            mVec.swap(q);
            mLock.unlock();
        }
        
        inline void reserve(int size)
        {
            mLock.lock();
            mVec.reserve(size);
            mLock.unlock();
        }
        
        inline void clear()
        {
            std::lock_guard<SpinLock> guard(mLock);
            mVec.clear();
        }
        
        inline void clearNDestruct()
        {
            mLock.lock();
            mVec.length = 0;
            mLock.unlock();
        }
        
    private:
        std::vector<T> mVec;
        SpinLock mLock;
    };
}
