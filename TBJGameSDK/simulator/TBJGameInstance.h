/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#pragma once

#include <vector>
#include <string>
#include "TBJGameEnviroment.h"

class TBJEnvContext;
class TBJStorageContext;

class TBJGameInstance
{
public:
    TBJGameInstance();
    ~TBJGameInstance();
    
    inline TBJEnvContext* getEnvContext() { return mEnvContext; }
    inline void setEnvContext(TBJEnvContext* context) { mEnvContext = context; }
    inline TBJStorageContext* getStorageContext() { return mStorageContext; }
    inline void setStorageContext(TBJStorageContext* context) { mStorageContext = context; }
    
    void initGLESFrameBuffer(int glVersion);
    bool resizeBuffer();
    
    inline bool isGameShouldClose() { return mShouldClose; }
    inline void setGameShouldClose(bool value) { mShouldClose = value; }
    
    void gameLoopOnce(float time);
    inline void invokeGameEvent(const TBJGameEvent& event)
    {
        mEventPool.push_back(event);
    }
    
    inline void setGameLoop(TBJGameLoop gameLoop) { mGameLoop = gameLoop; }
    inline void setEventListener(TBJGameEventListener evtListener) { mEventListener = evtListener; }
    
private:
    TBJEnvContext* mEnvContext = nullptr;
    TBJStorageContext* mStorageContext = nullptr;
    
    unsigned int mDefaultFrameBuffer = 0;
    unsigned int mDefaultColorBuffer = 0;
    unsigned int mDefaultDepthBuffer = 0;
    int mFrameBufferWidth = 0;
    int mFrameBufferHeight = 0;
    
    bool mShouldClose = false;
    
    std::vector<TBJGameEvent> mEventPool;
    std::vector<TBJGameEvent> mExecutingEvent;
    
    TBJGameLoop mGameLoop = nullptr;
    TBJGameEventListener mEventListener = nullptr;
};
