/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#ifndef TBJHttpCallbackManager_h
#define TBJHttpCallbackManager_h

#ifdef JENOVA_SIMULATOR
#include "Queue.h"
#else
#include "Thread/Queue.h"
#endif

#include "TBJGameHttp_Internal.h"
#include <map>
#include <stdio.h>

class TBJHttpCallbackTask {
public:
    TBJHttpCallbackTask(TBJResponseCallback cb, TBJHttpResponseInternal* response, TBJHttpStage stage):callback(cb), response(response), stage(stage) {
    }
    TBJResponseCallback callback;
    TBJHttpResponseInternal* response;
    TBJHttpStage stage;
};

class TBJHttpCallbackManager {
private:
    WasmGame::Queue<std::shared_ptr<TBJHttpCallbackTask>> callbackQueue;
    
public:
    void readyToInvoke(TBJHttpResponseInternal* response, TBJResponseCallback callback, TBJHttpStage stage);

    void inovokeHttpCallbacks();
};

#endif /* TBJHttpCallbackManager_h */
