/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#pragma once

#include <string>
#include <map>

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
    std::map<std::string, std::pair<std::string, bool>> mFetchCache;
    
    std::string mPath;
    bool mInited;
    
    void* mDb;
    void* mSelect;
    void* mRemove;
    void* mUpdate;
    void* mClear;
};
