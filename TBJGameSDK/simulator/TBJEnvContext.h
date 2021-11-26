/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#pragma once

#include <map>
#include <string>

class TBJEnvContext
{
public:
    static const char* DRIVER;
    static const char* GLVERSION;
    static const char* APPNAME;
    static const char* GAMEURL;
    static const char* ORIENTATION;
    
public:
    inline void setGameRootPath(const char* rootPath) { mGameRootPath = rootPath; }
    inline void setGameWritePath(const char* writePath) { mGameWritePath = writePath; }
    inline const char* getGameRootPath() const { return mGameRootPath.c_str(); }
    inline const char* getGameWritePath() const { return mGameWritePath.c_str(); }
    
    inline void* getNativeContext() { return mNativeContext; }
    inline void setNativeContext(void* n) { mNativeContext = n; }
    
public:
    int getInt(const char*);
    void put(const char* key, const char* value);
    const char* getString(const char* key);
    bool parseFromConifg(const char* url);
    
private:
    std::string unescape(const std::string& url);
    
private:
    std::string mGameRootPath;
    std::string mGameWritePath;
    void* mNativeContext;
    
private:
    std::map<std::string, std::string> mConfig;
    char mErrorMsg[512];
};
