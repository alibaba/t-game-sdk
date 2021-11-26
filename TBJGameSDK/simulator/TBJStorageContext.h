/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#pragma once

#include <string>

class TBJStorageContext
{
public:
    TBJStorageContext(const char* path);
    ~TBJStorageContext();
    
    void putValue(const char* key, const char* msg);
    const char* getValue(const char* key);
    void removeKey(const char* key);
    void clear();
    
private:
    void init();
    void deinit();
    
private:
    std::string mPath;
    bool mInited;
    
    void* mDb;
    void* mSelect;
    void* mRemove;
    void* mUpdate;
    void* mClear;
};
